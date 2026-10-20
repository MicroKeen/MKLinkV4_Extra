/* HPM SDK CherryUSB MSC descriptor pattern, adapted for MKLink V4 SPI SD.
 * USB is initialized only by missing/incompatible-resource recovery. */
#include "fish_usb_hpm.h"
#include "fish_media.h"
#include "fish_fs_hpm.h"
#include "fish_sd_guard.h"
#include "fish_hpm_runtime.h"
#include "usbd_core.h"
#include "usbd_msc.h"
#include "board.h"
#include "diskio.h"
#include "rtthread.h"
enum {MSC_IN=0x81,MSC_OUT=0x02,CONFIG_SIZE=9+MSC_DESCRIPTOR_LEN};
static bool active;
static uint32_t sectors;
static struct usbd_interface msc_interface;
/* Use the scene/input clock for both quiet windows and frame updates. */
extern uint32_t fish_hpm_millis(void);
extern bool fish_hpm_watchdog_reset(void);
extern unsigned fish_hpm_reset_stage(void);
bool fish_media_port_watchdog_reset(void) {return fish_hpm_watchdog_reset();}
unsigned fish_media_port_reset_stage(void) {return fish_hpm_reset_stage();}
void fish_media_port_reset_info(fish_fault_info_t *out) {fish_hpm_reset_info(out);}
/* Local, licensed MSC adaptation adds a command-drained query; SDK untouched. */
extern bool fish_usb_msc_idle(uint8_t busid);
static const uint8_t device[]={USB_DEVICE_DESCRIPTOR_INIT(USB_2_0,0,0,0,0x34b7,0xf129,0x0101,1)};
static const uint8_t hs[]={USB_CONFIG_DESCRIPTOR_INIT(CONFIG_SIZE,1,1,USB_CONFIG_BUS_POWERED,200),
    MSC_DESCRIPTOR_INIT(0,MSC_OUT,MSC_IN,USB_BULK_EP_MPS_HS,2)};
static const uint8_t fs[]={USB_CONFIG_DESCRIPTOR_INIT(CONFIG_SIZE,1,1,USB_CONFIG_BUS_POWERED,200),
    MSC_DESCRIPTOR_INIT(0,MSC_OUT,MSC_IN,USB_BULK_EP_MPS_FS,2)};
static const uint8_t qualifier[]={USB_DEVICE_QUALIFIER_DESCRIPTOR_INIT(USB_2_0,0,0,0,1)};
static const uint8_t other_hs[]={USB_OTHER_SPEED_CONFIG_DESCRIPTOR_INIT(CONFIG_SIZE,1,1,USB_CONFIG_BUS_POWERED,200),
    MSC_DESCRIPTOR_INIT(0,MSC_OUT,MSC_IN,USB_BULK_EP_MPS_FS,2)};
static const uint8_t other_fs[]={USB_OTHER_SPEED_CONFIG_DESCRIPTOR_INIT(CONFIG_SIZE,1,1,USB_CONFIG_BUS_POWERED,200),
    MSC_DESCRIPTOR_INIT(0,MSC_OUT,MSC_IN,USB_BULK_EP_MPS_HS,2)};
static const uint8_t *device_cb(uint8_t speed) {(void)speed;return device;}
static const uint8_t *config_cb(uint8_t speed) {return speed==USB_SPEED_HIGH?hs:fs;}
static const uint8_t *qualifier_cb(uint8_t speed) {(void)speed;return qualifier;}
static const uint8_t *other_cb(uint8_t speed) {return speed==USB_SPEED_HIGH?other_hs:other_fs;}
static const char *string_cb(uint8_t speed,uint8_t index)
{
    (void)speed;
    static const char *const strings[]={"\x09\x04","MicroKeen","Fishing SD resources","MKLINK4-FISH-SD2"};
    return index<sizeof(strings)/sizeof(*strings)?strings[index]:NULL;
}
static const struct usb_descriptor descriptors={.device_descriptor_callback=device_cb,
    .config_descriptor_callback=config_cb,.device_quality_descriptor_callback=qualifier_cb,
    .other_speed_descriptor_callback=other_cb,.string_descriptor_callback=string_cb};
static void event(uint8_t bus,uint8_t kind) {(void)bus;(void)kind;}
void usbd_msc_get_cap(uint8_t bus,uint8_t lun,uint32_t *count,uint32_t *size)
{(void)bus;(void)lun;*count=sectors;*size=512;}
static bool range_ok(uint8_t bus,uint8_t lun,uint32_t sector,uint32_t bytes)
{return active && !bus && !lun && bytes && !(bytes%512) && sector<sectors && bytes/512<=sectors-sector;}
int fish_sd_guard_read(uint32_t sector,void *out)
{return disk_read(DEV_SD,out,sector,1)==RES_OK?0:-1;}
int usbd_msc_sector_read(uint8_t bus,uint8_t lun,uint32_t sector,uint8_t *buffer,uint32_t bytes)
{
    if(!buffer || !range_ok(bus,lun,sector,bytes))return -1;
    return disk_read(DEV_SD,buffer,sector,bytes/512)==RES_OK?0:-1;
}
int usbd_msc_sector_write(uint8_t bus,uint8_t lun,uint32_t sector,uint8_t *buffer,uint32_t bytes)
{
    if(!buffer || !range_ok(bus,lun,sector,bytes))return -1;
    fish_media_host_write(fish_hpm_millis(),bytes);
    return disk_write(DEV_SD,buffer,sector,bytes/512)==RES_OK?0:-1;
}
int fish_media_port_start(void)
{
    if(active)return 0;
    if(fish_fs_usb_begin())return -1;
    LBA_t count=0;WORD bytes=0;
    if(disk_ioctl(DEV_SD,GET_SECTOR_COUNT,&count)!=RES_OK ||
       disk_ioctl(DEV_SD,GET_SECTOR_SIZE,&bytes)!=RES_OK || bytes!=512 || !count || count>UINT32_MAX) {
        fish_fs_usb_end();return -1;
    }
    /* Match the existing downloader's LBA mapping. Refuse inconsistent MBR/BPB
     * bounds instead of exposing a disk Windows might offer to format. */
    if(fish_sd_guard_capacity((uint32_t)count,&sectors)) {fish_fs_usb_end();return -1;}
    board_init_usb((USB_Type *)CONFIG_HPM_USBD_BASE);
    intc_set_irq_priority(CONFIG_HPM_USBD_IRQn,2);
    usbd_desc_register(0,&descriptors);
    usbd_add_interface(0,usbd_msc_init_intf(0,&msc_interface,MSC_OUT,MSC_IN));
    active=true;
    if(usbd_initialize(0,CONFIG_HPM_USBD_BASE,event)) {
        intc_m_disable_irq(CONFIG_HPM_USBD_IRQn);usbd_deinitialize(0);
        active=false;fish_fs_usb_end();return -1;
    }
    return 0;
}
int fish_media_port_probe(bool enabled) {return fish_fs_usb_probe(enabled);}
int fish_media_port_stop(void)
{
    if(!active)return 0;
    /* Freeze only the USB IRQ while checking/closing. LCD and tick IRQs continue. */
    intc_m_disable_irq(CONFIG_HPM_USBD_IRQn);
    if(!fish_usb_msc_idle(0)) {intc_m_enable_irq(CONFIG_HPM_USBD_IRQn);return 1;}
    if(disk_ioctl(DEV_SD,CTRL_SYNC,NULL)!=RES_OK) {intc_m_enable_irq(CONFIG_HPM_USBD_IRQn);return -1;}
    usbd_deinitialize(0);active=false;fish_fs_usb_end();return 0;
}
void fish_usb_poll(void)
{
    if(active)usbd_msc_polling(0);
    /* Advance read-only checks between PFB tasks, rather than once per frame.
     * Only frame_start may grant first USB ownership after draining saves. */
    fish_media_phase_t phase=fish_media_phase();
    if(phase==FISH_MEDIA_CHECK || phase==FISH_MEDIA_SETTLE || fish_media_visible())
        fish_media_tick(fish_hpm_millis(),true);
}
