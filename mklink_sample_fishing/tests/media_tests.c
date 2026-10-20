#include "fish_media.h"
#include "fish_resources.h"
#include "fish_asset_layout.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static FILE *file;
static uint8_t cache[FISH_CACHE_BYTES];
static unsigned opens,closes,probes,draining,missing,wrong_version,corrupt,truncated,current_map;
static bool disk,probe;
static uint32_t now;
static unsigned diagnostics;
static fish_fault_info_t fault_info;
int fish_resource_port_init(void) {return 0;}
const uint8_t *fish_resource_flash(void) {return cache;}
int fish_resource_open(unsigned map,uint32_t *size)
{
    assert(!file && (!disk || probe));
    if(missing&(1u<<map))return -2;
    char path[1024];snprintf(path,sizeof(path),"%s/maps/%s.fgpk",FISH_TEST_PACK_DIR,map?"1_city_river":"0_wetland");
    file=fopen(path,"rb");assert(file);current_map=map;
    fseek(file,0,SEEK_END);*size=(uint32_t)ftell(file)-((truncated&(1u<<map))?1:0);return 0;
}
int fish_resource_read(uint32_t at,void *out,unsigned bytes)
{
    assert(file && (!disk || probe));
    if(fseek(file,at,SEEK_SET) || fread(out,1,bytes,file)!=bytes)return -1;
    uint8_t *p=out;
    if(!at && (wrong_version&(1u<<current_map))) {
        p[24]^=1;uint32_t crc=fish_resource_crc(~0u,p,28)^~0u;
        for(unsigned i=0;i<4;i++)p[28+i]=(uint8_t)(crc>>(i*8));
    }
    if(at==64 && (corrupt&(1u<<current_map)))p[0]^=1;
    return 0;
}
void fish_resource_close(void) {assert(file);fclose(file);file=NULL;}
int fish_resource_erase(uint32_t at) {(void)at;assert(!"Read-only import probe must never erase");return -1;}
int fish_resource_program(uint32_t at,const void *p,unsigned n) {(void)at;(void)p;(void)n;assert(!"Probe must never program");return -1;}
int fish_media_port_start(void) {assert(!disk && !file);disk=true;++opens;return 0;}
int fish_media_port_stop(void)
{
    assert(disk && !file && !probe);
    if(draining) {--draining;return 1;}
    disk=false;++closes;return 0;
}
int fish_media_port_probe(bool enabled)
{
    assert(disk && !file);
    probe=enabled;if(enabled)++probes;return 0;
}
static void tick(bool busy) {now+=20;fish_media_tick(now,busy);}
bool fish_media_port_watchdog_reset(void) {return diagnostics!=0;}
unsigned fish_media_port_reset_stage(void) {return diagnostics;}
void fish_media_port_reset_info(fish_fault_info_t *out) {*out=fault_info;}
static void until(fish_media_phase_t state,unsigned limit)
{while(fish_media_phase()!=state && limit--)tick(false);assert(fish_media_phase()==state);}
static void write(void)
{
    fish_media_host_write(now,512);assert(!file && !probe && disk);
    assert(fish_media_status().receiving && fish_media_status().received_bytes>=512);
    assert(fish_media_phase()==FISH_MEDIA_WAIT);
}
int main(void)
{
    memset(cache,255,sizeof(cache));fish_media_begin(now);
    unsigned previous=0;bool smooth=false;
    for(unsigned i=0;fish_media_phase()==FISH_MEDIA_CHECK && i<1000;++i) {
        tick(false);unsigned current=fish_media_progress();assert(current>=previous && current-previous<=2);
        if(current>10 && current<45)smooth=true;
        previous=current;
    }
    assert(fish_media_ready() && smooth && !opens && !closes && !disk);
    for(unsigned i=0;i<100;i++)tick(false);
    assert(!opens); /* Normal boot never initializes USB. */
    missing=2;fish_media_begin(now);until(FISH_MEDIA_WAIT,1000);
    assert(fish_media_status().error==FISH_RES_ERR_MISSING && fish_media_status().map==1);
    for(unsigned i=0;i<10;i++)tick(true);
    assert(!opens); /* Existing asynchronous save must drain first. */
    tick(false);assert(opens==1 && disk && fish_media_visible());
    now+=3000;until(FISH_MEDIA_WAIT,1000);tick(false);
    /* Missing second map, a self-consistent wrong version, an incomplete file
     * and corrupted payload all leave the same disk visible without flapping. */
    for(unsigned c=0;c<4;c++) {
        missing=c==0?2:0;wrong_version=c==1?2:0;
        truncated=c==2?2:0;corrupt=c==3?2:0;write();now+=3000;
        until(FISH_MEDIA_VERIFY,3);until(FISH_MEDIA_WAIT,1000);
        static const fish_res_error_t causes[]={FISH_RES_ERR_MISSING,FISH_RES_ERR_VERSION,FISH_RES_ERR_SIZE,FISH_RES_ERR_CRC};
        assert(fish_media_status().error==causes[c] && !fish_media_status().receiving);
        assert(opens==1 && !closes && disk);
    }
    missing=wrong_version=truncated=corrupt=0;
    write();now+=3000;until(FISH_MEDIA_VERIFY,3);
    for(unsigned i=0;i<10;i++)tick(false);
    assert(file && probe);write(); /* A new host write aborts the CRC snapshot. */
    now+=3000;until(FISH_MEDIA_SETTLE,1000);assert(!file && !probe && disk);
    write(); /* Even a late FAT metadata write restarts the entire check. */
    now+=3000;until(FISH_MEDIA_SETTLE,1000);
    for(unsigned i=0;i<74;i++)tick(false);
    assert(!closes && disk);draining=2;
    tick(false);assert(!closes);tick(false);assert(!closes);tick(false);
    assert(fish_media_ready() && closes==1 && !disk && !fish_media_visible());
    for(unsigned i=0;i<1000;i++)tick(false);
    assert(opens==1 && closes==1);
    diagnostics=8;fault_info=(fish_fault_info_t){.detail=0x70040,.cause=6,.pc=0x80045100,.value=0};
    unsigned before_probes=probes;fish_media_begin(now);
    assert(fish_media_phase()==FISH_MEDIA_FAULT && !file && !disk);
    for(unsigned i=0;i<1000;i++)tick(false);
    assert(probes==before_probes && fish_media_phase()==FISH_MEDIA_FAULT);
    assert(fish_media_status().fault.pc==0x80045100 && fish_media_status().reset_stage==8);
    fish_media_retry(now);until(FISH_MEDIA_READY,1000);
    diagnostics=0;fault_info=(fish_fault_info_t){0};
    /* Millisecond wrap does not reopen a good set or bypass the quiet period. */
    now=UINT32_MAX-50;missing=1;fish_media_begin(now);until(FISH_MEDIA_WAIT,3);tick(false);
    missing=0;write();for(unsigned i=0;i<100;i++)tick(false);
    assert(fish_media_phase()==FISH_MEDIA_WAIT);now+=1000;
    until(FISH_MEDIA_READY,2000);assert(opens==2 && closes==2);
    printf("PASS: full two-map CRC/version checks; missing/mixed/truncated/corrupt files; save drain; write-aborted probes; command drain; one attach/detach; timer wrap (%u probes)\n",probes);
}
