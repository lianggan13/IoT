#ifndef __SYN6288_H
#define __SYN6288_H

#include "main.h"

void HAL_UART3_IRQHandler(void);

void syn6288_send_cmd(uint8_t *HZdata);

#endif
