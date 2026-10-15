#include "fish_hpm_input.h"
#include "perf_counter.h"

uint32_t fish_hpm_millis(void)
{
    return (uint32_t)perfc_convert_ticks_to_ms(get_system_ticks());
}

bool fish_hpm_handle_message(ref_gui_t *gui,ngy_msg_core_t *message)
{
    (void)gui;
    static uint16_t previous_wheel;
    uint32_t now=fish_hpm_millis();
    unsigned id=ngy_helper_msg_item_get_id(message);
    if (id==NGY_MSG_GESTURE_EVT_WHEEL) {
        const ngy_msg_gesture_evt_t *wheel=(const ngy_msg_gesture_evt_t *)message;
        uint16_t value=(uint16_t)wheel->tRegion.tLocation.iY;
        int steps=(int16_t)(uint16_t)(value-previous_wheel);
        previous_wheel=value;
        fish_scene_input(FISH_ROTATE,steps*FISH_ENCODER_DIRECTION,now);
        return true;
    }
    if (id>=NGY_MSG_KEY_EVT_DOWN && id<=NGY_MSG_KEY_EVT_DOUBLE_PRESSED) {
        const ngy_msg_key_evt_t *key=(const ngy_msg_key_evt_t *)message;
        if (key->hwKeyValue!=RG_KEY_DOWN) return false;
        if (id==NGY_MSG_KEY_EVT_DOWN) fish_scene_input(FISH_PRESS,0,now);
        else if (id==NGY_MSG_KEY_EVT_UP) fish_scene_input(FISH_RELEASE,0,now);
        /* Ignore frontend repeats/clicks: game owns short/long-press semantics. */
        return true;
    }
    return false;
}
