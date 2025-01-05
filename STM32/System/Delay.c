#include "stm32f10x.h"

void APP_Delay(uint32_t xus)
{
	// 计算循环次数
	volatile uint32_t count;

	while (xus--)
	{
		// 72MHz 约 72 个循环大约 1 微秒
		count = 72; // 具体的计数值可能需要根据编译器优化进行调整
		while (count--)
			;
	}
}

/**
 * @brief  微秒级延时(注:避免嵌套调用)
 * @param  xus 延时时长，范围：0~233015
 * @retval 无
 */
void Delay_us(uint32_t xus)
{
	SysTick->LOAD = 72 * xus;	// 设置定时器重装值
	SysTick->VAL = 0x00;		// 清空当前计数值
	SysTick->CTRL = 0x00000005; // 设置时钟源为HCLK，启动定时器
	while (!(SysTick->CTRL & 0x00010000))
		;						// 等待计数到0
	SysTick->CTRL = 0x00000004; // 关闭定时器
}

/**
 * @brief  毫秒级延时(注:避免嵌套调用)
 * @param  xms 延时时长，范围：0~4294967295
 * @retval 无
 */
void Delay_ms(uint32_t xms)
{
	while (xms--)
	{
		Delay_us(1000);
	}
}

/**
 * @brief  秒级延时(注:避免嵌套调用)
 * @param  xs 延时时长，范围：0~4294967295
 * @retval 无
 */
void Delay_s(uint32_t xs)
{
	while (xs--)
	{
		Delay_ms(1000);
	}
}
