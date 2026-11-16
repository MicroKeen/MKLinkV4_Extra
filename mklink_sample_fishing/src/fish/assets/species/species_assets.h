#ifndef FISH_SPECIES_ASSETS_H
#define FISH_SPECIES_ASSETS_H
#include "arm_2d.h"
#include "fish_species.h"
typedef struct {const arm_2d_tile_t *sprite,*mask;} fish_species_art_t;
const fish_species_art_t *fish_species_portrait_art(unsigned index,uint16_t scale_q8);
#endif
