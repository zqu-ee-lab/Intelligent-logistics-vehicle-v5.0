
#ifndef PID_
#define PID_
#include "stm32f4xx.h"
struct PID
{
    int Target;
    float KP, KI, KD, limit_high, limit_low;
    float Cumulation_Error; // 当前误差
    float Last_Error;       // 上一次误差
    float Previous_Error;   // 上上次误差
};
void PID_Initialize(struct PID *pid, float KP, float KD, float KI, int Traget, float limit_high, float limit_low);
int PID_Realize(struct PID *pid, const float Current);
int PID_Realize_angle(struct PID *pid, float Current);
int PID_Increase(struct PID *pid, int Current);
void Series_PID(int *Out_PID_x, int *Out_PID_y);
int Limited_Out(int Out_PID, int Max, int Min);

#endif
