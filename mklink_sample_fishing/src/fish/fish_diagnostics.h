#ifndef FISH_DIAGNOSTICS_H
#define FISH_DIAGNOSTICS_H
#include <stdint.h>
/* Zero cause means no captured CPU exception; otherwise cause is mcause + 1
 * so instruction-address-misaligned (mcause 0) is still distinguishable. */
typedef struct {uint32_t detail,cause,pc,value;} fish_fault_info_t;
#endif
