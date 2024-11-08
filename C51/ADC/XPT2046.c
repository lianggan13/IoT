
#include "reg52.h"
#include <intrins.h>
#include "XPT2046.h"
#include "../Utility/CommData.h"
#include "../LED/DynamicLed.h"
#include "../Utility/delay.h"

typedef unsigned int u16; // 对数据类型进行声明定义
typedef unsigned char u8;

sbit DOUT = P3 ^ 7; // 输出
sbit CLK = P3 ^ 6;	// 时钟
sbit DIN = P3 ^ 4;	// 输入
sbit CS = P3 ^ 5;	// 片选

void SPI_Write(unsigned char dat)
{
	unsigned char i;
	CLK = 0;
	for (i = 0; i < 8; i++)
	{
		DIN = dat >> 7; // 放置最高位
		dat <<= 1;		// 左移: 最高位移走 次高位变最高位
		CLK = 0;		// 上升沿放置数据

		CLK = 1;
	}
}

unsigned int SPI_Read(void)
{
	unsigned int i, dat = 0;
	CLK = 0;
	for (i = 0; i < 12; i++) // 接收12位数据
	{
		dat <<= 1;

		CLK = 1;
		CLK = 0;

		dat |= DOUT;
	}
	return dat;
}

/****************************************************************************
 *函数名：Read_AD_Data
 *输  入：cmd：读取的X或者Y
 *输  出：endValue：最终信号处理后返回的值
 *功  能：读取触摸数据
 ****************************************************************************/
unsigned int Read_AD_Data(unsigned char cmd)
{
	unsigned char i;
	unsigned int AD_Value;
	CLK = 0;
	CS = 0;
	SPI_Write(cmd);
	for (i = 6; i > 0; i--)
		;	 // 延时等待转换结果
	CLK = 1; // 发送一个时钟周期，清除BUSY
	_nop_();
	_nop_();
	CLK = 0;
	_nop_();
	_nop_();
	AD_Value = SPI_Read();
	CS = 1;
	return AD_Value;
}

void datapros01()
{
	u16 temp;
	static u8 i;
	if (i == 50)
	{
		i = 0;
		// 控制字
		// AIN0 X+   电位器      0X94
		// AIN1 Y+   热敏电阻    0XD4
		// AIN2 VBAT 光敏电阻    0XA4
		// AIN3 AUX  外部输入    0XE4
		temp = Read_AD_Data(0x94); //   AIN0 电位器
								   // temp = Read_AD_Data(0xD4); //   AIN1 热敏电阻
								   // temp = Read_AD_Data(0xA4); //   AIN2 光敏电阻
								   // temp = Read_AD_Data(0xE4); //   AIN3 外部输入
	}
	i++;
	DisplayData[0] = smgduan[temp / 1000];			  // 千位
	DisplayData[1] = smgduan[temp % 1000 / 100];	  // 百位
	DisplayData[2] = smgduan[temp % 1000 % 100 / 10]; // 个位
	DisplayData[3] = smgduan[temp % 1000 % 100 % 10];
}

void TestXPT2046()
{
	while (1)
	{
		datapros01(); // 数据处理函数
		DigDisplay(); // 数码管显示函数
	}
}