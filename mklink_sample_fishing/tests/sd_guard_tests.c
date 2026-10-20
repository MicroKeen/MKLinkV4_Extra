#include "fish_sd_guard.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
static uint8_t sector0[512],sector2048[512];
static int read_error;
int fish_sd_guard_read(uint32_t sector,void *out)
{
    if(read_error || (sector!=0 && sector!=2048))return -1;
    memcpy(out,sector?sector2048:sector0,512);return 0;
}
static void put16(uint8_t *p,unsigned n) {p[0]=(uint8_t)n;p[1]=(uint8_t)(n>>8);}
static void put32(uint8_t *p,uint32_t n) {put16(p,n);put16(p+2,n>>16);}
static void fat(uint8_t *p,uint32_t total,bool fat32)
{
    memset(p,0,512);p[0]=0xeb;p[510]=0x55;p[511]=0xaa;
    put16(p+11,512);p[13]=4;put16(p+14,fat32?32:1);p[16]=2;
    put16(p+17,fat32?0:512);put32(p+32,total);
    if(fat32)put32(p+36,512);else put16(p+22,240);
}
int main(void)
{
    uint32_t n=0;fat(sector0,245760,false);
    assert(!fish_sd_guard_capacity(262144,&n) && n==245760);
    fat(sector0,262144,false);assert(!fish_sd_guard_capacity(262144,&n) && n==262144);
    fat(sector0,65536,false);assert(!fish_sd_guard_capacity(65536,&n) && n==65536);
    fat(sector0,262144,true);assert(!fish_sd_guard_capacity(262144,&n) && n==262144);
    fat(sector0,262145,false);assert(fish_sd_guard_capacity(262144,&n));
    memset(sector0,0,512);sector0[510]=0x55;sector0[511]=0xaa;
    sector0[450]=0x06;put32(sector0+454,2048);put32(sector0+458,243712);
    fat(sector2048,243712,false);
    assert(!fish_sd_guard_capacity(262144,&n) && n==245760);
    put32(sector0+458,260096);fat(sector2048,260096,true);
    assert(!fish_sd_guard_capacity(262144,&n) && n==262144);
    put32(sector0+458,260097);assert(fish_sd_guard_capacity(262144,&n));
    put32(sector0+454,UINT32_MAX);assert(fish_sd_guard_capacity(262144,&n));
    put32(sector0+454,2048);put32(sector0+458,243712);
    fat(sector2048,243713,false);assert(fish_sd_guard_capacity(262144,&n));
    sector0[450]=0xee;assert(fish_sd_guard_capacity(262144,&n));
    memset(sector0,0,512);sector0[510]=0x55;sector0[511]=0xaa;
    memcpy(sector0+3,"EXFAT   ",8);put32(sector0+72,245760);
    put32(sector0+80,24);put32(sector0+84,512);put32(sector0+88,1048);
    put32(sector0+92,30500);put32(sector0+96,2);
    sector0[108]=9;sector0[109]=3;sector0[110]=1;
    assert(!fish_sd_guard_capacity(262144,&n) && n==245760);
    sector0[108]=12;assert(fish_sd_guard_capacity(262144,&n));
    sector0[108]=9;put32(sector0+76,1);assert(fish_sd_guard_capacity(262144,&n));
    memset(sector0,0,512);assert(fish_sd_guard_capacity(262144,&n));
    fat(sector0,245760,false);read_error=1;assert(fish_sd_guard_capacity(262144,&n));
    assert(fish_sd_guard_capacity(0,&n) && fish_sd_guard_capacity(262144,NULL));
    puts("PASS: stock 120 MiB and full-card FAT layouts; MBR/FAT32/exFAT; malformed/oversize/overflow/unreadable media rejected without writes");
}
