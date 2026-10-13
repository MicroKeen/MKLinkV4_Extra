#include "board.h"
#include "hpm_gpio_drv.h"
#include "hpm_mchtmr_drv.h"
#include "perf_counter.h"
#include "ref_gui.h"

#define DEBOUNCE_THRESHOLD_IN_MS 0

#define ENCODER_GPIO_CTPL_A      HPM_GPIO0
#define ENCODER_GPIO_INDEX_A     GPIO_DI_GPIOA
#define ENCODER_GPIO_PIN_A       4
#define ENCODER_GPIO_CTPL_B      HPM_GPIO0
#define ENCODER_GPIO_INDEX_B     GPIO_DI_GPIOA
#define ENCODER_GPIO_PIN_B       30
#define ENCODER_GPIO_IRQ         IRQn_GPIO0_A

#define MCHTMR_CLK_NAME (clock_mchtmr0)

#include "ref_gui.h"
extern ref_gui_t g_tMyGUI;
static arm_2d_region_t tWheelRegion;

SDK_DECLARE_EXT_ISR_M(ENCODER_GPIO_IRQ, isr_gpio)
void isr_gpio(void)
{
    static int16_t iY = 0;
    if(gpio_check_pin_interrupt_flag(ENCODER_GPIO_CTPL_A,
                                  ENCODER_GPIO_INDEX_A,
                                  ENCODER_GPIO_PIN_A)){
          delay_us(100);
          uint8_t B = gpio_read_pin(ENCODER_GPIO_CTPL_B, ENCODER_GPIO_INDEX_B, ENCODER_GPIO_PIN_B);
          uint8_t A = gpio_read_pin(ENCODER_GPIO_CTPL_A, ENCODER_GPIO_INDEX_A, ENCODER_GPIO_PIN_A);

          if(B == 0 && A == 1){
              tWheelRegion.tLocation.iY++;   
          } 
          //printf(" B =%d,A =%d,iY=%d\n",B,A,tWheelRegion.tLocation.iY);
          gpio_clear_pin_interrupt_flag(ENCODER_GPIO_CTPL_A,
                                        ENCODER_GPIO_INDEX_A,
                                        ENCODER_GPIO_PIN_A);
    }

    if(gpio_check_pin_interrupt_flag(ENCODER_GPIO_CTPL_B,
                                  ENCODER_GPIO_INDEX_B,
                                  ENCODER_GPIO_PIN_B)){
          delay_us(100);
          uint8_t A = gpio_read_pin(ENCODER_GPIO_CTPL_A, ENCODER_GPIO_INDEX_A, ENCODER_GPIO_PIN_A);
          uint8_t B = gpio_read_pin(ENCODER_GPIO_CTPL_B, ENCODER_GPIO_INDEX_B, ENCODER_GPIO_PIN_B);

          if(A == 0 && B == 1){
              tWheelRegion.tLocation.iY--;
          }
          //printf(" A =%d,B =%d,iY=%d\n",A,B,tWheelRegion.tLocation.iY);
          gpio_clear_pin_interrupt_flag(ENCODER_GPIO_CTPL_B,
                                        ENCODER_GPIO_INDEX_B,
                                        ENCODER_GPIO_PIN_B);
    }

    if(iY != tWheelRegion.tLocation.iY){
          rg_send_gesture_evt(&g_tMyGUI, NGY_MSG_GESTURE_EVT_WHEEL,
                              0, &tWheelRegion, 20, NULL);
          iY = tWheelRegion.tLocation.iY;
    }
}


void key_encoder_init(void)
{

    gpio_interrupt_trigger_t trigger;

    gpio_set_pin_input(ENCODER_GPIO_CTPL_A, ENCODER_GPIO_INDEX_A,
                           ENCODER_GPIO_PIN_A);


    gpio_config_pin_interrupt(ENCODER_GPIO_CTPL_A, ENCODER_GPIO_INDEX_A,
                           ENCODER_GPIO_PIN_A, gpio_interrupt_trigger_edge_both);

    gpio_enable_pin_interrupt(ENCODER_GPIO_CTPL_A, ENCODER_GPIO_INDEX_A,
                           ENCODER_GPIO_PIN_A);


    gpio_set_pin_input(ENCODER_GPIO_CTPL_B, ENCODER_GPIO_INDEX_B,
                           ENCODER_GPIO_PIN_B);

    gpio_config_pin_interrupt(ENCODER_GPIO_CTPL_B, ENCODER_GPIO_INDEX_B,
                           ENCODER_GPIO_PIN_B, gpio_interrupt_trigger_edge_both);

    gpio_enable_pin_interrupt(ENCODER_GPIO_CTPL_B, ENCODER_GPIO_INDEX_B,
                           ENCODER_GPIO_PIN_B);

    intc_m_enable_irq_with_priority(ENCODER_GPIO_IRQ, 1);

    HPM_IOC->PAD[IOC_PAD_PA04].PAD_CTL = IOC_PAD_PAD_CTL_HYS_SET(1) | IOC_PAD_PAD_CTL_PRS_SET(2) | IOC_PAD_PAD_CTL_PE_SET(1) | IOC_PAD_PAD_CTL_PS_SET(1)  | IOC_PAD_PAD_CTL_SR_SET(1) | IOC_PAD_PAD_CTL_SPD_SET(3)| IOC_PAD_PAD_CTL_DS_SET(0);
    HPM_IOC->PAD[IOC_PAD_PA30].PAD_CTL = IOC_PAD_PAD_CTL_HYS_SET(1) | IOC_PAD_PAD_CTL_PRS_SET(2) | IOC_PAD_PAD_CTL_PE_SET(1) | IOC_PAD_PAD_CTL_PS_SET(1)  | IOC_PAD_PAD_CTL_SR_SET(1) | IOC_PAD_PAD_CTL_SPD_SET(3)| IOC_PAD_PAD_CTL_DS_SET(0);
}