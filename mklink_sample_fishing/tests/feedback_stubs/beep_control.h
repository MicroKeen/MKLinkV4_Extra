#ifndef BEEP_CONTROL_H
#define BEEP_CONTROL_H
#include <stdint.h>
void Beep_PWM_Init(void);
void Beep_PWM_frequency(uint32_t freq,uint8_t duty,uint16_t time);
#endif
