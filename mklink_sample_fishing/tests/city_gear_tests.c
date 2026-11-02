#include "fish_save.h"
#include "fish_save_crypto.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int fish_storage_init(uint8_t *k){(void)k;return -1;}
int fish_storage_read(unsigned s,uint8_t *p,size_t n){(void)s;(void)p;(void)n;return -2;}
int fish_storage_begin(unsigned s,const uint8_t *p,size_t n){(void)s;(void)p;(void)n;return -1;}
int fish_storage_poll(void){return 0;}
static void store(fish_game_t *g,unsigned map,unsigned venue,unsigned coins)
{fish_game_init(g,0);g->map_index=(uint8_t)map;g->shop_venue=(uint8_t)venue;g->mode=FISH_SHOP;g->game_minute=720;g->economy.coins=coins;}
static void fight(fish_game_t *g,unsigned rod)
{
    fish_game_init(g,0);g->map_index=1;g->mode=FISH_FIGHT;g->pending_species=17;g->pending_weight_g=30000;
    g->rod_index=(uint8_t)rod;fish_game_equip_line(g,FISH_PE_LINE);g->line_response_percent=45;
    g->base_pull=40;g->struggle_percent=100;g->stamina=500;g->stamina_max=1000;g->tension=40;
    g->cast_distance=180;g->cast_start_x=492;g->cast_x=672;
}
int main(void)
{
    fish_game_t g,a,b,loaded;uint8_t p[FISH_SAVE_PAYLOAD_BYTES],q[sizeof(p)],file[FISH_SAVE_FILE_BYTES],key[32]={1,7};uint64_t seq;
    assert(FISH_SAVE_FILE_BYTES==2147 && FISH_SAVE_V19_FILE_BYTES==2062 && FISH_SAVE_V18_FILE_BYTES==2037 && FISH_SAVE_V17_FILE_BYTES==1511 && FISH_ROD_COUNT==5);
    store(&g,0,0,10000);assert(fish_shop_trade(&g,FISH_SHOP_LONG_ROD)==FISH_MERCHANT_EMPTY);
    store(&g,1,FISH_VENDOR_CONVENIENCE,10000);assert(fish_shop_trade(&g,FISH_SHOP_LONG_ROD)==FISH_MERCHANT_EMPTY);
    store(&g,1,FISH_VENDOR_TACKLE,3499);assert(fish_shop_trade(&g,FISH_SHOP_LONG_ROD)==FISH_MERCHANT_POOR && g.economy.coins==3499 && g.rod_index==0);
    ++g.economy.coins;assert(fish_shop_trade(&g,FISH_SHOP_LONG_ROD)==FISH_MERCHANT_BOUGHT && !g.economy.coins && g.rod_index==FISH_LONG_ROD && (g.economy.rods&8) && g.save_requested);
    assert(fish_shop_trade(&g,FISH_SHOP_LONG_ROD)==FISH_MERCHANT_OWNED);
    assert(fish_shop_item_at(&g,3)==FISH_SHOP_LONG_ROD && fish_shop_item_at(&g,4)==3);
    for(unsigned line=0;line<FISH_LINE_COUNT;line++)for(int facing=-1;facing<=1;facing+=2) {
        fish_game_init(&a,0);a.mode=FISH_AIM;a.map_index=1;a.rod_index=FISH_LONG_ROD;a.boat_x=480;a.facing=facing;fish_game_equip_line(&a,line);
        unsigned limit=fish_line_sets[line].distance_limit_cm/4;assert(fish_game_max_cast_px(&a)==(limit<180?limit:180));
        fish_game_input(&a,FISH_PRESS,0,0);fish_game_input(&a,FISH_RELEASE,0,1500);
        assert(a.mode==FISH_CAST && a.cast_distance==fish_game_max_cast_px(&a));
        assert(a.cast_distance*4<=fish_line_sets[line].distance_limit_cm);
    }
    fight(&a,2);fight(&b,FISH_LONG_ROD);fish_game_input(&a,FISH_ROTATE,1,0);fish_game_input(&b,FISH_ROTATE,1,0);
    assert(b.reel_units>a.reel_units && b.tension+1==a.tension && b.mode==FISH_FIGHT);
    fight(&a,2);fight(&b,FISH_LONG_ROD);a.pending_weight_g=b.pending_weight_g=1000;
    fish_game_input(&a,FISH_ROTATE,1,0);fish_game_input(&b,FISH_ROTATE,1,0);assert(a.tension==b.tension);
    for(unsigned grams=300;grams<=1500;grams+=600) {
        fight(&a,2);fight(&b,FISH_LONG_ROD);a.pending_weight_g=b.pending_weight_g=grams;
        a.tension=b.tension=0;fish_game_update(&a,20);fish_game_update(&b,20);
        assert(a.slack_ms==20 && b.slack_ms==(grams<=500?30:grams<1500?20+(1500-grams)/100:20));
    }
    /* Correct venue, exact prices, no duplicate charges and no effect on failed trades. */
    store(&g,0,0,10000);assert(fish_shop_trade(&g,FISH_SHOP_UMBRELLA)==FISH_MERCHANT_EMPTY && fish_shop_trade(&g,FISH_SHOP_NOODLES)==FISH_MERCHANT_EMPTY);
    store(&g,1,FISH_VENDOR_TACKLE,10000);assert(fish_shop_trade(&g,FISH_SHOP_UMBRELLA)==FISH_MERCHANT_EMPTY);
    store(&g,1,FISH_VENDOR_CONVENIENCE,159);assert(fish_shop_trade(&g,FISH_SHOP_UMBRELLA)==FISH_MERCHANT_POOR && !g.comforts);
    ++g.economy.coins;assert(fish_shop_trade(&g,FISH_SHOP_UMBRELLA)==FISH_MERCHANT_BOUGHT && !g.economy.coins && (g.comforts&FISH_COMFORT_UMBRELLA));
    assert(fish_shop_trade(&g,FISH_SHOP_UMBRELLA)==FISH_MERCHANT_OWNED && !g.economy.coins);
    for(unsigned map=0;map<2;map++)for(unsigned land=0;land<2;land++) {
        fish_game_init(&a,0);a.map_index=(uint8_t)map;a.mode=land?FISH_SHORE:FISH_MOVE;a.weather_ms=map?13612278:500000;a.comforts=FISH_COMFORT_UMBRELLA;b=a;b.comforts=0;
        fish_weather_advance(&a,20000,false);fish_weather_advance(&b,20000,false);
        assert(!a.wetness_units && b.wetness_units>0);
        a.wetness_units=60000;fish_weather_advance(&a,20000,false);assert(a.wetness_units==40000);
    }
    store(&g,1,FISH_VENDOR_CONVENIENCE,27);assert(fish_shop_trade(&g,FISH_SHOP_NOODLES)==FISH_MERCHANT_POOR && !g.latte_remaining_ms);
    g.economy.coins=10000;g.comforts=FISH_COMFORT_UMBRELLA;
    assert(fish_shop_trade(&g,FISH_SHOP_NOODLES)==FISH_MERCHANT_NOODLES && g.economy.coins==9972 && g.latte_remaining_ms==240000);
    assert(fish_game_refresh_percent(&g)==110 && fish_game_line_speed_percent(&g)==110);
    g.wetness_units=FISH_WETNESS_FULL;assert(fish_game_line_speed_percent(&g)==88);g.wetness_units=0;
    g.merchant_reply=FISH_MERCHANT_NOODLES;assert(strstr(fish_merchant_says(&g),"面"));
    fish_game_update(&g,1000);assert(g.latte_remaining_ms==239000);
    assert(fish_shop_trade(&g,FISH_SHOP_LATTE)==FISH_MERCHANT_LATTE && fish_game_refresh_percent(&g)==110 && !(g.comforts&FISH_COMFORT_NOODLES) && (g.comforts&FISH_COMFORT_UMBRELLA));
    assert(fish_shop_trade(&g,FISH_SHOP_NOODLES)==FISH_MERCHANT_NOODLES && fish_game_refresh_percent(&g)==110 && g.latte_remaining_ms==240000);
    /* Noodles do not change any strike timing; latte still extends by10%. */
    fish_game_init(&a,0);a.mode=FISH_BITE;a.pending_species=0;a.bite_wait_ms=1000;a.phase_at=0;a.float_index=0;fish_game_equip_line(&a,1);
    b=a;b.comforts=FISH_COMFORT_NOODLES;b.latte_remaining_ms=240000;
    unsigned end_a=0,end_b=0;
    for(unsigned t=0;t<2000;t++){a.now=b.now=t;if(fish_game_strike_ready(&a))end_a=t+1;if(fish_game_strike_ready(&b))end_b=t+1;}
    assert(end_b==end_a);
    b.comforts=0;end_b=0;
    for(unsigned t=0;t<2000;t++){b.now=t;if(fish_game_strike_ready(&b))end_b=t+1;}
    unsigned delay=fish_species[0].strike_delay_ms;assert(end_b-delay==(end_a-delay)*110/100);
    /* Saved equipment/umbrella/meal preserve current location, time and weather. */
    g.rod_index=FISH_LONG_ROD;g.economy.rods|=8;g.latte_remaining_ms=123456;
    g.mode=FISH_SHORE;g.shore_x=FISH_SHOP_X;g.boat_x=FISH_BOAT_LEFT;g.facing=-1;g.game_minute=1261;g.weather_ms=13612278;
    fish_save_pack(&g,p);fish_save_encode(key,21,p,file);assert(fish_save_decode(key,file,sizeof(file),&seq,q) && seq==21);
    fish_game_init(&loaded,0);assert(fish_save_unpack(&loaded,q));
    assert(loaded.rod_index==FISH_LONG_ROD && loaded.economy.rods==g.economy.rods && loaded.comforts==g.comforts && loaded.latte_remaining_ms==123456);
    assert(loaded.shore_x==g.shore_x && loaded.facing==-1 && loaded.game_minute==1261 && loaded.weather_ms==g.weather_ms);
    /* Authenticate an actual V13 record: no new item is invented on migration. */
    fish_game_init(&g,0);g.map_index=1;g.game_minute=840;g.weather_ms=13612278;g.economy.coins=999;g.latte_remaining_ms=123456;
    fish_save_pack(&g,p);p[187]=25;fish_save_encode(key,11,p,file);file[4]=13;
    file[6]=FISH_SAVE_V14_PAYLOAD_BYTES&255;file[7]=FISH_SAVE_V14_PAYLOAD_BYTES>>8;fish_save_seal(key,file,16,p,FISH_SAVE_V14_PAYLOAD_BYTES,file+16,file+32);
    assert(fish_save_decode(key,file,FISH_SAVE_V14_FILE_BYTES,&seq,q) && seq==11 && fish_save_unpack(&loaded,q));
    assert(!loaded.comforts && loaded.economy.coins==999 && loaded.latte_remaining_ms==123456 && loaded.rod_index==0);
    store(&g,1,FISH_VENDOR_CONVENIENCE,100);fish_shop_trade(&g,FISH_SHOP_NOODLES);
    fish_game_update(&g,240000);assert(!g.latte_remaining_ms && !(g.comforts&FISH_COMFORT_NOODLES));
    store(&g,1,FISH_VENDOR_CONVENIENCE,100);fish_shop_trade(&g,FISH_SHOP_NOODLES);
    g.mode=FISH_LOADING;g.travel_requested=g.travel_paid=true;g.travel_target=0;g.economy.coins=180;
    fish_game_travel_complete(&g,true);assert(!g.latte_remaining_ms && !(g.comforts&FISH_COMFORT_NOODLES));
    puts("PASS: city3500G rod/casting/tension,160G permanent umbrella,28G4h noodles/replace/expiry/travel, V15 roundtrip and V13 compatibility;1511B saves");
}
