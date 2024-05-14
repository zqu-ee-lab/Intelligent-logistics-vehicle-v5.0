#include "stdio.h"
#include "PWM.h"
/**
 * @description: 初始化定时器1输出4路PWM
 * @param {uint16_t} arr：自动重装在计算值
 * @param {uint16_t} psc：预分频系数
 * @param {uint16_t} CCR1_Val：通道1比较寄存器值（注：此值改变占空比，越大占空比越大，但不能超过arr）
 * @param {uint16_t} CCR2_Val：通道2比较寄存器值（注：此值改变占空比，越大占空比越大，但不能超过arr）
 * @param {uint16_t} CCR3_Val：通道3比较寄存器值（注：此值改变占空比，越大占空比越大，但不能超过arr）
 * @param {uint16_t} CCR4_Val：通道4比较寄存器值（注：此值改变占空比，越大占空比越大，但不能超过arr）
 * @return {*}
 * @p 注意定时器初始化要在串口前
 */
//*默认调用PWM_TIM1_config(10,72,5,4,3,2);
void PWM_TIM1_config(uint16_t arr, uint16_t psc, uint16_t CCR1_Val, uint16_t CCR2_Val, uint16_t CCR3_Val, uint16_t CCR4_Val)
{

    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // 复用推挽输出
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure); // 初始化GPIO
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_TIM1);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_TIM1);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_TIM1);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_TIM1);
    // 初始化TIM1
    TIM_TimeBaseStructure.TIM_Period = arr - 1;                 // 设置在下一个更新事件装入活动的自动重装载寄存器周期的值
    TIM_TimeBaseStructure.TIM_Prescaler = psc - 1;              // 设置用来作为TIMx时钟频率除数的预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     // 死区时间
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM向上计数模式
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);             // 根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

    /*--------------------输出比较结构体初始化-------------------*/
    // 占空比配置

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;             // 配置为PWM模式1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 输出使能
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // 输出通道电平极性配置
    TIM_CtrlPWMOutputs(TIM1, ENABLE);                         // MOE 主输出使能
    // 输出比较通道 1       PE9
    TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    // 输出比较通道 2       PE11
    TIM_OCInitStructure.TIM_Pulse = CCR2_Val;
    TIM_OC2Init(TIM1, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);

    // 输出比较通道 3       PE13
    TIM_OCInitStructure.TIM_Pulse = CCR3_Val;
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);

    // 输出比较通道 4       PE14
    TIM_OCInitStructure.TIM_Pulse = CCR4_Val;
    TIM_OC4Init(TIM1, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);

    // 使能计数器
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable); // CH1预装载使能
    TIM_ARRPreloadConfig(TIM1, ENABLE);               // 使能TIMx在ARR上的预装载寄存器
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
}

//*默认调用PWM_TIM8_config(10,103,5,4,3,2);
void PWM_TIM8_config(uint16_t arr, uint16_t psc, uint16_t CCR1_Val, uint16_t CCR2_Val, uint16_t CCR3_Val, uint16_t CCR4_Val)
{

    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // 复用推挽输出
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure); // 初始化GPIO

    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_TIM8);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_TIM8);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_TIM8);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_TIM8);

    // 初始化TIM8
    TIM_TimeBaseStructure.TIM_Period = arr - 1;                 // 设置在下一个更新事件装入活动的自动重装载寄存器周期的值
    TIM_TimeBaseStructure.TIM_Prescaler = psc - 1;              // 设置用来作为TIMx时钟频率除数的预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     // 死区时间
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM向上计数模式
    TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);             // 根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

    /*--------------------输出比较结构体初始化-------------------*/
    // 占空比配置

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;             // 配置为PWM模式1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 输出使能
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // 输出通道电平极性配置
    TIM_CtrlPWMOutputs(TIM8, ENABLE);                         // MOE 主输出使能
    // 输出比较通道 1
    TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
    TIM_OC1Init(TIM8, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);
    // 输出比较通道 2
    TIM_OCInitStructure.TIM_Pulse = CCR2_Val;
    TIM_OC2Init(TIM8, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM8, TIM_OCPreload_Enable);

    // 输出比较通道 3
    TIM_OCInitStructure.TIM_Pulse = CCR3_Val;
    TIM_OC3Init(TIM8, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM8, TIM_OCPreload_Enable);

    // 输出比较通道 4
    TIM_OCInitStructure.TIM_Pulse = CCR4_Val;
    TIM_OC4Init(TIM8, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Enable);

    // 使能计数器
    TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable); // CH1预装载使能
    TIM_ARRPreloadConfig(TIM8, ENABLE);               // 使能TIMx在ARR上的预装载寄存器
    TIM_CtrlPWMOutputs(TIM8, ENABLE);
    TIM_Cmd(TIM8, ENABLE);
}

/**
 * @description: 设置比较寄存器X的值
 * @param {TIM_TypeDef} *TIMX
 * @param {uint16_t} CCRX_Val
 * @param {uint8_t} X取值{1-4}
 * @return {*}
 */
void SetCompare1(TIM_TypeDef *TIMX, u32 CCRX_Val, uint8_t X)
{
    switch (X)
    {
    case 1:
        TIM_SetCompare1(TIMX, CCRX_Val);
        break;
    case 2:
        TIM_SetCompare2(TIMX, CCRX_Val);
        break;
    case 3:
        TIM_SetCompare3(TIMX, CCRX_Val);
        break;
    case 4:
        TIM_SetCompare4(TIMX, CCRX_Val);
        break;

    default:
        printf("你的X输入有错，X的取值为：[1,4]");
        break;
    }
}
void ControledMonitor(u8 channel, u16 Count)
{
    if (Count > 250)
    {
        Count = 250;
    }
    if (Count < 50)
    {
        Count = 50;
    }

    switch (channel)
    {
    case 1:
        TIM_SetCompare1(TIM8, Count);
        break;
    case 2:
        TIM_SetCompare2(TIM8, Count);
        break;
    case 3:
        TIM_SetCompare3(TIM8, Count);
        break;
    case 4:
        TIM_SetCompare4(TIM8, Count);
        break;

    default:
        printf("你的X输入有错，X的取值为：[1,4]");
        break;
    }
}
