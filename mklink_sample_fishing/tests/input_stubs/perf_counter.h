#ifndef TEST_PERF_COUNTER_H
#define TEST_PERF_COUNTER_H
#include <stdint.h>
typedef enum {fsm_rt_cpl=0,fsm_rt_on_going=1} fsm_rt_t;
int64_t get_system_ticks(void);
static inline int64_t perfc_convert_ticks_to_ms(int64_t ticks){return ticks;}
#endif
