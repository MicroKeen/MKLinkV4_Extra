#include "fish_resources.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
uint8_t fish_host_resource_cache[FISH_CACHE_BYTES] __attribute__((aligned(32)));
static FILE *source;
static bool initialized;
int fish_resource_port_init(void) {if(!initialized) {memset(fish_host_resource_cache,255,sizeof(fish_host_resource_cache));initialized=true;}return 0;}
int fish_resource_open(unsigned map,uint32_t *size)
{
    const char *root=getenv("FISH_RESOURCE_DIR");if(!root)root=FISH_PC_RESOURCE_DIR;
    char path[1024];snprintf(path,sizeof(path),"%s/%s.fgpk",root,map==3?"common/fish":map==2?"maps/2_beach":map?"maps/1_city_river":"maps/0_wetland");
    source=fopen(path,"rb");if(!source)return errno==ENOENT?-2:-1;
    if(fseek(source,0,SEEK_END)) {fclose(source);source=NULL;return -1;}
    long n=ftell(source);if(n<0 || n>0x100000) {fclose(source);source=NULL;return -1;}
    *size=(uint32_t)n;return 0;
}
int fish_resource_read(uint32_t offset,void *out,unsigned bytes) {return !source || fseek(source,offset,SEEK_SET) || fread(out,1,bytes,source)!=bytes?-1:0;}
void fish_resource_close(void) {if(source)fclose(source);source=NULL;}
const uint8_t *fish_resource_flash(void) {return fish_host_resource_cache;}
int fish_resource_erase(uint32_t offset) {if(offset%4096 || offset>FISH_CACHE_BYTES-4096)return -1;memset(fish_host_resource_cache+offset,255,4096);return 0;}
int fish_resource_program(uint32_t offset,const void *data,unsigned bytes) {if(offset>FISH_CACHE_BYTES || bytes>FISH_CACHE_BYTES-offset)return -1;memcpy(fish_host_resource_cache+offset,data,bytes);return 0;}
