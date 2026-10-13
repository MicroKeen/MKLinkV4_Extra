/*
 * Copyright (c) 2022-2023 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "usbd_core.h"
#include "usbd_cdc_acm.h"
#include "rtthread.h"
#include "rthw.h"
#include "chry_ringbuffer.h"
#include <stdio.h>

/*!< endpoint address */
#define CDC_IN_EP  0x81
#define CDC_OUT_EP 0x01
#define CDC_INT_EP 0x83
static uint8_t chConsoleBuffer[1024];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX static uint8_t chCdcSendBuffer[512];
static chry_ringbuffer_t tConsoleQueue;
static bool bIsConsoleInit;
static bool cdc_configured;
static uint8_t console_busid;
/*!< config descriptor size */
#define USB_CONFIG_SIZE (9 + CDC_ACM_DESCRIPTOR_LEN)

static const uint8_t device_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xEF, 0x02, 0x01, USBD_VID, USBD_PID, 0x0100, 0x01)
};

static const uint8_t config_descriptor_hs[] = {
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x02, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ACM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, USB_BULK_EP_MPS_HS, 0x02),
};

static const uint8_t config_descriptor_fs[] = {
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x02, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ACM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, USB_BULK_EP_MPS_FS, 0x02),
};

static const uint8_t device_quality_descriptor[] = {
    USB_DEVICE_QUALIFIER_DESCRIPTOR_INIT(USB_2_0, 0xEF, 0x02, 0x01, 0x01),
};

static const uint8_t other_speed_config_descriptor_hs[] = {
    USB_OTHER_SPEED_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x02, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ACM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, USB_BULK_EP_MPS_FS, 0x02),
};

static const uint8_t other_speed_config_descriptor_fs[] = {
    USB_OTHER_SPEED_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x02, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ACM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, USB_BULK_EP_MPS_HS, 0x02),
};

static const char *string_descriptors[] = {
    (const char[]){ 0x09, 0x04 }, /* Langid */
    "HPMicro",                    /* Manufacturer */
    "HPMicro CDC DEMO",           /* Product */
    "2024051702",                 /* Serial Number */
};

static const uint8_t *device_descriptor_callback(uint8_t speed)
{
    (void)speed;

    return device_descriptor;
}

static const uint8_t *config_descriptor_callback(uint8_t speed)
{
    if (speed == USB_SPEED_HIGH) {
        return config_descriptor_hs;
    } else if (speed == USB_SPEED_FULL) {
        return config_descriptor_fs;
    } else {
        return NULL;
    }
}

static const uint8_t *device_quality_descriptor_callback(uint8_t speed)
{
    (void)speed;

    return device_quality_descriptor;
}

static const uint8_t *other_speed_config_descriptor_callback(uint8_t speed)
{
    if (speed == USB_SPEED_HIGH) {
        return other_speed_config_descriptor_hs;
    } else if (speed == USB_SPEED_FULL) {
        return other_speed_config_descriptor_fs;
    } else {
        return NULL;
    }
}

static const char *string_descriptor_callback(uint8_t speed, uint8_t index)
{
    (void)speed;

    if (index >= (sizeof(string_descriptors) / sizeof(char *))) {
        return NULL;
    }
    return string_descriptors[index];
}

const struct usb_descriptor cdc_descriptor = {
    .device_descriptor_callback = device_descriptor_callback,
    .config_descriptor_callback = config_descriptor_callback,
    .device_quality_descriptor_callback = device_quality_descriptor_callback,
    .other_speed_descriptor_callback = other_speed_config_descriptor_callback,
    .string_descriptor_callback = string_descriptor_callback,
};

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t read_buffer[2][512];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[2048];
volatile uint8_t read_buffer_index;
volatile bool dtr_enable = false;
volatile bool ep_tx_busy_flag = false;

/* Call with interrupts disabled: printf writers and USB callbacks share the queue. */
static void cdc_console_queue_init(void)
{
    if (!bIsConsoleInit) {
        chry_ringbuffer_init(&tConsoleQueue, chConsoleBuffer, sizeof(chConsoleBuffer));
        bIsConsoleInit = true;
    }
}

static void cdc_console_start_write(void)
{
    if (!cdc_configured || !dtr_enable || ep_tx_busy_flag) {
        return;
    }

    uint32_t len = chry_ringbuffer_peek(&tConsoleQueue, chCdcSendBuffer,
                                       sizeof(chCdcSendBuffer));
    if (len == 0) {
        return;
    }

    /* Mark busy before entering the driver, which can itself emit diagnostics. */
    ep_tx_busy_flag = true;
    if (usbd_ep_start_write(console_busid, CDC_IN_EP, chCdcSendBuffer, len) == 0) {
        chry_ringbuffer_drop(&tConsoleQueue, len);
    } else {
        ep_tx_busy_flag = false;
    }
}

static void usbd_event_handler(uint8_t busid, uint8_t event)
{
    rt_base_t level = rt_hw_interrupt_disable();
    switch (event) {
    case USBD_EVENT_RESET:
        cdc_configured = false;
        dtr_enable = false;
        ep_tx_busy_flag = false;
        break;
    case USBD_EVENT_CONNECTED:
        break;
    case USBD_EVENT_DISCONNECTED:
        cdc_configured = false;
        dtr_enable = false;
        ep_tx_busy_flag = false;
        break;
    case USBD_EVENT_RESUME:
        break;
    case USBD_EVENT_SUSPEND:
        break;
    case USBD_EVENT_CONFIGURED:
        /* setup first out ep read transfer */
        read_buffer_index = 0;
        ep_tx_busy_flag = false;
        cdc_configured = true;
        usbd_ep_start_read(busid, CDC_OUT_EP, &read_buffer[0][0], usbd_get_ep_mps(busid, CDC_OUT_EP));
        cdc_console_start_write();
        break;
    case USBD_EVENT_SET_REMOTE_WAKEUP:
        break;
    case USBD_EVENT_CLR_REMOTE_WAKEUP:
        break;

    default:
        break;
    }
    rt_hw_interrupt_enable(level);
}

void usbd_cdc_acm_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    uint8_t index = read_buffer_index;
    //USB_LOG_RAW("actual out len:%d\r\n", nbytes);

    read_buffer_index = (index == 0) ? 1 : 0;

    usbd_ep_start_read(busid, ep, &read_buffer[read_buffer_index][0], usbd_get_ep_mps(busid, ep));
}

void usbd_cdc_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    rt_base_t level = rt_hw_interrupt_disable();
    uint16_t mps = usbd_get_ep_mps(busid, ep);
    if (cdc_configured && mps != 0 && nbytes != 0 && (nbytes % mps) == 0) {
        /* send zlp */
        if (usbd_ep_start_write(busid, ep, NULL, 0) != 0) {
            ep_tx_busy_flag = false;
        }
    } else {
        ep_tx_busy_flag = false;
        cdc_console_start_write();
    }
    rt_hw_interrupt_enable(level);
}

/*!< endpoint call back */
struct usbd_endpoint cdc_out_ep = {
    .ep_addr = CDC_OUT_EP,
    .ep_cb = usbd_cdc_acm_bulk_out
};

struct usbd_endpoint cdc_in_ep = {
    .ep_addr = CDC_IN_EP,
    .ep_cb = usbd_cdc_acm_bulk_in
};

static struct usbd_interface intf0;
static struct usbd_interface intf1;

/* function ------------------------------------------------------------------*/

void cdc_acm_init(uint8_t busid, uint32_t reg_base)
{
    rt_base_t level = rt_hw_interrupt_disable();
    console_busid = busid;
    cdc_console_queue_init();
    rt_hw_interrupt_enable(level);
    /* CONFIG_NDEBUG_CONSOLE skips the SDK's normal stdio buffer setup. */
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    usbd_desc_register(busid, &cdc_descriptor);
    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &intf0));
    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &intf1));
    usbd_add_endpoint(busid, &cdc_out_ep);
    usbd_add_endpoint(busid, &cdc_in_ep);
    usbd_initialize(busid, reg_base, usbd_event_handler);
}

void usbd_cdc_acm_set_dtr(uint8_t busid, uint8_t intf, bool dtr)
{
    (void)intf;
    rt_base_t level = rt_hw_interrupt_disable();
    if (busid == console_busid) {
        dtr_enable = dtr;
        cdc_console_start_write();
    }
    rt_hw_interrupt_enable(level);
}

static int cdc_console_write(const char *data, unsigned int size)
{
    rt_base_t level = rt_hw_interrupt_disable();
    cdc_console_queue_init();
    cdc_console_start_write();
    /* Best-effort logging: discard excess data rather than block when the host
     * is absent or slow. Report success so stdio remains usable on reconnect. */
    chry_ringbuffer_write(&tConsoleQueue, (void *)data, size);
    cdc_console_start_write();
    rt_hw_interrupt_enable(level);
    return size;
}

/* GCC/newlib (and the SDK's IAR __write adapter) use this hook. The SDK's
 * CONFIG_NDEBUG_CONSOLE implementation is weak, so no SDK changes are needed. */
int _write(int file, char *data, int size)
{
    (void)file;
    return size > 0 ? cdc_console_write(data, (unsigned int)size) : 0;
}

#ifdef __SEGGER_RTL_VERSION
/* CMake renames the SDK's strong definition for SES. Restore the original
 * symbol only here so the prebuilt SEGGER runtime calls the CDC writer. */
#undef __SEGGER_RTL_X_file_write
#include "__SEGGER_RTL_Int.h"
int __SEGGER_RTL_X_file_write(__SEGGER_RTL_FILE *file, const char *data, unsigned int size)
{
    (void)file;
    return cdc_console_write(data, size);
}
#endif

