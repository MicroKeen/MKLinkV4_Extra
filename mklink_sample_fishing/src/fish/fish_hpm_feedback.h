#ifndef FISH_HPM_FEEDBACK_H
#define FISH_HPM_FEEDBACK_H
#include "fish_game.h"

/* Call from the game thread, after the RT-Thread kernel has started. */
void fish_hpm_feedback_init(void);
void fish_hpm_feedback_update(const fish_game_t *game);
#endif
