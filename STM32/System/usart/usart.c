#include <string.h>
#include "usart.h"
#include "timer.h"

#if 1
#pragma import(__use_no_semihosting)
// 标准库需要的支持函数
struct __FILE
{
	int handle;
};

FILE __stdout;
// 定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
	x = x;
}
// 重定义fputc函数
int fputc(int ch, FILE *f)
{
	while ((USART3->SR & 0X40) == 0)
		; // 循环发送,直到发送完毕
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

u16 USART3_RX_Bytes = 0;		 // 接收状态标记
u8 USART3_RX_BUF[RX_BufferSize]; // 接收缓冲,最大USART_REC_LEN个字节.
USART_DataReceivedEvent USART_DataReceived = NULL;

void USART3_Init(u32 bound)
{
	// GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // 使能USART3，GPIOC时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	// USART3_TX   GPIOC.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);			// 初始化GPIO

	// USART3_RX	  GPIOC.11初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);				  // 初始化GPIO

	// USART3 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = 39;				  // USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // 抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // 子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  // 根据指定的参数初始化VIC寄存器

	// USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;										// 串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 收发模式

	USART_Init(USART3, &USART_InitStructure);	   // 初始化串口3
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); // 开启串口接受中断
	USART_Cmd(USART3, ENABLE);					   // 使能串口3
}

void USART3_ResetRX(void)
{
	if (USART_DataReceived != NULL)
	{
		USART_DataReceived(USART3_RX_BUF, USART3_RX_Bytes); // 调用回调函数，传入接收到的数据
	}

	USART3_RX_Bytes = 0;
	memset(USART3_RX_BUF, 0, RX_BufferSize); // 可以选择清空接收缓冲区
}
