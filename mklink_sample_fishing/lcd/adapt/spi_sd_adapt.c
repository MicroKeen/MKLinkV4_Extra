/*
 * Copyright (c) 2023 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "spi_sd_adapt.h"
#include "hpm_spi_drv.h"
#include "hpm_clock_drv.h"
#include "board.h"
#include "hpm_dma_mgr.h"
#include "hpm_spi.h"

#define SD_SPI_BASE                     BOARD_APP_SPI_BASE
#define SD_SPI_CLK_NAME                 BOARD_APP_SPI_CLK_NAME
#define SD_SPI_DATA_LEN_BITS            (8U)
#if defined(FISH_SD_CPU_IO) && FISH_SD_CPU_IO && defined(USE_DMA_TRANSFER) && USE_DMA_TRANSFER
#error FISH_SD_CPU_IO requires CPU-blocking SPI transfers
#endif

static hpm_stat_t set_spi_speed(uint32_t freq);
static void cs_select(void);
static void cs_relese(void);
static bool sdcard_is_present(void);
static hpm_stat_t write_read_byte(uint8_t *in_byte, uint8_t *out_byte);
static hpm_stat_t write_cmd_data(uint8_t cmd, uint8_t *buffer, uint32_t size);
static hpm_stat_t write(uint8_t *buffer, uint32_t size);
static hpm_stat_t read(uint8_t *buffer, uint32_t size);

static sdcard_spi_interface_t g_spi_io;
/* Bound the SDK's 0xffffffff response/token polling through its callbacks.
 * Feed only at transaction boundaries, never inside an unresponsive poll. */
extern int64_t get_system_time_ms(void);
extern void fish_hpm_storage_heartbeat(void);
enum {SD_TRANSACTION_MS=2000,SD_SPI_CALL_MS=100};
static uint32_t transaction_start;
static bool transaction_active;
static uint32_t running_hz;
static bool write_clock;
static unsigned write_diagnostic;
unsigned spi_sd_write_diagnostic(void) {return write_diagnostic;}
static bool expired(void)
{return transaction_active && (uint32_t)((uint32_t)get_system_time_ms()-transaction_start)>=SD_TRANSACTION_MS;}
ATTR_PLACE_AT_NONCACHEABLE uint8_t tx_dummy[SPI_SD_BLOCK_SIZE] = {0xFF};

#if defined(USE_DMA_TRANSFER) && (USE_DMA_TRANSFER == 1)
static volatile bool rxdma_complete;
static volatile bool txdma_complete;

void spi_rxdma_complete_callback(uint32_t channel)
{
    (void)channel;
    rxdma_complete = true;
}

void spi_txdma_complete_callback(uint32_t channel)
{
    (void)channel;
    txdma_complete = true;
}
#endif

hpm_stat_t spi_sd_init(void)
{
    uint32_t spi_sclk_freq = 400000;
    spi_initialize_config_t init_config;
    board_init_spi_clock(SD_SPI_BASE);
    board_init_spi_pins_with_gpio_as_cs(SD_SPI_BASE);

    hpm_spi_get_default_init_config(&init_config);
    init_config.direction = spi_msb_first;
    init_config.mode = spi_master_mode;
    init_config.clk_phase = spi_sclk_sampling_even_clk_edges;
    init_config.clk_polarity = spi_sclk_high_idle;
    init_config.data_len = SD_SPI_DATA_LEN_BITS;
    /* step.1  initialize spi */
    if (hpm_spi_initialize(SD_SPI_BASE, &init_config) != status_success) {
        printf("hpm_spi_initialize fail\n");
        return status_fail;
    }
    if (hpm_spi_set_sclk_frequency(SD_SPI_BASE, spi_sclk_freq) != status_success) {
        printf("hpm_spi_set_sclk_frequency fail\n");
        return status_fail;
    }

#if defined(USE_DMA_TRANSFER) && (USE_DMA_TRANSFER == 1)
    if (hpm_spi_dma_mgr_install_callback(SD_SPI_BASE, spi_txdma_complete_callback, spi_rxdma_complete_callback) != status_success) {
        printf("hpm_spi_dma_mgr_install_callback fail\n");
        return status_fail;
    }
    rxdma_complete = false;
    txdma_complete = false;
#endif
    memset(tx_dummy, 0xFF, sizeof(tx_dummy));
    g_spi_io.set_spi_speed     = set_spi_speed;
    g_spi_io.cs_select         = cs_select;
    g_spi_io.cs_relese         = cs_relese;
    g_spi_io.sdcard_is_present = sdcard_is_present;
    g_spi_io.write_read_byte   = write_read_byte;
    g_spi_io.write_cmd_data    = write_cmd_data;
    g_spi_io.write             = write;
    g_spi_io.read              = read;
    g_spi_io.delay_ms          = board_delay_ms;
    g_spi_io.delay_us          = board_delay_us;
    return sdcard_spi_init(&g_spi_io);
}

static hpm_stat_t sector_failed(unsigned stage,uint8_t response)
{
    write_diagnostic=(stage<<8)|response;cs_relese();return status_fail;
}
static bool sector_ready(uint8_t *response)
{
    uint8_t dummy=0xff;
    for(unsigned i=0;i<500000;i++) {
        if(write_read_byte(&dummy,response)!=status_success)return false;
        if(*response==0xff)return true;
    }
    return false;
}
hpm_stat_t spi_sd_write_sector(uint32_t sector,const uint8_t *buffer,bool high_capacity)
{
    /* A single accepted sector is never replayed. CRC16 and a bounded token
     * wait cover cards that reject dummy CRCs or delay the data response.
     * All failures retain their wire stage for the cached Wxx/yy diagnostic. */
    if(!buffer || (!high_capacity && sector>0x7fffffu))return status_fail;
    uint16_t crc=0;
    for(unsigned i=0;i<512;i++) {
        crc^=(uint16_t)buffer[i]<<8;
        for(unsigned bit=0;bit<8;bit++)crc=(uint16_t)((crc<<1)^((crc&0x8000)?0x1021:0));
    }
    uint32_t address=high_capacity?sector:sector*512u;
    uint8_t command[5]={(uint8_t)(address>>24),(uint8_t)(address>>16),(uint8_t)(address>>8),(uint8_t)address,0xff};
    uint8_t data_crc[2]={(uint8_t)(crc>>8),(uint8_t)crc},response=0xff,dummy=0xff,token=0xfe;
    write_diagnostic=0;cs_select();
    if(!sector_ready(&response))return sector_failed(1,response);
    if(write_cmd_data(0x58,command,5)!=status_success)return sector_failed(2,response);
    unsigned i;
    for(i=0;i<64;i++) {
        if(write_read_byte(&dummy,&response)!=status_success)return sector_failed(3,response);
        if(!(response&0x80))break;
    }
    if(i==64 || response)return sector_failed(4,response);
    if(write_read_byte(&token,&response)!=status_success)return sector_failed(5,response);
    if(write((uint8_t *)buffer,512)!=status_success)return sector_failed(6,response);
    if(write(data_crc,2)!=status_success)return sector_failed(7,response);
    for(i=0;i<64;i++) {
        if(write_read_byte(&dummy,&response)!=status_success)return sector_failed(8,response);
        if(response!=0xff)break;
    }
    if(i==64 || (response&0x1f)!=5)return sector_failed(9,response);
    if(!sector_ready(&response))return sector_failed(10,response);
    cs_relese();return status_success;
}
static hpm_stat_t set_spi_speed(uint32_t freq)
{
    /* set SPI sclk frequency for master */
    if (hpm_spi_set_sclk_frequency(SD_SPI_BASE, freq) != status_success) {
        printf("hpm_spi_set_sclk_frequency fail\n");
        return status_fail;
    }

    running_hz=freq;return status_success;
}

static void cs_select(void)
{
    transaction_start=(uint32_t)get_system_time_ms();transaction_active=true;
    fish_hpm_storage_heartbeat();
    /* Match MKLink's original SD adapter: no software GPIO chip select.
     * Generic BOARD_SPI_CS_PIN is PA26, used by ST7789_LED_SET as backlight. */
}

static void cs_relese(void)
{
    if(write_clock) {
        /* All SDK write exits release the transaction, including token errors.
         * Restore resource-read speed; slow writes must not slow map loading. */
        hpm_spi_set_sclk_frequency(SD_SPI_BASE,running_hz);write_clock=false;
    }
    transaction_active=false;fish_hpm_storage_heartbeat();
    /* Never toggle PA26 here; the LCD owns its backlight level. */
}

static bool sdcard_is_present(void)
{
    /* boards is not detect pins, so it always true */

    return true;
}

static hpm_stat_t write_read_byte(uint8_t *in_byte, uint8_t *out_byte)
{
    if(expired())return status_timeout;
    hpm_stat_t stat;
    stat = hpm_spi_transmit_receive_blocking(SD_SPI_BASE, in_byte, out_byte, 1, SD_SPI_CALL_MS);
    return stat;
}

static hpm_stat_t write_cmd_data(uint8_t cmd, uint8_t *buffer, uint32_t size)
{
    if(expired())return status_timeout;
    if(((cmd&63U)==24U || (cmd&63U)==25U) && running_hz>4000000U) {
        if(hpm_spi_set_sclk_frequency(SD_SPI_BASE,4000000U)!=status_success)return status_fail;
        write_clock=true;
    }
    hpm_stat_t stat;
    /* command phase is 48bit,*/
    uint8_t buf[6];
    buf[0] = cmd;
    memcpy(&buf[1], buffer, size);
    stat = hpm_spi_transmit_blocking(SD_SPI_BASE, buf, size + 1, SD_SPI_CALL_MS);
    return stat;
}

static hpm_stat_t write(uint8_t *buffer, uint32_t size)
{
    if(expired())return status_timeout;
    hpm_stat_t stat = status_success;
#if defined(USE_DMA_TRANSFER) && (USE_DMA_TRANSFER == 1)
    stat = hpm_spi_transmit_nonblocking(SD_SPI_BASE, buffer, size);
    if (stat != status_success) {
        printf("hpm_spi_transmit_nonblocking fail\n");
        return stat;
    }
    while (txdma_complete == false) {
    };
    txdma_complete = false;
    /* When SPI sends, DMA completion does not mean the transfer is complete. need to wait for the SPI status to complete.*/
    while (spi_is_active(SD_SPI_BASE) == true) {
    };
#else
    stat = hpm_spi_transmit_blocking(SD_SPI_BASE, buffer, size, SD_SPI_CALL_MS);
#endif
    return stat;
}

static hpm_stat_t read(uint8_t *buffer, uint32_t size)
{
    if(expired())return status_timeout;
    hpm_stat_t stat = status_success;
#if defined(USE_DMA_TRANSFER) && (USE_DMA_TRANSFER == 1)
    stat = hpm_spi_transmit_receive_nonblocking(SD_SPI_BASE, tx_dummy, buffer, size);
    if (stat != status_success) {
        printf("hpm_spi_transmit_receive_nonblocking fail\n");
        return status_fail;
    }
    while (rxdma_complete == false) {
    };
    while (txdma_complete == false) {
    };
    rxdma_complete = false;
    txdma_complete = false;
#else
    stat = hpm_spi_transmit_receive_blocking(SD_SPI_BASE, tx_dummy, buffer, size, SD_SPI_CALL_MS);
#endif
    return stat;
}
