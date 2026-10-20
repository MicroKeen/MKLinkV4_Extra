#ifndef TEST_SPI_CARD_H
#define TEST_SPI_CARD_H
#include <stdbool.h>
#include <stdint.h>
typedef int hpm_stat_t;
enum {status_success=0,status_fail=1,card_type_sd_v2_hc=3};
typedef struct {
    uint64_t capacity;
    uint32_t block_size,block_count;
    uint8_t card_type;
    struct {uint32_t device_size;} csd;
} spi_sdcard_info_t;
#define HPM_BREAK_IF(c) if(c) break
hpm_stat_t sdcard_spi_get_card_info(spi_sdcard_info_t *info);
hpm_stat_t sdcard_spi_status(void);
hpm_stat_t sdcard_spi_read_multi_block(uint8_t *buffer,uint32_t sector,uint32_t count);
hpm_stat_t sdcard_spi_write_multi_block(uint8_t *buffer,uint32_t sector,uint32_t count);
hpm_stat_t sdcard_spi_write_block(uint32_t sector,uint8_t *buffer);
#endif
