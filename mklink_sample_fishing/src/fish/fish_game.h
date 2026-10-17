#ifndef FISH_GAME_H
#define FISH_GAME_H
#include <stdbool.h>
#include <stdint.h>
#include "fish_species.h"
#include "fish_rules.h"
#include "fish_map.h"
#include "fish_equipment.h"
#include "fish_economy.h"

enum { FISH_WORLD_WIDTH = FISH_MAP_WIDTH, FISH_HOLD_MS = 280, FISH_GUIDE_HOLD_MS=600, FISH_FULL_CHARGE_MS = 1400,
       FISH_WORLD_CM_PER_PIXEL = 4, FISH_LOSS_ANIMATION_MS = 1200 };
typedef enum { FISH_MOVE, FISH_AIM, FISH_CHARGE, FISH_CAST,
               FISH_WAIT, FISH_BITE, FISH_FIGHT, FISH_CAUGHT, FISH_GUIDE,
               FISH_BAG, FISH_SHORE, FISH_LOST, FISH_SHOP, FISH_HOLD } fish_mode_t;
enum {FISH_BAG_ROD,FISH_BAG_LINE,FISH_BAG_FLOAT,FISH_BAG_BAIT,FISH_BAG_DEPTH,FISH_BAG_MISC,
      FISH_BAG_DOCK,FISH_BAG_GUIDE,FISH_BAG_BACK,FISH_BAG_SAVE,FISH_BAG_SOUND,FISH_BAG_COUNT};
enum {FISH_BAG_VISIBLE_COUNT=FISH_BAG_COUNT-1}; /* Dock ID reserved, no menu entry. */
typedef enum { FISH_LOSS_NONE, FISH_LOSS_MISSED, FISH_LOSS_BREAK,
               FISH_LOSS_SLACK, FISH_LOSS_RANGE, FISH_LOSS_EARLY } fish_loss_t;
typedef enum { FISH_ROTATE, FISH_PRESS, FISH_RELEASE } fish_event_t;
typedef enum {FISH_SOUND_OFF,FISH_SOUND_FISH,FISH_SOUND_ALL,FISH_SOUND_COUNT} fish_sound_t;
extern const char *const fish_sound_names[FISH_SOUND_COUNT];
typedef struct fish_game {
    fish_mode_t mode, previous_mode;
    int boat_x, facing, cast_x, cast_start_x, cast_distance, reel, catches;
    uint32_t now, pressed_at, phase_at;
    bool pressed;
    uint8_t guide_index, guide_return_mode,guide_page,sound_mode;
    uint8_t rod_index,line_index,float_index,bait_index,depth_index;
    uint8_t bag_index,bag_return_mode;
    bool bag_editing;
    bool save_requested;
    uint8_t struggle_percent; /* Once-per-hook disposition; fixed state, no heap. */
    int8_t boat_speed; /* -2/-1/0/+1/+2: two cruise gears and stop. */
    int16_t boat_motion_q8;
    int16_t shore_x;
    int8_t shore_speed;
    uint16_t shore_remainder;
    uint8_t shore_walk_phase;
    uint16_t cast_bottom_cm,cast_depth_cm;
    uint32_t random_state;
    uint8_t pending_species, last_species;
    uint8_t pending_variant;
    bool merchant_hint;
    uint16_t pending_size_mm, last_size_mm;
    uint32_t pending_weight_g, last_weight_g;
    uint8_t base_pull,line_response_percent;
    uint16_t bite_wait_ms; /* Fits the former alignment gap before simulation_at. */
    uint32_t simulation_at;
    uint16_t overload_ms, slack_ms, range_ms;
    uint32_t line_capacity_units;
    uint8_t line_strength;
    int tension, reel_units;
    uint16_t stamina,stamina_max,recovery_ms;
    bool stamina_recovering;
    bool run_burst;
    int8_t run_direction; /* +1 away from player, -1 toward player; cached per burst. */
    /* Q8 water contact position; updated with the existing 20 ms fight tick. */
    int32_t float_dx_q8, float_dy_q8;
    uint32_t motion_random_state;
    uint16_t motion_remaining_ms;
    int16_t motion_target_q8;
    fish_loss_t loss;
    uint16_t species_catches[FISH_SPECIES_COUNT];
    uint16_t species_best_size_mm[FISH_SPECIES_COUNT];
    fish_economy_t economy;
    fish_money_feedback_t money; /* Visual feedback only; never saved. */
    uint32_t clock_at;
    uint16_t game_minute,clock_remainder_ms;
    uint8_t shop_index,hold_return_mode,merchant_line_variant;
    uint32_t shop_closed_at;
    bool shop_closed_notice;
    fish_merchant_reply_t merchant_reply;
    fish_cargo_result_t cargo_result;
} fish_game_t;

void fish_game_init(fish_game_t *g, uint32_t now);
void fish_game_update(fish_game_t *g, uint32_t now);
void fish_game_input(fish_game_t *g, fish_event_t event, int steps, uint32_t now);
int fish_game_power(const fish_game_t *g);
int fish_game_camera(const fish_game_t *g);
int fish_game_actor_x(const fish_game_t *g);
unsigned fish_game_shore_pose(const fish_game_t *g);
bool fish_game_ashore(const fish_game_t *g);
bool fish_game_at_shop(const fish_game_t *g);
bool fish_game_at_station(const fish_game_t *g);
bool fish_game_shop_open(const fish_game_t *g);
unsigned fish_game_night_opacity(const fish_game_t *g);
int fish_game_dock_action(const fish_game_t *g);
uint16_t fish_game_rig_depth(const fish_game_t *g);
void fish_game_equip_line(fish_game_t *g,unsigned index);
int fish_game_pull(const fish_game_t *g, uint32_t elapsed_ms);
void fish_game_float(const fish_game_t *g, int *dx, int *dy);
int fish_game_distance(const fish_game_t *g);
unsigned fish_game_distance_cm(const fish_game_t *g);
unsigned fish_game_max_cast_px(const fish_game_t *g);
unsigned fish_game_rod_flex(const fish_game_t *g);
bool fish_game_burst_warning(const fish_game_t *g);
bool fish_game_burst_active(const fish_game_t *g);
bool fish_game_strike_ready(const fish_game_t *g);
int fish_game_splash(const fish_game_t *g);
#endif
