#ifndef FISH_SD_GUARD_H
#define FISH_SD_GUARD_H
#include <stdint.h>
/* Read existing MBR/BPB only. Never create or change a partition/filesystem.
 * physical is the SDK's normalized 512-byte block_count, not raw C_SIZE. */
int fish_sd_guard_capacity(uint32_t physical,uint32_t *exported);
int fish_sd_guard_read(uint32_t sector,void *out);
#endif
