/* Audit actual input policies, rather than only proving that landing is possible. */
#include "fish_game.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct {unsigned map,id;int x,depth;} matchup_t;
#include "fight_control_cases.h"
static int bound(int v,int lo,int hi){return v<lo?lo:v>hi?hi:v;}
static void hook(fish_game_t *g,matchup_t c,unsigned mm,unsigned rod,unsigned line,unsigned seed)
{
 fish_game_init(g,0);g->map_index=c.map;g->rod_index=rod;fish_game_equip_line(g,line);
 int boat=bound(c.x-(int)fish_rods[rod].max_cast_px-12,fish_map_boat_left(c.map),fish_map_boat_right(c.map));
 int facing=1;if(c.x-boat-12<=0){boat=bound(c.x+fish_rods[rod].max_cast_px+12,fish_map_boat_left(c.map),fish_map_boat_right(c.map));facing=-1;}
 g->boat_x=boat;g->facing=facing;g->cast_start_x=boat+facing*12;g->cast_x=c.x;g->cast_distance=abs(c.x-g->cast_start_x);
 g->cast_bottom_cm=fish_map_depth_on(c.map,c.x);g->cast_depth_cm=c.depth;
 g->pending_species=fish_species_parent(c.id);g->pending_variant=fish_species_is_variant(c.id)?c.id:0;g->pending_size_mm=mm;
 g->pending_weight_g=fish_species_weight_g(fish_species_profile(c.map,c.id),mm);g->random_state=seed*0x9e3779b9u;
 g->mode=FISH_BITE;g->phase_at=0-fish_species_profile(c.map,g->pending_species)->strike_delay_ms;
 fish_game_input(g,FISH_PRESS,0,0);fish_game_input(g,FISH_RELEASE,0,0);assert(g->mode==FISH_FIGHT);
}
static unsigned mm_for(unsigned id,unsigned grams)
{
 const fish_species_t *s=fish_species_profile(1,id);unsigned lo=s->min_mm,hi=s->max_mm;
 while(lo<hi){unsigned m=(lo+hi)/2;if(fish_species_weight_g(s,m)<grams)lo=m+1;else hi=m;}return lo;
}
typedef struct {unsigned wins,breaks,range,slack,timeouts,pays,peak;unsigned long long ms;} result_t;
static void trial(result_t *r,matchup_t c,unsigned mm,unsigned rod,unsigned line,unsigned seed,unsigned policy)
{
 fish_game_t g;hook(&g,c,mm,rod,line,seed);unsigned peak=0,pays=0;int history[3]={25,25,25},own[3]={0};bool alert[3]={0};
 for(unsigned t=100;t<=180000 && g.mode==FISH_FIGHT;t+=100){
  fish_game_update(&g,t);if(g.mode!=FISH_FIGHT)break;
  unsigned tick=t/100;history[tick%3]=g.tension;alert[tick%3]=fish_game_burst_active(&g)||fish_game_burst_warning(&g);
  int estimate=history[(tick+1)%3];for(unsigned back=1;back<=2;back++)estimate+=own[(tick+3-back)%3]>0?5:own[(tick+3-back)%3]<0?-10:0;
  int steps=policy==0?(t%500?0:1):policy==1?(t%200?0:1):policy==2?1:policy==3?3:policy==6?
    g.tension>=g.line_strength*65/100?-1:(fish_game_burst_active(&g)||fish_game_burst_warning(&g))?0:3:policy==5?
    estimate>=g.line_strength*65/100?-1:alert[(tick+1)%3] && estimate>=g.line_strength/2?0:1:
    g.tension>=g.line_strength*65/100?-1:(fish_game_burst_active(&g)||fish_game_burst_warning(&g)) && g.tension>=g.line_strength/2?0:1;
  own[tick%3]=steps;
  pays+=steps<0;fish_game_input(&g,FISH_ROTATE,steps,t);if(g.tension>(int)peak)peak=g.tension;
  assert(g.tension>=0 && g.tension<=g.line_strength+20 && g.stamina<=g.stamina_max);
 }
 r->wins+=g.mode==FISH_CAUGHT;r->breaks+=g.loss==FISH_LOSS_BREAK;r->range+=g.loss==FISH_LOSS_RANGE;r->slack+=g.loss==FISH_LOSS_SLACK;r->timeouts+=g.mode==FISH_FIGHT;
 r->pays+=pays;r->ms+=g.now;if(peak>r->peak)r->peak=peak;
 if(g.mode==FISH_CAUGHT)assert(g.reel_units==10000 && g.loss==FISH_LOSS_NONE && g.last_species==c.id);
}
int main(int argc,char **argv)
{
 setvbuf(stdout,NULL,_IONBF,0);bool sample=argc>1 && !strcmp(argv[1],"--sample");
 puts("map,species,mm,grams,rod,line,policy,seeds,landed,breaks,range,slack,timeouts,mean_ms,peak_tension,payouts");
 unsigned rows=0;
 for(unsigned n=0;n<sizeof(matchups)/sizeof(matchups[0]);n++){
  matchup_t c=matchups[n];if(sample && (c.map!=1 || c.id!=17))continue;
  const fish_species_t *s=fish_species_profile(c.map,c.id);unsigned sizes[]={s->min_mm,(s->mean_x2_mm+1)/2,s->max_mm};
  for(unsigned size=0;size<(sample?2u:3u);size++)for(unsigned rod=0;rod<FISH_ROD_COUNT;rod++)for(unsigned line=0;line<FISH_LINE_COUNT;line++){
   if(!fish_tackle_compatible(rod,line) || (sample && line!=FISH_PE_LINE))continue;
   unsigned mm=sample?(size?s->max_mm:mm_for(17,40000)):sizes[size];
   fish_game_t probe;hook(&probe,c,mm,rod,line,1);
   if(fish_line_span_cm(probe.cast_distance*FISH_WORLD_CM_PER_PIXEL,c.depth)>fish_line_sets[line].distance_limit_cm)continue;
   if(c.depth==probe.cast_bottom_cm && !fish_line_bottom_allowed(line,c.depth))continue;
   for(unsigned policy=0;policy<7;policy++){
    result_t r={0};unsigned seeds=sample?64:8;
    for(unsigned seed=1;seed<=seeds;seed++)trial(&r,c,mm,rod,line,seed,policy);
    printf("%u,%u,%u,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%llu,%u,%u\n",c.map,c.id,mm,(unsigned long)fish_species_weight_g(s,mm),rod,line,policy,seeds,r.wins,r.breaks,r.range,r.slack,r.timeouts,r.ms/seeds,r.peak,r.pays);rows++;
    if(sample && policy>=1 && policy<=3)assert(!r.wins && r.breaks==seeds);
    if(sample && policy>=4)assert(r.wins>=seeds*9/10 && r.pays>0);
   }
  }
 }
 printf("AUDIT_ROWS,%u\n",rows);
}
