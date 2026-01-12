#include "esp8266.h"

#define RN "\r\n"
#define OK "OK"

Esp8266 esp8266;

// USART2 单字节中断接收缓冲
static uint8_t uart2_rx_byte = 0;

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

static void uart2_start_rx_it(void)
{
  // 说明：
  // 1) USART2_IRQHandler 中会调用 HAL_UART_IRQHandler(&huart2) 来分发串口中断
  // 2) HAL_UART_Receive_IT() 会配置 HAL 的接收状态机/缓冲区，并在内部使能所需的 RXNE/错误中断
  // 3) 这里按“单字节接收”启动一次；后续每收到 1 字节会进入 HAL_UART_RxCpltCallback，再在回调里重启接收

  // 可选：仅使能 RXNE 中断（通常不必手动使能，HAL_UART_Receive_IT 内部会处理）
  // __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);

  // 启动 1 字节中断接收（收到 1 字节后触发 HAL_UART_RxCpltCallback）
  HAL_UART_Receive_IT(&huart2, &uart2_rx_byte, 1);
}

static void uart2_send(uint8_t *cmd, uint8_t len, uint32_t timout)
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

bool wait_receive(uint32_t timout)
{
  if (esp8266.it3)
    return true;

  if (timout == 0)
  {
    while (!esp8266.it3)
      HAL_Delay(1);
    return true;
  }

  uint32_t start = HAL_GetTick();
  while (!esp8266.it3)
  {
    if ((HAL_GetTick() - start) >= timout)
      return false;
    HAL_Delay(1);
  }
  return true;
}

void clear_receive()
{
  // uint16_t safeLen = len;
  // if (safeLen > sizeof(esp8266.usart2_buf))
  //   safeLen = sizeof(esp8266.usart2_buf);

  // if (safeLen > 0)
  // {
  //   printf(">> %.*s\n", (int)safeLen, (const char *)esp8266.usart2_buf);
  // }

  uint16_t len = strlen((const char *)esp8266.usart2_buf);
  if (len > 0)
    printf(">> %s\r\n", esp8266.usart2_buf);

  memset(esp8266.usart2_buf, 0x00, sizeof(esp8266.usart2_buf));
  esp8266.usart2_count = 0;
  esp8266.it3 = false; // 表示本帧接收未开始，等待中
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

bool try_send_cmd(char *cmd, uint32_t timout, int8_t retries)
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
      return false;

  } while (check_cmd(OK, timout) == 0);

  return true;
}

bool check_cmd(char *res, uint32_t timout)
{
  bool retval = false;

  if (wait_receive(timout))
  {
    if (strstr((const char *)esp8266.usart2_buf, res))
    {
      retval = true;
    }
  }
  else
  {
    retval = false;
  }

  clear_receive();
  return retval;
}

void reset()
{
  const char *quit = "+++";
  clear_receive();

  uart2_send(quit, strlen(quit), 300);

  wait_receive(300);
  clear_receive();

  send_cmd("AT+RST", 3000);

  send_cmd("AT+CIPMODE=0", 1000);
}

bool connect_wifi(char *wifi)
{
  // send_cmd("AT+CWAUTOCONN=0", 1000);
  // send_cmd("AT+CIPCLOSE", 1000);

  HAL_Delay(2500);
  bool pass = try_send_cmd(wifi, 8000, 5); // AT+CWJAP="LG13_TPLink_2G","G15608212470*"\r\n
  if (pass)
    send_cmd("AT+CWAUTOCONN=1", 1000); // AT+CWAUTOCONN=1\r\n

  HAL_Delay(10);
  return pass;
}

bool connect_tcp_server(char *cmd)
{
  bool pass = try_send_cmd(cmd, 5000, 5); // AT+CIPSTART="TCP","192.168.0.107",1918
  if (pass)
  {
    // enter transparent transmission mode
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

  send_cmd("AT+CIPSTATUS", 1000);

  send_cmd("AT+CWQAP", 1000);

  // char cmd[32];
  // snprintf(cmd, sizeof(cmd), "AT+CIPMUX=%d", ESP8266_MULTI_CONNECTION);
  // send_cmd(cmd, 1000);
  send_cmd("AT+CIPMUX=1", 1000); //  AT+CIPMUX=1 --> link is builded

  set_wifi("Lianggan13_ESP8266", "G15608212470*", 3000); // AT+CWSAP="LG13_ESP8266","G15608212470*",1,3,4

  send_cmd("AT+CIPAP?", 3000); // AT+CIPAP?

  send_cmd("AT+CIPSERVER=1,8080", 1000); // AT+CIPSERVER=1,8080
}

void esp8266_init(void)
{
  esp8266.send = send_cmd;
  esp8266.waitRecv = wait_receive;
  esp8266.clearRecv = clear_receive;

  printf("1.start uart2 rx it...\n");
  uart2_start_rx_it();

  printf("2.set esp8266 sta mode...\n");
  set_sta(); // 设置为 STA 模式

  printf("3.connect wifi...\n");
  for (size_t i = 0; i < 5; i++)
  {
    char wifi[128]; // "AT+CWJAP=\"LG13_TPLink_2G\",\"G15608212470*\""
    flash_read_str(PAGE_ADDRESS(31), wifi, sizeof(wifi));
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

bool tcp_send(char *msg)
{
  char buffer[128];
  snprintf(buffer, sizeof(buffer), "AT+CIPSEND=%d,%d", 0, strlen(msg)); // AT+CIPSEND=0,2
  send_cmd((char *)buffer, 1000);

  uart2_send(buffer, strlen(buffer), 1500);

  return true;
}

uint16_t parse_cmd(const char *key, char *buf, size_t bufSize)
{
  if (key == NULL || buf == NULL || bufSize == 0)
    return 0;

  if (!esp8266.it3)
    return 0;

  const char *rx = (const char *)esp8266.usart2_buf;
  const char *found = strstr(rx, key);
  if (found == NULL)
    return 0;

  // 计算 found 起始到 rx 缓冲区末尾的最大可读长度，避免越界扫描
  size_t offset = (size_t)(found - rx);
  size_t remaining = 0;
  if (offset < sizeof(esp8266.usart2_buf))
    remaining = sizeof(esp8266.usart2_buf) - offset;

  size_t srcLen = strlen(found);

  // 拷贝并保证以 '\0' 结束
  size_t copyLen = (srcLen < (bufSize - 1)) ? srcLen : (bufSize - 1);
  if (copyLen > 0)
    memcpy(buf, found, copyLen);
  buf[copyLen] = '\0';

  clear_receive();
  return (uint16_t)copyLen;
}

void config_wifi(void)
{
  char wifi[128];
  char tcp[64];

  flash_read_str(PAGE_ADDRESS(31), wifi, sizeof(wifi)); // "AT+CWJAP=\"LG13_TPLink_2G\",\"G15608212470*\""
  flash_read_str(PAGE_ADDRESS(32), tcp, sizeof(tcp));   // "AT+CIPSTART=\"TCP\","

  // TODO
  // 如果没有连接上 wifi，就启动 AP 模式重新配置 wifi 参数
  // 客户端连接上 AP 后，通过网页配置 wifi 参数，然后保存到 flash 中

  uint8_t pass = connect_tcp_server(tcp);

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
          if (parse_cmd("AT+CWJAP", wifi, sizeof(wifi)) > 0)
          {
            tcp_send("OK");
            HAL_Delay(500);
          }
        }

        if (strlen(tcp) == 0)
        {
          if (parse_cmd("AT+CIPSTART", tcp, sizeof(tcp)) > 0)
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
        pass = connect_tcp_server(tcp);
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
