#include "APP_mecanum_car.h"
#include "mecanum.h"

/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "queue.h"


#include "sys.h"
#include <string.h>
#include "encoder.h"
#include "my_math.h"
#include <math.h>
#include <stdlib.h>
#include "PID.h"
#include "mecanum.h"

TimerHandle_t Turn_Angle_Handle = NULL;                       //+发送数据到上位机定时器句柄
TimerHandle_t sendto_Upper_Handle = NULL;                     //+发送数据到上位机定时器句柄
TimerHandle_t line_walking_Handle = NULL;                     //+巡线PID定时器句柄
TimerHandle_t Achieve_Distance_For_Front_Laser_Handle = NULL; // 根据激光来跑直线
TimerHandle_t Achieve_Distance_For_Right_Laser_Handle = NULL;
TimerHandle_t Car_Running_Handle = NULL;
TimerHandle_t Go_Forward_Base_On_Encoder_Handle = NULL;
TimerHandle_t Pan_Left_Base_On_Encoder_Handle = NULL;
TimerHandle_t Keep_The_Car_Have_Special_Right_Distance_Handle = NULL;

// define variables
struct distance
{
    u16 F_OUT;
    u16 R_OUT;
} Distance;

#define maximum_Y_accelerate_rate 100
int16_t maximum_Y_speed = 1900;

mecanum_car_speed_t car_speed = {.y_speed = 0, .x_speed = 0, .w_speed = 0};
int32_t position_of_car[3] = {0};
int32_t speed_of_car[3] = {0};
u8 already_turned = 0, Y_have_achieved = 0, X_have_achieved = 0; // 是否达到定时器目的的信号量

struct angle Angle; // the angle of the car
extern struct Buff VL53_USARTX_Buff;
struct PID Coord, Turn_Angle_PID, X_Speed_PID, Y_Speed_PID;
/**********************************************************************
 * @ 函数名  ： Timer_Task
 * @ 功能说明： Timer_Task任务主体
 * @ 参数    ：
 * @ 返回值  ： 无
 ********************************************************************/

void APP_MECANUM_CAR_INIT()
{
	// pid初始化
	PID_Initialize(&Coord, 55, 3, 0, 0, 0, -0);				  // 微调巡线的pid初始化
	PID_Initialize(&Turn_Angle_PID, 25, 0, 12, 0, 7.5, -7.5); // 转弯的pid初始化
	PID_Initialize(&X_Speed_PID, 3.75, 0, .5, 0, 125, -125);  // x方向的远距离基于编码器的pid
	PID_Initialize(&Y_Speed_PID, 0.75, 0., 4., 0, 30, -30);	  // y方向的远距离基于编码器的pid
}



/**
 * @description: this function is the software callback function that achieve pan left base on encoder num
 * @return {*}
 */
void Pan_Left_Base_On_Encoder(TimerHandle_t xTimer)
{
    static int i = 0, timeout = 0;
	timeout=timeout;
    int32_t distance = position_of_car[1];
    int32_t speed = -(int32_t)PID_Realize(&X_Speed_PID, distance);

    // limit the range of X_Speed
    VAL_LIMIT(speed, -500, 500);

    // assign the value of pid output to global variable
    taskENTER_CRITICAL();
    car_speed.x_speed = speed;
    taskEXIT_CRITICAL();
    if (abs(distance - X_Speed_PID.Target) < 15)
    {
        i++;
        if (i > 4)
        {
            i = 0;
            xTimerStop(Pan_Left_Base_On_Encoder_Handle, 0);
            X_have_achieved = 1;
        }
    }
}

/**
 * @description: this function is the software callback function that achieve go forward base on encoder num
 * @return {*}
 */
void Go_Forward_Base_On_Encoder(TimerHandle_t xTimer)
{
    static int i = 0, timeout = 0;
	timeout=timeout;
    int32_t distance = position_of_car[0];
    int32_t speed = -(int32_t)PID_Realize(&Y_Speed_PID, distance);
    static int32_t last_speed = 0;

    // limit the range of Y_Speed
    VAL_LIMIT(speed, -maximum_Y_speed, maximum_Y_speed);

    // limit the accelerate rate of Y_Speed
    if (speed > 0)
    {
        if (speed - last_speed > maximum_Y_accelerate_rate)
            speed = last_speed + maximum_Y_accelerate_rate;
    }
    else
    {
        if (speed - last_speed < -maximum_Y_accelerate_rate)
            speed = last_speed - maximum_Y_accelerate_rate;
    }
    last_speed = speed;

    // assign the value of pid output to global variable
    taskENTER_CRITICAL();
    car_speed.y_speed = speed;
    taskEXIT_CRITICAL();
    if (abs(distance - Y_Speed_PID.Target) < 20)
    {
        i++;
        if (i > 2)
        {
            i = 0;
            xTimerStop(Go_Forward_Base_On_Encoder_Handle, 0);
            Y_have_achieved = 1;
        }
    }
}



/**
 * @description: this function is the software callback function that achieve turn angle
 * @return {*}
 */
void Turn_Angle(TimerHandle_t xTimer)
{
    static u8 i = 0, timeout = 0;
    float angle;
    int PIDOUT;
    // get the angle of the car, and calculate the pid output
    angle = (float)Angle.z / 32768 * 180;
    PIDOUT = -PID_Realize_angle(&Turn_Angle_PID, angle);

    // limit the range of pid output
    VAL_LIMIT(PIDOUT, -1300, 1300);

    // assign the value of pid output to global variable
    taskENTER_CRITICAL();
    car_speed.w_speed = (int32_t)PIDOUT;
    taskEXIT_CRITICAL();

    // if the car have turned, then stop this timer
    if ((fabs(angle - Turn_Angle_PID.Target) < 1) || (fabs(angle - Turn_Angle_PID.Target) > 359))
    {
        i++;
        if (i > 4)
        {
            i = 0;
            xTimerStop(Turn_Angle_Handle, 0);
            already_turned = 1;
        }
    }
    if (speed_of_car[2] == 0)
    {
        timeout++;
        if (timeout > 50)
        {
            timeout = 0;
            xTimerStop(Turn_Angle_Handle, 0);
            already_turned = 1;
        }
    }
    else
        timeout = 0;
}

/**
 * @description: this function is the software callback function that keep the car walking straightly
 * @return {*}
 */
void line_walking(TimerHandle_t xTimer)
{
    float angle;
    int PIDOUT;
    //? PID开始巡线
    angle = (float)Angle.z / 32768 * 180;
    // angle_speed = -PID_Realize(&Coord, angle);

    // !code in here need to test
    PIDOUT = -PID_Realize_angle(&Coord, angle);

    // limit the range of pid output
    VAL_LIMIT(PIDOUT, -500, 500);

    // assign the value of pid output to global variable
    taskENTER_CRITICAL();
    car_speed.w_speed = PIDOUT;
    taskEXIT_CRITICAL();
    return;
}

/**
 * @description: this function is the software callback function that calculates the ccr register value of the wheel
 * @return {*}
 */
void Car_Running(TimerHandle_t xTimer)
{
    taskENTER_CRITICAL(); // enter critical zone and operate global variable

    // get the value of global variable
    mecanum_car_speed_t local_car_speed = {.y_speed = car_speed.y_speed, .x_speed = car_speed.x_speed, .w_speed = car_speed.w_speed};

    // clear the value of global variable
    car_speed.y_speed = 0;
    car_speed.x_speed = 0;
    car_speed.w_speed = 0;

    taskEXIT_CRITICAL(); // exit critical zone

    int32_t CCR_wheel[4] = {0};
    // calculate the ccr register value of the wheel
    mecanum_calculate(&local_car_speed, CCR_wheel);

    // change the direction of the wheel
    for (size_t i = 0; i < 4; i++)
    {
        if (CCR_wheel[i] > 0) // forward
        {
            Advance(i + 2); // it is because the wheel is from 2 to 5
        }
        else // backward
        {
            Back(i + 2);                  // it is because the wheel is from 2 to 5
            CCR_wheel[i] = -CCR_wheel[i]; // change the value to positive, because the function "SetCompare1" need a positive value
        }
    }

    // set the ccr register value of the wheel
    SetCompare1(TIM1, CCR_wheel[0], 1);
    SetCompare1(TIM1, CCR_wheel[1], 2);
    SetCompare1(TIM1, CCR_wheel[2], 3);
    SetCompare1(TIM1, CCR_wheel[3], 4);
}



/**
 * the software timer callback function is end in here
 */

/**
 * the following code is about open the software timer
 */


/**
 * @description:    Follow the encoder and go straight
 *                  cn:根据编码器来跑直线
 * @param {float} target        what is the distance of encoder you want?
 * @param {int} forwardOrPan    which is the encoder that you want to refer to? When the value is equal to 1, it indicates the encoder on the front of the reference
 * @return {*}
 */
void startStraight_Line_Base_On_Encoder(float target, int forwardOrPan)
{
    if (forwardOrPan == 0)
    {
        X_Speed_PID.Target = target;
        X_Speed_PID.Cumulation_Error = 0;
        X_Speed_PID.Last_Error = target - position_of_car[1];
        X_have_achieved = 0;
        position_of_car[1] = 0;
        xTimerStart(Pan_Left_Base_On_Encoder_Handle, 1);
        xTimerStart(Car_Running_Handle, 0);
    }
    else if (forwardOrPan == 1) // 根据编码器向前走
    {
        Y_Speed_PID.Target = target;
        Y_Speed_PID.Cumulation_Error = 0;
        Y_Speed_PID.Last_Error = target - position_of_car[0];
        Y_have_achieved = 0;
        position_of_car[0] = 0;
        xTimerStart(Go_Forward_Base_On_Encoder_Handle, 1);
        xTimerStart(Car_Running_Handle, 0);
    }
    return;
}

/**
 * @description: start to Keep the direction Angle of the car
 *              cn:开始保持车的方向角
 * @param {float} target_angle : the angle that you want to keep
 * @return {*}
 */
void startgostraight(float target_angle)
{
    Coord.Target = target_angle;
    Coord.Cumulation_Error = 0;
    Coord.Last_Error = target_angle - (float)Angle.z / 32768 * 180;
    xTimerStart(line_walking_Handle, 1);
}

/**
 * @description: start to turn left or turn right
 * @param {int} i car only turn left when the value of its only param "i" is equal to 1,turn right when it's equal to zero
 * @return {*}
 */
void start_trun(int i)
{
    float angle;
    angle = (float)Angle.z / 32768 * 180;
    if (i == 1)
        Turn_Angle_PID.Target = angle - 90;
    else if (i == 0)
        Turn_Angle_PID.Target = angle + 90;
    already_turned = 0;
    xTimerStart(Turn_Angle_Handle, 0);
    xTimerStart(Car_Running_Handle, 0);
}
