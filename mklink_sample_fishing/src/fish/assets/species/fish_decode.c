#include "species_assets.h"
#include "fish_asset_layout.h"
#include <string.h>

typedef struct {uint32_t offset;uint8_t width,height,bits;} fish_packed_t;
#include "fish_packed_data.h"
enum {PIXELS=96*56,SLOTS=2};
/* Two concurrent portraits cover stern fish + catch card. Fixed AHB SRAM,
 * separate from PFB/DLM/heaps; no allocator or per-frame full-screen buffer. */
static uint16_t rgb[SLOTS][PIXELS] __attribute__((section(".ahb_sram.fish_rgb"),aligned(32)));
static uint8_t alpha[SLOTS][PIXELS] __attribute__((section(".ahb_sram.fish_alpha"),aligned(32)));
static struct {arm_2d_tile_t image,mask;fish_species_art_t art;uint8_t key;bool valid;} slots[SLOTS];
static unsigned victim;
const fish_species_art_t *fish_species_portrait_art(unsigned index,uint16_t scale_q8)
{
    if(index>=FISH_SPECIES_COUNT)index=0;
    unsigned key=(scale_q8>256?large_ids:normal_ids)[index];
    for(unsigned i=0;i<SLOTS;i++)if(slots[i].valid && slots[i].key==key){victim=i^1u;return &slots[i].art;}
    unsigned n=victim;victim^=1u;
    const fish_packed_t *p=&packed[key];unsigned count=(unsigned)p->width*p->height;
    const uint8_t *data=FISH_COMMON_POINTER(p->offset),*indices=data+64,*bits=indices+(count*p->bits+7)/8;
    for(unsigned i=0;i<count;i++) {
        unsigned bit=i*p->bits,at=bit/8,v=indices[at];
        if(bit%8+p->bits>8)v|=(unsigned)indices[at+1]<<8;
        unsigned a=(bits[i/8]>>(i%8))&1u,k=(v>>(bit%8))&((1u<<p->bits)-1);
        alpha[n][i]=(uint8_t)(a?255:0);rgb[n][i]=(uint16_t)(data[2*k]|(unsigned)data[2*k+1]<<8);
    }
    slots[n].image=(arm_2d_tile_t){.tRegion={.tSize={p->width,p->height}},
        .tInfo={.bIsRoot=true,.bHasEnforcedColour=true,.tColourInfo={.chScheme=ARM_2D_COLOUR_RGB565}},.phwBuffer=rgb[n]};
    slots[n].mask=(arm_2d_tile_t){.tRegion={.tSize={p->width,p->height}},
        .tInfo={.bIsRoot=true,.bHasEnforcedColour=true,.tColourInfo={.chScheme=ARM_2D_COLOUR_GRAY8}},.pchBuffer=alpha[n]};
    slots[n].art=(fish_species_art_t){&slots[n].image,&slots[n].mask};slots[n].key=(uint8_t)key;slots[n].valid=true;
    return &slots[n].art;
}
