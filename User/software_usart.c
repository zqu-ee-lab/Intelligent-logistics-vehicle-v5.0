#include "software_usart.h"
#include "delay.h"
#include "buffer.h"
/*//!初始化
    TIM7_Int_Init(72, 8);
    Software_USART_IOConfig();
*/
//u8 len = 0; // 接收计数
//extern struct Buff USART1_buffer, U3_buffer, Soft_Usart;
//enum stat
//{
//    COM_START_BIT,
//    COM_D0_BIT,
//    COM_D1_BIT,
//    COM_D2_BIT,
//    COM_D3_BIT,
//    COM_D4_BIT,
//    COM_D5_BIT,
//    COM_D6_BIT,
//    COM_D7_BIT,
//    COM_STOP_BIT,
//};

//u8 recvStat = COM_STOP_BIT;
//u8 recvData;

void Software_USART_TXD(u8 Data)
{
    __set_PRIMASK(1);
    u8 i = 0;
    GPIO_ResetBits(SUSART_GPIOX_TX, SUSART_GPIOX_PIN_TX);
    Delayus(BAUDRATE);
    for (i = 0; i < 8; i++)
    {
        if (Data & 0x01)
            GPIO_SetBits(SUSART_GPIOX_TX, SUSART_GPIOX_PIN_TX);
        else
            GPIO_ResetBits(SUSART_GPIOX_TX, SUSART_GPIOX_PIN_TX);

        Delayus(BAUDRATE);
        Data = Data >> 1;
    }
    GPIO_SetBits(SUSART_GPIOX_TX, SUSART_GPIOX_PIN_TX);
    Delayus(BAUDRATE);
    __set_PRIMASK(0);
}

void USART_Send(u8 *buf, u8 len)
{
    u8 t;
    for (t = 0; t < len; t++)
    {
        Software_USART_TXD(buf[t]);
    }
}

void Software_USART_IOConfig(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
//    NVIC_InitTypeDef NVIC_InitStructure;
//    EXTI_InitTypeDef EXTI_InitStruct;
    RCC_AHB1PeriphClockCmd(SUSART_GPIOX_RCC_TX, ENABLE); // 使能PB,PC端口时钟

    // SoftWare Serial TXD
    GPIO_InitStructure.GPIO_Pin = SUSART_GPIOX_PIN_TX;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;     // 推挽输出
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // IO口速度为50MHz
    GPIO_Init(SUSART_GPIOX_TX, &GPIO_InitStructure);
    GPIO_SetBits(SUSART_GPIOX_TX, SUSART_GPIOX_PIN_TX);

    // SoftWare Serial RXD
    GPIO_InitStructure.GPIO_Pin = SUSART_GPIOX_PIN_RX;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // 推挽输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(SUSART_GPIOX_RX, &GPIO_InitStructure);

    // //  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);
    // EXTI_InitStruct.EXTI_Line = EXTI_Line14;
    // EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    // EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿触发中断
    // EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    // EXTI_Init(&EXTI_InitStruct);

    // NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    // NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    // NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    // NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    // NVIC_Init(&NVIC_InitStructure);
}

//void TIM7_Int_Init(u16 arr, u16 psc)
//{
//    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
//    NVIC_InitTypeDef NVIC_InitStructure;

//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE); // 时钟使能

//    // 定时器TIM7初始化
//    TIM_TimeBaseStructure.TIM_Period = arr - 1;                     // 设置在下一个更新事件装入活动的自动重装载寄存器周期的值
//    TIM_TimeBaseStructure.TIM_Prescaler = psc;                  // 设置用来作为TIMx时钟频率除数的预分频值
//    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     // 设置时钟分割:TDTS = Tck_tim
//    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM向上计数模式
//    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);             // 根据指定的参数初始化TIMx的时间基数单位
//    TIM_ClearITPendingBit(TIM7, TIM_FLAG_Update);
//    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE); // 使能指定的TIM3中断,允许更新中断

//    // 中断优先级NVIC设置
//    NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;           // TIM7中断
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 先占优先级1级
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        // 从优先级1级
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQ通道被使能
//    NVIC_Init(&NVIC_InitStructure);                           // 初始化NVIC寄存器
//}

//void EXTI15_10_IRQHandler(void)
//{
//    if (EXTI_GetFlagStatus(EXTI_Line14) != RESET)
//    {
//        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14) == 0)
//        {
//            if (recvStat == COM_STOP_BIT)
//            {
//                recvStat = COM_START_BIT;
//                TIM_Cmd(TIM7, ENABLE);
//            }
//        }
//        EXTI_ClearFlag(EXTI_Line14);
//    }
//}

//void TIM7_IRQHandler(void)
//{

//    if (TIM_GetFlagStatus(TIM7, TIM_FLAG_Update) != RESET)
//    {
//        TIM_ClearITPendingBit(TIM7, TIM_FLAG_Update);
//        
//        recvStat++;
//        if (recvStat == COM_STOP_BIT)
//        {
//            TIM_Cmd(TIM7, DISABLE);
//            Write_BUFF(&recvData, &Soft_Usart);
//            return;
//        }
//        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14))
//        {
//            recvData |= (1 << (recvStat - 1));
//        }
//        else
//        {
//            recvData &= ~(1 << (recvStat - 1));
//        }
//    }
//}
// void EXTI15_10_IRQHandler(void)
// {
//   if (EXTI_GetFlagStatus(EXTI_Line14) != RESET) //检查外部中断是否产生了
//   {
//     if (!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14)) //检测引脚高低电平，如果是低电平，则说明检测到起始位
//     {
//       TIMX_Delayus(TIM6, 8); //延时 0x432 约等于104us
//       if (recvStat == COM_STOP_BIT)
//       {
//         recvStat = COM_START_BIT;        //此时的状态是起始
//         while (recvStat != COM_STOP_BIT) // 循环到停止位
//         {
//           recvStat++;                                    // 改变状态
//           if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14)) //‘1’
//           {
//             recvData |= (1 << (recvStat - 1));
//           }
//           else
//           {
//             recvData &= ~(1 << (recvStat - 1));
//           }
//           TIMX_Delayus(TIM6, 8);
//         }
//       }
//     }
//     EXTI_ClearITPendingBit(EXTI_Line14); //清除EXTI_Line10中断挂起标志位
//   }
// }
