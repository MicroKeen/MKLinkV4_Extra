#ifndef FISH_EQUIPMENT_H
#define FISH_EQUIPMENT_H
#include "fish_species.h"
enum {FISH_ROD_COUNT=5,FISH_LONG_ROD=3,FISH_HEAVY_SEA_ROD=4,FISH_LINE_COUNT=5,FISH_PE_LINE=3,FISH_MARINE_LINE=4,FISH_FLOAT_COUNT=3,FISH_BAIT_COUNT=10,
      FISH_BAIT_SNAIL=6,FISH_BAIT_CORN=7,FISH_BAIT_LIFELIKE=8,FISH_BAIT_RAGWORM=9};
static inline bool fish_tackle_compatible(unsigned rod,unsigned line) {return rod!=0 || line<2;}
typedef struct {const char *name; uint16_t max_cast_px; uint8_t reel_percent,reel_tension_relief;} fish_rod_t;
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
unsigned fish_line_span_cm(unsigned horizontal_cm,unsigned depth_cm);
bool fish_line_bottom_allowed(unsigned line,unsigned bottom_cm);
/* Requested layer:0=surface,1=middle,2=bottom; resolve toward shallower. */
unsigned fish_line_resolve_layer(unsigned line,unsigned bottom_cm,unsigned horizontal_cm,unsigned requested);
#endif
