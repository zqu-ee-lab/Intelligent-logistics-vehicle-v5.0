/*
 * @Author: JAR_CHOW
 * @Date: 2024-05-27 19:53:37
 * @LastEditors: JAR_CHOW
 * @LastEditTime: 2024-06-24 22:23:43
 * @FilePath: \RVMDK（uv5）c:\Users\mrchow\Desktop\vscode_repo\Intelligent-logistics-vehicle-v5.0\APPLICATION\APP_task.c
 * @Description:
 *
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved.
 */
#include "Stepper.h"
#include "APP_task.h"
#include "APP_include.h"
#include "APP_mecanum_car.h"
#include "math.h"
#include "stdlib.h"
#include "stdio.h"
#include "oled_draw.h"

char qr_code_data_[6];	// the data of qr code
int16_t color_position[2]; // the color position of the car
int16_t cycle_position[2]; // the cycle position of the car
char claw_state;		// the state of the claw

extern struct Steeper_t *up_down_stepper_motor_handle;

static inline void set_servo_angle(uint16_t state)
{
	uint16_t angle = 0;
	if (state == 0)
	{
		angle = 535;
	}
	else if (state == 1)
	{
		angle = 1733;
	}
	else if (state == 2)
	{
		angle = 1860;
	}
	TIM_SetCompare1(TIM8, angle);
}

static inline void claws_operation(int state)
{
	if (state == 0)
	{
		TIM_SetCompare2(TIM8, 2000);
	}
	else if (state == 1)
	{
		TIM_SetCompare2(TIM8, 1000);
	}
}

static inline void claws_up_down(int state,int step)
{
	if (state == 0)
	{
		up_down_stepper_motor_handle->Achieve_Distance(up_down_stepper_motor_handle, Stepper_Forward, step, 0x2F, false);
	}
	else if (state == 1)
	{
		up_down_stepper_motor_handle->Achieve_Distance(up_down_stepper_motor_handle, Stepper_Backward, step, 0x2F, false);
	}
}

#ifdef pid_version
void main_task(void)
{
	while (1)
	{
		// Do something
		// go left right direction
		startStraight_Line_Base_On_Encoder(1000, 1);
		startStraight_Line_Base_On_Encoder(1000, 0);
	}
}
#else
static void main_task(void);
static TaskHandle_t main_task_handle = NULL;
struct PID adjust_direction_pid;
struct PID turn_pid;
void main_task_init()
{
	xTaskCreate((TaskFunction_t)main_task, "main_task", 1024, NULL, 3, (TaskHandle_t *)&main_task_handle);
	PID_Initialize(&adjust_direction_pid, 35, 0, 0.1, 0, 0x100, -0x100);
	PID_Initialize(&turn_pid,  1., 0.5, 0, 0, 0, 0); //! 转向PID
}

extern struct Steeper_t *left_front_stepper_motor_handle;
extern struct Steeper_t *right_front_stepper_motor_handle;
extern struct Steeper_t *left_rear_stepper_motor_handle;
extern struct Steeper_t *right_rear_stepper_motor_handle;

static inline void wait_can_stop(uint32_t timeout)
{
	int times = 0;
	int times2 = 0;
	while (1)
	{
		if(times2 > timeout) break;
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == Bit_SET)
		{
			times++;
			if (times > 5)
			{
				break;
			}
		}
		else
		{
			times = 0;
		}
		vTaskDelay(20);
		times2++;
	}
}

static inline void adjust_car_position(void)
{
	int times = 0;
	do
	{
		// 1. 读取陀螺仪的值,和位置的值，并计算误差
		// 2. 计算pid
		// 3. 设置步速
	} while (times < 5);
}

static void adjust_car_direction(const double angle)
{
	double cur_err = (double)Angle.z / 32768 * 180 - angle;
	char str[33];
	while (fabs(cur_err) > 0.02)
	{
		// 1. 读取陀螺仪的值
		cur_err = (double)Angle.z / 32768 * 180 - angle;
		// sprintf(str, "cur_err:%.2f", cur_err);
		// DrawString(4, 1, str);
		// 2. 计算pid
		int32_t output = PID_Realize_angle(&adjust_direction_pid, cur_err);
		// 3. 设置步速
		left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, output > 0 ? Stepper_Forward : Stepper_Backward, abs(output), 0x4F, true);
		vTaskDelay(1);
		left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, output > 0 ? Stepper_Forward : Stepper_Backward, abs(output), 0x4F, true);
		vTaskDelay(1);
		right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, output > 0 ? Stepper_Backward : Stepper_Forward, abs(output), 0x4F, true);
		vTaskDelay(1);
		right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, output > 0 ? Stepper_Backward : Stepper_Forward, abs(output), 0x4F, true);
		vTaskDelay(1);
		Stepper_synchronization(USART2);
		wait_can_stop(50);
	}
}

static void turn_left(double aim)
{
	uint8_t times = 4;
	turn_pid.Target = aim;
	char str[49];
	while(times){
		double cur_err = (double)Angle.z / 32768 * 180 - turn_pid.Target;
		// sprintf(str, "cur_err:%.2f", cur_err);
		// DrawString(4, 1, str);
		int output = PID_Realize_angle(&turn_pid, (double)Angle.z / 32768 * 180);
		if(abs(cur_err)<.4){
			times--;
			left_front_stepper_motor_handle->stop(left_front_stepper_motor_handle);
			vTaskDelay(1);
			left_rear_stepper_motor_handle->stop(left_rear_stepper_motor_handle);
			vTaskDelay(1);
			right_rear_stepper_motor_handle->stop(right_rear_stepper_motor_handle);
			vTaskDelay(1);
			right_front_stepper_motor_handle->stop(right_front_stepper_motor_handle);
			vTaskDelay(1);
			Stepper_synchronization(USART2);
		}
		else{
			if(output>255)output = 255;
			if(output<-255)output = -255;
			left_front_stepper_motor_handle->Speed_Control(left_front_stepper_motor_handle, output > 0 ? Stepper_Forward : Stepper_Backward, abs(output)+3, true);
			vTaskDelay(1);
			left_rear_stepper_motor_handle->Speed_Control(left_rear_stepper_motor_handle, output > 0 ? Stepper_Forward : Stepper_Backward, abs(output)+3, true);
			vTaskDelay(1);
			right_rear_stepper_motor_handle->Speed_Control(right_rear_stepper_motor_handle, output > 0 ? Stepper_Backward : Stepper_Forward, abs(output)+3, true);
			vTaskDelay(1);
			right_front_stepper_motor_handle->Speed_Control(right_front_stepper_motor_handle, output > 0 ? Stepper_Backward : Stepper_Forward, abs(output)+3, true);
			vTaskDelay(1);
			Stepper_synchronization(USART2);
		}
	}
}

		#include "usart.h"
void main_task(void)
{
	while (1)
	{
		// set_servo_angle(1);
		// claws_operation(0);

		// vTaskDelay(2000);
		// claws_up_down(1,0x500);
		// vTaskDelay(2000);
		// claws_operation(1);
		// vTaskDelay(1000);
		// claws_up_down(0,0x500);
		// vTaskDelay(1000);
		// set_servo_angle(0);
		// while(1){
		// 	vTaskDelay(100);
		// 	GPIO_SetBits(GPIOE, GPIO_Pin_1);
		// 	vTaskDelay(100);
		// 	GPIO_ResetBits(GPIOE, GPIO_Pin_1);
		// }
		// while (1)
		// 	vTaskDelay(20);
		// while(1) vTaskDelay(20);

		set_servo_angle(0);
		claws_operation(0);
		vTaskDelay(1000);
		char get_qrcode_instruction[3] = {0xFF, 0x02, 0xFF};
		for(int i=0;i<5;i++)
		Usart_SendArray(UART4, get_qrcode_instruction, 3);
		while(1) vTaskDelay(20);
		// adjust_car_direction(5);
		turn_left(-90);
		while(1) vTaskDelay(20);

		
		// Do something
		set_servo_angle(2);
		// go left right direction
		left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, Stepper_Forward, 0x9000, 0x40, true);
		vTaskDelay(1);
		left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, Stepper_Backward, 0x9000, 0x40, true);
		vTaskDelay(1);
		right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, Stepper_Forward, 0x9000, 0x40, true);
		vTaskDelay(1);
		right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, Stepper_Backward, 0x9000, 0x40, true);
		vTaskDelay(1);
		Stepper_synchronization(USART2);

		// wait for the car to stop
		wait_can_stop(300);

		// vTaskDelay(3000);

		// go straight
		left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, Stepper_Forward, 0x8000, 0x50, true);
		vTaskDelay(1);
		right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, Stepper_Forward, 0x8000, 0x50, true);
		vTaskDelay(1);
		left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, Stepper_Forward, 0x8000, 0x50, true);
		vTaskDelay(1);
		right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, Stepper_Forward, 0x8000, 0x50, true);
		vTaskDelay(1);
		Stepper_synchronization(USART2);

		wait_can_stop(300);
		set_servo_angle(0);

		// scan qr code
		while (1)
		{
			vTaskDelay(20);
		}

		// go straight to the turntable
		left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, Stepper_Forward, 0x8000, 0x100, true);
		left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, Stepper_Forward, 0x8000, 0x100, true);
		right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, Stepper_Forward, 0x8000, 0x100, true);
		right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, Stepper_Forward, 0x8000, 0x100, true);
		Stepper_synchronization(USART2);

		// wait for the car to stop
		wait_can_stop(300);

		// 对齐

		adjust_car_direction(0);

		// 夹

		// 去

		// 放

		// 回
	}
}

#endif // pid_version
