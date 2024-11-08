/**************************************************************************************
*		              DA-PWM												  *
用定时器来固定周期长度，用定时器确定时间
			T = (0.05s = 50ms = 50000us) x 50 次 = 2.5s
			20% 00~10次 0.5s 高电平
			80% 10~50次 2.0s 低电平
***************************************************************************************/

#include "reg52.h"
#include "pwm.h"

#define uchar unsigned char;

sbit PWM = P2 ^ 1;
bit DIR;

uchar T = 1000;	  // 周期
uchar t0 = 0;	  // 定时器累加
uchar sample = 0; // 调节占空比 采样
uchar C;		  // 占空比

void SetTimerCount()
{
	// 2^16 50ms == 0.05s
	// TH0 = (65536 - 50000) / 256;
	// TL0 = (65536 - 50000) % 256;

	// 1us
	TH0 = 0xFF;
	TL0 = 0xff;
}

void Timer0Init_PWM()
{
	TMOD |= 0X01; // GATE CT M1 M0 GATE CT M1 M0
	SetTimerCount();
	EA = 1;
	ET0 = 1;
	TR0 = 1;
}


void Time0Interrupt(void) interrupt 1 // 3 为定时器1的中断号  1 定时器0的中断号 0 外部中断1 2 外部中断2  4 串口中断
{
	SetTimerCount();
	t0++;
	sample++;
}

void TestPWM()
{
	Timer0Init_PWM();
	while (1)
	{
		if (sample > (T / 10))
		{
			sample = 0;
			if (DIR == 1)
			{
				C++;
			}
			if (DIR == 0)
			{
				C--;
			}
		}

		if (C == T)
		{
			DIR = 0;
		}
		if (C == 0)
		{
			DIR = 1;
		}

		if (t0 > T)
		{
			t0 = 0;
		}

		if (t0 < C)
		{
			PWM = 1; // 灯亮
		}
		else
		{
			PWM = 0; // 灯灭
		}
	}
}
