/**************************************************************************************
*		              DS1302时钟实验												  *
注意事项：	注意将时钟模块的JP1302短接片短接，
			74HC595模块的短接片JP595拔掉，
			NE555模块的短接片J11拔掉。
***************************************************************************************/

#include <reg52.h>
#include <intrins.h>
#include "ds1302.h"
#include "../Utility/CommData.h"
#include "../LED/DynamicLed.h"

//---定义ds1302使用的IO口---//
sbit DSIO = P3 ^ 4;
sbit RST = P3 ^ 5;
sbit SCLK = P3 ^ 6;

// 地址寄存器格式:
// D7 1
// D6 RAM/_CK =1片内RAM，=0日历、时钟寄存器选择位
// D5 ~ D1 地址位
// D0 RD/_W 读写选择，=1读，=0写

//---DS1302写入和读取时分秒的地址命令---//
//---秒 分 时(24时制作) 日 月 周 年 (最低位读写位);-------//
unsigned char code READ_RTC_ADDR[7] = {0x81, 0x83, 0x85, 0x87, 0x89, 0x8b, 0x8d};  // 读地址
unsigned char code WRITE_RTC_ADDR[7] = {0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c}; // 写地址

//---DS1302时钟初始化2016年5月7日星期六12点00分00秒。---//
//---存储顺序：秒 分 时 日 月 周 年 （存储格式 BCD码)---//
unsigned char TIME[7] = {0, 0, 0x12, 0x07, 0x05, 0x06, 0x16};

/*******************************************************************************
 * 函 数 名         : Ds1302Write
 * 函数功能		   : 向DS1302命令（地址+数据）
 * 输    入         : addr,dat
 * 输    出         : 无
 *******************************************************************************/

void Ds1302Write(unsigned char addr, unsigned char dat)
{
	unsigned char n;
	RST = 0;
	_nop_();

	SCLK = 0; // 先将SCLK置低电平。
	_nop_();
	RST = 1; // 然后将RST(CE)置高电平。
	_nop_();

	for (n = 0; n < 8; n++) // 开始传送八位地址命令
	{
		DSIO = addr & 0x01; // 数据从低位开始传送
		addr >>= 1;
		SCLK = 1; // 数据在上升沿时，DS1302读取数据
		_nop_();
		SCLK = 0;
		_nop_();
	}
	for (n = 0; n < 8; n++) // 写入8位数据
	{
		DSIO = dat & 0x01;
		dat >>= 1;
		SCLK = 1; // 数据在上升沿时，DS1302读取数据
		_nop_();
		SCLK = 0;
		_nop_();
	}

	RST = 0; // 传送数据结束
	_nop_();
}

/*******************************************************************************
 * 函 数 名         : Ds1302Read
 * 函数功能		   : 读取一个地址的数据
 * 输    入         : addr
 * 输    出         : dat
 *******************************************************************************/

unsigned char Ds1302Read(unsigned char addr)
{
	unsigned char n, dat, dat1;
	RST = 0;
	_nop_();

	SCLK = 0; // 先将SCLK置低电平。
	_nop_();
	RST = 1; // 然后将RST(CE)置高电平。
	_nop_();

	for (n = 0; n < 8; n++) // 开始传送八位地址命令
	{
		DSIO = addr & 0x01; // 数据从低位开始传送
		addr >>= 1;
		SCLK = 1; // 数据在上升沿时，DS1302读取数据
		_nop_();
		SCLK = 0; // DS1302下降沿时，放置数据
		_nop_();
	}
	_nop_();
	for (n = 0; n < 8; n++) // 读取8位数据
	{
		dat1 = DSIO; // 从最低位开始接收
		dat = (dat >> 1) | (dat1 << 7);
		SCLK = 1;
		_nop_();
		SCLK = 0; // DS1302下降沿时，放置数据
		_nop_();
	}

	RST = 0;
	_nop_(); // 以下为DS1302复位的稳定时间,必须的。
	SCLK = 1;
	_nop_();
	DSIO = 0;
	_nop_();
	DSIO = 1;
	_nop_();
	return dat;
}

/*******************************************************************************
 * 函 数 名         : Ds1302Init
 * 函数功能		   : 初始化DS1302.
 * 输    入         : 无
 * 输    出         : 无
 *******************************************************************************/

void Ds1302Init()
{
	unsigned char n;
	//
	Ds1302Write(0x8E, 0X00); // 禁止写保护，就是关闭写保护功能 写地址:0x8E 值: 0x00
	for (n = 0; n < 7; n++)	 // 写入7个字节的时钟信号：分秒时日月周年
	{
		Ds1302Write(WRITE_RTC_ADDR[n], TIME[n]);
	}
	Ds1302Write(0x8E, 0x80); // 打开写保护功能
}

/*******************************************************************************
 * 函 数 名         : Ds1302ReadTime
 * 函数功能		   : 读取时钟信息
 * 输    入         : 无
 * 输    出         : 无
 *******************************************************************************/

void Ds1302ReadTime()
{
	unsigned char n;
	for (n = 0; n < 7; n++) // 读取7个字节的时钟信号：分秒时日月周年
	{
		TIME[n] = Ds1302Read(READ_RTC_ADDR[n]);
	}
}

void SetDateTime()
{
	Ds1302ReadTime();
	// DisplayData[0] = smgduan[TIME[2]/16];  //时
	DisplayData[0] = smgduan[TIME[2] & 0xf0]; // 时
	DisplayData[1] = smgduan[TIME[2] & 0x0f];
	DisplayData[2] = 0x40;
	DisplayData[3] = smgduan[TIME[1] / 16]; // 分
	DisplayData[4] = smgduan[TIME[1] & 0x0f];
	DisplayData[5] = 0x40;
	DisplayData[6] = smgduan[TIME[0] / 16]; // 秒
	DisplayData[7] = smgduan[TIME[0] & 0x0f];
}

void TestDS1302()
{
	Ds1302Init();
	while (1)
	{
		SetDateTime(); // 数据处理函数
		DigDisplay();  // 数码管显示函数
	}
}
