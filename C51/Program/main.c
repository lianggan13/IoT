
#include <reg51.h>
#include <intrins.h>
#include <string.h>
#include "../Utility/delay.h"
#include "../NRF24L01/NRF24L01.h"
#include "../Keyboard/KeyMatrix.h"
#include "../Led/Led_8x8.h"
#include "../Interrupt/Interrupt.h"
#include "../ROM/EEPROM.h"
#include "../Temp/DS18B20.h"

// TODO List:
// Proteus https://zhuanlan.zhihu.com/p/655950841
// Altium Designer

void TestNRF24L01_Key()
{
	NRF24L01_Init();
	while (1)
	{
		int key = CheckKeyDown();

		if (NRF_IRQ == 0) // 如果无线模块接收到数据
		{
			if (!NRF24L01_RxPacket(nrf24l01_buf))
			{
				NRF24L01_TxHalfDuplex(nrf24l01_buf, 0xA5); // 接收数据后，回传
			}
		}

		if (key > -1)
		{
			uchar values[TX_PLOAD_WIDTH];
			values[1] = key;
			values[0] = 1; // strlen(values);

			NRF24L01_TxHalfDuplex(values, 0xA5);
		}

		delay_ms(10);
	}
}

void main()
{
	// TestNRF24L01();
	// TestKeyMatrix();
	// TestNRF24L01_Key();

	// DisplayZero();
	// DisplayHV();
	// DisplayCodes();

	// TestTF1();
	// TestEEPROM();

	TestDS18B20();
}
