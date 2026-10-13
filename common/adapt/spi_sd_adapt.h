/*
 * Copyright (c) 2023 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef SPI_SD_ADAPT_H
#define SPI_SD_ADAPT_H

#include "hpm_common.h"
#include "hpm_spi_sdcard.h"

#ifndef USE_DMA_TRANSFER
#define USE_DMA_TRANSFER         (0U)
#endif

hpm_stat_t spi_sd_init(void);
void display_sdcard_info(void);
hpm_stat_t __spi_read_multi_block(uint8_t *buffer, uint32_t start_sector, uint32_t num_sectors);
hpm_stat_t __spi_write_multi_block(uint8_t *buffer, uint32_t sector, uint32_t num_sectors);
#endif
