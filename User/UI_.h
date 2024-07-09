/*
 * @Author: JAR_CHOW
 * @Date: 2024-05-06 18:52:24
 * @LastEditors: JAR_CHOW
 * @LastEditTime: 2024-07-07 13:00:25
 * @FilePath: \RVMDK（uv5）c:\Users\mrchow\Desktop\vscode_repo\Intelligent-logistics-vehicle-v5.0\User\UI_.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#ifndef UI_H__
#define UI_H__

void UI_updata(void);
void UI_enter(void);
void UI_prev(void);
void UI_next(void);
void UI_back(void);
void test1(void);



#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"
#include "APP_INCLUDE.h"

#endif // UI_H__
