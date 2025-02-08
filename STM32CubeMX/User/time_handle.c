#include "time_handle.h"
#include "main.h"
#include "tim.h"
#include <stdio.h>

/*
*************************************
�궨��
*************************************
*/
/*
*************************************
�궨��
*************************************
*/
/*
*************************************
��������
*************************************
*/
/*
*************************************
��������
*************************************
*/
static uint16_t index_10ms = 0;
uint16_t index_led = 0;
uint16_t index_oled = 0;
uint16_t index_dht11 = 0;

/**
 * @brief          ��ʱ������ص�����
 * @param[in]      htim:��ʱ��
 * @retval         none
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == htim2.Instance)
  {
    // ���� TIM2 ������¼�,10msһ�λص�
    index_10ms++;
    if (index_10ms % 100 == 0)
    {
      index_led = 1;
      index_oled = 1;
      index_dht11 = 1;
    }
  }
  // else if (htim->Instance == htim3.Instance)
  else if (htim->Instance == TIM3)
  {
    // ���� TIM3 ������¼�,5msһ�λص�
    TIM3_PeriodElapsedCallback();
  }
}

// ���� TIM2 �ж�
void TIM2_Start(void)
{
  HAL_TIM_Base_Start_IT(&htim2); // �򿪶�ʱ��2�ж�
}

// ���� TIM3 �ж�
void TIM3_Start(void)
{
  HAL_TIM_Base_Start_IT(&htim3); // ���� TIM3 ���ж�
}

// ���� TIM3 ����
void TIM3_Reset(void)
{
  __HAL_TIM_SET_COUNTER(&htim3, 0); // ������������Ϊ 0
  __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_UPDATE);
}

// ֹͣ TIM3 �ж�
void TIM3_Stop(void)
{
  HAL_TIM_Base_Stop_IT(&htim3); // ֹͣ TIM3 ���ж�
}

// ��� TIM3 �Ƿ�����˸����ж�
uint8_t TIM3_IsUpdateInterrupt(void)
{
  // ��� TIM3 �����жϱ�־
  if (__HAL_TIM_GET_FLAG(&htim3, TIM_FLAG_UPDATE) != RESET)
  {
    // ����ж��Ƿ����
    if (__HAL_TIM_GET_IT_SOURCE(&htim3, TIM_IT_UPDATE) != RESET)
    {
      return 1; // �ж��Ѳ���
    }
  }
  return 0; // �ж�δ����
}
