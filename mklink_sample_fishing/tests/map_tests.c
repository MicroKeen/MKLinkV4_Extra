#include "fish_game.h"
#include <assert.h>
#include <stdio.h>

static uint32_t time_ms;
static void rotate(fish_game_t *g,int steps) {fish_game_input(g,FISH_ROTATE,steps,++time_ms);}
static void tap(fish_game_t *g)
{
    fish_game_input(g,FISH_PRESS,0,++time_ms); fish_game_input(g,FISH_RELEASE,0,++time_ms);
}
static void item(fish_game_t *g,unsigned index)
{
    assert(g->mode==FISH_BAG && !g->bag_editing);
    for(unsigned n=0;n<FISH_BAG_COUNT && g->bag_index!=index;n++) rotate(g,1);
    assert(g->bag_index==index);tap(g);
}
int main(void)
{
    /* Smooth phase changes preserve habitat; mandarin fish is explicitly
     * neutral between day and night, while catfish retain their own phase. */
    assert(fish_map_activity_percent(8,720)==100 && fish_map_activity_percent(8,0)==100);
    assert(fish_map_activity_percent(8,360)==115);
    assert(fish_map_activity_percent(3,0)>fish_map_activity_percent(3,720));
    assert(fish_map_activity_percent(6,0)>fish_map_activity_percent(6,720));
    for(unsigned species=0;species<FISH_SPECIES_COUNT;species++) {
        unsigned previous=fish_map_activity_percent(species,1439);
        for(unsigned minute=0;minute<1440;minute++) {
            unsigned activity=fish_map_activity_percent(species,minute);
            assert(activity>=55 && activity<=155);
            assert(activity<=previous+2 && previous<=activity+2);previous=activity;
            assert(!fish_map_weight_at(species,0,0,0,minute));
        }
    }
    unsigned day_catfish=0,night_catfish=0;
    for(unsigned n=1;n<=40000;n++) {
        uint32_t day=n*0x9e3779b9u,night=day;uint8_t a,b;uint16_t sa,sb;
        assert(fish_map_roll_at(&day,480,600,5,720,&a,&sa));
        assert(fish_map_roll_at(&night,480,600,5,0,&b,&sb));
        assert(fish_map_weight(a,480,600,5) && fish_map_weight(b,480,600,5));
        day_catfish+=a==3;night_catfish+=b==3;
    }
    assert(night_catfish>day_catfish*13/10);
    printf("activity_catfish_per_40000 day=%u night=%u; mandarin own day/night weights equal\n",day_catfish,night_catfish);
    /* Rarity must survive spatial/depth/bait normalization, including lake centre. */
    unsigned max_turtle_ppm=0;
    for(int x=64;x<904;x+=16) for(unsigned bait=0;bait<FISH_BAIT_COUNT;bait++) {
        int bottom=fish_map_depth(x);unsigned sum=0;
        for(unsigned i=0;i<FISH_SPECIES_COUNT;i++)sum+=fish_map_weight(i,x,bottom,bait);
        unsigned turtle=fish_map_weight(9,x,bottom,bait);
        if(sum) {unsigned ppm=turtle*1000000u/sum;assert(ppm<10000);if(ppm>max_turtle_ppm)max_turtle_ppm=ppm;}
    }
    for(unsigned bait=0;bait<FISH_BAIT_COUNT;bait++) {
        unsigned turtle=0;uint32_t seed=4123;uint8_t id;uint16_t size;
        for(unsigned n=0;n<100000;n++){assert(fish_map_roll(&seed,480,600,bait,&id,&size));turtle+=id==9;}
        assert(turtle<1000);printf("lake_centre_bait=%u turtle_per_100000=%u\n",bait,turtle);
    }
    printf("maximum_local_turtle_ppm=%u\n",max_turtle_ppm);
    for (unsigned n=0;n<FISH_DEPTH_NODE_COUNT;n++) assert(fish_map_depth(fish_depth_profile[n].x)==fish_depth_profile[n].depth_cm);
    for (int x=-2;x<=FISH_MAP_WIDTH+2;x++) {
        int depth=fish_map_depth(x);
        assert(depth>=0 && depth<=FISH_MAX_DEPTH_CM);
        if (x>FISH_WATER_LEFT && x<FISH_WATER_RIGHT) assert(depth>0 && fish_map_zone(x)>=0);
    }
    assert(fish_map_weight(4,176,160,0)>0 && fish_map_weight(4,177,fish_map_depth(177),0)==0);
    assert(fish_map_weight(10,784,160,0)>0 && fish_map_weight(10,783,fish_map_depth(783),0)==0);
    for (unsigned bait=0;bait<FISH_BAIT_COUNT;bait++) {
        assert(fish_map_weight(4,480,600,bait)==0 && fish_map_weight(10,480,600,bait)==0);
        assert(fish_map_weight(3,480,0,bait)==0 && fish_map_weight(3,480,300,bait)==0);
        assert(fish_map_weight(3,480,524,bait)==0);
        assert((fish_map_weight(3,480,525,bait)>0)==(fish_baits[bait].weights[3]>0));
    }
    assert(fish_map_weight(0,160,140,1)>fish_map_weight(0,160,140,0));
    assert(fish_map_weight(3,480,600,1)==0 && fish_map_weight(3,480,600,2)==0);
    assert(fish_map_weight(3,480,600,3)>fish_map_weight(3,480,600,0));
    assert(fish_map_weight(1,480,600,2)>fish_map_weight(1,480,600,1));
    assert(fish_map_weight(0,160,140,3)==0 && fish_map_weight(5,480,0,1)==0);
    assert(fish_map_weight(4,160,140,4)>fish_map_weight(4,160,140,0));
    uint32_t rng=42,unchanged=rng; uint8_t id=99; uint16_t size=99;
    assert(!fish_map_roll(&rng,40,0,0,&id,&size) && rng==unchanged && id==99 && size==99);
    unsigned seen=0,samples=0;
    const int positions[]={96,160,280,480,720,800,864};
    for (unsigned p=0;p<7;p++) for (unsigned option=0;option<FISH_DEPTH_OPTION_COUNT;option++) {
        int x=positions[p],bottom=fish_map_depth(x),depth=fish_depth_options[option];
        if (depth>bottom) depth=bottom;
        for (unsigned bait=0;bait<FISH_BAIT_COUNT;bait++) for (unsigned trial=0;trial<300;trial++) {
            unsigned total=0;for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) total+=fish_map_weight(i,x,depth,bait);
            uint32_t before=rng;
            if(!total) {assert(!fish_map_roll(&rng,x,depth,bait,&id,&size) && rng==before);continue;}
            assert(fish_map_roll(&rng,x,depth,bait,&id,&size));
            assert(id<FISH_SPECIES_COUNT && fish_map_weight(id,x,depth,bait)>0);
            assert(size>=fish_species[id].min_mm && size<=fish_species[id].max_mm);
            seen|=1u<<id; ++samples;
        }
    }
    assert(seen==(1u<<FISH_BASE_SPECIES_COUNT)-1);
    fish_game_t g;
    fish_game_init(&g,0); unchanged=g.random_state;
    g.economy.rods=g.economy.lines=g.economy.floats=7;g.rod_index=1;g.bait_index=0;
    for(unsigned i=0;i<FISH_SPECIAL_BAIT_COUNT;i++) g.economy.bait_stock[i]=5;
    tap(&g); assert(g.mode==FISH_BAG && !g.bag_editing);
    item(&g,FISH_BAG_ROD); assert(g.bag_editing); rotate(&g,1); tap(&g);
    assert(g.rod_index==2 && !g.bag_editing);
    item(&g,FISH_BAG_LINE); rotate(&g,1); tap(&g);
    assert(g.line_index==2 && g.line_strength==140 && g.line_capacity_units==26000);
    item(&g,FISH_BAG_FLOAT); rotate(&g,1); tap(&g); assert(g.float_index==1);
    item(&g,FISH_BAG_BAIT); rotate(&g,3); tap(&g); assert(g.bait_index==3);
    item(&g,FISH_BAG_DEPTH); rotate(&g,1); tap(&g); assert(fish_game_rig_depth(&g)==0);
    item(&g,FISH_BAG_MISC);assert(g.mode==FISH_HOLD && g.bag_index==FISH_BAG_MISC && !g.bag_editing);
    tap(&g);assert(g.mode==FISH_BAG && !g.bag_editing);
    assert(g.random_state==unchanged); item(&g,FISH_BAG_BACK); assert(g.mode==FISH_MOVE);
    for (unsigned rod=0;rod<FISH_ROD_COUNT;rod++) {
        fish_game_init(&g,10000+rod*10000); g.boat_x=400; g.rod_index=(uint8_t)rod;
        fish_game_input(&g,FISH_PRESS,0,g.now);
        fish_game_input(&g,FISH_RELEASE,0,g.now+1500);
        assert(g.mode==FISH_CAST && g.cast_distance==fish_game_max_cast_px(&g));
        assert(g.cast_bottom_cm==fish_map_depth(g.cast_x) && g.cast_depth_cm==g.cast_bottom_cm);
        assert(fish_map_weight(g.pending_species,g.cast_x,g.cast_depth_cm,g.bait_index)>0);
        for (int facing=-1;facing<=1;facing+=2) {
            fish_game_init(&g,0);g.boat_x=480;g.rod_index=(uint8_t)rod;g.facing=facing;
            fish_game_input(&g,FISH_PRESS,0,10);fish_game_update(&g,1510);
            int camera=fish_game_camera(&g);
            fish_game_input(&g,FISH_RELEASE,0,1510);
            assert(fish_game_camera(&g)==camera);
            int landing=g.cast_x-camera; assert(landing>=4 && landing<=236);
        }
    }
    /* Casting, preview camera and physical length share the same gear limit. */
    static const unsigned line_cm[]={450,650,850,1050};
    for(unsigned rod=0;rod<FISH_ROD_COUNT;rod++)for(unsigned line=0;line<FISH_LINE_COUNT;line++)
    for(int facing=-1;facing<=1;facing+=2) {
        fish_game_init(&g,0);g.boat_x=480;g.rod_index=rod;g.facing=facing;fish_game_equip_line(&g,line);
        assert(fish_line_sets[line].distance_limit_cm==line_cm[line]);
        fish_game_input(&g,FISH_PRESS,0,0);fish_game_update(&g,1500);
        int camera=fish_game_camera(&g);fish_game_input(&g,FISH_RELEASE,0,1500);
        assert(g.mode==FISH_CAST && g.cast_distance*FISH_WORLD_CM_PER_PIXEL<=line_cm[line]);
        assert(g.cast_distance==fish_game_max_cast_px(&g) && fish_game_camera(&g)==camera);
    }
    for (unsigned dock=0;dock<2;dock++) {
        fish_game_init(&g,0); time_ms=0; g.boat_x=fish_docks[dock].boat_x+(dock?-2:2);
        rotate(&g,dock?1:-1);time_ms=251;fish_game_update(&g,time_ms);
        assert(g.mode==FISH_MOVE && !g.boat_speed && g.boat_x==fish_docks[dock].boat_x);
        rotate(&g,dock?1:-1);
        assert(g.mode==FISH_SHORE && fish_game_ashore(&g) && g.shore_x==fish_docks[dock].shore_x);
        int boat=g.boat_x;
        assert(g.boat_x==boat && fish_game_actor_x(&g)==g.shore_x);
        assert(g.shore_speed==(dock?1:-1));
        for(unsigned n=0;n<40;n++) {time_ms+=50;fish_game_update(&g,time_ms);}
        assert(g.shore_x==(dock?fish_docks[dock].shore_x+64:32));
        tap(&g); assert(g.mode==FISH_BAG); /* Away from dock. */
        item(&g,FISH_BAG_BACK); rotate(&g,dock?-40:40);
        g.shore_x=fish_docks[dock].shore_x;
        g.shore_speed=0;rotate(&g,dock?-1:1);assert(g.mode==FISH_MOVE && !fish_game_ashore(&g) && !g.boat_speed);
    }
    fish_game_init(&g,0); g.depth_index=0; g.boat_x=400;
    fish_game_input(&g,FISH_PRESS,0,10); fish_game_input(&g,FISH_RELEASE,0,1510);
    assert(g.cast_depth_cm==0 && g.pending_species!=3);
    /* One detent starts slow walking without teleporting. A button stops it,
     * menus stay still, reversing works, and clock wrap preserves distance. */
    fish_game_init(&g,0);g.mode=FISH_SHORE;g.shore_x=0;g.boat_x=132;
    fish_game_input(&g,FISH_ROTATE,-1,0);assert(g.shore_x==0 && g.shore_speed==-1);
    assert(fish_game_shore_pose(&g)==1);
    for(unsigned t=10;t<=1000;t+=10) fish_game_update(&g,t);
    assert(g.shore_x==-32 && fish_game_shore_pose(&g)==2);fish_game_input(&g,FISH_ROTATE,1,1000);assert(!g.shore_speed);fish_game_update(&g,1000);assert(g.shore_x==-32);
    fish_game_input(&g,FISH_ROTATE,1,1000);
    for(unsigned t=1010;t<=2000;t+=10) fish_game_update(&g,t);
    assert(g.shore_x==0);fish_game_input(&g,FISH_PRESS,0,2000);fish_game_input(&g,FISH_RELEASE,0,2001);
    assert(g.mode==FISH_BAG && !g.shore_speed && !fish_game_shore_pose(&g));fish_game_update(&g,9000);assert(g.shore_x==0);
    fish_game_init(&g,UINT32_MAX-499);g.mode=FISH_SHORE;g.boat_x=132;g.shore_x=0;
    fish_game_input(&g,FISH_ROTATE,-1,UINT32_MAX-499);
    for(unsigned i=1;i<=100;i++) fish_game_update(&g,(UINT32_MAX-499)+i*10u);
    assert(g.shore_x==-32);
    fish_game_init(&g,0);g.mode=FISH_SHORE;g.boat_x=132;g.shore_x=96;g.shore_speed=-1;
    fish_game_input(&g,FISH_ROTATE,1,0);assert(g.mode==FISH_SHORE && !g.shore_speed);
    fish_game_input(&g,FISH_ROTATE,1,0);assert(g.mode==FISH_MOVE && !g.boat_speed);
    fish_game_init(&g,0);g.mode=FISH_SHORE;g.shore_x=FISH_SHOP_X;g.boat_x=132;
    fish_game_input(&g,FISH_ROTATE,1,0);
    for(unsigned t=16;t<=12000;t+=16) fish_game_update(&g,t);
    assert(g.shore_x==96 && !g.shore_speed && fish_game_dock_action(&g)==0);
    /* East shore extends to the station without moving the lake or changing
     * the stop-at-dock, then rotate-again boarding gesture. */
    fish_game_init(&g,0);g.boat_x=FISH_BOAT_RIGHT;
    fish_game_input(&g,FISH_ROTATE,1,0);
    assert(g.mode==FISH_SHORE && g.shore_x==fish_docks[1].shore_x && g.shore_speed==1);
    int prior_camera=fish_game_camera(&g);
    for(unsigned t=16;t<=16000;t+=16) {
        fish_game_update(&g,t);int camera=fish_game_camera(&g);
        assert(camera>=prior_camera && camera-prior_camera<=1);prior_camera=camera;
        assert(g.shore_x<=FISH_STATION_X && g.boat_x==FISH_BOAT_RIGHT);
    }
    assert(g.shore_x==FISH_STATION_X && !g.shore_speed && fish_game_at_station(&g));
    assert(fish_game_camera(&g)==FISH_SHORE_CAMERA_RIGHT);
    fish_game_input(&g,FISH_ROTATE,1,16000);
    for(unsigned t=16016;t<=18000;t+=16) fish_game_update(&g,t);
    assert(g.shore_x==FISH_SHORE_RIGHT && !g.shore_speed);
    fish_game_input(&g,FISH_ROTATE,-1,18000);
    for(unsigned t=18016;t<=20000;t+=16) fish_game_update(&g,t);
    assert(g.shore_x==FISH_STATION_X && !g.shore_speed);
    fish_game_input(&g,FISH_ROTATE,-1,20000);
    for(unsigned t=20016;t<=36000;t+=16) fish_game_update(&g,t);
    assert(g.shore_x==fish_docks[1].shore_x && !g.shore_speed && fish_game_dock_action(&g)==1);
    fish_game_input(&g,FISH_ROTATE,-1,36000);assert(g.mode==FISH_MOVE && !g.boat_speed);
    g.boat_x=FISH_BOAT_RIGHT;assert(fish_game_camera(&g)==FISH_BOAT_RIGHT-120);
    assert(fish_map_depth(FISH_STATION_X)==0 && fish_map_zone(FISH_STATION_X)<0);
    assert(fish_floats[1].strike_window_bonus_percent==15 && fish_floats[2].night_bite_bonus_percent==15);
    puts("PASS: continuous virtual terrain, shoreline/bottom exclusions, bait weighting, no fallback, all species reachable");
    printf("PASS: %u local weighted draws, equipment UI state, rods/line-hook sets/depth, both docks/walking/reboarding\n",samples);
}
