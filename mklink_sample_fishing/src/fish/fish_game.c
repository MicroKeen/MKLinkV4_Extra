#include "fish_game.h"
#include <string.h>
#include <limits.h>
#include <stdlib.h>

static int clamp(int n, int lo, int hi) { return n < lo ? lo : n > hi ? hi : n; }
const char *const fish_sound_names[FISH_SOUND_COUNT]={"关闭","仅鱼讯","全部开启"};
static void phase(fish_game_t *g, fish_mode_t mode) { g->mode = mode; g->phase_at = g->now; }

static void lost(fish_game_t *g, fish_loss_t reason)
{
    int dx,dy;fish_game_float(g,&dx,&dy);
    g->float_dx_q8=dx*256;g->float_dy_q8=dy*256;
    g->loss=reason;g->pressed=false;g->boat_speed=0;g->boat_motion_q8=0;
    phase(g,FISH_LOST);
}

/* Initial settling time also gives the first burst a readable warning. */
static int fight_cycle_phase(const fish_game_t *g,uint32_t elapsed_ms)
{
    if (g->stamina_recovering || elapsed_ms<fish_rules.hook_settle_ms || fish_species[g->pending_species].fight_style==FISH_FIGHT_PASSIVE) return -1;
    return (elapsed_ms-fish_rules.hook_settle_ms)%fish_species[g->pending_species].fight_cycle_ms;
}

bool fish_game_burst_active(const fish_game_t *g)
{
    if (g->mode!=FISH_FIGHT) return false;
    int p=fight_cycle_phase(g,g->now-g->phase_at);
    return p>=0 && p<fish_species[g->pending_species].burst_ms;
}

int fish_game_pull(const fish_game_t *g, uint32_t elapsed_ms)
{
    const fish_species_t *s=&fish_species[g->pending_species];
    if(g->stamina_recovering) return clamp(g->base_pull/8,3,12);
    int pull=g->base_pull*g->struggle_percent/100;
    /* Exhaustion opens recovery opportunities; cadence remains species-specific. */
    int fatigue=g->stamina_max?(g->stamina_max-g->stamina)*fish_rules.fatigue_max_percent/g->stamina_max:0;
    pull=pull*(100-fatigue)/100;
    int p=fight_cycle_phase(g,elapsed_ms);
    if (p>=0 && p<s->burst_ms) pull+=s->burst_bonus*pull/90;
    return clamp(pull,3,100);
}

static unsigned strike_deadline(const fish_game_t *g)
{
    const fish_species_t *s=&fish_species[g->pending_species];
    unsigned window=(s->bite_window_ms-s->strike_delay_ms)*fish_line_sets[g->line_index].strike_window_percent/100;
    return s->strike_delay_ms+window*(100+fish_floats[g->float_index].strike_window_bonus_percent)/100;
}
bool fish_game_strike_ready(const fish_game_t *g)
{
    return g->mode==FISH_BITE && g->pending_species<FISH_SPECIES_COUNT &&
        g->now-g->phase_at>=fish_species[g->pending_species].strike_delay_ms &&
        g->now-g->phase_at<strike_deadline(g);
}

int fish_game_distance(const fish_game_t *g) { return 10000-g->reel_units; }
unsigned fish_game_max_cast_px(const fish_game_t *g)
{
    unsigned rod=fish_rods[g->rod_index].max_cast_px;
    unsigned line=fish_line_sets[g->line_index].distance_limit_cm/FISH_WORLD_CM_PER_PIXEL;
    return rod<line?rod:line;
}
unsigned fish_game_rod_flex(const fish_game_t *g)
{
    int extra=fish_game_distance(g)-(int)g->line_capacity_units;
    if(g->mode!=FISH_FIGHT || extra<=0)return 0;
    unsigned room=g->line_capacity_units/10;if(!room)room=1;
    return (unsigned)clamp(20+extra*60/(int)room+g->range_ms*20/2000,20,100);
}

/* Virtual world scale: 25 pixels per metre. Preserve normalized reel progress
 * for fixed-point motion; convert the actual cast span and physical line length. */
static unsigned initial_distance_cm(const fish_game_t *g)
{
    int span=g->cast_x-g->cast_start_x;if(span<0) span=-span;
    if(!span) span=g->cast_distance?g->cast_distance:fish_rods[g->rod_index].max_cast_px;
    return (unsigned)span*FISH_WORLD_CM_PER_PIXEL;
}
unsigned fish_game_distance_cm(const fish_game_t *g)
{
    int distance=fish_game_distance(g);
    return distance>0?((unsigned)distance*initial_distance_cm(g)+9999)/10000:0;
}
static void spend_stamina(fish_game_t *g,unsigned cost)
{
    if(g->stamina_recovering) return;
    g->stamina=(uint16_t)(g->stamina>cost?g->stamina-cost:0);
    if(!g->stamina) {g->stamina_recovering=true;g->recovery_ms=0;}
}


bool fish_game_burst_warning(const fish_game_t *g)
{
    if (g->mode!=FISH_FIGHT || g->stamina_recovering) return false;
    const fish_species_t *s=&fish_species[g->pending_species];
    if (s->fight_style==FISH_FIGHT_PASSIVE) return false;
    uint32_t elapsed=g->now-g->phase_at;
    if (elapsed<fish_rules.hook_settle_ms)
        return fish_rules.hook_settle_ms-elapsed<=fish_rules.burst_warning_ms;
    return fight_cycle_phase(g,elapsed)>=s->fight_cycle_ms-fish_rules.burst_warning_ms;
}

/* Continuous periodic lobes, -256..256; integer arithmetic and no lookup table. */
static int wave_q8(uint32_t elapsed, unsigned period)
{
    unsigned p=(elapsed%period)*1024/period;
    unsigned half=p%512;
    int value=(int)(half*(512-half)/256);
    return p<512?value:-value;
}

int fish_game_splash(const fish_game_t *g)
{
    if (g->mode!=FISH_FIGHT) return 0;
    const fish_species_t *s=&fish_species[g->pending_species];
    if (s->fight_style!=FISH_FIGHT_ACTIVE) return 0;
    int p=fight_cycle_phase(g,g->now-g->phase_at);
    if (p<0 || p>=s->burst_ms) return 0;
    unsigned ramp=s->burst_ms/3;
    unsigned edge=p<s->burst_ms-p?p:s->burst_ms-p;
    return (1+g->base_pull/18)*(int)(edge<ramp?edge:ramp)/(int)ramp;
}

void fish_game_float(const fish_game_t *g, int *dx, int *dy)
{
    uint32_t elapsed=g->now-g->phase_at;
    *dx=0; *dy=0;
    if (g->pending_species>=FISH_SPECIES_COUNT) return;
    const fish_species_t *s=&fish_species[g->pending_species];
    if (g->mode==FISH_FIGHT || g->mode==FISH_LOST) {
        *dx=g->float_dx_q8/256; *dy=g->float_dy_q8/256;
        return;
    }
    if (g->mode!=FISH_WAIT && g->mode!=FISH_BITE) return;
    if (g->mode==FISH_WAIT) {
        /* Concealment changes bite frequency, preserving the final nibble cue. */
        unsigned wait=g->bite_wait_ms?g->bite_wait_ms:fish_rules.bite_attempt_ms;
        if (elapsed>=wait-900) *dy=(256-wave_q8(elapsed-(wait-900)+180,720))/256;
        return;
    }
    switch (s->bite_style) {
        case 0: *dy=elapsed<400?(256-wave_q8(elapsed,200))/256:
                    -clamp((int)(elapsed-400)*4/160,0,4); break;
        case 1: *dy=clamp((int)(elapsed/100),0,8); *dx=clamp((int)(elapsed/160),0,10); break;
        case 2: *dy=clamp((int)elapsed*8/160,0,8)+wave_q8(elapsed,240)/128;
                *dx=wave_q8(elapsed,480)/128; break;
        case 3: *dy=clamp((int)(elapsed/150)+2,2,10); break;
        default: *dy=clamp((int)elapsed/100,0,2); *dx=wave_q8(elapsed,2000)*9/256; break;
    }
}

static void fight_update(fish_game_t *g)
{
    /* A suspended/background simulator pauses cadence; bounded to 50 catch-up ticks.
     * Normal timestamped input and rendering keep the exact 20ms simulation. */
    uint32_t gap=g->now-g->simulation_at;
    if (gap>1000) { g->simulation_at+=gap-1000; g->phase_at+=gap-1000; }
    while (g->mode==FISH_FIGHT && g->now-g->simulation_at>=20) {
        g->simulation_at+=20;
        uint32_t elapsed=g->simulation_at-g->phase_at;
        int pull=fish_game_pull(g,elapsed);
        int response_pull=pull*g->line_response_percent/100;
        const fish_species_t *s=&fish_species[g->pending_species];
        int p=fight_cycle_phase(g,elapsed);
        bool burst=p>=0 && p<s->burst_ms;
        /* Fish can load the line even without retrieval. Rate-limited pressure
         * and stronger payout leave a deliberate recovery action each 100ms. */
        if (elapsed%100==0) {
            int target=clamp(burst?20+response_pull:20+response_pull*3/5,0,g->line_strength+20);
            int rise=s->fight_style!=FISH_FIGHT_ACTIVE?1:burst?clamp(2+response_pull/20,2,7):2;
            g->tension+=clamp(target-g->tension,-2,rise);
            if (fish_game_distance(g)>=g->line_capacity_units && burst)
                g->tension=clamp(g->tension+4,0,g->line_strength+20);
            if (g->tension>=g->line_strength*fish_rules.stamina_min_tension_percent/100 && g->tension<g->line_strength*9/10) {
                if(burst) spend_stamina(g,fish_rules.stamina_burst_cost);
            }
        }
        if(g->stamina_recovering) {
            g->recovery_ms+=20;
            if(g->recovery_ms>=fish_rules.stamina_recovery_delay_ms+fish_rules.stamina_recovery_interval_ms) {
                g->recovery_ms=fish_rules.stamina_recovery_delay_ms;
                if(g->stamina<g->stamina_max) ++g->stamina;
                unsigned resume=(g->stamina_max*fish_rules.stamina_resume_percent+99)/100;
                if(g->stamina>=resume) {g->stamina_recovering=false;g->recovery_ms=0;}
            }
            burst=false;
        }
        if (burst) {
            if(!g->run_burst)g->run_direction=fish_random_next(&g->motion_random_state)%100<70?1:-1;
            int run=s->fight_style==FISH_FIGHT_SLOW?pull/35:2+pull/20;
            run=(run*s->run_percent+50)/100;
            unsigned flex=g->line_capacity_units/10;if(!flex)flex=1;
            g->reel_units=clamp(g->reel_units-g->run_direction*run,10000-(int)(g->line_capacity_units+flex),9999);
        }
        g->run_burst=burst;
        /* Bring the contact point toward the boat as line is retrieved. Filter
         * detents and burst transitions so neither can teleport the float. */
        int progress=clamp(g->reel_units,-8000,10000);
        int target_x=(g->cast_start_x-g->cast_x)*g->facing*progress*256/10000;
        if (g->motion_remaining_ms<=20) {
            /* A separate visual stream never changes species/size draws. */
            uint32_t r=g->motion_random_state;
            r^=r<<13; r^=r>>17; r^=r<<5;
            g->motion_random_state=r;
            g->motion_target_q8=(int16_t)((int)(r%513)-256);
            g->motion_remaining_ms=(uint16_t)(s->fight_style==FISH_FIGHT_ACTIVE?450+(r>>16)%950:1800+(r>>16)%1200);
        } else g->motion_remaining_ms-=20;
        int amplitude=g->stamina_recovering?1:1+g->base_pull/12;
        if (s->fight_style==FISH_FIGHT_SLOW) amplitude=(amplitude+2)/3;
        else if (s->fight_style==FISH_FIGHT_PASSIVE) amplitude=1;
        target_x+=g->motion_target_q8*amplitude;
        if (burst && s->fight_style==FISH_FIGHT_ACTIVE) target_x+=wave_q8(elapsed,420)*(1+g->base_pull/30);
        int target_y=256+wave_q8(elapsed,900)/2+(burst?pull*(s->fight_style==FISH_FIGHT_SLOW?2:12):0);
        g->float_dx_q8+=(target_x-g->float_dx_q8)/6;
        g->float_dy_q8+=(target_y-g->float_dy_q8)/6;
        g->reel=clamp(g->reel_units/100,0,100);
        g->overload_ms=(uint16_t)clamp(g->overload_ms+(g->tension>=g->line_strength?20:
            -(int)fish_rules.overload_recovery_per_tick_ms),0,fish_rules.overload_limit_ms);
        g->slack_ms=(uint16_t)clamp(g->slack_ms+(g->tension<=g->line_strength*12/100?(burst?40:20):
            -(int)fish_rules.slack_recovery_per_tick_ms),0,fish_rules.slack_limit_ms);
        bool stretched=fish_game_distance(g)>g->line_capacity_units;
        g->range_ms=stretched?(uint16_t)clamp(g->range_ms+20,0,fish_rules.range_limit_ms):0;
        if(stretched)g->overload_ms=0; /* Rod flex provides the explicit two-second rescue window. */
        if (g->range_ms>=fish_rules.range_limit_ms) lost(g,FISH_LOSS_BREAK);
        else if (g->overload_ms>=fish_rules.overload_limit_ms) lost(g,FISH_LOSS_BREAK);
        else if (g->slack_ms>=fish_rules.slack_limit_ms) lost(g,FISH_LOSS_SLACK);
    }
}

void fish_game_init(fish_game_t *g, uint32_t now)
{
    memset(g, 0, sizeof(*g));
    g->sound_mode=FISH_SOUND_ALL;
    g->boat_x = 150; g->facing = 1; g->now = now; g->mode = FISH_MOVE;
    g->simulation_at=now;
    g->random_state=now^0x8f235a19u;
    g->struggle_percent=100;
    g->line_strength=fish_rules.default_line_strength;
    g->line_capacity_units=fish_rules.default_line_capacity_units;
    g->rod_index=FISH_DEFAULT_ROD; g->float_index=FISH_DEFAULT_FLOAT;
    g->bait_index=FISH_DEFAULT_BAIT; g->depth_index=FISH_DEPTH_OPTION_COUNT-1;
    fish_game_equip_line(g,FISH_DEFAULT_LINE_SET);
    fish_economy_init(&g->economy);
    g->clock_at=now;g->game_minute=(uint16_t)((360+now/1000)%1440);
}

void fish_game_equip_line(fish_game_t *g,unsigned index)
{
    if (index>=FISH_LINE_COUNT) return;
    g->line_index=(uint8_t)index;
    g->line_strength=fish_line_sets[index].strength;
    g->line_capacity_units=fish_line_sets[index].capacity_units;
}
uint16_t fish_game_rig_depth(const fish_game_t *g) {return fish_depth_options[g->depth_index];}
bool fish_game_ashore(const fish_game_t *g)
{
    fish_mode_t mode=g->mode;
    if(mode==FISH_SHOP) return true;
    if(mode==FISH_HOLD) mode=(fish_mode_t)g->hold_return_mode;
    if (mode==FISH_GUIDE) mode=(fish_mode_t)g->guide_return_mode;
    if (mode==FISH_BAG) mode=(fish_mode_t)g->bag_return_mode;
    return mode==FISH_SHORE || mode==FISH_SHOP;
}
bool fish_game_at_shop(const fish_game_t *g) {return fish_game_ashore(g) && abs(g->shore_x-FISH_SHOP_X)<=FISH_SHOP_REACH;}
bool fish_game_at_station(const fish_game_t *g) {return fish_game_ashore(g) && abs(g->shore_x-FISH_STATION_X)<=FISH_STATION_REACH;}
bool fish_game_shop_open(const fish_game_t *g) {return g->game_minute>=FISH_SHOP_OPEN_MINUTE && g->game_minute<FISH_SHOP_CLOSE_MINUTE;}
unsigned fish_game_night_opacity(const fish_game_t *g)
{
    unsigned t=g->game_minute;
    if(t<300 || t>=1200) return 150;
    if(t<420) return (420-t)*150/120;
    if(t<1080) return 0;
    return (t-1080)*150/120;
}
int fish_game_actor_x(const fish_game_t *g) {return fish_game_ashore(g)?g->shore_x:g->boat_x;}
unsigned fish_game_shore_pose(const fish_game_t *g) {return g->mode==FISH_SHORE && g->shore_speed?1+(g->shore_walk_phase>=6):0;}
int fish_game_dock_action(const fish_game_t *g)
{
    int dock=fish_map_dock(g->boat_x);
    if (dock<0) return -1;
    int delta=(int)g->shore_x-fish_docks[dock].shore_x;
    if (fish_game_ashore(g) && (delta<-FISH_DOCK_REACH || delta>FISH_DOCK_REACH)) return -1;
    return dock;
}

int fish_game_power(const fish_game_t *g)
{
    uint32_t elapsed = g->now - g->pressed_at;
    if (elapsed <= FISH_HOLD_MS) return 0;
    if (elapsed >= FISH_FULL_CHARGE_MS) return 100;
    return (int)((elapsed - FISH_HOLD_MS) * 100 / (FISH_FULL_CHARGE_MS - FISH_HOLD_MS));
}

int fish_game_camera(const fish_game_t *g)
{
    int distance=0;
    if (g->mode==FISH_CHARGE) distance=28+fish_game_power(g)*(fish_game_max_cast_px(g)-28)/100;
    else if (g->mode>=FISH_CAST && g->mode<=FISH_FIGHT) distance=g->cast_distance;
    /* Pan only as a long cast grows, preserving the landing point on screen. */
    int shift=distance>92?distance-92:0;
    if(g->mode==FISH_LOST && g->cast_distance>92) {
        unsigned t=g->now-g->phase_at;
        shift=(g->cast_distance-92)*(t<600?600: t<FISH_LOSS_ANIMATION_MS?FISH_LOSS_ANIMATION_MS-t:0)/600;
    }
    bool ashore=fish_game_ashore(g);
    return clamp(fish_game_actor_x(g)-120+g->facing*shift,ashore?FISH_SHORE_CAMERA_LEFT:0,ashore?FISH_SHORE_CAMERA_RIGHT:FISH_WORLD_WIDTH-240);
}

void fish_game_update(fish_game_t *g, uint32_t now)
{
    uint32_t delta=now-g->clock_at;g->clock_at=now;
    unsigned remainder=delta%1000+g->clock_remainder_ms;
    g->game_minute=(uint16_t)((g->game_minute+delta/1000+remainder/1000)%1440);
    g->clock_remainder_ms=(uint16_t)(remainder%1000);
    g->now = now;
    fish_money_update(g);
    if(g->mode==FISH_GUIDE && g->pressed && g->previous_mode==FISH_GUIDE && now-g->pressed_at>=FISH_GUIDE_HOLD_MS) {
        g->pressed=false;phase(g,(fish_mode_t)g->guide_return_mode);
    }
    if(g->mode==FISH_SHORE && !g->pressed && g->shore_speed) {
        unsigned elapsed=delta>250?250:delta;
        unsigned movement=g->shore_remainder+elapsed*FISH_SHORE_SPEED;
        int prior=g->shore_x,next=prior+g->shore_speed*(int)(movement/1000);
        int dock=g->boat_x<FISH_MAP_WIDTH/2?0:1;
        g->shore_remainder=(uint16_t)(movement%1000);
        next=clamp(next,dock?fish_docks[dock].walk_left:FISH_SHORE_LEFT,fish_docks[dock].walk_right);
        if(!dock && ((prior>FISH_SHOP_X && next<=FISH_SHOP_X) || (prior<FISH_SHOP_X && next>=FISH_SHOP_X))) next=FISH_SHOP_X;
        if(dock && ((prior>FISH_STATION_X && next<=FISH_STATION_X) || (prior<FISH_STATION_X && next>=FISH_STATION_X))) next=FISH_STATION_X;
        int landing=fish_docks[dock].shore_x;
        if((prior<landing && next>=landing) || (prior>landing && next<=landing)) next=landing;
        g->shore_walk_phase=(uint8_t)((g->shore_walk_phase+abs(next-prior))%12);
        g->shore_x=(int16_t)next;
        if(movement>=1000 && (next==(dock?fish_docks[dock].walk_left:FISH_SHORE_LEFT) || next==fish_docks[dock].walk_right ||
           next==landing || (!dock && next==FISH_SHOP_X) || (dock && next==FISH_STATION_X))) g->shore_speed=0;
    }
    if(g->shop_closed_notice && now-g->shop_closed_at>=1600) g->shop_closed_notice=false;
    if(g->mode==FISH_SHOP && !fish_game_shop_open(g)) {
        g->merchant_reply=FISH_MERCHANT_CLOSED;g->pressed=false;phase(g,FISH_SHORE);
    }
    if(g->mode==FISH_LOST) {
        if(now-g->phase_at<FISH_LOSS_ANIMATION_MS) return;
        phase(g,FISH_AIM);g->simulation_at=now;
    }
    if ((g->mode==FISH_MOVE || g->mode==FISH_AIM) && !g->pressed && g->boat_speed) {
        if (now-g->simulation_at>1000) g->simulation_at=now-1000;
        while (now-g->simulation_at>=50) {
            g->simulation_at+=50;g->boat_motion_q8+=g->boat_speed*128;
            int delta=g->boat_motion_q8/256;g->boat_motion_q8-=delta*256;
            g->boat_x=clamp(g->boat_x+delta,FISH_BOAT_LEFT,FISH_BOAT_RIGHT);
            if (g->boat_x==FISH_BOAT_LEFT || g->boat_x==FISH_BOAT_RIGHT) {
                g->boat_speed=0;g->boat_motion_q8=0;
            }
        }
    } else if (g->mode!=FISH_FIGHT) {g->simulation_at=now;g->boat_motion_q8=0;}
    if (g->pressed && (g->mode == FISH_MOVE || g->mode == FISH_AIM)
        && now - g->pressed_at >= FISH_HOLD_MS) phase(g, FISH_CHARGE);
    if (g->mode == FISH_CAST && now - g->phase_at >= 650) phase(g, FISH_WAIT);
    if (g->mode == FISH_WAIT && now - g->phase_at >= (g->bite_wait_ms?g->bite_wait_ms:fish_rules.bite_attempt_ms)
        && g->pending_species<FISH_SPECIES_COUNT) { g->reel = 0; phase(g, FISH_BITE); }
    if (g->mode == FISH_BITE && now - g->phase_at >= strike_deadline(g)) lost(g,FISH_LOSS_MISSED);
    if (g->mode == FISH_FIGHT) fight_update(g);
    if (g->mode == FISH_CAUGHT && now - g->phase_at >= 4000) phase(g, FISH_AIM);
}

void fish_game_input(fish_game_t *g, fish_event_t event, int steps, uint32_t now)
{
    fish_mode_t before=g->mode;
    fish_game_update(g, now);
    if(g->mode==FISH_LOST || (before==FISH_LOST && event==FISH_RELEASE)) return;
    if (event == FISH_ROTATE) {
        if ((g->mode==FISH_MOVE || g->mode==FISH_AIM || g->mode==FISH_SHORE) && g->pressed && steps) {
            g->guide_return_mode=(uint8_t)g->mode;
            g->guide_index=g->last_species;
            g->guide_page=0;
            phase(g,FISH_GUIDE);
        } else if (g->mode==FISH_GUIDE && steps) {
            int index=(g->guide_index+clamp(steps,-40,40))%FISH_SPECIES_COUNT;
            g->guide_index=(uint8_t)(index<0?index+FISH_SPECIES_COUNT:index);
        } else if(g->mode==FISH_SHOP && steps) {
            int index=(g->shop_index+clamp(steps,-40,40))%FISH_SHOP_COUNT;
            g->shop_index=(uint8_t)(index<0?index+FISH_SHOP_COUNT:index);
        } else if(g->mode==FISH_HOLD && steps) {
            int count=(int)fish_hold_page_count(&g->economy);
            if(!count) {g->guide_index=0;return;}
            int index=(g->guide_index+clamp(steps,-40,40))%count;
            g->guide_index=(uint8_t)(index<0?index+count:index);
        } else if (g->mode==FISH_BAG && steps) {
            int count=FISH_BAG_COUNT,index=g->bag_index;
            if (g->bag_editing) {
                const int counts[]={FISH_ROD_COUNT,FISH_LINE_COUNT,FISH_FLOAT_COUNT,FISH_BAIT_COUNT,FISH_DEPTH_OPTION_COUNT};
                uint8_t *values[]={&g->rod_index,&g->line_index,&g->float_index,&g->bait_index,&g->depth_index};
                count=g->bag_index==FISH_BAG_SOUND?FISH_SOUND_COUNT:counts[g->bag_index];
                uint8_t *value=g->bag_index==FISH_BAG_SOUND?&g->sound_mode:values[g->bag_index];
                index=*value;
                int direction=steps>0?1:-1;
                for(int n=0;n<abs(clamp(steps,-40,40));n++) {
                    for(int tried=0;tried<count;tried++) {
                        index=(index+direction+count)%count;
                        if(g->bag_index==FISH_BAG_SOUND || fish_equipment_owned(&g->economy,g->bag_index,(unsigned)index)) break;
                    }
                }
                *value=(uint8_t)index;
                if (g->bag_index==FISH_BAG_LINE) fish_game_equip_line(g,g->line_index);
            } else {
                /* Bottom row reads guide, sound, save, return from left to right.
                 * Keep menu IDs stable while following their visible order. */
                static const uint8_t order[FISH_BAG_VISIBLE_COUNT]={FISH_BAG_ROD,FISH_BAG_LINE,FISH_BAG_FLOAT,
                    FISH_BAG_BAIT,FISH_BAG_DEPTH,FISH_BAG_MISC,FISH_BAG_GUIDE,FISH_BAG_SOUND,FISH_BAG_SAVE,FISH_BAG_BACK};
                count=FISH_BAG_VISIBLE_COUNT;
                for(index=0;index<count && order[index]!=g->bag_index;index++) {}
                index=(index+clamp(steps,-40,40))%count;
                g->bag_index=order[index<0?index+count:index];
            }
        } else if (g->mode==FISH_SHORE && !g->pressed && steps) {
            int dock=g->boat_x<FISH_MAP_WIDTH/2?0:1;
            if(!g->shore_speed && g->shore_x==fish_docks[dock].shore_x && (steps>0?1:-1)==(dock?-1:1)) {
                g->shore_speed=0;g->shore_remainder=0;g->boat_speed=0;g->boat_motion_q8=0;
                g->facing=dock?-1:1;phase(g,FISH_MOVE);return;
            }
            int direction=steps>0?1:-1;
            g->shore_speed=g->shore_speed && g->shore_speed!=direction?0:direction;
            g->shore_remainder=0;g->shore_walk_phase=0;
            if(g->shore_speed) g->facing=g->shore_speed;
        } else if ((g->mode==FISH_MOVE || g->mode==FISH_AIM) && !g->pressed && steps) {
            int direction=steps>0?1:-1,previous=g->boat_speed;
            int dock=fish_map_dock(g->boat_x);
            if(!previous && dock>=0 && g->boat_x==fish_docks[dock].boat_x && direction==(dock?1:-1)) {
                g->shore_x=fish_docks[dock].shore_x;g->shore_speed=direction;g->shore_remainder=0;
                g->boat_motion_q8=0;g->facing=direction;phase(g,FISH_SHORE);return;
            }
            unsigned count=(unsigned)clamp(steps>0?steps:-steps,1,40);
            while (count--) {
                if (!g->boat_speed) g->boat_speed=(int8_t)direction;
                else if (g->boat_speed*direction>0) g->boat_speed=(int8_t)(direction*2);
                else g->boat_speed=0;
            }
            if (g->boat_speed) g->facing=g->boat_speed>0?1:-1;
            if (g->boat_speed!=previous) {g->simulation_at=now;g->boat_motion_q8=0;}
        } else if (g->mode == FISH_FIGHT) {
            int pull=fish_game_pull(g,now-g->phase_at);
            steps=clamp(steps,-40,40);
            if (steps>0) {
                /* Each detent costs tension; reeling during bursts is less effective. */
                for (int i=0;i<steps;i++) {
                    g->tension=clamp(g->tension+2+pull/20,0,g->line_strength+20);
                    int tired=g->stamina_max?(g->stamina_max-g->stamina)*100/g->stamina_max:100;
                    int gain=(850-pull*5)*fish_rods[g->rod_index].reel_percent/100;
                    if (g->line_response_percent>=150 && g->pending_weight_g>
                        (uint32_t)fish_line_sets[g->line_index].comfortable_weight_g*4)
                        gain=gain*75/100; /* Severe tackle mismatch loses retrieval leverage. */
                    if (fish_species[g->pending_species].fight_style==FISH_FIGHT_ACTIVE)
                        gain=gain*(75+tired*25/100)/100;
                    bool flexing=fish_game_rod_flex(g)>0;
                    if(g->tension<g->line_strength*9/10 || flexing) {
                        if(flexing)gain=gain>1?gain/2:1;
                        g->reel_units=clamp(g->reel_units+gain,10000-(int)(g->line_capacity_units+g->line_capacity_units/10),10000);
                        if(g->tension>=g->line_strength*fish_rules.stamina_min_tension_percent/100)
                            spend_stamina(g,fish_rules.stamina_rest_cost);
                    }
                    if(g->reel_units>=10000) break;
                }
            } else {
                int available=(int)g->line_capacity_units-fish_game_distance(g);
                int paid=clamp(-steps*150,0,available>0?available:0);
                g->tension=clamp(g->tension-paid*fish_rules.ease_tension_per_detent/150,0,g->line_strength+20);
                g->reel_units-=paid;
            }
            g->reel=clamp(g->reel_units/100,0,100);
            if (g->reel_units == 10000) {
                if(g->catches<INT_MAX) ++g->catches;
                g->last_species=g->pending_variant?g->pending_variant:g->pending_species; g->last_size_mm=g->pending_size_mm;
                g->last_weight_g=g->pending_weight_g;
                if(g->species_catches[g->last_species]<UINT16_MAX) ++g->species_catches[g->last_species];
                if (g->last_size_mm>g->species_best_size_mm[g->last_species])
                    g->species_best_size_mm[g->last_species]=g->last_size_mm;
                g->cargo_result=fish_hold_add(&g->economy,g->last_species,g->last_size_mm,g->last_weight_g);
                phase(g, FISH_CAUGHT);
            }
        }
    } else if (event == FISH_PRESS && !g->pressed) {
        g->shore_speed=0;g->shore_remainder=0;
        g->boat_speed=0;g->boat_motion_q8=0;
        g->pressed = true; g->pressed_at = now; g->previous_mode = g->mode;
        if (g->mode==FISH_BITE) {
            if (!fish_game_strike_ready(g)) {lost(g,FISH_LOSS_EARLY);return;}
            int dx,dy; fish_game_float(g,&dx,&dy);
            g->float_dx_q8=dx*256; g->float_dy_q8=dy*256;
            g->motion_random_state=g->random_state^g->pending_weight_g^now^0xa51b624du;
            if (!g->motion_random_state) g->motion_random_state=1;
            g->motion_remaining_ms=0; g->motion_target_q8=0;
            g->base_pull=fish_species_pull(&fish_species[g->pending_species],g->pending_weight_g);
            uint32_t disposition=g->random_state^g->pending_weight_g^now^0x891c62du;
            disposition^=disposition<<13;disposition^=disposition>>17;disposition^=disposition<<5;
            g->struggle_percent=fish_species[g->pending_species].fight_style==FISH_FIGHT_ACTIVE?
                (uint8_t)(85+disposition%36):100;
            g->line_response_percent=(uint8_t)fish_line_response_percent(g->line_index,g->pending_weight_g);
            g->reel=0; g->reel_units=0; g->tension=25;
            g->stamina_max=fish_species_stamina(&fish_species[g->pending_species],g->pending_weight_g);
            g->stamina=g->stamina_max;g->stamina_recovering=false;g->recovery_ms=0;
            if(g->cast_distance>0) {
                /* Metric fighting range: 4.5/6.5/8.5 m, independent of cast span. */
                g->line_capacity_units=(uint32_t)fish_line_sets[g->line_index].distance_limit_cm*10000/initial_distance_cm(g);
            }
            g->overload_ms=0; g->slack_ms=0; g->range_ms=0;g->run_burst=false;g->run_direction=0;
            phase(g,FISH_FIGHT); g->simulation_at=now;
        }
    } else if (event == FISH_RELEASE && g->pressed) {
        g->pressed = false;
        if(g->mode==FISH_SHOP && g->previous_mode==FISH_SHOP) {
            if(g->shop_index==FISH_SHOP_BACK) phase(g,FISH_SHORE);
            else if(g->shop_index==FISH_SHOP_CARGO) {g->hold_return_mode=FISH_SHOP;g->guide_index=0;phase(g,FISH_HOLD);}
            else {g->merchant_line_variant^=1;g->merchant_reply=fish_shop_trade(g,g->shop_index);}
        } else if(g->mode==FISH_HOLD && g->previous_mode==FISH_HOLD) {
            phase(g,g->hold_return_mode==FISH_SHOP && !fish_game_shop_open(g)?FISH_SHORE:(fish_mode_t)g->hold_return_mode);
        } else if (g->mode==FISH_GUIDE && g->previous_mode==FISH_GUIDE) {
            g->guide_page^=1;
        } else if (g->mode==FISH_BAG && g->previous_mode==FISH_BAG) {
            if (g->bag_editing) {g->bag_editing=false;if(g->bag_index==FISH_BAG_SOUND)g->save_requested=true;}
            else if (g->bag_index<=FISH_BAG_DEPTH || g->bag_index==FISH_BAG_SOUND) g->bag_editing=true;
            else if (g->bag_index==FISH_BAG_MISC) {g->hold_return_mode=FISH_BAG;g->guide_index=0;phase(g,FISH_HOLD);}
            else if (g->bag_index==FISH_BAG_SAVE) g->save_requested=true;
            else if (g->bag_index==FISH_BAG_BACK) phase(g,(fish_mode_t)g->bag_return_mode);
            else if (g->bag_index==FISH_BAG_GUIDE) {
                g->guide_return_mode=FISH_BAG; g->guide_index=g->last_species;g->guide_page=0;phase(g,FISH_GUIDE);
            }
        } else if (g->mode == FISH_CHARGE) {
            if(g->bait_index<FISH_SPECIAL_BAIT_COUNT && !g->economy.bait_stock[g->bait_index]) g->bait_index=FISH_PLASTIC_BAIT;
            g->cast_distance = 28 + fish_game_power(g) * (fish_game_max_cast_px(g)-28) / 100;
            g->cast_start_x = g->boat_x + g->facing * 12;
            g->cast_x = clamp(g->cast_start_x + g->facing * g->cast_distance, FISH_WATER_LEFT+4, FISH_WATER_RIGHT-4);
            g->cast_bottom_cm=(uint16_t)fish_map_depth(g->cast_x);
            uint16_t depth=fish_game_rig_depth(g);
            g->cast_depth_cm=depth>g->cast_bottom_cm?g->cast_bottom_cm:depth;
            g->pending_species=FISH_SPECIES_COUNT;g->pending_variant=0; g->pending_size_mm=0; g->pending_weight_g=0;
            g->bite_wait_ms=0;
            if (fish_map_roll_at(&g->random_state,g->cast_x,g->cast_depth_cm,g->bait_index,g->game_minute,&g->pending_species,&g->pending_size_mm)) {
                unsigned special=fish_species_special_roll(&g->random_state,g->pending_species);
                if(special>=FISH_BASE_SPECIES_COUNT) g->pending_variant=(uint8_t)special;
                g->pending_weight_g=fish_species_weight_g(&fish_species[g->pending_species],g->pending_size_mm);
                unsigned bonus=fish_floats[g->float_index].night_bite_bonus_percent*fish_game_night_opacity(g)/150;
                g->bite_wait_ms=fish_line_bite_wait_bonus(&g->random_state,g->line_index,g->pending_weight_g,bonus);
            }
            if(g->bait_index<FISH_SPECIAL_BAIT_COUNT) --g->economy.bait_stock[g->bait_index];
            g->loss=FISH_LOSS_NONE;
            phase(g, FISH_CAST);
        } else if ((g->mode==FISH_MOVE || g->mode==FISH_AIM || g->mode==FISH_SHORE) && g->previous_mode==g->mode) {
            if(g->mode==FISH_SHORE && fish_game_at_shop(g)) {
                if(fish_game_shop_open(g)) {g->shop_index=0;g->merchant_reply=FISH_MERCHANT_GREETING;g->merchant_line_variant^=1;
                    g->merchant_hint=fish_random_next(&g->random_state)%20==0;phase(g,FISH_SHOP);}
                else {g->merchant_reply=FISH_MERCHANT_CLOSED;g->shop_closed_notice=true;g->shop_closed_at=now;}
            }
            else {g->bag_return_mode=(uint8_t)g->mode; g->bag_editing=false; g->bag_index=0; phase(g,FISH_BAG);}
        }
        else if (g->mode == FISH_WAIT || g->mode == FISH_BITE) phase(g, FISH_AIM);
    }
}
