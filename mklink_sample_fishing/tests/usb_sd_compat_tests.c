/* Actual FatFs + HPM disk/USB ports on a disposable RAM card. Drive 0 models
 * Windows via MSC callbacks; drive 3 is the game's normal mounted volume. */
#include "board.h"
#include "rtthread.h"
#include "ff.h"
#include "diskio.h"
#include "hpm_spi_sd_disk.h"
#include "fish_fs_hpm.h"
#include "fish_media.h"
#include "fish_resources.h"
#include "fish_usb_hpm.h"
#include "usbd_core.h"
#include "usbd_msc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum {CARD_SECTORS=262144,STOCK_SECTORS=245760};
static uint8_t *card;
static unsigned nesting,device_inits,device_stops,writes,command_busy;
static bool host_attached,irq_off;
static uint32_t now;
static uint8_t cache[FISH_CACHE_BYTES];
static FIL resource;
static bool resource_open;
static const uint8_t original[]="Original MKLink downloader firmware/settings must survive import.";
DRESULT test_board_disk_read(BYTE,BYTE *,LBA_t,UINT);
DRESULT test_board_disk_write(BYTE,const BYTE *,LBA_t,UINT);
DRESULT test_board_disk_ioctl(BYTE,BYTE,void *);
DSTATUS test_board_disk_initialize(BYTE);
DSTATUS test_board_disk_status(BYTE);
/* The blocking SPI disk path must not translate a CPU pointer or invalidate
 * freshly CPU-written bytes. Regressions fail here even on a cacheless host. */
uint32_t core_local_mem_to_sys_address(unsigned core,uint32_t addr)
{(void)core;(void)addr;assert(!"CPU SPI must retain its CPU pointer");return 0;}
void l1c_dc_invalidate(uint32_t at,uint32_t n)
{(void)at;(void)n;assert(!"CPU SPI must not discard freshly written cache lines");}
void l1c_dc_writeback(uint32_t at,uint32_t n)
{(void)at;(void)n;assert(!"CPU SPI does not use a DMA address");}
int rt_mutex_init(struct rt_mutex *m,const char *name,int flags)
{(void)m;assert(!strcmp(name,"fishfs") && flags==RT_IPC_FLAG_PRIO);return 0;}
int rt_mutex_take(struct rt_mutex *m,int wait) {(void)m;assert(wait==-1);++nesting;return 0;}
int rt_mutex_release(struct rt_mutex *m) {(void)m;assert(nesting);--nesting;return 0;}
int spi_sd_init(void) {return status_success;}
hpm_stat_t sdcard_spi_get_card_info(spi_sdcard_info_t *info)
{
    *info=(spi_sdcard_info_t){.capacity=(uint64_t)CARD_SECTORS*512,
        .block_size=512,.block_count=CARD_SECTORS,.csd={.device_size=255}};
    return status_success;
}
hpm_stat_t sdcard_spi_status(void) {return status_success;}
hpm_stat_t sdcard_spi_read_multi_block(uint8_t *out,uint32_t sector,uint32_t n)
{assert(n && sector<CARD_SECTORS && n<=CARD_SECTORS-sector);memcpy(out,card+(size_t)sector*512,(size_t)n*512);return 0;}
hpm_stat_t sdcard_spi_write_multi_block(uint8_t *in,uint32_t sector,uint32_t n)
{
    assert(n && sector<CARD_SECTORS && n<=CARD_SECTORS-sector);
    if(host_attached)assert(!resource_open && fish_fs_resource_lock());
    ++writes;memcpy(card+(size_t)sector*512,in,(size_t)n*512);return 0;
}
hpm_stat_t sdcard_spi_write_block(uint32_t sector,uint8_t *in)
{return sdcard_spi_write_multi_block(in,sector,1);}
hpm_stat_t spi_sd_write_sector(uint32_t sector,const uint8_t *in,bool high_capacity)
{(void)high_capacity;return sdcard_spi_write_block(sector,(uint8_t *)in);}
DSTATUS disk_initialize(BYTE drive)
{if(drive==3)return test_board_disk_initialize(3);assert(!drive && host_attached);return 0;}
DSTATUS disk_status(BYTE drive)
{if(drive==3)return test_board_disk_status(3);return !drive && host_attached?0:STA_NOINIT;}
DRESULT disk_ioctl(BYTE drive,BYTE cmd,void *out)
{
    if(drive==3)return test_board_disk_ioctl(3,cmd,out);
    assert(!drive && host_attached);
    uint32_t count,size;usbd_msc_get_cap(0,0,&count,&size);
    if(cmd==GET_SECTOR_COUNT) {*(LBA_t *)out=count;return RES_OK;}
    if(cmd==GET_SECTOR_SIZE) {*(WORD *)out=(WORD)size;return RES_OK;}
    if(cmd==CTRL_SYNC)return RES_OK;
    return RES_PARERR;
}
DRESULT disk_read(BYTE drive,BYTE *out,LBA_t sector,UINT n)
{
    if(drive==3)return test_board_disk_read(3,out,sector,n);
    assert(!drive && host_attached);
    return usbd_msc_sector_read(0,0,sector,out,n*512)?RES_ERROR:RES_OK;
}
DRESULT disk_write(BYTE drive,const BYTE *in,LBA_t sector,UINT n)
{
    if(drive==3)return test_board_disk_write(3,in,sector,n);
    assert(!drive && host_attached);
    return usbd_msc_sector_write(0,0,sector,(BYTE *)in,n*512)?RES_ERROR:RES_OK;
}
void board_init_usb(USB_Type *base) {assert((uintptr_t)base==CONFIG_HPM_USBD_BASE);}
void intc_set_irq_priority(int irq,int priority) {assert(irq==7 && priority==2);}
void intc_m_disable_irq(int irq) {assert(irq==7);irq_off=true;}
void intc_m_enable_irq(int irq) {assert(irq==7);irq_off=false;}
int64_t get_system_time_ms(void) {return now;}
uint32_t fish_hpm_millis(void) {return now;}
bool fish_hpm_watchdog_reset(void) {return false;}
unsigned fish_hpm_reset_stage(void) {return 0;}
void fish_hpm_reset_info(fish_fault_info_t *out) {*out=(fish_fault_info_t){0};}
void usbd_desc_register(uint8_t bus,const struct usb_descriptor *descriptor)
{assert(!bus && descriptor && descriptor->device_descriptor_callback(USB_SPEED_HIGH));}
struct usbd_interface *usbd_msc_init_intf(uint8_t bus,struct usbd_interface *i,uint8_t out,uint8_t in)
{assert(!bus && out==2 && in==0x81);return i;}
void usbd_add_interface(uint8_t bus,struct usbd_interface *i) {assert(!bus && i);}
int usbd_initialize(uint8_t bus,uintptr_t base,void (*event)(uint8_t,uint8_t))
{
    assert(!bus && base==CONFIG_HPM_USBD_BASE && event && !nesting);
    uint32_t count,size;usbd_msc_get_cap(0,0,&count,&size);
    assert(count==STOCK_SECTORS && size==512);++device_inits;host_attached=true;return 0;
}
int usbd_deinitialize(uint8_t bus)
{assert(!bus && irq_off);++device_stops;host_attached=false;return 0;}
bool fish_usb_msc_idle(uint8_t bus) {assert(!bus);return !command_busy;}
void usbd_msc_polling(uint8_t bus) {assert(!bus);}
int fish_resource_port_init(void) {return 0;}
const uint8_t *fish_resource_flash(void) {return cache;}
int fish_resource_open(unsigned map,uint32_t *bytes)
{
    assert(!resource_open);if(fish_fs_resource_lock())return -1;
    char path[128];snprintf(path,sizeof(path),"3:/FishingGame_Resources_DO_NOT_DELETE/maps/%s.fgpk",map?"1_city_river":"0_wetland");
    FRESULT result=f_open(&resource,path,FA_READ);
    if(result==FR_OK) {*bytes=(uint32_t)f_size(&resource);resource_open=true;}
    fish_fs_unlock();return result==FR_OK?0:(result==FR_NO_FILE || result==FR_NO_PATH?-2:-1);
}
int fish_resource_read(uint32_t at,void *out,unsigned n)
{
    assert(resource_open);if(fish_fs_resource_lock())return -1;
    UINT got=0;FRESULT result=f_lseek(&resource,at);
    if(result==FR_OK)result=f_read(&resource,out,n,&got);
    fish_fs_unlock();return result==FR_OK && got==n?0:-1;
}
void fish_resource_close(void)
{assert(resource_open && !fish_fs_resource_lock());assert(f_close(&resource)==FR_OK);resource_open=false;fish_fs_unlock();}
int fish_resource_erase(uint32_t at) {(void)at;assert(!"Import probes must never erase flash");return -1;}
int fish_resource_program(uint32_t at,const void *p,unsigned n) {(void)at;(void)p;(void)n;assert(!"Import probes must never program flash");return -1;}
static void put16(uint8_t *p,unsigned n) {p[0]=(uint8_t)n;p[1]=(uint8_t)(n>>8);}
static void put32(uint8_t *p,uint32_t n) {put16(p,n);put16(p+2,n>>16);}
static void fixture(void)
{
    card=calloc(CARD_SECTORS,512);assert(card);memset(cache,255,sizeof(cache));
    uint8_t *p=card;p[0]=0xeb;p[1]=0x3c;p[2]=0x90;memcpy(p+3,"MSDOS5.0",8);
    put16(p+11,512);p[13]=4;put16(p+14,1);p[16]=2;put16(p+17,512);
    p[21]=0xf8;put16(p+22,240);put16(p+24,32);put16(p+26,64);put32(p+32,STOCK_SECTORS);
    p[38]=0x29;memcpy(p+43,"MKLINK     ",11);memcpy(p+54,"FAT16   ",8);
    p[510]=0x55;p[511]=0xaa;
    for(unsigned f=0;f<2;++f) {p=card+(1+f*240)*512;p[0]=0xf8;p[1]=p[2]=p[3]=0xff;}
    memset(card+(size_t)STOCK_SECTORS*512,0xa7,(CARD_SECTORS-STOCK_SECTORS)*512);
}
static void write_file(const char *path,const void *data,unsigned bytes)
{
    FIL f;UINT n=0;assert(f_open(&f,path,FA_WRITE|FA_CREATE_NEW)==FR_OK);
    assert(f_write(&f,data,bytes,&n)==FR_OK && n==bytes);assert(f_close(&f)==FR_OK);
}
static void verify_file(const char *path)
{
    FIL f;UINT n=0;uint8_t data[sizeof(original)];assert(f_open(&f,path,FA_READ)==FR_OK);
    assert(f_read(&f,data,sizeof(data),&n)==FR_OK && n==sizeof(data));assert(!memcmp(data,original,n));
    assert(f_close(&f)==FR_OK);
}
static void copy_pack(unsigned map)
{
    char source[1024],target[128];
    snprintf(source,sizeof(source),"%s/maps/%s.fgpk",FISH_TEST_PACK_DIR,map?"1_city_river":"0_wetland");
    snprintf(target,sizeof(target),"0:/FishingGame_Resources_DO_NOT_DELETE/maps/%s.fgpk",map?"1_city_river":"0_wetland");
    FILE *input=fopen(source,"rb");assert(input);FIL f;assert(f_open(&f,target,FA_WRITE|FA_CREATE_NEW)==FR_OK);
    uint8_t data[4096];size_t n;
    while((n=fread(data,1,sizeof(data),input))) {UINT wrote=0;assert(f_write(&f,data,(UINT)n,&wrote)==FR_OK && wrote==n);}
    assert(!ferror(input));fclose(input);assert(f_close(&f)==FR_OK);
}
static void until(fish_media_phase_t expected,unsigned limit)
{while(fish_media_phase()!=expected && limit--) {now+=20;fish_media_tick(now,false);}assert(fish_media_phase()==expected);}
int main(void)
{
    fixture();
    assert(!fish_fs_lock());
    write_file("3:/MicroLink.rbl",original,sizeof(original));
    assert(f_mkdir("3:/OriginalFolder")==FR_OK);write_file("3:/OriginalFolder/settings.ini",original,sizeof(original));
    assert(f_mkdir("3:/FishingGame_Saves_DO_NOT_DELETE")==FR_OK);
    write_file("3:/FishingGame_Saves_DO_NOT_DELETE/old-save.sav",original,sizeof(original));
    fish_fs_unlock();uint8_t boot_before[512];memcpy(boot_before,card,512);
    unsigned before=writes;fish_media_begin(now);until(FISH_MEDIA_WAIT,5);
    assert(!device_inits && writes==before);
    now+=20;fish_media_tick(now,true);assert(!device_inits); /* Save drain. */
    now+=20;fish_media_tick(now,false);assert(device_inits==1 && fish_media_visible());
    assert(writes==before && fish_fs_lock()); /* Host owns SD exclusively. */
    FATFS host;assert(f_mount(&host,"0:",1)==FR_OK);
    assert(f_mkdir("0:/FishingGame_Resources_DO_NOT_DELETE")==FR_OK);
    assert(f_mkdir("0:/FishingGame_Resources_DO_NOT_DELETE/maps")==FR_OK);
    copy_pack(0);copy_pack(1);assert(f_mount(NULL,"0:",0)==FR_OK);
    uint8_t scratch[1025];unsigned io_before=writes;
    assert(usbd_msc_sector_read(0,0,0,scratch+1,512)==0);assert(!memcmp(scratch+1,boot_before,512));
    assert(usbd_msc_sector_write(0,0,STOCK_SECTORS,scratch,512)<0);
    assert(usbd_msc_sector_write(0,0,UINT32_MAX,scratch,1024)<0);
    assert(writes==io_before);
    now+=3000;fish_media_tick(now,false);
    assert(fish_media_phase()==FISH_MEDIA_VERIFY && resource_open && writes==io_before);
    memcpy(scratch,card+(size_t)(STOCK_SECTORS-1)*512,512);
    assert(usbd_msc_sector_write(0,0,STOCK_SECTORS-1,scratch,512)==0);
    assert(fish_media_phase()==FISH_MEDIA_WAIT && !resource_open);
    io_before=writes;now+=3000;
    until(FISH_MEDIA_SETTLE,1000);assert(writes==io_before && !resource_open);
    /* A late host metadata write invalidates that snapshot before disk I/O. */
    memcpy(scratch,card+(size_t)(STOCK_SECTORS-1)*512,512);
    assert(usbd_msc_sector_write(0,0,STOCK_SECTORS-1,scratch,512)==0);
    assert(fish_media_phase()==FISH_MEDIA_WAIT);io_before=writes;
    now+=3000;until(FISH_MEDIA_SETTLE,1000);assert(writes==io_before);
    command_busy=1;now+=1500;fish_media_tick(now,false);assert(!device_stops);
    command_busy=0;now+=20;fish_media_tick(now,false);assert(fish_media_ready() && device_stops==1);
    assert(!fish_media_visible() && !fish_fs_lock());
    verify_file("3:/MicroLink.rbl");verify_file("3:/OriginalFolder/settings.ini");
    verify_file("3:/FishingGame_Saves_DO_NOT_DELETE/old-save.sav");fish_fs_unlock();
    assert(!memcmp(boot_before,card,512));
    for(size_t i=(size_t)STOCK_SECTORS*512;i<(size_t)CARD_SECTORS*512;++i)assert(card[i]==0xa7);
    assert(!nesting && device_inits==1 && device_stops==1);
    /* Refuse a damaged/unrecognized volume before another USB enumeration.
     * Only the synthetic fixture is changed here; no repair is attempted. */
    before=writes;card[510]=0;
    assert(fish_media_port_start()<0 && device_inits==1 && writes==before);
    memcpy(card,boot_before,512);
    assert(!fish_fs_lock());verify_file("3:/MicroLink.rbl");fish_fs_unlock();
    assert(writes==before && !nesting);
    /* Exercise the actual scene-thread poll entry, rather than directly
     * calling the state machine as earlier tests did. No frame callback is
     * assumed while the polling entry finishes a hidden boot check. */
    fish_media_begin(now);
    for(unsigned i=0;i<1000 && !fish_media_ready();++i) {now+=20;fish_usb_poll();}
    assert(fish_media_ready());
    assert(writes==before && device_inits==1); /* Next valid boot stays hidden. */
    free(card);puts("PASS: actual FatFs/USB/disk ports; original downloader files+saves survive two real package copies; stock capacity; CPU pointer/cache path; exclusive ownership; read-only CRC probes; late writes and command drain; reserved tail unchanged");
}
