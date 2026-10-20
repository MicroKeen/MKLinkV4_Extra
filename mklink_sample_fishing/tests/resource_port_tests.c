/* Actual HPM resource port, mocked ROM/peripherals: no physical media writes. */
#include "fish_resources.h"
#include "fish_hpm_runtime.h"
#include "board.h"
#include "hpm_romapi.h"
#include "rtthread.h"
#include "ff.h"
#include <assert.h>
#include <stdio.h>
static unsigned depth,beats,configs,programs,erases,invalidations;
static unsigned config_error,property_error,write_error;
static unsigned instruction_sync_pending,syncs;
static unsigned stage;
void fish_hpm_load_stage(unsigned value) {assert(!depth);stage=value;}
void fish_hpm_load_stage_at(unsigned value,uint32_t detail) {(void)detail;fish_hpm_load_stage(value);}
rt_base_t rt_hw_interrupt_disable(void) {assert(!depth);depth=1;return 0x55;}
void rt_hw_interrupt_enable(rt_base_t irq) {assert(depth==1 && irq==0x55 && !instruction_sync_pending);depth=0;}
void fencei(void) {assert(depth==1 && instruction_sync_pending);instruction_sync_pending=0;++syncs;}
void fish_hpm_storage_heartbeat(void) {assert(!depth);++beats;}
int rom_xpi_nor_auto_config(void *base,xpi_nor_config_t *cfg,xpi_nor_config_option_t *opt)
{
    assert(depth==1 && base==HPM_XPI0 && cfg);
    assert(stage==FISH_HPM_LOAD_CONFIGURE);
    assert(opt->header.U==BOARD_APP_XPI_NOR_CFG_OPT_HDR);
    assert(opt->option0.U==BOARD_APP_XPI_NOR_CFG_OPT_OPT0 && opt->option1.U==BOARD_APP_XPI_NOR_CFG_OPT_OPT1);
    instruction_sync_pending=1;++configs;return config_error?-1:0;
}
int rom_xpi_nor_get_property(void *base,xpi_nor_config_t *cfg,unsigned id,uint32_t *out)
{
    assert(!depth && base==HPM_XPI0 && cfg && id<3);
    assert(stage==FISH_HPM_LOAD_PROPERTIES);
    const uint32_t values[]={0x100000,4096,256};*out=values[id];
    return property_error?-1:0;
}
int rom_xpi_nor_program(void *base,unsigned ch,xpi_nor_config_t *cfg,const uint32_t *data,uint32_t at,unsigned n)
{
    assert(depth==1 && base==HPM_XPI0 && ch==xpi_xfer_channel_auto && cfg && data);
    assert(stage==FISH_HPM_LOAD_PROGRAM);
    assert(at>=0x80000 && at+n<=0xf3000);++programs;return write_error?-1:0;
}
int rom_xpi_nor_erase(void *base,unsigned ch,xpi_nor_config_t *cfg,uint32_t at,unsigned n)
{
    assert(depth==1 && base==HPM_XPI0 && ch==xpi_xfer_channel_auto && cfg);
    assert(stage==FISH_HPM_LOAD_ERASE);
    assert(at>=0x80000 && !(at%4096) && n==4096 && at+n<=0xf3000);
    ++erases;return write_error?-1:0;
}
void l1c_dc_invalidate(uint32_t at,unsigned n)
{assert(!depth && at>=FISH_CACHE_BASE && at+n<=FISH_CACHE_BASE+FISH_CACHE_BYTES && !(at%32) && !(n%32));++invalidations;}
int fish_fs_resource_lock(void) {assert(!"No filesystem access during NOR configuration/write");return -1;}
void fish_fs_unlock(void) {assert(0);}
FRESULT f_open(FIL *f,const char *p,unsigned m) {(void)f;(void)p;(void)m;assert(0);return FR_DISK_ERR;}
FRESULT f_lseek(FIL *f,unsigned n) {(void)f;(void)n;assert(0);return FR_DISK_ERR;}
FRESULT f_read(FIL *f,void *p,unsigned n,UINT *got) {(void)f;(void)p;(void)n;(void)got;assert(0);return FR_DISK_ERR;}
FRESULT f_close(FIL *f) {(void)f;assert(0);return FR_DISK_ERR;}
int main(void)
{
    uint32_t data[16]={0};
    assert(fish_resource_erase(0)==-1 && fish_resource_program(0,data,sizeof(data))==-1 && !beats);
    config_error=1;assert(fish_resource_port_init()==-1 && configs==1 && beats==2 && !depth);
    config_error=0;property_error=1;assert(fish_resource_port_init()==-1 && configs==2 && !depth);
    property_error=0;assert(!fish_resource_port_init() && configs==3 && !depth);
    assert(!fish_resource_port_init() && configs==3 && beats==6 && syncs==3);
    assert(fish_resource_erase(1)==-1 && fish_resource_erase(FISH_CACHE_BYTES)==-1);
    assert(fish_resource_program(0,NULL,1)==-1 && fish_resource_program(FISH_CACHE_BYTES,data,1)==-1);
    assert(fish_resource_program(0,(void *)0x80023000u,4)==-1 && !programs && !erases);
    assert(!fish_resource_erase(FISH_CACHE_BYTES-4096));
    assert(!fish_resource_program(0,data,sizeof(data)));
    write_error=1;assert(fish_resource_erase(0)==-1 && fish_resource_program(0,data,4)==-1);
    assert(erases==2 && programs==2 && beats==14 && invalidations==4 && !depth && stage==FISH_HPM_LOAD_IDLE);
    puts("PASS: actual NOR port config/erase/program mask IRQs, heartbeat boundaries, failure returns and fixed cache bounds; no filesystem access");
}
