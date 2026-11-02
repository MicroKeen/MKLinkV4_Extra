#include "fish_cat.h"
#include "fish_game.h"
#include <stdlib.h>

bool fish_cat_owned(const fish_game_t *g){return g->cat.friendship>=FISH_CAT_FRIEND_MAX;}
unsigned fish_cat_day(const fish_game_t *g){return g->weather_ms/1440000u;}
static unsigned encounter(const fish_game_t *g){return fish_cat_day(g)*24+g->game_minute/60+1;}
static bool scheduled(const fish_game_t *g)
{
    if(g->map_index!=1 || fish_cat_owned(g) || g->game_minute<720 || g->game_minute%60>=30)return false;
    /* Independent hourly visits with a half-hour gap. Reloading cannot reroll
     * presence or reset the three interactions of the same encounter. */
    uint32_t h=(encounter(g)+53u)*0x9e3779b9u;h^=h>>16;h*=0x85ebca6bu;h^=h>>13;
    return h%100<FISH_CAT_HOURLY_CHANCE_PERCENT;
}
bool fish_cat_present(const fish_game_t *g)
{
    if(g->map_index!=1 || fish_cat_owned(g))return false;
    return g->cat.flags&FISH_CAT_READY?(g->cat.flags&FISH_CAT_VISIBLE)!=0:scheduled(g);
}
static bool in_cat_area(const fish_game_t *g)
{
    if(g->map_index!=1 || !fish_game_ashore(g))return false;
    int camera=fish_game_camera(g),margin=FISH_CAT_WALK_LIMIT+48;
    return FISH_CAT_X+margin>=camera && FISH_CAT_X-margin<=camera+240;
}
int fish_cat_world_x(const fish_game_t *g)
{return FISH_CAT_X+g->cat.walk_x;}
bool fish_cat_facing_left(const fish_game_t *g)
{
    if(g->mode==FISH_CAT || (g->cat.flags&FISH_CAT_BLOCKED))return g->shore_x<fish_cat_world_x(g);
    unsigned state=g->cat.flags&FISH_CAT_STROLL_MASK;
    return state==FISH_CAT_WALK_LEFT || (state==FISH_CAT_SIT && g->cat.walk_x>0);
}
bool fish_cat_near(const fish_game_t *g)
{return fish_cat_present(g) && abs(g->shore_x-fish_cat_world_x(g))<=FISH_CAT_PERSONAL_SPACE+2;}
unsigned fish_cat_feeds_left(const fish_game_t *g)
{return g->cat.day!=fish_cat_day(g)?FISH_CAT_DAILY_FEEDS:FISH_CAT_DAILY_FEEDS-g->cat.feeds;}
void fish_cat_update(fish_game_t *g)
{
    if(g->cat.day!=fish_cat_day(g)) {
        if(g->cat.feeds)g->save_requested=true;
        g->cat.day=(uint8_t)fish_cat_day(g);g->cat.feeds=0;
    }
    bool nearby=in_cat_area(g),visible=(g->cat.flags&FISH_CAT_VISIBLE)!=0;
    if(!(g->cat.flags&FISH_CAT_READY)) {
        visible=scheduled(g);g->cat.flags=FISH_CAT_READY|(visible?FISH_CAT_VISIBLE|FISH_CAT_WALK_RIGHT:0);
        g->cat.stroll_at=g->now;
        if(visible && g->cat.encounter!=encounter(g)) {
            g->cat.encounter=(uint16_t)encounter(g);g->cat.actions=0;g->save_requested=true;
        }
    } else if(!nearby) {
        bool next=scheduled(g);
        if(next!=visible || (next && g->cat.encounter!=encounter(g))) {
            visible=next;g->cat.flags=FISH_CAT_READY|(next?FISH_CAT_VISIBLE|FISH_CAT_WALK_RIGHT:0);
            g->cat.walk_x=0;g->cat.stroll_at=g->now;g->save_requested=true;
            if(next){g->cat.encounter=(uint16_t)encounter(g);g->cat.actions=0;g->cat.reply=FISH_CAT_QUIET;}
        }
    }
    /* Keep the current visit (including absence and adoption) while the whole
     * wandering corridor can be seen. No hourly reroll under the player's eyes. */
    if(visible) {
        unsigned state=g->cat.flags&FISH_CAT_STROLL_MASK;
        int distance=g->shore_x-fish_cat_world_x(g);
        bool was_blocked=(g->cat.flags&FISH_CAT_BLOCKED)!=0;
        bool reacting=g->cat.reply && g->now-g->cat.reply_at<1800;
        bool blocked=nearby && abs(distance)<=FISH_CAT_PERSONAL_SPACE+8 &&
            ((state==FISH_CAT_WALK_RIGHT && distance>=0) || (state==FISH_CAT_WALK_LEFT && distance<=0));
        g->cat.flags=(g->cat.flags&~FISH_CAT_BLOCKED)|(blocked?FISH_CAT_BLOCKED:0);
        if(blocked) {
            if(!was_blocked || g->mode==FISH_CAT || reacting)g->cat.stroll_at=g->now;
            else if(g->now-g->cat.stroll_at>=2000) {
                g->cat.flags=(g->cat.flags&~(FISH_CAT_STROLL_MASK|FISH_CAT_BLOCKED))|
                    (state==FISH_CAT_WALK_RIGHT?FISH_CAT_WALK_LEFT:FISH_CAT_WALK_RIGHT);
                g->cat.stroll_at=g->now;
            }
        } else if(g->mode==FISH_CAT || reacting)g->cat.stroll_at=g->now;
        else {
            uint32_t elapsed=g->now-g->cat.stroll_at;
            if(state==FISH_CAT_WALK_RIGHT || state==FISH_CAT_WALK_LEFT) {
                unsigned steps=elapsed/100;if(steps>56)steps=56;
                int x=g->cat.walk_x+(state==FISH_CAT_WALK_RIGHT?1:-1)*(int)steps;
                g->cat.stroll_at+=steps*100;
                if((state==FISH_CAT_WALK_RIGHT && x>=FISH_CAT_WALK_LIMIT) || (state==FISH_CAT_WALK_LEFT && x<=-FISH_CAT_WALK_LIMIT)) {
                    x=x>0?FISH_CAT_WALK_LIMIT:-FISH_CAT_WALK_LIMIT;
                    g->cat.flags=(g->cat.flags&~FISH_CAT_STROLL_MASK)|(x>0?FISH_CAT_SIT:FISH_CAT_SLEEP);
                    g->cat.stroll_at=g->now;
                }
                g->cat.walk_x=(int8_t)x;
            } else if(elapsed>=(state==FISH_CAT_SLEEP?5000u:2000u)) {
                g->cat.flags=(g->cat.flags&~FISH_CAT_STROLL_MASK)|(g->cat.walk_x>0?FISH_CAT_WALK_LEFT:FISH_CAT_WALK_RIGHT);
                g->cat.stroll_at=g->now;
            }
        }
    }
    if(g->mode==FISH_CAT && ((!fish_cat_present(g) && !fish_cat_owned(g)) ||
       (fish_cat_owned(g) && g->now-g->cat.reply_at>=1600))) {
        g->mode=FISH_SHORE;g->pressed=false;
    }
}
void fish_cat_interact(fish_game_t *g,unsigned action)
{
    if(action>=2){g->mode=FISH_SHORE;return;}
    fish_cat_update(g);
    if(!fish_cat_near(g) || fish_cat_owned(g))return;
    g->cat.reply_at=g->now;
    if(g->cat.actions>=FISH_CAT_VISIT_ACTIONS){g->cat.reply=FISH_CAT_TIRED;return;}
    if(action) {
        if(!fish_cat_feeds_left(g)){g->cat.reply=FISH_CAT_FULL;return;}
        if(!g->cat.tins){g->cat.reply=FISH_CAT_NO_FOOD;return;}
        --g->cat.tins;++g->cat.feeds;g->cat.friendship+=5;g->cat.reply=FISH_CAT_FED;
    } else {++g->cat.friendship;g->cat.reply=FISH_CAT_PETTED;}
    ++g->cat.actions;
    if(g->cat.friendship>=FISH_CAT_FRIEND_MAX){g->cat.friendship=FISH_CAT_FRIEND_MAX;g->cat.reply=FISH_CAT_JOINED;}
    g->save_requested=true;
}
bool fish_cat_catch_menu(const fish_game_t *g)
{
    unsigned id=fish_species_parent(g->last_species);
    return !g->forage.return_shore && g->mode==FISH_CAUGHT && fish_cat_owned(g) && g->last_weight_g &&
        g->last_weight_g<=FISH_CAT_SMALL_FISH_G && id!=9 && id!=10 && id!=23 && id!=25 && id!=31 && !fish_species_is_foraging(id) && id!=FISH_SQUID &&
        g->cargo_result!=FISH_FED_CAT;
}
bool fish_cat_feed_catch(fish_game_t *g)
{
    if(!fish_cat_catch_menu(g))return false;
    fish_cat_update(g);g->cat.reply_at=g->now;
    if(!fish_cat_feeds_left(g)){g->cat.reply=FISH_CAT_FULL;return false;}
    if(g->cargo_result==FISH_STORED || g->cargo_result==FISH_CABIN_STORED) {
        fish_economy_t *e=&g->economy;unsigned id=g->last_species;
        uint32_t price=fish_catch_price_on(g->last_map,id,g->last_size_mm);
        bool cabin=g->cargo_result==FISH_CABIN_STORED;unsigned k=cabin?id-FISH_COAST_FIRST:id;
        uint16_t *count=cabin?e->cabin_count:e->count;uint32_t *grams=cabin?e->cabin_grams:e->grams,*value=cabin?e->cabin_value:e->value;
        if(!count[k] || grams[k]<g->last_weight_g || value[k]<price)return false;
        --count[k];grams[k]-=g->last_weight_g;value[k]-=price;
        unsigned start=cabin?FISH_PORTABLE_PRICE_GROUPS:0,end=cabin?FISH_PRICE_GROUPS:FISH_PORTABLE_PRICE_GROUPS;
        for(unsigned i=start;i<end;i++)if(e->prices[i].count && e->prices[i].price==price) {
            if(!--e->prices[i].count) {
                unsigned j=i;while(j+1<end && e->prices[j+1].count){e->prices[j]=e->prices[j+1];++j;}
                e->prices[j]=(fish_price_group_t){0};
            }
            break;
        }
    }
    ++g->cat.feeds;g->cat.reply=FISH_CAT_FISH_FED;g->cargo_result=FISH_FED_CAT;g->save_requested=true;
    return true;
}
bool fish_cat_input(fish_game_t *g,int event,int steps)
{
    bool menu=g->mode==FISH_CAT,catch_menu=fish_cat_catch_menu(g);
    if(!menu && !catch_menu)return false;
    unsigned count=menu?3:2;
    if(event==FISH_ROTATE) {
        int i=((int)g->cat.menu+steps%(int)count)%(int)count;
        g->cat.menu=(uint8_t)(i<0?i+(int)count:i);
    } else if(event==FISH_PRESS)g->pressed=true;
    else if(event==FISH_RELEASE && g->pressed) {
        g->pressed=false;
        if(menu)fish_cat_interact(g,g->cat.menu);
        else if(g->cat.menu) {if(fish_cat_feed_catch(g)){g->mode=FISH_AIM;g->phase_at=g->now;}}
        else {g->mode=FISH_AIM;g->phase_at=g->now;}
    }
    return true;
}
unsigned fish_cat_pose(const fish_game_t *g,bool aboard)
{
    if(g->cat.reply && g->now-g->cat.reply_at<1800) {
        if(g->cat.reply==FISH_CAT_FED || g->cat.reply==FISH_CAT_FISH_FED)return 8;
        if(g->cat.reply==FISH_CAT_PETTED || g->cat.reply==FISH_CAT_JOINED)return 5;
    }
    unsigned state=(g->now/6000)%3;
    if(!aboard) {
        if(g->mode==FISH_CAT || (g->cat.flags&FISH_CAT_BLOCKED))return g->now%4000<350?4:3;
        state=g->cat.flags&FISH_CAT_STROLL_MASK;
        if(state==FISH_CAT_WALK_RIGHT || state==FISH_CAT_WALK_LEFT)return (g->now/160)%3;
        return state==FISH_CAT_SLEEP?6+(g->now/700)%2:g->now%4000<350?4:3;
    }
    if(state<2)return g->now%4000<350?4:3;
    return 6+(g->now/700)%2;
}
const char *fish_cat_message(const fish_game_t *g)
{
    switch(g->cat.reply) {
        case FISH_CAT_PETTED:return "它蹭了蹭你的手";
        case FISH_CAT_FED:case FISH_CAT_FISH_FED:return "吃得还挺香";
        case FISH_CAT_NO_FOOD:return "没带鱼罐头，去店里看看";
        case FISH_CAT_FULL:return "今天吃饱了，明天再喂吧";
        case FISH_CAT_TIRED:return "让它歇会儿吧";
        case FISH_CAT_JOINED:return "这回赖上你的船了";
        default:return "一只在等人的流浪猫";
    }
}
