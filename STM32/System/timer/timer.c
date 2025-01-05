#include "stm32f10x.h"
#include "timer.h"
#include "usart.h"

// ͨ�ö�ʱ��3�жϳ�ʼ��
// ����ʱ��ѡ��ΪAPB1��2������APB1Ϊ42M
// arr���Զ���װֵ��
// psc��ʱ��Ԥ��Ƶ��
// ��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
// Ft=��ʱ������Ƶ��,��λ:Mhz
// ͨ�ö�ʱ���жϳ�ʼ��
// ����ʼ��ѡ��ΪAPB1��2������APB1Ϊ36M
// arr���Զ���װֵ��
// psc��ʱ��Ԥ��Ƶ��
void TIM3_Int_Init(u16 arr, u16 psc)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); // TIM3ʱ��ʹ��

	// ��ʱ��TIM3��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr;						// ��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler = psc;					// ����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		// ����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);				// ����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); // ʹ��ָ����TIM3�ж�,��������ж�

	TIM_Cmd(TIM3, ENABLE); // ������ʱ��3

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // ��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		  // �����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							  // ����ָ���Ĳ�����ʼ��VIC�Ĵ���

	TIM_Cmd(TIM3, DISABLE); // �رն�ʱ��3
}

// TIM4��ʼ��
void TIM4_Int_Init(u16 arr, u16 psc)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	// ʹ�� TIM4 ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	// ��ʱ�� TIM4 ��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr;						// �����Զ���װ�ؼĴ�������
	TIM_TimeBaseStructure.TIM_Prescaler = psc;					// ����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		// ����ʱ�ӷָ�
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // ���ϼ���ģʽ
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);				// ��ʼ�� TIM4

	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE); // ʹ�� TIM4 �����ж�

	TIM_Cmd(TIM4, ENABLE); // ������ʱ��4

	// NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;			  // �ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // ��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		  // �����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							  // ��ʼ�� NVIC

	TIM_Cmd(TIM4, DISABLE); // �رն�ʱ��4
}
