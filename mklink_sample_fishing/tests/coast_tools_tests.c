#include "fish_game.h"
#include "fish_save.h"
#include "fish_save_crypto.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int fish_storage_init(uint8_t *k){(void)k;return -1;}
int fish_storage_read(unsigned s,uint8_t *p,size_t n){(void)s;(void)p;(void)n;return -2;}
int fish_storage_begin(unsigned s,const uint8_t *p,size_t n){(void)s;(void)p;(void)n;return -1;}
int fish_storage_poll(void){return 0;}
static void setup(fish_game_t *g,unsigned line,unsigned boat,unsigned depth)
{
    fish_game_init(g,0);g->map_index=2;g->boat_x=boat;g->shore_x=72;g->game_minute=720;
    g->rod_index=FISH_LONG_ROD;g->economy.rods|=8;g->economy.lines|=(uint8_t)(1u<<line);
    g->comforts=FISH_COMFORT_UMBRELLA;fish_game_equip_line(g,line);g->depth_index=depth;
}
static void cast(fish_game_t *g)
{fish_game_input(g,FISH_PRESS,0,0);fish_game_input(g,FISH_RELEASE,0,1500);assert(g->mode==FISH_CAST);}
int main(void)
{
    assert(fish_species[27].rarity==FISH_RARITY_GOLD);
    unsigned day=720,night=1320,x=1320,bottom=fish_map_depth_on(2,x);
    for(unsigned id=34;id<=45;id+=11){
        assert(!fish_map_weight_on(2,id,x,0,4,day));
        assert(fish_map_weight_on(2,id,x,0,4,night));
        assert(fish_map_weight_on(2,id,x,bottom/2,4,day));
        unsigned ordinary=fish_map_lamp_weight_on(2,id,x,0,4,night,0,FISH_MARINE_LINE,false);
        assert(fish_map_lamp_weight_on(2,id,x,0,4,night,0,FISH_MARINE_LINE,true)==ordinary*4);
        assert(!fish_map_lamp_weight_on(2,id,x,0,4,day,0,FISH_MARINE_LINE,true));
        assert(!fish_map_lamp_weight_on(2,id,x,-1,4,night,0,FISH_MARINE_LINE,true));
    }
    assert(fish_map_lamp_weight_on(2,34,600,0,4,night,0,0,true)>0);
    assert(!fish_map_lamp_weight_on(2,34,600,0,4,night,0,0,false));
    assert(fish_map_weight_on(2,FISH_CHUB_MACKEREL,x,0,3,night)>0);
    unsigned mackerel=fish_map_lamp_weight_on(2,FISH_CHUB_MACKEREL,x,0,3,night,0,0,false);
    assert(fish_map_lamp_weight_on(2,FISH_CHUB_MACKEREL,x,0,3,night,0,0,true)==mackerel*4);
    assert(fish_map_lamp_weight_on(2,FISH_CHUB_MACKEREL,x,0,3,day,0,0,true)==fish_map_lamp_weight_on(2,FISH_CHUB_MACKEREL,x,0,3,day,0,0,false));
    fish_game_t g,a,b;setup(&g,FISH_MARINE_LINE,1100,FISH_DEPTH_OPTION_COUNT-1);
    g.mode=FISH_SHOP;g.economy.coins=20000;
    const unsigned expected[]={0,FISH_SHOP_HEAVY_ROD,FISH_SHOP_PE_LINE,FISH_SHOP_MARINE_LINE,FISH_SHOP_LAMP,FISH_SHOP_SHOVEL,FISH_SHOP_CROWBAR,FISH_SHOP_CABIN,FISH_SHOP_CARGO,FISH_SHOP_BACK};
    assert(fish_shop_count(&g)==sizeof(expected)/sizeof(*expected));
    for(unsigned i=0;i<fish_shop_count(&g);i++)assert(fish_shop_item_at(&g,i)==expected[i]);
    assert(fish_shop_trade(&g,1)==FISH_MERCHANT_EMPTY && g.economy.coins==20000);
    assert(fish_shop_trade(&g,FISH_SHOP_HEAVY_ROD)==FISH_MERCHANT_BOUGHT && g.rod_index==FISH_HEAVY_SEA_ROD);
    g.economy.lines&=(uint8_t)~(1u<<FISH_MARINE_LINE);
    assert(fish_shop_trade(&g,FISH_SHOP_MARINE_LINE)==FISH_MERCHANT_BOUGHT && g.line_index==FISH_MARINE_LINE);
    assert(fish_shop_trade(&g,FISH_SHOP_LAMP)==FISH_MERCHANT_BOUGHT && g.economy.coins==10000);
    assert((g.comforts&FISH_COMFORT_LAMP) && !(g.comforts&FISH_COMFORT_LAMP_ON));
    g.mode=FISH_BAG;g.bag_index=FISH_BAG_LAMP;
    assert(fish_game_bag_count(&g)==12 && fish_game_bag_item_at(&g,6)==FISH_BAG_LAMP);
    fish_game_input(&g,FISH_PRESS,0,100);fish_game_input(&g,FISH_RELEASE,0,101);
    g.game_minute=night;assert(fish_game_lamp_active(&g));g.map_index=1;assert(!fish_game_lamp_active(&g) && fish_game_bag_count(&g)==11);g.map_index=2;
    uint8_t p[FISH_SAVE_PAYLOAD_BYTES],q[FISH_SAVE_PAYLOAD_BYTES],file[FISH_SAVE_FILE_BYTES],key[32]={1};uint64_t seq;
    g.mode=FISH_MOVE;fish_save_pack(&g,p);fish_save_encode(key,4,p,file);
    assert(sizeof(file)==2147 && FISH_SAVE_V19_FILE_BYTES==2062 && FISH_SAVE_V18_FILE_BYTES==2037 && fish_save_decode(key,file,sizeof(file),&seq,q));fish_game_init(&a,0);
    assert(fish_save_unpack(&a,q) && a.rod_index==FISH_HEAVY_SEA_ROD && a.line_index==FISH_MARINE_LINE && fish_game_lamp_active(&a) && !a.economy.cabin_owned);
    p[187]=FISH_SAVE_V18_SPECIES_COUNT;file[4]=19;file[6]=FISH_SAVE_V18_PAYLOAD_BYTES&255;file[7]=FISH_SAVE_V18_PAYLOAD_BYTES>>8;
    fish_save_seal(key,file,16,p,FISH_SAVE_V18_PAYLOAD_BYTES,file+16,file+32);
    assert(fish_save_decode(key,file,FISH_SAVE_V18_FILE_BYTES,&seq,q));fish_game_init(&a,0);
    assert(fish_save_unpack(&a,q) && fish_game_lamp_active(&a) && a.economy.hanging_species==FISH_SPECIES_COUNT);
    b=g;b.rod_index=FISH_LONG_ROD;b.line_index=FISH_PE_LINE;b.economy.rods=15;b.economy.lines=15;b.comforts=FISH_COMFORT_UMBRELLA;
    fish_save_pack(&b,p);fish_save_encode(key,3,p,file);file[4]=18;file[6]=FISH_SAVE_V18_PAYLOAD_BYTES&255;file[7]=FISH_SAVE_V18_PAYLOAD_BYTES>>8;p[187]=FISH_SAVE_V18_SPECIES_COUNT;fish_save_seal(key,file,16,p,FISH_SAVE_V18_PAYLOAD_BYTES,file+16,file+32);
    assert(fish_save_decode(key,file,FISH_SAVE_V18_FILE_BYTES,&seq,q));fish_game_init(&a,0);assert(fish_save_unpack(&a,q) && a.economy.hanging_species==FISH_SPECIES_COUNT && a.rod_index==FISH_LONG_ROD && !(a.comforts&FISH_COMFORT_LAMP));
    b.economy.cabin_owned=true;assert(fish_hold_add_on(&b.economy,2,FISH_CHUB_MACKEREL,400,700)==FISH_CABIN_STORED);
    fish_save_pack(&b,p);fish_save_encode(key,5,p,file);assert(fish_save_decode(key,file,sizeof(file),&seq,q));
    fish_game_init(&a,0);assert(fish_save_unpack(&a,q) && a.economy.cabin_count[FISH_CHUB_MACKEREL-FISH_COAST_FIRST]==1 && fish_cabin_weight(&a.economy)==700);
    unsigned old_price=b.economy.cabin_value[FISH_CHUB_MACKEREL-FISH_COAST_FIRST]*3;
    b.economy.cabin_value[FISH_CHUB_MACKEREL-FISH_COAST_FIRST]=old_price;b.economy.prices[FISH_PORTABLE_PRICE_GROUPS].price=(uint16_t)old_price;
    assert(old_price>fish_catch_price_on(2,FISH_CHUB_MACKEREL,500));
    fish_save_pack(&b,p);fish_game_init(&a,0);assert(fish_save_unpack(&a,p) && a.economy.cabin_value[FISH_CHUB_MACKEREL-FISH_COAST_FIRST]==old_price);
    setup(&g,2,1200,FISH_DEPTH_OPTION_COUNT-1);cast(&g);assert(g.short_line && !g.cast_depth_cm && !g.depth_index);
    setup(&g,FISH_PE_LINE,1200,1);cast(&g);assert(g.short_line && !g.cast_depth_cm);
    setup(&g,FISH_PE_LINE,540,FISH_DEPTH_OPTION_COUNT-1);g.rod_index=1;cast(&g);
    assert(g.cast_bottom_cm<1500 && g.cast_depth_cm && !g.short_line);
    setup(&g,FISH_MARINE_LINE,1250,FISH_DEPTH_OPTION_COUNT-1);cast(&g);assert(!g.short_line && g.cast_depth_cm>10000);
    fish_game_update(&g,2151);assert(g.mode==FISH_SINK && !g.rig_depth_cm && g.pending_species==FISH_SPECIES_COUNT);
    fish_game_update(&g,7151);assert(g.mode==FISH_SINK && g.rig_depth_cm==1000 && g.pending_species==FISH_SPECIES_COUNT);
    unsigned finish=g.phase_at+(g.cast_depth_cm*1000+FISH_MARINE_SINK_CM_PER_S-1)/FISH_MARINE_SINK_CM_PER_S+1;
    fish_game_update(&g,finish);assert(g.mode==FISH_WAIT && g.rig_depth_cm==g.cast_bottom_cm);
    fish_game_input(&g,FISH_PRESS,0,finish+1);fish_game_input(&g,FISH_RELEASE,0,finish+2);assert(g.mode==FISH_RECOVER);
    unsigned last=fish_game_current_depth_cm(&g);fish_game_update(&g,finish+1002);assert(g.mode==FISH_RECOVER && fish_game_current_depth_cm(&g)<last && g.reel_units>0);
    fish_game_update(&g,finish+100000);assert(g.mode==FISH_AIM && !g.cast_depth_cm);
    setup(&a,FISH_MARINE_LINE,1000,1);a.mode=FISH_FIGHT;a.cast_start_x=100;a.cast_x=175;a.cast_distance=75;a.cast_depth_cm=400;a.cast_bottom_cm=12000;a.reel_units=0;
    assert(fish_game_distance_cm(&a)==500 && fish_game_current_depth_cm(&a)==400);a.reel_units=5000;assert(fish_game_distance_cm(&a)==250 && fish_game_current_depth_cm(&a)==200);
    a.map_index=1;assert(fish_game_distance_cm(&a)==250);
    setup(&a,FISH_MARINE_LINE,1000,1);a.mode=FISH_RECOVER;a.cast_start_x=100;a.cast_x=175;a.cast_depth_cm=12000;a.cast_bottom_cm=12500;
    a.line_capacity_units=fish_game_distance_cm(&a);b=a;b.rod_index=FISH_HEAVY_SEA_ROD;
    unsigned full=a.line_capacity_units;fish_game_update(&a,1000);fish_game_update(&b,1000);
    assert(full-a.line_capacity_units==375 && full-b.line_capacity_units==450);
    a.rod_index=FISH_LONG_ROD;b.rod_index=FISH_HEAVY_SEA_ROD;
    assert(fish_game_max_cast_px(&b)*100==fish_game_max_cast_px(&a)*70 && fish_rods[b.rod_index].reel_percent*100==fish_rods[a.rod_index].reel_percent*120);
    unsigned photo[2]={0};uint64_t waits[2]={0};
    for(unsigned seed=1;seed<=5000;seed++)for(unsigned light=0;light<2;light++){
        setup(&g,0,540,0);g.game_minute=1320;g.random_state=seed*0x9e3779b9u;
        if(light)g.comforts|=FISH_COMFORT_LAMP|FISH_COMFORT_LAMP_ON;
        cast(&g);fish_game_update(&g,2151);assert(g.mode==FISH_WAIT);
        if(fish_map_light_target(g.pending_species)){++photo[light];waits[light]+=g.bite_wait_ms;}
    }
    assert(photo[1]>photo[0] && waits[1]*photo[0]*100<waits[0]*photo[1]*85);
    printf("5000 night casts: light-target catches off/on=%u/%u; mean target wait_ms=%llu/%llu\n",photo[0],photo[1],(unsigned long long)(waits[0]/photo[0]),(unsigned long long)(waits[1]/photo[1]));
    /* Expanded coast coordinates must round-trip as signed values. The old
     * water geometry and positive V19 positions remain valid without moving
     * a saved boat or changing any fishing distribution. */
    assert(sizeof(fish_dock_t)==12 && fish_map_dock_at(2,0)->walk_left<0);
    static const int shores[]={FISH_BEACH_WALK_LEFT,FISH_BEACH_STATION_X,-216,FISH_BEACH_STALL_X,0,72,224,320,416,480};
    for(unsigned i=0;i<sizeof(shores)/sizeof(shores[0]);i++){
        setup(&g,FISH_MARINE_LINE,540,0);g.mode=FISH_SHORE;g.shore_x=(int16_t)shores[i];
        uint8_t saved[FISH_SAVE_PAYLOAD_BYTES];fish_save_pack(&g,saved);fish_game_init(&a,0);
        assert(fish_save_unpack(&a,saved) && a.map_index==2 && a.shore_x==shores[i] && a.boat_x==540);
        assert(fish_game_camera(&a)>=FISH_BEACH_CAMERA_LEFT && fish_game_camera(&a)<=FISH_BEACH_CAMERA_RIGHT);
    }
    setup(&g,FISH_MARINE_LINE,540,0);g.mode=FISH_SHORE;g.shore_x=FISH_SHOP_X-24;
    fish_game_input(&g,FISH_ROTATE,1,0);for(unsigned t=16;t<1600;t+=16)fish_game_update(&g,t);
    assert(g.shore_x>FISH_SHOP_X && g.shore_speed>0); /* no invisible wetland shop stop */
    setup(&g,FISH_MARINE_LINE,540,0);g.mode=FISH_SHORE;g.shore_x=224;
    fish_forage_update(&g);g.forage.taken=4095; /* isolated movement check */
    fish_game_input(&g,FISH_ROTATE,1,0);
    for(unsigned t=16;t<=9000;t+=16)fish_game_update(&g,t);
    assert(g.mode==FISH_SHORE && g.shore_x==480 && !g.shore_speed);
    fish_game_input(&g,FISH_ROTATE,1,g.now+1);assert(g.mode==FISH_MOVE && g.boat_x==540);
    puts("PASS: coastal equipment/depth/light/save plus signed extended shore, open tidal walk and unchanged boarding geometry");
}
