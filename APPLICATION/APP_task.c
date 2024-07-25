/*
 * @Author: JAR_CHOW
 * @Date: 2024-05-27 19:53:37
 * @LastEditors: JAR_CHOW
 * @LastEditTime: 2024-07-25 20:13:40
 * @FilePath: \RVMDK（uv5）c:\Users\mrchow\Desktop\vscode_repo\Intelligent-logistics-vehicle-v5.0\APPLICATION\APP_task.c
 * @Description:
 *
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved.
 */
#include "Stepper.h"
#include "APP_task.h"
#include "APP_include.h"
#include "math.h"
#include "stdlib.h"
#include "stdio.h"
#include "oled_draw.h"
#include "usart.h"

static inline void wait_can_stop(uint32_t timeout);

static void go_ahead(int32_t step, enum Stepper_Direction_t direction);

static void adjust_car_direction(const double angle);
static void adjust_car_position(const int aim_x, const int aim_y, const int16_t *position, uint8_t *flag, const uint8_t out_round);
static void adjust_car_position1(const int aim_x, const int aim_y);

char qr_code_data_[6] = {0};	   // the data of qr code
volatile uint8_t qr_code_flag = 0;			   // the flag of qr code
int16_t color_position[var_times][3] = {0, 0}; // the color position of the car
volatile uint8_t color_position_index = 0;
volatile uint8_t color_position_flag = 0;
int16_t cycle_position[2] = {0, 0}; // the cycle position of the car
volatile uint8_t cycle_position_flag = 0;
volatile char claw_state; // the state of the claw

extern struct Steeper_t *up_down_stepper_motor_handle;
extern struct Steeper_t *turntable_stepper_motor_handle;

#define init_pid_1 PID_Initialize(&adjust_x_pid, 13., 0, 0, 0, 0, 0), PID_Initialize(&adjust_y_pid, 13., 0, 0, 0, 0, 0)
#define init_pid_2 PID_Initialize(&adjust_x_pid, 20., 0, 0, 0, 0, 0), PID_Initialize(&adjust_y_pid, 20., 0, 0, 0, 0, 0)

double var_()
{
	u16 sum = 0;
	double k = 0, var = 0, avg = 0;
	u8 i = 0;
	for (i = 0; i < var_times; i++)
	{
		sum += color_position[i][1];
	}
	avg = sum * 1.0f / var_times;
	for (i = 0; i < var_times; i++)
	{
		var += pow(color_position[i][1] - avg, 2) / var_times; // 求方差
	}
	sqrt(k);
	return k;
}

static inline void set_servo_angle(uint16_t state)
{
	uint16_t angle = 0;
	if (state == 0)
	{
		TIM_SetCompare1(TIM9, 1000);
		angle = 535;
	}
	else if (state == 1)
	{
		angle = 1715;
		TIM_SetCompare1(TIM9, 2000);
	}
	else if (state == 2)
	{
		angle = 1715;
		TIM_SetCompare1(TIM9, 2000);
	}
	TIM_SetCompare1(TIM8, angle);
}

#define open 0
#define half_open 2
#define close 1
static inline void claws_operation(int state)
{
	if (state == 0)
	{
		TIM_SetCompare2(TIM8, 1400);
	}
	else if (state == 1)
	{
		TIM_SetCompare2(TIM8, 970);
	}
	else if (state == 2)
	{
		TIM_SetCompare2(TIM8, 1300);
	}
}

#define down 1
#define up 0
static void claws_up_down(int state, int step)
{
	if (state == 0)
	{
		up_down_stepper_motor_handle->Achieve_Distance(up_down_stepper_motor_handle, Stepper_Forward, step, 0x7AF, false);
	}
	else if (state == 1)
	{
		up_down_stepper_motor_handle->Achieve_Distance(up_down_stepper_motor_handle, Stepper_Backward, step, 0x7AF, false);
	}
}

/// @brief 夹取三个物体
/// @param color 颜色顺序
static void claws_action(const char *color)
{
	const uint8_t stop_instruction[3] = {0xFF, 0x0F, 0xFF};
	char get_color_instruction[3] = {0xFF, 0x02, 0xFF};
	color_position_flag = 0;
	while (!color_position_flag)
		Usart_SendArray(UART4, (const uint8_t *)get_color_instruction, 3), vTaskDelay(50);
	vTaskDelay(1);
	while (var_() > 1.1)
	{
		vTaskDelay(1);
	}
	adjust_car_position1(200, 320);
	Usart_SendArray(UART4, stop_instruction, 3);

	// 夹第一个物体
	if(color[0] == color_position[(color_position_index+var_times-1)%var_times][2])
	{
		vTaskDelay(6000);
	}

	char get_claws_instruction[3] = {0xFF, 0x04, 0x1};
	get_claws_instruction[2] = color[0];
	claw_state = 0;
	for (int i = 0; i < 3; i++)
		Usart_SendArray(UART4, (const uint8_t *)get_claws_instruction, 3);
	while (!claw_state)
		vTaskDelay(20);
	claw_state = 0;
	// claws_up_down(down, claws_from_outside_turnable_updown_dst-0x1200);
	claws_up_down(down, claws_from_outside_turnable_updown_dst);
	vTaskDelay(750);
	claws_operation(close);
	vTaskDelay(450);
	claws_up_down(up, claws_from_outside_turnable_updown_dst);
	vTaskDelay(700);
	set_servo_angle(1);
	vTaskDelay(600);
	claws_up_down(down, place_on_car_dst);
	vTaskDelay(550);
	claws_operation(open);
	vTaskDelay(200);
	claws_up_down(up, place_on_car_dst);
	claw_state = 0;
	vTaskDelay(300);
	turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);

	// 夹第二个物体
	set_servo_angle(0);
	vTaskDelay(1000);
	get_claws_instruction[2] = color[1];
	claw_state = 0;
	for (int i = 0; i < 3; i++)
		Usart_SendArray(UART4, (const uint8_t *)get_claws_instruction, 3);
	while (!claw_state)
		vTaskDelay(20);
	claw_state = 0;
	claws_up_down(down, claws_from_outside_turnable_updown_dst);
	vTaskDelay(850);
	claws_operation(close);
	vTaskDelay(450);
	claws_up_down(up, claws_from_outside_turnable_updown_dst);
	vTaskDelay(850);
	set_servo_angle(1);
	vTaskDelay(600);
	claws_up_down(down, place_on_car_dst);
	vTaskDelay(550);
	claws_operation(open);
	vTaskDelay(200);
	claws_up_down(up, place_on_car_dst);
	claw_state = 0;
	vTaskDelay(300);
	turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);

	// 夹第三个物体
	set_servo_angle(0);
	vTaskDelay(1000);
	get_claws_instruction[2] = color[2];
	claw_state = 0;
	for (int i = 0; i < 3; i++)
		Usart_SendArray(UART4, (const uint8_t *)get_claws_instruction, 3);
	while (!claw_state)
		vTaskDelay(20);
	claw_state = 0;
	claws_up_down(down, claws_from_outside_turnable_updown_dst);
	vTaskDelay(850);
	claws_operation(close);
}

/// @brief 抓三次放三次
/// @param color 颜色顺序
static void claws_place_and_grab(const char *color)
{
	// 放第一个物体
	const uint8_t stop_instruction[3] = {0xFF, 0x0F, 0xFF};
	set_servo_angle(0);
	claws_up_down(down, observe_dst);
	claws_operation(open);
	vTaskDelay(900);
	char get_cycle_instruction[3] = {0xFF, 0x03, 0xFF};
	cycle_position[0] = cycle_position[1] = 0;
	cycle_position_flag = 0;
	while (cycle_position_flag == 0)
		Usart_SendArray(UART4, (const uint8_t *)get_cycle_instruction, 3), vTaskDelay(50);
	adjust_car_position(place_aim_x, place_aim_y, cycle_position, (uint8_t *)&cycle_position_flag, 1);
	vTaskDelay(3);
	claws_up_down(up, observe_dst - (place_on_car_dst + 0x30));
	vTaskDelay(2);
	Usart_SendArray(UART4, (const uint8_t *)stop_instruction, 3);
	set_servo_angle(1);
	vTaskDelay(1000);

	claws_operation(close);
	vTaskDelay(500);
	claws_up_down(up, (place_on_car_dst + 0x30));
	vTaskDelay(300);
	set_servo_angle(0);
	turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
	vTaskDelay(700);
	claws_up_down(down, place_on_ground_dst);
	vTaskDelay(1500);
	claws_operation(open);
	vTaskDelay(300);
	claws_up_down(up, place_on_ground_dst - observe_dst);
	vTaskDelay(5);

	adjust_car_direction(-90);
	go_ahead(0x1F00 * (color[1] - color[0]), Stepper_Forward);
	wait_can_stop(50);
	vTaskDelay(300);

	// 放第二个物体
	cycle_position[0] = cycle_position[1] = 0;
	cycle_position_flag = 0;
	while (cycle_position_flag == 0)
		Usart_SendArray(UART4, (const uint8_t *)get_cycle_instruction, 3), vTaskDelay(50);
	adjust_car_position(place_aim_x, place_aim_y, cycle_position, (uint8_t *)&cycle_position_flag, 1);
	vTaskDelay(3);
	claws_up_down(up, observe_dst - (place_on_car_dst + 0x30));
	vTaskDelay(2);
	Usart_SendArray(UART4, (const uint8_t *)stop_instruction, 3);
	set_servo_angle(1);
	vTaskDelay(1000);

	claws_operation(close);
	vTaskDelay(500);
	claws_up_down(up, (place_on_car_dst + 0x30));
	vTaskDelay(300);
	set_servo_angle(0);
	turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
	vTaskDelay(700);
	claws_up_down(down, place_on_ground_dst);
	vTaskDelay(1500);
	claws_operation(open);
	vTaskDelay(300);
	claws_up_down(up, place_on_ground_dst - observe_dst);
	vTaskDelay(5);

	adjust_car_direction(-90);
	go_ahead(0x1F00 * (color[2] - color[1]), Stepper_Forward);
	wait_can_stop(50);
	vTaskDelay(300);

	// 放第三个物体
	cycle_position[0] = cycle_position[1] = 0;
	cycle_position_flag = 0;
	while (cycle_position_flag == 0)
		Usart_SendArray(UART4, (const uint8_t *)get_cycle_instruction, 3), vTaskDelay(50);
	adjust_car_position(place_aim_x, place_aim_y, cycle_position, (uint8_t *)&cycle_position_flag, 1);
	vTaskDelay(3);
	claws_up_down(up, observe_dst - (place_on_car_dst + 0x30));
	vTaskDelay(2);
	Usart_SendArray(UART4, (const uint8_t *)stop_instruction, 3);
	set_servo_angle(1);
	vTaskDelay(1000);

	claws_operation(close);
	vTaskDelay(400);
	claws_up_down(up, (place_on_car_dst + 0x30));
	vTaskDelay(300);
	set_servo_angle(0);
	turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320 * 2, 0x3F, false);
	vTaskDelay(500);
	claws_up_down(down, place_on_ground_dst);
	vTaskDelay(1600);
	claws_operation(open);
	vTaskDelay(300);
	claws_up_down(up, place_on_ground_dst - place_on_car_dst);
	vTaskDelay(100);

	// 拿第一个物体
	go_ahead(0x1F00 * (-color[(0 + 2) % 3] + color[(0 + 2 + 1) % 3]), Stepper_Forward);
	wait_can_stop(50);
	vTaskDelay(300);

	claws_up_down(down, place_on_ground_dst - place_on_car_dst);
	vTaskDelay(800);
	claws_operation(close);
	vTaskDelay(400);
	claws_up_down(up, place_on_ground_dst);
	vTaskDelay(400);
	set_servo_angle(1);
	go_ahead(0x1F00 * (-color[(1 + 2) % 3] + color[(1 + 2 + 1) % 3]), Stepper_Forward);
	vTaskDelay(800);
	claws_up_down(down, place_on_car_dst);
	vTaskDelay(700);
	claws_operation(open);
	vTaskDelay(200);
	set_servo_angle(0);
	turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
	vTaskDelay(100);

	// 拿第二个物体
	wait_can_stop(50);

	claws_up_down(down, place_on_ground_dst - place_on_car_dst);
	vTaskDelay(800);
	claws_operation(close);
	vTaskDelay(400);
	claws_up_down(up, place_on_ground_dst);
	vTaskDelay(400);
	go_ahead(0x1F00 * (-color[(2 + 2) % 3] + color[(2 + 2 + 1) % 3]), Stepper_Forward);
	set_servo_angle(1);
	vTaskDelay(800);
	claws_up_down(down, place_on_car_dst);
	vTaskDelay(700);
	claws_operation(open);
	vTaskDelay(200);
	set_servo_angle(0);
	turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
	vTaskDelay(100);

	// 拿第三个物体
	wait_can_stop(50);

	claws_up_down(down, place_on_ground_dst - place_on_car_dst);
	vTaskDelay(800);
	claws_operation(close);
	vTaskDelay(400);
	claws_up_down(up, place_on_ground_dst);
	vTaskDelay(400);
}

static void claws_place_on_object1()
{
	const uint8_t stop_instruction[3] = {0xFF, 0x0F, 0xFF};
	set_servo_angle(0);
	claws_up_down(down, observe_dst);
	claws_operation(open);
	vTaskDelay(900);
	char get_cycle_instruction[3] = {0xFF, 0x03, 0xFF};
	cycle_position[0] = cycle_position[1] = 0;
	cycle_position_flag = 0;
	while (cycle_position_flag == 0)
		Usart_SendArray(UART4, (const uint8_t *)get_cycle_instruction, 3), vTaskDelay(50);
	adjust_car_position(place_aim_x, place_aim_y, cycle_position, (uint8_t *)&cycle_position_flag, 20);
	vTaskDelay(3);
	claws_up_down(up, observe_dst - (place_on_car_dst + 0x30));
	vTaskDelay(2);
	Usart_SendArray(UART4, (const uint8_t *)stop_instruction, 3);
	vTaskDelay(300);

	set_servo_angle(1);
	vTaskDelay(900);
	claws_operation(close);
	vTaskDelay(400);
	claws_up_down(up, (place_on_car_dst + 0x30));
	vTaskDelay(300);
	turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
	set_servo_angle(0);
	vTaskDelay(700);
	claws_up_down(down, 0x1770 + adjust_dst);
	vTaskDelay(800);
	claws_operation(open);
	vTaskDelay(250);
	claws_up_down(up, 0x1770 + adjust_dst);
	vTaskDelay(5);
	vTaskDelay(100);
}

static void claws_place_on_object()
{
	// claws_operation(open);
	// vTaskDelay(300);

	// set_servo_angle(1);
	claws_up_down(down, (place_on_car_dst + 0x30));
	vTaskDelay(800);
	claws_operation(close);
	vTaskDelay(400);
	claws_up_down(up, (place_on_car_dst + 0x30));
	vTaskDelay(300);
	turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
	set_servo_angle(0);
	vTaskDelay(700);
	claws_up_down(down, 0x1770 + adjust_dst);
	vTaskDelay(800);
	claws_operation(open);
	vTaskDelay(250);
	claws_up_down(up, 0x1770 + adjust_dst);
	vTaskDelay(5);
	vTaskDelay(100);
}

static void main_task(void);
static TaskHandle_t main_task_handle = NULL;
struct PID adjust_direction_pid;
struct PID adjust_x_pid;
struct PID adjust_y_pid;

void main_task_init()
{
	TIM_SetCompare1(TIM8, 1860);
	xTaskCreate((TaskFunction_t)main_task, "main_task", 1024 * 2, NULL, 3, (TaskHandle_t *)&main_task_handle);
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
		if (times2 > (timeout * 4))
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
		vTaskDelay(5);
		times2++;
	}
}

static void adjust_car_direction(const double angle)
{
	vTaskDelay(1);
	double cur_err = (double)Angle.z / 32768 * 180 - angle;
	if (cur_err > 180)
		cur_err -= 360;
	if (cur_err < -180)
		cur_err += 360;
	// char str[33];
	while (fabs(cur_err) >= 0.18)
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
		wait_can_stop(15);
	}
}

static void adjust_car_position1(const int aim_x, const int aim_y)
{
	vTaskDelay(1);
	const double kx = -1.5, ky = -1.9;
	double err_x = aim_x - color_position[0][0];
	double err_y = aim_x - color_position[0][1];
	for (int i = 1; i < var_times; i++)
	{
		err_x += (aim_x - color_position[i][0]);
		err_y += (aim_x - color_position[i][1]);
	}
	err_x *= kx;
	err_y *= ky;

	left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, err_x > 0 ? Stepper_Backward : Stepper_Forward, abs((int)err_x), 0x4F, true);
	vTaskDelay(1);
	left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, err_x > 0 ? Stepper_Forward : Stepper_Backward, abs((int)err_x), 0x4F, true);
	vTaskDelay(1);
	right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, err_x > 0 ? Stepper_Backward : Stepper_Forward, abs((int)err_x), 0x4F, true);
	vTaskDelay(1);
	right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, err_x > 0 ? Stepper_Forward : Stepper_Backward, abs((int)err_x), 0x4F, true);
	vTaskDelay(1);
	Stepper_synchronization(USART2);
	wait_can_stop(50);

	left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, err_y > 0 ? Stepper_Backward : Stepper_Forward, abs((int)err_y), 0x4F, true);
	vTaskDelay(1);
	left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, err_y > 0 ? Stepper_Backward : Stepper_Forward, abs((int)err_y), 0x4F, true);
	vTaskDelay(1);
	right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, err_y > 0 ? Stepper_Backward : Stepper_Forward, abs((int)err_y), 0x4F, true);
	vTaskDelay(1);
	right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, err_y > 0 ? Stepper_Backward : Stepper_Forward, abs((int)err_y), 0x4F, true);
	vTaskDelay(1);
	Stepper_synchronization(USART2);
	wait_can_stop(50);
	color_position_flag = 0;

	for (int i = 0; i < 2; i++)
	{
		while (color_position_flag == 0 || var_() > 1.1)
			vTaskDelay(5);
		err_x = aim_x - color_position[(color_position_index + var_times - 1) % var_times][0];
		err_y = aim_y - color_position[(color_position_index + var_times - 1) % var_times][1];

		err_x *= kx;
		err_y *= ky;

		left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, err_x > 0 ? Stepper_Backward : Stepper_Forward, abs((int)err_x), 0x4F, true);
		vTaskDelay(1);
		left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, err_x > 0 ? Stepper_Forward : Stepper_Backward, abs((int)err_x), 0x4F, true);
		vTaskDelay(1);
		right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, err_x > 0 ? Stepper_Backward : Stepper_Forward, abs((int)err_x), 0x4F, true);
		vTaskDelay(1);
		right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, err_x > 0 ? Stepper_Forward : Stepper_Backward, abs((int)err_x), 0x4F, true);
		vTaskDelay(1);
		Stepper_synchronization(USART2);
		wait_can_stop(50);

		left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, err_y > 0 ? Stepper_Backward : Stepper_Forward, abs((int)err_y), 0x4F, true);
		vTaskDelay(1);
		left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, err_y > 0 ? Stepper_Backward : Stepper_Forward, abs((int)err_y), 0x4F, true);
		vTaskDelay(1);
		right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, err_y > 0 ? Stepper_Backward : Stepper_Forward, abs((int)err_y), 0x4F, true);
		vTaskDelay(1);
		right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, err_y > 0 ? Stepper_Backward : Stepper_Forward, abs((int)err_y), 0x4F, true);
		vTaskDelay(1);
		Stepper_synchronization(USART2);
		wait_can_stop(50);
		color_position_flag = 0;
	}
}

static void adjust_car_position(const int aim_x, const int aim_y, const int16_t *position, uint8_t *flag, const uint8_t out_round)
{
	vTaskDelay(1);
	int times = 0;
	do
	{
		while ((*flag) == 0)
			vTaskDelay(20);
		int err_x = aim_x - position[0];
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
		(*flag) = 0;
		vTaskDelay(40);
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
		int err_y = aim_y - position[1];
		int output_y = PID_Realize(&adjust_y_pid, err_y);

		left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, output_y > 0 ? Stepper_Backward : Stepper_Forward, abs(output_y), 0x2F, true);
		vTaskDelay(1);
		left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, output_y > 0 ? Stepper_Backward : Stepper_Forward, abs(output_y), 0x2F, true);
		vTaskDelay(1);
		right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, output_y > 0 ? Stepper_Backward : Stepper_Forward, abs(output_y), 0x2F, true);
		vTaskDelay(1);
		right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, output_y > 0 ? Stepper_Backward : Stepper_Forward, abs(output_y), 0x2F, true);
		vTaskDelay(1);
		Stepper_synchronization(USART2);
		wait_can_stop(50);
		(*flag) = 0;
		vTaskDelay(100);
		if (abs(err_y) <= out_round)
			times++;
		else
			times = 0;
	} while (times < 3);
}

static void turn_left_or_right(double aim)
{
	vTaskDelay(1);
	const uint32_t step = 0x3D80;
	if (aim < -1000)
	{
		left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, Stepper_Backward, step * 2, 0x80, true);
		vTaskDelay(1);
		left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, Stepper_Backward, step * 2, 0x80, true);
		vTaskDelay(1);
		right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, Stepper_Forward, step * 2, 0x80, true);
		vTaskDelay(1);
		right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, Stepper_Forward, step * 2, 0x80, true);
		vTaskDelay(1);
	}
	else if (aim < -190)
	{
		left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, Stepper_Backward, step, 0x80, true);
		vTaskDelay(1);
		left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, Stepper_Backward, step, 0x80, true);
		vTaskDelay(1);
		right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, Stepper_Forward, step, 0x80, true);
		vTaskDelay(1);
		right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, Stepper_Forward, step, 0x80, true);
		vTaskDelay(1);
	}
	else if (aim > ((double)Angle.z / 32768 * 180) || aim > 190)
	{
		left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, Stepper_Forward, step, 0x80, true);
		vTaskDelay(1);
		left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, Stepper_Forward, step, 0x80, true);
		vTaskDelay(1);
		right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, Stepper_Backward, step, 0x80, true);
		vTaskDelay(1);
		right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, Stepper_Backward, step, 0x80, true);
		vTaskDelay(1);
	}
	else
	{
		left_front_stepper_motor_handle->Achieve_Distance(left_front_stepper_motor_handle, Stepper_Backward, step, 0x80, true);
		vTaskDelay(1);
		left_rear_stepper_motor_handle->Achieve_Distance(left_rear_stepper_motor_handle, Stepper_Backward, step, 0x80, true);
		vTaskDelay(1);
		right_rear_stepper_motor_handle->Achieve_Distance(right_rear_stepper_motor_handle, Stepper_Forward, step, 0x80, true);
		vTaskDelay(1);
		right_front_stepper_motor_handle->Achieve_Distance(right_front_stepper_motor_handle, Stepper_Forward, step, 0x80, true);
		vTaskDelay(1);
	}
	Stepper_synchronization(USART2);
}

static void go_ahead(int32_t step, enum Stepper_Direction_t direction)
{
	vTaskDelay(1);
	if (step < 0)
	{
		step = -step;
		if (direction == Stepper_Forward)
			direction = Stepper_Backward;
		else
			direction = Stepper_Forward;
	}
	uint32_t speed = 0xF0;
	if (step < 0x1800)
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
	vTaskDelay(1);
	uint32_t speed = 0x98;
	if (step < 0)
	{
		step = -step;
		direction = !direction;
	}
	if (step < 0x1800)
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
	while (1)
	{

		claws_operation(close);

		// Do something
		left_side_step(0x2700, left);

		// wait for the car to stop
		wait_can_stop(100);

		// go straight
		go_ahead(0x9000, Stepper_Forward);
		vTaskDelay(1);

		wait_can_stop(150);

		// scan qr code
		while (0 == qr_code_flag)
			vTaskDelay(20);

		go_ahead(0xA150, Stepper_Forward);
		wait_can_stop(150);
		set_servo_angle(0);

		left_side_step(0x1400, right);
		claws_operation(open);
		wait_can_stop(150);

		init_pid_1;
		claws_action(qr_code_data_);

#if zhoujierong
		vTaskDelay(450);
		go_ahead(0x5500, Stepper_Forward);
		vTaskDelay(1);
		claws_up_down(up, claws_from_outside_turnable_updown_dst);
		vTaskDelay(850);
		set_servo_angle(1);
		wait_can_stop(80);
		vTaskDelay(200);
		left_side_step(0x1400, left);
		vTaskDelay(1);
		claws_up_down(down, place_on_car_dst);
		wait_can_stop(50);
		turn_left_or_right(-90);
		vTaskDelay(200);
		claws_operation(open);
		wait_can_stop(100);
		adjust_car_direction(-90);
		vTaskDelay(200);
		claws_up_down(up, place_on_car_dst);
		claw_state = 0;
		vTaskDelay(200);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320 * 2, 0x3F, false);

#else
		vTaskDelay(450);
		claws_up_down(up, claws_from_outside_turnable_updown_dst);
		vTaskDelay(850);
		set_servo_angle(1);
		vTaskDelay(500);
		claws_up_down(down, place_on_car_dst);
		vTaskDelay(650);
		claws_operation(open);
		vTaskDelay(200);
		claws_up_down(up, place_on_car_dst);
		claw_state = 0;
		vTaskDelay(200);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320 * 2, 0x3F, false);

		vTaskDelay(2);
		// 回到路中间
		set_servo_angle(2);
		claws_operation(close);
		vTaskDelay(500);
		left_side_step(0x1400, left);
		wait_can_stop(50);
		adjust_car_direction(0);
		vTaskDelay(1);

		// 去右上角
		go_ahead(0x5500, Stepper_Forward);
		wait_can_stop(150);
		turn_left_or_right(-90);
		wait_can_stop(100);
		adjust_car_direction(-90);
#endif


		vTaskDelay(1);

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

		claws_place_and_grab(qr_code_data_);

#if zhoujierong

		set_servo_angle(1);
		vTaskDelay(1);
		go_ahead(0x6F00 - 0x1E00 * (qr_code_data_[2] - 3), Stepper_Forward);
		vTaskDelay(700);
		claws_up_down(down, place_on_car_dst);
		vTaskDelay(700);
		claws_operation(open);
		vTaskDelay(200);
		claws_up_down(up, place_on_car_dst);
		vTaskDelay(5);
		wait_can_stop(50);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320 * 2, 0x3F, false);
		vTaskDelay(2);
		left_side_step(0x1200, left);
		wait_can_stop(50);

		turn_left_or_right(-180);
		wait_can_stop(100);
		adjust_car_direction(-180);

#else

		set_servo_angle(1);
		vTaskDelay(800);
		vTaskDelay(1);
		claws_up_down(down, place_on_car_dst);
		vTaskDelay(700);

		claws_operation(open);
		vTaskDelay(200);
		claws_up_down(up, place_on_car_dst);
		vTaskDelay(5);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320 * 2, 0x3F, false);
		vTaskDelay(230);

		left_side_step(0x1200, left);
		wait_can_stop(50);
		go_ahead(0x6F00 - 0x1E00 * (qr_code_data_[2] - 3), Stepper_Forward);
		vTaskDelay(100);

		// !!
		vTaskDelay(5);
		claws_operation(close);

		// 去左上角
		wait_can_stop(150);
		vTaskDelay(1);

		turn_left_or_right(-180);
		wait_can_stop(150);
		// while(1) vTaskDelay(20);
		adjust_car_direction(-180);
#endif

		if (qr_code_data_[0] == 0x1)
			go_ahead(0x8D00, Stepper_Forward);
		else if (qr_code_data_[0] == 0x2)
			go_ahead(0x8F00 + 0x1E00, Stepper_Forward);
		else if (qr_code_data_[0] == 0x3)
			go_ahead(0x8F00 + 0x1F00 + 0x1F00, Stepper_Forward);
		wait_can_stop(150);

		left_side_step(0xD00, right);
		wait_can_stop(50);

		init_pid_2;
		char get_cycle_instruction[3] = {0xFF, 0x03, 0xFF};

		// 放第一个物体
		set_servo_angle(0);
		claws_up_down(down, observe_dst);
		claws_operation(open);
		vTaskDelay(700);
		cycle_position[0] = cycle_position[1] = 0;
		cycle_position_flag = 0;
		while (cycle_position_flag == 0)
			Usart_SendArray(UART4, (const uint8_t *)get_cycle_instruction, 3), vTaskDelay(50);
		adjust_car_position(place_aim_x, place_aim_y, cycle_position, (uint8_t *)&cycle_position_flag, 1);
		vTaskDelay(3);
		claws_up_down(up, observe_dst - (place_on_car_dst + 0x30));
		vTaskDelay(2);
		Usart_SendArray(UART4, (const uint8_t *)stop_instruction, 3);
		set_servo_angle(1);
		vTaskDelay(900);

		claws_operation(close);
		vTaskDelay(700);
		claws_up_down(up, (place_on_car_dst + 0x30));
		vTaskDelay(300);
		set_servo_angle(0);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
		vTaskDelay(700);
		claws_up_down(down, place_on_ground_dst);
		vTaskDelay(1500);
		claws_operation(open);
		vTaskDelay(300);
		claws_up_down(up, place_on_ground_dst - observe_dst);
		vTaskDelay(5);

		adjust_car_direction(-180);
		go_ahead(0x1F00 * (qr_code_data_[1] - qr_code_data_[0]), Stepper_Forward);
		wait_can_stop(50);
		vTaskDelay(300);

		// 放第二个物体
		cycle_position[0] = cycle_position[1] = 0;
		cycle_position_flag = 0;
		while (cycle_position_flag == 0)
			Usart_SendArray(UART4, (const uint8_t *)get_cycle_instruction, 3), vTaskDelay(50);
		adjust_car_position(place_aim_x, place_aim_y, cycle_position, (uint8_t *)&cycle_position_flag, 1);
		vTaskDelay(3);
		claws_up_down(up, observe_dst - (place_on_car_dst + 0x30));
		vTaskDelay(2);
		Usart_SendArray(UART4, (const uint8_t *)stop_instruction, 3);
		set_servo_angle(1);
		vTaskDelay(900);

		claws_operation(close);
		vTaskDelay(700);
		claws_up_down(up, (place_on_car_dst + 0x30));
		vTaskDelay(300);
		set_servo_angle(0);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
		vTaskDelay(700);
		claws_up_down(down, place_on_ground_dst);
		vTaskDelay(1500);
		claws_operation(open);
		vTaskDelay(300);
		claws_up_down(up, place_on_ground_dst - observe_dst);
		vTaskDelay(5);

		adjust_car_direction(-180);
		go_ahead(0x1F00 * (qr_code_data_[2] - qr_code_data_[1]), Stepper_Forward);
		wait_can_stop(50);
		vTaskDelay(300);

		// 放第三个物体
		cycle_position[0] = cycle_position[1] = 0;
		cycle_position_flag = 0;
		while (cycle_position_flag == 0)
			Usart_SendArray(UART4, (const uint8_t *)get_cycle_instruction, 3), vTaskDelay(50);
		adjust_car_position(place_aim_x, place_aim_y, cycle_position, (uint8_t *)&cycle_position_flag, 1);
		vTaskDelay(3);
		claws_up_down(up, observe_dst - (place_on_car_dst + 0x30));
		vTaskDelay(2);
		Usart_SendArray(UART4, (const uint8_t *)stop_instruction, 3);
		set_servo_angle(1);
		vTaskDelay(900);

		claws_operation(close);
		vTaskDelay(700);
		claws_up_down(up, (place_on_car_dst + 0x30));
		vTaskDelay(300);
		set_servo_angle(0);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320 * 2, 0x3F, false);
		vTaskDelay(600);
		claws_up_down(down, place_on_ground_dst);
		vTaskDelay(1500);
		claws_operation(open);
		vTaskDelay(300);
		claws_up_down(up, place_on_ground_dst);
		vTaskDelay(100);

		left_side_step(0x1070, left);
		set_servo_angle(2);
		claws_operation(close);
		wait_can_stop(50);

		// ! 回去转盘的位置
		turn_left_or_right(-10000);
		wait_can_stop(200);
		adjust_car_direction(0);

		if (qr_code_data_[2] == 0x1)
			go_ahead(0x8B00, Stepper_Forward);
		else if (qr_code_data_[2] == 0x2)
			go_ahead(0x9000 + 0x1F00, Stepper_Forward);
		else if (qr_code_data_[2] == 0x3)
			go_ahead(0x9200 + 0x1F00 + 0x1F00, Stepper_Forward);
		wait_can_stop(150);

		turn_left_or_right(90);
		wait_can_stop(150);
		adjust_car_direction(90);

		go_ahead(0x96A0 + 0x1F00 + 0x1F00 + 0x6F00, Stepper_Forward);
		wait_can_stop(250);

		turn_left_or_right(0);
		wait_can_stop(150);
		adjust_car_direction(0);

		go_ahead(0x5100, Stepper_Backward);
		wait_can_stop(150);

		adjust_car_direction(0);
		vTaskDelay(2);
		left_side_step(0x1400, right);
		set_servo_angle(0);
		claws_operation(open);
		wait_can_stop(150);

		init_pid_1;
		claws_action(qr_code_data_ + 3);

#if zhoujierong
		vTaskDelay(450);
		go_ahead(0x5500, Stepper_Forward);
		vTaskDelay(1);
		claws_up_down(up, claws_from_outside_turnable_updown_dst);
		vTaskDelay(850);
		set_servo_angle(1);
		wait_can_stop(80);
		vTaskDelay(200);
		left_side_step(0x1400, left);
		vTaskDelay(1);
		claws_up_down(down, place_on_car_dst);
		wait_can_stop(50);
		turn_left_or_right(-90);
		wait_can_stop(100);
		vTaskDelay(200);
		claws_operation(open);
		adjust_car_direction(-90);
		vTaskDelay(20);
		claws_up_down(up, place_on_car_dst);
		claw_state = 0;
		vTaskDelay(200);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320 * 2, 0x3F, false);

#else
		vTaskDelay(450);
		claws_up_down(up, claws_from_outside_turnable_updown_dst);
		vTaskDelay(850);
		set_servo_angle(1);
		vTaskDelay(500);
		claws_up_down(down, place_on_car_dst);
		vTaskDelay(650);
		claws_operation(open);
		vTaskDelay(200);
		claws_up_down(up, place_on_car_dst);
		claw_state = 0;
		vTaskDelay(200);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320 * 2, 0x3F, false);

		vTaskDelay(2);
		// 回到路中间
		set_servo_angle(2);
		claws_operation(close);
		vTaskDelay(500);
		left_side_step(0x1400, left);
		wait_can_stop(50);
		adjust_car_direction(0);
		vTaskDelay(1);

		// 去右上角
		go_ahead(0x5500, Stepper_Forward);
		wait_can_stop(150);
		turn_left_or_right(-90);
		wait_can_stop(100);
		adjust_car_direction(-90);
#endif

		vTaskDelay(1);
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

		claws_place_and_grab(qr_code_data_ + 3);

#if zhoujierong

		set_servo_angle(1);
		vTaskDelay(1);
		go_ahead(0x6F00 - 0x1E00 * (qr_code_data_[5] - 3), Stepper_Forward);
		vTaskDelay(700);
		claws_up_down(down, place_on_car_dst);
		vTaskDelay(700);
		claws_operation(open);
		vTaskDelay(200);
		claws_up_down(up, place_on_car_dst);
		vTaskDelay(5);
		wait_can_stop(50);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320 * 2, 0x3F, false);
		vTaskDelay(2);
		left_side_step(0x1200, left);
		wait_can_stop(50);

		turn_left_or_right(-180);
		wait_can_stop(100);
		adjust_car_direction(-180);

#else

		set_servo_angle(1);
		vTaskDelay(800);
		vTaskDelay(1);
		claws_up_down(down, place_on_car_dst);
		vTaskDelay(700);

		claws_operation(open);
		vTaskDelay(200);
		claws_up_down(up, place_on_car_dst);
		vTaskDelay(230);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320 * 2, 0x3F, false);
		vTaskDelay(5);

		left_side_step(0x1200, left);
		wait_can_stop(50);
		go_ahead(0x6F00 - 0x1E00 * (qr_code_data_[5] - 3), Stepper_Forward);
		vTaskDelay(100);

		// !!
		vTaskDelay(5);
		claws_operation(close);

		// 去左上角
		wait_can_stop(150);
		vTaskDelay(1);

		turn_left_or_right(-180);
		wait_can_stop(150);
		// while(1) vTaskDelay(20);
		adjust_car_direction(-180);
#endif
		vTaskDelay(1);
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
			if (i == 0)
				claws_place_on_object1();
			else
				claws_place_on_object();
			// 去

			adjust_car_direction(180);
			set_servo_angle(1);
			go_ahead(0x1F00 * (qr_code_data_[i + 1 + 3] - qr_code_data_[i + 3]), Stepper_Forward);
			wait_can_stop(50);
		}

		claws_place_on_object();

		vTaskDelay(2);
		left_side_step(0x1070, left);
		vTaskDelay(1);
		turntable_stepper_motor_handle->Achieve_Distance(turntable_stepper_motor_handle, Stepper_Backward, 0x320, 0x5F, false);
		wait_can_stop(50);

		adjust_car_direction(180);
		set_servo_angle(2);
		claws_operation(close);

		go_ahead(0xAF00 - 0x1E00 * (qr_code_data_[5] - 3), Stepper_Forward);
		wait_can_stop(150);

		turn_left_or_right(-270);
		wait_can_stop(150);
		adjust_car_direction(90);

		go_ahead(0x96A0 + 0x1F00 + 0x1F00 + 0xF00 + 0x000, Stepper_Forward);
		wait_can_stop(250);
		left_side_step(0x1400, right);
		wait_can_stop(50);
		go_ahead(0x8500, Stepper_Forward);

		vTaskDelete(main_task_handle);
	}
}
