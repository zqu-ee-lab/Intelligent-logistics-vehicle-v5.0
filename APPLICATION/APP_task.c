/*
 * @Author: JAR_CHOW
 * @Date: 2024-05-27 19:53:37
 * @LastEditors: JAR_CHOW
 * @LastEditTime: 2024-07-11 22:53:02
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
#include "usart.h"

static void adjust_car_position(const int aim_x, const int aim_y, const int16_t *position, uint8_t *flag, const uint8_t out_round);

char qr_code_data_[6] = {3, 1, 2, 3, 2, 1}; // the data of qr code
int16_t color_position[2] = {0, 0};			// the color position of the car
volatile uint8_t color_position_flag = 0;
int16_t cycle_position[2] = {0, 0}; // the cycle position of the car
volatile uint8_t cycle_position_flag = 0;
volatile char claw_state; // the state of the claw

extern struct Steeper_t *up_down_stepper_motor_handle;
extern struct Steeper_t *turntable_stepper_motor_handle;

#define init_pid_1 PID_Initialize(&adjust_x_pid, 13., 0, 0, 0, 0, 0), PID_Initialize(&adjust_y_pid, 13., 0, 0, 0, 0, 0)
#define init_pid_2 PID_Initialize(&adjust_x_pid, 23., 0, 0, 0, 0, 0), PID_Initialize(&adjust_y_pid, 23., 0, 0, 0, 0, 0)

static inline void set_servo_angle(uint16_t state)
{
	uint16_t angle = 0;
	if (state == 0)
	{
		angle = 535;
	}
	else if (state == 1)
	{
		angle = 1720;
	}
	else if (state == 2)
	{
		angle = 1720;
	}
	TIM_SetCompare1(TIM8, angle);
}

#define open 0
#define close 1
static inline void claws_operation(int state)
{
	if (state == 0)
	{
		TIM_SetCompare2(TIM8, 2000);
	}
	else if (state == 1)
	{
		TIM_SetCompare2(TIM8, 970);
	}
}

#define down 1
#define up 0
static void claws_up_down(int state, int step)
{
	if (state == 0)
	{
		up_down_stepper_motor_handle->Achieve_Distance(up_down_stepper_motor_handle, Stepper_Forward, step, 0x8AF, false);
	}
	else if (state == 1)
	{
		up_down_stepper_motor_handle->Achieve_Distance(up_down_stepper_motor_handle, Stepper_Backward, step, 0x8AF, false);
	}
}

static void claws_action(char color)
{
	set_servo_angle(0);
	claws_operation(open);

	vTaskDelay(1000);

	char get_claws_instruction[3] = {0xFF, 0x04, 0x1};
	get_claws_instruction[2] = color;
	claw_state = 0;
	for (int i = 0; i < 3; i++)
		Usart_SendArray(UART4, (const uint8_t *)get_claws_instruction, 3);
	// while(1) vTaskDelay(20);
	while (!claw_state)
		vTaskDelay(20);
	claw_state = 0;
	claws_up_down(down, 0x1770);
	vTaskDelay(1500);
	claws_operation(close);
	vTaskDelay(1000);
	claws_up_down(up, 0x1770);
	vTaskDelay(2000);
	set_servo_angle(1);
	vTaskDelay(1000);
	claws_up_down(down, 0x8A0);
	vTaskDelay(1000);
	claws_operation(open);
	vTaskDelay(1000);
	claws_up_down(up, 0x8A0);
	vTaskDelay(1000);
	turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
}

static void claws_place_on_ground()
{
	const uint8_t stop_instruction[3] = {0xFF, 0x0F, 0xFF};
	set_servo_angle(0);
	claws_up_down(down, 0x1200);
	claws_operation(open);
	vTaskDelay(600);
	char get_cycle_instruction[3] = {0xFF, 0x03, 0xFF};
	cycle_position[0] = cycle_position[1] = 0;
	cycle_position_flag = 0;
	while (cycle_position_flag == 0)
		Usart_SendArray(UART4, (const uint8_t *)get_cycle_instruction, 3), vTaskDelay(50);
	adjust_car_position(197, 330, cycle_position, (uint8_t *)&cycle_position_flag, 1);
	vTaskDelay(3);
	claws_up_down(up, 0x1200);
	vTaskDelay(2);
	Usart_SendArray(UART4, (const uint8_t *)stop_instruction, 3);
	vTaskDelay(1000);
	
	set_servo_angle(1);
	claws_up_down(down, 0x930);
	vTaskDelay(600);
	claws_operation(close);
	vTaskDelay(700);
	claws_up_down(up, 0x930);
	vTaskDelay(300);
	set_servo_angle(0);
	vTaskDelay(1000);
	claws_up_down(down, 0x3570);
	vTaskDelay(1500);
	claws_operation(open);
	vTaskDelay(400);
	claws_up_down(up, 0x3570);
	vTaskDelay(5);
	turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
	vTaskDelay(1500);
}

static void claws_grab_from_ground()
{
	// const uint8_t stop_instruction[3] = {0xFF, 0x0F, 0xFF};
	set_servo_angle(0);
	claws_operation(open);
	vTaskDelay(1000);
	// char get_cycle_instruction[3] = {0xFF, 0x03, 0xFF};
	// cycle_position[0] = cycle_position[1] = 0;
	// cycle_position_flag = 0;
	// while (cycle_position_flag == 0)
	// 	Usart_SendArray(UART4, (const uint8_t *)get_cycle_instruction, 3), vTaskDelay(90);
	// adjust_car_position(200, 320, cycle_position, (uint8_t *)&cycle_position_flag, 5);
	// Usart_SendArray(UART4, (const uint8_t *)stop_instruction, 3);

	claws_up_down(down, 0x3500);
	vTaskDelay(1200);
	claws_operation(close);
	vTaskDelay(600);
	claws_up_down(up, 0x3500);
	vTaskDelay(700);
	set_servo_angle(1);
	vTaskDelay(500);
	claws_up_down(down, 0x900);
	vTaskDelay(1000);
	claws_operation(open);
	vTaskDelay(1000);
	claws_up_down(up, 0x900);
	vTaskDelay(100);
	turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
	vTaskDelay(100);
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
struct PID adjust_x_pid;
struct PID adjust_y_pid;

void main_task_init()
{
	xTaskCreate((TaskFunction_t)main_task, "main_task", 1024*2, NULL, 3, (TaskHandle_t *)&main_task_handle);
	PID_Initialize(&adjust_direction_pid, 180, 0, 0.1, 0, 0x100, -0x100);
	init_pid_1;
}

void main_task_deinit()
{
	vTaskDelete(main_task_handle);
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
		if (times2 > timeout)
			break;
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

static void adjust_car_direction(const double angle)
{
	double cur_err = (double)Angle.z / 32768 * 180 - angle;
	if (cur_err > 180)
		cur_err -= 360;
	if (cur_err < -180)
		cur_err += 360;
	// char str[33];
	while (fabs(cur_err) >= 0.05)
	{
		// 1. 读取陀螺仪的值
		cur_err = (double)Angle.z / 32768 * 180 - angle;
		if (cur_err > 180)
			cur_err -= 360;
		if (cur_err < -180)
			cur_err += 360;
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

static void adjust_car_position(const int aim_x, const int aim_y, const int16_t *position, uint8_t *flag, const uint8_t out_round)
{
	int times = 0;
	do
	{
		while ((*flag) == 0)
			vTaskDelay(20);
		int err_x = aim_x - position[0];
		(*flag) = 0;
		// int err_y = aim_y - position[1];
		int output_x = PID_Realize(&adjust_x_pid, err_x);
		// int output_y = PID_Realize(&adjust_y_pid, err_y);

		left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, output_x > 0 ? Stepper_Backward : Stepper_Forward, abs(output_x), 0x4F, true);
		vTaskDelay(1);
		left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, output_x > 0 ? Stepper_Forward : Stepper_Backward, abs(output_x), 0x4F, true);
		vTaskDelay(1);
		right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, output_x > 0 ? Stepper_Backward : Stepper_Forward, abs(output_x), 0x4F, true);
		vTaskDelay(1);
		right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, output_x > 0 ? Stepper_Forward : Stepper_Backward, abs(output_x), 0x4F, true);
		vTaskDelay(1);
		Stepper_synchronization(USART2);
		wait_can_stop(50);
		vTaskDelay(400);
		if (abs(err_x) <= out_round)
			times++;
		else
			times = 0;
	} while (times < 5);
	times = 0;
	do
	{
		while ((*flag) == 0)
			vTaskDelay(20);
		(*flag) = 0;
		int err_y = aim_y - position[1];
		int output_y = PID_Realize(&adjust_y_pid, err_y);

		left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, output_y > 0 ? Stepper_Backward : Stepper_Forward, abs(output_y), 0x4F, true);
		vTaskDelay(1);
		left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, output_y > 0 ? Stepper_Backward : Stepper_Forward, abs(output_y), 0x4F, true);
		vTaskDelay(1);
		right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, output_y > 0 ? Stepper_Backward : Stepper_Forward, abs(output_y), 0x4F, true);
		vTaskDelay(1);
		right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, output_y > 0 ? Stepper_Backward : Stepper_Forward, abs(output_y), 0x4F, true);
		vTaskDelay(1);
		Stepper_synchronization(USART2);
		wait_can_stop(50);
		vTaskDelay(40);
		if (abs(err_y) <= out_round)
			times++;
		else
			times = 0;
	} while (times < 5);
}

static void turn_left_or_right(double aim)
{
	const uint32_t step = 0x3D80;
	if (aim < -190)
	{
		left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, Stepper_Backward, step, 0x60, true);
		vTaskDelay(1);
		left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, Stepper_Backward, step, 0x60, true);
		vTaskDelay(1);
		right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, Stepper_Forward, step, 0x60, true);
		vTaskDelay(1);
		right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, Stepper_Forward, step, 0x60, true);
		vTaskDelay(1);
	}
	else if (aim > ((double)Angle.z / 32768 * 180) || aim > 190)
	{
		left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, Stepper_Forward, step, 0x60, true);
		vTaskDelay(1);
		left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, Stepper_Forward, step, 0x60, true);
		vTaskDelay(1);
		right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, Stepper_Backward, step, 0x60, true);
		vTaskDelay(1);
		right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, Stepper_Backward, step, 0x60, true);
		vTaskDelay(1);
	}
	else
	{
		left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, Stepper_Backward, step, 0x40, true);
		vTaskDelay(1);
		left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, Stepper_Backward, step, 0x40, true);
		vTaskDelay(1);
		right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, Stepper_Forward, step, 0x40, true);
		vTaskDelay(1);
		right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, Stepper_Forward, step, 0x40, true);
		vTaskDelay(1);
	}
	Stepper_synchronization(USART2);
}

static void go_ahead(int32_t step, enum Stepper_Direction_t direction)
{
	if (step < 0)
	{
		step = -step;
		if (direction == Stepper_Forward)
			direction = Stepper_Backward;
		else
			direction = Stepper_Forward;
	}
	uint32_t speed = 0xA0;
	if (step < 0x1000)
		speed = 0x30;
	left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, direction, step, speed, true);
	vTaskDelay(1);
	right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, direction, step, speed, true);
	vTaskDelay(1);
	left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, direction, step, speed, true);
	vTaskDelay(1);
	right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, direction, step, speed, true);
	vTaskDelay(1);
	Stepper_synchronization(USART2);
}

#define left false
#define right true
static void left_side_step(int32_t step, bool direction)
{
	uint32_t speed = 0x98;
	if(step < 0)
	{
		step = -step;
		direction = !direction;
	}
	if(step<0x1000)
		speed = 0x30;
	if (direction)
	{
		left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, Stepper_Forward, step, speed, true);
		vTaskDelay(1);
		left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, Stepper_Backward, step, speed, true);
		vTaskDelay(1);
		right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, Stepper_Forward, step, speed, true);
		vTaskDelay(1);
		right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, Stepper_Backward, step, speed, true);
		vTaskDelay(1);
	}
	else
	{
		left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, Stepper_Backward, step, speed, true);
		vTaskDelay(1);
		left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, Stepper_Forward, step, speed, true);
		vTaskDelay(1);
		right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, Stepper_Backward, step, speed, true);
		vTaskDelay(1);
		right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, Stepper_Forward, step, speed, true);
		vTaskDelay(1);
	}
	Stepper_synchronization(USART2);
}

void main_task(void)
{
	char stop_instruction[3] = {0xFF, 0x0F, 0xFF};
	char get_color_instruction[3] = {0xFF, 0x02, 0xFF};
	while (1)
	{
		// 测试

		// set_servo_angle(0);
		// claws_operation(open);
		// while (!color_position_flag)
		// 	Usart_SendArray(UART4, (const uint8_t *)get_color_instruction, 3), vTaskDelay(100);
		// while(1) vTaskDelay(20);

		

		set_servo_angle(2);
		claws_operation(close);

		// Do something
		set_servo_angle(2);
		// go left right direction
		left_side_step(0x2700, left);

		// wait for the car to stop
		wait_can_stop(100);

		// vTaskDelay(3000);

		// go straight
		go_ahead(0x9000, Stepper_Forward);
		vTaskDelay(1);

		wait_can_stop(150);

		// scan qr code
		// while (1)
		// {
		// 	vTaskDelay(20);
		// }

		go_ahead(0xA0F0, Stepper_Forward);
		wait_can_stop(150);
		set_servo_angle(0);

		left_side_step(0x1600, right);
		claws_operation(open);
		wait_can_stop(150);

		// char get_color_instruction[3] = {0xFF, 0x02, 0xFF};
		color_position_flag = 0;
		while (!color_position_flag)
			Usart_SendArray(UART4, (const uint8_t *)get_color_instruction, 3), vTaskDelay(50);
		vTaskDelay(1);
		init_pid_1;
		adjust_car_position(200, 320, color_position, (uint8_t *)&color_position_flag, 80);
		Usart_SendArray(UART4, (const uint8_t *)stop_instruction, 3);
		adjust_car_direction(0);

		// 夹
		for (int i = 0; i < 3; i++)
		{
			claws_action(qr_code_data_[i]);
		}
		vTaskDelay(2000);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
		vTaskDelay(200);

		// 回到路中间
		set_servo_angle(2);
		claws_operation(close);
		left_side_step(0x1400, left);
		wait_can_stop(150);

		// 去右上角
		go_ahead(0x5500, Stepper_Forward);
		wait_can_stop(150);
		turn_left_or_right(-90);
		wait_can_stop(150);
		adjust_car_direction(-90);

		// 去到暂存区
		if (qr_code_data_[0] == 0x1)
			go_ahead(0x96A0, Stepper_Forward);
		else if (qr_code_data_[0] == 0x2)
			go_ahead(0x96A0 + 0x1E00, Stepper_Forward);
		else if (qr_code_data_[0] == 0x3)
			go_ahead(0x96A0 + 0x1F00 + 0x1F00, Stepper_Forward);
		wait_can_stop(150);

		left_side_step(0xD00, right);
		wait_can_stop(50);

		init_pid_2;

		for (int i = 0; i < 2; i++)
		{
			// 放
			claws_place_on_ground();
			// 去
			adjust_car_direction(-90);
			go_ahead(0x1F00 * (qr_code_data_[i + 1] - qr_code_data_[i]), Stepper_Forward);
			wait_can_stop(50);
		}

		claws_place_on_ground();
		adjust_car_direction(-90);
		vTaskDelay(700);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
		vTaskDelay(200);
		
		// 拿
		for (int i = 0; i < 3; i++)
		{
			go_ahead(0x1F00 * (-qr_code_data_[(i + 2) % 3] + qr_code_data_[(i + 2 + 1) % 3]), Stepper_Forward);
			wait_can_stop(50);
			claws_grab_from_ground();
			// adjust_car_direction(-90);
		}
		vTaskDelay(1000);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
		vTaskDelay(5);

		set_servo_angle(2);
		claws_operation(close);

		left_side_step(0x1200, left);
		wait_can_stop(50);

		go_ahead(0x6F00 - 0x1E00 * (qr_code_data_[2] - 3), Stepper_Forward);
		wait_can_stop(150);

		turn_left_or_right(-180);
		wait_can_stop(150);
		// while(1) vTaskDelay(20);
		adjust_car_direction(-180);

		if (qr_code_data_[0] == 0x1)
			go_ahead(0x8F00, Stepper_Forward);
		else if (qr_code_data_[0] == 0x2)
			go_ahead(0x8F00 + 0x1E00, Stepper_Forward);
		else if (qr_code_data_[0] == 0x3)
			go_ahead(0x8F00 + 0x1F00 + 0x1F00, Stepper_Forward);
		wait_can_stop(150);

		left_side_step(0xD00, right);
		wait_can_stop(50);

		init_pid_2;

		for (int i = 0; i < 2; i++)
		{
			// 放
			claws_place_on_ground();
			// 去
			adjust_car_direction(180);
			go_ahead(0x1F00 * (qr_code_data_[i + 1] - qr_code_data_[i]), Stepper_Forward);
			wait_can_stop(50);
		}

		claws_place_on_ground();
		adjust_car_direction(180);

		left_side_step(0x1070, left);

		set_servo_angle(2);
		claws_operation(close);

		vTaskDelay(700);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
		wait_can_stop(50);

		// ! 回去转盘的位置
		turn_left_or_right(-90);
		wait_can_stop(150);
		turn_left_or_right(0);
		wait_can_stop(150);

		if (qr_code_data_[2] == 0x1)
			go_ahead(0x8F00, Stepper_Forward);
		else if (qr_code_data_[2] == 0x2)
			go_ahead(0x8F00 + 0x1E00, Stepper_Forward);
		else if (qr_code_data_[2] == 0x3)
			go_ahead(0x8F00 + 0x1F00 + 0x1F00, Stepper_Forward);
		wait_can_stop(150);

		turn_left_or_right(90);
		wait_can_stop(150);
		adjust_car_direction(90);

		go_ahead(0x96A0 + 0x1F00 + 0x1F00 + 0x6F00, Stepper_Forward);
		wait_can_stop(250);

		turn_left_or_right(0);
		wait_can_stop(150);
		adjust_car_direction(0);

		go_ahead(0x5500, Stepper_Backward);
		wait_can_stop(150);

		adjust_car_direction(0);
		vTaskDelay(1000);
		left_side_step(0x1400, right);
		vTaskDelay(2);
		set_servo_angle(0);
		claws_operation(open);
		wait_can_stop(150);

		color_position_flag = 0;
		while (!color_position_flag)
			Usart_SendArray(UART4, (const uint8_t *)get_color_instruction, 3), vTaskDelay(50);
		vTaskDelay(100);
		init_pid_1;
		adjust_car_position(200, 320, color_position, (uint8_t *)&color_position_flag, 80);
		Usart_SendArray(UART4, (const uint8_t *)stop_instruction, 3);

		for (int i = 3; i < 6; i++)
		{
			claws_action(qr_code_data_[i]);
		}
		vTaskDelay(800);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
		vTaskDelay(100);

		// 回到路中间
		set_servo_angle(2);
		claws_operation(close);
		left_side_step(0x1400, left);
		wait_can_stop(150);
		adjust_car_direction(0);

		go_ahead(0x5500, Stepper_Forward);
		wait_can_stop(150);
		turn_left_or_right(-90);
		wait_can_stop(150);
		adjust_car_direction(-90);
		if (qr_code_data_[3] == 0x1)
			go_ahead(0x96A0, Stepper_Forward);
		else if (qr_code_data_[3] == 0x2)
			go_ahead(0x96A0 + 0x1E00, Stepper_Forward);
		else if (qr_code_data_[3] == 0x3)
			go_ahead(0x96A0 + 0x1F00 + 0x1F00, Stepper_Forward);
		wait_can_stop(150);

		left_side_step(0xD00, right);
		wait_can_stop(50);

		init_pid_2;

		for (int i = 0; i < 2; i++)
		{
			// 放
			claws_place_on_ground();
			adjust_car_direction(-90);
			// 去
			go_ahead(0x1F00 * (qr_code_data_[i + 1 + 3] - qr_code_data_[i + 3]), Stepper_Forward);
			wait_can_stop(50);
		}
		claws_place_on_ground();
		adjust_car_direction(-90);
		vTaskDelay(700);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
		vTaskDelay(200);

		for (int i = 0; i < 3; i++)
		{
			go_ahead(0x1F00 * (-qr_code_data_[(i + 2) % 3 + 3] + qr_code_data_[(i + 2 + 1) % 3 + 3]), Stepper_Forward);
			wait_can_stop(50);
			claws_grab_from_ground();
			adjust_car_direction(-90);
		}
		vTaskDelay(700);
		left_side_step(0x1200, left);
		vTaskDelay(1);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
		wait_can_stop(50);

		set_servo_angle(2);
		claws_operation(close);

		vTaskDelay(1);

		go_ahead(0x6F00 - 0x1E00 * (qr_code_data_[5] - 3), Stepper_Forward);
		wait_can_stop(150);

		turn_left_or_right(-180);
		wait_can_stop(150);
		adjust_car_direction(-180);

		if (qr_code_data_[3] == 0x1)
			go_ahead(0x8F00, Stepper_Forward);
		else if (qr_code_data_[3] == 0x2)
			go_ahead(0x8F00 + 0x1E00, Stepper_Forward);
		else if (qr_code_data_[3] == 0x3)
			go_ahead(0x8F00 + 0x1F00 + 0x1F00, Stepper_Forward);
		wait_can_stop(150);

		left_side_step(0xD00, right);
		wait_can_stop(50);

		init_pid_2;

		for (int i = 0; i < 2; i++)
		{
			// 放
			claws_place_on_ground();
			// 去
			adjust_car_direction(180);
			go_ahead(0x1F00 * (qr_code_data_[i + 1 + 3] - qr_code_data_[i + 3]), Stepper_Forward);
			wait_can_stop(50);
		}

		claws_place_on_ground();

		left_side_step(0x1070, left);
		vTaskDelay(700);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
		wait_can_stop(50);
		
		adjust_car_direction(180);
		set_servo_angle(2);
		claws_operation(close);

		go_ahead(0xAF00 - 0x1E00*(qr_code_data_[5]-3), Stepper_Forward);
		wait_can_stop(150);

		turn_left_or_right(-270);
		wait_can_stop(150);
		adjust_car_direction(90);

		go_ahead(0x96A0 + 0x1F00 + 0x1F00 + 0x6F00, Stepper_Forward);
		wait_can_stop(250);

		vTaskDelete(main_task_handle);
		while (1)
			vTaskDelay(100);

		// 回
	}
}

#endif // pid_version
