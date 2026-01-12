#include "esp8266.h"

#define RN "\r\n"
#define OK "OK"

Esp8266 esp8266;

// USART2 单字节中断接收缓冲
static uint8_t uart2_rx_byte = 0;

static void esp8266_uart2_start_rx_it(void)
{
  // 启动 1 字节中断接收；后续在 HAL_UART_RxCpltCallback 里重复启动
  (void)HAL_UART_Receive_IT(&huart2, &uart2_rx_byte, 1);
}

void TIM3_PeriodElapsedCallback(void)
{
  esp8266.it3 = true; // 表示本帧接收结束
  TIM3_Reset();
  TIM3_Stop();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    uint8_t receive_data = uart2_rx_byte;

    TIM3_Reset();
    if (esp8266.usart2_count == 0)
    {
      TIM3_Start();
    }

    if (esp8266.usart2_count < (sizeof(esp8266.usart2_buf) - 1))
    {
      esp8266.usart2_buf[esp8266.usart2_count++] = receive_data;
      esp8266.usart2_buf[esp8266.usart2_count] = '\0';
    }
    else
    {
      esp8266.usart2_buf[sizeof(esp8266.usart2_buf) - 1] = '\0';
    }

    // 继续接收下一个字节
    (void)HAL_UART_Receive_IT(&huart2, &uart2_rx_byte, 1);
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    // 处理溢出等错误，避免接收卡死
    __HAL_UART_CLEAR_OREFLAG(huart);
    (void)HAL_UART_Receive_IT(&huart2, &uart2_rx_byte, 1);
  }
}

void uart2_clear_recv(uint16_t len)
{
  uint16_t safeLen = len;
  if (safeLen > sizeof(esp8266.usart2_buf))
    safeLen = sizeof(esp8266.usart2_buf);

  if (safeLen > 0)
  {
    printf(">> %.*s\n", (int)safeLen, (const char *)esp8266.usart2_buf);
  }

  memset((void *)esp8266.usart2_buf, 0x00, sizeof(esp8266.usart2_buf));
  esp8266.usart2_count = 0;
  esp8266.it3 = false; // 表示本帧接收未开始，等待中
}

uint8_t uart2_wait_recv(uint32_t timout)
{
  if (esp8266.it3)
    return 1;

  if (timout == 0)
  {
    while (!esp8266.it3)
      HAL_Delay(1);
    return 1;
  }

  uint32_t start = HAL_GetTick();
  while (!esp8266.it3)
  {
    if ((HAL_GetTick() - start) >= timout)
      return 0;
    HAL_Delay(1);
  }
  return 1;
}

void uart2_send(uint8_t *cmd, uint8_t len, uint32_t timout)
{
  printf("<< %.*s\n", (int)len, (const char *)cmd);

  HAL_UART_Transmit(&huart2, cmd, len, timout);
}

static int uart2_sendf(uint32_t timeoutMs, const char *fmt, ...)
{
  // uart2_sendf(1000, "AT+CWSAP=\"%s\",\"%s\",%d,%d,%d%s", "Lianggan13_ESP8266", "G15608212470*", 1, 3, 4, RN);
  uint8_t buf[256];

  va_list ap;
  va_start(ap, fmt);
  int n = vsnprintf((char *)buf, sizeof(buf), fmt, ap);
  va_end(ap);

  if (n < 0)
    return -1;
  if ((size_t)n >= sizeof(buf))
    return -2; // 截断

  uart2_send(buf, (uint8_t)n, timeoutMs);
  return n;
}

uint8_t check_cmd(char *rec_data, uint32_t timout)
{
  uint8_t retval = 0;

  uart2_wait_recv(timout);

  if (strstr((const char *)esp8266.usart2_buf, rec_data))
  {
    retval = 1;
  }
  else
  {
    retval = 0;
  }

  uart2_clear_recv(esp8266.usart2_count);
  return retval;
}

void send_cmd(const char *cmd, uint32_t timout)
{
  char buffer[256];
  snprintf(buffer, sizeof(buffer), "%s%s", cmd, RN);
  do
  {
    uart2_send((uint8_t *)buffer, strlen(buffer), timout);
  } while (check_cmd(OK, timout) == 0);
}

uint8_t try_send_cmd(char *cmd, uint32_t timout, int8_t retries)
{
  char buffer[128];
  snprintf(buffer, sizeof(buffer), "%s%s", cmd, RN);
  do
  {
    uart2_send((uint8_t *)buffer, strlen(buffer), timout);

    uint32_t times = 0;
    while ((esp8266.usart2_count == 0) && (times < timout))
    {
      APP_Delay(1);
      times++;
    }

    HAL_Delay(5000);
    if (retries-- < 0)
      return 0;

  } while (check_cmd(OK, timout) == 0);

  return 1;
}

uint8_t tcp_send(char *msg)
{
  char buffer[128];
  snprintf(buffer, sizeof(buffer), "AT+CIPSEND=%d,%d", 0, strlen(msg)); // AT+CIPSEND=0,2
  send_cmd((char *)buffer, 1000);

  uart2_send((uint8_t *)buffer, strlen(buffer), 1500);

  return 0;
}

uint8_t parse_cmd(const char *key, char *cmd)
{
  if (esp8266.it3)
  {
    char *found = strstr((const char *)esp8266.usart2_buf, key);
    if (found != NULL)
    {
      strncpy(cmd, found, strlen(found) + 1);
      uart2_clear_recv(esp8266.usart2_count);
      return strlen(cmd);
    }
  }

  HAL_Delay(10);
  return 0;
}

void reset()
{
  const char *quit = "+++";
  uart2_clear_recv(esp8266.usart2_count);

  uart2_send((uint8_t *)quit, strlen(quit), 300);

  uart2_wait_recv(300);
  uart2_clear_recv(esp8266.usart2_count);

  send_cmd("AT+RST", 3000);

  send_cmd("AT+CIPMODE=0", 1000);
}

uint8_t connect_wifi(char *wifi)
{
  // send_cmd("AT+CWAUTOCONN=0", 1000);
  // send_cmd("AT+CIPCLOSE", 1000);

  HAL_Delay(2500);
  uint8_t pass = try_send_cmd(wifi, 8000, 5); // AT+CWJAP="LG13_TPLink_2G","G15608212470*"\r\n
  if (pass)
    send_cmd("AT+CWAUTOCONN=1", 1000); // AT+CWAUTOCONN=1\r\n

  HAL_Delay(10);
  return pass;
}

uint8_t connect_tcp(char *tcp)
{
  uint8_t pass = try_send_cmd(tcp, 5000, 5); // AT+CIPSTART="TCP","192.168.0.107",1918
  if (pass)
  {
    send_cmd("AT+CIPMODE=1", 1000);
    send_cmd("AT+CIPSEND", 1000);
  }
  return pass;
}

void set_sta(void)
{
  reset();

  send_cmd("AT+CWMODE=1", 1000);

  send_cmd("AT+CWQAP", 1000);
}

void set_wifi(const char *ssid, const char *pwd, uint32_t timout)
{
  char buffer[128];
  snprintf(buffer, sizeof(buffer), "AT+CWSAP=\"%s\",\"%s\",%d,%d,%d%s", ssid, pwd, 1, 3, 4, RN);

  // uart2_send((uint8_t *)buffer, strlen(buffer), timout);
  send_cmd(buffer, timout);

  HAL_Delay(timout);
}

void set_ap(void)
{
  reset();

  send_cmd("AT+RST", 1000);

  send_cmd("AT+CWMODE=2", 1000); // AT+CWMODE=1\r\n

  send_cmd("AT+CIPSTATUS", 1000); // AT+CIPMUX=1

  send_cmd("AT+CWQAP", 1000); //  AT+CIPMUX=1 --> link is builded

  send_cmd("AT+CIPMUX=1", 1000); // AT+CIPMUX=1

  set_wifi("Lianggan13_ESP8266", "G15608212470*", 3000); // AT+CWSAP="LG13_ESP8266","G15608212470*",1,3,4

  send_cmd("AT+CIPAP?", 3000); // AT+CIPAP?

  send_cmd("AT+CIPSERVER=1,8080", 1000); // AT+CIPSERVER=1,8080
}

void esp8266_init(void)
{
  esp8266.send = send_cmd;
  esp8266.waitRecv = uart2_wait_recv;
  esp8266.clearRecv = uart2_clear_recv;

  // 启动 USART2 的 1 字节中断接收
  esp8266_uart2_start_rx_it();

  set_sta(); // 设置为 STA 模式

  char wifi[128]; // "AT+CWJAP=\"LG13_TPLink_2G\",\"G15608212470*\""
  flash_readString(PAGE_ADDRESS(31), wifi, sizeof(wifi));

  for (size_t i = 0; i < 5; i++)
  {
    if (connect_wifi(wifi)) // 连接 wifi
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
}

void config_wifi(void)
{
  char wifi[128];
  char tcp[64];

  flash_readString(PAGE_ADDRESS(31), wifi, sizeof(wifi)); // "AT+CWJAP=\"LG13_TPLink_2G\",\"G15608212470*\""
  flash_readString(PAGE_ADDRESS(32), tcp, sizeof(tcp));   // "AT+CIPSTART=\"TCP\","

  // 如果没有连接上 wifi，就启动 AP 模式重新配置 wifi 参数
  // 客户端连接上 AP 后，通过网页配置 wifi 参数，然后保存到 flash 中

  uint8_t pass = connect_tcp(tcp);

  if (pass == 0)
  {
    // 失败则启动 AP 模式重新配置 wifi 参数
    while (true)
    {
      set_ap(); // 设置为 AP 模式

      // TODO: 等待客户端通过网页配置 wifi 参数，保存到 flash 中
      memset(wifi, 0, sizeof(wifi));
      memset(tcp, 0, sizeof(tcp));
      while (strlen(wifi) == 0 || strlen(tcp) == 0)
      {
        if (strlen(wifi) == 0)
        {
          if (parse_cmd("AT+CWJAP", wifi) > 0)
          {
            tcp_send("OK");
            HAL_Delay(500);
          }
        }

        if (strlen(tcp) == 0)
        {
          if (parse_cmd("AT+CIPSTART", tcp) > 0)
          {
            tcp_send("OK");
            HAL_Delay(500);
          }
        }
      }

      set_sta();
      pass = connect_wifi(wifi);
      if (pass)
      {
        pass = connect_tcp(tcp);
        if (pass)
        {
          flash_write(PAGE_ADDRESS(31), (const uint8_t *)wifi, (uint32_t)strlen(wifi) + 1);
          flash_write(PAGE_ADDRESS(32), (const uint8_t *)tcp, (uint32_t)strlen(tcp) + 1);
          break;
        }
      }
    }
  }
}
