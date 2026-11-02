#include "fish_save.h"
#include "fish_save_crypto.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int fish_storage_init(uint8_t *k){(void)k;return -1;}
int fish_storage_read(unsigned s,uint8_t *p,size_t n){(void)s;(void)p;(void)n;return -2;}
int fish_storage_begin(unsigned s,const uint8_t *p,size_t n){(void)s;(void)p;(void)n;return -1;}
int fish_storage_poll(void){return 0;}
static void visit(fish_game_t *g)
{
    fish_game_init(g,0);g->map_index=1;g->mode=FISH_SHORE;g->shore_x=FISH_CAT_X;
    for(unsigned m=720;m<1440;m+=60){g->game_minute=(uint16_t)m;g->weather_ms=m*1000;if(fish_cat_present(g))break;}
    assert(fish_cat_present(g));fish_cat_update(g);
}
static void small_catch(fish_game_t *g)
{
    g->mode=FISH_CAUGHT;g->last_species=0;g->last_map=0;g->last_size_mm=200;
    g->last_weight_g=fish_species_weight_g(&fish_species[0],200);
    g->species_catches[0]=1;g->species_best_size_mm[0]=200;g->catches=1;
    g->cargo_result=fish_hold_add_on(&g->economy,0,0,200,g->last_weight_g);g->cat.menu=0;
}
int main(void)
{
    fish_game_t g,b;uint8_t p[FISH_SAVE_PAYLOAD_BYTES],q[FISH_SAVE_PAYLOAD_BYTES],file[FISH_SAVE_FILE_BYTES],key[32]={1};uint64_t sequence;
    visit(&g);uint32_t rng=g.random_state;
    unsigned present=0,absent=0;
    for(unsigned day=0;day<7;day++)for(unsigned minute=0;minute<1440;minute+=15){
        b=g;b.cat.flags=0;b.game_minute=(uint16_t)minute;b.weather_ms=(day*1440+minute)*1000;
        bool seen=fish_cat_present(&b);if(minute<720 || minute%60>=30)assert(!seen);
        if(minute>=720){present+=seen;absent+=!seen;}
    }
    assert(present && absent && g.random_state==rng);
    /* Visits span the complete saved calendar and do not consume fishing RNG. */
    unsigned hourly=0;
    for(unsigned day=0;day<96;day++)for(unsigned hour=12;hour<24;hour++){
        b=g;b.cat.flags=0;b.game_minute=hour*60;b.weather_ms=(day*1440+hour*60)*1000;
        hourly+=fish_cat_present(&b);
        b.game_minute+=30;assert(!fish_cat_present(&b));
        b.map_index=0;assert(!fish_cat_present(&b));b.map_index=2;assert(!fish_cat_present(&b));
    }
    assert(hourly>96*12*60/100 && hourly<96*12*70/100 && g.random_state==rng);
    printf("CAT_VISITS,%u,1152\n",hourly);
    /* Visible visits and absent streets both remain stable while watched. */
    visit(&b);b.cat.actions=3;b.game_minute+=45;b.weather_ms=b.game_minute*1000;
    fish_cat_update(&b);assert(fish_cat_present(&b) && b.cat.actions==3);
    b.cat.walk_x=-7;b.cat.flags=FISH_CAT_READY|FISH_CAT_VISIBLE|FISH_CAT_WALK_LEFT;
    fish_save_pack(&b,p);fish_game_t restored;fish_game_init(&restored,123);assert(fish_save_unpack(&restored,p));
    assert(fish_cat_present(&restored) && restored.cat.walk_x==-7 && restored.cat.actions==3);
    restored.mode=FISH_MOVE;fish_cat_update(&restored);assert(!fish_cat_present(&restored));
    visit(&b);b.cat.flags=FISH_CAT_READY;fish_cat_update(&b);assert(!fish_cat_present(&b));
    b.mode=FISH_MOVE;fish_cat_update(&b);assert(fish_cat_present(&b));
    /* Full56px stroll, both directions, resting and menu pause/resume. */
    visit(&b);b.shore_x=FISH_CAT_X-80;int min=0,max=0;unsigned left=0,right=0,rest=0;
    for(unsigned t=100;t<24000;t+=100){b.now=t;fish_cat_update(&b);int x=b.cat.walk_x;if(x<min)min=x;if(x>max)max=x;
        unsigned state=b.cat.flags&FISH_CAT_STROLL_MASK;left+=state==FISH_CAT_WALK_LEFT;right+=state==FISH_CAT_WALK_RIGHT;rest+=fish_cat_pose(&b,false)>=3;}
    assert(min==-28 && max==28 && left && right && rest);
    int held=b.cat.walk_x;b.mode=FISH_CAT;b.now+=5000;fish_cat_update(&b);
    assert(b.cat.walk_x==held && fish_cat_pose(&b,false)>=3);
    b.mode=FISH_SHORE;b.now+=100;fish_cat_update(&b);assert(abs(b.cat.walk_x-held)<=1);
    /* A walker yields to the player, pauses, then turns away without crossing
     * the actor. The next frame resumes actual travel rather than foot shuffling. */
    visit(&b);b.shore_x=FISH_CAT_X+30;b.now=100;fish_cat_update(&b);
    assert((b.cat.flags&FISH_CAT_BLOCKED) && !b.cat.walk_x);
    b.now=2099;fish_cat_update(&b);assert(!b.cat.walk_x);
    b.now=2100;fish_cat_update(&b);
    assert(!(b.cat.flags&FISH_CAT_BLOCKED) && (b.cat.flags&FISH_CAT_STROLL_MASK)==FISH_CAT_WALK_LEFT);
    b.now=2200;fish_cat_update(&b);assert(b.cat.walk_x==-1);
    g.cat.tins=4;fish_cat_interact(&g,0);assert(g.cat.friendship==1 && g.cat.actions==1 && g.cat.reply==FISH_CAT_PETTED);
    fish_cat_interact(&g,1);fish_cat_interact(&g,1);
    assert(g.cat.friendship==11 && g.cat.actions==3 && g.cat.feeds==2 && g.cat.tins==2);
    fish_cat_interact(&g,0);assert(g.cat.friendship==11 && g.cat.reply==FISH_CAT_TIRED);
    fish_save_pack(&g,p);fish_game_init(&b,0);assert(fish_save_unpack(&b,p));
    assert(b.cat.friendship==11 && b.cat.actions==3 && b.cat.feeds==2 && b.cat.encounter==g.cat.encounter);
    fish_cat_interact(&b,1);assert(b.cat.tins==2 && b.cat.friendship==11);
    g.weather_ms+=1440000;fish_cat_update(&g);assert(fish_cat_feeds_left(&g)==2 && !g.cat.feeds);
    visit(&g);g.cat.friendship=49;fish_cat_interact(&g,0);
    assert(fish_cat_owned(&g) && !fish_cat_present(&g) && g.cat.reply==FISH_CAT_JOINED);
    unsigned sleep=0,sit=0;
    for(unsigned t=2000;t<40000;t+=100){g.now=t;unsigned pose=fish_cat_pose(&g,true);assert(pose>=3 && pose<9);sleep+=pose==6 || pose==7;sit+=pose==3 || pose==4;}
    assert(sleep && sit);
    /* Adoption follows all six directed routes, persists after reloading in
     * each destination, and survives failed map loads without charging again. */
    for(unsigned from=0;from<3;from++)for(unsigned to=0;to<3;to++)if(from!=to){
        fish_game_init(&b,0);b.map_index=from;b.mode=FISH_LOADING;b.travel_target=to;
        b.travel_requested=b.travel_paid=true;b.economy.coins=1000;
        b.game_minute=780;b.weather_ms=780000;b.cat.friendship=50;b.cat.tins=7;b.cat.feeds=1;b.cat.day=0;
        b.cat.flags=FISH_CAT_READY|FISH_CAT_VISIBLE;
        fish_game_t failed=b;fish_game_travel_complete(&failed,false);fish_cat_update(&failed);
        assert(failed.map_index==from && fish_cat_owned(&failed) && failed.economy.coins==1000 && failed.cat.feeds==1);
        unsigned fare=fish_game_travel_fare(from,to);fish_game_travel_complete(&b,true);fish_cat_update(&b);
        assert(b.map_index==to && fish_cat_owned(&b) && b.economy.coins==1000-fare && b.cat.feeds==1 && b.cat.tins==7);
        assert(!fish_cat_present(&b) && !(b.cat.flags&FISH_CAT_VISIBLE));
        fish_save_pack(&b,p);fish_game_init(&restored,123);assert(fish_save_unpack(&restored,p));
        fish_cat_update(&restored);assert(restored.map_index==to && fish_cat_owned(&restored) && restored.cat.feeds==1 && restored.cat.tins==7);
        for(unsigned t=2000;t<20000;t+=1000){restored.now=t;unsigned pose=fish_cat_pose(&restored,true);assert(pose>=3 && pose<=7);}
    }
    /* Actual encoder dispatch: one press opens the cat, rotation selects food,
     * and press/release consumes one can without opening the backpack. */
    visit(&g);g.cat.tins=3;
    fish_game_input(&g,FISH_PRESS,0,0);fish_game_input(&g,FISH_RELEASE,0,20);
    assert(g.mode==FISH_CAT);
    fish_game_input(&g,FISH_ROTATE,1,21);
    fish_game_input(&g,FISH_PRESS,0,22);fish_game_input(&g,FISH_RELEASE,0,42);
    assert(g.mode==FISH_CAT && g.cat.friendship==5 && g.cat.tins==2 && g.cat.feeds==1);
    /* Midnight resets the shared limit; five hours of travel cross midnight
     * exactly once and preserve the adopted companion. */
    fish_game_init(&g,0);g.cat.friendship=50;g.cat.feeds=2;g.cat.day=0;
    g.game_minute=1439;g.weather_ms=1439000;fish_game_update(&g,1000);
    assert(g.game_minute==0 && fish_cat_day(&g)==1 && !g.cat.feeds);
    g.game_minute=1320;g.weather_ms=2760000;g.cat.day=1;g.cat.feeds=2;
    g.mode=FISH_LOADING;g.travel_requested=g.travel_paid=true;g.travel_target=1;g.economy.coins=180;
    fish_game_travel_complete(&g,true);fish_cat_update(&g);
    assert(g.map_index==1 && g.game_minute==180 && fish_cat_day(&g)==2 && !g.cat.feeds && fish_cat_owned(&g));
    /* Canned food and caught fish share the daily limit across both maps. */
    fish_game_init(&g,0);g.cat.friendship=50;g.cat.feeds=1;g.cat.day=(uint8_t)fish_cat_day(&g);
    small_catch(&g);small_catch(&g);assert(g.economy.count[0]==2 && fish_cat_catch_menu(&g));
    assert(fish_cat_feed_catch(&g) && g.economy.count[0]==1 && g.cat.feeds==2 && g.cat.friendship==50 && g.cargo_result==FISH_FED_CAT);
    fish_save_pack(&g,p);fish_game_init(&b,0);assert(fish_save_unpack(&b,p) && b.cat.feeds==2 && b.economy.count[0]==1);
    g.map_index=1;small_catch(&g);fish_economy_t before=g.economy;
    assert(!fish_cat_feed_catch(&g) && !memcmp(&before,&g.economy,sizeof(before)) && g.cat.reply==FISH_CAT_FULL);
    /* The disabled third feeding also stays on the caught card through real
     * input, and choosing to keep the fish exits without a second deduction. */
    g.cat.menu=1;fish_game_input(&g,FISH_PRESS,0,0);fish_game_input(&g,FISH_RELEASE,0,20);
    assert(g.mode==FISH_CAUGHT && !memcmp(&before,&g.economy,sizeof(before)));
    fish_game_input(&g,FISH_ROTATE,-1,21);fish_game_input(&g,FISH_PRESS,0,22);fish_game_input(&g,FISH_RELEASE,0,42);
    assert(g.mode==FISH_AIM && g.cat.feeds==2 && !memcmp(&before,&g.economy,sizeof(before)));
    g.last_species=23;g.last_weight_g=100;assert(!fish_cat_catch_menu(&g));
    /* A full container can still hand the just-caught small fish to the cat. */
    fish_game_init(&g,0);g.cat.friendship=50;g.economy.count[0]=1;g.economy.grams[0]=10000;g.economy.value[0]=8;
    small_catch(&g);assert(g.cargo_result==FISH_RELEASED && fish_cat_feed_catch(&g) && g.economy.grams[0]==10000 && g.cat.feeds==1);
    fish_game_init(&g,0);g.map_index=1;g.shop_venue=FISH_VENDOR_CONVENIENCE;g.game_minute=720;g.economy.coins=24;
    assert(fish_shop_price(FISH_SHOP_CAT_FOOD)==25 && fish_shop_trade(&g,FISH_SHOP_CAT_FOOD)==FISH_MERCHANT_POOR);
    g.economy.coins=100;assert(fish_shop_trade(&g,FISH_SHOP_CAT_FOOD)==FISH_MERCHANT_BOUGHT && g.cat.tins==1 && g.economy.coins==75);
    g.cat.tins=999;assert(fish_shop_trade(&g,FISH_SHOP_CAT_FOOD)==FISH_MERCHANT_FULL && g.economy.coins==75);
    g.cat.friendship=50;g.cat.feeds=2;g.cat.actions=3;g.cat.encounter=20;
    fish_save_pack(&g,p);fish_save_encode(key,99,p,file);assert(fish_save_decode(key,file,sizeof(file),&sequence,q));
    fish_game_init(&b,0);assert(fish_save_unpack(&b,q) && b.cat.tins==999 && b.cat.friendship==50 && b.cat.feeds==2);
    /* Original V15 cat bytes remain readable with no visibility flags. */
    file[4]=15;file[6]=FISH_SAVE_V17_PAYLOAD_BYTES&255;file[7]=FISH_SAVE_V17_PAYLOAD_BYTES>>8;p[187]=25;fish_save_seal(key,file,16,p,FISH_SAVE_V17_PAYLOAD_BYTES,file+16,file+32);
    assert(fish_save_decode(key,file,FISH_SAVE_V17_FILE_BYTES,&sequence,q));fish_game_init(&b,0);
    assert(fish_save_unpack(&b,q) && b.cat.friendship==50 && b.cat.feeds==2 && !b.cat.flags);
    /* Authenticated V14 migration defaults only the new cat fields. */
    file[4]=14;file[6]=FISH_SAVE_V14_PAYLOAD_BYTES&255;file[7]=FISH_SAVE_V14_PAYLOAD_BYTES>>8;
    fish_save_seal(key,file,16,p,FISH_SAVE_V14_PAYLOAD_BYTES,file+16,file+32);
    assert(fish_save_decode(key,file,FISH_SAVE_V14_FILE_BYTES,&sequence,q));
    fish_game_init(&b,0);assert(fish_save_unpack(&b,q) && !b.cat.friendship && !b.cat.tins && b.economy.coins==75);
    unsigned normal=0,with_cat=0;
    for(unsigned i=1;i<=400000;i++) {
        uint32_t a=i*0x9e3779b9u,before_rng=a,b_rng=a;
        normal+=fish_species_special_roll(&a,1)!=1;with_cat+=fish_species_special_roll_cat(&b_rng,1)!=1;
        uint32_t no_variant=before_rng;assert(fish_species_special_roll_cat(&no_variant,17)==17 && no_variant==before_rng);
    }
    assert(normal>3600 && normal<4400 && with_cat>9300 && with_cat<10700);
    printf("PASS:65%% hourly visits, six directed three-map routes and failed loads, destination saves, shared feeds, stationary boat poses; variants/400000 normal=%u cat=%u\n",normal,with_cat);
}
