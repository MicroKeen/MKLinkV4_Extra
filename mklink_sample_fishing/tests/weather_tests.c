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
int main(void)
{
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
 fish_save_pack(&a,p);fish_save_encode(key,12,p,file);assert(sizeof(file)==1503);
 assert(fish_save_decode(key,file,sizeof(file),&sequence,q));fish_game_init(&b,0);assert(fish_save_unpack(&b,q));
 assert(b.weather_ms==a.weather_ms && b.wetness_units==a.wetness_units && b.map_index==1 && b.latte_remaining_ms==54321 && b.economy.bait_stock[8]==7);
 put32(q+FISH_SAVE_WEATHER_OFFSET,FISH_WEATHER_CYCLE_MS);assert(!fish_save_unpack(&b,q));memcpy(q,p,sizeof(q));put32(q+FISH_SAVE_WETNESS_OFFSET,FISH_WETNESS_FULL+1);assert(!fish_save_unpack(&b,q));
 /* Real authenticated V11 prefix, including latte/stock. Boot finds its older
  * slot, migrates into V12_A/B, and never modifies the old file. */
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
 puts("PASS:2..48-hour rain events across midnights/maps/saves,visual strength swells,60-minute wetting/120-minute drying,exact partition/large gaps,20% line-speed loss,latte/travel/shelter and authenticated V11 migration");
 printf("Storms=%u, sampled duration range=%u..%u game minutes\n",storms,shortest,longest);
 printf("Weather cycle samples:rain=%u dry=%u transition=%u game minutes\n",rainy,clear,partial);
}
