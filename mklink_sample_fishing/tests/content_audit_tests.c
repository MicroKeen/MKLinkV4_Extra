#include "fish_game.h"
#include "fish_merchant_dialogue_data.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {int boat,facing,hold;unsigned depth,bait;} profile_t;
static void cast(fish_game_t *g,profile_t p,uint32_t seed)
{
    fish_game_init(g,0);g->boat_x=p.boat;g->facing=p.facing;g->depth_index=p.depth;g->bait_index=p.bait;
    if(p.bait<FISH_SPECIAL_BAIT_COUNT)g->economy.bait_stock[p.bait]=1;
    g->random_state=seed;g->game_minute=720;
    fish_game_input(g,FISH_PRESS,0,0);fish_game_input(g,FISH_RELEASE,0,p.hold);
    assert(g->mode==FISH_CAST);
}
static void landed(fish_game_t *g,unsigned id)
{
    g->mode=FISH_BITE;g->phase_at=g->now-fish_species[g->pending_species].strike_delay_ms;
    fish_game_input(g,FISH_PRESS,0,g->now);fish_game_input(g,FISH_RELEASE,0,g->now);
    assert(g->mode==FISH_FIGHT);g->reel_units=9999;
    fish_game_input(g,FISH_ROTATE,1,g->now);
    assert(g->mode==FISH_CAUGHT && g->last_species==id && g->species_catches[id]==1);
    assert(g->species_best_size_mm[id]==g->last_size_mm && fish_catch_price(id,g->last_size_mm)>0);
}
int main(void)
{
    setvbuf(stdout,NULL,_IONBF,0);
    unsigned seen[FISH_SPECIES_COUNT]={0};
    for(unsigned target=0;target<FISH_BASE_SPECIES_COUNT;target++) {
        profile_t best={0};unsigned best_weight=0,best_total=1;
        for(int boat=132;boat<=828;boat+=16)for(int facing=-1;facing<=1;facing+=2)
        for(unsigned hold=0;hold<3;hold++)for(unsigned depth=0;depth<FISH_DEPTH_OPTION_COUNT;depth++)
        for(unsigned bait=0;bait<FISH_BAIT_COUNT;bait++) {
            static const int holds[]={280,600,1500};profile_t p={boat,facing,holds[hold],depth,bait};fish_game_t g;cast(&g,p,1);
            unsigned total=0,w=fish_map_weight_at(target,g.cast_x,g.cast_depth_cm,bait,g.game_minute);
            for(unsigned i=0;i<FISH_BASE_SPECIES_COUNT;i++)total+=fish_map_weight_at(i,g.cast_x,g.cast_depth_cm,bait,g.game_minute);
            if((uint64_t)w*best_total>(uint64_t)best_weight*total) {best=p;best_weight=w;best_total=total;}
        }
        assert(best_weight>0);unsigned count=0,rare=0;
        for(unsigned trial=1;trial<=200000;trial++) {
            fish_game_t g;cast(&g,best,trial*0x9e3779b9u);
            if(g.pending_species!=target)continue;
            unsigned id=g.pending_variant?g.pending_variant:g.pending_species;
            count++;rare+=id>=FISH_BASE_SPECIES_COUNT;
            if(!seen[id])landed(&g,id);
            seen[id]++;
        }
        assert(count>100);
        printf("species=%s casts=200000 selected=%u rare=%u boat=%d facing=%d hold_ms=%d depth=%u bait=%u\n",
            fish_species[target].id,count,rare,best.boat,best.facing,best.hold,best.depth,best.bait);
    }
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++)assert(seen[i]>0);
    /* Both lines in every semantic context must select its configured expression. */
    static const fish_merchant_reply_t replies[]={FISH_MERCHANT_SOLD,FISH_MERCHANT_BOUGHT,FISH_MERCHANT_POOR,
        FISH_MERCHANT_EMPTY,FISH_MERCHANT_OWNED,FISH_MERCHANT_FULL};
    unsigned poses=0;
    for(unsigned context=0;context<14;context++) {
        fish_game_t g;fish_game_init(&g,0);g.merchant_reply=FISH_MERCHANT_GREETING;
        if(context==1)fish_hold_add(&g.economy,0,150,100);
        else if(context==2)fish_hold_add(&g.economy,1,1000,20000);
        else if(context>=3 && context<=8)g.merchant_reply=replies[context-3];
        else if(context>=9 && context<=12)fish_hold_add(&g.economy,11+context-9,300,100);
        else if(context==13)g.merchant_hint=true;
        for(unsigned line=0;line<2;line++) {
            g.merchant_line_variant=line;
            assert(fish_merchant_pose(&g)==fish_merchant_dialogue[context].pose);
            assert(!strcmp(fish_merchant_says(&g),fish_merchant_dialogue[context].lines[line]));
            poses|=1u<<fish_merchant_pose(&g);
        }
    }
    assert(poses==15);puts("PASS: all 15 fish reachable and landed; 14 merchant contexts, 28 lines and all four expressions.");
}
