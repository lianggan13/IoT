/**************************************************************************************
*		              DA-PWM												  *
�ö�ʱ�����̶����ڳ��ȣ��ö�ʱ��ȷ��ʱ��
			T = (0.05s = 50ms = 50000us) x 50 �� = 2.5s
			20% 00~10�� 0.5s �ߵ�ƽ
			80% 10~50�� 2.0s �͵�ƽ
***************************************************************************************/

#include "reg52.h"
#include "pwm.h"

#define uchar unsigned char;

sbit PWM = P2 ^ 1;
bit DIR;

uchar T = 1000;	  // ����
uchar t0 = 0;	  // ��ʱ���ۼ�
uchar sample = 0; // ����ռ�ձ� ����
uchar C;		  // ռ�ձ�

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


void Time0Interrupt(void) interrupt 1 // 3 Ϊ��ʱ��1���жϺ�  1 ��ʱ��0���жϺ� 0 �ⲿ�ж�1 2 �ⲿ�ж�2  4 �����ж�
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
			PWM = 1; // ����
		}
		else
		{
			PWM = 0; // ����
		}
	}
}
