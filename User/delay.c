#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////
// 如果使用OS,则包括下面的头文件即可
#if 1
#include "FreeRTOS.h" //FreeRTOS使用
#include "task.h"
#endif
//////////////////////////////////////////////////////////////////////////////////
// 本程序只供学习使用，未经作者许可，不得用于其它任何用途
// ALIENTEK STM32F407开发板
// 使用SysTick的普通计数模式对延迟进行管理(支持OS)
// 包括delay_us,delay_ms
// 正点原子@ALIENTEK
// 技术论坛:www.openedv.com
// 创建日期:2014/5/2
// 版本：V1.3
// 版权所有，盗版必究。
// Copyright(C) 广州市星翼电子科技有限公司 2014-2024
// All rights reserved
//********************************************************************************
// 修改说明
//////////////////////////////////////////////////////////////////////////////////

static u8 fac_us = 0;  // us延时倍乘数
static u16 fac_ms = 0; // ms延时倍乘数,在os下,代表每个节拍的ms数

extern void xPortSysTickHandler(void);

// systick中断服务函数,使用OS时用到
void SysTick_Handler(void)
{
	if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) // 系统已经运行
	{
		xPortSysTickHandler();
	}
}

/*
*********************************************************************************************************
*                                             寄存器
*********************************************************************************************************
*/
#define DWT_CYCCNT *(volatile unsigned int *)0xE0001004
#define DWT_CR *(volatile unsigned int *)0xE0001000
#define DEM_CR *(volatile unsigned int *)0xE000EDFC
#define DBGMCU_CR *(volatile unsigned int *)0xE0042004

#define DEM_CR_TRCENA (1 << 24)
#define DWT_CR_CYCCNTENA (1 << 0)

/*
*********************************************************************************************************
*	函 数 名: bsp_InitDWT
*	功能说明: 初始化DWT. 该函数被 bsp_Init() 调用。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitDWT(void)
{
	DEM_CR |= (unsigned int)DEM_CR_TRCENA; /* Enable Cortex-M4's DWT CYCCNT reg.  */
	DWT_CYCCNT = (unsigned int)0u;
	DWT_CR |= (unsigned int)DWT_CR_CYCCNTENA;
}

// 初始化延迟函数
// SYSTICK的时钟固定为AHB时钟，基础例程里面SYSTICK时钟频率为AHB/8
// 这里为了兼容FreeRTOS，所以将SYSTICK的时钟频率改为AHB的频率！
// SYSCLK:系统时钟频率
void delay_init(u8 SYSCLK)
{
	u32 reload;
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	fac_us = SYSCLK;						   // 不论是否使用OS,fac_us都需要使用
	reload = SYSCLK;						   // 每秒钟的计数次数 单位为M
	reload *= 1000000 / configTICK_RATE_HZ;	   // 根据configTICK_RATE_HZ设定溢出时间
											   // reload为24位寄存器,最大值:16777216,在168M下,约合0.0998s左右
	fac_ms = 1000 / configTICK_RATE_HZ;		   // 代表OS可以延时的最少单位
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk; // 开启SYSTICK中断
	SysTick->LOAD = reload;					   // 每1/configTICK_RATE_HZ断一次
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;  // 开启SYSTICK

	// 使用dwt延时
	bsp_InitDWT();
}

/*
*********************************************************************************************************
*	函 数 名: Delayus
*	功能说明: 这里的延时采用CPU的内部计数实现，32位计数器
*             	OSSchedLock(&err);
*				Delayus(5);
*				OSSchedUnlock(&err); 根据实际情况看看是否需要加调度锁或选择关中断
*	形    参: nus  延迟长度，单位1 us
*	返 回 值: 无
*   说    明: 1. 主频168MHz的情况下，32位计数器计满是2^32/168000000 = 25.565秒
*                建议使用本函数做延迟的话，延迟在1秒以下。
*             2. 实际通过示波器测试，微妙延迟函数比实际设置实际多运行0.25us左右的时间。
*             下面数据测试条件：
*             （1）. MDK5.15，优化等级0, 不同的MDK优化等级对其没有影响。
*             （2）. STM32F407IGT6
*             （3）. 测试方法：
*				 GPIOI->BSRRL = GPIO_Pin_8;
*				 Delayus(10);
*				 GPIOI->BSRRH = GPIO_Pin_8;
*             -------------------------------------------
*                测试                 实际执行
*             Delayus(1)          	1.2360us
*             Delayus(2)          	2.256us
*             Delayus(3)          	3.256us
*             Delayus(4)          	4.256us
*             Delayus(5)          	5.276us
*             Delayus(6)          	6.276us
*             Delayus(7)          	7.276us
*             Delayus(8)          	8.276us
*             Delayus(9)          	9.276us
*             Delayus(10)         	10.28us
*            3. 两个32位无符号数相减，获取的结果再赋值给32位无符号数依然可以正确的获取差值。
*              假如A,B,C都是32位无符号数。
*              如果A > B  那么A - B = C，这个很好理解，完全没有问题
*              如果A < B  那么A - B = C， C的数值就是0xFFFFFFFF - B + A + 1。这一点要特别注意，正好用于本函数。
*********************************************************************************************************
*/
void Delayus(u32 nus)
{
	uint32_t tCnt, tDelayCnt;
	uint32_t tStart;

	tStart = DWT_CYCCNT; /* 刚进入时的计数器值 */
	tCnt = 0;
	tDelayCnt = nus * (SystemCoreClock / 1000000); /* 需要的节拍数 */

	while (tCnt < tDelayCnt)
	{
		tCnt = DWT_CYCCNT - tStart; /* 求减过程中，如果发生第一次32位计数器重新计数，依然可以正确计算 */
	}
}

// 延时nms
// nms:要延时的ms数
// nms:0~25500
void Delayms(u32 nms)
{
	fac_us=fac_us;
	if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) // 系统已经运行
	{
		if (nms >= fac_ms) // 延时的时间大于OS的最少时间周期
		{
			vTaskDelay(nms / fac_ms); // FreeRTOS延时
		}
		nms %= fac_ms; // OS已经无法提供这么小的延时了,采用普通方式延时
	}
	else
		Delayus((u32)(1000) * nms); // 普通方式延时
}

// 延时nms,不会引起任务调度
// nms:要延时的ms数
void Delayxms_OS(u32 nms)
{
	Delayus(1000 * nms);
}

/// @brief 获取DWT计数器的值
/// @param  无
/// @return DWT计数器的值
uint32_t get_DWT_CYCCNT(void)
{
	return DWT_CYCCNT;
}
