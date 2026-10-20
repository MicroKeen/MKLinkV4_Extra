/* Minimal HPM SDK CherryUSB device configuration; MSC is main-thread polled. */
#ifndef CHERRYUSB_CONFIG_H
#define CHERRYUSB_CONFIG_H
#include "board.h"
#define CONFIG_USB_PRINTF(...) ((void)0)
#define CONFIG_USB_DBG_LEVEL 0
#define CONFIG_USB_HS
#define CONFIG_USB_ALIGN_SIZE 4
#define CONFIG_USB_MEMCPY_DISABLE
#define USB_NOCACHE_RAM_SECTION __attribute__((section(".noncacheable.non_init")))
#define CONFIG_USBDEV_ADVANCE_DESC
#define CONFIG_USBDEV_REQUEST_BUFFER_LEN 256
#define CONFIG_USBDEV_MAX_BUS 1
/* The SDK's shared OTG header declares host types even in device-only builds. */
#define CONFIG_USBHOST_MAX_ENDPOINTS 8
#define CONFIG_USBHOST_DEV_NAMELEN 16
#define CONFIG_USBHOST_MAX_INTF_ALTSETTINGS 2
#define CONFIG_USBHOST_MAX_INTERFACES 8
#define CONFIG_USBHOST_MAX_EHPORTS 4
#define CONFIG_USBHOST_MAX_RHPORTS 1
#define CONFIG_USBHOST_MAX_EXTHUBS 1
#define CONFIG_USBHOST_MAX_BUS 1
#define CONFIG_USBDEV_MSC_MAX_LUN 1
#define CONFIG_USBDEV_MSC_MAX_BUFSIZE 1024
#define CONFIG_USBDEV_MSC_POLLING
#define CONFIG_USBDEV_MSC_MANUFACTURER_STRING "MKLink"
#define CONFIG_USBDEV_MSC_PRODUCT_STRING "Fish resources"
#define CONFIG_USBDEV_MSC_VERSION_STRING "1.01"
#define CONFIG_HPM_USBD_BASE HPM_USB0_BASE
#define CONFIG_HPM_USBD_IRQn IRQn_USB0
#define usb_phyaddr2ramaddr(addr) core_local_mem_to_sys_address(BOARD_RUNNING_CORE,addr)
#define usb_ramaddr2phyaddr(addr) sys_address_to_core_local_mem(BOARD_RUNNING_CORE,addr)
#endif
