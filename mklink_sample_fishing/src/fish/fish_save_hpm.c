/* Single filesystem owner: boot restore, then an RT-Thread save worker.
 * SD uses SPI1; LCD keeps its existing SPI0/DMA path. No USB MSC writer. */
#include "fish_save.h"
#include "fish_save_storage.h"
#include "fish_save_crypto.h"
#include "board.h"
#include "hpm_otp_drv.h"
#include "rtthread.h"
#include "ff.h"
#include "spi_sd_adapt.h"
#include <string.h>

static FATFS fs;
static struct rt_thread worker;
static struct rt_semaphore request;
static char worker_stack[4096] __attribute__((aligned(8)));
static uint8_t queued[FISH_SAVE_FILE_BYTES];
static uint8_t checked[FISH_SAVE_FILE_BYTES]; /* Single worker; keep journal off its stack. */
static unsigned queued_slot;
static int completion;
static bool busy,ready;
static uint32_t started_at;
extern int64_t get_system_time_ms(void);
#define SAVE_DIRECTORY "3:/FishingGame_Saves_DO_NOT_DELETE"
static const char *paths[]={SAVE_DIRECTORY "/NorthWetland_V7_Save_A_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V7_Save_B_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V6_Save_A_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V6_Save_B_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V5_Save_A_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V5_Save_B_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_Save_A_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_Save_B_DO_NOT_DELETE.sav"};

int fish_storage_read(unsigned slot,uint8_t *data,size_t len)
{
    if(!ready || slot>7) return -1;
    FIL f;UINT count=0;FRESULT result=f_open(&f,paths[slot],FA_READ);
    if(result==FR_NO_FILE || result==FR_NO_PATH) return -2;
    if(result!=FR_OK) return -1;
    bool size_ok=f_size(&f)==len;
    if(size_ok) result=f_read(&f,data,(UINT)len,&count);
    FRESULT closed=f_close(&f);
    if(result!=FR_OK || closed!=FR_OK) return -1;
    return size_ok && count==len?0:-3;
}
static void save_worker(void *arg)
{
    (void)arg;
    for(;;) {
        rt_sem_take(&request,RT_WAITING_FOREVER);
        int result=-1;FIL f;UINT count=0;
        FRESULT r=f_open(&f,paths[queued_slot],FA_WRITE|FA_OPEN_ALWAYS);
        if(r==FR_OK) {
            r=f_write(&f,queued,sizeof(queued),&count);
            if(r==FR_OK && count==sizeof(queued)) r=f_truncate(&f);
            FRESULT synced=f_sync(&f),closed=f_close(&f);
            if(r==FR_OK && count==sizeof(queued) && synced==FR_OK && closed==FR_OK &&
               !fish_storage_read(queued_slot,checked,sizeof(checked)) &&
               !memcmp(queued,checked,sizeof(queued))) result=1;
        }
        rt_base_t irq=rt_hw_interrupt_disable();completion=result;rt_hw_interrupt_enable(irq);
    }
}
int fish_storage_init(uint8_t key[32])
{
    uint8_t uuid[16];bool all_zero=true,all_ff=true;
    otp_init();
    for(unsigned i=0;i<4;i++) {
        uint32_t word=otp_read_from_shadow(OTP_SOC_UUID_IDX+i);
        for(unsigned b=0;b<4;b++) {
            uint8_t v=(uint8_t)(word>>(8*b));uuid[4*i+b]=v;all_zero&=v==0;all_ff&=v==255;
        }
    }
    if(all_zero || all_ff) return -1;
    fish_save_device_key(uuid,key);
    if(spi_sd_init()!=status_success || f_mount(&fs,"3:",1)!=FR_OK) return -1;
    FRESULT made=f_mkdir(SAVE_DIRECTORY);if(made!=FR_OK && made!=FR_EXIST) return -1;
    /* A visible, human-readable ownership note; never replace an existing note. */
    FIL note;
    if(f_open(&note,SAVE_DIRECTORY "/README_V7_DO_NOT_DELETE.txt",FA_WRITE|FA_CREATE_NEW)==FR_OK) {
        static const char info[]="钓鱼游戏存档，请勿删除 / Fishing game saves: DO NOT DELETE.\r\n"
            "V7_A、V7_B 是本版存档，V6、V5和原 A、B 保留用于旧版。所有文件请保留。\r\n"
            "切回旧版不会覆盖 V7 进度、声音设置和单鱼售价。不要格式化 SD 卡。\r\n"
            "建议定期把整个目录复制到电脑备份，恢复到同一块板卡。\r\n";
        UINT count;f_write(&note,info,sizeof(info)-1,&count);f_sync(&note);f_close(&note);
    }
    if(rt_sem_init(&request,"fsreq",0,RT_IPC_FLAG_FIFO)!=RT_EOK) return -1;
    if(rt_thread_init(&worker,"fishsave",save_worker,RT_NULL,worker_stack,sizeof(worker_stack),26,10)!=RT_EOK) return -1;
    ready=true;
    if(rt_thread_startup(&worker)!=RT_EOK) {ready=false;return -1;}
    return 0;
}
int fish_storage_begin(unsigned slot,const uint8_t *data,size_t len)
{
    if(!ready || slot>1 || len!=sizeof(queued)) return -1;
    rt_base_t irq=rt_hw_interrupt_disable();
    if(busy) {rt_hw_interrupt_enable(irq);return -1;}
    busy=true;completion=0;queued_slot=slot;memcpy(queued,data,len);
    started_at=(uint32_t)get_system_time_ms();rt_hw_interrupt_enable(irq);
    if(rt_sem_release(&request)!=RT_EOK) {busy=false;return -1;}
    return 0;
}
int fish_storage_poll(void)
{
    rt_base_t irq=rt_hw_interrupt_disable();int result=completion;
    if(!result || (uint32_t)get_system_time_ms()-started_at<400) result=0;
    if(result) {busy=false;completion=0;}
    rt_hw_interrupt_enable(irq);return result;
}
