#include <reg52.h>
#include "../Utility/delay.h"
#include "../Utility/CommData.h"
#include "DynamicLed.h"

sbit LSA = P2 ^ 2;
sbit LSB = P2 ^ 3;
sbit LSC = P2 ^ 4;

unsigned char DisplayData[8];

void DigDisplay()
{
    unsigned char i;
    for (i = 0; i < 8; i++)
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
        case (6):
            LSA = 0;
            LSB = 1;
            LSC = 1;
            break; // 显示第6位
        case (7):
            LSA = 1;
            LSB = 1;
            LSC = 1;
            break; // 显示第7位
        }
        P0 = DisplayData[i]; // 发送数据
        delay_us(1);         // 间隔一段时间扫描
        P0 = 0x00;           // 消隐
    }
}