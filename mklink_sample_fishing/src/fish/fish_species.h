#ifndef FISH_SPECIES_H
#define FISH_SPECIES_H
#include <stdint.h>
#include <stdbool.h>
enum { FISH_BASE_SPECIES_COUNT=11,FISH_VARIANT_COUNT=4,FISH_LEGACY_SPECIES_COUNT=15,
       FISH_RIVER_SPECIES_COUNT=9,FISH_SPECIES_COUNT=50,FISH_GUIDE_COUNT=49,
       FISH_RETIRED_REDTAIL=21,FISH_BIGHEAD=24,FISH_COAST_FIRST=25,FISH_COAST_COUNT=25,
       FISH_OARFISH=35,FISH_FORAGE_FIRST=38,FISH_FORAGE_END=44,FISH_PEACOCK=44,FISH_SQUID=45,FISH_CHUB_MACKEREL=46,FISH_RAGWORM=47,FISH_HARP_SNAIL=48,FISH_GREEN_CRAB=49 };
unsigned fish_guide_index_at(unsigned position);
unsigned fish_guide_position(unsigned index);
static inline bool fish_species_is_variant(unsigned index) {return (index>=11 && index<15) || index==FISH_PEACOCK;}
static inline bool fish_species_is_colored(unsigned index) {return fish_species_is_variant(index) || index==FISH_OARFISH;}
static inline bool fish_species_is_foraging(unsigned index) {return (index>=FISH_FORAGE_FIRST && index<FISH_FORAGE_END) || (index>=FISH_RAGWORM && index<FISH_SPECIES_COUNT);}
unsigned fish_species_parent(unsigned index);
unsigned fish_species_special_roll(uint32_t *random_state,unsigned index);
unsigned fish_species_special_roll_cat(uint32_t *random_state,unsigned index);
uint32_t fish_random_next(uint32_t *random_state);
typedef enum { FISH_RARITY_BRONZE, FISH_RARITY_SILVER, FISH_RARITY_GOLD, FISH_RARITY_SPECIAL, FISH_RARITY_COUNT } fish_rarity_t;
typedef enum { FISH_SIZE_TOTAL_LENGTH, FISH_SIZE_CARAPACE_LENGTH, FISH_SIZE_CARAPACE_WIDTH,FISH_SIZE_SHELL_LENGTH,FISH_SIZE_MANTLE_LENGTH } fish_size_metric_t;
enum { FISH_LAYER_SURFACE=1, FISH_LAYER_MIDDLE=2, FISH_LAYER_LOWER=4, FISH_LAYER_BOTTOM=8, FISH_LAYER_SHORE=16 };
/* Ordinary gold pool ceiling per encounter attempt; special branches are separate. */
#define FISH_GOLD_POOL_CAP_PERCENT 5
typedef enum {FISH_FIGHT_ACTIVE,FISH_FIGHT_SLOW,FISH_FIGHT_PASSIVE} fish_fight_style_t;
typedef struct {
    const char *id, *name_zh, *scientific_name, *ui_name;
    uint16_t min_mm, max_mm, typical_min_mm, typical_max_mm;
    uint8_t layers, encounter_weight, size_metric;
    uint16_t mean_x2_mm, stddev_x2_mm;
    uint16_t star_min_mm[4]; /* Inclusive lower bounds for 2, 3, 4, 5 stars. */
    uint16_t weight_anchor_mm, weight_anchor_g; /* Game estimate, cubic scaling. */
    uint8_t bite_style, pull_base, burst_bonus, rarity;
    uint16_t bite_window_ms, fight_cycle_ms, burst_ms;
    uint8_t fight_style; /* Uses former tail padding; no per-species runtime object. */
    uint16_t strike_delay_ms,stamina_base;
    uint8_t portrait_width_px,portrait_height_px; /* Visible alpha bounds, not padded tile. */
    uint8_t portrait_centre_x,portrait_centre_y;
    uint8_t run_percent; /* Burst-only outward movement, 100 = original pace. */
    uint8_t coast_weight_index; /* 0 retains legacy cubic;1..22 is a reviewed endpoint fit. */
    uint8_t hook_pressure_percent,burst_force_extra; /* Use existing tail padding. */
} fish_species_t;
extern const fish_species_t fish_species[FISH_SPECIES_COUNT];
const fish_species_t *fish_species_profile(unsigned map,unsigned index);
extern const char *const fish_species_remarks[FISH_SPECIES_COUNT];
extern const uint16_t fish_rarity_colors[FISH_RARITY_COUNT];
extern const char *const fish_rarity_labels[FISH_RARITY_COUNT];
/* Millimetres and stable IDs are shared with data/species.json. */
void fish_species_roll(uint32_t *random_state, uint8_t *species_index, uint16_t *size_mm);
bool fish_species_roll_weights(uint32_t *random_state,const uint16_t *weights,uint8_t *species_index,uint16_t *size_mm);
bool fish_species_roll_weights_on(uint32_t *random_state,const uint16_t *weights,unsigned map,uint8_t *species_index,uint16_t *size_mm);
uint16_t fish_species_sample_size(uint32_t *random_state, const fish_species_t *species);
uint8_t fish_species_stars(const fish_species_t *species, uint16_t size_mm);
/* Shared compressed size scale, fitted by visible alpha bounds; Q8 unity=256. */
uint16_t fish_species_scale_q8(const fish_species_t *species, uint16_t size_mm);
uint32_t fish_species_weight_g(const fish_species_t *species, uint16_t size_mm);
uint8_t fish_species_pull(const fish_species_t *species, uint32_t weight_g);
uint16_t fish_species_stamina(const fish_species_t *species,uint32_t weight_g);
#endif
