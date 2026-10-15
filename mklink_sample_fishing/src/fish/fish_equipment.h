#ifndef FISH_EQUIPMENT_H
#define FISH_EQUIPMENT_H
#include "fish_species.h"
enum {FISH_ROD_COUNT=3,FISH_LINE_COUNT=3,FISH_FLOAT_COUNT=3,FISH_BAIT_COUNT=6};
typedef struct {const char *name; uint16_t max_cast_px; uint8_t reel_percent;} fish_rod_t;
typedef struct {
    const char *name,*hook;
    uint16_t capacity_units,distance_limit_cm;
    uint8_t strength,strike_window_percent,diameter_hundredths_mm,response_percent;
    uint16_t comfortable_weight_g;
    uint8_t bite_percent[3]; /* <=100g, <=1000g, >1000g; game concealment tuning. */
} fish_line_set_t;
typedef struct {const char *name; uint16_t tip,body; uint8_t tip_height,body_height,night_bite_bonus_percent,strike_window_bonus_percent;} fish_float_t;
typedef struct {const char *name,*description; uint8_t weights[FISH_SPECIES_COUNT];} fish_bait_t;
extern const fish_rod_t fish_rods[FISH_ROD_COUNT];
extern const fish_line_set_t fish_line_sets[FISH_LINE_COUNT];
extern const fish_float_t fish_floats[FISH_FLOAT_COUNT];
extern const fish_bait_t fish_baits[FISH_BAIT_COUNT];
uint8_t fish_line_bite_percent(unsigned index,uint32_t weight_g);
unsigned fish_line_response_percent(unsigned index,uint32_t weight_g);
uint16_t fish_line_bite_wait(uint32_t *random_state,unsigned index,uint32_t weight_g);
uint16_t fish_line_bite_wait_bonus(uint32_t *random_state,unsigned index,uint32_t weight_g,unsigned bonus_percent);
#endif
