#include "fish_map.h"
#include "fish_equipment.h"

int fish_map_depth(int x)
{return fish_map_depth_on(0,x);}
int fish_map_depth_on(unsigned map,int x)
{
    const fish_depth_node_t *profile=map?fish_city_depth_profile:fish_depth_profile;
    if (x<=FISH_WATER_LEFT || x>=FISH_WATER_RIGHT) return 0;
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
    uint16_t weights[FISH_SPECIES_COUNT];
    for (unsigned i=0;i<FISH_SPECIES_COUNT;i++) weights[i]=fish_map_weight(i,x,hook,bait);
    return fish_species_roll_weights(random,weights,index,size);
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
    uint16_t weights[FISH_SPECIES_COUNT];
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++) weights[i]=fish_map_weight_at(i,x,hook,bait,minute);
    return fish_species_roll_weights(random,weights,index,size);
}
extern const fish_distribution_t fish_city_distributions[FISH_SPECIES_COUNT];
extern const uint8_t fish_city_encounters[FISH_SPECIES_COUNT],fish_city_baits[FISH_BAIT_COUNT][FISH_SPECIES_COUNT];
uint16_t fish_map_weight_on(unsigned map,unsigned index,int x,int hook,unsigned bait,unsigned minute)
{
    if(!map)return fish_map_weight_at(index,x,hook,bait,minute);
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
    unsigned weight=fish_map_weather_weight_on(map,index,x,hook,bait,minute,rain);
    if(weight && line==FISH_PE_LINE) {
        const fish_species_t *s=fish_species_profile(map,index);
        weight=weight*fish_line_bite_percent(line,fish_species_weight_g(s,s->mean_x2_mm/2))/100;
        if(!weight)weight=1;
    }
    return (uint16_t)weight;
}
bool fish_map_roll_tackle_on(uint32_t *state,unsigned map,int x,int hook,unsigned bait,unsigned minute,unsigned rain,unsigned line,uint8_t *index,uint16_t *size)
{
    uint16_t weights[FISH_SPECIES_COUNT];
    for(unsigned i=0;i<FISH_SPECIES_COUNT;i++)weights[i]=fish_map_tackle_weight_on(map,i,x,hook,bait,minute,rain,line);
    bool found=fish_species_roll_weights_on(state,weights,map,index,size);
    if(found && bait==FISH_BAIT_LIFELIKE) {
        /* Two ordinary samples favour a larger individual, never bypass caps. */
        uint16_t other=fish_species_sample_size(state,fish_species_profile(map,*index));
        if(other>*size)*size=other;
    }
    return found;
}
