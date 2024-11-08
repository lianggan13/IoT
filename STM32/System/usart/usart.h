#ifndef __USART_H
#define __USART_H

#include "stdio.h"
#include "stm32f10x.h"

#define RX_BufferSize 256 // 定义最大接收字节数 200

typedef void (*USART_DataReceivedEvent)(uint8_t *data, uint16_t length);

extern u8 USART3_RX_BUF[RX_BufferSize]; // 接收缓冲,最大USART_REC_LEN个字节.末字节为换行符
extern u16 USART3_RX_Bytes;             // 接收状态标记
extern USART_DataReceivedEvent USART_DataReceived;

void USART3_Init(u32 bound);
void USART3_ResetRX(void);
void USART3_SendString(u8 *DAT, u8 len);
void USART3_SendData(u8 data);
#endif
