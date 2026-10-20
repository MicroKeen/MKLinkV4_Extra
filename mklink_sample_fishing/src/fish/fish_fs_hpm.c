#include "fish_fs_hpm.h"
#include "rtthread.h"
#include "ff.h"
#include "diskio.h"
#include "spi_sd_adapt.h"
static struct rt_mutex mutex;
static FATFS fs;
static int initialized,mounted,card_ready,usb_owned,probe;
static int take(void)
{
    if(!initialized) {if(rt_mutex_init(&mutex,"fishfs",RT_IPC_FLAG_PRIO)!=RT_EOK)return -1;initialized=1;}
    return rt_mutex_take(&mutex,RT_WAITING_FOREVER)==RT_EOK?0:-1;
}
static int mount(void)
{
    if(!card_ready) {
        if(spi_sd_init()!=status_success || disk_initialize(DEV_SD))return -1;
        card_ready=1;
    }
    if(!mounted) {
        if(f_mount(&fs,"3:",1)!=FR_OK)return -1;
        mounted=1;
    }
    return 0;
}
int fish_fs_lock(void)
{
    if(take())return -1;
    if(usb_owned || mount()) {fish_fs_unlock();return -1;}
    return 0;
}
int fish_fs_resource_lock(void)
{
    if(take())return -1;
    if((usb_owned && !probe) || mount()) {fish_fs_unlock();return -1;}
    return 0;
}
void fish_fs_unlock(void) {rt_mutex_release(&mutex);}
int fish_fs_usb_begin(void)
{
    if(take())return -1;
    /* Existing volume must mount before it can be handed to the host. Never
     * offer blank/unrecognized media or attempt to create/repair a filesystem. */
    if(mount()) {fish_fs_unlock();return -1;}
    f_mount(NULL,"3:",0);mounted=0;probe=0;usb_owned=1;
    fish_fs_unlock();return 0;
}
void fish_fs_usb_end(void)
{
    if(take())return;
    f_mount(NULL,"3:",0);mounted=probe=usb_owned=0;
    fish_fs_unlock();
}
int fish_fs_usb_probe(int enabled)
{
    if(take())return -1;
    int result=0;
    if(enabled) {
        if(!usb_owned || mount())result=-1;
        else probe=1;
    } else {f_mount(NULL,"3:",0);mounted=probe=0;}
    fish_fs_unlock();return result;
}
