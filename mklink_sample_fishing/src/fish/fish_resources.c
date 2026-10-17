#include "fish_resources.h"
#include "fish_asset_layout.h"
#include <string.h>
_Static_assert(FISH_CACHE_BASE+FISH_CACHE_BYTES==0x800f3000u,"Protect final flash reserve");
_Static_assert(FISH_ASSET_PAYLOAD_BYTES<=FISH_CACHE_BYTES-FISH_CACHE_SECTOR,"Map exceeds cache");
static uint8_t block[FISH_CACHE_SECTOR] __attribute__((aligned(32)));
static uint8_t header[FISH_PACK_HEADER] __attribute__((aligned(32)));
static fish_res_phase_t phase;
static unsigned target,active=2,progress;
static uint32_t at,crc,expected;
static bool valid,source_open;
static uint32_t word(const uint8_t *p) {return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
uint32_t fish_resource_crc(uint32_t crc,const void *data,unsigned bytes)
{
    const uint8_t *p=data;
    while(bytes--) {crc^=*p++;for(unsigned i=0;i<8;i++)crc=(crc>>1)^(0xedb88320u&-(crc&1u));}
    return crc;
}
static bool header_ok(const uint8_t *p)
{
    if(memcmp(p,"FGRP",4) || word(p+4)!=1 || word(p+8)!=target ||
       word(p+12)!=FISH_ASSET_LAYOUT_CRC || word(p+16)!=FISH_ASSET_PAYLOAD_BYTES ||
       word(p+20)!=(target?FISH_ASSET_MAP1_CRC:FISH_ASSET_MAP0_CRC) ||
       word(p+24)!=(fish_resource_crc(~0u,p,24)^~0u)) return false;
    for(unsigned i=28;i<FISH_PACK_HEADER;i++)if(p[i])return false;
    return true;
}
static void close_source(void) {if(source_open)fish_resource_close();source_open=false;}
static void fail(void) {close_source();phase=FISH_RES_ERROR;}
static void source_begin(void)
{
    uint32_t size;
    if(fish_resource_open(target,&size)) {fail();return;}
    source_open=true;
    if(size!=FISH_PACK_HEADER+FISH_ASSET_PAYLOAD_BYTES ||
       fish_resource_read(0,header,sizeof(header)) || !header_ok(header)) {fail();return;}
    at=0;crc=~0u;expected=word(header+20);phase=FISH_RES_SOURCE_CHECK;
}
void fish_resources_begin(unsigned map)
{
    close_source();target=map;progress=0;at=0;crc=~0u;
    if(map>=2 || fish_resource_port_init()) {phase=FISH_RES_ERROR;return;}
    const uint8_t *cache=fish_resource_flash();
    if(header_ok(cache)) {expected=word(cache+20);phase=FISH_RES_CACHE_CHECK;}
    else source_begin();
}
void fish_resources_tick(void)
{
    unsigned count=FISH_ASSET_PAYLOAD_BYTES-at;
    if(count>sizeof(block))count=sizeof(block);
    const uint8_t *cache=fish_resource_flash();
    switch(phase) {
    case FISH_RES_CACHE_CHECK:
        crc=fish_resource_crc(crc,cache+FISH_CACHE_SECTOR+at,count);at+=count;
        progress=at*100/FISH_ASSET_PAYLOAD_BYTES;
        if(at==FISH_ASSET_PAYLOAD_BYTES) {
            if((crc^~0u)==expected) {valid=true;active=target;phase=FISH_RES_DONE;progress=100;}
            else {valid=false;active=2;source_begin();progress=0;}
        }break;
    case FISH_RES_SOURCE_CHECK:
        if(fish_resource_read(FISH_PACK_HEADER+at,block,count)) {fail();break;}
        crc=fish_resource_crc(crc,block,count);at+=count;progress=at*20/FISH_ASSET_PAYLOAD_BYTES;
        if(at==FISH_ASSET_PAYLOAD_BYTES) {
            if((crc^~0u)!=expected) {fail();break;}
            at=0;phase=FISH_RES_ERASE_HEADER;
        }break;
    case FISH_RES_ERASE_HEADER:
        /* Commit sector is invalidated first; interruption can never expose a
         * mixture of maps as a valid resource set on the next boot. */
        valid=false;active=2;
        if(fish_resource_erase(0)) {fail();break;}
        phase=FISH_RES_ERASE_BLOCK;break;
    case FISH_RES_ERASE_BLOCK:
        if(fish_resource_read(FISH_PACK_HEADER+at,block,count) ||
           fish_resource_erase(FISH_CACHE_SECTOR+at)) {fail();break;}
        phase=FISH_RES_WRITE_BLOCK;break;
    case FISH_RES_WRITE_BLOCK:
        if(fish_resource_program(FISH_CACHE_SECTOR+at,block,count) ||
           memcmp(block,cache+FISH_CACHE_SECTOR+at,count)) {fail();break;}
        at+=count;progress=20+at*70/FISH_ASSET_PAYLOAD_BYTES;
        if(at==FISH_ASSET_PAYLOAD_BYTES) {at=0;crc=~0u;phase=FISH_RES_VERIFY;}
        else phase=FISH_RES_ERASE_BLOCK;break;
    case FISH_RES_VERIFY:
        crc=fish_resource_crc(crc,cache+FISH_CACHE_SECTOR+at,count);at+=count;
        progress=90+at*9/FISH_ASSET_PAYLOAD_BYTES;
        if(at==FISH_ASSET_PAYLOAD_BYTES) {
            if((crc^~0u)!=expected) {fail();break;}
            phase=FISH_RES_COMMIT;
        }break;
    case FISH_RES_COMMIT:
        if(fish_resource_program(0,header,sizeof(header)) || memcmp(cache,header,sizeof(header))) {fail();break;}
        valid=true;active=target;progress=100;phase=FISH_RES_DONE;close_source();break;
    default:break;
    }
}
bool fish_resources_valid(void) {return valid;}
unsigned fish_resources_map(void) {return active;}
unsigned fish_resources_progress(void) {return progress;}
fish_res_phase_t fish_resources_phase(void) {return phase;}
