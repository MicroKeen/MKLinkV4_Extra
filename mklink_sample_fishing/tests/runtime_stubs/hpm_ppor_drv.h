#include <stdint.h>
enum {ppor_reset_wdog0=1u<<16,ppor_reset_wdog1=1u<<17,ppor_reset_pmic_wdog=1u<<24};
uint32_t ppor_reset_get_flags(void *);
void ppor_reset_clear_flags(void *,uint32_t);
