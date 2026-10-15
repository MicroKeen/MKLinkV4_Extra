/* Compile the actual board SD adapter; all hardware calls are observed here. */
#include "sd_test_hal.h"
#include "spi_sd_adapt.h"
#include <assert.h>
SPI_Type sd_test_spi[2];
sd_test_ioc_t sd_test_ioc;
static sdcard_spi_interface_t *card;
static unsigned gpio_writes,pin_reroutes,transfers;
static bool backlight=true;
static void sd_only(SPI_Type *p) {assert(p==BOARD_APP_SPI_BASE);}
uint32_t board_init_spi_clock(SPI_Type *p) {sd_only(p);return 24000000;}
void board_init_spi_pins_with_gpio_as_cs(SPI_Type *p) {sd_only(p);}
void board_write_spi_cs(uint32_t pin,uint8_t state)
{
    assert(pin==26);++gpio_writes;backlight=state!=0;
}
void gpiom_set_pin_controller(unsigned base,unsigned port,unsigned pin,unsigned owner)
{
    (void)base;(void)port;(void)pin;(void)owner;++pin_reroutes;
}
void gpio_set_pin_output_with_initial(unsigned base,unsigned port,unsigned pin,bool high)
{
    (void)base;(void)port;board_write_spi_cs(pin,high);
}
void board_delay_ms(uint32_t ms) {(void)ms;}
void board_delay_us(uint32_t us) {(void)us;}
void hpm_spi_get_default_init_config(spi_initialize_config_t *c) {memset(c,0,sizeof(*c));}
hpm_stat_t hpm_spi_initialize(SPI_Type *p,spi_initialize_config_t *c)
{
    sd_only(p);assert(c->data_len==8);return status_success;
}
hpm_stat_t hpm_spi_set_sclk_frequency(SPI_Type *p,uint32_t hz)
{
    sd_only(p);assert(hz);return status_success;
}
hpm_stat_t hpm_spi_transmit_blocking(SPI_Type *p,uint8_t *b,uint32_t n,uint32_t timeout)
{
    sd_only(p);assert(b && n && timeout);++transfers;return status_success;
}
hpm_stat_t hpm_spi_transmit_receive_blocking(SPI_Type *p,uint8_t *tx,uint8_t *rx,uint32_t n,uint32_t timeout)
{
    sd_only(p);assert(tx && rx && n && timeout);memset(rx,0xff,n);++transfers;return status_success;
}
hpm_stat_t sdcard_spi_init(sdcard_spi_interface_t *io) {card=io;return status_success;}
int main(void)
{
    /* PA26 is a backlight pin, even though generic board.h calls it SPI CS. */
    assert(spi_sd_init()==status_success && card);
    assert(backlight);
    uint8_t block[512]={0},command[5]={0},tx=0xff,rx;
    for(unsigned i=0;i<128;i++) {
        card->cs_select();assert(backlight);
        assert(card->write_cmd_data(24,command,5)==status_success);
        assert(card->write(block,sizeof(block))==status_success);
        assert(card->write_read_byte(&tx,&rx)==status_success);
        assert(card->read(block,sizeof(block))==status_success);
        card->cs_relese();assert(backlight);
    }
    assert(transfers==512 && !gpio_writes && !pin_reroutes);
    puts("PASS: actual SD adapter, 128 command/write/read cycles use SPI1 without changing LCD backlight PA26 or its routing");
    return 0;
}
