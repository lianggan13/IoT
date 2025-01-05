#include <stdlib.h> // ���� malloc �� free
#include "syn6288.h"
#include "usart.h"
#include "string.h"
#include "delay.h"

// Music:ѡ�񱳾����֡�0:�ޱ������֣�1~15��ѡ�񱳾�����
void SYN_FrameInfo(u8 Music, u8 *HZdata)
{
  /****************��Ҫ���͵��ı�**********************************/
  unsigned char Frame_Info[50];
  unsigned char HZ_Length;
  unsigned char ecc = 0; // ����У���ֽ�
  unsigned int i = 0;
  HZ_Length = strlen((char *)HZdata); // ��Ҫ�����ı��ĳ���

  /*****************֡�̶�������Ϣ**************************************/
  Frame_Info[0] = 0xFD;              // ����֡ͷFD
  Frame_Info[1] = 0x00;              // �������������ȵĸ��ֽ�
  Frame_Info[2] = HZ_Length + 3;     // �������������ȵĵ��ֽ�
  Frame_Info[3] = 0x01;              // ���������֣��ϳɲ������� 0x01
  Frame_Info[4] = 0x01 | Music << 4; // ����������������������趨(��5λ)���ı����뷽ʽ(��3λ)

  /*******************У�������***************************************/
  for (i = 0; i < 5; i++) // ���η��͹���õ�5��֡ͷ�ֽ�
  {
    ecc = ecc ^ (Frame_Info[i]); // �Է��͵��ֽڽ������У��
  }

  for (i = 0; i < HZ_Length; i++) // ���η��ʹ��ϳɵ��ı�����
  {
    ecc = ecc ^ (HZdata[i]); // �Է��͵��ֽڽ������У��
  }
  /*******************����֡��Ϣ***************************************/
  memcpy(&Frame_Info[5], HZdata, HZ_Length);
  Frame_Info[5 + HZ_Length] = ecc;
  USART3_SendString(Frame_Info, 5 + HZ_Length + 1);
}

// Music:ѡ�񱳾����֡�0:�ޱ������֣�1~15��ѡ�񱳾�����
void SYN_Send(u8 *dataBuf)
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
    // u8 SYN_StopCom[] = {0xFD, 0X00, 0X02, 0X02, 0XFD};      // ֹͣ�ϳ�
    // u8 SYN_SuspendCom[] = {0XFD, 0X00, 0X02, 0X03, 0XFC};   // ��ͣ�ϳ�
    // u8 SYN_RecoverCom[] = {0XFD, 0X00, 0X02, 0X04, 0XFB};   // �ָ��ϳ�
    // u8 SYN_ChackCom[] = {0XFD, 0X00, 0X02, 0X21, 0XDE};     // ״̬��ѯ
    // u8 SYN_PowerDownCom[] = {0XFD, 0X00, 0X02, 0X88, 0X77}; // ����POWER DOWN ״̬����
    txt_len = 0;  // �ı����� 0
    head_len = 4; // ֡ͷ���� 4
  }
  size_t send_len = data_len + 3; // ֡�ܳ��� +3 ֡ͷ�����ȸߡ����ȵ�

  uint8_t *sendBuf = (uint8_t *)malloc(send_len * sizeof(uint8_t)); // Error: L6915E --> Options for Target �� Use MicroLIB

  unsigned char ecc = 0; // ����У���ֽ�
  unsigned int i = 0;

  /*****************֡�̶�������Ϣ**************************************/
  sendBuf[0] = 0xFD;                   // ����֡ͷFD
  sendBuf[1] = (data_len >> 8) & 0xFF; // �������������ȵĸ��ֽ�
  sendBuf[2] = data_len & 0xFF;        // �������������ȵĵ��ֽ�

  memcpy(&sendBuf[3], dataBuf, strlen((const char *)dataBuf));

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
  USART3_SendString(sendBuf, send_len);

  free(sendBuf);
}

/***********************************************************
 * ��    �ƣ� YS_SYN_Set(u8 *Info_data)
 * ��    �ܣ� ������	�������
 * ��ڲ����� *Info_data:�̶���������Ϣ����
 * ���ڲ�����
 * ˵    �����������������ã�ֹͣ�ϳɡ���ͣ�ϳɵ����� ��Ĭ�ϲ�����9600bps��
 * ���÷�����ͨ�������Ѿ�������������������á�
 **********************************************************/
void YS_SYN_Set(u8 *Info_data)
{
  u8 Com_Len;
  Com_Len = strlen((char *)Info_data);
  USART3_SendString(Info_data, Com_Len);
}

void TestSYN6288()
{
  /**************оƬ��������*********************/
  // u8 SYN_StopCom[] = {0xFD, 0X00, 0X02, 0X02, 0XFD};      // ֹͣ�ϳ�
  // u8 SYN_SuspendCom[] = {0XFD, 0X00, 0X02, 0X03, 0XFC};   // ��ͣ�ϳ�
  // u8 SYN_RecoverCom[] = {0XFD, 0X00, 0X02, 0X04, 0XFB};   // �ָ��ϳ�
  // u8 SYN_ChackCom[] = {0XFD, 0X00, 0X02, 0X21, 0XDE};     // ״̬��ѯ
  // u8 SYN_PowerDownCom[] = {0XFD, 0X00, 0X02, 0X88, 0X77}; // ����POWER DOWN ״̬����
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // �����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�

  USART3_Init(9600);

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
    SYN_FrameInfo(0, "[v3][m1][t5]6666");

    // �ش�����
    // 0x4A ��ʼ���ɹ��ش� 	       оƬ��ʼ���ɹ�
    // 0x41 �յ���ȷ������֡�ش�	  ���ճɹ�
    // 0x45 �յ�����ʶ������֡�ش�	����ʧ��
    // 0x4E оƬ����״̬�ش�			 �յ���״̬��ѯ����֡����оƬ�������ڲ���״̬
    // 0x4F оƬ����״̬�ش�		 	  ��һ֡���ݺϳ����Ժ�оƬ�������״̬�ش�0x4F;�����յ���״̬��ѯ����֡����оƬ���ڿ���״̬�ش�0x4F

    Delay_s(5);
    // ��ͣ�ϳɣ���ʱû���õ�������չʾ�����÷�
    // YS_SYN_Set(SYN_SuspendCom);
  }
}
