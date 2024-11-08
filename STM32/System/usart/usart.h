#ifndef __USART_H
#define __USART_H

#include "stdio.h"
#include "stm32f10x.h"

#define RX_BufferSize 256 // �����������ֽ��� 200

typedef void (*USART_DataReceivedEvent)(uint8_t *data, uint16_t length);

extern u8 USART3_RX_BUF[RX_BufferSize]; // ���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з�
extern u16 USART3_RX_Bytes;             // ����״̬���
extern USART_DataReceivedEvent USART_DataReceived;

void USART3_Init(u32 bound);
void USART3_ResetRX(void);
void USART3_SendString(u8 *DAT, u8 len);
void USART3_SendData(u8 data);
#endif
