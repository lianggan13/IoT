/**************************************************************************************
*		              RS485通信实验												  *
实现现象：下载程序后打开串口调试助手，将波特率设置为4800，选择发送的数据就可以显示
			在串口助手上。具体接线操作参考操作视频
注意事项：无。
***************************************************************************************/

#include "reg52.h" //此文件中定义了单片机的一些特殊功能寄存器

typedef unsigned int u16; // 对数据类型进行声明定义
typedef unsigned char u8;

sbit RS485DIR = P1 ^ 0; // 方向控制: RS485DIR=0为接收状态  RS485DIR=1为发送状态

/*******************************************************************************
 * 函 数 名         : delay
 * 函数功能		   : 延时函数，i=1时，大约延时10us
 *******************************************************************************/
void delay(u16 i)
{
	while (i--)
		;
}

/*******************************************************************************
 * 函数名         :UsartInit()
 * 函数功能		   :设置串口
 * 输入           : 无
 * 输出         	 : 无
 *******************************************************************************/
void UsartInit()
{
	SCON = 0X50; // 设置为工作方式1
	TMOD = 0X20; // 设置计数器工作方式2
	PCON = 0X80; // 波特率加倍
	TH1 = 0XF3;	 // 计数器初始值设置，注意波特率是4800的
	TL1 = 0XF3;
	ES = 1;	 // 打开接收中断
	EA = 1;	 // 打开总中断
	TR1 = 1; // 打开计数器
	RS485DIR = 0;
}

/*******************************************************************************
 * 函 数 名       : main
 * 函数功能		 : 主函数
 * 输    入       : 无
 * 输    出    	 : 无
 *******************************************************************************/
void main()
{
	UsartInit(); //	串口初始化
	while (1)
		;
}

/*******************************************************************************
 * 函数名         : Usart() interrupt 4
 * 函数功能		  : 串口通信中断函数
 * 输入           : 无
 * 输出         	 : 无
 *******************************************************************************/
void Usart() interrupt 4
{
	u8 receiveData;

	receiveData = SBUF; // 出去接收到的数据
	RI = 0;				// 清除接收中断标志位
	delay(100);
	RS485DIR = 1;		// 置位 发送
	SBUF = receiveData; // 将接收到的数据放入到发送寄存器
	while (!TI)
		;		  // 等待发送数据完成
	TI = 0;		  // 清除发送完成标志位
	RS485DIR = 0; // 复位 接收
}