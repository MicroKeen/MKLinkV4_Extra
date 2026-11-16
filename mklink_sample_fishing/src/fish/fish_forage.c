#include "fish_game.h"
#include <stdlib.h>
static uint32_t mix(uint32_t h){h^=h>>16;h*=0x85ebca6bu;h^=h>>13;h*=0xc2b2ae35u;return h^(h>>16);}
static uint32_t node_hash(const fish_game_t *g,unsigned node){return mix(0x41504c45u+g->forage.day*0x9e3779b9u+node*0x7f4a7c15u);}
unsigned fish_forage_day(const fish_game_t *g)
{return ((g->weather_ms+1200000u)/1440000u)%96;}
unsigned fish_forage_count(const fish_game_t *g,unsigned area)
{return 3+mix(0x1b873593u+g->forage.day*97u+area*419u)%4;}
bool fish_forage_exists(const fish_game_t *g,unsigned node)
{return g->forage.ready && node<12 && node%6<fish_forage_count(g,node/6) && !(g->forage.taken&(1u<<node));}
int fish_forage_x(const fish_game_t *g,unsigned node)
{
    unsigned area=node/6,count=fish_forage_count(g,area);
    if(node>=FISH_FORAGE_NODES || node%6>=count)return -1;
    /* Sample native2px positions over the whole area, allowing tight clusters.
     * Only exact duplicates are excluded; no artificial spacing is reserved.
     * Floyd sampling is bounded to six entries and never consumes fishing RNG. */
    unsigned range=(area?176:136)/2+1,sites[6];
    uint32_t seed=mix(0x3c6ef372u+g->forage.day*977u+area*419u);
    for(unsigned i=0;i<count;i++) {
        unsigned limit=range-count+i,t=mix(seed+i*0x9e3779b9u)%(limit+1);
        for(unsigned k=0;k<i;k++)if(sites[k]==t){t=limit;break;}
        sites[i]=t;
    }
    for(unsigned i=1;i<count;i++) {
        unsigned t=sites[i],k=i;
        while(k && sites[k-1]>t){sites[k]=sites[k-1];--k;}sites[k]=t;
    }
    return (area?288:124)+(int)sites[node%6]*2;
}
unsigned fish_forage_species(const fish_game_t *g,unsigned node)
{
    if((node_hash(g,node)>>16)%100<3)return node<6?FISH_HARP_SNAIL:FISH_GREEN_CRAB;
    unsigned ticket=(node_hash(g,node)>>8)%100;
    if(node<6)return ticket<36?38:ticket<50?40:ticket<72?42:FISH_RAGWORM;
    return ticket<48?39:ticket<64?41:43;
}
bool fish_forage_visible(const fish_game_t *g,unsigned node)
{
    return g->map_index==2 && fish_game_ashore(g) && fish_forage_exists(g,node) &&
        (g->forage.seen&(1u<<node)) && abs(g->shore_x-fish_forage_x(g,node))<=FISH_FORAGE_NOTICE;
}
int fish_forage_near(const fish_game_t *g)
{
    if(g->map_index!=2 || g->mode!=FISH_SHORE)return -1;
    int nearest=-1,distance=FISH_FORAGE_REACH+1;
    for(unsigned n=0;n<12;n++)if(fish_forage_exists(g,n) && (g->forage.seen&(1u<<n))){
        int d=abs(g->shore_x-fish_forage_x(g,n));
        if(d<distance){distance=d;nearest=(int)n;}
    }
    return nearest;
}
static void complete(fish_game_t *g,bool success)
{
    unsigned id=fish_forage_species(g,g->forage.node);g->pressed=false;g->phase_at=g->now;g->forage.at=g->now;
    if(!success){g->mode=FISH_SHORE;g->forage.reply=FISH_FORAGE_ESCAPED;return;}
    unsigned size=g->forage.size_mm;g->last_species=(uint8_t)id;g->last_map=2;g->last_size_mm=size;
    g->last_weight_g=fish_species_weight_g(&fish_species[id],size);
    unsigned amount=g->forage.amount?g->forage.amount:1;
    if((unsigned)g->catches<2147483647u-amount)g->catches+=(int)amount;else g->catches=2147483647;
    if(g->species_catches[id]<65535u-amount)g->species_catches[id]+=amount;else g->species_catches[id]=65535;
    if(size>g->species_best_size_mm[id]){g->species_best_size_mm[id]=size;g->species_best_map[id]=2;}
    g->forage.stored=0;
    if(id==FISH_RAGWORM){
        unsigned room=FISH_BAIT_STOCK_LIMIT-g->economy.bait_stock[FISH_BAIT_RAGWORM];
        g->forage.stored=(uint8_t)(amount<room?amount:room);
        g->economy.bait_stock[FISH_BAIT_RAGWORM]+=g->forage.stored;g->cargo_result=FISH_BAIT_STORED;
    }else{
        g->cargo_result=FISH_RELEASED;
        for(unsigned i=0;i<amount;i++){
            fish_cargo_result_t result=fish_hold_add_on(&g->economy,2,id,size,g->last_weight_g);
            if(result!=FISH_RELEASED){g->forage.stored++;g->cargo_result=result;}
        }
    }
    g->forage.return_shore=1;g->mode=FISH_CAUGHT;g->save_requested=true;
}
void fish_forage_update(fish_game_t *g)
{
    unsigned day=fish_forage_day(g);
    if(!g->forage.ready || (g->forage.day!=day && g->mode!=FISH_FORAGE_QTE)){
        g->forage.day=day;g->forage.ready=1;g->forage.taken=g->forage.seen=0;
        if(g->map_index==2)g->save_requested=true;
    }
    if(g->mode==FISH_FORAGE_QTE){if(g->now-g->forage.at>=FISH_FORAGE_QTE_MS)complete(g,false);return;}
    if(g->map_index!=2 || g->mode!=FISH_SHORE)return;
    for(unsigned n=0;n<FISH_FORAGE_NODES;n++)if(fish_forage_exists(g,n)){
        int distance=abs(fish_forage_x(g,n)-g->shore_x);
        if(distance<=FISH_FORAGE_NOTICE && !(g->forage.seen&(1u<<n))){g->forage.seen|=1u<<n;g->save_requested=true;}
    }
}
bool fish_forage_start(fish_game_t *g)
{
    int node=fish_forage_near(g);if(node<0)return false;
    unsigned area=(unsigned)node/6,id=fish_forage_species(g,node);g->forage.at=g->now;g->pressed=false;
    if(!g->forage.tool[area]){g->forage.reply=area?FISH_FORAGE_NEED_BAR:FISH_FORAGE_NEED_SHOVEL;return true;}
    if(id==FISH_RAGWORM && g->economy.bait_stock[FISH_BAIT_RAGWORM]>=FISH_BAIT_STOCK_LIMIT){g->forage.reply=FISH_FORAGE_WORM_FULL;return true;}
    g->forage.tool[area]--;g->forage.taken|=1u<<node;g->forage.node=node;g->forage.reply=0;g->forage.stored=0;
    g->forage.amount=(uint8_t)(id==38 || id==39?2+(node_hash(g,node)>>24)%3:id==FISH_RAGWORM?3+(node_hash(g,node)>>24)%3:1);
    uint32_t random=node_hash(g,node)^0x4cf5ad43u;
    g->forage.size_mm=fish_species_sample_size(&random,&fish_species[id]);
    g->shore_speed=0;g->save_requested=true;
    bool shell=id==38 || id==39 || id==40 || id==FISH_HARP_SNAIL;
    if(!shell && fish_random_next(&random)%100<60){g->mode=FISH_FORAGE_QTE;g->previous_mode=FISH_SHORE;g->phase_at=g->now;}
    else complete(g,true);
    return true;
}
unsigned fish_forage_slider_width(const fish_game_t *g)
{
    switch(fish_forage_species(g,g->forage.node)){case 41:case 42:return 16;case 43:case FISH_GREEN_CRAB:return 18;default:return 24;}
}
unsigned fish_forage_slider(const fish_game_t *g)
{
    unsigned id=fish_forage_species(g,g->forage.node),period=id==41?1400:id==42?1200:id==43 || id==FISH_GREEN_CRAB?1500:1700;
    unsigned t=(g->now-g->forage.at)%period,half=period/2;unsigned span=FISH_FORAGE_BAR_WIDTH-fish_forage_slider_width(g);
    return (t<half?t:period-t)*span/half;
}
bool fish_forage_hit(const fish_game_t *g)
{unsigned centre=fish_forage_slider(g)+fish_forage_slider_width(g)/2;return centre>=FISH_FORAGE_TARGET_LEFT && centre<FISH_FORAGE_TARGET_LEFT+FISH_FORAGE_TARGET_WIDTH;}
bool fish_forage_input(fish_game_t *g,int event,int steps)
{
    (void)steps;
    if(g->mode!=FISH_FORAGE_QTE)return false;
    if(event==FISH_PRESS)complete(g,fish_forage_hit(g));
    return true;
}
const char *fish_forage_message(const fish_game_t *g)
{
    if(g->now-g->forage.at>2200)return "";
    switch(g->forage.reply){case FISH_FORAGE_NEED_SHOVEL:return "得先带把铲子";case FISH_FORAGE_NEED_BAR:return "这里得用撬棍";
        case FISH_FORAGE_ESCAPED:return "一转眼就溜走了";case FISH_FORAGE_WORM_FULL:return "饵盒已经装满了";default:return "";}
}
