#ifndef Encoder__
#define Encoder__

#include "stm32f4xx.h"
#define ENCODER_TURNS 13   //编码器线数
#define REDUCTION_RATIO 30 //减速比
#define PERIMETER 10       //车轮周长
#define CYCLE 10           //周期

void TIMX_Delay_Init(uint32_t ClOCK, uint16_t TIM_PERIOD, uint16_t TIM_PRESCALER, TIM_TypeDef *TIMX);
void Encoder_Init_TIM2(void);
void Encoder_Init(void);
int Read_Encoder(u8 TIMX);
#endif
