/*
 * ......................................&&.........................
 * ....................................&&&..........................
 * .................................&&&&............................
 * ...............................&&&&..............................
 * .............................&&&&&&..............................
 * ...........................&&&&&&....&&&..&&&&&&&&&&&&&&&........
 * ..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............
 * ................&...&&&&&&&&&&&&&&&&&&&&&&&&&&&&.................
 * .......................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........
 * ...................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...............
 * ..................&&&   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * ...............&&&&&@  &&&&&&&&&&..&&&&&&&&&&&&&&&&&&&...........
 * ..............&&&&&&&&&&&&&&&.&&....&&&&&&&&&&&&&..&&&&&.........
 * ..........&&&&&&&&&&&&&&&&&&...&.....&&&&&&&&&&&&&...&&&&........
 * ........&&&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&....&&&.......
 * .......&&&&&&&&.....................&&&&&&&&&&&&&&&&.....&&......
 * ........&&&&&.....................&&&&&&&&&&&&&&&&&&.............
 * ..........&...................&&&&&&&&&&&&&&&&&&&&&&&............
 * ................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * ..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&..&&&&&............
 * ..............&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&....&&&&&............
 * ...........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&......&&&&............
 * .........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........&&&&............
 * .......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&............
 * ......&&&&&&&&&&&&&&&&&&&...&&&&&&...............&&&.............
 * .....&&&&&&&&&&&&&&&&............................&&..............
 * ....&&&&&&&&&&&&&&&.................&&...........................
 * ...&&&&&&&&&&&&&&&.....................&&&&......................
 * ...&&&&&&&&&&.&&&........................&&&&&...................
 * ..&&&&&&&&&&&..&&..........................&&&&&&&...............
 * ..&&&&&&&&&&&&...&............&&&.....&&&&...&&&&&&&.............
 * ..&&&&&&&&&&&&&.................&&&.....&&&&&&&&&&&&&&...........
 * ..&&&&&&&&&&&&&&&&..............&&&&&&&&&&&&&&&&&&&&&&&&.........
 * ..&&.&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&&&&&&&&&&&&.......
 * ...&&..&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&...&&&&&&&&&&&&......
 * ....&..&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&&&&&.....
 * .......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............&&&&&&&....
 * .......&&&&&.&&&&&&&&&&&&&&&&&&..&&&&&&&&...&..........&&&&&&....
 * ........&&&.....&&&&&&&&&&&&&.....&&&&&&&&&&...........&..&&&&...
 * .......&&&........&&&.&&&&&&&&&.....&&&&&.................&&&&...
 * .......&&&...............&&&&&&&.......&&&&&&&&............&&&...
 * ........&&...................&&&&&&.........................&&&..
 * .........&.....................&&&&........................&&....
 * ...............................&&&.......................&&......
 * ................................&&......................&&.......
 * .................................&&..............................
 * ..................................&..............................
 *
 * @Author: zhaojianchao jar-chou 2722642511@qq.com
 * @Date: 2023-09-06 13:02:19
 * @LastEditors: jar-chou 2722642511@qq.com
 * @LastEditTime: 2023-09-07 13:50:25
 * @FilePath: \delivering_car\User\main.c
 * @Description: 龙王保佑此文件无bug！！！
 */

/*
*************************************************************************
*                             包含的头文件
*************************************************************************
*/
/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "queue.h"
/* 开发板硬件bsp头文件 */
#include "bsp_led.h"
#include "usart.h"
#include "delay.h"
#include "PID.h"
#include "stdio.h"
#include "OLED.h"
#include "sys.h"
#include "key.h"
#include "PWM.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include "encoder.h"
#include "UI_.h"

// user header file
#include "Stepper.h"
#include "SysInfoTest.h"
#include "my_math.h"
#include "oled_draw.h"
#include "oled_buffer.h"

/**************************** 任务句柄 ********************************/
/*
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */


QueueHandle_t Task_Number_Handle = NULL;

static TaskHandle_t analyse_data_Handle = NULL;	 //+解析数据定时器句柄
static TaskHandle_t OLED_SHOW_Handle = NULL;	 //+OLDE显示句柄
static TaskHandle_t AppTaskCreate_Handle = NULL; //+创建任务句柄
static TaskHandle_t KEY_SCAN_Handle = NULL;		 //+KEY_SCAN句柄
static TaskHandle_t Task_schedule_Handle = NULL; //+SysInfoTestSent句柄

void avoid_warning()
{
	Task_schedule_Handle=Task_schedule_Handle;
}
EventGroupHandle_t Group_One_Handle = NULL; //+事件组句柄

/******************************* Global variable declaration ************************************/
/*
 * 当我们在写应用程序的时候，可能需要用到一些全局变量。
 */

/*
*************************************************************************
*                             函数声明
*************************************************************************
*/
static void analyse_data(void);
static void AppTaskCreate(void);			   /* 用于创建任务 */
static void OLED_SHOW(void *pvParameters);	   /* Test_Task任务实现 */
static void KEY_SCAN(void *pvParameters);	   /* Test_Task任务实现 */
static void BSP_Init(void); /* 用于初始化板载相关资源 */
static void USER_Init(void);

/*****************************************************************
  * @brief  主函数
  * @param  无
  * @retval 无
  * @note   第一步：开发板硬件初始化
			第二步：创建APP应用任务
			第三步：启动FreeRTOS，开始多任务调度
  ****************************************************************/
int main(void)
{
	BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为pdPASS */

	/* 开发板硬件初始化 */
	BSP_Init();

	USER_Init();

	USART1_DMA_TranData((uint8_t *)"hello world\r\n", 13);

	/* 创建AppTaskCreate任务 */
	xReturn = xTaskCreate((TaskFunction_t)AppTaskCreate,		  /* 任务入口函数 */
						  (const char *)"AppTaskCreate",		  /* 任务名字 */
						  (uint16_t)1024,						  /* 任务栈大小 */
						  (void *)NULL,							  /* 任务入口函数参数 */
						  (UBaseType_t)1,						  /* 任务的优先级 */
						  (TaskHandle_t *)&AppTaskCreate_Handle); /* 任务控制块指针 */
	/* 启动任务调度 */
	if (pdPASS == xReturn)
		vTaskStartScheduler(); /* 启动任务，开启调度 */
	else
		return -1;

	while (1)
		; /* 正常不会执行到这里 */
}

/***********************************************************************
 * @ 函数名  ： AppTaskCreate
 * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
 * @ 参数    ： 无
 * @ 返回值  ： 无
 **********************************************************************/
static void AppTaskCreate(void)
{
	BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为pdPASS */

	taskENTER_CRITICAL(); // 进入临界区

	/* 创建Test_Task任务 */

#if configGENERATE_RUN_TIME_STATS && configUSE_STATS_FORMATTING_FUNCTIONS
	xReturn = xTaskCreate((TaskFunction_t)SysInfoTestSent,			/* 任务入口函数 */
						  (const char *)"SysInfoTestSent",			/* 任务名字 */
						  (uint16_t)512,							/* 任务栈大小 */
						  (void *)NULL,								/* 任务入口函数参数 */
						  (UBaseType_t)1,							/* 任务的优先级 */
						  (TaskHandle_t *)&SysInfoTestSent_Handle); /* 任务控制块指针 */
	if (xReturn == pdPASS)
		printf("SysInfoTestSent任务创建成功\r\n");
#endif
	xReturn = xTaskCreate((TaskFunction_t)KEY_SCAN,			 /* 任务入口函数 */
						  (const char *)"KEY_SCAN",			 /* 任务名字 */
						  (uint16_t)512,					 /* 任务栈大小 */
						  (void *)NULL,						 /* 任务入口函数参数 */
						  (UBaseType_t)10,					 /* 任务的优先级 */
						  (TaskHandle_t *)&KEY_SCAN_Handle); /* 任务控制块指针 */
	if (xReturn == pdPASS)
		App_Printf("KEY_SCAN任务创建成功\r\n");
	xReturn = xTaskCreate((TaskFunction_t)OLED_SHOW,		  /* 任务入口函数 */
						  (const char *)"OLED_SHOW",		  /* 任务名字 */
						  (uint16_t)512,					  /* 任务栈大小 */
						  (void *)NULL,						  /* 任务入口函数参数 */
						  (UBaseType_t)3,					  /* 任务的优先级 */
						  (TaskHandle_t *)&OLED_SHOW_Handle); /* 任务控制块指针 */
	if (xReturn == pdPASS)
		App_Printf("OLED_SHOW任务创建成功\r\n");
	xReturn = xTaskCreate((TaskFunction_t)analyse_data,
						  (const char *)"analyse_data",
						  (uint16_t)256,						 /* 任务栈大小 */
						  (void *)NULL,							 /* 任务入口函数参数 */
						  (UBaseType_t)7,						 /* 任务的优先级 */
						  (TaskHandle_t *)&analyse_data_Handle); /* 任务控制块指针 */
	if (xReturn == pdPASS)
		App_Printf("analyse_data任务创建成功\r\n");

	Task_Number_Handle = xQueueCreate(1, 1); // 开始解析数据
	Group_One_Handle = xEventGroupCreate();
	SET_EVENT(GAME_OVER);

	// 挂机任务，等待选择任务
	vTaskDelete(AppTaskCreate_Handle); // 删除AppTaskCreate任务

	taskEXIT_CRITICAL(); // 退出临界区
}

/**
 * @description: this function is the software callback function that analyse data
 * @return {*}
 */
static void analyse_data(void)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(15); // 10ms
	xLastWakeTime = xTaskGetTickCount();
	for (;;)
	{
		/* code */


		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

/**
 * @description: this function is the software callback function that send data to upper computer
 * @return {*}
 */
void sendto_Upper(void *parameter)
{
		float VOFA_Data[4];
    VOFA_Data[0] = 0;
    VOFA_Send_float(VOFA_Data, 4); //! 发送数据给VOFA
}



/**
 * the following code is about the task that we create
 */

/**
 * @description: this task is is used to show necessary information on the OLED
 * @param {void} *parameter :this param is necessary for freertos task
 * @return {*}
 */
static void OLED_SHOW(void *pvParameters)
{

	while (1)
	{
		// enter critical area
		taskENTER_CRITICAL();
		UpdateScreenDisplay();
		// exit critical area
		taskEXIT_CRITICAL();
		vTaskDelay(200);
	}
}

/**
 * @description: this task is including the main logic of the program
 * @param {void} *parameter :this param is necessary for freertos task
 * @return {*}
 */
static void KEY_SCAN(void *parameter)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(10); // 10ms
	xLastWakeTime = xTaskGetTickCount();
	uint8_t Key_Value = 0;
	while (1)
	{
		/* 按键扫描 */
		bsp_KeyScan();
		Key_Value = bsp_GetKey();
		switch (Key_Value)
			{
			case up___:
				UI_prev();
				break;
			case enter___:
				UI_enter();
				break;
			case down___:
				UI_next();
				break;
			case back___:
				UI_back();
				break;
			default:
				break;
			}
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

/**
 * the code that create the task is end in here
 */
struct Steeper_t *left_front_stepper_motor_handle = NULL;
struct Steeper_t *right_front_stepper_motor_handle = NULL;
struct Steeper_t *left_rear_stepper_motor_handle = NULL;
struct Steeper_t *right_rear_stepper_motor_handle = NULL;
static void USER_Init(void)
{
#if configGENERATE_RUN_TIME_STATS && configUSE_STATS_FORMATTING_FUNCTIONS
	vSetupSysInfoTest();
#endif
	left_front_stepper_motor_handle = Stepper_Init(USART2, 0x01, U2_buffer_handle, Stepper_Check_Way_0X6B, Stepper_FOC_Version_5_0);
	left_rear_stepper_motor_handle = Stepper_Init(USART2, 0x02, U2_buffer_handle, Stepper_Check_Way_0X6B, Stepper_FOC_Version_5_0);
	right_rear_stepper_motor_handle = Stepper_Init(USART2, 0x03, U2_buffer_handle, Stepper_Check_Way_0X6B, Stepper_FOC_Version_5_0);
	right_front_stepper_motor_handle = Stepper_Init(USART2, 0x04, U2_buffer_handle, Stepper_Check_Way_0X6B, Stepper_FOC_Version_5_0);

	test1();
	UI_updata();
}

/***********************************************************************
 * @ 函数名  ： BSP_Init
 * @ 功能说明： 板级外设初始化，所有板子上的初始化均可放在这个函数里面
 * @ 参数    ：
 * @ 返回值  ： 无
 ***********************************************************************/
static void BSP_Init(void)
{
	/*
	 * STM32中断优先级分组为4，即4bit都用来表示抢占优先级，范围为：0~15
	 * 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断，
	 * 都统一用这个优先级分组，千万不要再分组，切忌。
	 */
	RCC_ClocksTypeDef get_rcc_clock;
	RCC_GetClocksFreq(&get_rcc_clock);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	delay_init(168);
	bsp_InitKey();
	Delayms(1000);
	OLED_Init();
	OLED_ShowString(1,1,"hello");
	PWM_TIM8_config(20000, 168, 500, 2500, 2000/3*2+500, 2);

	Init_USART1_All(); //*调试信息输出
	Init_USART2_All(); //*USART2 _stepper_motor

	Buzzer_ONE();
	Delayms(1);

	GPIO_SetBits(GPIOE, GPIO_Pin_1);
}

///********************************END OF FILE****************************/
