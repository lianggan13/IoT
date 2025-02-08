#include "main_program.h"
#include "time_handle.h"
#include "led.h"
// #include "oled.h"
// #include "dht11.h"
#include "esp8266.h"
#include "usart.h"
#include "flash.h"
#include <stdio.h>

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
extern uint16_t index_led;
extern uint16_t index_oled;
extern uint16_t index_dht11;
/*
*************************************
函数声明
*************************************
*/

/*
*************************************
变量定义
*************************************
*/
uint8_t led_status = 0;
uint8_t temp_value = 0;
uint8_t humi_value = 0;
/*
*************************************
函数定义
*************************************
*/

/**
 * @brief          初始化函数,相当于Arduino的setup()函数,只在初始的时候执行一次
 * @param[in]      none
 * @retval         none
 */
void user_init_program(void)
{
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE); // 打开串口2接收中断
  __HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE); // 打开串口3接收中断

  // test_flash();
  test_led();
  esp8266_init();
  // OLED_init();
  // while(dht11_init())
  // {
  //   HAL_Delay(500);
  //   printf("dht11 init faild\r\n");
  //   OLED_printf (0,0,"dht11 init faild");
  // }
  // OLED_printf (0,0,"  Alibaba Cloud IOT ");
  TIM2_Start();
}
/**
 * @brief          初始化函数,相当于Arduino的loop()函数,一直执行该函数
 * @param[in]      none
 * @retval         none
 */
void user_main_program(void)
{

  // if(index_led==1)
  // {
  //   led_status = !led_status;
  //   set_led(led_status);
  //   printf("hello,world\r\n");
  //   index_led=0;
  // }
  // if(index_dht11 ==1)
  // {
  //   dht11_read_data(&temp_value,&humi_value);
  //   index_dht11=0;
  // }

  // if(index_oled==1)
  // {
  //   OLED_printf (2,0,"temp:%d",temp_value);
  //   OLED_printf (4,0,"humi:%d",humi_value);
  //   index_oled=0;
  // }
}
