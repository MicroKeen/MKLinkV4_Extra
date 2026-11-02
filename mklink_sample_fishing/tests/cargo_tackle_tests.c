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

static void cast(fish_game_t *g)
{
    g->mode=FISH_AIM;g->pressed=false;
    uint32_t t=g->now+1;
    fish_game_input(g,FISH_PRESS,0,t);fish_game_input(g,FISH_RELEASE,0,t+1500);
}
int main(void)
{
    fish_game_t g,loaded;uint8_t payload[FISH_SAVE_PAYLOAD_BYTES];
    const fish_species_t *blue=fish_species_profile(1,17);
    unsigned mm=blue->min_mm;
    while(fish_species_weight_g(blue,(uint16_t)mm)<11000)++mm;
    uint32_t grams=fish_species_weight_g(blue,(uint16_t)mm);
    assert(grams>=11000 && grams<11100);
    /* The reported35kg-box case, the20kg upgrade and the starter bucket.
     * Real fish/price records must survive validation, reload and sale. */
    for(unsigned container=0;container<FISH_CONTAINER_COUNT;container++) {
        fish_game_init(&g,0);g.map_index=1;g.game_minute=720;g.economy.container=(uint8_t)container;
        unsigned fill=container==3?29:container==2?11:container==1?6:0;
        for(unsigned i=0;i<fill;i++)assert(fish_hold_add_on(&g.economy,1,0,450,2500)==FISH_STORED);
        uint32_t stored=fish_hold_weight(&g.economy);
        assert(fish_hold_add_on(&g.economy,1,17,(uint16_t)mm,grams)==FISH_HANGING);
        assert(g.economy.hanging_species==17 && g.economy.hanging_grams==grams && fish_hold_weight(&g.economy)==stored);
        fish_economy_t before=g.economy;
        assert(fish_hold_add_on(&g.economy,1,17,(uint16_t)mm,grams)==FISH_RELEASED);
        assert(!memcmp(&before,&g.economy,sizeof(before)));
        fish_save_pack(&g,payload);fish_game_init(&loaded,0);assert(fish_save_unpack(&loaded,payload));
        assert(loaded.economy.hanging_species==17 && loaded.economy.hanging_map==1 && loaded.economy.hanging_grams==grams);
        uint32_t value=fish_hold_value(&loaded.economy);
        assert(fish_shop_trade(&loaded,0)==FISH_MERCHANT_SOLD && loaded.economy.coins==value);
        assert(loaded.economy.hanging_species==FISH_SPECIES_COUNT);
    }
    fish_game_init(&g,0);g.map_index=1;g.economy.container=2;
    assert(fish_hold_add_on(&g.economy,1,17,(uint16_t)mm,grams)==FISH_STORED);
    assert(g.economy.hanging_species==FISH_SPECIES_COUNT);
    /* City replaces the wooden bucket with a portable80kg upgrade. Historical
     * cargo and hanging-fish prices must survive moving into the bigger box. */
    for(unsigned old=0;old<3;old++){
        fish_game_init(&g,0);g.map_index=1;g.mode=FISH_SHOP;g.shop_venue=FISH_VENDOR_TACKLE;g.game_minute=720;g.economy.container=old;
        assert(fish_shop_position(&g,12)==fish_shop_count(&g) && fish_shop_position(&g,13)<fish_shop_position(&g,FISH_SHOP_LARGE_BOX));
        assert(fish_shop_price(FISH_SHOP_LARGE_BOX)==1800 && fish_containers[3].capacity_g==80000);
        assert(fish_hold_add_on(&g.economy,1,0,200,160)==FISH_STORED);g.economy.value[0]=17;g.economy.prices[0]=(fish_price_group_t){17,1};
        assert(fish_hold_add_on(&g.economy,1,17,1411,40025)==FISH_HANGING);g.economy.hanging_value=777;
        g.economy.coins=1799;fish_economy_t before=g.economy;
        assert(fish_shop_trade(&g,FISH_SHOP_LARGE_BOX)==FISH_MERCHANT_POOR && !memcmp(&before,&g.economy,sizeof(before)));
        g.economy.coins=1800;assert(fish_shop_trade(&g,FISH_SHOP_LARGE_BOX)==FISH_MERCHANT_BOUGHT);
        assert(g.economy.container==3 && !g.economy.coins && fish_hold_weight(&g.economy)==40185 && fish_hold_value(&g.economy)==794 && g.economy.hanging_species==FISH_SPECIES_COUNT);
        assert(fish_shop_trade(&g,FISH_SHOP_LARGE_BOX)==FISH_MERCHANT_OWNED);
        fish_save_pack(&g,payload);fish_game_init(&loaded,0);assert(fish_save_unpack(&loaded,payload) && loaded.economy.container==3 && fish_hold_value(&loaded.economy)==794);
        g.map_index=0;assert(fish_shop_trade(&g,FISH_SHOP_LARGE_BOX)==FISH_MERCHANT_EMPTY && fish_hold_capacity(&g.economy)==80000);
        assert(fish_shop_position(&g,12)<fish_shop_count(&g));
        g.map_index=2;assert(fish_shop_trade(&g,FISH_SHOP_LARGE_BOX)==FISH_MERCHANT_EMPTY && fish_hold_capacity(&g.economy)==80000);
    }
    fish_game_init(&g,0);g.economy.container=3;assert(fish_hold_add_on(&g.economy,1,17,1411,40000)==FISH_STORED);
    assert(fish_hold_add_on(&g.economy,1,17,1411,40000)==FISH_STORED && fish_hold_weight(&g.economy)==80000);
    assert(fish_hold_add_on(&g.economy,1,0,80,1)==FISH_RELEASED);
    /* The larger box must preserve money even if the fixed price ledger fills. */
    fish_game_init(&g,0);g.map_index=1;g.game_minute=720;g.shore_x=FISH_CITY_STALL_X;
    g.economy.container=3;uint32_t historical_total=0;
    for(unsigned i=0;i<FISH_PORTABLE_PRICE_GROUPS;i++){
        g.economy.prices[i]=(fish_price_group_t){(uint16_t)(1000+i),1};historical_total+=1000+i;
    }
    g.economy.count[0]=FISH_PORTABLE_PRICE_GROUPS;g.economy.grams[0]=22400;g.economy.value[0]=historical_total;
    uint32_t extra_price=fish_catch_price_on(1,17,1411);assert(extra_price<1000);
    assert(fish_hold_add_on(&g.economy,1,17,1411,40000)==FISH_STORED && fish_hold_weight(&g.economy)==62400);
    fish_save_pack(&g,payload);fish_game_init(&loaded,0);assert(fish_save_unpack(&loaded,payload));
    assert(fish_hold_value(&loaded.economy)==historical_total+extra_price);
    uint32_t before_coins=loaded.economy.coins;
    assert(fish_shop_trade(&loaded,0)==FISH_MERCHANT_SOLD && loaded.economy.coins==before_coins+historical_total+extra_price);
    /* Authentic V21 migrates the same payload without assigning the new item. */
    uint8_t key[32]={7},file[FISH_SAVE_FILE_BYTES],out[FISH_SAVE_PAYLOAD_BYTES];uint64_t seq;
    for(unsigned old=0;old<3;old++){
        fish_game_init(&g,0);g.economy.container=old;g.economy.coins=12345;fish_save_pack(&g,payload);fish_save_encode(key,77,payload,file);file[4]=21;
        fish_save_seal(key,file,16,payload,sizeof(payload),file+16,file+32);
        assert(fish_save_decode(key,file,sizeof(file),&seq,out));fish_game_init(&loaded,0);assert(fish_save_unpack(&loaded,out) && loaded.economy.container==old && loaded.economy.coins==12345);
        fish_save_encode(key,78,out,file);assert(file[4]==22 && FISH_SAVE_FILE_BYTES==2147);
    }
    /* The casting path cannot bypass rod/rig compatibility or consume bait. */
    for(unsigned rod=0;rod<FISH_ROD_COUNT;rod++)for(unsigned line=0;line<FISH_LINE_COUNT;line++) {
        fish_game_init(&g,0);g.rod_index=(uint8_t)rod;g.economy.rods|=(uint8_t)(1u<<rod);
        g.economy.lines=15;fish_game_equip_line(&g,line);g.bait_index=0;g.economy.bait_stock[0]=3;
        cast(&g);bool allowed=rod || line<2;
        assert((g.mode==FISH_CAST)==allowed && g.economy.bait_stock[0]==(allowed?2:3));
    }
    fish_game_init(&g,0);g.mode=FISH_BAG;g.bag_editing=true;g.bag_index=FISH_BAG_LINE;g.economy.lines=15;
    fish_game_input(&g,FISH_ROTATE,1,0);assert(g.line_index==0);
    fish_game_input(&g,FISH_ROTATE,-1,0);assert(g.line_index==1);
    g.rod_index=1;g.economy.rods=15;fish_game_equip_line(&g,2);g.bag_index=FISH_BAG_ROD;
    fish_game_input(&g,FISH_ROTATE,-1,0);assert(g.rod_index==0 && g.line_index==1 && g.economy.lines==15);
    g.map_index=1;g.game_minute=720;g.economy.coins=1000;g.economy.lines=2;
    assert(fish_shop_trade(&g,4)==FISH_MERCHANT_BOUGHT && (g.economy.lines&4) && g.line_index==1);
    /* Old bamboo+strong saves retain every item and select an owned valid rig. */
    fish_game_equip_line(&g,2);fish_save_pack(&g,payload);fish_game_init(&loaded,0);
    assert(fish_save_unpack(&loaded,payload) && loaded.rod_index==0 && loaded.line_index==1 && loaded.economy.lines==6);
    g.economy.lines=4;fish_save_pack(&g,payload);fish_game_init(&loaded,0);
    assert(fish_save_unpack(&loaded,payload) && loaded.economy.lines==4);cast(&loaded);assert(loaded.mode!=FISH_CAST);
    /* Three visible modes, with historical fixed-depth IDs still readable. */
    fish_game_init(&g,0);g.mode=FISH_BAG;g.bag_index=FISH_BAG_DEPTH;g.bag_editing=true;
    unsigned expected[]={0,1,FISH_DEPTH_OPTION_COUNT-1};
    for(unsigned i=0;i<6;i++){fish_game_input(&g,FISH_ROTATE,1,0);assert(g.depth_index==expected[i%3]);}
    for(unsigned map=0;map<2;map++)for(unsigned depth=0;depth<FISH_DEPTH_OPTION_COUNT;depth++) {
        fish_game_init(&g,0);g.map_index=(uint8_t)map;g.boat_x=480;g.depth_index=(uint8_t)depth;
        cast(&g);assert(g.mode==FISH_CAST);
        unsigned actual=fish_line_resolve_layer(g.line_index,g.cast_bottom_cm,
            abs(g.cast_x-g.cast_start_x)*FISH_WORLD_CM_PER_PIXEL,!depth?0:depth==FISH_DEPTH_OPTION_COUNT-1?2:1);
        unsigned want=actual==2?g.cast_bottom_cm:actual?g.cast_bottom_cm/2:0;
        assert(g.cast_depth_cm==want);
        fish_save_pack(&g,payload);fish_game_init(&loaded,0);assert(fish_save_unpack(&loaded,payload));
        assert(loaded.depth_index==g.depth_index);
    }
    /* Existing stock becomes permanent ownership; new purchase is600G once. */
    fish_game_init(&g,0);g.map_index=1;g.game_minute=720;g.shop_venue=FISH_VENDOR_TACKLE;g.economy.coins=599;
    assert(fish_shop_price(FISH_SHOP_LIFELIKE)==600 && fish_shop_trade(&g,FISH_SHOP_LIFELIKE)==FISH_MERCHANT_POOR);
    g.economy.coins=1000;assert(fish_shop_trade(&g,FISH_SHOP_LIFELIKE)==FISH_MERCHANT_BOUGHT && g.economy.coins==400 && g.economy.bait_stock[8]==1);
    assert(fish_shop_trade(&g,FISH_SHOP_LIFELIKE)==FISH_MERCHANT_OWNED && g.economy.coins==400);
    for(unsigned i=0;i<20;i++){cast(&g);assert(g.mode==FISH_CAST && g.bait_index==8 && g.economy.bait_stock[8]==1);}
    g.economy.bait_stock[8]=9;fish_save_pack(&g,payload);fish_game_init(&loaded,0);assert(fish_save_unpack(&loaded,payload));
    cast(&loaded);assert(loaded.bait_index==8 && loaded.economy.bait_stock[8]==9);
    loaded.economy.bait_stock[8]=0;cast(&loaded);assert(loaded.bait_index==FISH_PLASTIC_BAIT);
    printf("PASS:35kg-box11kg blue carp stern/save/sale (%ug), all rods/rigs, three depth modes and600G permanent lure\n",grams);
}
