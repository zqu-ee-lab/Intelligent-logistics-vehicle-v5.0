/*
 * @Author: JAR_CHOW
 * @Date: 2024-05-14 20:47:46
 * @LastEditors: JAR_CHOW
 * @LastEditTime: 2024-06-24 22:24:56
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

extern struct angle Angle; // the angle of the car
extern char qr_code_data_[6]; // the data of qr code
extern int16_t color_position[2]; // the color position of the car
extern int16_t cycle_position[2]; // the cycle position of the car
extern char claw_state; // the state of the claw

#endif // __APP_INCLUDE_H__
