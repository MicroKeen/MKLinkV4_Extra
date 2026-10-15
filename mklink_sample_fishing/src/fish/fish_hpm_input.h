#ifndef FISH_HPM_INPUT_H
#define FISH_HPM_INPUT_H
#include "ref_gui.h"
#include "fish_game.h"
uint32_t fish_hpm_millis(void);
bool fish_hpm_handle_message(ref_gui_t *gui,ngy_msg_core_t *message);
void fish_scene_input(fish_event_t event,int steps,uint32_t timestamp);
#ifndef FISH_ENCODER_DIRECTION
#define FISH_ENCODER_DIRECTION 1
#endif
#endif
