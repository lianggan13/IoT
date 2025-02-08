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
�궨��
*************************************
*/

/*
*************************************
��������
*************************************
*/
extern uint16_t index_led;
extern uint16_t index_oled;
extern uint16_t index_dht11;
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
uint8_t led_status = 0;
uint8_t temp_value = 0;
uint8_t humi_value = 0;
/*
*************************************
��������
*************************************
*/

/**
 * @brief          ��ʼ������,�൱��Arduino��setup()����,ֻ�ڳ�ʼ��ʱ��ִ��һ��
 * @param[in]      none
 * @retval         none
 */
void user_init_program(void)
{
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE); // �򿪴���2�����ж�
  __HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE); // �򿪴���3�����ж�

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
 * @brief          ��ʼ������,�൱��Arduino��loop()����,һֱִ�иú���
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
