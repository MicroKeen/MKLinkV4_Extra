#include "fish_resources.h"
#include "fish_fs_hpm.h"
#include "board.h"
#include "hpm_romapi.h"
#include "hpm_l1c_drv.h"
#include "rtthread.h"
#include "ff.h"
static xpi_nor_config_t config;
static bool configured,opened;
static FIL file;
int fish_resource_port_init(void)
{
    if(configured)return 0;
    xpi_nor_config_option_t option={0};
    option.header.U=BOARD_APP_XPI_NOR_CFG_OPT_HDR;
    option.option0.U=BOARD_APP_XPI_NOR_CFG_OPT_OPT0;
    option.option1.U=BOARD_APP_XPI_NOR_CFG_OPT_OPT1;
    if(rom_xpi_nor_auto_config(HPM_XPI0,&config,&option)!=status_success)return -1;
    uint32_t size=0,sector=0,page=0;
    if(rom_xpi_nor_get_property(HPM_XPI0,&config,xpi_nor_property_total_size,&size)!=status_success ||
       rom_xpi_nor_get_property(HPM_XPI0,&config,xpi_nor_property_sector_size,&sector)!=status_success ||
       rom_xpi_nor_get_property(HPM_XPI0,&config,xpi_nor_property_page_size,&page)!=status_success ||
       size<0x100000 || sector!=4096 || !page || page>4096)return -1;
    configured=true;return 0;
}
int fish_resource_open(unsigned map,uint32_t *bytes)
{
    static const char *paths[]={"3:/FishingGame_Resources_DO_NOT_DELETE/maps/0_wetland.fgpk",
        "3:/FishingGame_Resources_DO_NOT_DELETE/maps/1_city_river.fgpk"};
    if(map>=2 || fish_fs_lock())return -1;
    FRESULT r=f_open(&file,paths[map],FA_READ);
    if(r==FR_OK) {*bytes=f_size(&file);opened=true;}
    fish_fs_unlock();return r==FR_OK?0:-1;
}
int fish_resource_read(uint32_t offset,void *out,unsigned bytes)
{
    if(!opened || fish_fs_lock())return -1;
    UINT count=0;FRESULT r=f_lseek(&file,offset);
    if(r==FR_OK)r=f_read(&file,out,bytes,&count);
    fish_fs_unlock();return r==FR_OK && count==bytes?0:-1;
}
void fish_resource_close(void) {if(opened && !fish_fs_lock()) {f_close(&file);fish_fs_unlock();}opened=false;}
const uint8_t *fish_resource_flash(void) {return (const uint8_t *)FISH_CACHE_BASE;}
/* During erase/program, every executed instruction is in ILM or the silicon
 * ROM. IRQs stay masked until the ROM call returns with NOR ready for XIP. */
static ATTR_RAMFUNC __attribute__((noinline)) int cache_write_critical(uint32_t off,const void *data,unsigned bytes)
{
    rt_base_t irq=rt_hw_interrupt_disable();
    hpm_stat_t result;
    if(data)result=rom_xpi_nor_program(HPM_XPI0,xpi_xfer_channel_auto,&config,(const uint32_t *)data,0x80000u+off,bytes);
    else result=rom_xpi_nor_erase(HPM_XPI0,xpi_xfer_channel_auto,&config,0x80000u+off,4096);
    rt_hw_interrupt_enable(irq);
    return result==status_success?0:-1;
}
static void invalidate(uint32_t off,unsigned bytes)
{
    uint32_t begin=HPM_L1C_CACHELINE_ALIGN_DOWN(FISH_CACHE_BASE+off);
    uint32_t end=HPM_L1C_CACHELINE_ALIGN_UP(FISH_CACHE_BASE+off+bytes);
    l1c_dc_invalidate(begin,end-begin);
}
int fish_resource_erase(uint32_t offset)
{
    if(!configured || offset%4096 || offset>FISH_CACHE_BYTES-4096)return -1;
    int result=cache_write_critical(offset,0,4096);invalidate(offset,4096);return result;
}
int fish_resource_program(uint32_t offset,const void *data,unsigned bytes)
{
    if(!configured || !data || !bytes || offset>FISH_CACHE_BYTES || bytes>FISH_CACHE_BYTES-offset ||
       ((uintptr_t)data>=0x80000000u && (uintptr_t)data<0x80100000u))return -1;
    int result=cache_write_critical(offset,data,bytes);invalidate(offset,bytes);return result;
}
