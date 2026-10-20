#ifndef FISH_MAP_H
#define FISH_MAP_H
#include <stdbool.h>
#include "fish_species.h"
#include "fish_map_config.h"
typedef struct {uint16_t x,depth_cm;} fish_depth_node_t;
typedef struct {uint16_t left,right;} fish_zone_t;
typedef struct {uint16_t deck_left,deck_right,boat_x,shore_x,walk_left,walk_right;} fish_dock_t;
typedef struct {
    uint16_t min_depth_cm,max_depth_cm;
    uint8_t layers,nearshore_only,zone_weights[FISH_ZONE_COUNT];
    uint8_t activity_percent[3]; /* day, dawn/dusk peak, night; game tuning. */
} fish_distribution_t;
extern const fish_depth_node_t fish_depth_profile[FISH_DEPTH_NODE_COUNT];
extern const fish_depth_node_t fish_city_depth_profile[FISH_DEPTH_NODE_COUNT];
extern const fish_zone_t fish_map_zones[FISH_ZONE_COUNT];
extern const fish_dock_t fish_docks[2];
extern const uint16_t fish_depth_options[FISH_DEPTH_OPTION_COUNT];
extern const uint16_t fish_city_depth_options[FISH_DEPTH_OPTION_COUNT];
extern const fish_distribution_t fish_distributions[FISH_SPECIES_COUNT];
int fish_map_depth(int world_x);
int fish_map_depth_on(unsigned map,int world_x);
int fish_map_zone(int world_x);
int fish_map_dock(int boat_x);
uint8_t fish_map_layer(int bottom_cm,int hook_cm);
uint16_t fish_map_weight(unsigned species,int world_x,int hook_cm,unsigned bait);
bool fish_map_roll(uint32_t *random,int world_x,int hook_cm,unsigned bait,uint8_t *index,uint16_t *size_mm);
unsigned fish_map_activity_percent(unsigned species,unsigned game_minute);
uint16_t fish_map_weight_at(unsigned species,int world_x,int hook_cm,unsigned bait,unsigned game_minute);
bool fish_map_roll_at(uint32_t *random,int world_x,int hook_cm,unsigned bait,unsigned game_minute,uint8_t *index,uint16_t *size_mm);
uint16_t fish_map_weight_on(unsigned map,unsigned species,int world_x,int hook_cm,unsigned bait,unsigned minute);
bool fish_map_roll_on(uint32_t *random,unsigned map,int world_x,int hook_cm,unsigned bait,unsigned minute,uint8_t *index,uint16_t *size_mm);
uint16_t fish_map_weather_weight_on(unsigned map,unsigned species,int world_x,int hook_cm,unsigned bait,unsigned minute,unsigned rain);
bool fish_map_roll_weather_on(uint32_t *random,unsigned map,int world_x,int hook_cm,unsigned bait,unsigned minute,unsigned rain,uint8_t *index,uint16_t *size_mm);
uint16_t fish_map_tackle_weight_on(unsigned map,unsigned species,int world_x,int hook_cm,unsigned bait,unsigned minute,unsigned rain,unsigned line);
bool fish_map_roll_tackle_on(uint32_t *random,unsigned map,int world_x,int hook_cm,unsigned bait,unsigned minute,unsigned rain,unsigned line,uint8_t *index,uint16_t *size_mm);
#endif
