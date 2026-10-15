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
    assert(total==197 && weights[0]==140 && weights[1]==50 && weights[2]==7);
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
    fish_game_t g;
    fish_game_init(&g,0); uint32_t before=g.random_state;
    int x=g.boat_x;
    fish_game_input(&g,FISH_PRESS,0,10);
    fish_game_input(&g,FISH_ROTATE,1,30); assert(g.mode==FISH_GUIDE && g.guide_index==0);
    fish_game_input(&g,FISH_RELEASE,0,40); assert(g.mode==FISH_GUIDE && !g.pressed);
    fish_game_input(&g,FISH_ROTATE,-1,50); assert(g.guide_index==FISH_SPECIES_COUNT-1);
    fish_game_input(&g,FISH_ROTATE,2,60); assert(g.guide_index==1);
    fish_game_update(&g,5000); assert(g.mode==FISH_GUIDE);
    assert(g.random_state==before && g.boat_x==x && g.catches==0);
    fish_game_input(&g,FISH_PRESS,0,5010);
    fish_game_input(&g,FISH_RELEASE,0,5020); assert(g.mode==FISH_MOVE);
    /* Restore FISH mode when the guide was opened while aiming. */
    g.mode=FISH_AIM;
    fish_game_input(&g,FISH_PRESS,0,5100); fish_game_input(&g,FISH_ROTATE,1,5120);
    fish_game_input(&g,FISH_RELEASE,0,5130); assert(g.mode==FISH_GUIDE);
    fish_game_input(&g,FISH_PRESS,0,5200); fish_game_input(&g,FISH_RELEASE,0,5210);
    assert(g.mode==FISH_AIM);
    /* Existing records remain until a larger catch is landed. */
    g.pending_species=0; g.pending_size_mm=200;
    g.pending_weight_g=fish_species_weight_g(&fish_species[0],200);
    g.species_best_size_mm[0]=250;
    g.mode=FISH_BITE; g.phase_at=5300;
    g.phase_at-=fish_species[0].strike_delay_ms;
    fish_game_input(&g,FISH_PRESS,0,5300); fish_game_input(&g,FISH_RELEASE,0,5300);
    g.reel_units=9900; g.tension=20;g.stamina=0;
    fish_game_input(&g,FISH_ROTATE,1,5300);
    assert(g.mode==FISH_CAUGHT && g.species_best_size_mm[0]==250);
    g.mode=FISH_BITE; g.phase_at=5400; g.pending_size_mm=300;
    g.phase_at-=fish_species[0].strike_delay_ms;
    g.pending_weight_g=fish_species_weight_g(&fish_species[0],300);
    fish_game_input(&g,FISH_PRESS,0,5400); fish_game_input(&g,FISH_RELEASE,0,5400);
    g.reel_units=9900; g.tension=20;g.stamina=0;
    fish_game_input(&g,FISH_ROTATE,1,5400);
    assert(g.mode==FISH_CAUGHT && g.species_best_size_mm[0]==300 && g.species_catches[0]==2);
    puts("PASS: corrected tiers, 1M encounter rolls, guide chord/navigation/return/no-RNG, best-size records");
}
