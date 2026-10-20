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
static unsigned writer_inits,init_fail;
int fish_resource_port_init(void) {++writer_inits;return init_fail?-1:0;}
const uint8_t *fish_resource_flash(void) {return guarded+32;}
int fish_resource_open(unsigned map,uint32_t *size)
{
    if(missing)return -1;
    char p[1024];snprintf(p,sizeof(p),"%s/maps/%s.fgpk",FISH_TEST_PACK_DIR,map?"1_city_river":"0_wetland");
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
{assert(at%4096==0 && at<=FISH_CACHE_BYTES-4096);erases++;memset(guarded+32+at,255,4096);return 0;}
int fish_resource_program(uint32_t at,const void *p,unsigned n)
{
    assert(at<=FISH_CACHE_BYTES && n<=FISH_CACHE_BYTES-at);programs++;
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
    fish_resources_begin(0);assert(!writer_inits);finish();assert(fish_resources_valid() && fish_resources_map()==0 && programs && erases && writer_inits==1);
    unsigned e=erases,p=programs;missing=true;fish_resources_begin(0);
    unsigned cached_steps=0;
    while(fish_resources_phase()==FISH_RES_CACHE_CHECK) {fish_resources_tick();++cached_steps;}
    assert(cached_steps==(FISH_ASSET_PAYLOAD_BYTES+16383)/16384);
    assert(fish_resources_phase()==FISH_RES_DONE && erases==e && programs==p && writer_inits==1);
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
    assert(writer_inits==1); /* Bad source and warm cache never reconfigure XPI. */
    init_fail=1;fish_resources_begin(1);finish();
    assert(fish_resources_error()==FISH_RES_ERR_FLASH && erases==e && fish_resources_valid() && writer_inits==2);
    init_fail=0;
    /* Simulated power interruption after the first resource sector: header stays invalid. */
    fish_resources_begin(1);while(fish_resources_phase()!=FISH_RES_WRITE_BLOCK)fish_resources_tick();fish_resources_tick();
    assert(!fish_resources_valid() && memcmp(fish_resource_flash(),"FGRP",4));
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
        g.shore_x=FISH_STATION_X;g.boat_x=FISH_BOAT_RIGHT;g.game_minute=station_hours[t];
        press(&g,0);assert(g.mode==FISH_ROUTE && g.route_index==map && !g.shop_closed_notice);
    }
    /* Current stops and their larger illustrations fit together. Selection
     * must not move this compact overview or clip either label/current mark. */
    fish_route_view_t view={0};
    for(unsigned selected=0;selected<FISH_MAP_COUNT;selected++) {
        fish_route_view_update(&view,true,selected,1000+selected*100);
        assert(view.x==222 && view.y==119);
        for(unsigned stop=0;stop<FISH_MAP_COUNT;stop++) {
            int x=fish_route_stops[stop].x-view.x,y=fish_route_stops[stop].y-view.y;
            assert(x-38>=0 && x+38<=FISH_ROUTE_VIEW_WIDTH);
            assert(y-58>=0 && y+23<=FISH_ROUTE_VIEW_HEIGHT);
        }
    }
    fish_route_view_update(&view,true,0,1500);assert(view.x==222 && view.y==119);
    fish_route_view_update(&view,false,0,1600);assert(!view.active);
    fish_route_view_update(&view,true,1,1700);assert(view.x==222 && view.y==119);
    /* Retain bounded easing for future networks, including timer wrap. */
    view.x=view.from_x=0;view.y=view.from_y=0;view.started_at=2000;
    fish_route_view_sample(&view,2090);assert(view.x>0 && view.x<222 && view.y>0 && view.y<119);
    fish_route_view_sample(&view,2360);assert(view.x==222 && view.y==119);
    view.x=view.from_x=0;view.y=view.from_y=0;view.started_at=UINT32_MAX-30;
    fish_route_view_sample(&view,60);assert(view.x>0 && view.x<222 && view.y>0 && view.y<119);
    fish_route_view_sample(&view,400);assert(view.x==222 && view.y==119);
    assert(fish_route_focus(0,640,224)==0 && fish_route_focus(640,640,224)==416);
    puts("PASS: guarded real SD packs, cache reuse, CRC/read/write failures, interruption/retry, both-way fare/time, 24-hour stations, compact route overview, input, funds and RNG");return 0;
}
