/*
 * @Author: JAR_CHOW
 * @Date: 2024-05-14 20:47:46
 * @LastEditors: JAR_CHOW
 * @LastEditTime: 2024-05-27 23:27:17
 * @FilePath: \RVMDK（uv5）c:\Users\mrchow\Desktop\vscode_repo\Intelligent-logistics-vehicle-v5.0\APPLICATION\APP_mecanum_car.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#ifndef APP_MECANUM_CAR_H
#define APP_MECANUM_CAR_H

#include "APP_include.h"

void mecanum_car_init(void);
void startStraight_Line_Base_On_Encoder(float target, int forwardOrPan);
void startgostraight(float target_angle);
void start_trun(int i);

#endif // APP_MECANUM_CAR_H
