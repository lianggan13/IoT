#include "stm32f10x.h"
#include "Delay.h"
#include "NRF24L01.h"

uint8_t T_ADDR[5] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t R_ADDR[5] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void W_MOSI(uint8_t Value)
{
	GPIO_WriteBit(MOSI_Port, MOSI_Pin, (BitAction)Value);
}

void W_SCK(uint8_t Value)
{
	GPIO_WriteBit(SCK_Port, SCK_Pin, (BitAction)Value);
}

void W_CSN(uint8_t Value)
{
	GPIO_WriteBit(CSN_Port, CSN_Pin, (BitAction)Value);
}

void W_CE(uint8_t Value)
{
	GPIO_WriteBit(CE_Port, CE_Pin, (BitAction)Value);
}

uint8_t R_IRQ(void)
{
	return GPIO_ReadInputDataBit(IRQ_Port, IRQ_Pin);
}

uint8_t R_MISO(void)
{
	return GPIO_ReadInputDataBit(MISO_Port, MISO_Pin);
}

void NRF24L01_Pin_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;

	// 配置成推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = MOSI_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(MOSI_Port, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = SCK_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SCK_Port, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = CSN_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CSN_Port, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = CE_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CE_Port, &GPIO_InitStructure);

	// 配置成上拉输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = IRQ_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(IRQ_Port, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = MISO_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(MISO_Port, &GPIO_InitStructure);
}

// SPI交换一个字节
uint8_t SPI_RW(uint8_t byte)
{
	// uint8_t i, ByteReceive = 0x00;
	// for (i = 0; i < 8; i++)
	// {
	// 	W_MOSI(byte & (0x80 >> i));
	// 	W_SCK(1);
	// 	if (R_MISO() == 1)
	// 	{
	// 		ByteReceive = ByteReceive | (0x80 >> i);
	// 	}
	// 	W_SCK(0);
	// }
	// return ByteReceive;

	uint8_t bit_ctr;
	for (bit_ctr = 0; bit_ctr < 8; bit_ctr++) // 输出8位
	{
		if ((uint8_t)(byte & 0x80) == 0x80)
			W_MOSI(1); // MSB TO MOSI
		else
			W_MOSI(0);
		byte = (byte << 1); // shift next bit to MSB
		W_SCK(1);
		byte |= R_MISO(); // capture current MISO bit
		W_SCK(0);
	}
	return byte;
}

void W_Reg(uint8_t Reg, uint8_t Value)
{
	W_CSN(0);	   // 表示选中NRF24L01
	SPI_RW(Reg);   // 交换的第一个字节就是指令
	SPI_RW(Value); // 交换的第二个字节就是交换的数据
	W_CSN(1);	   // 停止选中NRF24L01
}

uint8_t R_Reg(uint8_t Reg)
{
	uint8_t Value;
	W_CSN(0);			 // 表示选中NRF24L01
	SPI_RW(Reg);		 // 交换的第一个字节就是指令
	Value = SPI_RW(NOP); // 交换的第二个字节就是交换的数据
	W_CSN(1);			 // 停止选中NRF24L01
	return Value;
}

void W_Buf(uint8_t Reg, uint8_t *Buf, uint8_t Len)
{
	uint8_t i;
	W_CSN(0); // 选中NRF24L01
	SPI_RW(Reg);
	for (i = 0; i < Len; i++)
	{
		SPI_RW(Buf[i]);
	}
	W_CSN(1); // 停止选中NRF24L01
}

void R_Buf(uint8_t Reg, uint8_t *Buf, uint8_t Len)
{
	uint8_t i;
	W_CSN(0); // 选中NRF24L01
	SPI_RW(Reg);
	for (i = 0; i < Len; i++)
	{
		Buf[i] = SPI_RW(NOP);
	}
	W_CSN(1); // 停止选中NRF24L01
}

void NRF24L01_TX_Mode(uint8_t *tx_addr)
{
	W_CE(0);
	W_Buf(W_REGISTER + TX_ADDR, tx_addr, TX_ADR_WIDTH); // 写TX节点地址
	W_Reg(W_REGISTER + CONFIG, 0x0E);
	// W_Reg(FLUSH_RX, NOP);
	// W_Reg(FLUSH_TX, NOP);
	W_CE(1);
}

void NRF24L01_RX_Mode(void)
{
	W_CE(0);
	W_Reg(W_REGISTER + CONFIG, 0x0F);
	// W_Reg(FLUSH_RX, NOP);
	// W_Reg(FLUSH_TX, NOP);
	W_CE(1);
}

void ClearFlag()
{
	uint8_t state;

	state = R_Reg(STATUS);			   // 读取状态寄存器的值
	W_Reg(W_REGISTER + STATUS, state); // 清除TX_DS或MAX_RT中断标志
}

uint8_t NRF24L01_Check(void)
{
	uint8_t check_in_buf[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
	uint8_t check_out_buf[5] = {0x00};

	W_SCK(0); // NRF_SCK = 0;
	W_CSN(1); // NRF_CSN = 1;
	W_CE(0);  // NRF_CE = 0;

	W_Buf(W_REGISTER + TX_ADDR, check_in_buf, 5);

	R_Buf(W_REGISTER + TX_ADDR, check_out_buf, 5);

	if ((check_out_buf[0] == 0x11) &&
		(check_out_buf[1] == 0x22) &&
		(check_out_buf[2] == 0x33) &&
		(check_out_buf[3] == 0x44) &&
		(check_out_buf[4] == 0x55))
		return 0;
	else
		return 1;
}

void NRF24L01_Init(void)
{
	NRF24L01_Pin_Init();

	//	while (NRF24L01_Check())
	//		; // 等待检测到NRF24L01，程序才会向下执行

	W_CE(0);

	W_Reg(W_REGISTER + RX_PW_P0, RX_PLOAD_WIDTH); // 配置接收通道0接收的数据宽度32字节
	W_Reg(FLUSH_RX, NOP);

	W_Buf(W_REGISTER + TX_ADDR, T_ADDR, TX_ADR_WIDTH);	  // 配置发送地址
	W_Buf(W_REGISTER + RX_ADDR_P0, R_ADDR, RX_ADR_WIDTH); // 配置接收通道0
	W_Reg(W_REGISTER + EN_AA, 0x01);					  // 通道0开启自动应答
	W_Reg(W_REGISTER + EN_RXADDR, 0x01);				  // 接收通道0使能
	W_Reg(W_REGISTER + SETUP_RETR, 0x1A);				  // 配置580us重发时间间隔,重发10次
	W_Reg(W_REGISTER + RF_CH, 0x00);					  // 配置通信频率2.4G

	W_Reg(W_REGISTER + RF_SETUP, 0x0F); // 设置TX发射参数,0db增益,2Mbps,低噪声增益开启
	W_Reg(W_REGISTER + CONFIG, 0x0F);	// 配置成接收模式

	W_Reg(FLUSH_RX, NOP);
	W_Reg(FLUSH_TX, NOP);

	W_CE(1);

	Delay_us(15);

	ClearFlag();
}

// void Receive(uint8_t *Buf)
// {
// 	uint8_t Status;
// 	Status = R_Reg(R_REGISTER + STATUS);
// 	if (Status & RX_OK)
// 	{
// 		R_Buf(R_RX_PAYLOAD, Buf, 32);
// 		W_Reg(FLUSH_RX, NOP);
// 		W_Reg(W_REGISTER + STATUS, Status);
// 		Delay_us(150);
// 	}
// }

uint8_t NRF24L01_RxPacket(uint8_t *rxbuf)
{
	uint8_t state;

	state = R_Reg(STATUS);			   // 读取状态寄存器的值
	W_Reg(W_REGISTER + STATUS, state); // 清除TX_DS或MAX_RT中断标志
	if (state & RX_OK)				   // 接收到数据
	{
		W_CE(0);
		R_Buf(R_RX_PAYLOAD, rxbuf, RX_PLOAD_WIDTH); // 读取数据
		W_Reg(FLUSH_RX, NOP);						// 清除RX FIFO寄存器
		W_CE(1);
		Delay_us(150);
		return 0;
	}
	return 1; // 没收到任何数据
}

uint8_t NRF24L01_TxPacket(uint8_t *Buf)
{
	uint8_t state;

	W_CE(0);
	W_Buf(W_TX_PAYLOAD, Buf, TX_PLOAD_WIDTH); // 在发送数据缓存器发送要发送的数据
	W_CE(1);

	while (R_IRQ() == 1)
		; // 等待中断

	state = R_Reg(R_REGISTER + STATUS);
	W_Reg(W_REGISTER + STATUS, state); // 清除TX_DS或MAX_RT中断标志

	if (state & MAX_TX) // 如果发送达到最大次数
	{
		W_Reg(FLUSH_TX, NOP); // 清除发送数据缓存器
		return MAX_TX;
	}
	if (state & TX_OK) // 如果发送成功,接收到应答信号
	{
		return TX_OK;
	}
	return 0xff; // 发送失败
}

void NRF24L01_TxHalfDuplex(uint8_t *buf, uint8_t addr)
{
	T_ADDR[4] = addr;

	NRF24L01_TX_Mode(T_ADDR);

	// Delay_us(15);
	WhileDelay_us(15);

	NRF24L01_TxPacket(buf);

	NRF24L01_RX_Mode();
}

// uint8_t nrf24l01_buf[32];

void TestNRF24L01()
{
	uint8_t Buf[32] = {0x05, 0x66, 0x66, 0x66, 0x66, 0x66};

	NRF24L01_Init();

	//	while (1)
	//	{
	//		NRF24L01_TxPacket(Buf);
	//		Delay_ms(1100);
	//	}

	while (1)
	{
		if (R_IRQ() == 0) // 如果无线模块接收到数据
		{
			if (!NRF24L01_RxPacket(Buf))
			{
				NRF24L01_TxHalfDuplex(Buf, 0xFF); // 接收数据后，回传

				// if (nrf24l01_buf[1] == '1') // 第1位以后是收到的命令数据，rece_buf[0]是数据位数长度
				//     LED = 0;
				// if (nrf24l01_buf[1] == '2') // 第1位以后是收到的命令数据，rece_buf[0]是数据位数长度
				//     LED = 1;
			}
	
		}

		Delay_ms(10);
	}
}
