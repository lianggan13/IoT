#include "main_program.h"
#include "time_handle.h"
#include "led.h"
// #include "oled.h"
// #include "dht11.h"

#include "usart.h"
#include "flash.h"
#include "esp8266.h"
#include "mqtt.h"


/**
 * @brief          初始化函数,相当于Arduino的setup()函数,只在初始的时候执行一次
 * @param[in]      none
 * @retval         none
 */
void user_init_program(void)
{
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE); // 打开串口2接收中断
  __HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE); // 打开串口3接收中断

  esp8266_init();

  mqtt_init(&esp8266, 0);
}
/**
 * @brief          初始化函数,相当于Arduino的loop()函数,一直执行该函数
 * @param[in]      none
 * @retval         none
 */
void user_main_program(void)
{
  mqtt_start();
}
