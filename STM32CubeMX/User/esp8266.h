#ifndef _ESP8266_H_
#define _ESP8266_H_

#include "main.h"
void TIM3_PeriodElapsedCallback(void);
void HAL_UART2_IRQHandler(void);

void esp8266_init(void);
#endif
