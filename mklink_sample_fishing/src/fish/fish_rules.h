#ifndef FISH_RULES_H
#define FISH_RULES_H
#include <stdint.h>

/* Immutable tuning table. Tackle limits are copied into game state at init;
 * a future line item can replace those two values without changing fish data. */
typedef struct {
    uint16_t overload_limit_ms, overload_recovery_per_tick_ms;
    uint16_t slack_limit_ms, slack_recovery_per_tick_ms;
    uint16_t range_limit_ms, range_recovery_per_tick_ms;
    uint16_t default_line_capacity_units, burst_warning_ms, hook_settle_ms,bite_attempt_ms;
    uint16_t stamina_max,stamina_recovery_delay_ms,stamina_recovery_interval_ms;
    uint8_t default_line_strength, range_warning_percent, ease_tension_per_detent;
    uint8_t fatigue_max_percent,overweight_response_gain,max_line_response_percent,bite_attempt_limit;
    uint8_t stamina_weight_scale,stamina_rest_cost,stamina_burst_cost,stamina_min_tension_percent,stamina_resume_percent;
} fish_rules_t;
extern const fish_rules_t fish_rules;
#endif
