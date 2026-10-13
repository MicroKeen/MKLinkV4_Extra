#include "hpm_spi.h"
#include "board.h"
#include "st7789.h"
#include "arm_2d_disp_adapter_0.h"
#define TEST_SPI                    BOARD_LCD_SPI_BASE
#define TEST_SPI_SCLK_FREQ          (50000000UL)
#define TEST_SPI_DATA_LEN_BITS      (8U)
#define TEST_SPI_DATA_LEN_BYTES     ((TEST_SPI_DATA_LEN_BITS + 7) / 8)
static void spi_txdma_complete_callback(uint32_t channel);
void lcd_spi_init(void)
{
    spi_initialize_config_t init_config;
    board_init_spi_clock(TEST_SPI);
    /* pins init*/
    board_init_spi_pins_with_gpio_as_cs(TEST_SPI);
    hpm_spi_get_default_init_config(&init_config);
    init_config.direction = spi_msb_first;
    init_config.mode = spi_master_mode;
    init_config.clk_phase = spi_sclk_sampling_odd_clk_edges;
    init_config.clk_polarity = spi_sclk_low_idle;
    init_config.data_len = TEST_SPI_DATA_LEN_BITS;
    /* step.1  initialize spi */
    if (hpm_spi_initialize(TEST_SPI, &init_config) != status_success) {
        printf("hpm_spi_initialize fail\n");
        while (1) {
        }
    }
    /* step.2  set spi sclk frequency for master */
    if (hpm_spi_set_sclk_frequency(TEST_SPI, TEST_SPI_SCLK_FREQ) != status_success) {
        printf("hpm_spi_set_sclk_frequency fail\n");
        while (1) {
        }
    }
    /* step.3 install dma callback if want use dma */
    if (hpm_spi_dma_mgr_install_callback(TEST_SPI, spi_txdma_complete_callback, NULL) != status_success) {
        printf("hpm_spi_dma_install_callback fail\n");
        while (1) {
        }
    }
}

void Disp0_DrawBitmap(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint8_t *bitmap)
{
   ST7789_DrawImage(x,y,width,height,(const uint16_t *)bitmap);
}

void __disp_adapter0_request_async_flushing( 
                void *pTarget,
                bool bIsNewFrame,
                int16_t iX, 
                int16_t iY,
                int16_t iWidth,
                int16_t iHeight,
                const uint16_t *pBuffer){
  (void)pTarget;
  (void)bIsNewFrame;               
  ST7789_DrawDMAImage(iX,iY,iWidth,iHeight,(const uint16_t *)pBuffer);
}

static void spi_txdma_complete_callback(uint32_t channel)
{
    (void)channel;
     disp_adapter0_insert_async_flushing_complete_event_handler(); 
}
