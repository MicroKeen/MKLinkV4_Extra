#include "fish_game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    for(unsigned v=FISH_BASE_SPECIES_COUNT;v<FISH_SPECIES_COUNT;v++) {
        unsigned base=fish_species_parent(v),special=0;uint32_t seed=0x8bd134u+v;
        for(unsigned n=0;n<200000;n++) {unsigned id=fish_species_special_roll(&seed,base);assert(id==base || id==v);special+=id==v;}
        assert(special>1800 && special<2200);
        for(unsigned size=fish_species[v].min_mm;size<=fish_species[v].max_mm;size++) assert(fish_catch_price(v,size)==10*fish_catch_price(base,size));
        fish_game_t g;fish_game_init(&g,0);g.game_minute=720;uint16_t size=fish_species[v].mean_x2_mm/2;
        assert(fish_hold_add(&g.economy,v,size,fish_species_weight_g(&fish_species[v],size))==FISH_STORED);
        assert(fish_hold_page_count(&g.economy)==1 && fish_hold_page_index(&g.economy,0)==v);
        g.merchant_hint=true;assert(fish_merchant_pose(&g)==(v==13?2:3));
        const char *line=fish_merchant_says(&g);uint32_t before=g.random_state;
        assert(!strcmp(line,fish_merchant_says(&g)) && before==g.random_state);
        uint32_t price=fish_hold_value(&g.economy);assert(price==fish_catch_price(v,size));
        assert(fish_shop_trade(&g,0)==FISH_MERCHANT_SOLD && g.economy.coins==price && !fish_hold_page_count(&g.economy));
    }
    for(unsigned i=0;i<FISH_BASE_SPECIES_COUNT;i++) if(i!=1 && i!=2 && i!=3 && i!=9) {uint32_t seed=42;assert(fish_species_special_roll(&seed,i)==i && seed==42);}
    unsigned hints=0;
    for(unsigned i=1;i<=10000;i++) {
        fish_game_t g;fish_game_init(&g,0);g.mode=FISH_SHORE;g.shore_x=FISH_SHOP_X;g.game_minute=720;g.random_state=i*0x9e3779b9u;
        fish_game_input(&g,FISH_PRESS,0,0);fish_game_input(&g,FISH_RELEASE,0,1);assert(g.mode==FISH_SHOP);hints+=g.merchant_hint;
        uint32_t seed=g.random_state;bool hint=g.merchant_hint;
        fish_game_input(&g,FISH_ROTATE,1,2);assert(seed==g.random_state && hint==g.merchant_hint);
    }
    assert(hints>400 && hints<600);
    fish_game_t g;fish_game_init(&g,0);g.mode=FISH_FIGHT;g.pending_species=3;g.pending_variant=11;
    g.pending_size_mm=450;g.pending_weight_g=fish_species_weight_g(&fish_species[3],450);g.reel_units=10000;
    fish_game_input(&g,FISH_ROTATE,1,0);
    assert(g.mode==FISH_CAUGHT && g.last_species==11 && g.species_catches[11]==1 && !g.species_catches[3]);
    assert(g.economy.value[11]==fish_catch_price(3,450)*10);
    puts("PASS: four conditional1% rolls, exact rounded x10 prices, distinct cargo, rare greetings,5% cached hints and actual landed identity");
}
