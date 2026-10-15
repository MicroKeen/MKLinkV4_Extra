#include "fish_game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void tap(fish_game_t *g,uint32_t t) {fish_game_input(g,FISH_PRESS,0,t);fish_game_input(g,FISH_RELEASE,0,t+1);}
int main(void)
{
    fish_game_t g;fish_game_init(&g,0);g.game_minute=720;
    assert(g.rod_index==0 && g.line_index==1 && g.bait_index==FISH_PLASTIC_BAIT && g.economy.coins==0);
    assert(!fish_hold_page_count(&g.economy) && fish_hold_page_index(&g.economy,0)==FISH_SPECIES_COUNT+1);
    fish_economy_t sparse=g.economy;
    fish_hold_add(&sparse,8,200,160);fish_hold_add(&sparse,0,200,160);
    assert(fish_hold_page_count(&sparse)==2 && fish_hold_page_index(&sparse,0)==0 && fish_hold_page_index(&sparse,1)==8);
    fish_hold_add(&sparse,1,1000,fish_species_weight_g(&fish_species[1],1000));
    assert(fish_hold_page_count(&sparse)==3 && fish_hold_page_index(&sparse,2)==FISH_SPECIES_COUNT);
    g.economy=sparse;g.mode=FISH_HOLD;g.guide_index=0;
    fish_game_input(&g,FISH_ROTATE,-1,0);assert(g.guide_index==2);
    fish_game_input(&g,FISH_ROTATE,1,0);assert(g.guide_index==0);
    fish_economy_init(&g.economy);fish_game_input(&g,FISH_ROTATE,40,0);assert(g.guide_index==0);
    fish_game_init(&g,0);g.game_minute=720;
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) {
        assert(fish_catch_price(i,fish_species[i].mean_x2_mm/2)==fish_base_prices[i]);
        assert(fish_catch_price(i,fish_species[i].min_mm)<=fish_catch_price(i,fish_species[i].max_mm));
    }
    assert(fish_base_prices[2]==16 && fish_base_prices[3]==12 && fish_base_prices[8]==72 && fish_base_prices[9]==60);
    assert(fish_hold_add(&g.economy,0,200,5000)==FISH_STORED);
    assert(fish_hold_add(&g.economy,1,450,5000)==FISH_STORED && fish_hold_weight(&g.economy)==10000);
    fish_economy_t full=g.economy;
    assert(fish_hold_add(&g.economy,0,100,1)==FISH_RELEASED && !memcmp(&full,&g.economy,sizeof(full)));
    unsigned giant=1;uint16_t size=fish_species[giant].max_mm;
    uint32_t grams=fish_species_weight_g(&fish_species[giant],size);assert(grams>10000);
    assert(fish_hold_add(&g.economy,giant,size,grams)==FISH_HANGING);
    assert(fish_hold_add(&g.economy,giant,size,grams)==FISH_RELEASED);
    uint32_t revenue=fish_hold_value(&g.economy);assert(revenue>0);
    assert(fish_shop_trade(&g,0)==FISH_MERCHANT_SOLD && g.economy.coins==revenue && !fish_hold_weight(&g.economy));
    assert(g.economy.hanging_species==FISH_SPECIES_COUNT && fish_shop_trade(&g,0)==FISH_MERCHANT_EMPTY);
    g.economy.coins=0;full=g.economy;assert(fish_shop_trade(&g,2)==FISH_MERCHANT_POOR && !memcmp(&full,&g.economy,sizeof(full)));
    g.economy.coins=2000;assert(fish_shop_trade(&g,2)==FISH_MERCHANT_BOUGHT && g.rod_index==2 && g.economy.coins==200);
    assert(fish_shop_trade(&g,2)==FISH_MERCHANT_OWNED && g.economy.coins==200);
    assert(fish_shop_trade(&g,7)==FISH_MERCHANT_BOUGHT && g.economy.bait_stock[0]==5 && g.bait_index==0);
    g.economy.bait_stock[0]=999;uint32_t coins=g.economy.coins;
    assert(fish_shop_trade(&g,7)==FISH_MERCHANT_FULL && g.economy.coins==coins);
    /* Starter gear cycles only across owned items. */
    fish_game_init(&g,0);g.game_minute=720;tap(&g,1);tap(&g,3);fish_game_input(&g,FISH_ROTATE,1,5);
    assert(g.rod_index==0);tap(&g,6);g.economy.coins=1000;fish_shop_trade(&g,1);
    tap(&g,8);fish_game_input(&g,FISH_ROTATE,1,10);assert(g.rod_index==0);
    /* Special bait costs one per cast, then transparently falls back. */
    fish_game_init(&g,0);g.game_minute=720;g.bait_index=0;g.economy.bait_stock[0]=1;
    fish_game_input(&g,FISH_PRESS,0,10);fish_game_input(&g,FISH_RELEASE,0,1510);
    assert(g.economy.bait_stock[0]==0 && g.mode==FISH_CAST);
    g.mode=FISH_AIM;fish_game_input(&g,FISH_PRESS,0,1600);fish_game_input(&g,FISH_RELEASE,0,3100);
    assert(g.bait_index==FISH_PLASTIC_BAIT);
    /* Left dock -> extended shore -> shop -> hold -> shop -> path. */
    fish_game_init(&g,0);g.game_minute=720;g.mode=FISH_SHORE;g.shore_x=96;g.boat_x=132;
    fish_game_input(&g,FISH_ROTATE,-1,1);
    for(unsigned t=17;t<=11249;t+=16) fish_game_update(&g,t);
    assert(g.shore_x==FISH_SHOP_X && !g.shore_speed && fish_game_at_shop(&g) && fish_game_camera(&g)==FISH_SHORE_CAMERA_LEFT);
    int prior_camera=fish_game_camera(&g);
    for(int x=FISH_SHOP_X+1;x<=120;x++) {
        g.shore_x=x;int camera=fish_game_camera(&g);
        assert(camera>=prior_camera && camera-prior_camera<=1);prior_camera=camera;
    }
    fish_game_init(&g,0);g.game_minute=720;g.mode=FISH_SHORE;g.shore_x=FISH_SHOP_X;g.boat_x=132;
    tap(&g,3);assert(g.mode==FISH_SHOP && fish_game_ashore(&g));g.shop_index=FISH_SHOP_CARGO;tap(&g,5);
    assert(g.mode==FISH_HOLD && fish_game_ashore(&g));tap(&g,7);assert(g.mode==FISH_SHOP);
    g.shop_index=FISH_SHOP_BACK;tap(&g,9);assert(g.mode==FISH_SHORE);
    /* Larger containers retain existing fish, transfer a fitting tail catch,
     * and reject unaffordable/repeated/lower-tier purchases atomically. */
    fish_game_init(&g,0);g.game_minute=720;assert(g.economy.container==0 && fish_hold_capacity(&g.economy)==10000);
    assert(!strcmp(fish_containers[0].name,"小木桶"));full=g.economy;
    assert(fish_shop_trade(&g,12)==FISH_MERCHANT_POOR && !memcmp(&full,&g.economy,sizeof(full)));
    g.economy.coins=1050;fish_hold_add(&g.economy,1,size,grams);
    uint32_t preserved=fish_hold_value(&g.economy);
    assert(fish_shop_trade(&g,12)==FISH_MERCHANT_BOUGHT && g.economy.coins==750);
    assert(fish_hold_capacity(&g.economy)==20000 && fish_hold_weight(&g.economy)==grams);
    assert(g.economy.hanging_species==FISH_SPECIES_COUNT && fish_hold_value(&g.economy)==preserved);
    assert(fish_shop_trade(&g,12)==FISH_MERCHANT_OWNED && g.economy.coins==750);
    assert(fish_shop_trade(&g,13)==FISH_MERCHANT_BOUGHT && fish_hold_capacity(&g.economy)==35000 && g.economy.coins==0);
    full=g.economy;assert(fish_shop_trade(&g,12)==FISH_MERCHANT_OWNED && !memcmp(&full,&g.economy,sizeof(full)));
    fish_game_init(&g,0);g.game_minute=720;g.economy.coins=1050;fish_hold_add(&g.economy,0,200,5000);fish_hold_add(&g.economy,1,size,grams);
    assert(fish_shop_trade(&g,12)==FISH_MERCHANT_BOUGHT && g.economy.hanging_species==1 && fish_hold_weight(&g.economy)==5000);
    preserved=fish_hold_value(&g.economy);assert(fish_shop_trade(&g,13)==FISH_MERCHANT_BOUGHT);
    assert(g.economy.hanging_species==FISH_SPECIES_COUNT && fish_hold_weight(&g.economy)==5000+grams && fish_hold_value(&g.economy)==preserved);
    /* System monotonic milliseconds, fractional seconds, midnight, uint32 wrap. */
    fish_game_init(&g,0);fish_game_update(&g,999);assert(g.game_minute==360);
    fish_game_update(&g,1000);assert(g.game_minute==361);
    fish_game_update(&g,60000);assert(g.game_minute==420);
    g.game_minute=1439;fish_game_update(&g,61000);assert(g.game_minute==0 && fish_game_night_opacity(&g)==150);
    g.game_minute=720;assert(fish_game_night_opacity(&g)==0);
    fish_game_init(&g,UINT32_MAX-500);unsigned minute=g.game_minute;
    fish_game_update(&g,499);assert(g.game_minute==(minute+1)%1440);
    assert(fish_shop_price(1)==550 && fish_shop_price(2)==1800 && fish_shop_price(5)==90 && fish_shop_price(6)==120);
    assert(fish_shop_price(12)==300 && fish_shop_price(13)==750);
    fish_game_init(&g,0);g.mode=FISH_SHORE;g.shore_x=FISH_SHOP_X;g.game_minute=419;
    tap(&g,0);assert(g.mode==FISH_SHORE && g.merchant_reply==FISH_MERCHANT_CLOSED);
    assert(g.shop_closed_notice);fish_game_update(&g,1600);assert(g.shop_closed_notice);
    fish_game_update(&g,1601);assert(!g.shop_closed_notice);
    /* The short above-player notice also expires correctly across wrap. */
    fish_game_init(&g,UINT32_MAX-500);g.mode=FISH_SHORE;g.shore_x=FISH_SHOP_X;g.game_minute=1080;
    tap(&g,UINT32_MAX-500);assert(g.shop_closed_notice && g.mode==FISH_SHORE);
    fish_game_update(&g,1100);assert(!g.shop_closed_notice);
    fish_game_init(&g,0);g.mode=FISH_SHORE;g.shore_x=FISH_SHOP_X;
    g.game_minute=420;tap(&g,2);assert(g.mode==FISH_SHOP);
    const char *welcome=fish_merchant_says(&g);g.merchant_line_variant^=1;
    assert(strcmp(welcome,fish_merchant_says(&g)) && fish_merchant_pose(&g)<4);
    g.game_minute=1079;g.clock_at=10;g.clock_remainder_ms=0;
    fish_game_update(&g,1009);assert(g.mode==FISH_SHOP && fish_game_shop_open(&g));
    fish_game_update(&g,1010);assert(g.mode==FISH_SHORE && !fish_game_shop_open(&g));
    full=g.economy;assert(fish_shop_trade(&g,1)==FISH_MERCHANT_CLOSED && !memcmp(&full,&g.economy,sizeof(full)));
    g.mode=FISH_HOLD;g.hold_return_mode=FISH_SHOP;tap(&g,1011);assert(g.mode==FISH_SHORE);
    /* Actual casts: the glow float helps only at night, without changing
     * species/size selection or another float's waiting time. */
    for(unsigned seed=1;seed<=1000;seed++) {
        uint16_t waits[4],sizes[4];uint8_t ids[4];
        for(unsigned v=0;v<4;v++) {
            fish_game_init(&g,0);g.boat_x=400;g.random_state=seed*0x9e3779b9u;
            g.float_index=v<2?2:0;g.game_minute=v%2?0:720;
            fish_game_input(&g,FISH_PRESS,0,0);fish_game_input(&g,FISH_RELEASE,0,1500);
            waits[v]=g.bite_wait_ms;ids[v]=g.pending_species;sizes[v]=g.pending_size_mm;
        }
        assert(waits[0]==waits[2] && waits[1]<=waits[3]);
        assert(ids[0]==ids[2] && sizes[0]==sizes[2]);
        assert(ids[1]==ids[3] && sizes[1]==sizes[3]);
    }
    puts("PASS: hold capacity, one giant, release, prices, atomic sales/purchases, gear unlocks, bait, shop input, day/night and timer wrap");
}
