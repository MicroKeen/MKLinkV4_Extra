#ifndef FISH_SHOP_INTERIOR_INDEXED_H
#define FISH_SHOP_INTERIOR_INDEXED_H
#include "fish_asset_layout.h"
/* Fixed-width indices reconstruct every original RGB565 pixel exactly. */
static inline uint16_t fish_shop_interior_pixel(unsigned x,unsigned y)
{
    const uint8_t *p=FISH_ASSET_POINTER(FISH_OFFSET_c_bmpShopInteriorIndices);
    const uint16_t *palette=(const uint16_t *)FISH_ASSET_POINTER(FISH_OFFSET_c_bmpShopInteriorPalette);
    unsigned bit=(y*120+x)*9;
    p+=bit/8;
    unsigned word=(unsigned)p[0]|(unsigned)p[1]<<8|(unsigned)p[2]<<16;
    return palette[(word>>(bit%8))&511];
}
#endif
