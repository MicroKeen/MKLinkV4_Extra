#include "fish_hpm_feedback.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>

static unsigned initialized,events,tones;
static uint32_t last_hz;
static uint8_t last_duty;
static uint16_t last_time;
void Beep_PWM_Init(void) {initialized++;}
void Beep_PWM_frequency(uint32_t hz,uint8_t duty,uint16_t time)
{
    events++;last_hz=hz;last_duty=duty;last_time=time;
    if(hz) {
        assert((hz<=200 || hz==660 || hz==1100) && duty==20 && time>0 && time<=8);
        tones++;
    }
}
static void update(fish_game_t *g,uint32_t now)
{
    g->now=now;fish_hpm_feedback_update(g);
}
static void hook(fish_game_t *g,uint32_t now,unsigned species)
{
    fish_game_init(g,now);g->mode=FISH_BITE;g->pending_species=(uint8_t)species;
    g->pending_size_mm=fish_species[species].weight_anchor_mm;
    g->pending_weight_g=fish_species_weight_g(&fish_species[species],g->pending_size_mm);
    g->phase_at=now-fish_species[species].strike_delay_ms;
    fish_game_input(g,FISH_PRESS,0,now);
    assert(g->mode==FISH_FIGHT);
    fish_hpm_feedback_update(g);
    assert(last_hz==160 && last_time==6);
}
static void trade(fish_game_t *g,unsigned item,uint32_t now)
{
    g->shop_index=(uint8_t)item;
    fish_game_input(g,FISH_PRESS,0,now);fish_hpm_feedback_update(g);
    fish_game_input(g,FISH_RELEASE,0,now+1);fish_hpm_feedback_update(g);
}
int main(void)
{
    fish_game_t g;fish_game_init(&g,1000);fish_hpm_feedback_init();
    assert(initialized==1 && events==1 && !last_hz && !last_duty);
    /* Waiting/probing and all menus stay silent. No species hint before hooking. */
    for(unsigned mode=0;mode<=FISH_HOLD;mode++)if(mode!=FISH_FIGHT) {
        g.mode=(fish_mode_t)mode;update(&g,1000+mode);
    }
    assert(tones==0 && events==1);
    hook(&g,2000,1);unsigned count=events;
    for(unsigned i=0;i<30;i++)update(&g,2000+i);
    assert(events==count); /* Frame/input callbacks cannot replay the hook cue. */
    uint32_t burst_at=g.phase_at+fish_rules.hook_settle_ms;
    update(&g,burst_at-1);assert(events==count);
    update(&g,burst_at);assert(events==++count && last_hz==140 && last_time==3);
    update(&g,burst_at+239);assert(events==count);
    update(&g,burst_at+240);assert(events==++count && last_hz==140);
    update(&g,burst_at+241);assert(events==count);
    update(&g,burst_at+fish_species[1].burst_ms);assert(events==++count && !last_hz);
    /* Exhausted fish and normal lulls must not keep pulsing. */
    g.stamina_recovering=true;update(&g,burst_at+fish_species[1].fight_cycle_ms);
    assert(events==count);
    g.stamina_recovering=false;update(&g,burst_at+fish_species[1].fight_cycle_ms+1);
    assert(events==++count && last_hz==140);
    g.mode=FISH_CAUGHT;update(&g,g.now+1);assert(events==++count && !last_hz);
    update(&g,g.now+1);assert(events==count);
    /* Real early strikes produce no hook tone. */
    fish_game_init(&g,5000);g.mode=FISH_BITE;g.pending_species=1;g.phase_at=g.now;
    unsigned before=tones;fish_game_input(&g,FISH_PRESS,0,g.now);
    assert(g.mode==FISH_LOST);fish_hpm_feedback_update(&g);assert(tones==before);
    /* Passive species get one hook cue but no invented struggle. */
    hook(&g,6000,10);before=tones;
    for(unsigned ms=1;ms<6000;ms+=20)update(&g,6000+ms);
    assert(tones==before);g.mode=FISH_AIM;update(&g,g.now+1);assert(!last_hz);
    /* Every exit stops immediately, including failure, menus and restart. */
    for(unsigned mode=0;mode<=FISH_HOLD;mode++)if(mode!=FISH_FIGHT) {
        hook(&g,10000+mode*10,1);g.mode=(fish_mode_t)mode;
        update(&g,g.now+1);assert(!last_hz);
    }
    /* Unsigned elapsed time handles the millisecond counter wrap. */
    hook(&g,UINT_MAX-1000u,1);burst_at=g.phase_at+fish_rules.hook_settle_ms;
    update(&g,burst_at);count=events;update(&g,burst_at+239);assert(events==count);
    update(&g,burst_at+240);assert(events==count+1 && last_hz==140);
    fish_hpm_feedback_init();assert(initialized==2 && !last_hz);
    /* Real purchases/sales trigger an ascending two-note cashier cue. */
    fish_game_init(&g,1000);g.mode=FISH_SHOP;g.game_minute=720;g.economy.coins=1000;
    before=tones;update(&g,1000);assert(tones==before);
    trade(&g,1,1000);assert(g.merchant_reply==FISH_MERCHANT_BOUGHT && tones==before+1 && last_hz==660 && last_time==4);
    count=events;update(&g,1050);assert(events==count);
    update(&g,1051);assert(events==count+1 && last_hz==1100 && last_time==8);
    update(&g,1131);count=events;update(&g,1132);assert(events==count);
    before=tones;trade(&g,1,1200);assert(g.merchant_reply==FISH_MERCHANT_OWNED && tones==before);
    trade(&g,2,1210);assert(g.merchant_reply==FISH_MERCHANT_POOR && tones==before);
    trade(&g,0,1220);assert(g.merchant_reply==FISH_MERCHANT_EMPTY && tones==before);
    g.economy.bait_stock[0]=999;trade(&g,7,1230);
    assert(g.merchant_reply==FISH_MERCHANT_FULL && tones==before);
    fish_hold_add(&g.economy,0,200,160);trade(&g,0,1240);
    assert(g.merchant_reply==FISH_MERCHANT_SOLD && tones==before+1 && last_hz==660);
    update(&g,1291);assert(last_hz==1100);
    g.mode=FISH_SHORE;update(&g,1292);assert(!last_hz);
    /* Repeated successful purchases restart; a persistent reply cannot replay. */
    g.mode=FISH_SHOP;g.economy.bait_stock[0]=0;update(&g,1300);
    trade(&g,7,1310);count=events;update(&g,1312);assert(events==count);
    trade(&g,7,1320);assert(events==count+1 && last_hz==660);
    update(&g,1371);assert(last_hz==1100);update(&g,1451);
    /* A full coin purse rejects a sale without any cashier cue. */
    g.economy.coins=FISH_COIN_LIMIT;fish_hold_add(&g.economy,0,200,160);
    g.merchant_reply=FISH_MERCHANT_GREETING;update(&g,1500);before=tones;
    trade(&g,0,1510);assert(g.merchant_reply==FISH_MERCHANT_FULL && tones==before);
    g.mode=FISH_SHORE;update(&g,1600);fish_game_init(&g,UINT_MAX-10u);
    g.mode=FISH_SHOP;g.game_minute=720;g.economy.coins=1000;
    update(&g,UINT_MAX-10u);trade(&g,7,UINT_MAX-10u);
    assert(g.merchant_reply==FISH_MERCHANT_BOUGHT && last_hz==660);
    update(&g,40);assert(last_hz==1100);g.mode=FISH_AIM;update(&g,41);assert(!last_hz);
    puts("PASS: hook/burst pulses, silent failures, rising cashier notes, repeat/exit and clock wrap");
    return 0;
}
