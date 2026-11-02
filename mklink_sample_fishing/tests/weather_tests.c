#include "fish_save.h"
#include "fish_save_crypto.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const uint8_t key[32]={4,7,12};
static uint8_t old[FISH_SAVE_V11_FILE_BYTES],written[FISH_SAVE_FILE_BYTES];
static bool legacy_present,pending;static unsigned write_slot;
int fish_storage_init(uint8_t *k){memcpy(k,key,32);return 0;}
int fish_storage_read(unsigned slot,uint8_t *p,size_t n)
{if(slot!=2 || !legacy_present)return -2;if(n!=sizeof(old))return -3;memcpy(p,old,n);return 0;}
int fish_storage_begin(unsigned slot,const uint8_t *p,size_t n)
{assert(slot<2 && n==sizeof(written));write_slot=slot;memcpy(written,p,n);pending=true;return 0;}
int fish_storage_poll(void){if(pending){pending=false;return 1;}return 0;}
static void put32(uint8_t *p,uint32_t value){for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(value>>(8*i));}
static void fight(fish_game_t *g)
{fish_game_init(g,10000);g->mode=FISH_FIGHT;g->pending_species=22;g->pending_weight_g=10000;g->base_pull=30;g->struggle_percent=100;g->stamina=50;g->stamina_max=1000;g->tension=30;g->phase_at=9000;}
static void rest_checks(void)
{
 for(unsigned map=0;map<3;map++){
  fish_game_t g,normal,loaded;fish_game_init(&g,0);g.map_index=map;g.mode=FISH_BAG;g.bag_return_mode=FISH_MOVE;g.bag_index=FISH_BAG_REST;
  g.boat_x=fish_map_boat_left(map)+100;g.cat.friendship=50;g.cat.feeds=2;
  g.game_minute=1430;g.weather_ms=1430000;g.latte_remaining_ms=240000;g.comforts=FISH_COMFORT_NOODLES;
  normal=g;normal.bag_index=FISH_BAG_BACK;
  bool found=false;unsigned count=fish_game_bag_count(&g);
  for(unsigned i=0;i<count;i++){unsigned item=fish_game_bag_item_at(&g,i);found|=item==FISH_BAG_REST;for(unsigned j=0;j<i;j++)assert(item!=fish_game_bag_item_at(&g,j));}
  assert(found && fish_game_bag_item_at(&g,count-1)==FISH_BAG_BACK);
  int x=g.boat_x;uint32_t random=g.random_state;
  fish_game_input(&g,FISH_PRESS,0,0);fish_game_update(&g,599);assert(!fish_game_resting(&g) && g.clock_remainder_ms==599);
  fish_game_update(&g,600);assert(fish_game_resting(&g));fish_game_update(&g,1600);
  fish_game_update(&normal,20600);assert(g.game_minute==10 && g.clock_remainder_ms==600);
  assert(g.weather_ms==normal.weather_ms && g.wetness_units==normal.wetness_units && g.latte_remaining_ms==normal.latte_remaining_ms);
  assert(!g.cat.feeds && g.cat.day==normal.cat.day && g.boat_x==x && g.random_state==random && g.mode==FISH_BAG);
  uint8_t p[FISH_SAVE_PAYLOAD_BYTES];fish_save_pack(&g,p);fish_game_init(&loaded,0);assert(fish_save_unpack(&loaded,p));
  assert(loaded.game_minute==g.game_minute && loaded.clock_remainder_ms==g.clock_remainder_ms && loaded.weather_ms==g.weather_ms && fish_cat_owned(&loaded) && !loaded.pressed && !fish_game_resting(&loaded));
  fish_game_input(&g,FISH_RELEASE,0,1700);uint32_t weather=g.weather_ms;assert(!fish_game_resting(&g) && g.mode==FISH_BAG && !g.bag_editing && g.save_requested);
  fish_game_update(&g,2700);assert(g.weather_ms==weather+1000 && g.boat_x==x);
  fish_game_input(&g,FISH_PRESS,0,2800);fish_game_input(&g,FISH_RELEASE,0,3000);assert(g.weather_ms==weather+1300 && !g.bag_editing);
  fish_game_input(&g,FISH_PRESS,0,3100);fish_game_input(&g,FISH_ROTATE,1,4100);assert(!g.pressed && g.bag_index==FISH_BAG_SAVE);
  weather=g.weather_ms;g.save_requested=false;fish_game_input(&g,FISH_RELEASE,0,5100);assert(g.weather_ms==weather+1000 && !g.save_requested);
  g.bag_return_mode=FISH_SHORE;g.bag_index=FISH_BAG_REST;
  for(unsigned i=0;i<fish_game_bag_count(&g);i++)assert(fish_game_bag_item_at(&g,i)!=FISH_BAG_REST);
  fish_game_input(&g,FISH_PRESS,0,5200);weather=g.weather_ms;fish_game_update(&g,7200);assert(!fish_game_resting(&g) && g.weather_ms==weather+2000);
 }
 /* Exact threshold integration also works across the32-bit clock wrap. */
 fish_game_t g;fish_game_init(&g,UINT32_MAX-300);g.mode=FISH_BAG;g.bag_return_mode=FISH_MOVE;g.bag_index=FISH_BAG_REST;
 fish_game_input(&g,FISH_PRESS,0,UINT32_MAX-300);uint32_t weather=g.weather_ms;fish_game_update(&g,1299);
 assert(fish_game_resting(&g) && g.weather_ms==weather+20600);
 puts("PASS: three-map boat rest20x, exact600ms hold, release/cancel, normal tap, shoreline exclusion, midnight/buff/weather/save and32-bit clock wrap");
}
int main(void)
{
 rest_checks();
 fish_game_t a,b;fish_game_init(&a,0);assert(!fish_weather_rain(&a) && !a.wetness_units);
 a.weather_ms=450000;assert(!fish_weather_rain(&a));a.weather_ms+=4000;assert(fish_weather_rain(&a)==127);a.weather_ms+=4000;assert(fish_weather_rain(&a)==255);
 a.weather_ms=596000;assert(fish_weather_rain(&a)==127);a.weather_ms=600000;assert(!fish_weather_rain(&a));
 /* Light/strong visual swells never change the weather/catch RNG or rain's
  * wetting/encounter envelope. The envelope still fades on the storm's edges. */
 uint32_t rng=a.random_state;
 a.weather_ms=500000;assert(!fish_weather_strength(&a) && fish_weather_rain(&a)==255);
 a.weather_ms=540000;assert(fish_weather_strength(&a)==127 && fish_weather_rain(&a)==255);
 a.weather_ms=570000;assert(fish_weather_strength(&a)==255 && fish_weather_rain(&a)==255);
 unsigned previous=0;
 for(unsigned t=450000;t<=600000;t+=100){a.weather_ms=t;unsigned strength=fish_weather_strength(&a),visible=strength*fish_weather_rain(&a)/255;assert(strength<=255 && abs((int)visible-(int)previous)<=4);previous=visible;}
 assert(a.random_state==rng && !fish_weather_strength(&a));
 a.weather_ms=500000;fish_weather_advance(&a,30000,false);assert(a.wetness_units==60000 && fish_game_wetness_percent(&a)==50 && fish_game_line_speed_percent(&a)==90);
 fish_weather_advance(&a,30000,false);assert(a.wetness_units==FISH_WETNESS_FULL && fish_game_line_speed_percent(&a)==80);
 a.latte_remaining_ms=1;assert(fish_game_line_speed_percent(&a)==88);a.latte_remaining_ms=0;
 fish_weather_advance(&a,10000,false);assert(a.wetness_units==FISH_WETNESS_FULL);
 fish_weather_advance(&a,60000,true);assert(a.wetness_units==60000);fish_weather_advance(&a,60000,true);assert(!a.wetness_units);
 a.weather_ms=600000;a.wetness_units=FISH_WETNESS_FULL;fish_weather_advance(&a,60000,false);assert(a.wetness_units==60000);fish_weather_advance(&a,60000,false);assert(!a.wetness_units);
 /* Wetness is independent of frame partition, period crossings and uint32 clock wrap. */
 for(unsigned seed=0;seed<50;seed++) {
  fish_game_init(&a,0);a.weather_ms=(430123+seed*617123)%FISH_WEATHER_CYCLE_MS;a.wetness_units=seed*1973%FISH_WETNESS_FULL;b=a;
  unsigned remaining=197654;fish_weather_advance(&a,remaining,false);
  while(remaining){unsigned part=13+(remaining*17u+seed)%1999;if(part>remaining)part=remaining;fish_weather_advance(&b,part,false);remaining-=part;}
  assert(a.weather_ms==b.weather_ms && a.wetness_units==b.wetness_units);
 }
 fish_game_init(&a,0);a.weather_ms=1234567;a.wetness_units=110000;b=a;fish_weather_advance(&a,UINT32_MAX,false);
 uint32_t remaining=UINT32_MAX;while(remaining){unsigned part=remaining>720000?720000:remaining;fish_weather_advance(&b,part,false);remaining-=part;}
 assert(a.weather_ms==b.weather_ms && a.wetness_units==b.wetness_units);
 fish_game_init(&a,UINT32_MAX-500);a.weather_ms=500000;a.wetness_units=10000;fish_game_update(&a,499);assert(a.weather_ms==501000 && a.wetness_units==12000);
 a.mode=FISH_LOADING;unsigned phase=a.weather_ms,wet=a.wetness_units;fish_game_update(&a,8000);assert(a.weather_ms==phase && a.wetness_units==wet);
 a.travel_requested=true;a.travel_paid=true;a.travel_target=1;a.economy.coins=180;fish_game_travel_complete(&a,true);assert(a.weather_ms==phase+300000 && !a.wetness_units);
 fish_game_init(&a,0);a.mode=FISH_SHOP;a.weather_ms=500000;a.wetness_units=100000;fish_game_update(&a,10000);assert(a.wetness_units==90000);
 a.mode=FISH_HOLD;a.hold_return_mode=FISH_SHOP;fish_game_update(&a,20000);assert(a.wetness_units==80000);
 a.mode=FISH_BAG;a.bag_return_mode=FISH_MOVE;fish_game_update(&a,30000);assert(a.wetness_units==100000);
 unsigned last=101;
 for(unsigned wet=0;wet<=FISH_WETNESS_FULL;wet+=1200){a.wetness_units=wet;a.latte_remaining_ms=0;unsigned speed=fish_game_line_speed_percent(&a);assert(speed<=last && speed>=80);last=speed;}
 fight(&a);b=a;b.wetness_units=FISH_WETNESS_FULL;fish_game_input(&a,FISH_ROTATE,1,10000);fish_game_input(&b,FISH_ROTATE,1,10000);assert(b.reel_units==a.reel_units*80/100);
 fight(&a);b=a;b.wetness_units=FISH_WETNESS_FULL;fish_game_input(&a,FISH_ROTATE,-1,10000);fish_game_input(&b,FISH_ROTATE,-1,10000);assert(a.reel_units==-150 && b.reel_units==-120);
 b.latte_remaining_ms=1000;b.reel_units=0;fish_game_input(&b,FISH_ROTATE,-1,10000);assert(b.reel_units==-132);
 b.reel_units=10000-(int)b.line_capacity_units;fish_game_input(&b,FISH_ROTATE,-40,10000);assert(fish_game_distance(&b)==(int)b.line_capacity_units);
 fish_game_init(&a,0);a.weather_ms=480000;a.wetness_units=12345;a.game_minute=480;a.map_index=1;a.latte_remaining_ms=54321;a.economy.bait_stock[8]=7;a.economy.coins=4321;
 uint8_t p[FISH_SAVE_PAYLOAD_BYTES],q[FISH_SAVE_PAYLOAD_BYTES],file[FISH_SAVE_FILE_BYTES];uint64_t sequence;
 fish_save_pack(&a,p);fish_save_encode(key,12,p,file);assert(sizeof(file)==FISH_SAVE_FILE_BYTES && FISH_SAVE_V17_FILE_BYTES==1511);
 assert(fish_save_decode(key,file,sizeof(file),&sequence,q));fish_game_init(&b,0);assert(fish_save_unpack(&b,q));
 assert(b.weather_ms==a.weather_ms && b.wetness_units==a.wetness_units && b.map_index==1 && b.latte_remaining_ms==54321 && b.economy.bait_stock[8]==7);
 put32(q+FISH_SAVE_WEATHER_OFFSET,FISH_WEATHER_CYCLE_MS);assert(!fish_save_unpack(&b,q));memcpy(q,p,sizeof(q));put32(q+FISH_SAVE_WETNESS_OFFSET,FISH_WETNESS_FULL+1);assert(!fish_save_unpack(&b,q));
 /* Real authenticated V11 prefix, including latte/stock. Boot finds its older
  * slot, migrates into V12_A/B, and never modifies the old file. */
 p[187]=25; /* V11 used species25 as the empty stern sentinel. */
 memcpy(old,"FSG1",4);old[4]=11;old[6]=FISH_SAVE_V11_PAYLOAD_BYTES&255;old[7]=FISH_SAVE_V11_PAYLOAD_BYTES>>8;old[8]=25;
 fish_save_seal(key,old,16,p,FISH_SAVE_V11_PAYLOAD_BYTES,old+16,old+32);uint8_t preserved[sizeof(old)];memcpy(preserved,old,sizeof(old));
 legacy_present=true;fish_save_t save;fish_game_init(&b,100);fish_save_init(&save,&b,100);
 assert(b.save_requested && !save.blocked && save.sequence==25 && b.weather_ms==480000 && !b.wetness_units && b.latte_remaining_ms==54321 && b.economy.bait_stock[8]==7 && b.economy.coins==4321);
 fish_save_tick(&save,&b,101);assert(save.writing);fish_save_tick(&save,&b,102);assert(save.status==FISH_SAVE_OK && write_slot<2 && !memcmp(preserved,old,sizeof(old)));
 assert(fish_save_decode(key,written,sizeof(written),&sequence,q) && sequence==26);
 a.map_index=0;
 unsigned rainy=0,clear=0,partial=0,run=0,shortest=9999,longest=0,storms=0,long_start=0,different=0;
 for(unsigned time=0;time<FISH_WEATHER_CYCLE_MS;time+=1000){
  a.weather_ms=time;unsigned rain=fish_weather_rain(&a);assert(rain<=255);
  b=a;b.map_index=1;different+=(fish_weather_rain(&b)>0)!=(rain>0);
  if(!rain){clear++;if(run){if(run<shortest)shortest=run;if(run>longest){longest=run;long_start=time-run*1000;}run=0;++storms;}}
  else {++run;if(rain==255)rainy++;else partial++;}
 }
 assert(rainy>600 && clear>rainy*2 && partial>50);
 assert(!run && storms>=10 && shortest>=119 && shortest<=120 && longest>=2879 && longest<=2880);
 assert(different>10000); /* Region schedules must not mirror one another. */
 /* A 48-hour event survives both midnights, saving/reloading and map switches;
  * a dry gap must still remove all wetness after a long storm. */
 fish_game_init(&a,0);a.weather_ms=long_start+1000;fish_weather_advance(&a,24*60000,false);
 assert(fish_weather_rain(&a)==255 && a.wetness_units==FISH_WETNESS_FULL);
 fish_save_pack(&a,p);fish_game_init(&b,0);assert(fish_save_unpack(&b,p));
 assert(b.weather_ms==a.weather_ms && fish_weather_rain(&b)==255 && fish_weather_strength(&b)==fish_weather_strength(&a));
 fish_weather_advance(&a,23*60000,false);assert(fish_weather_rain(&a)==255 && a.wetness_units==FISH_WETNESS_FULL);
 fish_weather_advance(&a,3*60000,false);assert(!fish_weather_rain(&a) && !a.wetness_units);
 a.weather_ms=500000;a.map_index=0;b=a;b.map_index=1;
 assert(fish_weather_rain(&a)==255 && !fish_weather_rain(&b));
 b.map_index=0;assert(fish_weather_rain(&b)==255 && b.weather_ms==a.weather_ms);
 /* Arriving from wetland uses the city's own schedule, including travel time. */
 a.mode=FISH_LOADING;a.travel_requested=a.travel_paid=true;a.travel_target=1;a.economy.coins=180;
 fish_game_travel_complete(&a,true);assert(a.map_index==1 && a.weather_ms==800000 && !fish_weather_rain(&a));
 a.weather_ms=13612278;assert(fish_weather_rain(&a)==255);fish_save_pack(&a,p);assert(fish_save_unpack(&b,p));
 assert(b.map_index==1 && fish_weather_rain(&b)==255);b.map_index=0;assert(!fish_weather_rain(&b));
 /* Rain scales existing nonzero weights only; it cannot add a fish through
  * a zero bait multiplier, closed habitat, retired ID or variant path. */
 for(unsigned map=0;map<2;map++)for(unsigned x=80;x<904;x+=83)for(unsigned bait=0;bait<9;bait++)for(unsigned band=0;band<3;band++)for(unsigned id=0;id<25;id++) {
  int bottom=fish_map_depth_on(map,x),depth=band==0?0:band==1?bottom/2:bottom;
  unsigned base=fish_map_weight_on(map,id,x,depth,bait,720),dry=fish_map_weather_weight_on(map,id,x,depth,bait,720,0),rain=fish_map_weather_weight_on(map,id,x,depth,bait,720,255);
  assert(base==dry);if(!base)assert(!rain);else {assert(rain && rain<=base*115/100+1 && rain+1>=base*92/100);}
  if(id!=1 && id!=7 && id!=3 && id!=6 && id!=15 && id!=20 && id!=22)assert(base==rain);
 }
 unsigned rain_counts[2][25]={{0}};
 for(unsigned trial=1;trial<=40000;trial++)for(unsigned wet=0;wet<2;wet++) {
  fish_game_init(&a,0);a.map_index=1;a.game_minute=720;a.boat_x=480;a.depth_index=0;a.random_state=trial*0x9e3779b9u;
  a.weather_ms=wet?13612278:360000;fish_game_input(&a,FISH_PRESS,0,0);fish_game_input(&a,FISH_RELEASE,0,1500);
  assert(a.pending_species<25 && !a.pending_variant);++rain_counts[wet][a.pending_species];
 }
 assert(rain_counts[1][15]>rain_counts[0][15]*103/100 && rain_counts[1][7]<rain_counts[0][7]*98/100);
 printf("40000 urban surface casts each:culter dry=%u rain=%u;white-stripe dry=%u rain=%u\n",rain_counts[0][15],rain_counts[1][15],rain_counts[0][7],rain_counts[1][7]);
 /* Store eaves and stall canopy shelter walkers, including after closing. */
 for(unsigned venue=0;venue<2;venue++) {
  fish_game_init(&a,0);a.map_index=1;a.mode=FISH_SHORE;a.game_minute=0;a.weather_ms=13612278;
  a.shore_x=venue?FISH_CITY_STALL_X:FISH_SHOP_X+18;a.wetness_units=10000;
  assert(fish_weather_rain(&a)>0 && fish_game_sheltered(&a));fish_game_update(&a,1000);assert(a.wetness_units<10000);
 }
 fish_game_init(&a,0);a.map_index=1;a.mode=FISH_SHORE;a.shore_x=-160;a.weather_ms=13612278;
 assert(!fish_game_sheltered(&a));fish_game_update(&a,1000);assert(a.wetness_units>0);
 puts("PASS:rain/weather/save/speed plus closed-store eaves and fish-stall canopy shelter");
 printf("Storms=%u, sampled duration range=%u..%u game minutes\n",storms,shortest,longest);
 printf("Weather cycle samples:rain=%u dry=%u transition=%u game minutes\n",rainy,clear,partial);
}
