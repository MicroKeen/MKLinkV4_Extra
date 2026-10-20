#ifndef FISH_HPM_RUNTIME_H
#define FISH_HPM_RUNTIME_H
#include <stdbool.h>
#include "fish_diagnostics.h"
void fish_hpm_capture_reset(void);
void fish_hpm_watchdog_ready(void);
void fish_hpm_storage_heartbeat(void);
bool fish_hpm_watchdog_reset(void);
enum {FISH_HPM_LOAD_IDLE,FISH_HPM_LOAD_CONFIGURE,FISH_HPM_LOAD_PROPERTIES,
    FISH_HPM_LOAD_ERASE,FISH_HPM_LOAD_PROGRAM,FISH_HPM_FS_WAIT,
    FISH_HPM_SD_OPEN,FISH_HPM_SD_SEEK,FISH_HPM_SD_READ,FISH_HPM_SD_CLOSE,
    FISH_HPM_SD_READ_DONE,FISH_HPM_SD_OPEN_DONE,FISH_HPM_SD_CLOSE_DONE,
    FISH_HPM_GUI,FISH_HPM_MEDIA,FISH_HPM_CPU_FAULT};
void fish_hpm_load_stage(unsigned stage);
void fish_hpm_load_stage_at(unsigned stage,uint32_t detail);
unsigned fish_hpm_reset_stage(void);
void fish_hpm_reset_info(fish_fault_info_t *out);
void fish_hpm_record_fault(uint32_t cause,uint32_t pc,uint32_t value);
#endif
