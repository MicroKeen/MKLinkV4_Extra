/*
 * Copyright (c) 2021-2025 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "ffconf.h"
#include "hpm_spi_sd_disk.h"

static spi_sdcard_info_t sd_info;
static bool initialized;
extern hpm_stat_t spi_sd_write_sector(uint32_t sector,const uint8_t *buffer,bool high_capacity);

DSTATUS spi_sd_disk_initialize(BYTE pdrv)
{
    /* the spi hardware has been initialized before the sd disk init*/

    hpm_stat_t sta;
    if (pdrv != DEV_SD) {
        return STA_NOINIT;
    }
    initialized = false;
    sta = sdcard_spi_get_card_info(&sd_info);
    /* The SDK normalizes both SDSC and SDHC to 512-byte logical sectors.
     * CSD device_size is raw C_SIZE, not a sector count. Reject inconsistent
     * geometry before any read/write; never attempt to format or repair it. */
    if (sta != status_success || sd_info.block_size != 512U ||
        sd_info.block_count == 0U ||
        sd_info.capacity != (uint64_t)sd_info.block_count * 512U) {
        return STA_NOINIT;
    }
    initialized = true;
    return 0;
}

DSTATUS spi_sd_disk_deinitialize(BYTE pdrv)
{
    if (pdrv != DEV_SD) {
        return STA_NOINIT;
    }
    initialized = false;
    return RES_OK;
}

DSTATUS spi_sd_disk_status(BYTE pdrv)
{
    if (pdrv != DEV_SD) {
        return STA_NOINIT;
    }
    return (initialized && sdcard_spi_status() == status_success) ? 0 : STA_NOINIT;
}

static bool range_ok(LBA_t sector, UINT count)
{
    return count && sector < sd_info.block_count &&
           count <= sd_info.block_count - sector;
}

DSTATUS spi_sd_disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
    if (pdrv != DEV_SD) {
        return RES_PARERR;
    }

    if (!initialized) return RES_NOTRDY;
    if (buff == NULL || !range_ok(sector, count)) return RES_PARERR;

    if (sdcard_spi_read_multi_block((uint8_t *) buff, (uint32_t) sector, (uint32_t) count) != status_success) {
        return RES_ERROR;
    }

    return RES_OK;
}

DSTATUS spi_sd_disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
    if (pdrv != DEV_SD) {
        return RES_PARERR;
    }

    if (!initialized) return RES_NOTRDY;
    if (buff == NULL || !range_ok(sector, count)) return RES_PARERR;

    /* Match stock MKLink's single-sector metadata path. FatFs often writes
     * exactly one directory/FAT sector while creating a save; CMD25 is not
     * needed here. Stop on the first failure, without replaying accepted data. */
    for(UINT i=0;i<count;i++)
        if(spi_sd_write_sector((uint32_t)sector+i,buff+i*512U,sd_info.card_type==card_type_sd_v2_hc)!=status_success)
            return RES_ERROR;

    return RES_OK;
}

DRESULT spi_sd_disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    DRESULT result = RES_PARERR;
    do {
        HPM_BREAK_IF((pdrv != DEV_SD) || ((cmd != CTRL_SYNC) && (buff == NULL)));
        if (!initialized) return RES_NOTRDY;
        result = RES_OK;
        switch (cmd) {
        case GET_SECTOR_COUNT:
            *(LBA_t *) buff = sd_info.block_count;
            break;
        case GET_SECTOR_SIZE:
            *(WORD *) buff = sd_info.block_size;
            break;
        case GET_BLOCK_SIZE:
            /* Conservative erase alignment, in sectors rather than bytes.
             * Formatting and TRIM remain disabled in this application. */
            *(DWORD *) buff = 1U;
            break;
        case CTRL_SYNC:
            result = RES_OK;
            break;
        default:
            result = RES_PARERR;
            break;
        }

    } while (false);

    return result;
}
