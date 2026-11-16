#include "fish_map.h"
#include "fish_equipment.h"
#include "fish_marine_map.h"

const fish_dock_t *fish_map_dock_at(unsigned map,unsigned side)
{
    static const fish_dock_t beach={456,512,540,480,FISH_BEACH_WALK_LEFT,FISH_BEACH_WALK_RIGHT};
    return map==2?&beach:&fish_docks[side<2?side:0];
}
int fish_map_dock_on(unsigned map,int x)
{
    for(unsigned i=0;i<(unsigned)fish_map_dock_count(map);i++) {
        int d=x-fish_map_dock_at(map,i)->boat_x;
        if(d>=-FISH_DOCK_REACH && d<=FISH_DOCK_REACH)return (int)i;
    }
    return -1;
}

int fish_map_depth(int x)
{return fish_map_depth_on(0,x);}
int fish_map_depth_on(unsigned map,int x)
{
    const fish_depth_node_t *profile=fish_map_profile(map);
    if (x<=fish_map_water_left(map) || x>=fish_map_water_right(map)) return 0;
    for (unsigned i=1;i<FISH_DEPTH_NODE_COUNT;i++) {
        if (x<=profile[i].x) {
            const fish_depth_node_t *a=&profile[i-1],*b=&profile[i];
            return a->depth_cm+((int)b->depth_cm-a->depth_cm)*(x-a->x)/(b->x-a->x);
        }
    }
    return 0;
}
int fish_map_zone(int x)
{
    for (unsigned i=0;i<FISH_ZONE_COUNT;i++)
        if (x>=fish_map_zones[i].left && x<fish_map_zones[i].right) return (int)i;
    return -1;
}
int fish_map_dock(int x)
{
    for (unsigned i=0;i<2;i++) {
        int delta=x-fish_docks[i].boat_x;
        if (delta>=-FISH_DOCK_REACH && delta<=FISH_DOCK_REACH) return (int)i;
    }
    return -1;
}
uint8_t fish_map_layer(int bottom,int hook)
{
    int band=bottom/FISH_BOTTOM_BAND_DIVISOR;
    if (band<FISH_BOTTOM_BAND_MIN_CM) band=FISH_BOTTOM_BAND_MIN_CM;
    if (bottom-hook<=band) return FISH_LAYER_BOTTOM;
    if (hook<=FISH_SURFACE_BAND_CM) return FISH_LAYER_SURFACE;
    return hook*100>=bottom*60?FISH_LAYER_LOWER:FISH_LAYER_MIDDLE;
}
uint16_t fish_map_weight(unsigned index,int x,int hook,unsigned bait)
{
    if (index>=FISH_BASE_SPECIES_COUNT || bait>=FISH_BAIT_COUNT) return 0;
    int zone=fish_map_zone(x),bottom=fish_map_depth(x);
    const fish_distribution_t *s=&fish_distributions[index];
    if (zone<0 || bottom<=0 || hook<0 || hook>bottom || hook<s->min_depth_cm || hook>s->max_depth_cm) return 0;
    if (!(s->layers&fish_map_layer(bottom,hook))) return 0;
    int distance=x-FISH_WATER_LEFT;
    if (FISH_WATER_RIGHT-x<distance) distance=FISH_WATER_RIGHT-x;
    if (s->nearshore_only && distance>FISH_NEARSHORE_DISTANCE) return 0;
    return fish_species[index].encounter_weight*s->zone_weights[zone]*fish_baits[bait].weights[index];
}
bool fish_map_roll(uint32_t *random,int x,int hook,unsigned bait,uint8_t *index,uint16_t *size)
{
    return fish_map_roll_on(random,0,x,hook,bait,720,index,size);
}
static unsigned activity(const uint8_t *p,unsigned minute)
{
    unsigned t=minute%1440;int from,to,elapsed;
    if(t<300 || t>=1200) return p[2];
    if(t<360) {from=p[2];to=p[1];elapsed=t-300;}
    else if(t<420) {from=p[1];to=p[0];elapsed=t-360;}
    else if(t<1080) return p[0];
    else if(t<1140) {from=p[0];to=p[1];elapsed=t-1080;}
    else {from=p[1];to=p[2];elapsed=t-1140;}
    return (unsigned)(from+(to-from)*elapsed/60);
}
unsigned fish_map_activity_percent(unsigned index,unsigned minute)
{return index<FISH_SPECIES_COUNT?activity(fish_distributions[index].activity_percent,minute):100;}
uint16_t fish_map_weight_at(unsigned index,int x,int hook,unsigned bait,unsigned minute)
{
    uint32_t base=fish_map_weight(index,x,hook,bait);
    if(!base) return 0;
    uint32_t weight=base*fish_map_activity_percent(index,minute)/100;
    if(!weight) weight=1;
    return weight>UINT16_MAX?UINT16_MAX:(uint16_t)weight;
}
bool fish_map_roll_at(uint32_t *random,int x,int hook,unsigned bait,unsigned minute,uint8_t *index,uint16_t *size)
{
    return fish_map_roll_on(random,0,x,hook,bait,minute,index,size);
}
extern const fish_distribution_t fish_city_distributions[FISH_SPECIES_COUNT];
extern const uint8_t fish_city_encounters[FISH_SPECIES_COUNT],fish_city_baits[FISH_BAIT_COUNT][FISH_SPECIES_COUNT];
uint16_t fish_map_weight_on(unsigned map,unsigned index,int x,int hook,unsigned bait,unsigned minute)
{
    if(!map)return fish_map_weight_at(index,x,hook,bait,minute);
    if(map==2) {
        if(index<FISH_COAST_FIRST || index>=FISH_SPECIES_COUNT || bait>=FISH_BAIT_COUNT)return 0;
        const unsigned i=index-FISH_COAST_FIRST;int bottom=fish_map_depth_on(2,x);
        unsigned layers=fish_map_night(minute)?marine_habitats[i].night_layers:marine_habitats[i].layers;
        if(bottom<=0 || hook<0 || hook>bottom || !(layers&fish_map_layer(bottom,hook)))return 0;
        unsigned zone=x<620?0:x<840?1:x<1080?2:x<1200?3:4;
        unsigned n=marine_habitats[i].encounter*marine_habitats[i].zones[zone]*marine_habitats[i].baits[bait];
        return (uint16_t)(n*activity(marine_habitats[i].activity,minute)/100);
    }
    if(map!=1 || index>=FISH_SPECIES_COUNT || bait>=FISH_BAIT_COUNT || !fish_city_encounters[index])return 0;
    int zone=fish_map_zone(x),bottom=fish_map_depth_on(map,x);
    const fish_distribution_t *s=&fish_city_distributions[index];
    if(zone<0 || bottom<=0 || hook<0 || hook>bottom || !(s->layers&fish_map_layer(bottom,hook)))return 0;
    unsigned n=fish_city_encounters[index]*s->zone_weights[zone]*fish_city_baits[bait][index];
    if(!n)return 0;
    n=n*activity(s->activity_percent,minute)/100;
    return (uint16_t)(n?n:1);
}
bool fish_map_roll_on(uint32_t *state,unsigned map,int x,int hook,unsigned bait,unsigned minute,uint8_t *index,uint16_t *size)
{return fish_map_roll_weather_on(state,map,x,hook,bait,minute,0,index,size);}
bool fish_map_coast_special(uint32_t *state,int x,int hook,unsigned bait,unsigned rain,unsigned strength,bool cat,uint8_t *index,uint16_t *size)
{
    int bottom=fish_map_depth_on(2,x);
    if(x<1200 || bottom<=0 || hook<0 || hook>bottom || bait>=FISH_BAIT_COUNT || rain<200 || strength<200 ||
       !(fish_map_layer(bottom,hook)&(FISH_LAYER_MIDDLE|FISH_LAYER_LOWER)) ||
       !marine_habitats[FISH_OARFISH-FISH_COAST_FIRST].baits[bait])return false;
    if(fish_random_next(state)%1000>=(cat?25u:10u))return false;
    *index=FISH_OARFISH;*size=fish_species_sample_size(state,&fish_species[FISH_OARFISH]);return true;
}
uint16_t fish_map_weather_weight_on(unsigned map,unsigned index,int x,int hook,unsigned bait,unsigned minute,unsigned rain)
{
    uint32_t base=fish_map_weight_on(map,index,x,hook,bait,minute);if(!base)return 0;
    if(rain>255)rain=255;
    /* Stable IDs:carp1,catfish3,yellow-catfish6,sharpbelly7,culter15,
     * yellowcheek20,longsnout-catfish22. Hooksnout2 remains unchanged. */
    int bonus=index==1?10:index==7?-8:!map?(index==3 || index==6?15:0):index==22?15:index==15?12:index==20?10:0;
    unsigned percent=(unsigned)(100+bonus*(int)rain/255);
    uint32_t weight=base*percent/100;
    return (uint16_t)(weight>UINT16_MAX?UINT16_MAX:weight?weight:1);
}
bool fish_map_roll_weather_on(uint32_t *state,unsigned map,int x,int hook,unsigned bait,unsigned minute,unsigned rain,uint8_t *index,uint16_t *size)
{return fish_map_roll_tackle_on(state,map,x,hook,bait,minute,rain,FISH_LINE_COUNT,index,size);}
uint16_t fish_map_tackle_weight_on(unsigned map,unsigned index,int x,int hook,unsigned bait,unsigned minute,unsigned rain,unsigned line)
{
    int bottom=fish_map_depth_on(map,x);
    if(line<FISH_LINE_COUNT && hook>0 &&
       ((hook>=bottom && !fish_line_bottom_allowed(line,(unsigned)bottom)) ||
        hook>fish_line_sets[line].distance_limit_cm))return 0;
    unsigned weight=fish_map_weather_weight_on(map,index,x,hook,bait,minute,rain);
    if(weight && line==FISH_PE_LINE) {
        const fish_species_t *s=fish_species_profile(map,index);
        weight=weight*fish_line_bite_percent(line,fish_species_weight_g(s,s->mean_x2_mm/2))/100;
        if(!weight)weight=1;
    }
    return (uint16_t)weight;
}
bool fish_map_roll_tackle_on(uint32_t *state,unsigned map,int x,int hook,unsigned bait,unsigned minute,unsigned rain,unsigned line,uint8_t *index,uint16_t *size)
{return fish_map_roll_lamp_on(state,map,x,hook,bait,minute,rain,line,false,index,size);}
uint16_t fish_map_lamp_weight_on(unsigned map,unsigned index,int x,int hook,unsigned bait,unsigned minute,unsigned rain,unsigned line,bool lamp)
{
    unsigned weight=fish_map_tackle_weight_on(map,index,x,hook,bait,minute,rain,line);
    if(map==2 && lamp && fish_map_night(minute) && fish_map_light_target(index) && bait<FISH_BAIT_COUNT &&
       fish_map_depth_on(2,x)>0 && hook>=0 && hook<=fish_map_depth_on(2,x) && fish_map_layer(fish_map_depth_on(2,x),hook)==FISH_LAYER_SURFACE){
        /* Light also gathers these two species over the inner shelf. */
        if(!weight){unsigned i=index-FISH_COAST_FIRST;
            weight=marine_habitats[i].encounter*2*marine_habitats[i].baits[bait]*activity(marine_habitats[i].activity,minute)/100;
            if(line==FISH_PE_LINE)weight=weight*fish_line_bite_percent(line,fish_species_weight_g(&fish_species[index],fish_species[index].mean_x2_mm/2))/100;
        }
        weight*=4;
    }
    return (uint16_t)(weight>UINT16_MAX?UINT16_MAX:weight);
}
const uint8_t fish_map_base_priors[3]={70,27,3};
void fish_map_encounter_weights_on(unsigned map,int x,int hook,unsigned bait,unsigned minute,unsigned rain,unsigned line,bool lamp,uint16_t weights[FISH_SPECIES_COUNT])
{
    unsigned baseline[3]={0};
    enum {REFERENCE_BAIT=5}; /* Stable ordinary-plastic bait ID. */
    /* Calibrate eligible tiers before bait/tackle changes. A layer with one
     * bronze species must not become silver-heavy merely by species count.
     * Ordinary plastic bait at the same time/layer is the neutral reference. */
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) {
        unsigned tier=fish_species_profile(map,i)->rarity;
        if(tier<3)baseline[tier]+=fish_map_weight_on(map,i,x,hook,REFERENCE_BAIT,minute);
        weights[i]=fish_map_lamp_weight_on(map,i,x,hook,bait,minute,rain,line,lamp);
    }
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) {
        unsigned tier=fish_species_profile(map,i)->rarity;
        if(weights[i] && tier<3 && baseline[tier]) {
            unsigned n=(unsigned)weights[i]*fish_map_base_priors[tier]*32/baseline[tier];
            weights[i]=(uint16_t)(n>UINT16_MAX?UINT16_MAX:n?n:1);
        }
    }
}
bool fish_map_roll_lamp_on(uint32_t *state,unsigned map,int x,int hook,unsigned bait,unsigned minute,unsigned rain,unsigned line,bool lamp,uint8_t *index,uint16_t *size)
{
    uint16_t weights[FISH_SPECIES_COUNT];
    fish_map_encounter_weights_on(map,x,hook,bait,minute,rain,line,lamp,weights);
    bool found=fish_species_roll_weights_on(state,weights,map,index,size);
    if(found && bait==FISH_BAIT_LIFELIKE) {
        /* Two ordinary samples favour a larger individual, never bypass caps. */
        uint16_t other=fish_species_sample_size(state,fish_species_profile(map,*index));
        if(other>*size)*size=other;
    }
    return found;
}
