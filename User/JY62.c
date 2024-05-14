//#include "JY62.h"

//#if EN_JY62
//const char YAWCMD[3] = {0XFF, 0XAA, 0X52};
//const char ACCCMD[3] = {0XFF, 0XAA, 0X67};
//const char SLEEPCMD[3] = {0XFF, 0XAA, 0X60};
//const char UARTMODECMD[3] = {0XFF, 0XAA, 0X61};
//const char IICMODECMD[3] = {0XFF, 0XAA, 0X62};
//unsigned char TxBuffer[256];
//unsigned char TxCounter = 0;
//unsigned char count1 = 0;
//struct SAcc stcAcc;//角加速度结构体
//struct SGyro stcGyro;//角速度结构体
//struct SAngle stcAngle;// 角度结构体
//static void NVIC_USART4_Configuration(void)
//{
//    NVIC_InitTypeDef NVIC_InitStructure;

//    /* 嵌套向量中断控制器组选择 */
//    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

//    /* 配置USART为中断源 */
//    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
//    /* 抢断优先级*/
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
//    /* 子优先级 */
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//    /* 使能中断 */
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//    /* 初始化配置NVIC */
//    NVIC_Init(&NVIC_InitStructure);
//}

//void USART4_Config_JY62(void)
//{
//    GPIO_InitTypeDef GPIO_InitStructure;
//    USART_InitTypeDef USART_InitStructure;

//    // 打开串口GPIO的时钟
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
//    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
//    // 打开串口外设的时钟
//		GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_UART4);
//	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_UART4);

//    // 将USART Tx的GPIO配置为推挽复用模式
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
//    GPIO_Init(GPIOC, &GPIO_InitStructure);

//    // 将USART Rx的GPIO配置为浮空输入模式
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//    GPIO_Init(GPIOC, &GPIO_InitStructure);

//    // 配置串口的工作参数
//    // 配置波特率
//    USART_InitStructure.USART_BaudRate = 115200;
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
//    USART_Init(JY62_USARTX, &USART_InitStructure);

//    // 串口中断优先级配置
//    NVIC_USART4_Configuration();

//    //	// 使能串口接收中断
//    USART_ITConfig(JY62_USARTX, USART_IT_RXNE, ENABLE);

//    // 使能串口
//    USART_Cmd(JY62_USARTX, ENABLE);
//}

//void CopeSerial2Data(unsigned char ucData)
//{
//    static unsigned char ucRxBuffer[250];
//    static unsigned char ucRxCnt = 0;
//    ucRxBuffer[ucRxCnt++] = ucData; // 将收到的数据存入缓冲区中
//    if (ucRxBuffer[0] != 0x55)      // 数据头不对，则重新开始寻找0x55数据头
//    {
//        ucRxCnt = 0;
//        return;
//    }
//    if (ucRxCnt < 11)
//    {
//        return;
//    } // 数据不满11个，则返回
//    else
//    {
//        switch (ucRxBuffer[1]) // 判断数据是哪种数据，然后将其拷贝到对应的结构体中，有些数据包需要通过上位机打开对应的输出后，才能接收到这个数据包的数据
//        {
//        // memcpy为编译器自带的内存拷贝函数，需引用"string.h"，将接收缓冲区的字符拷贝到数据结构体里面，从而实现数据的解析。
//        case 0x51:
//            memcpy(&stcAcc, &ucRxBuffer[2], 8);
//            break;
//        case 0x52:
//            memcpy(&stcGyro, &ucRxBuffer[2], 8);
//            break;
//        case 0x53:
//            memcpy(&stcAngle, &ucRxBuffer[2], 8);
//            break;
//        }
//        ucRxCnt = 0; // 清空缓存区
//    }
//}

//extern unsigned char TxBuffer[256];
//extern unsigned char TxCounter;
//extern unsigned char count1;
//void JY62_IRQHandler(void)
//{
//    if (USART_GetITStatus(JY62_USARTX, USART_IT_TXE) != RESET)
//    {
//        USART_SendData(JY62_USARTX, TxBuffer[TxCounter++]);
//        USART_ClearITPendingBit(JY62_USARTX, USART_IT_TXE);
//        if (TxCounter == count1)
//            USART_ITConfig(JY62_USARTX, USART_IT_TXE, DISABLE);
//    }
//    else if (USART_GetITStatus(JY62_USARTX, USART_IT_RXNE) != RESET)
//    {
//        CopeSerial2Data((unsigned char)JY62_USARTX->DR); // 处理数据
//        USART_ClearITPendingBit(JY62_USARTX, USART_IT_RXNE);
//    }

//    USART_ClearITPendingBit(JY62_USARTX, USART_IT_ORE);
//}

//void sendcmd(const char cmd[])
//{
//    char i;
//    for (i = 0; i < 3; i++)
//        UART2_Put_Char(cmd[i]);
//}
//void UART2_Put_Char(unsigned char DataToSend)
//{
//    TxBuffer[count1++] = DataToSend;
//    USART_ITConfig(JY62_USARTX, USART_IT_TXE, ENABLE);
//}
//#endif
