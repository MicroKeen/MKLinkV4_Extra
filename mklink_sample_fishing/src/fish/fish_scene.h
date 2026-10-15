#ifndef FISH_SCENE_H
#define FISH_SCENE_H
#include "arm_2d_helper.h"
#include "fish_game.h"
void fish_scene_init(arm_2d_scene_player_t *player);
void fish_scene_memory_layout(void);
/* Deterministic rendering checks use the same scene and PFB path. */
extern int fish_smoke_test;
#endif
