#include <string.h>
#include "usart.h"
#include "timer.h"

#if 1
#pragma import(__use_no_semihosting)
// ��׼����Ҫ��֧�ֺ���
struct __FILE
{
	int handle;
};

FILE __stdout;
// ����_sys_exit()�Ա���ʹ�ð�����ģʽ
void _sys_exit(int x)
{
	x = x;
}
// �ض���fputc����
int fputc(int ch, FILE *f)
{
	while ((USART3->SR & 0X40) == 0)
		; // ѭ������,ֱ���������
	USART3->DR = (u8)ch;
	return ch;
}
#endif

void USART3_SendData(u8 data)
{
	while ((USART3->SR & 0X40) == 0)
		;
	USART3->DR = data;
}

void USART3_SendString(u8 *DAT, u8 len)
{
	u8 i;
	for (i = 0; i < len; i++)
	{
		USART3_SendData(*DAT++);
	}
}

u8 USART3_RecvData(void)
{
	return (u8)(USART3->DR & (u8)0x00FF);
}

u16 USART3_RX_Bytes = 0;		 // ����״̬���
u8 USART3_RX_BUF[RX_BufferSize]; // ���ջ���,���USART_REC_LEN���ֽ�.
USART_DataReceivedEvent USART_DataReceived = NULL;

void USART3_Init(u32 bound)
{
	// GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // ʹ��USART3��GPIOCʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	// USART3_TX   GPIOC.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // �����������
	GPIO_Init(GPIOB, &GPIO_InitStructure);			// ��ʼ��GPIO

	// USART3_RX	  GPIOC.11��ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // ��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);				  // ��ʼ��GPIO

	// USART3 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = 39;				  // USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // ��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // �����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							  // ����ָ���Ĳ�����ʼ��VIC�Ĵ���

	// USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;										// ���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// �ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;								// ����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // ��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// �շ�ģʽ

	USART_Init(USART3, &USART_InitStructure);	   // ��ʼ������3
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); // �������ڽ����ж�
	USART_Cmd(USART3, ENABLE);					   // ʹ�ܴ���3
}

void USART3_ResetRX(void)
{
	if (USART_DataReceived != NULL)
	{
		USART_DataReceived(USART3_RX_BUF, USART3_RX_Bytes); // ���ûص�������������յ�������
	}

	USART3_RX_Bytes = 0;
	memset(USART3_RX_BUF, 0, RX_BufferSize); // ����ѡ����ս��ջ�����
}