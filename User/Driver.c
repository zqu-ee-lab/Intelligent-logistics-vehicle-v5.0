/*
!硬件连接图!
   *TIM2           TIM3*
?A->CHANNEL1     A->CHANNEL1
     A15             A6
?B->CHANNEL2     B->CHANNEL2
     B3              A7
?控制引脚
1 D10  2 D11     3 D15  4 D14


   *TIM4           TIM5*
?A->CHANNEL1     A->CHANNEL1
     D12             A0
?B->CHANNEL2     B->CHANNEL2
     D13             A1
?控制引脚
1 D3  2 D4       3 D7  4 D6
*/

/*//!初始化
    Back(2);
    Advance(3);
    Advance(4);
    Advance(5);
    Initial_Control_PIN();
*/
#include "Driver.h"
struct TIMX
{
    uint8_t Circle;
    int Cumulation_value;
};
struct TIMX TIM5_ = {0, 0};
struct TIMX TIM4_ = {0, 0};
struct TIMX TIM3_ = {0, 0};
struct TIMX TIM2_ = {0, 0};

void Control_PIN(uint16_t GPIO_Pin_X, GPIO_TypeDef *GPIOX)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_X;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOX, &GPIO_InitStructure);
}
void Initial_Control_PIN()
{
    u8 i;
    u16 GPIO_Pin_X[] = {GPIO_Pin_10, GPIO_Pin_11, GPIO_Pin_15, GPIO_Pin_14, GPIO_Pin_3, GPIO_Pin_4, GPIO_Pin_7, GPIO_Pin_6};
    for (i = 0; i < 8; i++)
    {
        Control_PIN(GPIO_Pin_X[i], GPIOD);
    }
}

void Advance(u8 a)
{
    switch (a)
    {
    case 2:
        GPIO_SetBits(GPIOD, GPIO_Pin_11);
        GPIO_ResetBits(GPIOD, GPIO_Pin_10);
        break;
    case 3:
        GPIO_SetBits(GPIOD, GPIO_Pin_15);
        GPIO_ResetBits(GPIOD, GPIO_Pin_14);
        break;
    case 4:
        GPIO_SetBits(GPIOD, GPIO_Pin_4);
        GPIO_ResetBits(GPIOD, GPIO_Pin_3);
        break;
    case 5:
        GPIO_SetBits(GPIOD, GPIO_Pin_6);
        GPIO_ResetBits(GPIOD, GPIO_Pin_7);
        break;

    default:
        break;
   } 
}
void Back(u8 a)
{
    switch (a)
    {
    case 2:
        GPIO_ResetBits(GPIOD, GPIO_Pin_11);
        GPIO_SetBits(GPIOD, GPIO_Pin_10);
        break;
    case 3:
        GPIO_ResetBits(GPIOD, GPIO_Pin_15);
        GPIO_SetBits(GPIOD, GPIO_Pin_14);
        break;
    case 4:
        GPIO_ResetBits(GPIOD, GPIO_Pin_4);
        GPIO_SetBits(GPIOD, GPIO_Pin_3);
        break;
    case 5:
        GPIO_ResetBits(GPIOD, GPIO_Pin_6);
        GPIO_SetBits(GPIOD, GPIO_Pin_7);
        break;

    default:
        break;
    }
}
void PULL_High()
{
    GPIO_SetBits(GPIOD, GPIO_Pin_10);
    GPIO_SetBits(GPIOD, GPIO_Pin_15);
    GPIO_SetBits(GPIOD, GPIO_Pin_3);
    GPIO_SetBits(GPIOD, GPIO_Pin_7);
    GPIO_SetBits(GPIOD, GPIO_Pin_6);
    GPIO_SetBits(GPIOD, GPIO_Pin_4);
    GPIO_SetBits(GPIOD, GPIO_Pin_14);
    GPIO_SetBits(GPIOD, GPIO_Pin_11);
}
void PULL_Low()
{
    GPIO_ResetBits(GPIOD, GPIO_Pin_10);
    GPIO_ResetBits(GPIOD, GPIO_Pin_15);
    GPIO_ResetBits(GPIOD, GPIO_Pin_3);
    GPIO_ResetBits(GPIOD, GPIO_Pin_7);
    GPIO_ResetBits(GPIOD, GPIO_Pin_6);
    GPIO_ResetBits(GPIOD, GPIO_Pin_4);
    GPIO_ResetBits(GPIOD, GPIO_Pin_14);
    GPIO_ResetBits(GPIOD, GPIO_Pin_11);
}

