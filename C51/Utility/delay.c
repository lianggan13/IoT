
#include <reg51.h>
#include <intrins.h>
#include "delay.h"

void delay_us(unsigned char us)
{
    int i = 0;
    while (us--)
        for (i = 0; i < 12; i++)
            ; // 空操作循环，大约1us
}

void delay_ms(unsigned char ms)
{
    while (ms--)
        // delay_us(1000);
        delay_us(800);
}
