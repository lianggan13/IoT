#ifndef _NRF24L01_H
#define _NRF24L01_H

// 通信引脚
#define IRQ_Port GPIOB
#define CE_Port GPIOB
#define MISO_Port GPIOA
#define MOSI_Port GPIOA
#define SCK_Port GPIOA
#define CSN_Port GPIOA

#define IRQ_Pin GPIO_Pin_1
#define CE_Pin GPIO_Pin_0
#define MOSI_Pin GPIO_Pin_7
#define MISO_Pin GPIO_Pin_6
#define SCK_Pin GPIO_Pin_5
#define CSN_Pin GPIO_Pin_4

// 寄存器地址代码
#define CONFIG 0x00
#define EN_AA 0x01
#define EN_RXADDR 0x02
#define SETUP_AW 0x03
#define SETUP_RETR 0x04
#define RF_CH 0x05
#define RF_SETUP 0x06
#define STATUS 0x07
#define OBSERVE_TX 0x08
#define CD 0x09
#define RX_ADDR_P0 0x0A
#define RX_ADDR_P1 0x0B
#define RX_ADDR_P2 0x0C
#define RX_ADDR_P3 0x0D
#define RX_ADDR_P4 0x0E
#define RX_ADDR_P5 0x0F
#define TX_ADDR 0x10
#define RX_PW_P0 0x11
#define RX_PW_P1 0x12
#define RX_PW_P2 0x13
#define RX_PW_P3 0x14
#define RX_PW_P4 0x15
#define RX_PW_P5 0x16
#define FIFO_STATUS 0x17
#define DYNPD 0x1C
#define FEATURE 0x1D

// 操作指令代码
#define R_REGISTER 0x00
#define W_REGISTER 0x20
#define R_RX_PAYLOAD 0x61
#define W_TX_PAYLOAD 0xA0
#define FLUSH_TX 0xE1
#define FLUSH_RX 0xE2
#define NOP 0xFF

// 状态
#define RX_OK 0x40
#define TX_OK 0x20
#define MAX_TX 0x10

/*********     24L01发送接收数据宽度定义	  ***********/
#define TX_ADR_WIDTH 5    // 5字节地址宽度
#define RX_ADR_WIDTH 5    // 5字节地址宽度
#define TX_PLOAD_WIDTH 32 // 32字节有效数据宽度(Len + Data)
#define RX_PLOAD_WIDTH 32 // 32字节有效数据宽度(Len + Data)

void W_Reg(uint8_t Reg, uint8_t Value);
uint8_t R_Reg(uint8_t Reg);
void W_Buf(uint8_t Reg, uint8_t *Buf, uint8_t Len);
void R_Buf(uint8_t Reg, uint8_t *Buf, uint8_t Len);
void NRF24L01_Init(void);
uint8_t NRF24L01_RxPacket(uint8_t *rxbuf);
uint8_t NRF24L01_TxPacket(uint8_t *Buf);
void NRF24L01_TxHalfDuplex(uint8_t *buf, uint8_t addr);
uint8_t R_IRQ(void);
void NRF24L01_Pin_Init(void);
void NRF24L01_TX_Mode(uint8_t *tx_addr);
void TestNRF24L01(void);

#endif
