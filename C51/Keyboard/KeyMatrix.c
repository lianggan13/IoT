/**************************************************************************************
*		              矩阵按键实验												  *
实现现象：下载程序后数码管显示0，按下矩阵按键上的按键显示对应的数字
            S1-S4：0-3
            S5-S8：4-7
            S9-S12：8-B
            S13-S16：C-F。
注意事项：
***************************************************************************************/

#include "reg52.h" //此文件中定义了单片机的一些特殊功能寄存器
#include "KeyMatrix.h"
#include "../Utility/delay.h"
#include "../Utility/CommData.h"

typedef unsigned int u16; // 对数据类型进行声明定义
typedef unsigned char u8;

#define GPIO_DIG P0 // P0^7_P0^0 - ~ ABCDEFG
#define GPIO_KEY P1 // P1^7_P1^0 - C0C1C2C3K0K1K2K3

/*******************************************************************************
 行列扫描按键
 行：高4位 列：低4位
 高4位为0，低4位为1 0x0F(0b00001111) -> 高4位为1，低4位为0 0xF0(0b11110000)
 *******************************************************************************/
int CheckKeyDown()
{
    char a = 0;
    int Key = -1; // 用来存放读取到的键值
    GPIO_KEY = 0x0F;
    if (GPIO_KEY != 0x0F) // 读取按键是否按下
    {
        delay_ms(10);         // 延时10ms进行消抖
        if (GPIO_KEY != 0x0f) // 再次检测键盘是否按下
        {
            // 确定按键所在列
            // GPIO_KEY = 0X0F;
            switch (GPIO_KEY)
            {
            case (0X07):
                Key = 0;
                break;
            case (0X0b):
                Key = 1;
                break;
            case (0X0d):
                Key = 2;
                break;
            case (0X0e):
                Key = 3;
                break;
            }
            // 确定按键所在列行
            GPIO_KEY = 0XF0;
            switch (GPIO_KEY)
            {
            case (0X70):
                Key = Key;
                break;
            case (0Xb0):
                Key = Key + 4;
                break;
            case (0Xd0):
                Key = Key + 8;
                break;
            case (0Xe0):
                Key = Key + 12;
                break;
            }
            // 检测按键松手检测
            // while ((a < 50) && (GPIO_KEY != 0XF0))
            // {
            //     delay_ms(10);
            //     a++; // 如果按键一直按下，超过 50*10ms，就强制退出，保证程序继续执行
            // }
        }
    }
    return Key;
}

void TestKeyMatrix()
{

    while (1)
    {
        unsigned char Key = CheckKeyDown();
        GPIO_DIG = ~smgduan[Key];
    }
}
