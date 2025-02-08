#ifndef __TIME_HANDLE_H__
#define __TIME_HANDLE_H__

#include "esp8266.h"

void TIM2_Start(void);
void TIM3_Start(void);
void TIM3_Reset(void);
void TIM3_Stop(void);
uint8_t TIM3_IsUpdateInterrupt(void);

#endif
