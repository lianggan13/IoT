#ifndef _ESP8266_H_
#define _ESP8266_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "syn6288.h"
#include "usart.h"
#include "time_handle.h"
#include "flash.h"
#include "core_json.h"
#include "utility.h"

#define ESP8266_STA_MODE 1
#define ESP8266_AP_MODE 2
#define ESP8266_STA_AP_MODE 3

#define ESP8266_SINGLE_CONNECTION 0
#define ESP8266_MULTI_CONNECTION 1

typedef struct Esp8266 Esp8266;

typedef void (*Send)(const char *cmd, uint32_t timeoutMs);
typedef uint8_t (*WaitRecv)(uint32_t timout);
typedef void (*ClearRecv)(void);

struct Esp8266
{
    uint8_t usart2_buf[512];        // 串口2接收缓存数组
    volatile uint16_t usart2_count; // 串口2接收数据计数器
    volatile bool it3;              // 定时器3中断标志 （true=TIM3 空闲超时->本帧接收结束；false=未开始/接收中）

    Send send;
    WaitRecv waitRecv;
    ClearRecv clearRecv;
};

extern Esp8266 esp8266;

void TIM3_PeriodElapsedCallback(void);

void HAL_UART2_IRQHandler(void);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);

void esp8266_init(void);

#endif
