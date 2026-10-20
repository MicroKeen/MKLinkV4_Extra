#include "fish_hpm_runtime.h"
#include "board.h"
#include "hpm_ppor_drv.h"
#include "rtthread.h"
#include <assert.h>
#include <stdio.h>
static uint32_t flags;
static unsigned fed,irq_depth;
uint32_t ppor_reset_get_flags(void *p) {assert(p==HPM_PPOR);return flags;}
void ppor_reset_clear_flags(void *p,uint32_t bits) {assert(p==HPM_PPOR && bits==flags);flags=0;}
void ewdg_refresh(void *p) {assert(p==HPM_EWDG0 && irq_depth==1);++fed;}
rt_base_t rt_hw_interrupt_disable(void) {assert(!irq_depth);++irq_depth;return 0x55;}
void rt_hw_interrupt_enable(rt_base_t saved) {assert(irq_depth==1 && saved==0x55);--irq_depth;}
int main(void)
{
    fish_hpm_storage_heartbeat();assert(!fed && !irq_depth);
    flags=ppor_reset_wdog0;fish_hpm_capture_reset();assert(fish_hpm_watchdog_reset() && !flags);
    fish_hpm_watchdog_ready();assert(fed==1 && !irq_depth);
    fish_hpm_storage_heartbeat();assert(fed==2 && !irq_depth);
    flags=1u<<31;fish_hpm_capture_reset();assert(!fish_hpm_watchdog_reset() && !flags);
    flags=ppor_reset_pmic_wdog;fish_hpm_capture_reset();assert(fish_hpm_watchdog_reset() && !flags);
    flags=0;fish_hpm_capture_reset();assert(!fish_hpm_watchdog_reset());
    fish_hpm_load_stage(FISH_HPM_LOAD_CONFIGURE);flags=ppor_reset_wdog0;
    fish_hpm_capture_reset();assert(fish_hpm_reset_stage()==FISH_HPM_LOAD_CONFIGURE);
    flags=ppor_reset_wdog0;fish_hpm_capture_reset();assert(!fish_hpm_reset_stage());
    fish_hpm_load_stage(FISH_HPM_LOAD_ERASE);flags=ppor_reset_wdog0;
    fish_hpm_capture_reset();assert(fish_hpm_reset_stage()==FISH_HPM_LOAD_ERASE);
    fish_hpm_load_stage(FISH_HPM_LOAD_PROGRAM);flags=1u<<31;
    fish_hpm_capture_reset();assert(!fish_hpm_reset_stage());
    fish_fault_info_t fault;
    fish_hpm_load_stage_at(FISH_HPM_SD_READ,0x70040);
    fish_hpm_record_fault(5,0x80045100,0xdeadbeef);
    flags=ppor_reset_wdog0;fish_hpm_capture_reset();fish_hpm_reset_info(&fault);
    assert(fish_hpm_reset_stage()==FISH_HPM_SD_READ && fault.detail==0x70040);
    assert(fault.cause==6 && fault.pc==0x80045100 && fault.value==0xdeadbeef);
    fish_hpm_record_fault(0,0,0);flags=ppor_reset_wdog0;fish_hpm_capture_reset();fish_hpm_reset_info(&fault);
    assert(fish_hpm_reset_stage()==FISH_HPM_CPU_FAULT && fault.cause==1 && !fault.pc);
    puts("PASS: watchdog reset capture/clear, service only after init, atomic refresh keys across save/scene threads");
}
