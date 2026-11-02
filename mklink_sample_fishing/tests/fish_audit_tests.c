#include "fish_game.h"
#include "fish_save.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int fish_storage_init(uint8_t *k){(void)k;return -1;}
int fish_storage_read(unsigned s,uint8_t *p,size_t n){(void)s;(void)p;(void)n;return -2;}
int fish_storage_begin(unsigned s,const uint8_t *p,size_t n){(void)s;(void)p;(void)n;return -1;}
int fish_storage_poll(void){return 0;}
extern const uint8_t fish_city_encounters[FISH_SPECIES_COUNT];
typedef struct {int x,depth,boat,facing;unsigned bait,minute,weight;} witness_t;
static witness_t witness[3][FISH_SPECIES_COUNT];
static bool ordinary(unsigned map,unsigned id)
{
    if(!map)return id<FISH_BASE_SPECIES_COUNT;
    if(map==1)return fish_city_encounters[id]>0;
    return id>=FISH_COAST_FIRST && id!=FISH_OARFISH && !fish_species_is_variant(id) && !fish_species_is_foraging(id);
}
static int bounded(int x,int low,int high){return x<low?low:x>high?high:x;}
static void hook(fish_game_t *g,unsigned map,unsigned id,unsigned mm,unsigned rod,unsigned line,unsigned seed,witness_t w)
{
    fish_game_init(g,0);g->map_index=map;g->rod_index=rod;g->economy.rods|=1u<<rod;
    g->economy.lines|=1u<<line;fish_game_equip_line(g,line);g->boat_x=w.boat;g->facing=w.facing;
    g->mode=FISH_BITE;g->pending_species=fish_species_parent(id);g->pending_variant=fish_species_is_variant(id)?id:0;
    g->pending_size_mm=mm;g->pending_weight_g=fish_species_weight_g(fish_species_profile(map,id),mm);
    g->random_state=seed;g->phase_at=0-fish_species_profile(map,g->pending_species)->strike_delay_ms;
    g->cast_start_x=w.boat+w.facing*12;g->cast_x=w.x;g->cast_distance=abs(w.x-g->cast_start_x);
    g->cast_bottom_cm=fish_map_depth_on(map,w.x);g->cast_depth_cm=w.depth;
    fish_game_input(g,FISH_PRESS,0,0);fish_game_input(g,FISH_RELEASE,0,0);assert(g->mode==FISH_FIGHT);
}
int main(void)
{
    setvbuf(stdout,NULL,_IONBF,0);
    const unsigned times[]={720,1320,360,1140};unsigned normal_count[3]={0};
    for(unsigned map=0;map<3;map++){
        unsigned line=map==2?FISH_MARINE_LINE:map?FISH_PE_LINE:2;
        for(int x=fish_map_water_left(map)+4;x<fish_map_water_right(map);x+=8){
            int bottom=fish_map_depth_on(map,x);if(bottom<=0)continue;
            int boat=bounded(x-40,fish_map_boat_left(map),fish_map_boat_right(map)),facing=1;
            if(x-boat-12<=0){boat=bounded(x+40,fish_map_boat_left(map),fish_map_boat_right(map));facing=-1;}
            if((x-boat-facing*12)*facing<=0)continue;
            int depths[]={0,bottom/2,bottom};
            for(unsigned t=0;t<4;t++)for(unsigned d=0;d<3;d++)for(unsigned bait=0;bait<FISH_BAIT_COUNT;bait++){
                for(unsigned id=0;id<FISH_SPECIES_COUNT;id++){
                    unsigned weight=fish_map_tackle_weight_on(map,id,x,depths[d],bait,times[t],0,line);
                    if(!weight)continue;
                    assert(ordinary(map,id));
                    if(!witness[map][id].weight)witness[map][id]=(witness_t){x,depths[d],boat,facing,bait,times[t],weight};
                }
            }
        }
        for(unsigned id=0;id<FISH_SPECIES_COUNT;id++)if(ordinary(map,id)){
            witness_t w=witness[map][id];if(!w.weight)fprintf(stderr,"unreachable map=%u species=%u\n",map,id);
            assert(w.weight);normal_count[map]++;
            printf("DISTRIBUTION,%u,%u,%d,%d,%u,%u,%u\n",map,id,w.x,w.depth,w.bait,w.minute,w.weight);
        }
    }
    assert(normal_count[0]==11 && normal_count[1]==12 && normal_count[2]==14);
    unsigned profiles=0,trials=0,landing_checks=0;
    for(unsigned map=0;map<3;map++)for(unsigned id=0;id<FISH_SPECIES_COUNT;id++){
        witness_t w=witness[map][fish_species_parent(id)];
        bool rare=fish_species_is_variant(id) && w.weight;
        if(id==FISH_OARFISH && map==2){w=(witness_t){1320,fish_map_depth_on(2,1320)/2,1280,1,4,1320,1};rare=true;}
        if(!ordinary(map,id) && !rare)continue;
        const fish_species_t *s=fish_species_profile(map,id);unsigned rod=map==2?FISH_HEAVY_SEA_ROD:map?FISH_LONG_ROD:2;
        unsigned line=map==2?FISH_MARINE_LINE:map?FISH_PE_LINE:2;
        unsigned sizes[]={s->min_mm,(s->mean_x2_mm+1)/2,s->max_mm};
        unsigned away=0,toward=0,peak=0,lost=0,landed[3]={0};int greatest=0;
        for(unsigned size=0;size<3;size++)for(unsigned seed=1;seed<=16;seed++){
            fish_game_t g;hook(&g,map,id,sizes[size],rod,line,seed,w);unsigned start=fish_game_distance_cm(&g);
            for(unsigned t=20;t<=4000 && g.mode==FISH_FIGHT;t+=20){
                int before=(int)fish_game_distance_cm(&g);fish_game_update(&g,t);int after=(int)fish_game_distance_cm(&g);
                away+=after>before;toward+=after<before;if(g.tension>(int)peak)peak=g.tension;
                if(abs(after-(int)start)>greatest)greatest=abs(after-(int)start);
            }
            lost+=g.mode==FISH_LOST;trials++;
            hook(&g,map,id,sizes[size],rod,line,seed,w);g.reel_units=9999;g.tension=0;g.stamina_recovering=true;
            fish_game_input(&g,FISH_ROTATE,1,0);assert(g.mode==FISH_CAUGHT && g.reel_units==10000);landing_checks++;
            hook(&g,map,id,sizes[size],rod,line,seed,w);
            for(unsigned t=100;t<=180000 && g.mode==FISH_FIGHT;t+=100){
                fish_game_update(&g,t);if(g.mode!=FISH_FIGHT)break;
                int steps=g.tension>=g.line_strength*65/100?-1:
                    (fish_game_burst_active(&g) || fish_game_burst_warning(&g)) && g.tension>=g.line_strength/2?0:1;
                fish_game_input(&g,FISH_ROTATE,steps,t);
            }
            landed[size]+=g.mode==FISH_CAUGHT;
        }
        printf("MOTION,%u,%u,%u,%u,%u,%d,%u,%u\n",map,id,s->fight_style,away,toward,greatest,peak,lost);profiles++;
        printf("CONTROLLER,%u,%u,%u,%u,%u\n",map,id,landed[0],landed[1],landed[2]);
    }
    /* Test real legal deep habitats with every compatible rod, including a
     * weaker old rod. A safe detent must always make positive progress. */
    unsigned checks=0,stalls=0;
    for(int x=600;x<1440;x+=80)for(unsigned layer=1;layer<3;layer++){
        int bottom=fish_map_depth_on(2,x),depth=layer==1?bottom/2:bottom;
        witness_t w={x,depth,bounded(x-40,540,1368),1,4,1320,1};
        for(unsigned id=FISH_COAST_FIRST;id<FISH_SPECIES_COUNT;id++)if(ordinary(2,id)){
            bool present=false;for(unsigned bait=0;bait<FISH_BAIT_COUNT;bait++)present|=fish_map_tackle_weight_on(2,id,x,depth,bait,1320,0,FISH_MARINE_LINE)>0;
            if(!present)continue;
            for(unsigned rod=1;rod<FISH_ROD_COUNT;rod++)for(unsigned stage=0;stage<2;stage++){
                fish_game_t g;hook(&g,2,id,fish_species[id].max_mm,rod,FISH_MARINE_LINE,42,w);
                g.reel_units=9750;g.tension=0;int before=g.reel_units;unsigned cm=fish_game_distance_cm(&g);
                unsigned at=stage?fish_rules.hook_settle_ms+50:0;
                g.clock_at=g.simulation_at=g.now=at; /* snapshot just after safe payout */
                fish_game_input(&g,FISH_ROTATE,1,at);checks++;
                if(g.reel_units<=before){stalls++;fprintf(stderr,"STALL,%u,%u,%d,%lu,%u,%d\n",id,rod,depth,(unsigned long)g.pending_weight_g,g.line_response_percent,fish_game_pull(&g,0));}
                else assert(fish_game_distance_cm(&g)<=cm);
            }
        }
    }
    printf("SUMMARY,%u,%u,%u,%u,%u\n",profiles,trials,landing_checks,checks,stalls);
    assert(!stalls);
    puts("PASS: reachable fish across all maps, valid special branches, size/seed motion, final landing and positive safe deep reeling");
}
