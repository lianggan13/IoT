
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
    // 熄灭
    // HAL_GPIO_WritePin(GPIOB, LEDb13_Pin, GPIO_PIN_RESET); // 设置管脚1为LOW
    // HAL_GPIO_WritePin(GPIOB, LEDb14_Pin, GPIO_PIN_RESET); // 设置管脚3为LOW
    // printf("LOW LOW\n");
    // HAL_Delay(time);

    // 颜色1亮
    HAL_GPIO_WritePin(GPIOB, LEDb13_Pin, GPIO_PIN_SET);   // 设置管脚1为HIGH
    HAL_GPIO_WritePin(GPIOB, LEDb14_Pin, GPIO_PIN_RESET); // 设置管脚3为LOW
    HAL_GPIO_WritePin(GPIOB, Jb3_Pin, GPIO_PIN_SET);
    printf("HIGH LOW\n");
    HAL_Delay(time);

    // 颜色2亮
    HAL_GPIO_WritePin(GPIOB, LEDb13_Pin, GPIO_PIN_RESET); // 设置管脚1为LOW
    HAL_GPIO_WritePin(GPIOB, LEDb14_Pin, GPIO_PIN_SET);   // 设置管脚3为HIGH
    HAL_GPIO_WritePin(GPIOB, Jb3_Pin, GPIO_PIN_RESET);
    printf("LOW HIGH\n");
    HAL_Delay(time);

    // 颜色1亮 + 颜色2亮 （形成混合色）
    // HAL_GPIO_WritePin(GPIOB, LEDb13_Pin, GPIO_PIN_SET); // 设置管脚1为HIGH
    // HAL_GPIO_WritePin(GPIOB, LEDb14_Pin, GPIO_PIN_SET); // 设置管脚3为HIGH
    // printf("HIGH HIGH\n");
    // HAL_Delay(time);
  }
}
