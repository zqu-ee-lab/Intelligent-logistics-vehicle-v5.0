
#include "PID.h"
/**
 * @description: 初始化PID结构体变量
 * @param {PID} pid 传入PID结构体变量
 * @param {float} KP
 * @param {float} KD
 * @param {float} KI
 * @param {float} limit
 * @return {*}
 */
void PID_Initialize(struct PID *pid, float KP, float KD, float KI, int Traget, float limit_high, float limit_low)
{
    pid->Target = Traget;
    pid->KP = KP;
    pid->KI = KI;
    pid->KD = KD;
    pid->limit_high = limit_high;
    pid->limit_low = limit_low;
    pid->Cumulation_Error = 0;
    pid->Last_Error = 0;
    pid->Previous_Error = 0;
}

//! 位置式PID控制
/**
 * @description:位置式PID控制
 * @param {PID} pid 传入PID结构体变量
 * @param {float} Target 目标值
 * @param {float} Current 当前值
 * @return {*}
 */
int PID_Realize(struct PID *pid, const float Current)
{
    
    
    float iError, Realize;                                                                                     // 实际输出
    iError = pid->Target - Current;                                                                            // 计算当前误差
    pid->Cumulation_Error += iError;                                                                           // 误差积分
    pid->Cumulation_Error = pid->Cumulation_Error > pid->limit_high ? pid->limit_high : pid->Cumulation_Error; // 积分限幅//上限
    pid->Cumulation_Error = pid->Cumulation_Error < pid->limit_low ? pid->limit_low : pid->Cumulation_Error;   // 积分限幅//下限
    Realize = pid->KP * iError + pid->Cumulation_Error * pid->KI + pid->KD * (iError - pid->Last_Error);
    pid->Last_Error = iError; // 更新上次误差
    return (int)Realize;           // 返回实际值
}


/**
 * @description: en: achieve pid of angle control
 *              cn: 实现角度控制的pid
 * @param {PID} en: the structure of pid that you want to use
 *              cn: 你想要使用的pid结构体
 * @param {float} Current en: current angle
 *                      cn: 当前角度
 * @return {int} en: output of pid
 *              cn: pid的输出
 */
int PID_Realize_angle(struct PID *pid, float Current)
{
    float iError, Realize;                                                                                     // 实际输出
    iError = pid->Target - Current;                                                                            // 计算当前误差
    // because the range of angle is -180~180, and the range of iError is -360~360, so we need to change the error to -180~180
    if (iError > 180)
    {
        iError = iError - 360;
    }
    else if (iError < -180)
    {
        iError = iError + 360;
    }
    pid->Cumulation_Error += iError;                                                                           // 误差积分
    pid->Cumulation_Error = pid->Cumulation_Error > pid->limit_high ? pid->limit_high : pid->Cumulation_Error; // 积分限幅//上限
    pid->Cumulation_Error = pid->Cumulation_Error < pid->limit_low ? pid->limit_low : pid->Cumulation_Error;   // 积分限幅//下限
    Realize = pid->KP * iError + pid->Cumulation_Error * pid->KI + pid->KD * (iError - pid->Last_Error);
    pid->Last_Error = iError; // 更新上次误差
    return (int)Realize;           // 返回实际值
}


/**
 * @description: 增量式PID算法函数
 * @param {PID} *pid
 * @param {float} Target
 * @param {float} Current
 * @return {*}
 */
int PID_Increase(struct PID *pid, int Current)
{

    float iError, // 当前误差
        Increase; // 最后得出的实际增量

    iError = pid->Target - Current; // 计算当前误差

    Increase = pid->KP * (iError - pid->Last_Error) + pid->KI * iError + pid->KD * (iError - 2 * pid->Last_Error + pid->Previous_Error); // 微分D

    pid->Previous_Error = pid->Last_Error; // 更新前次误差
    pid->Last_Error = iError;              // 更新上次误差

    return Increase; // 返回增量
}
/**
 * @description: 输出限幅函数
 * @param {int} Out_PID PID输出值
 * @param {int} Max 最大值
 * @param {int} Min 最小值
 * @return {*}
 */
int Limited_Out(int Out_PID, int Max, int Min)
{
    if (Out_PID >= Max)
    {
        Out_PID = Max;
    }
    if (Out_PID <= Min)
    {
        Out_PID = Min;
    }
    return Out_PID;
}
/**
 * @description: 串级PID
 * @param {int} *Out_PID_x
 * @param {int} *Out_PID_y
 * @return {*}
 */
void Series_PID(int *Out_PID_x, int *Out_PID_y)
{
    // int x, y;
    // x = PID_Realize(&extral, x1);
    // y = PID_Realize(&extral, y1);
    // vTaskDelay(100);
    // *Out_PID_x = 1500 + PID_Realize(&inner, x1);
    // *Out_PID_y = 1500 - PID_Realize(&inner, y1);
    // set_computer_value(SEND_TARGET_CMD, CURVES_CH1, &target_speed, 1);
}
/*
增量式与位置式区别：
1增量式算法不需要做累加，控制量增量的确定仅与最近几次偏差采样值有关，计算误差对控制 量计算的影响较小。而位置式算法要用到过去偏差的累加值，容易产生较大的累加误差。

        2增量式算法得出的是控制量的增量，例如在阀门控制中，只输出阀门开度的变化部分，误动作 影响小，必要时还可通过逻辑判断限制或禁止本次输出，不会严重影响系统的工作。 而位置式的输出直接对应对象的输出，因此对系统影响较大。

        3增量式PID控制输出的是控制量增量，并无积分作用，因此该方法适用于执行机构带积分部件的对象，如步进电机等，而位置式PID适用于执行机构不带积分部件的对象，如电液伺服阀。

        4在进行PID控制时，位置式PID需要有积分限幅和输出限幅，而增量式PID只需输出限幅

        位置式PID优缺点：
            优点：
①位置式PID是一种非递推式算法，可直接控制执行机构（如平衡小车），u(k) 的值和执行机构的实际位置（如小车当前角度）是一一对应的，因此在执行机构不带积分部件的对象中可以很好应用

        缺点：
①每次输出均与过去的状态有关，计算时要对e(k) 进行累加，运算工作量大。

        增量式PID优缺点：
        优点：
①误动作时影响小，必要时可用逻辑判断的方法去掉出错数据。
②手动 / 自动切换时冲击小，便于实现无扰动切换。当计算机故障时，仍能保持原值。
③算式中不需要累加。控制增量Δu(k) 的确定仅与最近3次的采样值有关。

        缺点：
①积分截断效应大，有稳态误差；

②溢出的影响大。有的被控对象用增量式则不太好；
*/
