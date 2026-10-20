#include "fish_species.h"
#include "fish_rules.h"

uint32_t fish_random_next(uint32_t *state)
{
    uint32_t value=*state ? *state : 0x6d2b79f5u;
    value^=value<<13; value^=value>>17; value^=value<<5;
    *state=value;
    return value;
}
#define next_random fish_random_next
static const uint8_t guide_order[FISH_GUIDE_COUNT]={
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,24,19,20,22,23};
unsigned fish_guide_index_at(unsigned position) {return guide_order[position%FISH_GUIDE_COUNT];}
unsigned fish_guide_position(unsigned index) {
    for(unsigned i=0;i<FISH_GUIDE_COUNT;i++)if(guide_order[i]==index)return i;
    return 0;
}
extern const fish_species_t fish_city_profiles[4];
extern const fish_species_t fish_legacy_city_silver;
const fish_species_t *fish_species_profile(unsigned map,unsigned index)
{
    if(map==2 && index==18)return &fish_legacy_city_silver;
    if(map) {
        if(index==0)return &fish_city_profiles[0];
        if(index==1)return &fish_city_profiles[1];
        if(index==7)return &fish_city_profiles[2];
        if(index==13)return &fish_city_profiles[3];
    }
    return &fish_species[index<FISH_SPECIES_COUNT?index:0];
}
unsigned fish_species_parent(unsigned index)
{
    static const uint8_t parent[FISH_VARIANT_COUNT]={3,2,1,9};
    return fish_species_is_variant(index)?parent[index-FISH_BASE_SPECIES_COUNT]:index;
}
unsigned fish_species_special_roll(uint32_t *state,unsigned index)
{
    for(unsigned v=FISH_BASE_SPECIES_COUNT;v<FISH_LEGACY_SPECIES_COUNT;v++)
        if(fish_species_parent(v)==index) return next_random(state)%100==0?v:index;
    return index;
}

void fish_species_roll(uint32_t *state, uint8_t *index, uint16_t *size_mm)
{
    unsigned total=0;
    for (unsigned i=0;i<FISH_SPECIES_COUNT;i++) total+=fish_species[i].encounter_weight;
    unsigned ticket=next_random(state)%total;
    *index=0;
    for (unsigned i=0;i<FISH_SPECIES_COUNT;i++) {
        if (ticket<fish_species[i].encounter_weight) { *index=(uint8_t)i; break; }
        ticket-=fish_species[i].encounter_weight;
    }
    *size_mm=fish_species_sample_size(state,&fish_species[*index]);
}

bool fish_species_roll_weights(uint32_t *state,const uint16_t *weights,uint8_t *index,uint16_t *size)
{return fish_species_roll_weights_on(state,weights,0,index,size);}
bool fish_species_roll_weights_on(uint32_t *state,const uint16_t *weights,unsigned map,uint8_t *index,uint16_t *size)
{
    unsigned total=0;
    for (unsigned i=0;i<FISH_SPECIES_COUNT;i++) total+=weights[i];
    if (!total) return false; /* Empty water does not fall back to global fish. */
    unsigned ticket=next_random(state)%total;
    for (unsigned i=0;i<FISH_SPECIES_COUNT;i++) {
        if (ticket<weights[i]) {
            *index=(uint8_t)i; *size=fish_species_sample_size(state,fish_species_profile(map,i)); return true;
        }
        ticket-=weights[i];
    }
    return false;
}

uint16_t fish_species_sample_size(uint32_t *state, const fish_species_t *species)
{
    /* Sum of 12 U(0..255): mean 1530, stddev sqrt(65535) ~= 256.
     * Integer-only normal approximation, then reject outside the species bounds.
     * Half-millimetre parameters avoid losing precision for small crabs/gobies.
     */
    for (unsigned attempt=0;attempt<32;attempt++) {
        int32_t sum=0;
        for (unsigned i=0;i<12;i++) sum+=(int32_t)(next_random(state)>>24);
        int32_t numerator=(int32_t)species->mean_x2_mm*256
            +(sum-1530)*(int32_t)species->stddev_x2_mm;
        int32_t size=(numerator+256)/512;
        if (size>=species->min_mm && size<=species->max_mm) return (uint16_t)size;
    }
    /* Hard work bound for an embedded input event; valid models have tiny fallback probability. */
    return (species->mean_x2_mm+1)/2;
}

uint8_t fish_species_stars(const fish_species_t *species, uint16_t size_mm)
{
    uint8_t stars=1;
    for (unsigned i=0;i<4;i++) if (size_mm>=species->star_min_mm[i]) ++stars;
    return stars;
}

static uint32_t isqrt_u32(uint32_t value)
{
    uint32_t root=0,bit=1u<<30;
    while (bit>value) bit>>=2;
    while (bit) {
        if (value>=root+bit) { value-=root+bit; root=(root>>1)+bit; }
        else root>>=1;
        bit>>=2;
    }
    return root;
}

uint16_t fish_species_scale_q8(const fish_species_t *species, uint16_t size_mm)
{
    unsigned width=32+(unsigned)size_mm*192/1000;
    if (width>220) width=220;
    unsigned scale=width*256/species->portrait_width_px;
    unsigned height_limit=110u*256/species->portrait_height_px;
    return (uint16_t)(scale<height_limit?scale:height_limit);
}

uint16_t fish_species_stamina(const fish_species_t *species,uint32_t weight_g)
{
    unsigned scale=species->fight_style==FISH_FIGHT_ACTIVE?fish_rules.stamina_weight_scale:1;
    unsigned stamina=species->stamina_base+isqrt_u32(weight_g*4u)*scale;
    return (uint16_t)(stamina<fish_rules.stamina_max?stamina:fish_rules.stamina_max);
}

uint32_t fish_species_weight_g(const fish_species_t *species, uint16_t size_mm)
{
    /* Game anchor, not a measured species/population regression. One cast only. */
    uint64_t numerator=(uint64_t)species->weight_anchor_g*size_mm*size_mm*size_mm;
    uint64_t denominator=(uint64_t)species->weight_anchor_mm*species->weight_anchor_mm*species->weight_anchor_mm;
    uint32_t weight=(uint32_t)((numerator+denominator/2)/denominator);
    return weight?weight:1;
}

uint8_t fish_species_pull(const fish_species_t *species, uint32_t weight_g)
{
    uint32_t root=isqrt_u32(weight_g*4u);
    unsigned pull=species->pull_base*(12+root)/100;
    return pull<5?5:pull>90?90:(uint8_t)pull;
}
