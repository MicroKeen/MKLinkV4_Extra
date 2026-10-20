#ifndef FISH_MEDIA_H
#define FISH_MEDIA_H
#include <stdbool.h>
#include <stdint.h>
#include "fish_resources.h"
#include "fish_diagnostics.h"
typedef enum {FISH_MEDIA_CHECK,FISH_MEDIA_WAIT,FISH_MEDIA_VERIFY,
    FISH_MEDIA_SETTLE,FISH_MEDIA_READY,FISH_MEDIA_ERROR,FISH_MEDIA_FAULT} fish_media_phase_t;
void fish_media_begin(uint32_t now);
void fish_media_require(uint32_t now);
void fish_media_tick(uint32_t now,bool save_busy);
void fish_media_retry(uint32_t now);
/* Called before every raw USB write, including FAT/directory metadata. The
 * polling MSC backend and read-only probe run on the same application thread. */
void fish_media_host_write(uint32_t now,uint32_t bytes);
typedef struct {
    fish_media_phase_t phase;
    unsigned progress,map,reset_stage;
    uint32_t received_bytes,elapsed_ms;
    fish_res_error_t error;
    bool receiving,watchdog_reset;
    fish_fault_info_t fault;
} fish_media_status_t;
fish_media_status_t fish_media_status(void);
bool fish_media_port_watchdog_reset(void);
unsigned fish_media_port_reset_stage(void);
void fish_media_port_reset_info(fish_fault_info_t *out);
bool fish_media_ready(void);
bool fish_media_visible(void);
fish_media_phase_t fish_media_phase(void);
unsigned fish_media_progress(void);
/* Port owns/unmounts SD before enumeration. Probes are read-only; writes abort
 * the probe before touching media. stop returns 1 while a USB command drains. */
int fish_media_port_start(void);
int fish_media_port_stop(void);
int fish_media_port_probe(bool enabled);
#endif
