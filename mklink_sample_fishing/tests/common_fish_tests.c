#include "fish_resources.h"
#include "fish_asset_layout.h"
#include "assets/species/species_assets.h"
#include <assert.h>
#include <stdio.h>
#include "fish_decode_expected.h"
static void finish(void)
{
    unsigned i=0,last=0;
    while(fish_resources_phase()!=FISH_RES_DONE && fish_resources_phase()!=FISH_RES_ERROR && i++<1000) {
        fish_resources_tick();unsigned p=fish_resources_progress();assert(p>=last && p<=100);last=p;
    }
    assert(i<1000 && fish_resources_valid());
}
static uint32_t pixels(const fish_species_art_t *art)
{
    unsigned n=art->sprite->tRegion.tSize.iWidth*art->sprite->tRegion.tSize.iHeight;
    assert(n<=96*56 && art->mask->tRegion.tSize.iWidth==art->sprite->tRegion.tSize.iWidth);
    uint32_t crc=fish_resource_crc(~0u,art->sprite->phwBuffer,n*2);
    return fish_resource_crc(crc,art->mask->pchBuffer,n)^~0u;
}
int main(void)
{
    uint32_t common_crc=0;
    for(unsigned map=0;map<3;map++) {
        fish_resources_begin(map);finish();assert(fish_resources_map()==map);
        uint32_t crc=fish_resource_crc(~0u,FISH_COMMON_POINTER(0),FISH_COMMON_PAYLOAD_BYTES)^~0u;
        assert(crc==FISH_COMMON_CRC);if(map)assert(crc==common_crc);common_crc=crc;
        for(unsigned id=0;id<FISH_SPECIES_COUNT;id++)for(unsigned large=0;large<2;large++)
            assert(pixels(fish_species_portrait_art(id,large?512:256))==expected_crc[id][large]);
        /* Stern and catch card remain resident together, including high-res
         * shapes. Re-selecting either cannot mutate the other's tile pair. */
        const fish_species_art_t *a=fish_species_portrait_art(17,512),*b=fish_species_portrait_art(1,256);
        assert(a!=b && pixels(a)==expected_crc[17][1] && pixels(b)==expected_crc[1][0]);
        assert(fish_species_portrait_art(17,512)==a && pixels(b)==expected_crc[1][0]);
    }
    puts("PASS: all fish normal/large RGB565+alpha byte identities on all3 maps; independent common cache; two simultaneous portraits");
}
