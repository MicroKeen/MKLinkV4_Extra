/* Existing HPM/RT-Thread board and timing entry; fishing replaces only the app. */
#include "board.h"
#include "hpm_dma_mgr.h"
#include "perf_counter.h"
#include "rtt_port.h"
#include "rtthread.h"
#include "key_task.h"
#include "fish_hpm_runtime.h"

void rt_hw_board_init(void)
{
    fish_hpm_capture_reset();board_init();dma_mgr_init();board_init_gpio_pins();rtt_base_init();
}
extern int arm_2d_scene_app_player_init(void);
int main(void)
{
    /* RT-Thread already owns MCHTMR: observe it without clearing its counter. */
    init_cycle_counter(true);
    arm_2d_scene_app_player_init();
    for (;;) {
        key_module_display(1);
        rt_thread_mdelay(1);
    }
}
int64_t get_system_time_ms(void)
{
    return hpm_csr_get_core_cycle()/(int64_t)clock_get_core_clock_ticks_per_ms();
}
int64_t arm_2d_helper_get_system_timestamp(void) {return hpm_csr_get_core_cycle();}
uint32_t arm_2d_helper_get_reference_clock_frequency(void) {return clock_get_frequency(clock_cpu0);}
