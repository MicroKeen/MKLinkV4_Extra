#include "fish_hpm_runtime.h"
#include "board.h"
#include "hpm_ewdg_drv.h"
#include "hpm_ppor_drv.h"
#include "rtthread.h"
static bool watchdog_ready,watchdog_reset;
/* The existing NOLOAD AHB section is excluded from startup zeroing. A guarded
 * breadcrumb survives some warm resets; bootloader/cold-reset clearing is
 * allowed and simply yields no stage. Never write diagnostics to user SD. */
static volatile struct {uint32_t magic,stage;fish_fault_info_t fault;uint32_t inverse;} load_trace
    __attribute__((section(".ahb_sram.fish_load_trace")));
static unsigned reset_stage;
static fish_fault_info_t reset_fault;
void fish_hpm_load_stage(unsigned stage)
{
    fish_hpm_load_stage_at(stage,0);
}
void fish_hpm_load_stage_at(unsigned stage,uint32_t detail)
{
    load_trace.magic=0;load_trace.stage=stage;
    load_trace.fault=(fish_fault_info_t){.detail=detail};
    load_trace.inverse=~(stage^detail);load_trace.magic=0x464c5333u;
}
/* The default SDK exception handler returns the same EPC. Capture in ILM,
 * then stop; no filesystem/LCD/allocator access and no watchdog refresh. */
ATTR_RAMFUNC __attribute__((noinline)) void fish_hpm_record_fault(uint32_t cause,uint32_t pc,uint32_t value)
{
    load_trace.magic=0;
    if(!load_trace.stage || load_trace.stage>FISH_HPM_CPU_FAULT)load_trace.stage=FISH_HPM_CPU_FAULT;
    load_trace.fault.cause=cause+1;load_trace.fault.pc=pc;load_trace.fault.value=value;
    load_trace.inverse=~(load_trace.stage^load_trace.fault.detail^(cause+1)^pc^value);
    load_trace.magic=0x464c5333u;
}
#if defined(__riscv)
ATTR_RAMFUNC __attribute__((noreturn)) long exception_handler(long cause,long pc)
{
    fish_hpm_record_fault((uint32_t)cause,(uint32_t)pc,read_csr(CSR_MTVAL));
    for(;;)__asm volatile("wfi");
}
#endif
unsigned fish_hpm_reset_stage(void) {return reset_stage;}
void fish_hpm_reset_info(fish_fault_info_t *out) {*out=reset_fault;}
void fish_hpm_capture_reset(void)
{
    uint32_t flags=ppor_reset_get_flags(HPM_PPOR);
    watchdog_reset=(flags&(ppor_reset_wdog0|ppor_reset_wdog1|ppor_reset_pmic_wdog))!=0;
    bool retained=watchdog_reset && load_trace.magic==0x464c5333u &&
        (load_trace.stage^load_trace.fault.detail^load_trace.fault.cause^load_trace.fault.pc^
         load_trace.fault.value^load_trace.inverse)==UINT32_MAX && load_trace.stage<=FISH_HPM_CPU_FAULT;
    reset_stage=retained?load_trace.stage:0;
    reset_fault=retained?load_trace.fault:(fish_fault_info_t){0};
    fish_hpm_load_stage(FISH_HPM_LOAD_IDLE);
    /* Capture before board_init can clear flags. The bootloader may have
     * consumed them already; absence is never reported as proof of no reset. */
    ppor_reset_clear_flags(HPM_PPOR,flags);
}
bool fish_hpm_watchdog_reset(void) {return watchdog_reset;}
void fish_hpm_watchdog_ready(void) {watchdog_ready=true;fish_hpm_storage_heartbeat();}
void fish_hpm_storage_heartbeat(void)
{
    if(watchdog_ready) {
        /* Save and scene threads can both finish SD transactions. Protect the
         * hardware refresh key sequence against scheduler preemption. */
        rt_base_t irq=rt_hw_interrupt_disable();ewdg_refresh(HPM_EWDG0);rt_hw_interrupt_enable(irq);
    }
}
