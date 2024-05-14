#include "VL53.h"
#include "buffer.h"
#include "usart.h"

#define EN_VL53 1
#if EN_VL53
// static void NVIC_USARTX_VL53_Configuration(void)
//{
//     NVIC_InitTypeDef NVIC_InitStructure;

//    /* 嵌套向量中断控制器组选择 */
//    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

//    /* 配置USART为中断源 */
//    NVIC_InitStructure.NVIC_IRQChannel = VL53_USARTX_IRQn;
//    /* 抢断优先级*/
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
//    /* 子优先级 */
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//    /* 使能中断 */
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//    /* 初始化配置NVIC */
//    NVIC_Init(&NVIC_InitStructure);
//}

// void USARTX_VL53_Config(void)
//{
//     GPIO_InitTypeDef GPIO_InitStructure;
//     USART_InitTypeDef USART_InitStructure;

//    // 打开串口GPIO的时钟
//    RCC_AHB1PeriphClockCmd(VL53_GPIO_CLOCK, ENABLE);

//    // 打开串口外设的时钟
//    RCC_APB1PeriphClockCmd(VL53_USARTX_CLOCK, ENABLE);
//		GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2);
//		GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2);
//    // 将USART Tx的GPIO配置为推挽复用模式
//    GPIO_InitStructure.GPIO_Pin = VL53_TX;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
//    GPIO_Init(VL53_GPIOX, &GPIO_InitStructure);

//    // 将USART Rx的GPIO配置为浮空输入模式
//    GPIO_InitStructure.GPIO_Pin = VL53_RX;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//    GPIO_Init(VL53_GPIOX, &GPIO_InitStructure);

//    // 配置串口的工作参数
//    // 配置波特率
//    USART_InitStructure.USART_BaudRate = VL53_BAUDRATE;
//    // 配置 针数据字长
//    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//    // 配置停止位
//    USART_InitStructure.USART_StopBits = USART_StopBits_1;
//    // 配置校验位
//    USART_InitStructure.USART_Parity = USART_Parity_No;
//    // 配置硬件流控制
//    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//    // 配置工作模式，收发一起
//    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
//    // 完成串口的初始化配置
//    USART_Init(VL53_USARTX, &USART_InitStructure);

//    // 串口中断优先级配置
//    NVIC_USARTX_VL53_Configuration();

//    // 使能串口接收中断
//    USART_ITConfig(VL53_USARTX, USART_IT_RXNE, ENABLE);

//    // 使能串口
//    USART_Cmd(VL53_USARTX, ENABLE);
//}

// void VL53_USARTX_IRQHandler()
//{
//     u8 Data;
//     if (USART_GetFlagStatus(VL53_USARTX, USART_FLAG_RXNE) == 1)
//     {
//         Data = VL53_USARTX->DR;
//         Write_BUFF(&Data, &VL53_USARTX_Buff); // 把串口接收到的数据并存进环形缓冲区
//     }
//     USART_ClearFlag(VL53_USARTX, USART_FLAG_RXNE);
// }
/**
 * @description: 初始化激光测距（总函数）
 * @return {*}
 */
void VL53_Initial()
{
    Init_UART5_All();
    return;
}
/**
 * @description: 读取Buff数据，并解析为距离数据
 * @param {u16} *distance 传入一个u16的指针接收距离数据
 * @return {*}
 */
// void VL53_Read_Data(u16 *distance)
// {

//     u8 H_bit, L_bit, i = 5;
//     while (i--)
//     {
//         if (*(u8 *)Read_BUFF(&VL53_USARTX_Buff) == VL53_Agreement_RX[0])
//         {
//             if (*(u8 *)Read_BUFF(&VL53_USARTX_Buff) == VL53_Agreement_RX[1])
//             {
//                 if (*(u8 *)Read_BUFF(&VL53_USARTX_Buff) == VL53_Agreement_RX[2])
//                 {
//                     H_bit = *(u8 *)Read_BUFF(&VL53_USARTX_Buff);
//                     L_bit = *(u8 *)Read_BUFF(&VL53_USARTX_Buff);
//                     *distance = (H_bit << 8) + L_bit;
//                     break;
//                 }
//             }
//         }
//     }
// }
/**
 * @description: 发送读取数据命令
 * @return {*}
 */
void VL53_Send_Agrement()
{
    u8 i;
    const u8 Agreement[] = {0x50, 0x03, 0x00, 0x34, 0x00, 0X01, 0XC8, 0X45};
//    const u8 Agreement1[8] = {0x57, 0x10, 0xff, 0xff, 0x01, 0xff, 0xff, 0x64};
	
    for (i = 0; i < 8; i++)
    {
        USART_SendData(VL53_USARTX, Agreement[i]);
        while (USART_GetFlagStatus(VL53_USARTX, USART_FLAG_TXE) == RESET)
            ;
    }
    for (i = 0; i < 8; i++)
    {
        USART_SendData(USART3, Agreement[i]);
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
            ;
    }
}

#endif
