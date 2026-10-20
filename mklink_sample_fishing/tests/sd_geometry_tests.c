/* Compile the actual disk adapter; no real card or filesystem is touched. */
#include "hpm_spi_sd_disk.h"
#include <assert.h>
#include <stdio.h>
static spi_sdcard_info_t card;
static unsigned reads,writes;
static uint32_t last_sector,last_count;
static int card_error;
static uint32_t fail_sector=UINT32_MAX;
hpm_stat_t sdcard_spi_get_card_info(spi_sdcard_info_t *info)
{*info=card;return card_error?status_fail:status_success;}
hpm_stat_t sdcard_spi_status(void) {return card_error?status_fail:status_success;}
hpm_stat_t sdcard_spi_read_multi_block(uint8_t *b,uint32_t s,uint32_t n)
{assert(b);++reads;last_sector=s;last_count=n;return status_success;}
hpm_stat_t sdcard_spi_write_multi_block(uint8_t *b,uint32_t s,uint32_t n)
{(void)b;(void)s;(void)n;assert(!"FatFs writes must use single-sector CMD24");return status_fail;}
hpm_stat_t sdcard_spi_write_block(uint32_t s,uint8_t *b)
{assert(b);++writes;last_sector=s;last_count=1;return s==fail_sector?status_fail:status_success;}
hpm_stat_t spi_sd_write_sector(uint32_t s,const uint8_t *b,bool high_capacity)
{(void)high_capacity;return sdcard_spi_write_block(s,(uint8_t *)b);}
static void geometry(uint32_t c_size,uint32_t sectors)
{
    card=(spi_sdcard_info_t){.capacity=(uint64_t)sectors*512,.block_size=512,
        .block_count=sectors,.csd={.device_size=c_size}};
    unsigned before_writes=writes;
    assert(!spi_sd_disk_initialize(DEV_SD));
    LBA_t count=0;WORD size=0;DWORD erase=0;
    assert(spi_sd_disk_ioctl(DEV_SD,GET_SECTOR_COUNT,&count)==RES_OK && count==sectors);
    assert(spi_sd_disk_ioctl(DEV_SD,GET_SECTOR_SIZE,&size)==RES_OK && size==512);
    assert(spi_sd_disk_ioctl(DEV_SD,GET_BLOCK_SIZE,&erase)==RES_OK && erase==1);
    assert(spi_sd_disk_ioctl(DEV_SD,CTRL_FORMAT,&erase)==RES_PARERR);
    assert(spi_sd_disk_ioctl(DEV_SD,CTRL_TRIM,&erase)==RES_PARERR);
    assert(writes==before_writes); /* Geometry queries never change the card. */
    uint8_t buffer[1024]={0};
    assert(spi_sd_disk_read(DEV_SD,buffer,sectors-1,1)==RES_OK);
    assert(last_sector==sectors-1 && last_count==1);
    assert(spi_sd_disk_write(DEV_SD,buffer,sectors-2,2)==RES_OK);
    assert(last_sector==sectors-1 && last_count==1);
    unsigned attempted=writes;fail_sector=1;
    assert(spi_sd_disk_write(DEV_SD,buffer,0,2)==RES_ERROR && writes==attempted+2);
    assert(last_sector==1);fail_sector=UINT32_MAX;
    unsigned before_reads=reads;before_writes=writes;
    const LBA_t invalid[]={sectors-1,sectors,UINT32_MAX,UINT64_C(1)<<32,UINT64_MAX};
    for(unsigned i=0;i<sizeof(invalid)/sizeof(*invalid);++i) {
        assert(spi_sd_disk_read(DEV_SD,buffer,invalid[i],2)==RES_PARERR);
        assert(spi_sd_disk_write(DEV_SD,buffer,invalid[i],2)==RES_PARERR);
    }
    assert(spi_sd_disk_write(DEV_SD,buffer,0,0)==RES_PARERR);
    assert(spi_sd_disk_write(DEV_SD,NULL,0,1)==RES_PARERR);
    assert(spi_sd_disk_read(DEV_SD,buffer,0,UINT32_MAX)==RES_PARERR);
    assert(spi_sd_disk_write(DEV_SD+1,buffer,0,1)==RES_PARERR);
    assert(reads==before_reads && writes==before_writes);
}
int main(void)
{
    uint8_t buffer[512]={0};LBA_t count=0;
    assert(spi_sd_disk_write(DEV_SD,buffer,0,1)==RES_NOTRDY);
    /* 128 MiB SDSC: (C_SIZE+1)*2^(C_SIZE_MULT+2), multiplier 8. */
    geometry(255,262144);
    /* 4 GiB SDHC: (C_SIZE+1)*1024 sectors. */
    geometry(8191,8388608);
    unsigned before=reads+writes;
    card.capacity--;
    assert(spi_sd_disk_initialize(DEV_SD)==STA_NOINIT);
    assert(spi_sd_disk_ioctl(DEV_SD,GET_SECTOR_COUNT,&count)==RES_NOTRDY);
    assert(spi_sd_disk_write(DEV_SD,buffer,0,1)==RES_NOTRDY);
    card.capacity++;card.block_size=1024;
    assert(spi_sd_disk_initialize(DEV_SD)==STA_NOINIT);
    card.block_size=512;card_error=1;
    assert(spi_sd_disk_initialize(DEV_SD)==STA_NOINIT);
    assert(spi_sd_disk_status(DEV_SD)==STA_NOINIT);
    assert(spi_sd_disk_read(DEV_SD,buffer,0,1)==RES_NOTRDY);
    card_error=0;assert(!spi_sd_disk_initialize(DEV_SD));
    assert(!spi_sd_disk_deinitialize(DEV_SD));
    assert(spi_sd_disk_write(DEV_SD,buffer,0,1)==RES_NOTRDY);
    assert(reads+writes==before);
    puts("PASS: SDSC/SDHC real geometry; final-sector access; overflow and invalid geometry blocked before I/O; no format/TRIM");
}
