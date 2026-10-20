#include "fish_save.h"
#include <assert.h>
#include <stdio.h>
int fish_storage_init(uint8_t *k){(void)k;return -1;}
int fish_storage_read(unsigned s,uint8_t *p,size_t n){(void)s;(void)p;(void)n;return -2;}
int fish_storage_begin(unsigned s,const uint8_t *p,size_t n){(void)s;(void)p;(void)n;return -1;}
int fish_storage_poll(void){return 0;}
static void fight(fish_game_t *g,unsigned line,unsigned owned)
{
 fish_game_init(g,0);g->economy.lines=(uint8_t)owned;fish_game_equip_line(g,line);
 g->mode=FISH_FIGHT;g->pending_species=1;g->pending_weight_g=13000;
 g->base_pull=40;g->stamina=g->stamina_max=1000;g->tension=g->line_strength;
}
int main(void)
{
 fish_game_t g,loaded;uint8_t payload[FISH_SAVE_PAYLOAD_BYTES];
 assert(fish_shop_price(3)==120 && fish_shop_price(FISH_SHOP_STANDARD_LINE)==200 && fish_shop_price(4)==450);
 assert(fish_shop_price(FISH_SHOP_PE_LINE)==600 && fish_line_sets[FISH_PE_LINE].strength>fish_line_sets[2].strength);
 /* City-only sale; ownership and equipment survive the existing save schema. */
 fish_game_init(&g,0);g.game_minute=720;g.economy.coins=600;
 assert(fish_shop_trade(&g,FISH_SHOP_PE_LINE)==FISH_MERCHANT_EMPTY);
 g.map_index=1;assert(fish_shop_trade(&g,FISH_SHOP_PE_LINE)==FISH_MERCHANT_BOUGHT && g.line_index==FISH_PE_LINE && !g.economy.coins);
 fish_save_pack(&g,payload);fish_game_init(&loaded,0);assert(fish_save_unpack(&loaded,payload) && loaded.line_index==FISH_PE_LINE && (loaded.economy.lines&8));
 uint64_t small[2]={0},big[2]={0};
 for(unsigned n=1;n<=20000;n++)for(unsigned line=0;line<2;line++) {
  uint32_t seed=n*0x9e3779b9u;small[line]+=fish_line_bite_wait(&seed,line?FISH_PE_LINE:2,500);
  seed=n*0x9e3779b9u;big[line]+=fish_line_bite_wait(&seed,line?FISH_PE_LINE:2,30000);
 }
 assert(small[1]>small[0]*2 && big[1]<big[0]);
 unsigned small_catches[2]={0};uint64_t waits[2]={0};
 for(unsigned n=1;n<=10000;n++)for(unsigned line=0;line<2;line++) {
  fish_game_init(&g,0);g.map_index=1;g.boat_x=480;g.depth_index=0;g.random_state=n*0x9e3779b9u;
  fish_game_equip_line(&g,line?FISH_PE_LINE:2);g.economy.lines|=(uint8_t)(1u<<g.line_index);
  fish_game_input(&g,FISH_PRESS,0,0);fish_game_input(&g,FISH_RELEASE,0,1500);
  assert(g.mode==FISH_CAST && g.pending_species<FISH_SPECIES_COUNT);
  small_catches[line]+=g.pending_weight_g<=1000;waits[line]+=g.bite_wait_ms;
 }
 printf("10000 city casts:small fish heavy=%u PE=%u;mean wait heavy=%llu PE=%llu ms\n",small_catches[0],small_catches[1],(unsigned long long)(waits[0]/10000),(unsigned long long)(waits[1]/10000));
 assert(small_catches[1]<small_catches[0]*3/4 && waits[1]>waits[0]);
 for(unsigned map=0;map<2;map++)for(unsigned coins=0;coins<=120;coins+=17) {
  fish_game_init(&g,0);g.map_index=(uint8_t)map;g.game_minute=720;g.economy.coins=coins;
  fish_hold_add(&g.economy,0,200,100); /* Relief no longer requires an empty hold. */
  assert(fish_shop_free_line(&g));assert(fish_shop_trade(&g,3)==FISH_MERCHANT_RELIEF);
  assert(g.economy.coins==coins && (g.economy.lines&1) && g.save_requested);
  g.merchant_reply=FISH_MERCHANT_RELIEF;assert(fish_merchant_says(&g)[0] && fish_merchant_pose(&g)<4);
  assert(fish_shop_trade(&g,3)==FISH_MERCHANT_OWNED);
  fish_save_pack(&g,payload);assert(fish_save_unpack(&loaded,payload) && (loaded.economy.lines&1));
 }
 fish_game_init(&g,0);g.game_minute=720;g.economy.coins=120;
 assert(!fish_shop_free_line(&g) && fish_shop_trade(&g,3)==FISH_MERCHANT_BOUGHT && !g.economy.coins);
 for(unsigned map=0;map<2;map++)for(unsigned line=0;line<FISH_LINE_COUNT;line++) {
  fight(&g,line,(1u<<FISH_LINE_COUNT)-1);g.map_index=(uint8_t)map;g.overload_ms=fish_rules.overload_limit_ms-20;
  fish_game_update(&g,20);assert(g.mode==FISH_LOST && g.loss==FISH_LOSS_BREAK && g.save_requested);
  assert(g.economy.lines==(((1u<<FISH_LINE_COUNT)-1)&~(1u<<line)) && fish_equipment_owned(&g.economy,FISH_BAG_LINE,g.line_index));
  fish_save_pack(&g,payload);fish_game_init(&loaded,0);assert(fish_save_unpack(&loaded,payload));
  assert(loaded.economy.lines==g.economy.lines && loaded.line_index==g.line_index);
 }
 /* Last rig stays lost after load and cannot cast. Paid replacements recover
  * fishing; missing funds never changes ownership or consumes bait. */
 for(unsigned line=0;line<FISH_LINE_COUNT;line++) {
  fight(&g,line,1u<<line);g.overload_ms=fish_rules.overload_limit_ms-20;fish_game_update(&g,20);
  assert(!g.economy.lines);fish_save_pack(&g,payload);fish_game_init(&loaded,0);assert(fish_save_unpack(&loaded,payload));
  assert(!loaded.economy.lines);loaded.mode=FISH_AIM;loaded.economy.bait_stock[0]=5;loaded.bait_index=0;
  fish_game_input(&loaded,FISH_PRESS,0,1);fish_game_input(&loaded,FISH_RELEASE,0,1501);
  assert(loaded.mode!=FISH_CAST && loaded.economy.bait_stock[0]==5);
  loaded.game_minute=720;assert(fish_shop_trade(&loaded,FISH_SHOP_STANDARD_LINE)==FISH_MERCHANT_POOR && !loaded.economy.lines);
  loaded.economy.coins=200;assert(fish_shop_trade(&loaded,FISH_SHOP_STANDARD_LINE)==FISH_MERCHANT_BOUGHT && loaded.economy.lines==2 && loaded.line_index==1 && !loaded.economy.coins);
  loaded.mode=FISH_AIM;fish_game_input(&loaded,FISH_PRESS,0,2000);fish_game_input(&loaded,FISH_RELEASE,0,3500);assert(loaded.mode==FISH_CAST);
 }
 /* Both overload and the range limit destroy the current rig; slack, early
  * strikes and missed bites preserve it. */
 fight(&g,1,2);g.reel_units=9999-g.line_capacity_units;g.range_ms=fish_rules.range_limit_ms-20;fish_game_update(&g,20);
 assert(g.mode==FISH_LOST && g.loss==FISH_LOSS_BREAK && !g.economy.lines);
 fight(&g,1,2);g.tension=0;g.slack_ms=fish_rules.slack_limit_ms-20;fish_game_update(&g,20);
 assert(g.loss==FISH_LOSS_SLACK && g.economy.lines==2);
 fish_game_init(&g,0);g.mode=FISH_BITE;g.pending_species=0;g.phase_at=0;
 fish_game_input(&g,FISH_PRESS,0,0);fish_game_input(&g,FISH_RELEASE,0,0);assert(g.loss==FISH_LOSS_EARLY && g.economy.lines==2);
 fish_game_init(&g,0);g.mode=FISH_BITE;g.pending_species=0;fish_game_update(&g,10000);assert(g.loss==FISH_LOSS_MISSED && g.economy.lines==2);
 puts("PASS: line prices, break-only damage, both maps, spare selection, last-rig save/reload, no-rig cast lock and paid replacements");
}
