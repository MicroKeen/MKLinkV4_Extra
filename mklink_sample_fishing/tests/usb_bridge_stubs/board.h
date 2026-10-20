#ifndef TEST_USB_BOARD_H
#define TEST_USB_BOARD_H
#include <stdbool.h>
#include <stdint.h>
typedef struct {unsigned unused;} USB_Type;
#define CONFIG_HPM_USBD_BASE 0x12340000U
#define CONFIG_HPM_USBD_IRQn 7
#define BOARD_RUNNING_CORE 0
#define HPM_L1C_CACHELINE_SIZE 32
#define ATTR_ALIGN(n) __attribute__((aligned(n)))
void board_init_usb(USB_Type *base);
void intc_set_irq_priority(int irq,int priority);
void intc_m_disable_irq(int irq);
void intc_m_enable_irq(int irq);
uint32_t core_local_mem_to_sys_address(unsigned core,uint32_t addr);
#endif
