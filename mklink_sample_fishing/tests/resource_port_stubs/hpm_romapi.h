typedef struct {unsigned unused;} xpi_nor_config_t;
typedef struct {struct {uint32_t U;} header,option0,option1;} xpi_nor_config_option_t;
enum {xpi_nor_property_total_size,xpi_nor_property_sector_size,xpi_nor_property_page_size,xpi_xfer_channel_auto};
int rom_xpi_nor_auto_config(void *,xpi_nor_config_t *,xpi_nor_config_option_t *);
int rom_xpi_nor_get_property(void *,xpi_nor_config_t *,unsigned,uint32_t *);
int rom_xpi_nor_program(void *,unsigned,xpi_nor_config_t *,const uint32_t *,uint32_t,unsigned);
int rom_xpi_nor_erase(void *,unsigned,xpi_nor_config_t *,uint32_t,unsigned);
