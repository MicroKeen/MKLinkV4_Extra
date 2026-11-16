#include "fish_game.h"
#include "fish_save.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
int fish_storage_init(uint8_t *k){(void)k;return -1;}
int fish_storage_read(unsigned i,uint8_t *p,size_t n){(void)i;(void)p;(void)n;return -2;}
int fish_storage_begin(unsigned i,const uint8_t *p,size_t n){(void)i;(void)p;(void)n;return -1;}
int fish_storage_poll(void){return 0;}
int main(void)
{
    assert(fish_line_sets[2].distance_limit_cm==2000 && fish_line_sets[FISH_PE_LINE].distance_limit_cm==2500);
    assert(fish_line_bottom_allowed(0,499) && !fish_line_bottom_allowed(0,500));
    assert(fish_line_bottom_allowed(1,999) && !fish_line_bottom_allowed(1,1000));
    assert(fish_line_bottom_allowed(2,1600) && fish_line_bottom_allowed(FISH_PE_LINE,2000));
    assert(fish_line_resolve_layer(0,400,300,2)==1); /* 5m diagonal exceeds4.5m */
    assert(fish_line_resolve_layer(1,900,400,2)==1);
    assert(fish_line_resolve_layer(1,1600,400,2)==0);
    assert(fish_line_resolve_layer(2,3000,400,2)==1);
    assert(fish_line_resolve_layer(2,6500,400,2)==0);
    assert(fish_line_resolve_layer(FISH_PE_LINE,2200,300,2)==2);
    unsigned casts=0,old_bottom=0,mid=0,surface=0;
    for(unsigned map=0;map<3;map++)for(int boat=fish_map_boat_left(map);boat<=fish_map_boat_right(map);boat+=31)
    for(unsigned line=0;line<FISH_LINE_COUNT;line++)for(unsigned request=0;request<3;request++)for(int facing=-1;facing<=1;facing+=2){
        fish_game_t g;fish_game_init(&g,0);g.map_index=map;g.boat_x=boat;g.facing=facing;
        g.rod_index=FISH_LONG_ROD;g.economy.rods|=8;g.economy.lines|=1u<<line;fish_game_equip_line(&g,line);
        g.depth_index=request==2?FISH_DEPTH_OPTION_COUNT-1:request;
        fish_game_input(&g,FISH_PRESS,0,0);fish_game_input(&g,FISH_RELEASE,0,1500);assert(g.mode==FISH_CAST);
        unsigned horizontal=abs(g.cast_x-g.cast_start_x)*FISH_WORLD_CM_PER_PIXEL;
        unsigned actual=g.depth_index==FISH_DEPTH_OPTION_COUNT-1?2:g.depth_index?1:0;
        assert(actual<=request && g.short_line==(actual!=request));
        assert(fish_line_span_cm(horizontal,g.cast_depth_cm)<=fish_game_line_limit_cm(&g));
        if(actual==2)assert(fish_line_bottom_allowed(line,g.cast_bottom_cm) && g.cast_depth_cm==g.cast_bottom_cm);
        if(actual==1)assert(g.cast_depth_cm==g.cast_bottom_cm/2);
        if(!actual)assert(!g.cast_depth_cm);
        if(map<2 && line>=2 && request==2){assert(actual==2 && !g.short_line);old_bottom++;}
        if(actual<request){mid+=actual==1;surface+=!actual;}
        uint8_t saved[FISH_SAVE_PAYLOAD_BYTES];fish_save_pack(&g,saved);fish_game_t loaded;fish_game_init(&loaded,0);
        assert(fish_save_unpack(&loaded,saved) && loaded.depth_index==g.depth_index && loaded.line_index==line);
        casts++;
    }
    assert(old_bottom && mid && surface);
    /* The appended far-right screen remains reachable, castable and saveable. */
    for(unsigned line=FISH_PE_LINE;line<=FISH_MARINE_LINE;line++){
        fish_game_t g,loaded;fish_game_init(&g,0);g.map_index=2;g.boat_x=1607;g.facing=1;g.boat_speed=2;
        fish_game_update(&g,1000);assert(g.boat_x==1608 && !g.boat_speed);
        g.rod_index=FISH_HEAVY_SEA_ROD;g.economy.rods|=1u<<FISH_HEAVY_SEA_ROD;g.economy.lines|=1u<<line;fish_game_equip_line(&g,line);
        g.depth_index=FISH_DEPTH_OPTION_COUNT-1;
        fish_game_input(&g,FISH_PRESS,0,1000);fish_game_input(&g,FISH_RELEASE,0,2500);
        assert(g.mode==FISH_CAST && g.cast_x<=1680 && g.cast_x>1608 && g.cast_bottom_cm==12500);
        if(line==FISH_MARINE_LINE)assert(!g.short_line && g.cast_depth_cm==12500);
        else assert(g.short_line && !g.cast_depth_cm && !g.depth_index);
        uint8_t saved[FISH_SAVE_PAYLOAD_BYTES];fish_save_pack(&g,saved);fish_game_init(&loaded,0);
        assert(fish_save_unpack(&loaded,saved) && loaded.boat_x==1608 && loaded.map_index==2);
    }
    printf("PASS: %u casts across3maps; %u strong+ bottom casts in oldmaps; %u middle/%u surface fallbacks; actual line length and V19 persistence verified\n",casts,old_bottom,mid,surface);
}
