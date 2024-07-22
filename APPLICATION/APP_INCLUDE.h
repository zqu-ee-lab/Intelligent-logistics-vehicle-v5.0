/*
 * @Author: JAR_CHOW
 * @Date: 2024-05-14 20:47:46
 * @LastEditors: JAR_CHOW
 * @LastEditTime: 2024-07-22 20:48:52
 * @FilePath: \RVMDK（uv5）c:\Users\mrchow\Desktop\vscode_repo\Intelligent-logistics-vehicle-v5.0\APPLICATION\APP_INCLUDE.h
 * @Description:
 *
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved.
 */
#ifndef __APP_INCLUDE_H__
#define __APP_INCLUDE_H__

#include "APP_task.h"
#include "APP_mecanum_car.h"

#include "FreeRTOS.h"
#include "task.h"
#include "delay.h"
#include "buffer.h"
#include "PID.h"

void main_task_init(void);
void main_task_deinit(void);

#define var_times 5

#define adjust_dst (0xB50)

#define claws_from_outside_turnable_updown_dst (0x1070+adjust_dst)
#define place_on_car_dst (0x9C0+adjust_dst)
#define observe_dst (0x1550+adjust_dst)
#define place_on_ground_dst (0x3530+adjust_dst)

#define place_aim_x (213)
#define place_aim_y (320)

extern struct angle Angle;                   // the angle of the car
extern char qr_code_data_[6];                // the data of qr code
extern volatile uint8_t qr_code_flag;        // the flag of qr code
extern int16_t color_position[var_times][3]; // the color position of the car
extern volatile uint8_t color_position_index;
extern volatile uint8_t color_position_flag; // the flag of color position
extern int16_t cycle_position[2];            // the cycle position of the car
extern volatile uint8_t cycle_position_flag; // the flag of cycle position
extern volatile char claw_state;             // the state of the claw

#endif // __APP_INCLUDE_H__
