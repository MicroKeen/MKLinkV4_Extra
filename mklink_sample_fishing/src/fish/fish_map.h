#ifndef FISH_MAP_H
#define FISH_MAP_H
#include <stdbool.h>
#include "fish_species.h"
#include "fish_map_config.h"
typedef struct {uint16_t x,depth_cm;} fish_depth_node_t;
typedef struct {uint16_t left,right;} fish_zone_t;
typedef struct {uint16_t deck_left,deck_right,boat_x,shore_x;int16_t walk_left,walk_right;} fish_dock_t;
typedef struct {
    uint16_t min_depth_cm,max_depth_cm;
    uint8_t layers,nearshore_only,zone_weights[FISH_ZONE_COUNT];
    uint8_t activity_percent[3]; /* day, dawn/dusk peak, night; game tuning. */
} fish_distribution_t;
extern const fish_depth_node_t fish_depth_profile[FISH_DEPTH_NODE_COUNT];
extern const fish_depth_node_t fish_city_depth_profile[FISH_DEPTH_NODE_COUNT];
extern const fish_zone_t fish_map_zones[FISH_ZONE_COUNT];
extern const fish_dock_t fish_docks[2];
extern const fish_depth_node_t fish_beach_depth_profile[FISH_DEPTH_NODE_COUNT];
const fish_dock_t *fish_map_dock_at(unsigned map,unsigned side);
int fish_map_dock_on(unsigned map,int boat_x);
static inline int fish_map_width(unsigned map){return map==2?FISH_BEACH_WORLD_WIDTH:FISH_MAP_WIDTH;}
static inline int fish_map_water_left(unsigned map){return map==2?480:FISH_WATER_LEFT;}
static inline int fish_map_water_right(unsigned map){return map==2?FISH_BEACH_WORLD_WIDTH:FISH_WATER_RIGHT;}
static inline int fish_map_boat_left(unsigned map){return map==2?540:FISH_BOAT_LEFT;}
static inline int fish_map_boat_right(unsigned map){return map==2?FISH_BEACH_BOAT_RIGHT:FISH_BOAT_RIGHT;}
static inline int fish_map_station(unsigned map){return map==2?FISH_BEACH_STATION_X:FISH_STATION_X;}
static inline int fish_map_dock_count(unsigned map){return map==2?1:2;}
static inline int fish_map_max_depth(unsigned map){return map==2?FISH_BEACH_MAX_DEPTH_CM:map?FISH_CITY_MAX_DEPTH_CM:FISH_MAX_DEPTH_CM;}
static inline const fish_depth_node_t *fish_map_profile(unsigned map){return map==2?fish_beach_depth_profile:map?fish_city_depth_profile:fish_depth_profile;}
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
bool fish_map_coast_special(uint32_t *random,int world_x,int hook_cm,unsigned bait,unsigned rain,unsigned strength,bool cat,uint8_t *index,uint16_t *size_mm);
uint16_t fish_map_weather_weight_on(unsigned map,unsigned species,int world_x,int hook_cm,unsigned bait,unsigned minute,unsigned rain);
bool fish_map_roll_weather_on(uint32_t *random,unsigned map,int world_x,int hook_cm,unsigned bait,unsigned minute,unsigned rain,uint8_t *index,uint16_t *size_mm);
uint16_t fish_map_tackle_weight_on(unsigned map,unsigned species,int world_x,int hook_cm,unsigned bait,unsigned minute,unsigned rain,unsigned line);
bool fish_map_roll_tackle_on(uint32_t *random,unsigned map,int world_x,int hook_cm,unsigned bait,unsigned minute,unsigned rain,unsigned line,uint8_t *index,uint16_t *size_mm);
bool fish_map_roll_lamp_on(uint32_t *random,unsigned map,int world_x,int hook_cm,unsigned bait,unsigned minute,unsigned rain,unsigned line,bool lamp,uint8_t *index,uint16_t *size_mm);
uint16_t fish_map_lamp_weight_on(unsigned map,unsigned index,int world_x,int hook_cm,unsigned bait,unsigned minute,unsigned rain,unsigned line,bool lamp);
static inline bool fish_map_night(unsigned minute) {return minute%1440<360 || minute%1440>=1080;}
static inline bool fish_map_light_target(unsigned species)
{return species==34 || species==FISH_SQUID || species==FISH_CHUB_MACKEREL;}
void fish_map_encounter_weights_on(unsigned map,int x,int hook,unsigned bait,unsigned minute,unsigned rain,unsigned line,bool lamp,uint16_t weights[FISH_SPECIES_COUNT]);
extern const uint8_t fish_map_base_priors[3]; /* Bronze/silver/gold, before bait and tackle. */
#endif
