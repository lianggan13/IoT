#include "time_handle.h"
#include "main.h"
#include "tim.h"
#include <stdio.h>

/*
*************************************
宏定义
*************************************
*/
/*
*************************************
宏定义
*************************************
*/
/*
*************************************
变量声明
*************************************
*/
/*
*************************************
变量定义
*************************************
*/
static uint16_t index_10ms = 0;
uint16_t index_led = 0;
uint16_t index_oled = 0;
uint16_t index_dht11 = 0;

/**
 * @brief          定时器溢出回调函数
 * @param[in]      htim:定时器
 * @retval         none
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == htim2.Instance)
  {
    // 处理 TIM2 的溢出事件,10ms一次回调
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
    // 处理 TIM3 的溢出事件,5ms一次回调
    TIM3_PeriodElapsedCallback();
  }
}

// 启动 TIM2 中断
void TIM2_Start(void)
{
  HAL_TIM_Base_Start_IT(&htim2); // 打开定时器2中断
}

// 启动 TIM3 中断
void TIM3_Start(void)
{
  HAL_TIM_Base_Start_IT(&htim3); // 启动 TIM3 的中断
}

// 重置 TIM3 计数
void TIM3_Reset(void)
{
  __HAL_TIM_SET_COUNTER(&htim3, 0); // 将计数器重置为 0
  __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_UPDATE);
}

// 停止 TIM3 中断
void TIM3_Stop(void)
{
  HAL_TIM_Base_Stop_IT(&htim3); // 停止 TIM3 的中断
}

// 检查 TIM3 是否产生了更新中断
uint8_t TIM3_IsUpdateInterrupt(void)
{
  // 检查 TIM3 更新中断标志
  if (__HAL_TIM_GET_FLAG(&htim3, TIM_FLAG_UPDATE) != RESET)
  {
    // 检查中断是否被清除
    if (__HAL_TIM_GET_IT_SOURCE(&htim3, TIM_IT_UPDATE) != RESET)
    {
      return 1; // 中断已产生
    }
  }
  return 0; // 中断未产生
}
