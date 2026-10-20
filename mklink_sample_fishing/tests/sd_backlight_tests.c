/* Compile the actual board SD adapter; all hardware calls are observed here. */
#include "sd_test_hal.h"
#include "spi_sd_adapt.h"
#include <assert.h>
SPI_Type sd_test_spi[2];
sd_test_ioc_t sd_test_ioc;
static sdcard_spi_interface_t *card;
static unsigned gpio_writes,pin_reroutes,transfers;
static bool backlight=true;
static uint32_t now;
static unsigned heartbeats;
static uint32_t clock_hz;
static bool wire_test,wire_reject,wire_stuck;
static unsigned wire_phase,wire_polls,wire_busy;
static uint32_t wire_address;
int64_t get_system_time_ms(void) {return now;}
void fish_hpm_storage_heartbeat(void) {++heartbeats;}
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
    sd_only(p);assert(hz);clock_hz=hz;return status_success;
}
hpm_stat_t hpm_spi_transmit_blocking(SPI_Type *p,uint8_t *b,uint32_t n,uint32_t timeout)
{
    sd_only(p);assert(b && n && timeout==100);++transfers;
    if(wire_test) {
        if(n==6){assert(b[0]==0x58);wire_address=(uint32_t)b[1]<<24|(uint32_t)b[2]<<16|(uint32_t)b[3]<<8|b[4];wire_phase=1;}
        else if(n==512){assert(wire_phase==2);for(unsigned i=0;i<n;i++)assert(b[i]==0xff);wire_phase=3;}
        else if(n==2){assert(wire_phase==3 && b[0]==0x7f && b[1]==0xa1);wire_phase=4;}
    }
    return status_success;
}
hpm_stat_t hpm_spi_transmit_receive_blocking(SPI_Type *p,uint8_t *tx,uint8_t *rx,uint32_t n,uint32_t timeout)
{
    sd_only(p);assert(tx && rx && n && timeout==100);memset(rx,0xff,n);++transfers;
    if(wire_test) {
        if(wire_phase==1){*rx=0;wire_phase=2;}
        else if(wire_phase==2)assert(*tx==0xfe);
        else if(wire_phase==4){if(++wire_polls>3){*rx=wire_reject?0x0b:5;wire_phase=5;}}
        else if(wire_phase==5){if(wire_stuck || ++wire_busy<=3){*rx=0;++now;}}
    }
    return status_success;
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
    assert(card->set_spi_speed(20000000)==status_success);
    card->cs_select();assert(card->write_cmd_data(0x58,command,5)==status_success);
    assert(clock_hz==4000000);card->cs_relese();assert(clock_hz==20000000);
    card->cs_select();assert(card->write_cmd_data(0x52,command,5)==status_success);
    assert(clock_hz==20000000);card->cs_relese();
    card->cs_select();assert(card->write_cmd_data(0x58,command,5)==status_success);
    now+=2000;assert(card->write(block,sizeof(block))==status_timeout);
    card->cs_relese();assert(clock_hz==20000000);
    /* An SDK-style response polling loop must escape without petting the dog
     * inside the loop. This also exercises uint32_t clock wrap. */
    now=UINT32_MAX-1000;card->cs_select();unsigned fed=heartbeats,attempts=0;
    int result;
    do {now+=20;result=card->write_read_byte(&tx,&rx);++attempts;}while(result==status_success);
    assert(result==status_timeout && attempts==100 && heartbeats==fed);
    unsigned before=transfers;
    assert(card->read(block,sizeof(block))==status_timeout);
    assert(card->write(block,sizeof(block))==status_timeout);
    assert(card->write_cmd_data(24,command,5)==status_timeout && transfers==before);
    card->cs_relese();assert(heartbeats==fed+1);
    card->cs_select();assert(card->read(block,sizeof(block))==status_success);card->cs_relese();
    assert(backlight && !gpio_writes && !pin_reroutes);
    puts("PASS: actual SD adapter, 128 command/write/read cycles use SPI1 without changing LCD backlight PA26 or its routing");
    memset(block,0xff,sizeof(block));wire_test=true;wire_phase=wire_polls=wire_busy=0;
    assert(spi_sd_write_sector(17,block,true)==status_success && wire_address==17 && !spi_sd_write_diagnostic());
    assert(clock_hz==20000000 && wire_polls==4 && wire_busy==4 && backlight);
    wire_phase=wire_polls=wire_busy=0;
    assert(spi_sd_write_sector(17,block,false)==status_success && wire_address==17*512);
    wire_phase=wire_polls=wire_busy=0;wire_reject=true;
    assert(spi_sd_write_sector(18,block,true)!=status_success && spi_sd_write_diagnostic()==0x090b);
    assert(clock_hz==20000000 && backlight);wire_reject=false;
    wire_phase=wire_polls=wire_busy=0;wire_stuck=true;
    assert(spi_sd_write_sector(19,block,true)!=status_success && (spi_sd_write_diagnostic()>>8)==10);
    assert(clock_hz==20000000 && backlight && !gpio_writes && !pin_reroutes);
    puts("PASS: CMD24/SDSC/SDHC addressing, real CRC16, delayed response acceptance, CRC rejection and bounded busy failure, read clock/backlight restored");
    return 0;
}
