/* Boot restore and an RT-Thread save worker share the filesystem mutex with
 * the incremental map loader.
 * SD uses SPI1; LCD keeps its existing SPI0/DMA path. Recovery USB owns SD
 * exclusively, after queued saves drain; ordinary writes stay suspended. */
#include "fish_save.h"
#include "fish_save_storage.h"
#include "fish_save_crypto.h"
#include "board.h"
#include "hpm_otp_drv.h"
#include "rtthread.h"
#include "ff.h"
#include "fish_fs_hpm.h"
#include "spi_sd_adapt.h"
#include <string.h>


static struct rt_thread worker;
static struct rt_semaphore request;
static char worker_stack[4096] __attribute__((aligned(8)));
static uint8_t queued[FISH_SAVE_FILE_BYTES];
static uint8_t checked[FISH_SAVE_FILE_BYTES]; /* Single worker; keep journal off its stack. */
static unsigned queued_slot;
static int completion;
static bool busy,ready;
static uint32_t started_at;
static unsigned diagnostic;
unsigned fish_storage_diagnostic(void) {return diagnostic;}
unsigned fish_storage_write_diagnostic(void) {return spi_sd_write_diagnostic();}
static int failed(unsigned stage,FRESULT result) {diagnostic=(stage<<8)|(unsigned)result;return -1;}
extern int64_t get_system_time_ms(void);
#define SAVE_DIRECTORY "3:/FishingGame_Saves_DO_NOT_DELETE"
static const char *paths[]={SAVE_DIRECTORY "/NorthWetland_V13_Save_A_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V13_Save_B_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V12_Save_A_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V12_Save_B_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V11_Save_A_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V11_Save_B_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V10_Save_A_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V10_Save_B_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V9_Save_A_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V9_Save_B_DO_NOT_DELETE.sav",SAVE_DIRECTORY "/NorthWetland_V8_Save_A_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V8_Save_B_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V7_Save_A_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V7_Save_B_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V6_Save_A_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V6_Save_B_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V5_Save_A_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_V5_Save_B_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_Save_A_DO_NOT_DELETE.sav",
    SAVE_DIRECTORY "/NorthWetland_Save_B_DO_NOT_DELETE.sav"};

static int storage_read_locked(unsigned slot,uint8_t *data,size_t len)
{
    if(!ready || slot>=FISH_SAVE_SLOT_COUNT) return -1;
    FIL f;UINT count=0;FRESULT result=f_open(&f,paths[slot],FA_READ);
    if(result==FR_NO_FILE || result==FR_NO_PATH) return -2;
    if(result!=FR_OK) return failed(4,result);
    bool size_ok=f_size(&f)==len;
    if(size_ok) result=f_read(&f,data,(UINT)len,&count);
    FRESULT closed=f_close(&f);
    if(result!=FR_OK || closed!=FR_OK) return failed(result!=FR_OK?5:6,result!=FR_OK?result:closed);
    if(!size_ok || count!=len){failed(17,0);return -3;}
    return 0;
}
int fish_storage_read(unsigned slot,uint8_t *data,size_t len)
{
    if(fish_fs_lock())return failed(1,0);
    int result=storage_read_locked(slot,data,len);fish_fs_unlock();return result;
}
static void save_worker(void *arg)
{
    (void)arg;
    for(;;) {
        rt_sem_take(&request,RT_WAITING_FOREVER);
        int result=-1;FIL f;UINT count=0;
        if(fish_fs_lock()) {completion=failed(1,0);continue;}
        FRESULT r=f_open(&f,paths[queued_slot],FA_WRITE|FA_OPEN_ALWAYS);
        if(r==FR_OK) {
            r=f_write(&f,queued,sizeof(queued),&count);
            if(r!=FR_OK || count!=sizeof(queued))failed(8,r);
            else {r=f_truncate(&f);if(r!=FR_OK)failed(9,r);}
            FRESULT synced=f_sync(&f);if(r==FR_OK && synced!=FR_OK)failed(10,synced);
            FRESULT closed=f_close(&f);if(r==FR_OK && synced==FR_OK && closed!=FR_OK)failed(11,closed);
            if(r==FR_OK && count==sizeof(queued) && synced==FR_OK && closed==FR_OK &&
               !storage_read_locked(queued_slot,checked,sizeof(checked))) {
                if(!memcmp(queued,checked,sizeof(queued))) {result=1;diagnostic=0;}
                else failed(12,0);
            }
        } else failed(7,r);
        fish_fs_unlock();
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
    if(all_zero || all_ff) return failed(2,0);
    fish_save_device_key(uuid,key);
    if(ready)return 0; /* Boot recovery may retry after USB restores SD access. */
    if(fish_fs_lock())return failed(1,0);
    FRESULT made=f_mkdir(SAVE_DIRECTORY);if(made!=FR_OK && made!=FR_EXIST) {fish_fs_unlock();return failed(3,made);}
    /* A visible, human-readable ownership note; never replace an existing note. */
    FIL note;
    if(f_open(&note,SAVE_DIRECTORY "/README_V13_DO_NOT_DELETE.txt",FA_WRITE|FA_CREATE_NEW)==FR_OK) {
        static const char info[]="钓鱼游戏存档，请勿删除 / Fishing game saves: DO NOT DELETE.\r\n"
            "V13_A、V13_B 是本版存档，V12、V11、V10、V9、V8、V7、V6、V5和原 A、B 保留用于旧版。所有文件请保留。\r\n"
            "V13保存具体位置、朝向、时间和天气，旧存档保留。不要格式化 SD 卡。\r\n"
            "建议定期把整个目录复制到电脑备份，恢复到同一块板卡。\r\n";
        UINT count;f_write(&note,info,sizeof(info)-1,&count);f_sync(&note);f_close(&note);
    }
    fish_fs_unlock();
    if(rt_sem_init(&request,"fsreq",0,RT_IPC_FLAG_FIFO)!=RT_EOK) return failed(13,0);
    if(rt_thread_init(&worker,"fishsave",save_worker,RT_NULL,worker_stack,sizeof(worker_stack),24,10)!=RT_EOK) return failed(14,0);
    ready=true;
    if(rt_thread_startup(&worker)!=RT_EOK) {ready=false;return failed(15,0);}
    return 0;
}
int fish_storage_begin(unsigned slot,const uint8_t *data,size_t len)
{
    if(!ready || slot>1 || len!=sizeof(queued)) return failed(16,0);
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
