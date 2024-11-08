/**************************************************************************************
LED 8x8 点阵
***************************************************************************************/

#include "reg51.h"
#include "intrins.h"
#include "../Utility/delay.h"
#include "../HC595/HC595.h"
#include "Led_8x8.h"

unsigned char code cols[] = {0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe}; // 列选-P0 	低电平有效
unsigned char code rows[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}; // 行选-HC595 高电平有效

#define ROWS 19
#define COLS 8

// 点阵字码 (取模工具-参数设置-选项：纵向取模 字节 D7~D0 正序)
unsigned char code CHARCODE[ROWS][COLS] = {

    {0x00, 0x00, 0x3e, 0x41, 0x41, 0x41, 0x3e, 0x00}, // 0

    {0x00, 0x00, 0x00, 0x00, 0x21, 0x7f, 0x01, 0x00}, // 1

    {0x00, 0x00, 0x27, 0x45, 0x45, 0x45, 0x39, 0x00}, // 2

    {0x00, 0x00, 0x22, 0x49, 0x49, 0x49, 0x36, 0x00}, // 3

    {0x00, 0x00, 0x0c, 0x14, 0x24, 0x7f, 0x04, 0x00}, // 4

    {0x00, 0x00, 0x72, 0x51, 0x51, 0x51, 0x4e, 0x00}, // 5

    {0x00, 0x00, 0x3e, 0x49, 0x49, 0x49, 0x26, 0x00}, // 6

    {0x00, 0x00, 0x40, 0x40, 0x40, 0x4f, 0x70, 0x00}, // 7

    {0x00, 0x00, 0x36, 0x49, 0x49, 0x49, 0x36, 0x00}, // 8

    {0x00, 0x00, 0x32, 0x49, 0x49, 0x49, 0x3e, 0x00}, // 9

    {0x00, 0x00, 0x7F, 0x48, 0x48, 0x30, 0x00, 0x00}, // P

    {0x00, 0x00, 0x7F, 0x48, 0x4C, 0x73, 0x00, 0x00}, // R

    {0x00, 0x00, 0x7F, 0x49, 0x49, 0x49, 0x00, 0x00}, // E

    {0x00, 0x00, 0x3E, 0x41, 0x41, 0x62, 0x00, 0x00}, // C

    {0x00, 0x00, 0x7F, 0x08, 0x08, 0x7F, 0x00, 0x00}, // H

    {0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00}, // I

    {0x00, 0x7F, 0x10, 0x08, 0x04, 0x7F, 0x00, 0x00}, // N

    {0x7C, 0x48, 0x48, 0xFF, 0x48, 0x48, 0x7C, 0x00}, // 中

    {0x7C, 0x44, 0xFF, 0x45, 0x7D, 0x03, 0x00, 0x00}, // 电
};

void DisplayZero()
{
    unsigned char i;
    P0 = 0x7f;

    while (1)
    {
        for (i = 0; i < 8; i++)
        {
            P0 = cols[i];
            HC595_WriteByte(CHARCODE[0][i]);
            delay_us(1);
            HC595_WriteByte(0x00); // 消隐
        }
    }
}

void DisplayHV()
{
    unsigned char i, j;
    while (1)
    {
        for (j = 0; j < 3; j++) // 从左到右3次
        {
            for (i = 0; i < 8; i++) // 循环8次逐条点亮8个LED点阵灯
            {
                P0 = cols[i];
                HC595_WriteByte(0xff);

                delay_ms(1000);
                HC595_WriteByte(0x00);
            }
        }

        for (j = 0; j < 3; j++) // 从右到左3次
        {
            for (i = 0; i < 8; i++) // 循环8次逐条点亮8个LED点阵灯
            {
                P0 = cols[7 - i];
                HC595_WriteByte(0xff);
                delay_ms(1000);
            }
        }

        for (j = 0; j < 3; j++) // 从上到下3次
        {
            for (i = 0; i < 8; i++) // 循环8次逐条点亮8个LED点阵灯
            {
                P0 = 0x00;
                HC595_WriteByte(rows[7 - i]);
                delay_ms(1000);
            }
        }

        for (j = 0; j < 3; j++) // 从下到上3次
        {
            for (i = 0; i < 8; i++) // 循环8次逐条点亮8个LED点阵灯
            {
                P0 = 0x00;
                HC595_WriteByte(rows[i]);
                delay_ms(1000);
            }
        }
    }
}

void DisplayCodes()
{
    unsigned int i, j, a;

    while (1)
    {
        for (i = 0; i < ROWS; i++)
        {
            while (a++ < 500) // 重复执行的时间，即为显示一个字的时间
            {
                for (j = 0; j < 8; j++)
                {
                    P0 = cols[j];
                    HC595_WriteByte(CHARCODE[i][j]);

                    delay_us(1);
                    HC595_WriteByte(0x00); // 消隐
                }
            }
            a = 0;
        }
    }
}
