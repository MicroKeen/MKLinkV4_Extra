#include "fish_game.h"
#include "fish_resources.h"
#include "fish_asset_layout.h"
#include "fish_route_view.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint8_t guarded[FISH_CACHE_BYTES+64],source[FISH_ASSET_PAYLOAD_BYTES+64];
static unsigned opened,erases,programs,last_progress,read_fail,write_fail;
static uint32_t source_size;
static bool missing;
static unsigned header_bad,truncated,corrupt_write;
static unsigned writer_inits,init_fail,common_erases,common_programs;
int fish_resource_port_init(void) {++writer_inits;return init_fail?-1:0;}
const uint8_t *fish_resource_flash(void) {return guarded+32;}
int fish_resource_open(unsigned map,uint32_t *size)
{
    if(missing)return -1;
    char p[1024];snprintf(p,sizeof(p),"%s/%s.fgpk",FISH_TEST_PACK_DIR,map==3?"common/fish":map==2?"maps/2_beach":map?"maps/1_city_river":"maps/0_wetland");
    FILE *f=fopen(p,"rb");assert(f);source_size=fread(source,1,sizeof(source),f);assert(fgetc(f)==EOF);fclose(f);
    if(header_bad) {
        unsigned at=header_bad-1;source[at]^=1;
        if(at!=28) {uint32_t crc=fish_resource_crc(~0u,source,28)^~0u;for(unsigned i=0;i<4;i++)source[28+i]=crc>>(i*8);}
    }
    source_size-=truncated;*size=source_size;opened=1;return 0;
}
int fish_resource_read(uint32_t at,void *out,unsigned n)
{if(!opened || (read_fail && at>64) || at>source_size || n>source_size-at)return -1;memcpy(out,source+at,n);return 0;}
void fish_resource_close(void) {opened=0;}
int fish_resource_erase(uint32_t at)
{assert(at%4096==0 && at<=FISH_CACHE_BYTES-4096);erases++;common_erases+=at<FISH_COMMON_CACHE_BYTES;memset(guarded+32+at,255,4096);return 0;}
int fish_resource_program(uint32_t at,const void *p,unsigned n)
{
    assert(at<=FISH_CACHE_BYTES && n<=FISH_CACHE_BYTES-at);programs++;common_programs+=at<FISH_COMMON_CACHE_BYTES;
    if(write_fail && programs>=write_fail)return -1;
    const uint8_t *s=p;for(unsigned i=0;i<n;i++){assert((guarded[32+at+i]&s[i])==s[i]);guarded[32+at+i]=s[i];}
    if(corrupt_write)guarded[32+at]^=1;return 0;
}
static void finish(void)
{
    unsigned i=0;last_progress=0;
    while(fish_resources_phase()!=FISH_RES_DONE && fish_resources_phase()!=FISH_RES_ERROR && i++<1000) {
        fish_resources_tick();unsigned p=fish_resources_progress();assert(p<=100);last_progress=p;
        for(unsigned j=0;j<32;j++)assert(guarded[j]==0xa5 && guarded[FISH_CACHE_BYTES+32+j]==0xa5);
    }
    assert(i<1000);
}
static void press(fish_game_t *g,uint32_t now) {fish_game_input(g,FISH_PRESS,0,now);fish_game_input(g,FISH_RELEASE,0,now+50);}
int main(void)
{
    memset(guarded,0xa5,sizeof(guarded));memset(guarded+32,255,FISH_CACHE_BYTES);
    /* A bad public fish file is rejected before either bank is erased. */
    const unsigned common_bad[]={0,4,8,12,16,20,24,28,32};
    for(unsigned i=0;i<sizeof(common_bad)/sizeof(common_bad[0]);i++) {
        header_bad=common_bad[i]+1;fish_resources_begin(0);finish();
        assert(fish_resources_phase()==FISH_RES_ERROR && !erases && !programs);
    }
    header_bad=0;fish_resources_begin(0);source[64]^=1;finish();
    assert(fish_resources_error()==FISH_RES_ERR_CRC && !erases && !programs);
    /* A failed second common-sector program leaves its header uncommitted;
     * retry reconstructs only validated bytes before loading a scene. */
    write_fail=2;fish_resources_begin(0);finish();
    assert(fish_resources_error()==FISH_RES_ERR_FLASH && !fish_resources_valid());
    assert(memcmp(fish_resource_flash(),"FGRP",4));write_fail=0;
    fish_resources_begin(0);assert(writer_inits==1);finish();assert(fish_resources_valid() && fish_resources_map()==0 && programs && erases && writer_inits==3);
    unsigned common_e=common_erases,common_p=common_programs;
    unsigned e=erases,p=programs;missing=true;fish_resources_begin(0);
    unsigned cached_steps=0;
    while(fish_resources_phase()==FISH_RES_CACHE_CHECK) {fish_resources_tick();++cached_steps;}
    assert(cached_steps==(FISH_ASSET_PAYLOAD_BYTES+16383)/16384);
    assert(fish_resources_phase()==FISH_RES_DONE && erases==e && programs==p && writer_inits==3);
    fish_resources_begin(1);assert(fish_resources_phase()==FISH_RES_ERROR && fish_resources_valid() && erases==e);
    missing=false;
    /* Header, CRC, layout, map, length and reserved bytes must pass before any erase. */
    const unsigned bad[]={0,4,8,12,16,20,24,28,32};
    for(unsigned i=0;i<sizeof(bad)/sizeof(*bad);i++) {
        header_bad=bad[i]+1;fish_resources_begin(1);finish();
        assert(fish_resources_phase()==FISH_RES_ERROR && erases==e && fish_resources_valid());
    }
    header_bad=0;truncated=1;fish_resources_begin(1);finish();assert(fish_resources_phase()==FISH_RES_ERROR && erases==e);truncated=0;
    fish_resources_begin(1);source[64]^=1;finish();assert(fish_resources_phase()==FISH_RES_ERROR && erases==e && fish_resources_valid());
    read_fail=1;fish_resources_begin(1);finish();assert(fish_resources_phase()==FISH_RES_ERROR && erases==e);read_fail=0;
    assert(writer_inits==3); /* Bad source and warm cache never reconfigure XPI. */
    init_fail=1;fish_resources_begin(1);finish();
    assert(fish_resources_error()==FISH_RES_ERR_FLASH && erases==e && fish_resources_valid() && writer_inits==4);
    init_fail=0;
    /* Simulated power interruption after the first resource sector: header stays invalid. */
    fish_resources_begin(1);while(fish_resources_phase()!=FISH_RES_WRITE_BLOCK)fish_resources_tick();fish_resources_tick();
    assert(!fish_resources_valid() && memcmp(fish_resource_flash()+FISH_MAP_CACHE_OFFSET,"FGRP",4));
    fish_resources_begin(0);finish();assert(fish_resources_valid() && fish_resources_map()==0);
    corrupt_write=1;fish_resources_begin(1);finish();assert(fish_resources_phase()==FISH_RES_ERROR && !fish_resources_valid());
    corrupt_write=0;fish_resources_begin(0);finish();assert(fish_resources_valid() && fish_resources_map()==0);
    write_fail=programs+2;fish_resources_begin(1);finish();assert(fish_resources_phase()==FISH_RES_ERROR && !fish_resources_valid());
    write_fail=0;fish_resources_begin(1);finish();assert(fish_resources_valid() && fish_resources_map()==1 && !opened);
    fish_resources_begin(0);finish();assert(fish_resources_valid() && fish_resources_map()==0);
    fish_game_t g;fish_game_init(&g,0);g.mode=FISH_SHORE;g.shore_x=FISH_STATION_X;g.boat_x=FISH_BOAT_RIGHT;g.economy.coins=500;g.game_minute=1320;
    press(&g,0);assert(g.mode==FISH_ROUTE && g.route_index==0);fish_game_input(&g,FISH_ROTATE,1,60);press(&g,70);
    assert(g.mode==FISH_LOADING && g.economy.coins==500);unsigned minute=g.game_minute;uint32_t rng=g.random_state;
    fish_game_update(&g,10000);assert(g.game_minute==minute);fish_game_input(&g,FISH_ROTATE,-1,10000);assert(g.mode==FISH_LOADING);
    fish_game_travel_complete(&g,true);assert(g.map_index==1 && g.economy.coins==320 && g.game_minute==(minute+300)%1440 && g.save_requested && g.random_state==rng);
    fish_game_travel_complete(&g,true);assert(g.economy.coins==320);
    press(&g,10001);fish_game_input(&g,FISH_ROTATE,-1,10070);press(&g,10080);minute=g.game_minute;
    fish_game_travel_complete(&g,false);assert(g.mode==FISH_ROUTE && g.map_index==1 && g.economy.coins==320 && g.game_minute==minute);
    press(&g,10140);fish_game_travel_complete(&g,true);assert(g.map_index==0 && g.economy.coins==140 && g.game_minute==(minute+300)%1440);
    press(&g,10210);fish_game_input(&g,FISH_ROTATE,1,10270);press(&g,10280);assert(g.mode==FISH_ROUTE && g.travel_error==1 && g.economy.coins==140);
    fish_game_input(&g,FISH_PRESS,0,10340);fish_game_update(&g,10940);assert(g.mode==FISH_SHORE && !g.pressed);fish_game_input(&g,FISH_RELEASE,0,10950);assert(g.mode==FISH_SHORE);
    /* Stations accept passengers around the clock on both maps, independently
     * of the fishing shop's 07:00-18:00 opening hours. */
    static const unsigned station_hours[]={0,359,419,420,1079,1080,1439};
    for(unsigned map=0;map<FISH_MAP_COUNT;map++)for(unsigned t=0;t<sizeof(station_hours)/sizeof(station_hours[0]);t++) {
        fish_game_init(&g,0);g.map_index=map;g.mode=FISH_SHORE;
        g.shore_x=fish_map_station(map);g.boat_x=fish_map_boat_right(map);g.game_minute=station_hours[t];
        press(&g,0);assert(g.mode==FISH_ROUTE && g.route_index==map && !g.shop_closed_notice);
    }
    /* Three stops exceed the old overview: focus each selected card and ease
     * the virtual paper camera without clipping the chosen label or thumb. */
    fish_route_view_t view={0};
    assert(fish_route_stops[2].x>fish_route_stops[1].x);
    for(unsigned selected=0;selected<FISH_MAP_COUNT;selected++) {
        fish_route_view_update(&view,false,selected,1000);
        fish_route_view_update(&view,true,selected,1000);
        int x=fish_route_stops[selected].x-view.x,y=fish_route_stops[selected].y-view.y;
        assert(x-38>=0 && x+38<=FISH_ROUTE_VIEW_WIDTH && y-58>=0 && y+23<=FISH_ROUTE_VIEW_HEIGHT);
    }
    view.x=view.from_x=0;view.y=view.from_y=0;view.selected=2;view.started_at=UINT32_MAX-30;
    fish_route_view_sample(&view,60);assert(view.x>0 && view.y>0);
    fish_route_view_sample(&view,400);
    int tx,ty;fish_route_target(2,&tx,&ty);assert(view.x==tx && view.y==ty);
    /* Real resource3 preflight/cache, fare/time and coast spawn. */
    fish_resources_begin(2);finish();assert(fish_resources_valid() && fish_resources_map()==2);
    g.map_index=1;g.mode=FISH_LOADING;g.travel_target=2;g.travel_requested=g.travel_paid=true;g.economy.coins=500;g.game_minute=720;
    fish_game_travel_complete(&g,true);
    assert(g.map_index==2 && g.shore_x==fish_map_station(2) && g.boat_x==540 && g.economy.coins==220 && g.game_minute==1020);
    assert(fish_game_at_station(&g));
    for(int x=481,prior=0;x<1440;x++) {int depth=fish_map_depth_on(2,x);assert(depth>=prior);prior=depth;}
    assert(fish_map_depth_on(2,FISH_BEACH_SHELF_EDGE_X)==1600);
    assert(fish_map_depth_on(2,960)==1200 && fish_map_depth_on(2,1200)==6500);
    assert(fish_map_depth_on(2,fish_map_boat_right(2))>10000 && fish_map_max_depth(2)==12500);
    /* Shelf slope is gentle until three quarters of the world, then sharp. */
    assert(fish_map_depth_on(2,1140)-fish_map_depth_on(2,1080)>10*(fish_map_depth_on(2,1080)-fish_map_depth_on(2,1020)));
    assert(fish_map_width(2)==1680 && fish_map_width(0)==960 && fish_map_width(1)==960);
    assert(fish_map_boat_right(2)==1608 && fish_map_depth_on(2,1440)==12500 && fish_map_depth_on(2,1608)==12500);
    assert(fish_map_dock_on(2,540)==0 && fish_map_dock_on(2,1368)<0);
    g.mode=FISH_MOVE;g.boat_x=540;g.boat_speed=0;
    fish_game_input(&g,FISH_ROTATE,-1,g.now+1);assert(g.mode==FISH_SHORE && g.shore_x==480);
    fish_game_input(&g,FISH_ROTATE,1,g.now+1);g.shore_speed=0;g.shore_x=480;
    fish_game_input(&g,FISH_ROTATE,1,g.now+1);assert(g.mode==FISH_MOVE);
    g.boat_x=1368;g.boat_speed=0;fish_game_input(&g,FISH_ROTATE,1,g.now+1);assert(g.mode==FISH_MOVE);
    for(unsigned id=0;id<FISH_COAST_FIRST;id++)assert(!fish_map_weight_on(2,id,700,0,0,720));
    assert(common_erases==common_e && common_programs==common_p);
    const unsigned fares[3][3]={{0,180,460},{180,0,280},{460,280,0}};
    for(unsigned from=0;from<3;from++)for(unsigned to=0;to<3;to++){
        unsigned fare=fares[from][to];assert(fish_game_travel_fare(from,to)==fare);if(from==to)continue;
        fish_game_init(&g,0);g.map_index=from;g.mode=FISH_ROUTE;g.route_index=to;g.economy.coins=fare-1;
        press(&g,10);assert(g.mode==FISH_ROUTE && g.travel_error==1 && g.economy.coins==fare-1);
        g.economy.coins=fare;press(&g,100);assert(g.mode==FISH_LOADING && g.economy.coins==fare);
        fish_game_travel_complete(&g,false);assert(g.mode==FISH_ROUTE && g.map_index==from && g.economy.coins==fare);
        press(&g,200);assert(g.mode==FISH_LOADING);fish_game_travel_complete(&g,true);
        assert(g.map_index==to && g.economy.coins==0);fish_game_travel_complete(&g,true);assert(g.economy.coins==0);
    }
    puts("PASS: guarded SD packs/cache/failures, six directed180/280/460G fares, exact funds, no failed-load/double charge, stations/time/input/RNG");return 0;
}
