#ifndef SD_TEST_HAL_H
#define SD_TEST_HAL_H
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef int hpm_stat_t;
enum {status_success=0,status_fail=-1,status_timeout=-2,SPI_SD_BLOCK_SIZE=512};
#define ATTR_PLACE_AT_NONCACHEABLE
typedef struct {unsigned marker;} SPI_Type;
extern SPI_Type sd_test_spi[2];
#define BOARD_APP_SPI_BASE (&sd_test_spi[1])
#define BOARD_APP_SPI_CLK_NAME 1
#define BOARD_SPI_CS_PIN 26
#define BOARD_SPI_CS_GPIO_CTRL 0
#define GPIO_GET_PORT_INDEX(pin) 0
#define GPIO_GET_PIN_INDEX(pin) (pin)
#define IOC_PA26_FUNC_CTL_GPIO_A_26 0
#define GPIOM_ASSIGN_GPIOA 0
#define gpiom_soc_gpio0 0
#define HPM_GPIOM 0
typedef struct {uint32_t FUNC_CTL;} sd_test_pad_t;
typedef struct {sd_test_pad_t PAD[32];} sd_test_ioc_t;
extern sd_test_ioc_t sd_test_ioc;
#define HPM_IOC (&sd_test_ioc)
enum {spi_msb_first,spi_master_mode,spi_sclk_sampling_even_clk_edges,spi_sclk_high_idle};
typedef struct {unsigned direction,mode,clk_phase,clk_polarity,data_len;} spi_initialize_config_t;
typedef struct {
    hpm_stat_t (*set_spi_speed)(uint32_t);
    void (*cs_select)(void),(*cs_relese)(void);
    bool (*sdcard_is_present)(void);
    hpm_stat_t (*write_read_byte)(uint8_t *,uint8_t *);
    hpm_stat_t (*write_cmd_data)(uint8_t,uint8_t *,uint32_t);
    hpm_stat_t (*write)(uint8_t *,uint32_t),(*read)(uint8_t *,uint32_t);
    void (*delay_ms)(uint32_t),(*delay_us)(uint32_t);
} sdcard_spi_interface_t;
uint32_t board_init_spi_clock(SPI_Type *p);
void board_init_spi_pins_with_gpio_as_cs(SPI_Type *p);
void board_write_spi_cs(uint32_t pin,uint8_t state);
void gpiom_set_pin_controller(unsigned base,unsigned port,unsigned pin,unsigned owner);
void gpio_set_pin_output_with_initial(unsigned base,unsigned port,unsigned pin,bool high);
void board_delay_ms(uint32_t ms);
void board_delay_us(uint32_t us);
void hpm_spi_get_default_init_config(spi_initialize_config_t *c);
hpm_stat_t hpm_spi_initialize(SPI_Type *p,spi_initialize_config_t *c);
hpm_stat_t hpm_spi_set_sclk_frequency(SPI_Type *p,uint32_t hz);
hpm_stat_t hpm_spi_transmit_blocking(SPI_Type *p,uint8_t *b,uint32_t n,uint32_t timeout);
hpm_stat_t hpm_spi_transmit_receive_blocking(SPI_Type *p,uint8_t *tx,uint8_t *rx,uint32_t n,uint32_t timeout);
hpm_stat_t sdcard_spi_init(sdcard_spi_interface_t *io);
#endif
