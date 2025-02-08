#include "syn6288.h"
#include "usart.h"
// #include "time_handle.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern void SYN6288_ReceivedData(uint8_t receivedByte); // 引用 UART3 的处理函数

void HAL_UART3_IRQHandler(void)
{
  unsigned char receive_data = 0;
  if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE) != RESET)
  {
    uint8_t receivedByte; // 用于存储接收到的字节

    HAL_UART_Receive(&huart3, &receivedByte, 1, 1000); // 串口2接收1位数据

    SYN6288_ReceivedData(receivedByte);
  }
}

void uart3_send(uint8_t *DAT, uint8_t len)
{
  HAL_UART_Transmit(&huart3, DAT, len, 1500);
}

// Music:选择背景音乐。0:无背景音乐，1~15：选择背景音乐
void syn6288_send_cmd(uint8_t *dataBuf)
{
  /*****************帧结构*************************************
   * 帧头(1字节)
   * 数据区长度(2字节)
   * 数据区(小于等于203字节)
   * -- 命令字(1字节)
   * -- 命令参数(1字节)
   * -- 待发送文本(小于等于200字节)
   * -- 异或校验(1字节)
   * **********************************************************/
  size_t head_len = 5;                                 // 帧头长度 5
  size_t data_len = strlen((const char *)dataBuf) + 1; // 数据区长度 +1 CRC
  size_t txt_len = data_len - 3;                       // 文本长度 -3 命令、参数、CRC
  uint8_t cmd = dataBuf[0];
  if (cmd != 0x01)
  {
    // uint8_t SYN_StopCom[] = {0xFD, 0X00, 0X02, 0X02, 0XFD};      // 停止合成
    // uint8_t SYN_SuspendCom[] = {0XFD, 0X00, 0X02, 0X03, 0XFC};   // 暂停合成
    // uint8_t SYN_RecoverCom[] = {0XFD, 0X00, 0X02, 0X04, 0XFB};   // 恢复合成
    // uint8_t SYN_ChackCom[] = {0XFD, 0X00, 0X02, 0X21, 0XDE};     // 状态查询
    // uint8_t SYN_PowerDownCom[] = {0XFD, 0X00, 0X02, 0X88, 0X77}; // 进入POWER DOWN 状态命令
    txt_len = 0;  // 文本长度 0
    head_len = 4; // 帧头长度 4
  }
  size_t send_len = data_len + 3; // 帧总长度 +3 帧头、长度高、长度低

  uint8_t *sendBuf = (uint8_t *)malloc(send_len * sizeof(uint8_t)); // Error: L6915E --> Options for Target √ Use MicroLIB

  unsigned char ecc = 0; // 定义校验字节
  unsigned int i = 0;

  /*****************帧固定配置信息**************************************/
  sendBuf[0] = 0xFD;                                           // 构造帧头FD
  sendBuf[1] = (data_len >> 8) & 0xFF;                         // 构造数据区长度的高字节
  sendBuf[2] = data_len & 0xFF;                                // 构造数据区长度的低字节
  memcpy(&sendBuf[3], dataBuf, strlen((const char *)dataBuf)); // 拷贝整个数据区

  /*******************校验码计算***************************************/
  for (i = 0; i < head_len; i++) // 依次发送构造好的5个帧头字节
  {
    ecc = ecc ^ (sendBuf[i]); // 对发送的字节进行异或校验
  }

  for (i = head_len; i < txt_len + head_len; i++) // 依次发送待合成的文本数据
  {
    ecc = ecc ^ (sendBuf[i]); // 对发送的字节进行异或校验
  }

  sendBuf[send_len - 1] = ecc;

  /*******************发送帧信息***************************************/
  uart3_send(sendBuf, send_len);

  free(sendBuf);
}

void TestSYN6288()
{
  /**************芯片设置命令*********************/
  // uint8_t SYN_StopCom[] = {0xFD, 0X00, 0X02, 0X02, 0XFD};      // 停止合成
  // uint8_t SYN_SuspendCom[] = {0XFD, 0X00, 0X02, 0X03, 0XFC};   // 暂停合成
  // uint8_t SYN_RecoverCom[] = {0XFD, 0X00, 0X02, 0X04, 0XFB};   // 恢复合成
  // uint8_t SYN_ChackCom[] = {0XFD, 0X00, 0X02, 0X21, 0XDE};     // 状态查询
  // uint8_t SYN_PowerDownCom[] = {0XFD, 0X00, 0X02, 0X88, 0X77}; // 进入POWER DOWN 状态命令
  // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组为组2：2位抢占优先级，2位响应优先级

  while (1)
  {
    // 发送报文
    // 选择背景音乐2 (0：无背景音乐  1-15：背景音乐可选)
    // m[0~16]:0背景音乐为静音，16背景音乐音量最大
    // v[0~16]:0朗读音量为静音，16朗读音量最大
    // t[0~5]:0朗读语速最慢，5朗读语速最快
    // o[0~1]:0自然朗读方式，1设置 Word-By-Word 方式
    // n[0~2]:0自动判断，1数字作号码处理，2数字作数值处理
    // [2]:控制标记后的2个汉字强制读成“两字词”
    // [3]:控制标记后的3个汉字强制读成“三字词”
    // SYN_FrameInfo(0, "[v7][m1][t5]欢迎使用绿深旗舰店SYN6288语音合成模块");
    syn6288_send_cmd("[v3][m1][t5]6666");

    // 回传报文
    // 0x4A 初始化成功回传 	       芯片初始化成功
    // 0x41 收到正确的命令帧回传	  接收成功
    // 0x45 收到不能识别命令帧回传	接收失败
    // 0x4E 芯片播音状态回传			 收到“状态查询命令帧”，芯片处在正在播音状态
    // 0x4F 芯片空闲状态回传		 	  当一帧数据合成完以后，芯片进入空闲状态回传0x4F;或者收到“状态查询命令帧”，芯片处于空闲状态回传0x4F

    HAL_Delay(5);
  }
}
