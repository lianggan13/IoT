#include <reg52.h>
#include "../Utility/delay.h"
#include "../Utility/CommData.h"
#include "DynamicLed.h"

sbit LSA = P2 ^ 2;
sbit LSB = P2 ^ 3;
sbit LSC = P2 ^ 4;

void DigDisplay(unsigned char *DisplayData)
{
    unsigned char i;
    for (i = 0; i < 6; i++)
    {
        switch (i) // 位选，选择点亮的数码管，
        {
        case (0):
            LSA = 0;
            LSB = 0;
            LSC = 0;
            break; // 显示第0位
        case (1):
            LSA = 1;
            LSB = 0;
            LSC = 0;
            break; // 显示第1位
        case (2):
            LSA = 0;
            LSB = 1;
            LSC = 0;
            break; // 显示第2位
        case (3):
            LSA = 1;
            LSB = 1;
            LSC = 0;
            break; // 显示第3位
        case (4):
            LSA = 0;
            LSB = 0;
            LSC = 1;
            break; // 显示第4位
        case (5):
            LSA = 1;
            LSB = 0;
            LSC = 1;
            break; // 显示第5位
        }
        P0 = DisplayData[i]; // 发送数据
        delay_ms(10);        // 间隔一段时间扫描
        P0 = 0x00;           // 消隐
    }
}