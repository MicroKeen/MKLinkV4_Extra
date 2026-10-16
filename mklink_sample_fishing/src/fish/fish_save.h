#ifndef FISH_SAVE_H
#define FISH_SAVE_H
#include <stddef.h>
#include "fish_game.h"
enum {FISH_SAVE_VERSION=6,FISH_SAVE_EQUIPMENT_EPOCH=1,
      FISH_SAVE_INTERVAL_MS=300000,FISH_SAVE_LEGACY_PAYLOAD_BYTES=60,
      FISH_SAVE_LEGACY_FILE_BYTES=32+FISH_SAVE_LEGACY_PAYLOAD_BYTES,
      FISH_SAVE_PREVIOUS_PAYLOAD_BYTES=204,FISH_SAVE_PREVIOUS_FILE_BYTES=236,
      FISH_SAVE_V5_PAYLOAD_BYTES=204+14*FISH_VARIANT_COUNT,FISH_SAVE_V5_FILE_BYTES=32+FISH_SAVE_V5_PAYLOAD_BYTES,
      FISH_SAVE_PAYLOAD_BYTES=FISH_SAVE_V5_PAYLOAD_BYTES+1,FISH_SAVE_FILE_BYTES=32+FISH_SAVE_PAYLOAD_BYTES};
typedef enum {FISH_SAVE_IDLE,FISH_SAVE_SAVING,FISH_SAVE_OK,FISH_SAVE_ERROR,
              FISH_SAVE_RECOVERED,FISH_SAVE_INVALID} fish_save_status_t;
typedef struct {
    uint8_t key[32],saved[FISH_SAVE_PAYLOAD_BYTES],pending[FISH_SAVE_PAYLOAD_BYTES];
    uint8_t file[FISH_SAVE_FILE_BYTES];
    uint64_t sequence;
    int active_slot;
    uint32_t auto_at,status_at;
    fish_save_status_t status;
    fish_save_status_t blocked_status;
    bool writing,blocked,manual_pending;
} fish_save_t;
void fish_save_pack(const fish_game_t *g,uint8_t payload[FISH_SAVE_PAYLOAD_BYTES]);
bool fish_save_unpack(fish_game_t *g,const uint8_t payload[FISH_SAVE_PAYLOAD_BYTES]);
void fish_save_encode(const uint8_t key[32],uint64_t sequence,const uint8_t *payload,uint8_t *file);
bool fish_save_decode(const uint8_t key[32],const uint8_t *file,size_t len,uint64_t *sequence,uint8_t *payload);
void fish_save_init(fish_save_t *s,fish_game_t *g,uint32_t now);
void fish_save_tick(fish_save_t *s,fish_game_t *g,uint32_t now);
/* Cached status only; no filesystem calls from a draw handler. */
const char *fish_save_label(const fish_save_t *s);
#endif
