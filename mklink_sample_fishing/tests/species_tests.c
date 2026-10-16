#include "fish_species.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    unsigned seen=0; uint32_t state=42, repeat=42;
    for (int i=0;i<FISH_SPECIES_COUNT;i++) {
        const fish_species_t *s=&fish_species[i];
        assert(s->min_mm<=s->typical_min_mm && s->typical_min_mm<=s->typical_max_mm && s->typical_max_mm<=s->max_mm);
        assert(s->layers && (i>=FISH_BASE_SPECIES_COUNT || s->encounter_weight));
        assert(s->mean_x2_mm>=s->min_mm*2 && s->mean_x2_mm<=s->max_mm*2);
        assert(s->stddev_x2_mm>0);
        assert(fish_species_stars(s,s->min_mm)==1 && fish_species_stars(s,s->max_mm)==5);
        for (unsigned j=0;j<4;j++) {
            assert(fish_species_stars(s,s->star_min_mm[j]-1)==j+1);
            assert(fish_species_stars(s,s->star_min_mm[j])==j+2);
        }
        uint8_t previous=1;
        uint16_t previous_scale=0;
        for (unsigned size=s->min_mm;size<=s->max_mm;size++) {
            uint8_t stars=fish_species_stars(s,(uint16_t)size);
            assert(stars>=previous && stars<=5); previous=stars;
            uint16_t scale=fish_species_scale_q8(s,(uint16_t)size);
            assert(scale>=previous_scale && scale>0); previous_scale=scale;
            assert(s->portrait_width_px*scale/256<=220 && s->portrait_height_px*scale/256<=110);
        }
        assert(fish_species_scale_q8(s,s->min_mm)<fish_species_scale_q8(s,(s->mean_x2_mm+1)/2));
        for (int j=0;j<i;j++) assert(strcmp(s->id,fish_species[j].id));
    }
    assert(fish_species[9].size_metric==FISH_SIZE_CARAPACE_LENGTH);
    assert(fish_species[10].size_metric==FISH_SIZE_CARAPACE_WIDTH);
    unsigned carp_width=fish_species_scale_q8(&fish_species[1],625)*fish_species[1].portrait_width_px/256;
    unsigned catfish_width=fish_species_scale_q8(&fish_species[3],450)*fish_species[3].portrait_width_px/256;
    unsigned crucian_width=fish_species_scale_q8(&fish_species[0],200)*fish_species[0].portrait_width_px/256;
    assert(carp_width>=150 && catfish_width>crucian_width+40);
    assert(fish_species_scale_q8(&fish_species[3],1200)*fish_species[3].portrait_width_px/256>=218);
    printf("visible_pixels 4kg_carp=%u typical_catfish=%u typical_crucian=%u\n",carp_width,catfish_width,crucian_width);
    for (int i=0;i<30000;i++) {
        uint8_t species, again; uint16_t size, same;
        fish_species_roll(&state,&species,&size); fish_species_roll(&repeat,&again,&same);
        assert(species<FISH_SPECIES_COUNT && species==again && size==same);
        assert(size>=fish_species[species].min_mm && size<=fish_species[species].max_mm);
        seen|=1u<<species;
    }
    assert(seen==((1u<<FISH_BASE_SPECIES_COUNT)-1));
    /* Compare distribution shape and rarity per species, not just global averages. */
    puts("id,samples,mean_mm,star1,star2,star3,star4,star5,typical,min_endpoint,max_endpoint");
    for (unsigned i=0;i<FISH_SPECIES_COUNT;i++) {
        const fish_species_t *s=&fish_species[i];
        unsigned histogram[5]={0},typical=0,min_endpoint=0,max_endpoint=0;
        const unsigned samples=100000; uint64_t sum=0;
        uint32_t random=0x53491u+i;
        for (unsigned n=0;n<samples;n++) {
            uint16_t size=fish_species_sample_size(&random,s);
            assert(size>=s->min_mm && size<=s->max_mm);
            ++histogram[fish_species_stars(s,size)-1]; sum+=size;
            typical+=(size>=s->typical_min_mm && size<=s->typical_max_mm);
            min_endpoint+=(size==s->min_mm); max_endpoint+=(size==s->max_mm);
        }
        assert(histogram[4]>samples*7/1000 && histogram[4]<samples*20/1000);
        assert(histogram[1]+histogram[2]>samples*60/100);
        assert(typical>samples*60/100 && typical<samples*85/100);
        assert(min_endpoint<samples*3/100 && max_endpoint<samples*3/100);
        printf("%s,%u,%.6f,%u,%u,%u,%u,%u,%u,%u,%u\n",s->id,samples,(double)sum/samples,
            histogram[0],histogram[1],histogram[2],histogram[3],histogram[4],typical,min_endpoint,max_endpoint);
    }
    puts("PASS: ranges, deterministic rolls, star thresholds/monotonicity, 1.1M bounded bell-shaped samples");
}
