/* Host test boundary only: no GPIO, ISR, DMA or actual GUI queue is emulated. */
#ifndef TEST_REF_GUI_H
#define TEST_REF_GUI_H
#include <stdbool.h>
#include <stdint.h>
typedef struct {int unused;} ref_gui_t;
typedef struct {unsigned id;} ngy_msg_core_t;
typedef struct {ngy_msg_core_t core;uint16_t hwKeyValue;} ngy_msg_key_evt_t;
typedef struct {ngy_msg_core_t core;struct {struct {int16_t iX,iY;} tLocation;} tRegion;} ngy_msg_gesture_evt_t;
enum {NGY_MSG_KEY_EVT_DOWN=0xb0,NGY_MSG_KEY_EVT_REPEAT,NGY_MSG_KEY_EVT_UP,
    NGY_MSG_KEY_EVT_PRESSED,NGY_MSG_KEY_EVT_LONG_PRESSING,NGY_MSG_KEY_EVT_DOUBLE_PRESSED,
    NGY_MSG_GESTURE_EVT_WHEEL=0xc1,RG_KEY_DOWN=0x51};
static inline unsigned ngy_helper_msg_item_get_id(ngy_msg_core_t *m){return m->id;}
bool rg_send_key_evt(ref_gui_t *gui,uint8_t event,uint16_t key);
#endif
