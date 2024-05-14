/*
*********************************************************************************************************
*
*	模块名称 : 定时器时基
*	文件名称 : SysInfoTest.c
*	版    本 : V1.0
*	说    明 : 为了获取FreeRTOS的任务信息，需要创建一个定时器，这个定时器的时间基准精度要高于
*              系统时钟节拍。这样得到的任务信息才准确。
*              本文件提供的函数仅用于测试目的，切不可将其用于实际项目，原因有两点：
*               1. FreeRTOS的系统内核没有对总的计数时间做溢出保护。
*               2. 定时器中断是50us进入一次，比较影响系统性能。
*              --------------------------------------------------------------------------------------
*              本文件使用的是32位变量来保存50us一次的计数值，最大支持计数时间：
*              2^32 * 50us / 3600s = 59.6分钟。使用中测试的任务运行计数和任务占用率超过了59.6分钟将不准确。
*
*	修改记录 :
*		版本号    日期        作者     说明
*		V1.0    2015-08-19  Eric2013   首发
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

/*
NOTE：必须在FreeRTOSConfig.h 把configGENERATE_RUN_TIME_STATS和configUSE_STATS_FORMATTING_FUNCTIONS define成1
*/
#include "bsp_base_tim.h"
#include "stm32f4xx.h"                  // Device header
#include "FreeRTOS.h"
#include "task.h"
#include "key.h"


/* 定时器频率，50us一次中断 */
#define  timerINTERRUPT_FREQUENCY	20000

/* 中断优先级 */
#define  timerHIGHEST_PRIORITY		2

/* 被系统调用 */
volatile uint32_t ulHighFrequencyTimerTicks = 0UL;

TaskHandle_t SysInfoTestSent_Handle = NULL;       			//+任务2句柄

/*
*********************************************************************************************************
*	函 数 名: vSetupTimerTest
*	功能说明: 创建定时器
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void vSetupSysInfoTest(void)
{
	bsp_SetTIMforInt(TIM6, timerINTERRUPT_FREQUENCY, timerHIGHEST_PRIORITY, 0);
}

#if configGENERATE_RUN_TIME_STATS&&configUSE_STATS_FORMATTING_FUNCTIONS
/**
 * @description: this task is used to control the car,make it go to the red area in the right hand side firstly,then go to the red area in the left hand side
 * @param {void} *parameter :this param is necessary for freertos task
 * @return {*}
 */
void SysInfoTestSent(void *parameter)
{
	uint8_t ucKeyCode;
	uint8_t pcWriteBuffer[500];

    while(1)
    {
		ucKeyCode = bsp_GetKey();
		
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				/* K1键按下 打印任务执行情况 */
				case KEY_DOWN_K1:			 
					printf("=================================================\r\n");
					printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
					vTaskList((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);
				
					printf("\r\n任务名       运行计数         使用率\r\n");
					vTaskGetRunTimeStats((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);
					break;
				
				/* K2键按下，启动单次定时器中断，50ms后在定时器中断给任务vTaskMsgPro发送消息 */
				case KEY_DOWN_K2:
					printf("K2键按下，启动单次定时器中断，50ms后在定时器中断给任务vTaskMsgPro发送消息\r\n");
					break;
				
				/* K3键按下，启动单次定时器中断，50ms后在定时器中断给任务vTaskLED发送消息 */
				case KEY_DOWN_K3:
					printf("K3键按下，启动单次定时器中断，50ms后在定时器中断给任务vTaskLED发送消息\r\n");
					break;

				/* 其他的键值不处理 */
				default:                     
					break;
			}
		}
		
		vTaskDelay(20);
	}
}

#endif


/*
*********************************************************************************************************
*	函 数 名: TIM6_DAC_IRQHandler
*	功能说明: TIM6中断服务程序。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void TIM6_DAC_IRQHandler( void )
{
	if(TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
	{
		ulHighFrequencyTimerTicks++;
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
