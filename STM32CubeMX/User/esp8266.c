#include "esp8266.h"
#include "syn6288.h"
#include "usart.h"
#include "time_handle.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #include "oled.h"
#include "flash.h"
#include "core_json.h"
#include "utility.h"

#define RN "\r\n"
#define OK "OK"

uint8_t usart2_buf[512];   // ����2���ջ�������
uint16_t usart2_count = 0; // ����2�������ݼ�����
static uint8_t led_status;

static uint8_t it;

void TIM3_PeriodElapsedCallback(void)
{
  it = 1; // TIM3_IsUpdateInterrupt();
  TIM3_Reset();
  TIM3_Stop();
}

void uart2_clear_recv(uint16_t len)
{
  if (len > 0)
  {
    printf(">> %s\n", usart2_buf); // ���ǰ ��ӡ���ջ������е�����
  }

  memset(usart2_buf, 0x00, len);
  usart2_count = 0;
  it = 0;
}

void HAL_UART2_IRQHandler(void)
{
  uint8_t receive_data = 0;
  if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE) != RESET)
  {
    HAL_UART_Receive(&huart2, &receive_data, 1, 1000); // ����2����1λ����

    TIM3_Reset(); // ���������ü���
    if (usart2_count == 0)
    {
      TIM3_Start(); // ʹ�ܶ�ʱ��3���ж� (������ǵ�һ�ν������ݣ������ü�ʱ�����ڳ�ʱ����)
    }

    usart2_buf[usart2_count++] = receive_data;
  }
}

void uart2_send(uint8_t *cmd, uint8_t len, uint32_t timout)
{
  if (len > 1)
    printf("<< %s\n", cmd);
  else
    printf("<< %c\n", cmd[0]);

  HAL_UART_Transmit(&huart2, cmd, len, timout);

  // while ((usart2_count == 0) || (count < timout))
  // {
  //   count++;
  //   APP_Delay(1);
  // }
}

uint8_t uart2_wait_recv(uint32_t timout)
{
  uint16_t count = 0;
  HAL_Delay(timout);
  do
  {
    count++;
    HAL_Delay(1);
    if (it)
    {
      count = 0;
      break;
    }
  } while (count < timout || timout == 0);
  return count;
}

uint8_t check_cmd(char *rec_data, uint32_t timout)
{
  uint8_t retval = 0;

  uart2_wait_recv(timout);

  if (strstr((const char *)usart2_buf, rec_data))
  {
    retval = 1;
  }
  else
  {
    retval = 0;
  }

  uart2_clear_recv(usart2_count);
  return retval;
}

void esp8266_send_cmd(char *cmd, uint32_t timout)
{
  char buffer[256];
  snprintf(buffer, sizeof(buffer), "%s%s", cmd, RN);
  do
  {
    uart2_send((uint8_t *)buffer, strlen(buffer), timout);
  } while (check_cmd(OK, timout) == 0);
}

uint8_t esp8266_try_send_cmd(char *cmd, uint32_t timout, int8_t count)
{
  char buffer[128];
  snprintf(buffer, sizeof(buffer), "%s%s", cmd, RN);
  do
  {
    uart2_send((uint8_t *)buffer, strlen(buffer), timout);
    uint16_t count2 = 0;
    while ((usart2_count == 0) || (count2++ < timout))
    {
      count2++;
      APP_Delay(1);
    }

    HAL_Delay(5000);
    if (count-- < 0)
      return 0;
  } while (check_cmd(OK, timout) == 0);
  return 1;
}

uint8_t esp8266_board_msg(char *msg)
{
  char buffer[128];
  snprintf(buffer, sizeof(buffer), "AT+CIPSEND=%d,%d", 0, strlen(msg)); // AT+CIPSEND=0,2
  esp8266_send_cmd((char *)buffer, 1000);

  // uart2_wait_recv(500);
  // uart2_clear_recv(usart2_count);
  uart2_send((uint8_t *)buffer, strlen(buffer), 1500);

  return 0;
}

uint8_t esp8266_parse_cmd(const char *key, char *cmd)
{
  if (it == 1)
  {
    char *found = strstr((const char *)usart2_buf, key);
    if (found != NULL)
    {
      strncpy(cmd, found, strlen(found) + 1);
      uart2_clear_recv(usart2_count);
      return strlen(cmd);
    }
  }

  HAL_Delay(10);
  return 0;
}

void SYN6288_ReceivedData(uint8_t receivedByte)
{
  printf("SYN6288 >> %c\n", receivedByte);

  uint8_t data[1] = {receivedByte};

  // uart2_send(data, sizeof(data), 1000);
}

void esp8266_reset()
{
  const char *quit = "+++";
  uart2_clear_recv(usart2_count);

  uart2_send((uint8_t *)quit, strlen(quit), 300);

  uart2_wait_recv(300);
  uart2_clear_recv(usart2_count);

  esp8266_send_cmd("AT+RST", 3000);

  esp8266_send_cmd("AT+CIPMODE=0", 1000);
}

uint8_t esp8266_connect_tcp(char *tcp)
{
  uint8_t pass = esp8266_try_send_cmd(tcp, 5000, 5); // AT+CIPSTART="TCP","192.168.0.107",1918
  if (pass)
  {
    esp8266_send_cmd("AT+CIPMODE=1", 1000);
    esp8266_send_cmd("AT+CIPSEND", 1000);
  }
  return pass;
}

uint8_t esp8266_connect_wifi(char *wifi)
{
  // esp8266_send_cmd("AT+CWAUTOCONN=0", 1000);
  // esp8266_send_cmd("AT+CIPCLOSE", 1000);

  HAL_Delay(2500);
  uint8_t pass = esp8266_try_send_cmd(wifi, 8000, 5); // AT+CWJAP="LG13_TPLink_2G","G15608212470*"\r\n
  if (pass)
    esp8266_send_cmd("AT+CWAUTOCONN=1", 1000); // AT+CWAUTOCONN=1\r\n

  HAL_Delay(10);
  return pass;
}

void esp8266_sta(void)
{
  esp8266_reset();

  esp8266_send_cmd("AT+CWMODE=1", 1000);

  esp8266_send_cmd("AT+CWQAP", 1000);
}

void esp8266_set_wifi(const char *ssid, const char *pwd, uint32_t timout)
{
  char buffer[128];
  snprintf(buffer, sizeof(buffer), "AT+CWSAP=\"%s\",\"%s\",%d,%d,%d%s", ssid, pwd, 1, 3, 4, RN);

  // uart2_send((uint8_t *)buffer, strlen(buffer), timout);
  esp8266_send_cmd(buffer, timout);
  HAL_Delay(timout);
}

void esp8266_ap(void)
{
  // AP+STA
  esp8266_reset();

  esp8266_send_cmd("AT+RST", 1000);
  esp8266_send_cmd("AT+CWMODE=2", 1000); // AT+CWMODE=1\r\n

  esp8266_send_cmd("AT+CIPSTATUS", 1000); // AT+CIPMUX=1

  esp8266_send_cmd("AT+CWQAP", 1000); //  AT+CIPMUX=1 --> link is builded

  esp8266_send_cmd("AT+CIPMUX=1", 1000); // AT+CIPMUX=1

  esp8266_set_wifi("Lianggan13_ESP8266", "G15608212470*", 3000); // AT+CWSAP="LG13_ESP8266","G15608212470*",1,3,4

  esp8266_send_cmd("AT+CIPAP?", 3000); // AT+CIPAP?

  esp8266_send_cmd("AT+CIPSERVER=1,8080", 1000); // AT+CIPSERVER=1,8080
}

void parse_json(const char *json, const char *start, const char *end, char *buf)
{

  const char *start_index = strstr(json, start);
  if (start_index)
  {
    start_index += strlen(start);
    // const char *chars[] = {"\"", "}", ","};
    // int num_chars = sizeof(chars) / sizeof(chars[0]);

    // printf("chars[%d] = %s method_start=%s\n", end, start_index);

    if (strcmp(end, "\"") == 0)
      start_index++;

    const char *end_index = strstr(start_index, end);
    if (end_index)
    {
      size_t length = end_index - start_index;
      // printf("ength : %d method_end=[%s]\n", length, end_index);
      strncpy(buf, start_index, length);
      buf[length] = '\0';
    }
  }
}
#define PUB_TOPIC2 "/sys/a1TGt6tIcAE/mqtt_stm32/thing/event/property/post"
#define JSON_FORMAT "{\\\"params\\\":{\\\"temp\\\":%d\\,\\\"humi\\\":%d\\}\\,\\\"version\\\":\\\"1.0.0\\\"}"
const char *SUB_TOPIC = "/sys/k21juUUmcsq/stm32/thing/service/property/set";
const char *PUB_TOPIC = "/sys/k21juUUmcsq/stm32/thing/event/property/post";

void mqtt_test(void)
{
  return;
  char mqtt_buf2[256];
  snprintf(mqtt_buf2, sizeof(mqtt_buf2), "AT+MQTTPUB=0,\"%s\",\"{\\\"params\\\":{\\\"led\\\":%d\\}\\,\\\"version\\\":\\\"1.0.0\\\"}\",%d,%d", PUB_TOPIC, 8, 0, 0);
  printf("%s\n", mqtt_buf2);

  // ClearArray(mqtt_buf2, sizeof(mqtt_buf2));
  memset(mqtt_buf2, 0x00, sizeof(mqtt_buf2));
  snprintf((char *)mqtt_buf2, sizeof(mqtt_buf2), "AT+MQTTPUB=0,\"" PUB_TOPIC2 "\",\"" JSON_FORMAT "\",0,0\r\n", 8, 8);
  printf("%s\n", mqtt_buf2);

  while (true)
  {
  }
}

void mqtt_start(void)
{
  // AliYun:  https://help.aliyun.com/zh/iot/product-overview/limits | https://developer.aliyun.com/article/1163947
  // AT+MQTT: https://blog.csdn.net/espressif/article/details/101713780

  // AT+MQTTUSERCFG=<LinkID>,<scheme>,<"client_id">,<"username">,<"password">,<cert_key_ID>,<CA_ID>,<"path">
  // client_id: ��Ӧ MQTT client ID, ���ڱ�־ client ���, � 256 �ֽ�
  // username: ���ڵ�¼ MQTT broker �� username, � 64 �ֽ�
  // password: ���ڵ�¼ MQTT broker �� password, � 64 �ֽ�
  // cert_key_ID: ֤�� ID, Ŀǰ֧��һ�� cert ֤��, ����Ϊ 0
  // CA_ID: CA ID, Ŀǰ֧��һ�� CA ֤��, ����Ϊ 0
  // path: ��Դ·��, � 32 �ֽ�
  const char *client_id = "k21juUUmcsq.stm32|securemode=2\\,signmethod=hmacsha256\\,timestamp=1735605362073|";
  const char *username = "stm32&k21juUUmcsq";
  const char *password = "bb60e378712ebacff26c316d7e2c695b1723626b9162a27625907d1eb4f5b037";
  char mqtt_buf[256];
  snprintf(mqtt_buf, sizeof(mqtt_buf), "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",%d,%d,\"%s\"", client_id, username, password, 0, 0, "");
  esp8266_send_cmd(mqtt_buf, 3000);

  memset(mqtt_buf, 0x00, sizeof(mqtt_buf));

  // AT+MQTTCONN=<LinkID>,<"host">,<port>,<reconnect>
  // LinkID: ��ǰֻ֧�� 0
  // host: ���� MQTT broker ����, ��� 128 �ֽ�
  // port: ���� MQTT broker �˿�, ��� 65535
  // path: ��Դ·��, � 32 �ֽ�
  // reconnect: �Ƿ����� MQTT, ������Ϊ 1, ��Ҫ���Ľ϶��ڴ���Դ
  const char *broker_addr = "k21juUUmcsq.iot-as-mqtt.cn-shanghai.aliyuncs.com";
  snprintf(mqtt_buf, sizeof(mqtt_buf), "AT+MQTTCONN=0,\"%s\",%d,%d", broker_addr, 1883, 0);
  esp8266_send_cmd(mqtt_buf, 3000);

  memset(mqtt_buf, 0x00, sizeof(mqtt_buf));

  // AT+MQTTSUB=<LinkID>,<"topic">,<qos>
  // LinkID: ��ǰֻ֧�� 0
  // state: MQTT ��ǰ״̬, ״̬˵������:
  // 0: ����δ��ʼ��
  // 1: ������ MQTTUSERCFG
  // 2: ������ MQTTCONNCFG
  // 3: �����ѶϿ�
  // 4: �ѽ�������
  // 5: ������, ��δ���� topic
  // 6: ������, �Ѷ��Ĺ� topic
  // topic*: ���Ĺ�������
  // qos: ���Ĺ��� QoS
  // "AT+MQTTSUB=0,\"/sys/a1THAR4Jl2A/mqtt_stm32/thing/service/property/set\",0\r\n"
  const char *topic = "/sys/k21juUUmcsq/stm32/thing/service/property/set";
  snprintf(mqtt_buf, sizeof(mqtt_buf), "AT+MQTTSUB=0,\"%s\",%d", topic, 0);
  esp8266_send_cmd(mqtt_buf, 3000);

  memset(mqtt_buf, 0x00, sizeof(mqtt_buf));

  while (true)
  {
    uart2_wait_recv(0);

    ClearArray(mqtt_buf, sizeof(mqtt_buf));
    CopyArray(mqtt_buf, (const uint8_t *)usart2_buf, usart2_count);

    uart2_clear_recv(usart2_count);

    // +MQTTSUBRECV:0,"/sys/k21juUUmcsq/stm32/thing/service/property/set",93,{"method":"thing.service.property.set","id":"417480659","params":{"led":1},"version":"1.0.0"}
    // TODO: topic �ַ����Ƚ�
    if (strstr((const char *)mqtt_buf, "+MQTTSUBRECV:"))
    {
      int msg_len = 0;
      uint8_t json[128] = {0};

      {
        char *params = strstr(mqtt_buf, "params");
        if (params != NULL)
        {
          if (strstr(params, "led"))
          {
            char buf[50] = {0};
            parse_json(params, "\"led\":", "}", buf);
            // printf("result : %s\n", buf);

            // AT+MQTTPUB=<LinkID>,<"topic">,<"data">,<qos>,<retain>
            // LinkID: ��ǰֻ֧�� 0
            // topic: ��������, � 64 �ֽ�
            // data: ������Ϣ, data ���ܰ��� \0, ��ȷ������ AT+MQTTPUB ������ AT ָ�����󳤶�����
            // qos: ������������, ������ѡ 0,1,2, Ĭ��Ϊ 0
            // retain: ���� retain

            int led = atoi(buf);
            memset(mqtt_buf, 0x00, sizeof(mqtt_buf));
            snprintf(mqtt_buf, sizeof(mqtt_buf), "AT+MQTTPUB=0,\"%s\",\"{\\\"params\\\":{\\\"led\\\":%d\\}\\,\\\"version\\\":\\\"1.0.0\\\"}\",%d,%d", PUB_TOPIC, led, 0, 0);
            esp8266_send_cmd(mqtt_buf, 3000);
          }

          if (strstr(params, "voice"))
          {
            char buf[50] = {0};
            parse_json(params, "\"voice\":", "\"", buf);
            // printf("result : %s\n", buf);

            syn6288_send_cmd(buf);

            memset(mqtt_buf, 0x00, sizeof(mqtt_buf));
            snprintf(mqtt_buf, sizeof(mqtt_buf), "AT+MQTTPUB=0,\"%s\",\"{\\\"params\\\":{\\\"voice\\\":\\\"%s\\\"\\}\\,\\\"version\\\":\\\"1.0.0\\\"}\",%d,%d", PUB_TOPIC, buf, 0, 0);
            esp8266_send_cmd(mqtt_buf, 3000);
          }
        }
      }
    }
  }
}

void esp8266_init(void)
{
  mqtt_test();

  char wifi[128];
  char tcp[64];

  Flash_Read(PAGE_ADDRESS(31), (uint32_t *)wifi, sizeof(wifi));
  Flash_Read(PAGE_ADDRESS(32), (uint32_t *)tcp, sizeof(tcp));

  esp8266_sta();
  uint8_t pass = esp8266_connect_wifi(wifi);
  // mqtt...
  mqtt_start();

  while (true)
  {
  }

  if (pass)
    pass = esp8266_connect_tcp(tcp);

  if (pass == 0)
  {
    // ʧ��
    while (true)
    {
      esp8266_ap();

      memset(wifi, 0, sizeof(wifi));
      memset(tcp, 0, sizeof(tcp));
      while (strlen(wifi) == 0 || strlen(tcp) == 0)
      {
        if (strlen(wifi) == 0)
        {
          if (esp8266_parse_cmd("AT+CWJAP", wifi) > 0)
          {
            esp8266_board_msg("OK");
            HAL_Delay(500);
          }
        }

        if (strlen(tcp) == 0)
        {
          if (esp8266_parse_cmd("AT+CIPSTART", tcp) > 0)
          {
            esp8266_board_msg("OK");
            HAL_Delay(500);
          }
        }
      }

      esp8266_sta();
      pass = esp8266_connect_wifi(wifi);
      if (pass)
      {
        pass = esp8266_connect_tcp(tcp);
        if (pass)
        {
          Flash_Write(PAGE_ADDRESS(31), (uint32_t *)wifi, sizeof(wifi));
          Flash_Write(PAGE_ADDRESS(32), (uint32_t *)tcp, sizeof(tcp));
          break;
        }
      }
    }
  }

  // while (true)
  // {
  //   uart2_wait_recv(500);
  //   if (usart2_count > 0)
  //     uart2_clear_recv(usart2_count);

  //   const char *heart = "heart";
  //   uart2_send((uint8_t *)heart, strlen(heart), 10);
  //   // uart2_clear_recv(usart2_count);

  //   HAL_Delay(1000);
  // }

  while (true)
  {
    uart2_wait_recv(0);

    uint8_t syn_buf[512];
    memset(syn_buf, 0, sizeof(syn_buf));
    CopyArray(syn_buf, (const uint8_t *)usart2_buf, usart2_count);

    // uint8_t *syn_buf = (uint8_t *)malloc(usart2_count * sizeof(uint8_t)); // ��̬�����ڴ�
    // if (syn_buf == NULL)
    //   continue;
    // ClearArray(syn_buf, sizeof(syn_buf));
    // CopyArray(syn_buf, (const uint8_t *)usart2_buf, usart2_count);

    uart2_clear_recv(usart2_count);
    syn6288_send_cmd(syn_buf);

    // uart2_wait_recv(500);
    // uart2_clear_recv(usart2_count);

    // free(syn_buf);
  }
}
