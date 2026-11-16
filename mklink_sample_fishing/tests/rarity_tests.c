#include "fish_game.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    const uint8_t expected[]={0,0,1,0,1,1,1,0,2,2,1};
    unsigned weights[3]={0},total=0,counts[FISH_SPECIES_COUNT]={0},tiers[3]={0};
    for (unsigned i=0;i<FISH_BASE_SPECIES_COUNT;i++) {
        assert(fish_species[i].rarity==expected[i]);
        weights[expected[i]]+=fish_species[i].encounter_weight;
        total+=fish_species[i].encounter_weight;
    }
    assert(total==199 && weights[0]==140 && weights[1]==50 && weights[2]==9);
    uint32_t random=0x52ad21u;
    const unsigned samples=1000000;
    for (unsigned n=0;n<samples;n++) {
        uint8_t index; uint16_t size;
        fish_species_roll(&random,&index,&size);
        ++counts[index]; ++tiers[fish_species[index].rarity];
    }
    puts("id,tier,samples,actual_count,expected_count");
    for (unsigned i=0;i<FISH_SPECIES_COUNT;i++) {
        unsigned expected_count=samples*fish_species[i].encounter_weight/total;
        assert(abs((int)counts[i]-(int)expected_count)<3000);
        printf("%s,%u,%u,%u,%u\n",fish_species[i].id,fish_species[i].rarity,samples,counts[i],expected_count);
    }
    for (unsigned i=0;i<3;i++) {
        assert(abs((int)tiers[i]-(int)(samples*weights[i]/total))<3000);
        printf("tier_%u,%u,%u,%u,%u\n",i,i,samples,tiers[i],samples*weights[i]/total);
    }
    /* Target baits must not normalize a gold fish into a common encounter. */
    for(unsigned kind=0;kind<3;kind++) {
        uint16_t pool[FISH_SPECIES_COUNT]={0};pool[9]=kind==2?1:900;
        if(kind==0)pool[0]=100;
        if(kind==2)pool[0]=99;
        unsigned hit=0,miss=0;uint32_t state=0x329887u;
        for(unsigned n=0;n<100000;n++) {
            uint8_t id=255;uint16_t mm=0;
            if(fish_species_roll_weights(&state,pool,&id,&mm)){assert(id==0 || id==9);hit+=id==9;}
            else {assert(kind==1 && id==255 && mm==0);miss++;}
        }
        if(kind<2)assert(hit>4500 && hit<5500);else assert(hit>800 && hit<1200);
        assert(kind==1?miss>94000:!miss);
    }
    /* Empty rare-only attempts retry without resetting the ten-second hint.
     * The same interval works across the wall-clock rollover, with no new RAM. */
    for(unsigned wrap=0;wrap<2;wrap++) {
        fish_game_t wait;uint32_t start=wrap?UINT32_MAX-5000:0;
        fish_game_init(&wait,start);wait.mode=FISH_WAIT;wait.phase_at=start;wait.map_index=2;
        wait.pending_species=FISH_SPECIES_COUNT;wait.bait_index=0;wait.line_index=FISH_MARINE_LINE;
        wait.cast_x=1000;wait.cast_depth_cm=fish_map_depth_on(2,1000)/2;
        wait.game_minute=720;bool delayed=false;
        for(unsigned elapsed=100;elapsed<600000 && wait.mode==FISH_WAIT;elapsed+=100) {
            fish_game_update(&wait,start+elapsed);
            if(elapsed>10000 && wait.pending_species==FISH_SPECIES_COUNT){assert(wait.phase_at==start);delayed=true;}
        }
        assert(delayed && wait.mode==FISH_BITE && wait.pending_species==27);
    }
    /* No directional pockets for ordinary freshwater fish. */
    for(unsigned i=0;i<FISH_BASE_SPECIES_COUNT;i++)for(unsigned z=0;z<5;z++) {
        assert(fish_distributions[i].zone_weights[z]==fish_distributions[i].zone_weights[4-z]);
        assert(fish_distributions[i].nearshore_only==(i==4 || i==10));
    }
    /* At neutral bait/tackle, bronze dominates every eligible layer even if
     * that layer has many more silver species. Target baits can remove bronze. */
    for(unsigned map=0;map<3;map++)for(unsigned layer=0;layer<3;layer++)for(unsigned minute=360;minute<=1320;minute+=480) {
        int x=map==2?1320:480,bottom=fish_map_depth_on(map,x),hook=layer==0?0:layer==1?bottom/2:bottom;
        uint16_t pool[FISH_SPECIES_COUNT];unsigned sums[3]={0},sum=0;
        fish_map_encounter_weights_on(map,x,hook,FISH_PLASTIC_BAIT,minute,0,FISH_LINE_COUNT,false,pool);
        for(unsigned i=0;i<FISH_SPECIES_COUNT;i++){unsigned tier=fish_species_profile(map,i)->rarity;if(tier<3)sums[tier]+=pool[i];sum+=pool[i];}
        if(sums[0])assert(sums[0]*100/sum>=69);
        if(sums[2])assert(sums[2]*100/sum<=5);
    }
    {
        uint16_t pool[FISH_SPECIES_COUNT];
        fish_map_encounter_weights_on(1,480,800,FISH_BAIT_LIFELIKE,720,0,FISH_LINE_COUNT,false,pool);
        unsigned bronze=0;for(unsigned i=0;i<FISH_SPECIES_COUNT;i++)if(fish_species_profile(1,i)->rarity==FISH_RARITY_BRONZE)bronze+=pool[i];
        assert(!bronze); /* Player's deliberate filtering remains effective. */
    }
    fish_game_t g;
    fish_game_init(&g,0); uint32_t before=g.random_state;
    int x=g.boat_x;
    fish_game_input(&g,FISH_PRESS,0,10);
    fish_game_input(&g,FISH_ROTATE,1,30); assert(g.mode==FISH_GUIDE && g.guide_index==0);
    fish_game_input(&g,FISH_RELEASE,0,40); assert(g.mode==FISH_GUIDE && !g.pressed);
    fish_game_input(&g,FISH_ROTATE,-1,50); assert(g.guide_index==fish_guide_index_at(FISH_GUIDE_COUNT-1));
    fish_game_input(&g,FISH_ROTATE,2,60); assert(g.guide_index==1);
    fish_game_update(&g,5000); assert(g.mode==FISH_GUIDE);
    assert(g.random_state==before && g.boat_x==x && g.catches==0);
    fish_game_input(&g,FISH_PRESS,0,5010);
    fish_game_input(&g,FISH_RELEASE,0,5020); assert(g.mode==FISH_GUIDE && g.guide_page==1);
    fish_game_input(&g,FISH_PRESS,0,5030);fish_game_input(&g,FISH_RELEASE,0,5630);assert(g.mode==FISH_MOVE);
    /* Restore FISH mode when the guide was opened while aiming. */
    fish_game_init(&g,0);g.mode=FISH_AIM;
    fish_game_input(&g,FISH_PRESS,0,5100); fish_game_input(&g,FISH_ROTATE,1,5120);
    fish_game_input(&g,FISH_RELEASE,0,5130); assert(g.mode==FISH_GUIDE);
    fish_game_input(&g,FISH_PRESS,0,5200); fish_game_input(&g,FISH_RELEASE,0,5800);
    assert(g.mode==FISH_AIM);
    /* Existing records remain until a larger catch is landed. */
    g.pending_species=0; g.pending_size_mm=200;
    g.pending_weight_g=fish_species_weight_g(&fish_species[0],200);
    g.species_best_size_mm[0]=250;
    g.mode=FISH_BITE; g.phase_at=6300;
    g.phase_at-=fish_species[0].strike_delay_ms;
    fish_game_input(&g,FISH_PRESS,0,6300); fish_game_input(&g,FISH_RELEASE,0,6300);
    g.reel_units=9900; g.tension=20;g.stamina=0;
    fish_game_input(&g,FISH_ROTATE,1,6300);
    assert(g.mode==FISH_CAUGHT && g.species_best_size_mm[0]==250);
    g.mode=FISH_BITE; g.phase_at=6400; g.pending_size_mm=300;
    g.phase_at-=fish_species[0].strike_delay_ms;
    g.pending_weight_g=fish_species_weight_g(&fish_species[0],300);
    fish_game_input(&g,FISH_PRESS,0,6400); fish_game_input(&g,FISH_RELEASE,0,6400);
    g.reel_units=9900; g.tension=20;g.stamina=0;
    fish_game_input(&g,FISH_ROTATE,1,6400);
    assert(g.mode==FISH_CAUGHT && g.species_best_size_mm[0]==300 && g.species_catches[0]==2);
    puts("PASS: corrected tiers, 1M encounter rolls, guide chord/navigation/return/no-RNG, best-size records");
}
