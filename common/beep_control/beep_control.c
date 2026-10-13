#include "beep_control.h"
#include "board.h"
#include "hpm_gptmr_drv.h"
#include "rtthread.h"
#include "rthw.h"

#define BEEP_PWM_CHANNEL 2

static struct rt_timer beep_timer;
static bool beep_initialized;

/* Call with interrupts disabled to serialize tone updates and timer expiry. */
static void beep_stop(void)
{
    gptmr_stop_counter(HPM_GPTMR0, BEEP_PWM_CHANNEL);
    gptmr_channel_set_cmp_initial_polarity(HPM_GPTMR0, BEEP_PWM_CHANNEL, false);
    gptmr_channel_reset_count(HPM_GPTMR0, BEEP_PWM_CHANNEL);
}

static void beep_time_out(void *parameter)
{
    (void)parameter;
    rt_base_t level = rt_hw_interrupt_disable();
    beep_stop();
    rt_hw_interrupt_enable(level);
}

void Beep_PWM_Init(void)
{
    rt_base_t level = rt_hw_interrupt_disable();
    if (!beep_initialized) {
        HPM_IOC->PAD[IOC_PAD_PA10].FUNC_CTL = IOC_PA10_FUNC_CTL_GPTMR0_COMP_2;
        rt_timer_init(&beep_timer, "beep", beep_time_out, RT_NULL, 1,
                      RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_HARD_TIMER);
        beep_initialized = true;
        beep_stop();
    }
    rt_hw_interrupt_enable(level);
}

void Beep_PWM_frequency(uint32_t freq, uint8_t duty, uint16_t time)
{
    Beep_PWM_Init();
    rt_base_t level = rt_hw_interrupt_disable();
    rt_timer_stop(&beep_timer);
    beep_stop();

    uint32_t timer_freq = clock_get_frequency(clock_gptmr0);
    if (freq == 0 || duty == 0 || freq > timer_freq / 2) {
        rt_hw_interrupt_enable(level);
        return;
    }
    if (duty > 100) {
        duty = 100;
    }

    gptmr_channel_config_t config;
    gptmr_channel_get_default_config(HPM_GPTMR0, &config);
    config.reload = timer_freq / freq;
    config.cmp_initial_polarity_high = true;
    gptmr_channel_config(HPM_GPTMR0, BEEP_PWM_CHANNEL, &config, false);
    uint32_t cmp = (uint32_t)(((uint64_t)config.reload * duty) / 100) + 1;
    gptmr_update_cmp(HPM_GPTMR0, BEEP_PWM_CHANNEL, 0, cmp);
    gptmr_update_cmp(HPM_GPTMR0, BEEP_PWM_CHANNEL, 1, config.reload);
    gptmr_channel_reset_count(HPM_GPTMR0, BEEP_PWM_CHANNEL);
    gptmr_start_counter(HPM_GPTMR0, BEEP_PWM_CHANNEL);

    if (time != 0) {
        rt_tick_t ticks = rt_tick_from_millisecond((rt_int32_t)time * 10);
        rt_timer_control(&beep_timer, RT_TIMER_CTRL_SET_TIME, &ticks);
        rt_timer_start(&beep_timer);
    }
    rt_hw_interrupt_enable(level);
}
