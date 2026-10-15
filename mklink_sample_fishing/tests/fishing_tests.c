#include "fish_game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void bite(fish_game_t *g, unsigned id, uint16_t size, uint32_t now)
{
    fish_game_init(g,now); g->mode=FISH_BITE; g->phase_at=now;
    g->pending_species=(uint8_t)id; g->pending_size_mm=size;
    g->pending_weight_g=fish_species_weight_g(&fish_species[id],size);
}
static void hook(fish_game_t *g, unsigned id, uint16_t size, uint32_t now)
{
    bite(g,id,size,now);
    g->phase_at-=fish_species[id].strike_delay_ms;
    fish_game_input(g,FISH_PRESS,0,now); fish_game_input(g,FISH_RELEASE,0,now);
    assert(g->mode==FISH_FIGHT);
}
static void hook_matched(fish_game_t *g,unsigned id,uint16_t size,uint32_t now)
{
    bite(g,id,size,now);fish_game_equip_line(g,2);
    g->phase_at-=fish_species[id].strike_delay_ms;
    fish_game_input(g,FISH_PRESS,0,now);fish_game_input(g,FISH_RELEASE,0,now);
}
int main(void)
{
    unsigned patterns=0;
    printf("state_bytes=%zu species_record_bytes=%zu catalog_bytes=%zu\n",
        sizeof(fish_game_t),sizeof(fish_species_t),sizeof(fish_species));
    puts("id,min_g,mean_g,max_g,min_pull,max_pull,landing_ms");
    for (unsigned i=0;i<FISH_SPECIES_COUNT;i++) {
        const fish_species_t *s=&fish_species[i];
        uint32_t previous=0; int previous_pull=0;
        assert(s->burst_ms<s->fight_cycle_ms && s->bite_window_ms-s->strike_delay_ms>=400);
        assert(fish_species_weight_g(s,s->weight_anchor_mm)==s->weight_anchor_g);
        for (unsigned mm=s->min_mm;mm<=s->max_mm;mm++) {
            uint32_t weight=fish_species_weight_g(s,(uint16_t)mm);
            int pull=fish_species_pull(s,weight);
            assert(weight>=previous && pull>=previous_pull && pull>=5 && pull<=90);
            previous=weight; previous_pull=pull;
        }
        fish_game_t g;
        bite(&g,i,s->min_mm,0);
        g.now=600; int dx,dy; fish_game_float(&g,&dx,&dy);
        patterns|=1u<<s->bite_style;
        fish_game_input(&g,FISH_ROTATE,40,600); assert(g.mode==FISH_BITE && g.reel==0);
        fish_game_update(&g,s->bite_window_ms); assert(g.mode==FISH_LOST && g.loss==FISH_LOSS_MISSED);
        for (unsigned which=0;which<3;which++) {
            uint16_t size=which==0?s->min_mm:which==1?(s->mean_x2_mm+1)/2:s->max_mm;
            hook_matched(&g,i,size,0);
            int burst=fish_game_pull(&g,fish_rules.hook_settle_ms),rest=fish_game_pull(&g,fish_rules.hook_settle_ms+s->burst_ms);
            assert(burst>=rest);
            for (uint32_t t=100;t<=45000 && g.mode==FISH_FIGHT;t+=100) {
                fish_game_update(&g,t);
                if (g.mode==FISH_FIGHT) fish_game_input(&g,FISH_ROTATE,g.tension>=g.line_strength*75/100?-2:1,t);
            }
            assert(g.mode==FISH_CAUGHT && g.catches==1 && g.last_species==i);
            assert(g.last_weight_g==g.pending_weight_g && g.species_catches[i]==1);
            if (which==2) printf("%s,%lu,%lu,%lu,%u,%u,%lu\n",s->id,
                (unsigned long)fish_species_weight_g(s,s->min_mm),
                (unsigned long)fish_species_weight_g(s,(s->mean_x2_mm+1)/2),
                (unsigned long)fish_species_weight_g(s,s->max_mm),
                fish_species_pull(s,fish_species_weight_g(s,s->min_mm)),
                fish_species_pull(s,fish_species_weight_g(s,s->max_mm)),(unsigned long)g.phase_at);
        }
    }
    assert(patterns==31);
    /* Actual bite timing, including line-window modifiers and exact boundaries. */
    for (unsigned i=0;i<FISH_SPECIES_COUNT;i++) for(unsigned line=0;line<FISH_LINE_COUNT;line++) for(unsigned fl=0;fl<FISH_FLOAT_COUNT;fl++) {
        const fish_species_t *s=&fish_species[i];fish_game_t test;
        unsigned base=(s->bite_window_ms-s->strike_delay_ms)*fish_line_sets[line].strike_window_percent/100;
        unsigned end=s->strike_delay_ms+base*(fl==1?115:100)/100;
        bite(&test,i,s->min_mm,0);fish_game_equip_line(&test,line);test.float_index=fl;
        fish_game_input(&test,FISH_PRESS,0,s->strike_delay_ms-1);
        assert(test.loss==FISH_LOSS_EARLY && test.mode==FISH_LOST);
        fish_game_input(&test,FISH_RELEASE,0,s->strike_delay_ms);assert(test.mode==FISH_LOST);
        bite(&test,i,s->min_mm,0);fish_game_equip_line(&test,line);test.float_index=fl;
        fish_game_input(&test,FISH_PRESS,0,s->strike_delay_ms);assert(test.mode==FISH_FIGHT);
        bite(&test,i,s->min_mm,0);fish_game_equip_line(&test,line);test.float_index=fl;
        fish_game_input(&test,FISH_PRESS,0,end-1);assert(test.mode==FISH_FIGHT);
        bite(&test,i,s->min_mm,0);fish_game_equip_line(&test,line);test.float_index=fl;
        fish_game_input(&test,FISH_PRESS,0,end);assert(test.loss==FISH_LOSS_MISSED);
        fish_game_input(&test,FISH_RELEASE,0,end+1);assert(test.mode==FISH_LOST);
    }
    fish_game_t g,a,b;
    /* Energy stays steady in a lull; exhaustion pauses bursts and recovers. */
    hook(&g,0,80,0);unsigned fresh=g.stamina;
    fish_game_update(&g,600);assert(g.stamina==fresh);
    g.stamina=1;fish_game_update(&g,700);
    assert(g.stamina==0 && g.stamina_recovering && !fish_game_burst_active(&g) && !fish_game_burst_warning(&g));
    assert(fish_game_splash(&g)==0);
    int resting_distance=fish_game_distance(&g);
    for(unsigned t=800;t<=2200;t+=100) fish_game_update(&g,t);
    assert(g.stamina==0 && g.mode==FISH_FIGHT && fish_game_distance(&g)==resting_distance);
    fish_game_update(&g,2500);assert(g.stamina>0 && g.stamina_recovering);
    while(g.stamina_recovering && g.now<10000) fish_game_update(&g,g.now+100);
    assert(!g.stamina_recovering && g.stamina>0 && g.mode==FISH_FIGHT);
    /* A fish can be landed with energy remaining as soon as distance is zero. */
    hook(&g,1,666,0);g.reel_units=9999;
    fish_game_input(&g,FISH_ROTATE,1,0);
    assert(g.mode==FISH_CAUGHT && g.stamina>0 && fish_game_distance_cm(&g)==0);
    /* Metric distance follows actual cast span; line length is independent of it. */
    bite(&g,0,200,0);g.cast_x=210;g.cast_start_x=162;g.cast_distance=48;
    fish_game_equip_line(&g,0);g.phase_at-=fish_species[0].strike_delay_ms;
    fish_game_input(&g,FISH_PRESS,0,0);fish_game_input(&g,FISH_RELEASE,0,0);
    assert(fish_game_distance_cm(&g)==192);
    g.reel_units=5000;assert(fish_game_distance_cm(&g)==96);
    fish_game_input(&g,FISH_ROTATE,-1,0);assert(fish_game_distance_cm(&g)>96);
    g.reel_units=10000-(int)g.line_capacity_units;assert(fish_game_distance_cm(&g)==600);
    hook(&g,1,666,0);
    assert(g.stamina_max>400 && g.stamina==g.stamina_max);
    fish_game_input(&g,FISH_ROTATE,40,0);assert(g.mode==FISH_FIGHT && g.reel_units<=4000);
    /* Time alone with a slack line cannot exhaust a fish. */
    hook(&g,1,666,0);unsigned energy=g.stamina;g.tension=0;
    fish_game_update(&g,100);assert(g.stamina==energy);
    hook(&g,1,1000,0);
    fish_game_input(&g,FISH_ROTATE,40,0);
    fish_game_update(&g,400); assert(g.mode==FISH_FIGHT && g.overload_ms==400);
    fish_game_input(&g,FISH_ROTATE,-5,400);
    fish_game_update(&g,500); assert(g.overload_ms>0 && g.overload_ms<400);
    fish_game_update(&g,900); assert(g.overload_ms==0 && g.mode==FISH_FIGHT);
    fish_game_input(&g,FISH_ROTATE,40,900);
    fish_game_update(&g,1900); assert(g.mode==FISH_LOST && g.loss==FISH_LOSS_BREAK && g.catches==0);
    int retained_dx,retained_dy;fish_game_float(&g,&retained_dx,&retained_dy);
    uint32_t failure_at=g.phase_at;
    fish_game_input(&g,FISH_PRESS,0,failure_at+50);
    fish_game_input(&g,FISH_ROTATE,40,failure_at+100);
    fish_game_update(&g,failure_at+FISH_LOSS_ANIMATION_MS-1);
    assert(g.mode==FISH_LOST && !g.pressed && !g.boat_speed);
    int retained_dx2,retained_dy2;fish_game_float(&g,&retained_dx2,&retained_dy2);
    assert(retained_dx==retained_dx2 && retained_dy==retained_dy2);
    fish_game_input(&g,FISH_RELEASE,0,failure_at+FISH_LOSS_ANIMATION_MS);
    assert(g.mode==FISH_AIM && !g.pressed);
    hook(&g,0,80,0);
    for (uint32_t t=0;t<=3000 && g.mode==FISH_FIGHT;t+=20) {
        fish_game_update(&g,t);
        if (g.mode==FISH_FIGHT && g.tension>1) fish_game_input(&g,FISH_ROTATE,-1,t);
    }
    assert(g.loss==FISH_LOSS_SLACK && g.catches==0);
    /* Range failure with healthy tension, independent of overload and slack. */
    hook(&g,0,200,0); g.reel_units=10000-g.line_capacity_units;
    fish_game_update(&g,600); assert(g.mode==FISH_FIGHT && g.range_ms==600);
    fish_game_input(&g,FISH_ROTATE,1,600);
    fish_game_update(&g,800); assert(g.range_ms<600 && g.range_ms>0);
    fish_game_update(&g,1000); assert(g.range_ms==0);
    g.reel_units=10000-g.line_capacity_units;
    fish_game_update(&g,2000); assert(g.loss==FISH_LOSS_RANGE && g.catches==0);
    hook(&g,0,80,0);
    for (uint32_t t=100;t<=60000;t+=100) fish_game_update(&g,t);
    assert(g.mode==FISH_FIGHT && g.loss==FISH_LOSS_NONE); /* No hard fight timeout. */
    hook(&g,1,650,0); g.now=fish_rules.hook_settle_ms+fish_species[1].fight_cycle_ms-100;
    assert(fish_game_burst_warning(&g));
    g.now=fish_rules.hook_settle_ms+fish_species[1].fight_cycle_ms; assert(!fish_game_burst_warning(&g));
    hook(&g,1,1000,0);
    fish_game_input(&g,FISH_ROTATE,-1,0);
    assert(g.reel==0 && fish_game_distance(&g)>10000); /* Giving line increases distance. */
    g.reel_units=0; g.tension=60;
    fish_game_input(&g,FISH_ROTATE,1,0);
    assert(g.reel>0 && fish_game_distance(&g)<10000);
    bite(&g,1,1000,0);fish_game_equip_line(&g,2);
    g.phase_at-=fish_species[1].strike_delay_ms;
    fish_game_input(&g,FISH_PRESS,0,0);fish_game_input(&g,FISH_RELEASE,0,0);g.tension=120;
    fish_game_update(&g,1000); assert(g.mode==FISH_FIGHT && g.overload_ms==0);
    hook(&g,0,80,0); fish_game_update(&g,60000);
    assert(g.mode==FISH_FIGHT && g.now-g.simulation_at<20 && g.phase_at==59000); /* Background gap is a bounded pause. */
    /* Timestamped inputs must produce the same result at 60 Hz-ish and low FPS. */
    hook(&a,1,650,0); b=a;
    for (uint32_t t=100;t<=5000;t+=100) {
        for (uint32_t frame=t-90;frame<t;frame+=10) fish_game_update(&a,frame);
        int steps=t%300?1:-1;
        fish_game_input(&a,FISH_ROTATE,steps,t); fish_game_input(&b,FISH_ROTATE,steps,t);
    }
    assert(memcmp(&a,&b,sizeof(a))==0);
    hook(&a,0,200,0); b=a;
    fish_game_input(&a,FISH_ROTATE,5,0);
    for (int i=0;i<5;i++) fish_game_input(&b,FISH_ROTATE,1,0);
    assert(a.reel==b.reel && a.tension==b.tension);
    hook(&g,0,200,UINT32_MAX-99);
    fish_game_update(&g,100); assert(g.mode==FISH_FIGHT && g.simulation_at==100);
    bite(&g,0,200,UINT32_MAX-49);g.phase_at-=fish_species[0].bite_window_ms;
    fish_game_update(&g,UINT32_MAX-49);assert(g.mode==FISH_LOST);
    fish_game_update(&g,1150);assert(g.mode==FISH_AIM);
    /* Motion must remain continuous through burst edges and actual reel/ease
     * detents, deterministic at different render rates, and not repeat a cycle. */
    for (unsigned i=0;i<FISH_SPECIES_COUNT;i++) {
        const fish_species_t *s=&fish_species[i];
        hook_matched(&a,i,s->max_mm,0); a.cast_x=270; a.cast_start_x=162; b=a;
        uint32_t catalog_rng=a.random_state;
        int px,py; fish_game_float(&a,&px,&py);
        int first_cycle[250]={0}; unsigned different=0;
        for (uint32_t t=20;t<=s->fight_cycle_ms*3u;t+=20) {
            fish_game_update(&a,t-10); fish_game_update(&a,t);
            fish_game_update(&b,t);
            if (t==200 || t==400) {
                int steps=t==200?1:-1;
                fish_game_input(&a,FISH_ROTATE,steps,t); fish_game_input(&b,FISH_ROTATE,steps,t);
            }
            if (t%100==0 && a.tension>=a.line_strength*70/100) {
                fish_game_input(&a,FISH_ROTATE,-1,t); fish_game_input(&b,FISH_ROTATE,-1,t);
            }
            assert(a.mode==FISH_FIGHT);
            int dx,dy; fish_game_float(&a,&dx,&dy);
            assert(abs(dx-px)<=4 && abs(dy-py)<=2);
            px=dx; py=dy;
            unsigned slot=(t%s->fight_cycle_ms)/20;
            if (t<s->fight_cycle_ms) first_cycle[slot]=dx;
            else if (t<s->fight_cycle_ms*2u && dx!=first_cycle[slot]) ++different;
            assert(fish_game_splash(&a)>=0 && fish_game_splash(&a)<=6);
            assert(memcmp(&a,&b,sizeof(a))==0 && a.random_state==catalog_rng);
        }
        assert(different>(s->fight_style==FISH_FIGHT_ACTIVE?5:0));
    }
    puts("PASS: 33 landings, weight/pull, burst cues, strike window, danger recovery, break/slack/range, no timeout, batching, FPS and wrap");
    puts("PASS: 11 seeded aperiodic fight paths, bounded displacement, reel/ease smoothing, independent RNG and frame-rate determinism");
}
