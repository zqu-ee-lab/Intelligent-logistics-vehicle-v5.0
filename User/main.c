/*
 * @Author: JAR_CHOW
 * @Date: 2024-05-14 20:47:46
 * @LastEditors: JAR_CHOW
 * @LastEditTime: 2024-07-09 21:10:33
 * @FilePath: \RVMDK（uv5）c:\Users\mrchow\Desktop\vscode_repo\Intelligent-logistics-vehicle-v5.0\User\main.c
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
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
#include "APP_INCLUDE.h"

/**************************** 任务句柄 ********************************/
/*
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */

QueueHandle_t Task_Number_Handle = NULL;

static TimerHandle_t analyse_data_Handle = NULL; //+解析数据定时器句柄
static TaskHandle_t OLED_SHOW_Handle = NULL;	 //+OLDE显示句柄
static TaskHandle_t AppTaskCreate_Handle = NULL; //+创建任务句柄
static TaskHandle_t KEY_SCAN_Handle = NULL;		 //+KEY_SCAN句柄
static TaskHandle_t Task_schedule_Handle = NULL; //+SysInfoTestSent句柄

void avoid_warning()
{
	Task_schedule_Handle = Task_schedule_Handle;
}

struct angle Angle; // the angle of the car


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
static void AppTaskCreate(void);		   /* 用于创建任务 */
static void OLED_SHOW(void *pvParameters); /* Test_Task任务实现 */
static void KEY_SCAN(void *pvParameters);  /* Test_Task任务实现 */
static void BSP_Init(void);				   /* 用于初始化板载相关资源 */
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

extern void main_task_init(void);
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
	// xReturn = xTaskCreate((TaskFunction_t)analyse_data,
	// 					  (const char *)"analyse_data",
	// 					  (uint16_t)256,						 /* 任务栈大小 */
	// 					  (void *)NULL,							 /* 任务入口函数参数 */
	// 					  (UBaseType_t)7,						 /* 任务的优先级 */
	// 					  (TaskHandle_t *)&analyse_data_Handle); /* 任务控制块指针 */
	// if (xReturn == pdPASS)
	// 	App_Printf("analyse_data任务创建成功\r\n");

	// software callback function
	analyse_data_Handle = xTimerCreate("analyse_data", pdMS_TO_TICKS(15), pdTRUE, (void *)0, (TimerCallbackFunction_t)analyse_data);

	

	Task_Number_Handle = xQueueCreate(1, 1); // 开始解析数据
	Group_One_Handle = xEventGroupCreate();
	SET_EVENT(GAME_OVER);

	// 挂机任务，等待选择任务
	vTaskDelete(AppTaskCreate_Handle); // 删除AppTaskCreate任务

	xTimerStart(analyse_data_Handle, 0);

	taskEXIT_CRITICAL(); // 退出临界区
}

/**
 * @description: this function is the software callback function that analyse data
 * @return {*}
 */
static void analyse_data(void)
{
	u8 check_sum;
    const char TOFSENSE[] = {0x55, 0x53};
	if(BUFF_pop_by_Protocol(&U1_buffer, TOFSENSE, 2, Angle.data, 9) == 9){
		check_sum = (0x55 + 0x53 + Angle.data[5] + Angle.data[4] + Angle.data[7] + Angle.data[6]);
		if (Angle.data[8] == check_sum)
		{
			Angle.z = -((Angle.data[5] << 8) + Angle.data[4]);
			printf("%.2f\r\n", (float)Angle.z / 32768 * 180);
		}
	}
	
	const char head_qr_code[] = {0xFF, 0x01};
	if (BUFF_pop_with_check_by_Protocol(&U4_buffer, head_qr_code, 2, qr_code_data_, 8, 1, 6) == 6){
		// char str[33];
		// sprintf(str, "%c%c%c%c%c%c", qr_code_data_[0]+'0',qr_code_data_[1]+'0',qr_code_data_[2]+'0',qr_code_data_[3]+'0',qr_code_data_[4]+'0',qr_code_data_[5]+'0');
		// DrawString(4, 1, str);
		// const uint8_t nmb[3]={0xFF, 0xFF,0xFF};
		// Usart_SendArray(UART5, nmb, 3);
	}

	const char head_color_position[] = {0xFF, 0x02};
	if(BUFF_pop_with_check_by_Protocol(&U4_buffer, head_color_position, 2, color_position, 16, 1, 2) == 2){
		char str[33];
		// int16_t temp = color_position[0];
		// color_position[0] = color_position[1];
		// color_position[1] = temp;
		color_position[0]^=color_position[1];
		color_position[1]^=color_position[0];
		color_position[0]^=color_position[1];
		sprintf(str, "%d %d", color_position[0], color_position[1]);
		color_position_flag = 1;
		DrawString(4, 1, str);
	}

	const char head_cycle[] = {0xFF, 0x03};
	if(BUFF_pop_with_check_by_Protocol(&U4_buffer, head_cycle, 2, cycle_position, 16, 1, 2) == 2){
		cycle_position_flag = 1;
		char str[33];
		// int16_t temp = cycle_position[0];
		// cycle_position[0] = cycle_position[1];
		// cycle_position[1] = temp;
		cycle_position[0]^=cycle_position[1];
		cycle_position[1]^=cycle_position[0];
		cycle_position[0]^=cycle_position[1];
		sprintf(str, "%d %d", cycle_position[0], cycle_position[1]);
		DrawString(4, 1, str);
	}

	const char head_claws[] = {0xFF, 0x04};
	if(BUFF_pop_with_check_by_Protocol(&U4_buffer, head_claws, 2, &claw_state, 8, 1, 1) == 1){
		if(claw_state != 1){
			claw_state = 0;
		}
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
	char str[70];
	uint32_t start_time = 0;
	uint32_t end_time = 0;
	float fps = 0;
	while (1)
	{
		// enter critical area
		taskENTER_CRITICAL();
		start_time = get_DWT_CYCCNT();
		DrawString(6, 1, "       ");

		sprintf(str, "angle:%.2f", (float)Angle.z / 32768 * 180);
		DrawString(5, 1, str);

		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == Bit_SET)
		{
			sprintf(str, "stop");
		}
		else if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == Bit_RESET)
		{
			sprintf(str, "running");
		}
		DrawString(6, 1, str);
		sprintf(str, "fps:%.2f", fps);
		// sprintf(str, "%4d %4d",TIM8->CCR1,TIM8->CCR2);
		DrawString(7, 1, str);
		UpdateScreenDisplay();
		// exit critical area
		end_time = get_DWT_CYCCNT();
		fps = (float)SystemCoreClock / (end_time - start_time);
		taskEXIT_CRITICAL();
		vTaskDelay(200);
	}


    // DrawPicture(4, 0, 32, 32, (const uint8_t*)bigone);
	// DrawPicture(4, 32, 32, 32, (const uint8_t*)bigtwo);
	// DrawPicture(4, 64, 32, 32, (const uint8_t*)bigthree);
	// UpdateScreenDisplay();
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
struct Steeper_t *up_down_stepper_motor_handle = NULL;
struct Steeper_t *turntable_stepper_motor_handle = NULL;
static void USER_Init(void)
{
#if configGENERATE_RUN_TIME_STATS && configUSE_STATS_FORMATTING_FUNCTIONS
	vSetupSysInfoTest();
#endif
	left_front_stepper_motor_handle = Stepper_Init(USART2, 0x02, U2_buffer_handle, Stepper_Check_Way_0X6B, Stepper_FOC_Version_5_0);
	left_rear_stepper_motor_handle = Stepper_Init(USART2, 0x01, U2_buffer_handle, Stepper_Check_Way_0X6B, Stepper_FOC_Version_5_0);
	right_rear_stepper_motor_handle = Stepper_Init(USART2, 0x04, U2_buffer_handle, Stepper_Check_Way_0X6B, Stepper_FOC_Version_5_0);
	right_front_stepper_motor_handle = Stepper_Init(USART2, 0x03, U2_buffer_handle, Stepper_Check_Way_0X6B, Stepper_FOC_Version_5_0);
	right_rear_stepper_motor_handle->direction_invert = 1;
	right_front_stepper_motor_handle->direction_invert = 1;


	
	up_down_stepper_motor_handle = Stepper_Init(USART2, 0x05, U2_buffer_handle, Stepper_Check_Way_0X6B, Stepper_FOC_Version_5_0);
	up_down_stepper_motor_handle->direction_invert = 1;
	turntable_stepper_motor_handle = Stepper_Init(USART2, 0x06, U2_buffer_handle, Stepper_Check_Way_0X6B, Stepper_FOC_Version_5_0);

	// turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Forward, 0x400, 0x900, false);

	// while(1)
	// Delayms(1000);

	// up_down_stepper_motor_handle->Achieve_Distance(up_down_stepper_motor_handle, Stepper_Forward, 0x400, 0x100, false);
	

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
	OLED_ShowString(1, 1, "hello");
	// Iinitial_BUFF(&U1_buffer, BUFFER_SIZE_U1);
	// Iinitial_BUFF(&U2_buffer, BUFFER_SIZE_U2);
	// Iinitial_BUFF(&U3_buffer, BUFFER_SIZE_U3);
	// Iinitial_BUFF(&U4_buffer, BUFFER_SIZE_U4);
	// Iinitial_BUFF(&U5_buffer, BUFFER_SIZE_U5);
	PWM_TIM8_config(20000, 168, 1860, 2000, 10000, 2);
	PWM_TIM9_config(2000, 84, 1000, 0);

	Init_USART1_All(); //*调试信息输出
	Init_USART2_All(); //*USART2 _stepper_motor
	Init_UART4_All();  //*linux
	Init_UART5_All();  //*


	Buzzer_ONE();
	Delayms(1);

	GPIO_SetBits(GPIOE, GPIO_Pin_1);
	// Usart_SendArray(UART5, "mr_chow\n", 8);
	// while(1){
	// 	char data[12];
	// 	if(BUFF_pop_by_Protocol(&U5_buffer, (char *)"mr_chow", 7, data, 8) == 8){
	// 		Usart_SendArray(UART5,"mr_chow",7);
	// 		GPIO_ResetBits(GPIOE, GPIO_Pin_1);
	// 	}
	// 	Delayms(50);
	// 	GPIO_ResetBits(GPIOE, GPIO_Pin_1);
	// 	Delayms(50);
	// 	GPIO_SetBits(GPIOE, GPIO_Pin_1);
	// }
}

///********************************END OF FILE****************************/
