/**************************************************************************************
*		           中断												  *
中断响应条件
 中断源有中断请求；
 此中断源的中断允许位为1；
 CPU开中断（即EA=1）。
以上三条同时满足时，CPU才有可能响应中断

P3.0/Rx
P3.1/Tx
P3.2/INT0
P3.3/INT1
P3.4/T0
P3.5/T1

IE 中断允许控制
EX0 外部中断0 允许位 —— INT0
ET0 定时器、计数器 T0 中断允许位 —— T0
EX1 外部中断1 允许位 —— INT1
ET1 定时器、计数器 T1 中断允许位 —— T1
x
x
ES  串行口中断允许位 —— RX TX
EA  总中断允许位

TCON 中断请求标志
IT0（TCON.0），外部中断0触发方式控制位。
		当IT0=0时，为电平触发方式。
		当IT0=1时，为边沿触发方式（下降沿有效）。
IE0（TCON.1），外部中断0中断请求标志位。
IT1（TCON.2），外部中断1触发方式控制位。
IE1（TCON.3），外部中断1中断请求标志位。

TR0（TCON.4），启动 定时/计数器T0
TF0（TCON.5），定时/计数器T0溢出中断请求标志位。
TR1（TCON.6），启动 定时/计数器T1
TF1（TCON.7），定时/计数器T1溢出中断请求标志位。


TMOD 定时/计数器工作方式
GATE是门控位
C/T :定时/计数模式选择位。C/T ＝0为定时模式;C/T =1为计数模式。
M1M0：工作方式设置位。定时/计数器有四种工作方式

***************************************************************************************/

#include "reg52.h" //此文件中定义了单片机的一些特殊功能寄存器
#include "Interrupt.h"
#include "../Utility/delay.h"
#include "../Utility/CommData.h"

typedef unsigned int u16; // 对数据类型进行声明定义
typedef unsigned char u8;

u8 n = 0;

sbit k3 = P3 ^ 2;  // 定义按键K3
sbit k4 = P3 ^ 3;  // 定义按键K4
sbit led = P2 ^ 0; // 定义P20口是led

void delay(u16 i)
{
	while (i--)
		;
}

void Int0Init()
{
	// 设置INT0
	IT0 = 1; // 跳变沿出发方式（下降沿）
	EX0 = 1; // 打开INT0的中断允许。
	EA = 1;	 // 打开总中断
}

void Int1Init()
{
	// 设置INT1
	IT1 = 1; // 跳变沿出发方式（下降沿）
	EX1 = 1; // 打开INT1的中断允许。
	EA = 1;	 // 打开总中断
}

void Timer0Init()
{
	TMOD |= 0X01; // 选择为定时器0模式，工作方式1，仅用TR0打开启动。

	TH0 = 0XFC; // 给定时器赋初值，定时1ms
	TL0 = 0X18;
	ET0 = 1; // 打开定时器0中断允许
	EA = 1;	 // 打开总中断
	TR0 = 1; // 打开定时器
}

void Timer1Init()
{
	TMOD |= 0X10; // 选择为定时器1模式，工作方式1，仅用TR1打开启动。

	TH1 = 0XFC; // 给定时器赋初值，定时1ms
	TL1 = 0X18;
	ET1 = 1; // 打开定时器1中断允许
	EA = 1;	 // 打开总中断
	TR1 = 1; // 打开定时器
}

void Int0() interrupt 0 // K3-P3^2-INT0 当 K3 按下 下降沿触发外部中断0
{
	delay(1000); // 延时消抖
	if (k3 == 0)
	{
		led = ~led;
	}
}

void Int1() interrupt 2 // 外部中断1的中断函数
{
	delay(1000); // 延时消抖
	if (k4 == 0)
	{
		led = ~led;
	}
}

void Timer0() interrupt 1
{
	static u16 i;
	TH0 = 0XFC; // 给定时器赋初值，定时1ms
	TL0 = 0X18;
	i++;
	if (i == 1000)
	{
		i = 0;
		led = ~led;
	}
}

void Timer1() interrupt 3
{
	static u16 j;
	TH1 = 0XFC; // 给定时器赋初值，定时1ms
	TL1 = 0X18;
	j++;
	if (j == 1000)
	{
		j = 0;
		P0 = ~smgduan[n++];
		if (n == 16)
			n = 0;
	}
}

void TestINT0()
{
	Int0Init(); //	设置外部中断0
	while (1)
		;
}

void TestINT1()
{
	Int1Init(); //	设置外部中断0
	while (1)
		;
}

void TestTF0()
{
	Timer0Init(); // 定时器0初始化
	while (1)
		;
}

void TestTF1()
{
	Timer1Init(); // 定时器0初始化
	while (1)
		;
}
