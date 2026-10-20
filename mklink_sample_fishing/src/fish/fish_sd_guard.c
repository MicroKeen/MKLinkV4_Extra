#include "fish_sd_guard.h"
#include <stdbool.h>
#include <string.h>
/* Plus downloader branches cap MSC at 120 MiB. Preserve that view when the
 * existing layout fits it; V4 full-card layouts keep their full capacity. */
enum {STOCK_PLUS_SECTORS=245760};
static uint8_t boot[512];
static uint16_t u16(const uint8_t *p) {return (uint16_t)(p[0]|(uint16_t)p[1]<<8);}
static uint32_t u32(const uint8_t *p) {return (uint32_t)u16(p)|(uint32_t)u16(p+2)<<16;}
static bool signature(void) {return boot[510]==0x55 && boot[511]==0xaa;}
static bool power2(unsigned n) {return n && !(n&(n-1));}
static bool volume(uint32_t available,uint32_t *length)
{
    if(!signature())return false;
    if(!memcmp(boot+3,"EXFAT   ",8)) {
        if(u32(boot+76) || boot[108]!=9 || boot[109]>16 ||
           (boot[110]!=1 && boot[110]!=2))return false;
        uint32_t total=u32(boot+72),fat=u32(boot+80),size=u32(boot+84);
        uint32_t heap=u32(boot+88),clusters=u32(boot+92),root=u32(boot+96);
        if(!total || total>available || !fat || !size || fat>=heap || heap>=total ||
           size>(heap-fat)/boot[110] || !clusters || clusters>0xfffffffdU ||
           clusters>((total-heap)>>boot[109]) || root<2 || root-2>=clusters)return false;
        *length=total;return true;
    }
    if((boot[0]!=0xeb && boot[0]!=0xe9) || u16(boot+11)!=512 ||
       !power2(boot[13]) || boot[13]>128 || !u16(boot+14) ||
       (boot[16]!=1 && boot[16]!=2))return false;
    uint32_t total=u16(boot+19);if(!total)total=u32(boot+32);
    uint32_t fat=u16(boot+22);if(!fat)fat=u32(boot+36);
    uint32_t root=((uint32_t)u16(boot+17)*32+511)/512;
    uint32_t reserved=u16(boot+14);
    if(!total || total>available || !fat || reserved>=total || root>=total-reserved ||
       fat>=(total-reserved-root+boot[16]-1)/boot[16])return false;
    *length=total;return true;
}
int fish_sd_guard_capacity(uint32_t physical,uint32_t *exported)
{
    if(!physical || !exported || fish_sd_guard_read(0,boot) || !signature())return -1;
    uint32_t length=0,end=0;
    if(volume(physical,&length))end=length;
    else {
        /* Save the small partition table before reading each volume's BPB. */
        uint8_t partitions[64];memcpy(partitions,boot+446,sizeof(partitions));
        bool found=false;
        for(unsigned i=0;i<4;++i) {
            const uint8_t *p=partitions+i*16;
            uint32_t start=u32(p+8),count=u32(p+12);
            if(!p[4] && !start && !count)continue;
            if(!p[4] || p[4]==0xee || !start || !count || start>=physical || count>physical-start)return -1;
            if(start+count>end)end=start+count;
            if(fish_sd_guard_read(start,boot))return -1;
            if(volume(count,&length))found=true;
        }
        if(!found)return -1;
    }
    *exported=physical>STOCK_PLUS_SECTORS && end<=STOCK_PLUS_SECTORS?STOCK_PLUS_SECTORS:physical;
    return 0;
}
