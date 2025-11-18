#include <stdio.h>	// 用于 printf
#include <stdlib.h> // 用于 malloc 和 free
#include <string.h> // 用于 memset
#include <stdint.h> // 用于 uint8_t

#include "stm32f10x.h"
#include "string.h"
#include "utility.h"
#include "Delay.h"
#include "timer.h"
#include "NRF24L01.h"
#include "syn6288.h"
#include "usart.h"

#define RN "\r\n"

uint8_t NRF_Buf[RX_PLOAD_WIDTH] = {0};
uint8_t SYN_Buf[RX_BufferSize] = {0};

size_t SYN_RX_Bytes()
{
	size_t len = strlen((const char *)SYN_Buf);
	return len;
}

void DataReceivedHandler(uint8_t *data, uint16_t length)
{
	// USART3_SendString(data, length);

	uint8_t NRF_Buf[TX_PLOAD_WIDTH] = {0};
	NRF_Buf[0] = length;
	AppendByteArray(NRF_Buf, sizeof(NRF_Buf), data, length);
	NRF24L01_TxHalfDuplex(NRF_Buf, 0xFF);
}

// USART3 中断服务程序
void USART3_IRQHandler(void)
{
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		u8 byte = USART3_RecvData();		 // 读取接收到的数据
		if (USART3_RX_Bytes < RX_BufferSize) // 还可以接收数据
		{
			TIM_SetCounter(TIM3, 0); // 计数器清空
			if (USART3_RX_Bytes == 0)
			{
				TIM_Cmd(TIM3, ENABLE); // 使能定时器3的中断 (如果这是第一次接收数据，则启用计时器用于超时处理)
			}

			USART3_RX_BUF[USART3_RX_Bytes++] = byte; // 记录接收到的值
		}
		else
		{
			USART3_ResetRX(); // 强制标记接收完成
		}
	}
}

// TIM3 中断服务程序 监测 USART3 接收数据
void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // 清除TIM3更新中断标志
		TIM_Cmd(TIM3, DISABLE);						// 关闭TIM3 (停止定时器，如果不需要持续监测)

		USART3_ResetRX(); // 标记接收完成 （重置接收标志）
	}
}

// TIM4 中断处理函数 监测 SYN8266 完整一帧数据
void TIM4_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) // 检查更新中断
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update); // 清除中断标志
		TIM_Cmd(TIM4, DISABLE);

		if (SYN_RX_Bytes() > 0)
		{
			SYN_Send(SYN_Buf);
			ClearArray(SYN_Buf, sizeof(SYN_Buf));
		}
	}
}

void send_cmd(const char *cmd, uint32_t timout)
{
	char buffer[64];
	snprintf(buffer, strlen(buffer), "%s%s", cmd, RN);
	// do
	//   uart2_send((uint8_t *)buffer, strlen(buffer), timout);
	// while (check_cmd(OK, timout));

	AppendByteArray((uint8_t *)buffer, sizeof(buffer), (uint8_t *)RN, strlen(RN));
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	char cmd[64];
	const char *ip = "10.60.0.66";
	const char *port = "6688";
	snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%s", ip, port);

	send_cmd(cmd, 1000);
	while (1)
	{
		Delay_ms(1000);
	}

	USART3_Init(9600);

	TIM3_Int_Init(49, 7199); // 中断周期-重载次数 5ms-50 10ms-100 50ms-500 1s-10000
	TIM4_Int_Init(3999, 7199);

	NRF24L01_Init();

	USART_DataReceived = DataReceivedHandler; // 设置 USART 接收回调函数

	// TestSYN6288();

	while (1)
	{
		if (R_IRQ() == 0)
		{
			if (!NRF24L01_RxPacket(NRF_Buf))
			{
				NRF24L01_TxHalfDuplex(NRF_Buf, 0x13);

				continue;

				TIM_SetCounter(TIM4, 0); // 计数器清空

				if (SYN_RX_Bytes() == 0)
				{
					TIM_Cmd(TIM4, ENABLE); // 使能定时器4的中断 (如果这是第一次接收数据，则启用计时器用于超时处理)
				}

				NRF24L01_TxHalfDuplex(NRF_Buf, 0x13);

				size_t NRF_Len = NRF_Buf[0];
				AppendByteArray(SYN_Buf, sizeof(SYN_Buf), &NRF_Buf[1], NRF_Len);
			}
		}
		else
		{
			Delay_ms(10);
		}
	}
}
