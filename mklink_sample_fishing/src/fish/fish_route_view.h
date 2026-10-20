#ifndef FISH_ROUTE_VIEW_H
#define FISH_ROUTE_VIEW_H
#include <stdbool.h>
#include <stdint.h>
#include "fish_map_config.h"

/* Virtual printed route sheet. These are UI coordinates, not lake geometry. */
enum {FISH_ROUTE_WIDTH=640,FISH_ROUTE_HEIGHT=480,
      FISH_ROUTE_VIEW_X=8,FISH_ROUTE_VIEW_Y=40,
      FISH_ROUTE_VIEW_WIDTH=224,FISH_ROUTE_VIEW_HEIGHT=136,FISH_ROUTE_PAN_MS=360};
typedef struct {int16_t x,y;} fish_route_stop_t;
static const fish_route_stop_t fish_route_stops[FISH_MAP_COUNT]={{280,180},{388,230}};
typedef struct {
    int16_t x,y,from_x,from_y;
    uint32_t started_at;
    uint8_t selected;
    bool active;
} fish_route_view_t;
_Static_assert(sizeof(fish_route_view_t)==16,"Route camera should remain small");

static inline int fish_route_focus(int position,int canvas,int window)
{
    int start=position-window/2;
    return start<0?0:start>canvas-window?canvas-window:start;
}
static inline bool fish_route_overview(int *x,int *y)
{
    int left=FISH_ROUTE_WIDTH,right=0,top=FISH_ROUTE_HEIGHT,bottom=0;
    for(unsigned i=0;i<FISH_MAP_COUNT;i++) {
        int sx=fish_route_stops[i].x,sy=fish_route_stops[i].y;
        if(sx-38<left)left=sx-38;
        if(sx+38>right)right=sx+38;
        if(sy-58<top)top=sy-58;
        if(sy+23>bottom)bottom=sy+23;
    }
    if(right-left>FISH_ROUTE_VIEW_WIDTH || bottom-top>FISH_ROUTE_VIEW_HEIGHT)return false;
    *x=fish_route_focus((left+right)/2,FISH_ROUTE_WIDTH,FISH_ROUTE_VIEW_WIDTH);
    *y=fish_route_focus((top+bottom)/2,FISH_ROUTE_HEIGHT,FISH_ROUTE_VIEW_HEIGHT);
    return true;
}
static inline void fish_route_target(unsigned selected,int *x,int *y)
{
    /* Keep a compact network together; only pan after it outgrows the window. */
    if(fish_route_overview(x,y))return;
    *x=fish_route_focus(fish_route_stops[selected].x,FISH_ROUTE_WIDTH,FISH_ROUTE_VIEW_WIDTH);
    *y=fish_route_focus(fish_route_stops[selected].y,FISH_ROUTE_HEIGHT,FISH_ROUTE_VIEW_HEIGHT);
}
static inline void fish_route_view_sample(fish_route_view_t *view,uint32_t now)
{
    uint32_t elapsed=now-view->started_at;
    int inverse=elapsed>=FISH_ROUTE_PAN_MS?0:1024-(int)(elapsed*1024/FISH_ROUTE_PAN_MS);
    int ease=1024-(((inverse*inverse)>>10)*inverse>>10);
    int tx,ty;fish_route_target(view->selected,&tx,&ty);
    view->x=(int16_t)(view->from_x+(tx-view->from_x)*ease/1024);
    view->y=(int16_t)(view->from_y+(ty-view->from_y)*ease/1024);
}
/* Called once at frame start; PFB blocks all consume the same camera. */
static inline void fish_route_view_update(fish_route_view_t *view,bool active,unsigned selected,uint32_t now)
{
    if(!active) {view->active=false;return;}
    if(selected>=FISH_MAP_COUNT)selected=0;
    if(!view->active) {
        view->selected=(uint8_t)selected;view->active=true;
        int tx,ty;fish_route_target(selected,&tx,&ty);
        view->x=view->from_x=(int16_t)tx;view->y=view->from_y=(int16_t)ty;
        view->started_at=now;return;
    }
    fish_route_view_sample(view,now);
    if(view->selected!=selected) {
        view->from_x=view->x;view->from_y=view->y;
        view->selected=(uint8_t)selected;view->started_at=now;
    }
}
#endif
