#ifndef BEEP_CONTROL_H
#define BEEP_CONTROL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PA10 / GPTMR0 channel 2; initialize after the RT-Thread kernel starts. */
void Beep_PWM_Init(void);

/* Frequency in Hz, duty in percent, time in 10 ms units.
 * time == 0: continuous tone; freq == 0 or duty == 0: stop. */
void Beep_PWM_frequency(uint32_t freq, uint8_t duty, uint16_t time);

#ifdef __cplusplus
}
#endif

#endif /* BEEP_CONTROL_H */
