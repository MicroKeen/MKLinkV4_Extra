#ifndef FISH_RESOURCES_H
#define FISH_RESOURCES_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
enum {FISH_CACHE_BASE=0x80080000u,FISH_CACHE_BYTES=0x73000,
      FISH_CACHE_SECTOR=4096,FISH_PACK_HEADER=64,FISH_CACHE_DATA=0x80081000u};
typedef enum {FISH_RES_IDLE,FISH_RES_CACHE_CHECK,FISH_RES_SOURCE_CHECK,
    FISH_RES_ERASE_HEADER,FISH_RES_ERASE_BLOCK,FISH_RES_WRITE_BLOCK,
    FISH_RES_VERIFY,FISH_RES_COMMIT,FISH_RES_DONE,FISH_RES_ERROR} fish_res_phase_t;
void fish_resources_begin(unsigned map);
void fish_resources_tick(void);
bool fish_resources_valid(void);
unsigned fish_resources_map(void);
unsigned fish_resources_progress(void);
fish_res_phase_t fish_resources_phase(void);
/* Ports serialize filesystem access with saves. All offsets below are cache
 * relative; the port must independently enforce the complete physical bounds. */
int fish_resource_port_init(void);
int fish_resource_open(unsigned map,uint32_t *bytes);
int fish_resource_read(uint32_t offset,void *out,unsigned bytes);
void fish_resource_close(void);
const uint8_t *fish_resource_flash(void);
int fish_resource_erase(uint32_t offset);
int fish_resource_program(uint32_t offset,const void *data,unsigned bytes);
uint32_t fish_resource_crc(uint32_t crc,const void *data,unsigned bytes);
#endif
