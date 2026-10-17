#include "fish_fs_hpm.h"
#include "rtthread.h"
#include "ff.h"
#include "spi_sd_adapt.h"
static struct rt_mutex mutex;
static FATFS fs;
static int initialized,mounted;
int fish_fs_lock(void)
{
    /* Called first by scene boot, before the save worker starts. */
    if(!initialized) {if(rt_mutex_init(&mutex,"fishfs",RT_IPC_FLAG_PRIO)!=RT_EOK)return -1;initialized=1;}
    if(rt_mutex_take(&mutex,RT_WAITING_FOREVER)!=RT_EOK)return -1;
    if(!mounted) {
        if(spi_sd_init()!=status_success || f_mount(&fs,"3:",1)!=FR_OK) {rt_mutex_release(&mutex);return -1;}
        mounted=1;
    }
    return 0;
}
void fish_fs_unlock(void) {rt_mutex_release(&mutex);}
