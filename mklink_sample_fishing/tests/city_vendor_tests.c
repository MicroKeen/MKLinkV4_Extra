#include "fish_save.h"
#include "fish_save_crypto.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int fish_storage_init(uint8_t *k){(void)k;return -1;}
int fish_storage_read(unsigned s,uint8_t *p,size_t n){(void)s;(void)p;(void)n;return -2;}
int fish_storage_begin(unsigned s,const uint8_t *p,size_t n){(void)s;(void)p;(void)n;return -1;}
int fish_storage_poll(void){return 0;}
static void tap(fish_game_t *g,uint32_t now){fish_game_input(g,FISH_PRESS,0,now);fish_game_input(g,FISH_RELEASE,0,now+1);}
static void start(fish_game_t *g,unsigned vendor){fish_game_init(g,0);g->mode=FISH_SHOP;g->map_index=1;g->shop_venue=vendor;g->game_minute=720;g->economy.coins=10000;}
static void cast(fish_game_t *g,unsigned bait,unsigned seed)
{
 fish_game_init(g,0);g->map_index=1;g->game_minute=720;g->boat_x=480;g->facing=1;g->depth_index=7;
 g->random_state=seed;g->bait_index=(uint8_t)bait;g->economy.bait_stock[bait]=5;fish_game_equip_line(g,2);
 fish_game_input(g,FISH_PRESS,0,0);fish_game_input(g,FISH_RELEASE,0,1500);
}
static void fight(fish_game_t *g)
{
 fish_game_init(g,10000);g->mode=FISH_FIGHT;g->pending_species=22;g->pending_weight_g=10000;
 g->base_pull=30;g->struggle_percent=100;g->stamina=50;g->stamina_max=1000;g->tension=30;g->phase_at=9000;
}
int main(void)
{
 fish_game_t g,a,b;unsigned removed[]={5,7,8,9};
 start(&g,FISH_VENDOR_TACKLE);assert(fish_shop_count(&g)==17);
 for(unsigned i=0;i<fish_shop_count(&g);i++) {unsigned id=fish_shop_item_at(&g,i);assert(fish_shop_position(&g,id)==i);for(unsigned j=0;j<4;j++)assert(id!=removed[j]);}
 for(unsigned j=0;j<4;j++) {fish_economy_t saved=g.economy;assert(fish_shop_trade(&g,removed[j])==FISH_MERCHANT_EMPTY);assert(!memcmp(&saved,&g.economy,sizeof(saved)));}
 for(unsigned i=16;i<=18;i++) {assert(fish_shop_trade(&g,i)==FISH_MERCHANT_BOUGHT);assert(g.economy.bait_stock[6+i-16]==5);assert(g.bait_index==6+i-16);}
 assert(fish_shop_trade(&g,19)==FISH_MERCHANT_EMPTY);
 fish_game_init(&g,0);g.game_minute=720;g.economy.coins=1000;
 assert(fish_shop_count(&g)==17);
 for(unsigned j=0;j<4;j++)assert(fish_shop_trade(&g,removed[j])==FISH_MERCHANT_BOUGHT);
 assert(fish_shop_trade(&g,16)==FISH_MERCHANT_EMPTY && fish_shop_trade(&g,19)==FISH_MERCHANT_EMPTY);
 for(unsigned i=0;i<fish_shop_count(&g);i++)assert(fish_shop_position(&g,fish_shop_item_at(&g,i))==i);
 start(&g,FISH_VENDOR_CONVENIENCE);assert(fish_shop_count(&g)==2 && fish_shop_item_at(&g,0)==19 && fish_shop_item_at(&g,1)==15);
 fish_hold_add(&g.economy,0,200,100);unsigned value=fish_hold_value(&g.economy);assert(fish_shop_trade(&g,0)==FISH_MERCHANT_EMPTY && fish_hold_value(&g.economy)==value);
 assert(fish_shop_trade(&g,19)==FISH_MERCHANT_LATTE && g.latte_remaining_ms==360000 && g.economy.coins==9965 && g.save_requested);
 g.merchant_reply=FISH_MERCHANT_LATTE;assert(fish_merchant_pose(&g)==4 && strstr(fish_merchant_says(&g),"烫"));
 fish_game_update(&g,100000);assert(g.latte_remaining_ms==260000);
 assert(fish_shop_trade(&g,19)==FISH_MERCHANT_LATTE && g.latte_remaining_ms==360000);
 g.economy.coins=0;assert(fish_shop_trade(&g,19)==FISH_MERCHANT_POOR && g.latte_remaining_ms==360000);
 g.mode=FISH_LOADING;g.travel_requested=true;g.travel_paid=true;g.travel_target=0;g.economy.coins=180;
 fish_game_update(&g,200000);assert(g.latte_remaining_ms==360000);fish_game_travel_complete(&g,true);assert(g.latte_remaining_ms==60000 && !g.economy.coins);
 fish_game_update(&g,260000);assert(!g.latte_remaining_ms);
 fish_game_init(&g,UINT32_MAX-50);g.latte_remaining_ms=100;fish_game_update(&g,48);assert(g.latte_remaining_ms==1);fish_game_update(&g,49);assert(!g.latte_remaining_ms);
 fight(&a);b=a;b.latte_remaining_ms=10000;fish_game_input(&a,FISH_ROTATE,1,10000);fish_game_input(&b,FISH_ROTATE,1,10000);
 assert(b.reel_units==a.reel_units*110/100);
 fight(&a);b=a;b.latte_remaining_ms=10000;fish_game_input(&a,FISH_ROTATE,-1,10000);fish_game_input(&b,FISH_ROTATE,-1,10000);assert(a.reel_units==-150 && b.reel_units==-165);
 a.reel_units=10000-(int)a.line_capacity_units;b.reel_units=10000-(int)b.line_capacity_units;
 fish_game_input(&a,FISH_ROTATE,-40,10000);fish_game_input(&b,FISH_ROTATE,-40,10000);assert(a.reel_units==b.reel_units);
 fish_game_init(&a,0);a.mode=FISH_BITE;a.pending_species=0;a.phase_at=0;b=a;b.latte_remaining_ms=10000;
 unsigned end_a=0,end_b=0;for(unsigned t=0;t<5000;t++){a.now=b.now=t;if(fish_game_strike_ready(&a))end_a=t+1;if(fish_game_strike_ready(&b))end_b=t+1;}
 unsigned delay=fish_species[0].strike_delay_ms;assert(end_b-delay==(end_a-delay)*110/100);
 for(unsigned bait=6;bait<9;bait++) {
  cast(&g,bait,42);assert(g.bait_index==bait && g.economy.bait_stock[bait]==4 && g.pending_species<FISH_SPECIES_COUNT);
  assert(fish_equipment_owned(&g.economy,FISH_BAG_BAIT,bait));g.economy.bait_stock[bait]=0;g.mode=FISH_AIM;
  fish_game_input(&g,FISH_PRESS,0,2000);fish_game_input(&g,FISH_RELEASE,0,3500);assert(g.bait_index==5);
 }
 /* Categorical checks over all x/layers/times preserve habitat constraints. */
 unsigned target_hits[3]={0};
 for(unsigned bait=6;bait<9;bait++)for(unsigned x=70;x<904;x+=31)for(unsigned depth=0;depth<=1600;depth+=100)for(unsigned minute=0;minute<1440;minute+=180)for(unsigned id=0;id<25;id++) {
  unsigned w=fish_map_weight_on(1,id,x,depth,bait,minute);if(!w)continue;
  assert(depth<=(unsigned)fish_map_depth_on(1,x));assert(id!=21 && !fish_species_is_variant(id));
  if(bait==6)assert(id==1 || id==17);
  if(bait==7)assert(id==0 || id==1 || id==16 || id==18 || id==24);
  if(bait==8)assert(id==15 || id==20 || id==22 || id==23);
  ++target_hits[bait-6];
 }
 for(unsigned i=0;i<3;i++)assert(target_hits[i]>100);
 uint64_t waits[2]={0};unsigned selected[25]={0};
 for(unsigned seed=1;seed<=10000;seed++)for(unsigned variant=0;variant<2;variant++) {
  cast(&g,variant?6:5,seed*0x9e3779b9u);waits[variant]+=g.bite_wait_ms;if(variant)++selected[g.pending_species];
 }
 assert(selected[17]>selected[1]*4 && waits[1]>waits[0]);
 /* Independent control uses the same eligible species weights with normal size sampling. */
 uint64_t sizes[2]={0};unsigned counts[2]={0};
 for(unsigned seed=1;seed<=10000;seed++)for(unsigned variant=0;variant<2;variant++) {
  uint32_t random=seed*0x9e3779b9u;uint8_t id;uint16_t size;uint16_t weights[25];
  for(unsigned i=0;i<25;i++)weights[i]=fish_map_weight_on(1,i,480,1600,8,720);
  bool found=variant?fish_map_roll_on(&random,1,480,1600,8,720,&id,&size):fish_species_roll_weights_on(&random,weights,1,&id,&size);
  assert(found);if(id==22){sizes[variant]+=size;++counts[variant];}
 }
 assert(counts[0]>100 && counts[1]>100 && sizes[1]*counts[0]>sizes[0]*counts[1]);
 start(&g,FISH_VENDOR_CONVENIENCE);g.latte_remaining_ms=123456;g.economy.bait_stock[6]=998;g.economy.bait_stock[7]=1;g.economy.bait_stock[8]=12;
 uint8_t p[FISH_SAVE_PAYLOAD_BYTES],q[FISH_SAVE_PAYLOAD_BYTES],key[32]={1,2},old[FISH_SAVE_V10_FILE_BYTES]={0};uint64_t seq;
 fish_save_pack(&g,p);fish_game_init(&a,0);assert(fish_save_unpack(&a,p) && a.latte_remaining_ms==123456 && a.economy.bait_stock[6]==998 && a.economy.bait_stock[8]==12 && a.map_index==1);
 memcpy(old,"FSG1",4);old[4]=10;old[6]=FISH_SAVE_V10_PAYLOAD_BYTES&255;old[7]=FISH_SAVE_V10_PAYLOAD_BYTES>>8;old[8]=17;
 fish_save_seal(key,old,16,p,FISH_SAVE_V10_PAYLOAD_BYTES,old+16,old+32);
 assert(fish_save_decode(key,old,sizeof(old),&seq,q) && seq==17);fish_game_init(&a,0);assert(fish_save_unpack(&a,q));
 assert(a.map_index==1 && a.economy.coins==g.economy.coins && !a.latte_remaining_ms && !a.economy.bait_stock[6] && !a.economy.bait_stock[8]);
 g.latte_remaining_ms=360001;fish_save_pack(&g,p);assert(!fish_save_unpack(&a,p));
 g.latte_remaining_ms=0;g.economy.bait_stock[8]=1000;fish_save_pack(&g,p);assert(!fish_save_unpack(&a,p));
 fish_game_init(&g,0);g.mode=FISH_SHORE;g.map_index=1;g.shore_x=24;g.boat_x=132;g.game_minute=720;
 fish_game_input(&g,FISH_ROTATE,-1,0);for(unsigned t=16;t<5000;t+=16)fish_game_update(&g,t);
 assert(g.shore_x==FISH_CITY_STALL_X && !g.shore_speed);tap(&g,5000);assert(g.mode==FISH_SHOP && g.shop_venue==FISH_VENDOR_TACKLE);
 g.shop_index=15;tap(&g,5002);fish_game_input(&g,FISH_ROTATE,-1,5004);for(unsigned t=5020;t<12000;t+=16)fish_game_update(&g,t);
 assert(g.shore_x==FISH_SHOP_X && !g.shore_speed);tap(&g,12000);assert(g.mode==FISH_SHOP && g.shop_venue==FISH_VENDOR_CONVENIENCE && g.shop_index==19);
 puts("PASS: separate vendors, unchanged wetland, specialized bait habitats/size/wait statistics, latte10%/6h/travel/wrap and V10-to-V11 preservation");
 printf("Urban bottom mean waits: general=%.1fms snail=%.1fms; snail blue-carp hits=%u/10000\n",waits[0]/10000.0,waits[1]/10000.0,selected[17]);
}
