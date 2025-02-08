#include "syn6288.h"
#include "usart.h"
// #include "time_handle.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern void SYN6288_ReceivedData(uint8_t receivedByte); // ���� UART3 �Ĵ�����

void HAL_UART3_IRQHandler(void)
{
  unsigned char receive_data = 0;
  if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE) != RESET)
  {
    uint8_t receivedByte; // ���ڴ洢���յ����ֽ�

    HAL_UART_Receive(&huart3, &receivedByte, 1, 1000); // ����2����1λ����

    SYN6288_ReceivedData(receivedByte);
  }
}

void uart3_send(uint8_t *DAT, uint8_t len)
{
  HAL_UART_Transmit(&huart3, DAT, len, 1500);
}

// Music:ѡ�񱳾����֡�0:�ޱ������֣�1~15��ѡ�񱳾�����
void syn6288_send_cmd(uint8_t *dataBuf)
{
  /*****************֡�ṹ*************************************
   * ֡ͷ(1�ֽ�)
   * ����������(2�ֽ�)
   * ������(С�ڵ���203�ֽ�)
   * -- ������(1�ֽ�)
   * -- �������(1�ֽ�)
   * -- �������ı�(С�ڵ���200�ֽ�)
   * -- ���У��(1�ֽ�)
   * **********************************************************/
  size_t head_len = 5;                                 // ֡ͷ���� 5
  size_t data_len = strlen((const char *)dataBuf) + 1; // ���������� +1 CRC
  size_t txt_len = data_len - 3;                       // �ı����� -3 ���������CRC
  uint8_t cmd = dataBuf[0];
  if (cmd != 0x01)
  {
    // uint8_t SYN_StopCom[] = {0xFD, 0X00, 0X02, 0X02, 0XFD};      // ֹͣ�ϳ�
    // uint8_t SYN_SuspendCom[] = {0XFD, 0X00, 0X02, 0X03, 0XFC};   // ��ͣ�ϳ�
    // uint8_t SYN_RecoverCom[] = {0XFD, 0X00, 0X02, 0X04, 0XFB};   // �ָ��ϳ�
    // uint8_t SYN_ChackCom[] = {0XFD, 0X00, 0X02, 0X21, 0XDE};     // ״̬��ѯ
    // uint8_t SYN_PowerDownCom[] = {0XFD, 0X00, 0X02, 0X88, 0X77}; // ����POWER DOWN ״̬����
    txt_len = 0;  // �ı����� 0
    head_len = 4; // ֡ͷ���� 4
  }
  size_t send_len = data_len + 3; // ֡�ܳ��� +3 ֡ͷ�����ȸߡ����ȵ�

  uint8_t *sendBuf = (uint8_t *)malloc(send_len * sizeof(uint8_t)); // Error: L6915E --> Options for Target �� Use MicroLIB

  unsigned char ecc = 0; // ����У���ֽ�
  unsigned int i = 0;

  /*****************֡�̶�������Ϣ**************************************/
  sendBuf[0] = 0xFD;                                           // ����֡ͷFD
  sendBuf[1] = (data_len >> 8) & 0xFF;                         // �������������ȵĸ��ֽ�
  sendBuf[2] = data_len & 0xFF;                                // �������������ȵĵ��ֽ�
  memcpy(&sendBuf[3], dataBuf, strlen((const char *)dataBuf)); // ��������������

  /*******************У�������***************************************/
  for (i = 0; i < head_len; i++) // ���η��͹���õ�5��֡ͷ�ֽ�
  {
    ecc = ecc ^ (sendBuf[i]); // �Է��͵��ֽڽ������У��
  }

  for (i = head_len; i < txt_len + head_len; i++) // ���η��ʹ��ϳɵ��ı�����
  {
    ecc = ecc ^ (sendBuf[i]); // �Է��͵��ֽڽ������У��
  }

  sendBuf[send_len - 1] = ecc;

  /*******************����֡��Ϣ***************************************/
  uart3_send(sendBuf, send_len);

  free(sendBuf);
}

void TestSYN6288()
{
  /**************оƬ��������*********************/
  // uint8_t SYN_StopCom[] = {0xFD, 0X00, 0X02, 0X02, 0XFD};      // ֹͣ�ϳ�
  // uint8_t SYN_SuspendCom[] = {0XFD, 0X00, 0X02, 0X03, 0XFC};   // ��ͣ�ϳ�
  // uint8_t SYN_RecoverCom[] = {0XFD, 0X00, 0X02, 0X04, 0XFB};   // �ָ��ϳ�
  // uint8_t SYN_ChackCom[] = {0XFD, 0X00, 0X02, 0X21, 0XDE};     // ״̬��ѯ
  // uint8_t SYN_PowerDownCom[] = {0XFD, 0X00, 0X02, 0X88, 0X77}; // ����POWER DOWN ״̬����
  // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // �����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�

  while (1)
  {
    // ���ͱ���
    // ѡ�񱳾�����2 (0���ޱ�������  1-15���������ֿ�ѡ)
    // m[0~16]:0��������Ϊ������16���������������
    // v[0~16]:0�ʶ�����Ϊ������16�ʶ��������
    // t[0~5]:0�ʶ�����������5�ʶ��������
    // o[0~1]:0��Ȼ�ʶ���ʽ��1���� Word-By-Word ��ʽ
    // n[0~2]:0�Զ��жϣ�1���������봦��2��������ֵ����
    // [2]:���Ʊ�Ǻ��2������ǿ�ƶ��ɡ����ִʡ�
    // [3]:���Ʊ�Ǻ��3������ǿ�ƶ��ɡ����ִʡ�
    // SYN_FrameInfo(0, "[v7][m1][t5]��ӭʹ�������콢��SYN6288�����ϳ�ģ��");
    syn6288_send_cmd("[v3][m1][t5]6666");

    // �ش�����
    // 0x4A ��ʼ���ɹ��ش� 	       оƬ��ʼ���ɹ�
    // 0x41 �յ���ȷ������֡�ش�	  ���ճɹ�
    // 0x45 �յ�����ʶ������֡�ش�	����ʧ��
    // 0x4E оƬ����״̬�ش�			 �յ���״̬��ѯ����֡����оƬ�������ڲ���״̬
    // 0x4F оƬ����״̬�ش�		 	  ��һ֡���ݺϳ����Ժ�оƬ�������״̬�ش�0x4F;�����յ���״̬��ѯ����֡����оƬ���ڿ���״̬�ش�0x4F

    HAL_Delay(5);
  }
}
