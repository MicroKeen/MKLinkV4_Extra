#include <stdint.h>
typedef int hpm_stat_t;
enum {status_success=0};
#define HPM_XPI0 ((void *)1)
#define BOARD_APP_XPI_NOR_CFG_OPT_HDR 0xfcf90002u
#define BOARD_APP_XPI_NOR_CFG_OPT_OPT0 6u
#define BOARD_APP_XPI_NOR_CFG_OPT_OPT1 0x1000u
#define ATTR_RAMFUNC __attribute__((section(".test_ramfunc")))
void fencei(void);
