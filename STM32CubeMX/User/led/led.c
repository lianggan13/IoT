
#include "led.h"

void Set_Green()
{
  HAL_GPIO_WritePin(GPIOB, LEDb13_Pin, GPIO_PIN_SET);   // 设置管脚1为HIGH
  HAL_GPIO_WritePin(GPIOB, LEDb14_Pin, GPIO_PIN_RESET); // 设置管脚3为LOW
  HAL_GPIO_WritePin(GPIOB, Jb3_Pin, GPIO_PIN_RESET);    // 继电器断开
}

void Set_Red()
{
  HAL_GPIO_WritePin(GPIOB, LEDb13_Pin, GPIO_PIN_RESET); // 设置管脚1为LOW
  HAL_GPIO_WritePin(GPIOB, LEDb14_Pin, GPIO_PIN_SET);   // 设置管脚3为HIGH
  HAL_GPIO_WritePin(GPIOB, Jb3_Pin, GPIO_PIN_SET);      // 继电器闭合
}

void Set_Off()
{
  HAL_GPIO_WritePin(GPIOB, LEDb13_Pin, GPIO_PIN_RESET); // 设置管脚1为LOW
  HAL_GPIO_WritePin(GPIOB, LEDb14_Pin, GPIO_PIN_RESET); // 设置管脚3为LOW
  HAL_GPIO_WritePin(GPIOB, Jb3_Pin, GPIO_PIN_RESET);    // 继电器断开
}

void test_led()
{
  int time = 5000;
  while (1)
  {
    // 熄灭
    // Set_Off();
    // printf("LOW LOW\n");
    // HAL_Delay(time);

    // 颜色1亮
    Set_Green();

    printf("HIGH LOW\n");
    HAL_Delay(time);

    // 颜色2亮
    Set_Red();
    printf("LOW HIGH\n");
    HAL_Delay(time);

    // 颜色1亮 + 颜色2亮 （形成混合色）
    // HAL_GPIO_WritePin(GPIOB, LEDb13_Pin, GPIO_PIN_SET); // 设置管脚1为HIGH
    // HAL_GPIO_WritePin(GPIOB, LEDb14_Pin, GPIO_PIN_SET); // 设置管脚3为HIGH
    // printf("HIGH HIGH\n");
    // HAL_Delay(time);
  }
}
