#include "board.h"
#include "arm_2d_helper.h"
#include "ref_gui.h"
#include "st7789.h"
#include "rtthread.h"
#include "hpm_ewdg_drv.h"
#include "key_task.h"
#include "fish_scene.h"
#include "fish_hpm_input.h"
#include "fish_usb_hpm.h"
#include "fish_hpm_runtime.h"

ref_gui_t g_tMyGUI;
static ngy_msg_t messages[32];
static ngy_msg_pointer_frontend_pipeline_t pointer_pipelines[2];
static char arm_2d_stack[8192];
static struct rt_thread arm_2d_thread;
static const ref_gui_cfg_t gui_config={
    .fnDispAdapterTask=&__disp_adapter0_task,
    .ptDispAdapter=&DISP0_ADAPTER,
    .Message={.ptItems=messages,.hwCount=dimof(messages)},
    .tKeyFrontend={.u4First3RepeatInterval=10,.u4RestRepeatInterval=3,
        .bDisableDoublePressedDetection=true,.bDisableLongPressDetection=true},
    .tPointerFrontend={.u10DoubleClickLimit=200},
    .ptPipelines=pointer_pipelines,.chPipelineNo=dimof(pointer_pipelines),
    .u4MessageBatchProcessingNumber=15,
    .fnUserMessagePreprocessingHandler=fish_hpm_handle_message,
};

extern void lcd_spi_init(void);
static void arm_2d_thread_entry(void *parameter)
{
    (void)parameter;
    clock_add_to_group(clock_watchdog0,0);
    ewdg_config_t watchdog;
    ewdg_get_default_config(HPM_EWDG0,&watchdog);
    watchdog.enable_watchdog=true;
    watchdog.int_rst_config.enable_timeout_reset=true;
    watchdog.ctrl_config.use_lowlevel_timeout=false;
    watchdog.ctrl_config.cnt_clk_sel=ewdg_cnt_clk_src_ext_osc_clk;
    watchdog.cnt_src_freq=32768;
    watchdog.ctrl_config.timeout_reset_us=10*1000*1000;
    ewdg_init(HPM_EWDG0,&watchdog);
    fish_hpm_watchdog_ready();
    lcd_spi_init();ST7789_Init();
    fish_hpm_storage_heartbeat();
    arm_irq_safe {arm_2d_init();}
    disp_adapter0_init();
    rg_init(&g_tMyGUI,(ref_gui_cfg_t *)&gui_config);
    fish_scene_init(rg_get_disp_adapter(&g_tMyGUI));
    /* Initialize the branch's existing key driver after the message pool is live. */
    key_init();key_encoder_init();
    for (;;) {
        /* USB polling and resource probes share this scene thread. */
        fish_hpm_storage_heartbeat();
        fish_hpm_load_stage(FISH_HPM_MEDIA);
        fish_usb_poll();
        fish_hpm_storage_heartbeat();
        fish_hpm_load_stage(FISH_HPM_GUI);
        arm_fsm_rt_t result=rg_task(&g_tMyGUI,30);
        fish_hpm_storage_heartbeat();
        if (result==arm_fsm_rt_cpl) rt_thread_mdelay(1);
    }
}

int arm_2d_scene_app_player_init(void)
{
    rt_thread_init(&arm_2d_thread,"arm2d",arm_2d_thread_entry,RT_NULL,
        arm_2d_stack,sizeof(arm_2d_stack),25,10);
    rt_thread_startup(&arm_2d_thread);
    rt_thread_mdelay(1000);
    return RT_EOK;
}
