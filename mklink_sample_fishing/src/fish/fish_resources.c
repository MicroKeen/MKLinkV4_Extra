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
static bool valid,source_open,check_only,source_error;
static fish_res_error_t error;
static uint32_t word(const uint8_t *p) {return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
uint32_t fish_resource_crc(uint32_t crc,const void *data,unsigned bytes)
{
    const uint8_t *p=data;
    while(bytes--) {crc^=*p++;for(unsigned i=0;i<8;i++)crc=(crc>>1)^(0xedb88320u&-(crc&1u));}
    return crc;
}
static fish_res_error_t header_error(const uint8_t *p)
{
    if(memcmp(p,"FGRP",4))return FISH_RES_ERR_PACK;
    if(word(p+4)!=FISH_ASSET_PACK_FORMAT || word(p+24)!=FISH_ASSET_VERSION_ID)return FISH_RES_ERR_VERSION;
    if(word(p+8)!=target ||
       word(p+12)!=FISH_ASSET_LAYOUT_CRC || word(p+16)!=FISH_ASSET_PAYLOAD_BYTES ||
       word(p+20)!=(target?FISH_ASSET_MAP1_CRC:FISH_ASSET_MAP0_CRC))return FISH_RES_ERR_PACK;
    if(word(p+28)!=(fish_resource_crc(~0u,p,28)^~0u))return FISH_RES_ERR_CRC;
    for(unsigned i=32;i<FISH_PACK_HEADER;i++)if(p[i])return FISH_RES_ERR_PACK;
    return FISH_RES_ERR_NONE;
}
static void close_source(void) {if(source_open)fish_resource_close();source_open=false;}
static void fail(fish_res_error_t cause) {error=cause;close_source();phase=FISH_RES_ERROR;}
static void source_begin(void)
{
    source_error=true;
    uint32_t size;
    int opened=fish_resource_open(target,&size);
    if(opened) {fail(opened==-2?FISH_RES_ERR_MISSING:FISH_RES_ERR_IO);return;}
    source_open=true;
    if(size!=FISH_PACK_HEADER+FISH_ASSET_PAYLOAD_BYTES) {fail(FISH_RES_ERR_SIZE);return;}
    if(fish_resource_read(0,header,sizeof(header))) {fail(FISH_RES_ERR_IO);return;}
    fish_res_error_t cause=header_error(header);
    if(cause!=FISH_RES_ERR_NONE) {fail(cause);return;}
    at=0;crc=~0u;expected=word(header+20);phase=FISH_RES_SOURCE_CHECK;
}
void fish_resources_begin(unsigned map)
{
    close_source();target=map;progress=0;at=0;crc=~0u;check_only=false;source_error=false;error=FISH_RES_ERR_NONE;
    if(map>=2) {fail(FISH_RES_ERR_PACK);return;}
    const uint8_t *cache=fish_resource_flash();
    if(header_error(cache)==FISH_RES_ERR_NONE) {expected=word(cache+20);phase=FISH_RES_CACHE_CHECK;}
    else source_begin();
}
void fish_resources_check_begin(unsigned map)
{
    close_source();target=map;progress=0;at=0;crc=~0u;check_only=true;source_error=true;error=FISH_RES_ERR_NONE;
    if(map>=2) {fail(FISH_RES_ERR_PACK);return;}
    source_begin();
}
void fish_resources_cancel(void) {close_source();phase=FISH_RES_IDLE;}
bool fish_resources_source_error(void) {return phase==FISH_RES_ERROR && source_error;}
void fish_resources_tick(void)
{
    unsigned count=FISH_ASSET_PAYLOAD_BYTES-at;
    if(count>sizeof(block))count=sizeof(block);
    const uint8_t *cache=fish_resource_flash();
    switch(phase) {
    case FISH_RES_CACHE_CHECK:
        /* Cached XIP CRC needs no SD transaction or staging buffer. Check up
         * to 16 KiB per frame so a warm map is not paced as 114 SD reads. */
        count=FISH_ASSET_PAYLOAD_BYTES-at;
        if(count>4*sizeof(block))count=4*sizeof(block);
        crc=fish_resource_crc(crc,cache+FISH_CACHE_SECTOR+at,count);at+=count;
        progress=at*100/FISH_ASSET_PAYLOAD_BYTES;
        if(at==FISH_ASSET_PAYLOAD_BYTES) {
            if((crc^~0u)==expected) {valid=true;active=target;phase=FISH_RES_DONE;progress=100;}
            else {valid=false;active=2;source_begin();progress=0;}
        }break;
    case FISH_RES_SOURCE_CHECK:
        if(fish_resource_read(FISH_PACK_HEADER+at,block,count)) {fail(FISH_RES_ERR_IO);break;}
        crc=fish_resource_crc(crc,block,count);at+=count;progress=at*(check_only?100:20)/FISH_ASSET_PAYLOAD_BYTES;
        if(at==FISH_ASSET_PAYLOAD_BYTES) {
            if((crc^~0u)!=expected) {fail(FISH_RES_ERR_CRC);break;}
            if(check_only) {close_source();phase=FISH_RES_DONE;progress=100;break;}
            source_error=false;at=0;phase=FISH_RES_CONFIGURE;
        }break;
    case FISH_RES_CONFIGURE:
        /* A valid XIP cache needs reads only. Configure the NOR writer lazily
         * after the entire replacement source has passed preflight. */
        if(fish_resource_port_init()) {fail(FISH_RES_ERR_FLASH);break;}
        phase=FISH_RES_ERASE_HEADER;break;
    case FISH_RES_ERASE_HEADER:
        /* Commit sector is invalidated first; interruption can never expose a
         * mixture of maps as a valid resource set on the next boot. */
        valid=false;active=2;
        if(fish_resource_erase(0)) {fail(FISH_RES_ERR_FLASH);break;}
        phase=FISH_RES_ERASE_BLOCK;break;
    case FISH_RES_ERASE_BLOCK:
        if(fish_resource_read(FISH_PACK_HEADER+at,block,count)) {source_error=true;fail(FISH_RES_ERR_IO);break;}
        if(fish_resource_erase(FISH_CACHE_SECTOR+at)) {fail(FISH_RES_ERR_FLASH);break;}
        phase=FISH_RES_WRITE_BLOCK;break;
    case FISH_RES_WRITE_BLOCK:
        if(fish_resource_program(FISH_CACHE_SECTOR+at,block,count) ||
           memcmp(block,cache+FISH_CACHE_SECTOR+at,count)) {fail(FISH_RES_ERR_FLASH);break;}
        at+=count;progress=20+at*70/FISH_ASSET_PAYLOAD_BYTES;
        if(at==FISH_ASSET_PAYLOAD_BYTES) {at=0;crc=~0u;phase=FISH_RES_VERIFY;}
        else phase=FISH_RES_ERASE_BLOCK;break;
    case FISH_RES_VERIFY:
        crc=fish_resource_crc(crc,cache+FISH_CACHE_SECTOR+at,count);at+=count;
        progress=90+at*9/FISH_ASSET_PAYLOAD_BYTES;
        if(at==FISH_ASSET_PAYLOAD_BYTES) {
            if((crc^~0u)!=expected) {fail(FISH_RES_ERR_FLASH);break;}
            phase=FISH_RES_COMMIT;
        }break;
    case FISH_RES_COMMIT:
        if(fish_resource_program(0,header,sizeof(header)) || memcmp(cache,header,sizeof(header))) {fail(FISH_RES_ERR_FLASH);break;}
        valid=true;active=target;progress=100;phase=FISH_RES_DONE;close_source();break;
    default:break;
    }
}
bool fish_resources_valid(void) {return valid;}
unsigned fish_resources_map(void) {return active;}
unsigned fish_resources_progress(void) {return progress;}
fish_res_phase_t fish_resources_phase(void) {return phase;}
fish_res_error_t fish_resources_error(void) {return error;}
