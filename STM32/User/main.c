#include <stdio.h>	// ���� printf�������Ҫ��
#include <stdlib.h> // ���� malloc �� free
#include <string.h> // ���� memset
#include <stdint.h> // ���� uint8_t

#include "stm32f10x.h"
#include "string.h"
#include "Delay.h"
#include "NRF24L01.h"
#include "syn6288.h"
#include "usart.h"

void AppendString(char *str, uint8_t *buf, int buf_len, uint8_t *output, int *output_len)
{
	int len = strlen(str);
	strcpy((char *)output, str);
	memcpy(output + len, buf, buf_len);
	*output_len = len + buf_len;
}

void AppendByteArray(uint8_t *bigArray, size_t bigArraySize, uint8_t *smallArray, size_t smallArraySize)
{
	// �ҵ�������ĵ�ǰ��Ч����
	size_t offset = strlen((const char *)bigArray); // ��������������ַ�����ʽ�洢��
	size_t bytesToCopy = smallArraySize;

	if (offset + smallArraySize >= bigArraySize)
	{
		bytesToCopy = bigArraySize - offset;
	}

	memcpy(bigArray + offset, smallArray, bytesToCopy); // ׷��С����
}

void ClearArray(uint8_t *array, size_t size)
{
	memset(array, 0, size); // ����������Ԫ������Ϊ 0
}

void DataReceivedHandler(uint8_t *data, uint16_t length)
{
	USART3_SendString(data, length);

	uint8_t NRF_Buf[TX_PLOAD_WIDTH] = {0};
	NRF_Buf[0] = length;
	AppendByteArray(NRF_Buf, sizeof(NRF_Buf), data, length);
	NRF24L01_TxHalfDuplex(NRF_Buf, 0xFF);
}

int main(void)
{
	uint8_t NRF_Buf[RX_PLOAD_WIDTH] = {0};
	uint8_t SYN_Buf[RX_BufferSize] = {0};

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	USART3_Init(9600);

	NRF24L01_Init();

	USART_DataReceived = DataReceivedHandler; // ���� USART ���ջص�����

	while (1)
	{
		if (R_IRQ() == 0)
		{
			if (!NRF24L01_RxPacket(NRF_Buf))
			{
				NRF24L01_TxHalfDuplex(NRF_Buf, 0xFF);

				size_t NRF_Len = NRF_Buf[0];

				if (NRF_Len == 2)
				{
					if (NRF_Buf[1] == 0xA3 && NRF_Buf[2] == 0xBA) // 0xA3BA  <--> ��
					{
						ClearArray(SYN_Buf, sizeof(SYN_Buf));
						continue;
					}
					else if (NRF_Buf[1] == 0xA1 && NRF_Buf[2] == 0xA3) // 0xA1A3 <--> ��
					{
						// SYN_FrameInfo(0, SYN_Buf);
						// ClearArray(SYN_Buf, sizeof(SYN_Buf));

						SYN_Send(SYN_Buf);
						ClearArray(SYN_Buf, sizeof(SYN_Buf));
						continue;
					}
				}

				AppendByteArray(SYN_Buf, sizeof(SYN_Buf), &NRF_Buf[1], NRF_Len);
			}
		}
		else
		{
			Delay_ms(10);
		}
	}
}
