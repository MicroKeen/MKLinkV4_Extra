#include "fish_game.h"
#include <assert.h>
#include <stdio.h>
static unsigned size_for_weight(const fish_species_t *s,unsigned grams)
{
    unsigned a=s->min_mm,b=s->max_mm;
    while(a<b){unsigned m=(a+b)/2;if(fish_species_weight_g(s,m)<grams)a=m+1;else b=m;}
    return a;
}
static void hook(fish_game_t *g,unsigned id,unsigned grams,unsigned line,unsigned seed)
{
    fish_game_init(g,0);g->map_index=1;g->rod_index=0;fish_game_equip_line(g,line);
    const fish_species_t *s=fish_species_profile(1,id);
    g->mode=FISH_BITE;g->pending_species=id;g->pending_size_mm=size_for_weight(s,grams);
    g->pending_weight_g=fish_species_weight_g(s,g->pending_size_mm);g->random_state=seed;
    g->phase_at=0-s->strike_delay_ms;g->cast_start_x=492;g->cast_distance=fish_game_max_cast_px(g);g->cast_x=492+g->cast_distance;
    fish_game_input(g,FISH_PRESS,0,0);fish_game_input(g,FISH_RELEASE,0,0);assert(g->mode==FISH_FIGHT);
}
static void trial(unsigned id,unsigned grams,unsigned line,unsigned policy)
{
    fish_game_t g;hook(&g,id,grams,line,42);
    unsigned peak=0,pays=0,bursts=0;int farthest=0;bool prior=false;
    for(unsigned t=100;t<=120000 && g.mode==FISH_FIGHT;t+=100){
        fish_game_update(&g,t);bool burst=fish_game_burst_active(&g);bursts+=burst&&!prior;prior=burst;
        int steps=policy==3?(t%500==0?1:0):policy==0?1:policy==1?3:g.tension>=g.line_strength*65/100?-1:
            (burst||fish_game_burst_warning(&g)) && g.tension>=g.line_strength/2?0:1;
        pays+=steps<0;fish_game_input(&g,FISH_ROTATE,steps,t);
        if(g.tension>0 && (unsigned)g.tension>peak)peak=g.tension;
        if(fish_game_distance(&g)>farthest)farthest=fish_game_distance(&g);
    }
    printf("%u,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",id,(unsigned long)g.pending_weight_g,line,policy,g.mode==FISH_CAUGHT,g.now,peak,g.line_strength,pays,bursts,g.stamina,farthest);
    if(id==22 && line==2 && policy==2) {
        assert(g.mode==FISH_CAUGHT && g.now<40000);
        assert(peak>=g.line_strength*60/100);
    }
}
int main(void)
{
    puts("species,weight_g,line,policy,landed,duration_ms,peak_tension,line_strength,payouts,bursts,stamina_remaining,farthest_units");
    const unsigned ids[]={1,13,16,17,18,20,22,24};
    for(unsigned i=0;i<8;i++)for(unsigned line=0;line<3;line++)for(unsigned policy=0;policy<3;policy++)trial(ids[i],13000,line,policy);
    for(unsigned id=15;id<FISH_SPECIES_COUNT;id++)if(id!=21)trial(id,fish_species_weight_g(fish_species_profile(1,id),fish_species_profile(1,id)->max_mm),2,2);
    trial(22,13000,2,3);
    fish_game_t fresh;hook(&fresh,22,13000,2,42);fresh.reel_units=9999;
    fish_game_input(&fresh,FISH_ROTATE,1,0);assert(fresh.mode==FISH_CAUGHT && fresh.reel_units==10000);
    /* Fresh giants can land when safely alongside, without a stamina gate. */
    for(unsigned id=15;id<FISH_SPECIES_COUNT;id++)if(id!=21) {
        hook(&fresh,id,fish_species_weight_g(fish_species_profile(1,id),fish_species_profile(1,id)->max_mm),2,42);
        fresh.reel_units=9500;fresh.tension=20;
        fish_game_input(&fresh,FISH_ROTATE,40,0);assert(fresh.mode==FISH_CAUGHT && fresh.reel_units==10000);
    }
    /* No input: slow heavy fish must make measurable runs and pressure. */
    hook(&fresh,22,13000,2,42);unsigned peak=0,runs=0;
    for(unsigned t=20;t<=4200;t+=20){int distance=fish_game_distance(&fresh);fish_game_update(&fresh,t);
        if(fish_game_distance(&fresh)>distance)++runs;if(fresh.tension>(int)peak)peak=fresh.tension;}
    assert(fresh.mode==FISH_FIGHT && peak>=70 && runs>=20);
    /* Bottom rollers stay submerged; the puffer remains passive. */
    assert(fish_species_profile(1,22)->fight_style==FISH_FIGHT_SLOW);
    assert(fish_species_profile(1,23)->fight_style==FISH_FIGHT_PASSIVE);
    for(unsigned id=17;id<=20;id+=3) {
        unsigned away=0,toward=0;uint64_t sum_cm=0;
        const fish_species_t *s=fish_species_profile(1,id);
        for(unsigned seed=1;seed<=128;seed++) {
            hook(&fresh,id,30000,FISH_PE_LINE,seed);int before=0,after=0;
            unsigned begin=fish_rules.hook_settle_ms;
            for(unsigned t=20;t<=begin+s->burst_ms;t+=20) {
                if(t<=begin)before=(int)fish_game_distance_cm(&fresh);
                fish_game_update(&fresh,t);assert(fresh.mode==FISH_FIGHT);
                after=(int)fish_game_distance_cm(&fresh);
            }
            int moved=after-before;away+=moved>0;toward+=moved<0;
            unsigned magnitude=(unsigned)(moved<0?-moved:moved);sum_cm+=magnitude;
            assert(magnitude>=40);
        }
        assert(away>toward && toward>0);
        printf("surge species=%u mean_cm=%llu outward=%u inward=%u,128 seeds\n",id,(unsigned long long)(sum_cm/128),away,toward);
    }
}
