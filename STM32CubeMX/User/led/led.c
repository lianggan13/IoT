
#include "led.h"

void set_led(uint8_t status)
{
  if (status == 1)
  {
    HAL_GPIO_WritePin(GPIOB, LEDb13_Pin, GPIO_PIN_RESET);
  }
  else
  {
    HAL_GPIO_WritePin(GPIOB, LEDb13_Pin, GPIO_PIN_SET);
  }
}

void test_led()
{
  int time = 5000;
  while (1)
  {
    // Ϩ��
    // HAL_GPIO_WritePin(GPIOB, LEDb13_Pin, GPIO_PIN_RESET); // ���ùܽ�1ΪLOW
    // HAL_GPIO_WritePin(GPIOB, LEDb14_Pin, GPIO_PIN_RESET); // ���ùܽ�3ΪLOW
    // printf("LOW LOW\n");
    // HAL_Delay(time);

    // ��ɫ1��
    HAL_GPIO_WritePin(GPIOB, LEDb13_Pin, GPIO_PIN_SET);   // ���ùܽ�1ΪHIGH
    HAL_GPIO_WritePin(GPIOB, LEDb14_Pin, GPIO_PIN_RESET); // ���ùܽ�3ΪLOW
    HAL_GPIO_WritePin(GPIOB, Jb3_Pin, GPIO_PIN_SET);
    printf("HIGH LOW\n");
    HAL_Delay(time);

    // ��ɫ2��
    HAL_GPIO_WritePin(GPIOB, LEDb13_Pin, GPIO_PIN_RESET); // ���ùܽ�1ΪLOW
    HAL_GPIO_WritePin(GPIOB, LEDb14_Pin, GPIO_PIN_SET);   // ���ùܽ�3ΪHIGH
    HAL_GPIO_WritePin(GPIOB, Jb3_Pin, GPIO_PIN_RESET);
    printf("LOW HIGH\n");
    HAL_Delay(time);

    // ��ɫ1�� + ��ɫ2�� ���γɻ��ɫ��
    // HAL_GPIO_WritePin(GPIOB, LEDb13_Pin, GPIO_PIN_SET); // ���ùܽ�1ΪHIGH
    // HAL_GPIO_WritePin(GPIOB, LEDb14_Pin, GPIO_PIN_SET); // ���ùܽ�3ΪHIGH
    // printf("HIGH HIGH\n");
    // HAL_Delay(time);
  }
}
