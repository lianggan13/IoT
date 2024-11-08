#include "reg52.h"
#include "LCD1602.h"

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint
#define uint unsigned int
#endif

#define LCD1602_DATAPINS P0 // 数据8位 DB0-DB7
sbit LCD1602_E = P2 ^ 7;    // 使能信号
sbit LCD1602_RW = P2 ^ 5;   // 读写选择 R/_W
sbit LCD1602_RS = P2 ^ 6;   // 数据/_命令 选择

uchar Disp[] = " LiangganNo.13 ";
uchar Disp2[] = " 666666 ";

/*******************************************************************************
 * 函 数 名         : Lcd1602_Delay1ms
 * 函数功能		   : 延时函数，延时1ms
 * 输    入         : c
 * 输    出         : 无
 * 说    名         : 该函数是在12MHZ晶振下，12分频单片机的延时。
 *******************************************************************************/

void Lcd1602_Delay1ms(uint c) // 误差 0us
{
    uchar a, b;
    for (; c > 0; c--)
    {
        for (b = 199; b > 0; b--)
        {
            for (a = 1; a > 0; a--)
                ;
        }
    }
}

/*******************************************************************************
 * 函 数 名         : LcdWriteCom
 * 函数功能		   : 向LCD写入一个字节的命令
 * 输    入         : com
 * 输    出         : 无
 *******************************************************************************/
#ifndef LCD1602_4PINS       // 当没有定义这个LCD1602_4PINS时
void LcdWriteCom(uchar com) // 写入命令
{
    LCD1602_E = 0;  // 使能
    LCD1602_RS = 0; // 选择发送命令
    LCD1602_RW = 0; // 选择写入

    LCD1602_DATAPINS = com; // 放入命令
    Lcd1602_Delay1ms(1);    // 等待数据稳定

    LCD1602_E = 1;       // 写入时序
    Lcd1602_Delay1ms(5); // 保持时间
    LCD1602_E = 0;
}
#else
void LcdWriteCom(uchar com) // 写入命令
{
    LCD1602_E = 0;  // 使能清零
    LCD1602_RS = 0; // 选择写入命令
    LCD1602_RW = 0; // 选择写入

    LCD1602_DATAPINS = com; // 由于4位的接线是接到P0口的高四位，所以传送高四位不用改
    Lcd1602_Delay1ms(1);

    LCD1602_E = 1; // 写入时序
    Lcd1602_Delay1ms(5);
    LCD1602_E = 0;

    LCD1602_DATAPINS = com << 4; // 发送低四位
    Lcd1602_Delay1ms(1);

    LCD1602_E = 1; // 写入时序
    Lcd1602_Delay1ms(5);
    LCD1602_E = 0;
}
#endif
/*******************************************************************************
 * 函 数 名         : LcdWriteData
 * 函数功能		   : 向LCD写入一个字节的数据
 * 输    入         : dat
 * 输    出         : 无
 *******************************************************************************/
#ifndef LCD1602_4PINS
void LcdWriteData(uchar dat) // 写入数据
{
    LCD1602_E = 0;  // 使能清零
    LCD1602_RS = 1; // 选择输入数据
    LCD1602_RW = 0; // 选择写入

    LCD1602_DATAPINS = dat; // 写入数据
    Lcd1602_Delay1ms(1);

    LCD1602_E = 1;       // 写入时序
    Lcd1602_Delay1ms(5); // 保持时间
    LCD1602_E = 0;
}
#else
void LcdWriteData(uchar dat) // 写入数据
{
    LCD1602_E = 0;  // 使能清零
    LCD1602_RS = 1; // 选择写入数据
    LCD1602_RW = 0; // 选择写入

    LCD1602_DATAPINS = dat; // 由于4位的接线是接到P0口的高四位，所以传送高四位不用改
    Lcd1602_Delay1ms(1);

    LCD1602_E = 1; // 写入时序
    Lcd1602_Delay1ms(5);
    LCD1602_E = 0;

    LCD1602_DATAPINS = dat << 4; // 写入低四位
    Lcd1602_Delay1ms(1);

    LCD1602_E = 1; // 写入时序
    Lcd1602_Delay1ms(5);
    LCD1602_E = 0;
}
#endif
/*******************************************************************************
 * 函 数 名       : LcdInit()
 * 函数功能		 : 初始化LCD屏
 * 输    入       : 无
 * 输    出       : 无
 *******************************************************************************/
#ifndef LCD1602_4PINS
void LcdInit() // LCD初始化子程序
{
    // 功能设定
    // 001
    // DL 0=数据总线为 4 位     1=数据总线为 8 位
    // N 0=显示 1 行     1=显示 2 行
    // F 0=5×7 点阵/每字符     1=5×10 点阵/每字符
    // 00
    LcdWriteCom(0x38); // 开显示

    // 显示开关控制指令
    // 00001
    // D     0=显示功能关   1=显示功能开
    // C     0=无光标       1=有光标
    // B     0=光标闪烁     1=光标不闪烁
    LcdWriteCom(0x0c); // 开显示不显示光标

    // 模式设置
    // 000001
    // I/D  0=写入新数据后光标左移     1=写入新数据后光标右移
    // S    0=写入新数据后显示屏不移动 1=写入新数据后显示屏整体右
    LcdWriteCom(0x06); // 写一个指针加1

    LcdWriteCom(0x01); // 清屏

    // 0x00 + 0x80
    LcdWriteCom(0x80); // 设置数据指针起点
}
#else
void LcdInit() // LCD初始化子程序
{
    LcdWriteCom(0x32); // 将8位总线转为4位总线
    LcdWriteCom(0x28); // 在四位线下的初始化
    LcdWriteCom(0x0c); // 开显示不显示光标
    LcdWriteCom(0x06); // 写一个指针加1
    LcdWriteCom(0x01); // 清屏
    LcdWriteCom(0x80); // 设置数据指针起点
}
#endif

void TestLCD1602()
{
    uchar i;
    LcdInit();
    for (i = 0; i < 16; i++)
    {
        LcdWriteData(Disp[i]);
    }

    // 0x40 + 0x80
    LcdWriteCom(0x40 + 0x80); // 设置数据指针起点 在第二行

    for (i = 0; i < 16; i++)
    {
        LcdWriteData(Disp2[i]);
    }

    while (1)
        ;
}
