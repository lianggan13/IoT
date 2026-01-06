#include "esp8266.h"
#include "syn6288.h"
#include "usart.h"
#include "time_handle.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #include "oled.h"
#include "mqtt.h"
#include "led.h"
#include "flash.h"
#include "core_json.h"
#include "utility.h"

#define RN "\r\n"
#define OK "OK"

uint8_t usart2_buf[512];            // 串口2接收缓存数组
volatile uint16_t usart2_count = 0; // 串口2接收数据计数器
static volatile uint8_t it;         // 定时器3中断标志

void esp8266_send_cmd(char *cmd, uint32_t timout);

void TIM3_PeriodElapsedCallback(void)
{
  it = 1; // TIM3_IsUpdateInterrupt();
  TIM3_Reset();
  TIM3_Stop();
}

void uart2_clear_recv(uint16_t len)
{
  uint16_t safeLen = len;
  if (safeLen > sizeof(usart2_buf))
    safeLen = sizeof(usart2_buf);

  if (safeLen > 0)
  {
    // 清除前 打印接收缓冲区中的数据
    printf(">> %.*s\n", (int)safeLen, (const char *)usart2_buf);
  }

  memset((void *)usart2_buf, 0x00, sizeof(usart2_buf));
  usart2_count = 0;
  it = 0;
}

void HAL_UART2_IRQHandler(void)
{
  uint8_t receive_data = 0;
  if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE) != RESET)
  {
    HAL_UART_Receive(&huart2, &receive_data, 1, 1000); // 串口2接收1位数据

    TIM3_Reset(); // 计数器重置计数
    if (usart2_count == 0)
    {
      TIM3_Start(); // 使能定时器3的中断 (如果这是第一次接收数据，则启用计时器用于超时处理)
    }

    // usart2_buf[usart2_count++] = receive_data;
    if (usart2_count < (sizeof(usart2_buf) - 1))
    {
      usart2_buf[usart2_count++] = receive_data;
      usart2_buf[usart2_count] = '\0'; // 保证 '\0' 结尾
    }
    else
    {
      // 缓冲区满：丢弃后续字节，避免越界
      usart2_buf[sizeof(usart2_buf) - 1] = '\0';
    }
  }
}

uint8_t uart2_wait_recv(uint32_t timout)
{
  if (it)
    return 1;

  if (timout == 0)
  {
    while (!it)
      HAL_Delay(1);
    return 1;
  }

  uint32_t start = HAL_GetTick();
  while (!it)
  {
    if ((HAL_GetTick() - start) >= timout)
      return 0;
    HAL_Delay(1);
  }
  return 1;
}

void uart2_send(uint8_t *cmd, uint8_t len, uint32_t timout)
{
  // if (len > 1)
  //   printf("<< %s\n", cmd);
  // else
  //   printf("<< %c\n", cmd[0]);

  printf("<< %.*s\n", (int)len, (const char *)cmd);

  HAL_UART_Transmit(&huart2, cmd, len, timout);

  // while ((usart2_count == 0) || (count < timout))
  // {
  //   count++;
  //   APP_Delay(1);
  // }
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

  // uint8_t data[1] = {receivedByte};
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

#define PUB_TOPIC2 "/sys/a1TGt6tIcAE/mqtt_stm32/thing/event/property/post"
#define JSON_FORMAT "{\\\"params\\\":{\\\"temp\\\":%d\\,\\\"humi\\\":%d\\}\\,\\\"version\\\":\\\"1.0.0\\\"}"

static const char *PUB_TOPIC = "/sys/k21juUUmcsq/stm32/thing/event/property/post";

static void handle_aliyun_property_set(MqttClient *client, const char *json, size_t jsonLen)
{
  if (JSON_Validate(json, jsonLen) != JSONSuccess)
    return;

  const char *val = NULL;
  size_t valLen = 0;
  JSONTypes_t type;

  if (JSON_SearchConst(json, jsonLen, "params.led", strlen("params.led"), &val, &valLen, &type) == JSONSuccess && type == JSONNumber)
  {
    int led = parse_int_n(val, valLen);

    char payload[128];
    snprintf(payload, sizeof(payload), "{\\\"params\\\":{\\\"led\\\":%d\\}\\,\\\"version\\\":\\\"1.0.0\\\"}", led);

    if (led == 1)
      Set_Red();
    else if (led == 0)
      Set_Green();
    else
      Set_Off();

    client->publish(client, PUB_TOPIC, payload, 0, 0, 3000);
  }

  if (JSON_SearchConst(json, jsonLen, "params.voice", strlen("params.voice"), &val, &valLen, &type) == JSONSuccess && type == JSONString)
  {
    char voice[80];
    if (valLen >= sizeof(voice))
      valLen = sizeof(voice) - 1;
    memcpy(voice, val, valLen);
    voice[valLen] = '\0';

    syn6288_send_cmd((uint8_t *)voice);

    char payload[200];
    snprintf(payload, sizeof(payload), "{\\\"params\\\":{\\\"voice\\\":\\\"%s\\\"\\}\\,\\\"version\\\":\\\"1.0.0\\\"}", voice);
    client->publish(client, PUB_TOPIC, payload, 0, 0, 3000);
  }
}

void mqtt_start(void)
{
  // AliYun:  https://help.aliyun.com/zh/iot/product-overview/limits | https://developer.aliyun.com/article/1163947
  // AT+MQTT: https://blog.csdn.net/espressif/article/details/101713780

  // Step 1: 设置 MQTT 用户信息
  // AT+MQTTUSERCFG=<LinkID>,<scheme>,<"client_id">,<"username">,<"password">,<cert_key_ID>,<CA_ID>,<"path">
  // client_id: 对应 MQTT client ID, 用于标志 client 身份, 最长 256 字节
  // username: 用于登录 MQTT broker 的 username, 最长 64 字节
  // password: 用于登录 MQTT broker 的 password, 最长 64 字节
  // cert_key_ID: 证书 ID, 目前支持一套 cert 证书, 参数为 0
  // CA_ID: CA ID, 目前支持一套 CA 证书, 参数为 0
  // path: 资源路径, 最长 32 字节
  MqttClient client;
  mqtt_client_init(&client, 0, esp8266_send_cmd);

  const char *client_id = "k21juUUmcsq.stm32|securemode=2\\,signmethod=hmacsha256\\,timestamp=1735605362073|";
  const char *username = "stm32&k21juUUmcsq";
  const char *password = "bb60e378712ebacff26c316d7e2c695b1723626b9162a27625907d1eb4f5b037";
  client.config(&client, 1, client_id, username, password, 0, 0, "", 3000);

  // Step 2: 连接 MQTT Broker
  // AT+MQTTCONN=<LinkID>,<"host">,<port>,<reconnect>
  // LinkID: 当前只支持 0
  // host: 连接 MQTT broker 域名, 最大 128 字节
  // port: 连接 MQTT broker 端口, 最大 65535
  // path: 资源路径, 最长 32 字节
  // reconnect: 是否重连 MQTT, 若设置为 1, 需要消耗较多内存资源
  const char *broker_addr = "k21juUUmcsq.iot-as-mqtt.cn-shanghai.aliyuncs.com";
  client.connect(&client, broker_addr, 1883, 1, 3000);

  // Step 3: 订阅主题
  // AT+MQTTSUB=<LinkID>,<"topic">,<qos>
  // LinkID: 当前只支持 0
  // state: MQTT 当前状态, 状态说明如下:
  // 0: 连接未初始化
  // 1: 已设置 MQTTUSERCFG
  // 2: 已设置 MQTTCONNCFG
  // 3: 连接已断开
  // 4: 已建立连接
  const char *SUB_TOPIC = "/sys/k21juUUmcsq/stm32/thing/service/property/set";
  client.subscribe(&client, SUB_TOPIC, 0, 3000);

  char mqtt_buf[256];
  memset(mqtt_buf, 0x00, sizeof(mqtt_buf));

  while (true)
  {
    if (uart2_wait_recv(0) == 0)
      continue;

    size_t copyLen = usart2_count;
    if (copyLen >= sizeof(mqtt_buf))
      copyLen = sizeof(mqtt_buf) - 1;
    memset(mqtt_buf, 0, sizeof(mqtt_buf));
    memcpy(mqtt_buf, usart2_buf, copyLen);
    mqtt_buf[copyLen] = '\0';

    uart2_clear_recv(usart2_count);

    MqttSubRecv sub;
    if (client.parseSubRecv(mqtt_buf, strlen(mqtt_buf), &sub))
    {
      if (topic_equals(sub.topic, sub.topicLen, SUB_TOPIC))
      {
        handle_aliyun_property_set(&client, sub.json, sub.jsonLen);
      }
      else
      {
        printf("[MQTT] Unknown topic: %.*s\n", (int)sub.topicLen, sub.topic);
      }
    }
    else
    {
      printf("[MQTT] Invalid recv: %s\n", mqtt_buf);
      // +MQTTDISCONNECTED:0
    }
  }
}

// 5: 已连接, 但未订阅 topic
// 6: 已连接, 已订阅过 topic
// topic*: 订阅过的主题
// qos: 订阅过的 QoS
// "AT+MQTTSUB=0,\"/sys/a1THAR4Jl2A/mqtt_stm32/thing/service/property/set\",0\r\n"

void esp8266_init(void)
{
  char wifi[128];
  char tcp[64];
  uint8_t pass = 0;

  Flash_Read(PAGE_ADDRESS(31), (uint32_t *)wifi, sizeof(wifi)); // "AT+CWJAP=\"LG13_TPLink_2G\",\"G15608212470*\""
  Flash_Read(PAGE_ADDRESS(32), (uint32_t *)tcp, sizeof(tcp));   // "AT+CIPSTART=\"TCP\",\"

  // sta --> connect wifi --> connect tcp --> mqtt
  esp8266_sta();
  for (size_t i = 0; i < 5; i++)
  {
    pass = esp8266_connect_wifi(wifi);
    if (pass)
    {
      printf("Connect WiFi success!\n");
      break;
    }
    else
    {
      HAL_Delay(1000);
      printf("Connect WiFi failed, try again...\n");
    }
  }

  mqtt_start();

  if (pass)
    pass = esp8266_connect_tcp(tcp);

  if (pass == 0)
  {
    // 失败
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

    // uint8_t *syn_buf = (uint8_t *)malloc(usart2_count * sizeof(uint8_t)); // 动态分配内存
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
