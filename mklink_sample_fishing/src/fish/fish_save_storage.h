#ifndef FISH_SAVE_STORAGE_H
#define FISH_SAVE_STORAGE_H
#include <stddef.h>
#include <stdint.h>
/* Storage owns async worker buffers. Boot reads run before the scene starts.
 * read slots0/1 are V6, slots2/3 are V5, slots4/5 are V1-V4 (read-only).
 * read: 0 exact file, -2 absent, -3 wrong size, -1 IO error.
 * begin: 0 accepted; poll: 0 pending, 1 durable+readback verified, -1 failed.
 * begin must not modify the currently active slot. */
int fish_storage_init(uint8_t key[32]);
int fish_storage_read(unsigned slot,uint8_t *file,size_t len);
int fish_storage_begin(unsigned slot,const uint8_t *file,size_t len);
int fish_storage_poll(void);
#endif
