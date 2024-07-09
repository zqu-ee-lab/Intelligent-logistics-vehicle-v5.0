/*
 * @Date: 2023-10-04 11:45:55
 * @LastEditors: JAR_CHOW
 * @LastEditTime: 2024-06-24 20:36:47
 * @FilePath: \RVMDK（uv5）c:\Users\mrchow\Desktop\vscode_repo\Intelligent-logistics-vehicle-v5.0\User\usart.c
 * @Verson:V0.3(配合buffer v0.2)
 * v0.1 USART3增加DMA接受
 * v0.2 USART4，USART5，USART2增加DMA接受
 * v0.3 串口可使用DMA接受和中断接受，只需在h文件改宏定义
 */
#include "usart.h"
#include "buffer.h"
#include <string.h>
#include <stdarg.h>

struct Buff U3_buffer, U2_buffer, U5_buffer, U4_buffer, U1_buffer;
struct Buff *U3_buffer_handle = &U3_buffer, *U2_buffer_handle = &U2_buffer, *U1_buffer_handle = &U1_buffer,
			*U4_buffer_handle = &U4_buffer, *U5_buffer_handle = &U5_buffer;

#ifdef DMA_USART1_RX_TX

/******************************** 串口1 开始 **********************************/
uint8_t USART1_TX_Buff[USART1_TX_BUF_SIZE] = {0};
uint8_t USART1_RC_Flag = 0;
uint8_t USART1_TC_Flag = 0;

static void USART1_IO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void USART1_Config(uint32_t baud)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	USART_DeInit(USART1);

	USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate = baud;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART1, &USART_InitStructure);

	// 使用dma缓冲区的话，串口接收中断和空闲中断不是必须的
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_ClearFlag(USART1, USART_FLAG_RXNE);
	USART_ClearFlag(USART1, USART_FLAG_IDLE);

	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE); // 空闲中断
	//  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);		//串口接收中断

	USART_Cmd(USART1, ENABLE);
}

static void USART1_DMA_RX_TX_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE); // DMA2时钟使能

	// DMA2_USART1_RX:通道4，数据流5
	DMA_Cmd(DMA2_Stream5, DISABLE);
	DMA_DeInit(DMA2_Stream5);

	DMA_InitStructure.DMA_Channel = DMA_Channel_4;								/* 配置DMA通道 */
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(USART1->DR));		/* 源 */
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)U1_buffer_handle->head_p; /* 目的 */
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;						/* 方向 */
	DMA_InitStructure.DMA_BufferSize = U1_buffer_handle->size;					/* 长度 */
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			/* 外设地址是否自增 */
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						/* 内存地址是否自增 */
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;			/* 目的数据带宽 */
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;				/* 源数据宽度 */
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;								/* 单次传输模式/循环传输模式 */
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;						/* DMA优先级 */
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;						/* FIFO模式/直接模式 */
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;			/* FIFO大小 */
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;					/* 单次传输 */
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream5, &DMA_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_ClearFlag(DMA2_Stream5, DMA_FLAG_TCIF5); // 清空一下标志位
	// DMA_ITConfig(DMA2_Stream5, DMA_IT_TC, ENABLE); // 使能dma完成中断  	ps:如果dma是循环传输模式的话，完成中断将不会工作

	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE); // 串口DMA接收使能
	DMA_Cmd(DMA2_Stream5, ENABLE);				   // DMA传输使能

	// !the following code can initialize the usart1 tx dma
	// DMA2_USART1_TX:通道4，数据流7
	DMA_Cmd(DMA2_Stream7, DISABLE); // DMA传输失能
	DMA_DeInit(DMA2_Stream7);

	/* 确保DMA数据流复位完成 */
	while (DMA_GetCmdStatus(DMA2_Stream7) != DISABLE)
	{
	}

	/*usart1 tx对应dma2，通道4，数据流7*/
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(USART1_BASE + 0x04); /*设置DMA源：串口数据寄存器地址*/

	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)USART1_TX_Buff; /*内存地址(要传输的变量的指针)*/

	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral; /*方向：从内存到外设*/

	DMA_InitStructure.DMA_BufferSize = USART1_TX_BUF_SIZE; /*传输大小DMA_BufferSize=SENDBUFF_SIZE*/

	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; /*外设地址不增*/

	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; /*内存地址自增*/

	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; /*外设数据单位*/

	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; /*内存数据单位 8bit*/

	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; /*DMA模式：一次循环*/
	// 也可以给他赋值 DMA_Mode_Circular 如下,但是对于发送串口来说，十分鸡肋，我们不希望他一直循环发送，这会导致发送了缓冲区的没用数据和过期数据
	/*DMA模式：不断循环*/
	// DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;

	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; /*优先级：中*/

	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; /*禁用FIFO*/
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;

	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single; /*存储器突发传输 16个节拍*/

	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; /*外设突发传输 1个节拍*/
	DMA_Init(DMA2_Stream7, &DMA_InitStructure);

	DMA2_Stream7->NDTR = 0;

	//? 开启DMA中断
	// NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;
	// NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	// NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	// NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	// NVIC_Init(&NVIC_InitStructure);

	// USART1_TC_Flag = 0;

	// DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF7);   // chear the interrupt flag bit of dma2 stream 7
	// DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE); // !使能发送完成中断，如果不用双缓冲区，the code in here is not necessary.

	// // USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE);//串口DMA发送使能

	// DMA_Cmd(DMA2_Stream7, ENABLE); // DMA传输使能
}

/******** 串口1初始化配置 *********/
void Init_USART1_All()
{
	Iinitial_BUFF(U1_buffer_handle, 30);

	USART1_IO_Config();

	USART1_Config(115200);

	USART1_DMA_RX_TX_Config();
}

/******** 串口1 DMA接收数据 *********/
void USART1_DMA_ReceData(uint8_t *data)
{
	DMA_Cmd(DMA2_Stream5, DISABLE); // 关闭DMA设备（需要在DMA关闭的状态下才能进行配置）

	DMA_ClearFlag(DMA2_Stream5, DMA_FLAG_TCIF5); // 这是必要的，如果不提前清除标志位，将无法使能dma，代码将会卡死在此函数的第二个循环里面

	while (DMA_GetCmdStatus(DMA2_Stream5) != DISABLE)
		; // 等待关闭完成

	DMA2_Stream5->M0AR = (uint32_t)data; // 接收数据所放的地址
	DMA2_Stream5->NDTR = (uint32_t)U1_buffer_handle->size;
	DMA_Cmd(DMA2_Stream5, ENABLE);					 // 开启DMA（等待接收数据）
	while (DMA_GetCmdStatus(DMA2_Stream5) != ENABLE) // 等待开启完成
		DMA_Cmd(DMA2_Stream5, ENABLE);				 // 开启DMA（等待接收数据）
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);	 // 串口DMA接收使能
}

/******** 串口1 DMA发送数据 *********/
void USART1_DMA_TranData(uint8_t *data, uint32_t dataLen)
{
	while (DMA2_Stream7->NDTR)
	{
		; // 等待上一次数据传输完成
	}

	USART1_TC_Flag = 1; // 将传输标志位置1表示正在传数据

	DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7); // 如果不清除标志位，会开启不了dma

	memcpy(USART1_TX_Buff, data, dataLen); // 将要传输的数据复制到dma缓冲区

	DMA_Cmd(DMA2_Stream7, DISABLE); // 关闭DMA设备（需要在DMA关闭的状态下才能进行配置）

	while (DMA_GetCmdStatus(DMA2_Stream7) != DISABLE)
		; // 等待关闭完成

	DMA2_Stream7->M0AR = (uint32_t)USART1_TX_Buff; // 需要发送的数据地址
	DMA2_Stream7->NDTR = dataLen;				   // 需要发送的数据长度
	// DMA_SetCurrDataCounter(DMA2_Stream7,dataLen);

	DMA_Cmd(DMA2_Stream7, ENABLE); // 开启DMA（开始发送数据）
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
}

/*
*********************************************************************************************************
*	函 数 名: App_Printf
*	功能说明: 线程安全的printf方式
*	形    参: 同printf的参数。
*             在C中，当无法列出传递函数的所有实参的类型和数目时,可以用省略号指定参数表
*	返 回 值: 无
*********************************************************************************************************
*/
void App_Printf(char *format, ...)
{
	char buf_str[200 + 1];
	va_list v_args;

	va_start(v_args, format);
	(void)vsnprintf((char *)&buf_str[0],
					(size_t)sizeof(buf_str),
					(char const *)format,
					v_args);
	va_end(v_args);

	USART1_DMA_TranData((uint8_t *)&buf_str[0], strlen((char *)&buf_str[0]));
}

///

/******** 串口1 中断服务函数 ***********/
void USART1_IRQHandler(void)
{
	// static uint8_t a[50];
	if (USART_GetITStatus(USART1, USART_IT_IDLE) == SET)
	{
		USART1_RC_Flag = 1;

		Write_BUFF_P((U1_buffer_handle->size) - (uint32_t)DMA2_Stream5->NDTR, U1_buffer_handle);
		// sprintf((char *)a, "%d\r\n", U1_buffer_handle->buffer_used); // 通过发生串口告知缓冲区内dma当前的写入地址偏移量 也就是之前缓冲区库里面的write_p-head_p
		// USART1_DMA_TranData(a, strlen((char *)a));

		USART1->SR;
		USART1->DR; // 清除串口空闲中断标志位
	}
}

/**** 串口1 DMA接收中断服务函数 *****/
void DMA2_Stream5_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA2_Stream5, DMA_IT_TCIF5) == SET)
	{
		DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_TCIF5);
		DMA_ClearFlag(DMA2_Stream5, DMA_FLAG_TCIF5);
		// USART1_DMA_TranData(U1_buffer_handle->head_p, (U1_buffer_handle->size) - (uint32_t)DMA2_Stream5->NDTR);
		Write_BUFF_P((U1_buffer_handle->size) - (uint32_t)DMA2_Stream5->NDTR, U1_buffer_handle);

		USART1_DMA_ReceData(U1_buffer_handle->head_p);
	}
}

/**** 串口1 DMA发送中断服务函数 *****/
void DMA2_Stream7_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7) == SET)
	{
		USART1_TC_Flag = 0;
		DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
	}
}

/******************************** 串口1 结束 **********************************/

#endif

#ifdef NORMAL_USART1 // 宏定义是否使用这个代码，在头文件修改
static void NVIC_USART1_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
static void USART1_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; // TX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; // RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	NVIC_USART1_Configuration();
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART1, ENABLE);
}
void Init_USART1_All()
{
	Iinitial_BUFF(&U1_buffer, BUFFER_SIZE_U2);
	USART1_Config();
}
void USART1_IRQHandler()
{

	if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == 1)
	{
		USART_ReceiveData(USART1);
	}
	USART_ClearFlag(USART1, USART_FLAG_RXNE);
}
#endif

#ifdef DMA_USART2_RX_TX

/******************************** 串口2 开始 **********************************/
uint8_t USART2_TX_Buff[USART2_TX_BUF_SIZE] = {0};
uint8_t USART2_RC_Flag = 0;
uint8_t USART2_TC_Flag = 0;

static void USART2_IO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; // 上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; // 上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void USART2_Config(uint32_t baud)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	USART_DeInit(USART2);

	USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate = baud;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; // 8位数据
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		// 1位停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;			// 无奇偶校验
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控制
	USART_Init(USART2, &USART_InitStructure);

	// 使用dma缓冲区的话，串口接收中断和空闲中断不是必须的
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // 抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  // 子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);

	USART_ClearFlag(USART2, USART_FLAG_TC);
	USART_ClearFlag(USART2, USART_FLAG_RXNE);
	USART_ClearFlag(USART2, USART_FLAG_IDLE);

	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE); // 空闲中断
	//  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);		//串口接收中断

	USART_Cmd(USART2, ENABLE);
}

static void USART2_DMA_RX_TX_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE); // DMA1时钟使能

	// DMA1_USART2_RX:通道4，数据流5
	DMA_Cmd(DMA1_Stream5, DISABLE);
	DMA_DeInit(DMA1_Stream5);

	DMA_InitStructure.DMA_Channel = DMA_Channel_4;								/* 配置DMA通道 */
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(USART2->DR));		/* 源 */
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)U2_buffer_handle->head_p; /* 目的 */
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;						/* 方向 */
	DMA_InitStructure.DMA_BufferSize = U2_buffer_handle->size;					/* 长度 */
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			/* 外设地址是否自增 */
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						/* 内存地址是否自增 */
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;			/* 目的数据带宽 */
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;				/* 源数据宽度 */
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;								/* 单次传输模式/循环传输模式 */
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;						/* DMA优先级 */
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;						/* FIFO模式/直接模式 */
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;			/* FIFO大小 */
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;					/* 单次传输 */
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream5, &DMA_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_TCIF5); // 清空一下标志位
	// DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, ENABLE); // 使能dma完成中断  	ps:如果dma是循环传输模式的话，完成中断将不会工作

	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
	DMA_Cmd(DMA1_Stream5, ENABLE);

	// !the following code can initialize the usart1 tx dma
	// DMA1_USART2_TX:dma1 通道4，数据流6

	DMA_Cmd(DMA1_Stream6, DISABLE); // DMA传输失能
	DMA_DeInit(DMA1_Stream6);

	/* 确保DMA数据流复位完成 */
	while (DMA_GetCmdStatus(DMA1_Stream6) != DISABLE)
	{
	}

	/*usart2 tx对应dma1，通道4，数据流6*/
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(USART2_BASE + 0x04); /*设置DMA源：串口数据寄存器地址*/

	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)USART2_TX_Buff; /*内存地址(要传输的变量的指针)*/

	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral; /*方向：从内存到外设*/

	DMA_InitStructure.DMA_BufferSize = USART2_TX_BUF_SIZE; /*传输大小DMA_BufferSize=SENDBUFF_SIZE*/

	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; /*外设地址不增*/

	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; /*内存地址自增*/

	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; /*外设数据单位*/

	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; /*内存数据单位 8bit*/

	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; /*DMA模式：一次循环*/
	// 也可以给他赋值 DMA_Mode_Circular 如下,但是对于发送串口来说，十分鸡肋，我们不希望他一直循环发送，这会导致发送了缓冲区的没用数据和过期数据
	/*DMA模式：不断循环*/
	// DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; /*优先级：中*/

	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; /*禁用FIFO*/

	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;

	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single; /*存储器突发传输 16个节拍*/

	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; /*外设突发传输 1个节拍*/

	DMA_Init(DMA1_Stream6, &DMA_InitStructure);

	DMA1_Stream6->NDTR = 0;

	//? 开启DMA中断

	// NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream6_IRQn;
	// NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	// NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	// NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	// NVIC_Init(&NVIC_InitStructure);

	// USART2_TC_Flag = 0;

	// DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6);   // chear the interrupt flag bit of dma1 stream 6
	// DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE); // !使能发送完成中断，如果不用双缓冲区，the code in here is not necessary.

	// // USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE);//串口DMA发送使能

	// DMA_Cmd(DMA1_Stream6, ENABLE); // DMA传输使能
}

/******** 串口2初始化配置 *********/
void Init_USART2_All()
{
	Iinitial_BUFF(&U2_buffer, BUFFER_SIZE_U2);
	USART2_IO_Config();
	USART2_Config(115200);
	USART2_DMA_RX_TX_Config();
}

/******** 串口2 DMA接收数据 *********/
void USART2_DMA_ReceData(uint8_t *data)
{
	DMA_Cmd(DMA1_Stream5, DISABLE); // 关闭DMA设备（需要在DMA关闭的状态下才能进行配置）

	DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_TCIF5); // 这是必要的，如果不提前清除标志位，将无法使能dma，代码将会卡死在此函数的第二个循环里面

	while (DMA_GetCmdStatus(DMA1_Stream5) != DISABLE)
		; // 等待关闭完成

	DMA1_Stream5->M0AR = (uint32_t)data; // 接收数据所放的地址
	DMA1_Stream5->NDTR = (uint32_t)U2_buffer_handle->size;
	DMA_Cmd(DMA1_Stream5, ENABLE);					 // 开启DMA（等待接收数据）
	while (DMA_GetCmdStatus(DMA1_Stream5) != ENABLE) // 等待开启完成
		DMA_Cmd(DMA1_Stream5, ENABLE);				 // 开启DMA（等待接收数据）
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);	 // 串口DMA接收使能
}

/******** 串口2 DMA发送数据 *********/
void USART2_DMA_TranData(uint8_t *data, uint32_t dataLen)
{
	while (DMA1_Stream6->NDTR)
	{
		; // 等待上一次数据传输完成
	}

	USART2_TC_Flag = 1; // 将传输标志位置1表示正在传数据

	DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6); // 如果不清除标志位，会开启不了dma

	memcpy(USART2_TX_Buff, data, dataLen); // 将要传输的数据复制到dma缓冲区

	DMA_Cmd(DMA1_Stream6, DISABLE); // 关闭DMA设备（需要在DMA关闭的状态下才能进行配置）

	while (DMA_GetCmdStatus(DMA1_Stream6) != DISABLE)
		; // 等待关闭完成

	DMA1_Stream6->M0AR = (uint32_t)USART2_TX_Buff; // 需要发送的数据地址
	DMA1_Stream6->NDTR = dataLen;				   // 需要发送的数据长度
	// DMA_SetCurrDataCounter(DMA1_Stream6,dataLen);

	DMA_Cmd(DMA1_Stream6, ENABLE); // 开启DMA（开始发送数据）
	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
}

/******** 串口2 中断服务函数 ***********/
void USART2_IRQHandler(void)
{
	if (USART_GetITStatus(USART2, USART_IT_IDLE) == SET)
	{
		USART2_RC_Flag = 1;

		Write_BUFF_P((U2_buffer_handle->size) - (uint32_t)DMA1_Stream5->NDTR, U2_buffer_handle);
		// sprintf((char *)a, "%d\r\n", U2_buffer_handle->buffer_used); // 通过发生串口告知缓冲区内dma当前的写入地址偏移量 也就是之前缓冲区库里面的write_p-head_p
		// USART2_DMA_TranData(a, strlen((char *)a));

		USART2->SR;
		USART2->DR; // 清除串口空闲中断标志位
	}
}

/**** 串口2 DMA接收中断服务函数 *****/
void DMA1_Stream5_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5) == SET)
	{
		DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
		DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_TCIF5);
		// USART2_DMA_TranData(U2_buffer_handle->head_p, (U2_buffer_handle->size) - (uint32_t)DMA1_Stream5->NDTR);
		Write_BUFF_P((U2_buffer_handle->size) - (uint32_t)DMA1_Stream5->NDTR, U2_buffer_handle);

		USART2_DMA_ReceData(U2_buffer_handle->head_p);
	}
}

/**** 串口2 DMA发送中断服务函数 *****/
void DMA1_Stream6_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_Stream6, DMA_IT_TCIF6) == SET)
	{
		USART2_TC_Flag = 0;
		DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6);
	}
}

#endif

#ifdef DMA_USART2_RX

static void NVIC_USART2_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* 嵌套向量中断控制器组选择 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* 配置USART为中断源 */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	/* 抢断优先级*/
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	/* 子优先级 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	/* 使能中断 */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/* 初始化配置NVIC */
	NVIC_Init(&NVIC_InitStructure);
}

void USART2_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// 打开串口GPIO的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE); // 开启DMA时钟
	// 打开串口外设的时钟

	// 将USART Tx的GPIO配置为推挽复用模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// 将USART Rx的GPIO配置为浮空输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
	// 配置串口的工作参数
	// 配置波特率
	USART_InitStructure.USART_BaudRate = 921600;
	// 配置 针数据字长
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// 配置停止位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// 配置校验位
	USART_InitStructure.USART_Parity = USART_Parity_No;
	// 配置硬件流控制
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// 配置工作模式，收发一起
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// 完成串口的初始化配置
	USART_Init(USART2, &USART_InitStructure);

	// 串口中断优先级配置
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
	NVIC_USART2_Configuration();
	DMA_DeInit(DMA1_Stream5);
	while (DMA_GetCmdStatus(DMA1_Stream5) != DISABLE)
		; // 等待DMA可配置
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32) & (USART2->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)U2_buffer.Data;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = BUFFER_SIZE_U2;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		/* 外设地址是否自增 */
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					/* 内存地址是否自增 */
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; /* 目的数据带宽 */
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			/* 源数据宽度 */
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;							/* 单次传输模式/循环传输模式 */
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;					/* DMA优先级 */
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;					/* FIFO模式/直接模式 */
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;		/* FIFO大小 */
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;				/* 单次传输 */
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream5, &DMA_InitStructure); // DMA1_Channel3 配置
	// !如果使用DMA接收中断，则需要下面代码配置DMA接收中断
	// DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, ENABLE);                      // DMA接收缓冲区满中断使能

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_Cmd(USART2, ENABLE);
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
	DMA_Cmd(DMA1_Stream5, ENABLE);
}

void USART2_DMA_ReceData(uint8_t *data)
{
	DMA_Cmd(DMA1_Stream5, DISABLE); // 关闭DMA设备（需要在DMA关闭的状态下才能进行配置）

	DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_TCIF5); // 这是必要的，如果不提前清除标志位，将无法使能dma，代码将会卡死在此函数的第二个循环里面

	while (DMA_GetCmdStatus(DMA1_Stream5) != DISABLE)
		; // 等待关闭完成

	DMA1_Stream5->M0AR = (uint32_t)data; // 接收数据所放的地址
	DMA1_Stream5->NDTR = (uint32_t)U2_buffer_handle->size;
	DMA_Cmd(DMA1_Stream5, ENABLE);					 // 开启DMA（等待接收数据）
	while (DMA_GetCmdStatus(DMA1_Stream5) != ENABLE) // 等待开启完成
		DMA_Cmd(DMA1_Stream5, ENABLE);				 // 开启DMA（等待接收数据）
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);	 // 串口DMA接收使能
}

void DMA1_Stream5_IRQHandler()
{
	if (DMA_GetFlagStatus(DMA1_Stream5, DMA_FLAG_TCIF5) == SET)
	{
		DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_TCIF5);
		DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
		// Write_BUFF_P((U2_buffer_handle->size) - (DMA1_Stream5->NDTR), U2_buffer_handle);
		USART2_DMA_ReceData(U2_buffer_handle->head_p);
	}
}
void USART2_IRQHandler(void) // 串口1中断服务程序
{
	if (USART_GetITStatus(USART2, USART_IT_IDLE) != RESET) // 串口空闲中断
	{
		Write_BUFF_P((U2_buffer_handle->size) - (DMA1_Stream5->NDTR), U2_buffer_handle);
		// App_Printf("succeed:%d\n", U2_buffer_handle->buffer_used);
		USART_ReceiveData(USART2); // 清除空闲中断标志位（接收函数有清标志位的作用）
	}
}
void Init_USART2_All()
{
	Iinitial_BUFF(U2_buffer_handle, BUFFER_SIZE_U2);
	USART2_Config();
}
#endif
#ifdef NORMAL_USART2
static void NVIC_USART2_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* 嵌套向量中断控制器组选择 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* 配置USART为中断源 */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	/* 抢断优先级*/
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	/* 子优先级 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	/* 使能中断 */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/* 初始化配置NVIC */
	NVIC_Init(&NVIC_InitStructure);
}

static void USART2_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// 打开串口GPIO的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	// 打开串口外设的时钟

	// 将USART Tx的GPIO配置为推挽复用模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// 将USART Rx的GPIO配置为浮空输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	NVIC_USART2_Configuration();
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART2, ENABLE);
}
void USART2_IRQHandler()
{
	u8 buff_usart2;
	if (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == 1)
	{
		buff_usart2 = USART_ReceiveData(USART2);
		Write_BUFF(&buff_usart2, &U2_buffer);
	}
	USART_ClearFlag(USART2, USART_FLAG_RXNE);
}
void Init_USART2_All()
{
	Iinitial_BUFF(&U2_buffer, BUFFER_SIZE_U2);
	USART2_Config();
}
#endif
#ifdef DMA_USART3_RX
static void NVIC_USART3_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void USART3_Config(void)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE); // 开启DMA时钟
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);

	// 将USART Tx的GPIO配置为推挽复用模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	// 将USART Rx的GPIO配置为浮空输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	// 配置串口的工作参数
	// 配置波特率
	USART_InitStructure.USART_BaudRate = 115200;
	// 配置 针数据字长
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// 配置停止位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// 配置校验位
	USART_InitStructure.USART_Parity = USART_Parity_No;
	// 配置硬件流控制
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// 配置工作模式，收发一起
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// 完成串口的初始化配置
	USART_Init(USART3, &USART_InitStructure);

	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
	NVIC_USART3_Configuration();

	DMA_DeInit(DMA1_Stream1);
	while (DMA_GetCmdStatus(DMA1_Stream1) != DISABLE)
		; // 等待DMA可配置
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32) & (USART3->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)U3_buffer_handle->Data;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = BUFFER_SIZE_U3;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		/* 外设地址是否自增 */
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					/* 内存地址是否自增 */
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; /* 目的数据带宽 */
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			/* 源数据宽度 */
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;							/* 单次传输模式/循环传输模式 */
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;					/* DMA优先级 */
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;					/* FIFO模式/直接模式 */
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;		/* FIFO大小 */
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;				/* 单次传输 */
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream1, &DMA_InitStructure); // DMA1_Channel3 配置
	// DMA_ITConfig(DMA1_Stream1, DMA_IT_TC, ENABLE);                      // DMA接收缓冲区满中断使能

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_Cmd(USART3, ENABLE);
	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
	DMA_Cmd(DMA1_Stream1, ENABLE);
	// DMA1_Stream3->NDTR
}
void DMA1_Stream1_IRQHandler()
{
	if (DMA_GetFlagStatus(DMA1_Stream1, DMA_FLAG_TCIF1) != RESET)
	{
		Write_BUFF_P(0, U3_buffer_handle);
	}
	DMA_ClearFlag(DMA1_Stream1, DMA_FLAG_TCIF1);
}
void USART3_IRQHandler(void) // 串口1中断服务程序
{
	if (USART_GetITStatus(USART3, USART_IT_IDLE) == SET) // 串口空闲中断
	{
		Write_BUFF_P(U3_buffer_handle->size - DMA1_Stream1->NDTR, &U3_buffer);
		USART_ReceiveData(USART3); // 清除空闲中断标志位（接收函数有清标志位的作用）
	}
}

void Init_USART3_All()
{
	Iinitial_BUFF(U3_buffer_handle, BUFFER_SIZE_U3);
	USART3_Config();
}
#endif
#ifdef NORMAL_USART3
static void NVIC_USART3_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

static void USART3_Config(void)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	// 将USART Tx的GPIO配置为推挽复用模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	// 将USART Rx的GPIO配置为浮空输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);

	// 配置串口的工作参数
	// 配置波特率
	USART_InitStructure.USART_BaudRate = 115200;
	// 配置 针数据字长
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// 配置停止位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// 配置校验位
	USART_InitStructure.USART_Parity = USART_Parity_No;
	// 配置硬件流控制
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// 配置工作模式，收发一起
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// 完成串口的初始化配置
	USART_Init(USART3, &USART_InitStructure);
	NVIC_USART3_Configuration();

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

	USART_Cmd(USART3, ENABLE);
}
void USART3_IRQHandler()
{
	u8 abn;

	if (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == 1)
	{
		abn = USART_ReceiveData(USART3);
		Write_BUFF(&abn, &U3_buffer);
	}
	USART_ClearFlag(USART3, USART_FLAG_RXNE);
}
void Init_USART3_All()
{
	Iinitial_BUFF(&U3_buffer, BUFFER_SIZE_U3);
	USART3_Config();
}
#endif
#ifdef DMA_UART4_RX
static void NVIC_UART4_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

static void UART4_Config(void)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE); // 开启DMA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_UART4);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_UART4);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &USART_InitStructure);
	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);
	NVIC_UART4_Configuration();
	DMA_DeInit(DMA1_Stream2);
	while (DMA_GetCmdStatus(DMA1_Stream2) != DISABLE)
		; // 等待DMA可配置
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32) & (UART4->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)U4_buffer.Data;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = BUFFER_SIZE_U4;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;				// 不开启FIFO模式
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;		// FIFO阈值
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;			// 存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; // 外设突发单次传输
	DMA_Init(DMA1_Stream2, &DMA_InitStructure);							// DMA1_Channel3 配置
	// DMA_ITConfig(DMA1_Stream2, DMA_IT_TC, ENABLE);                      // DMA接收缓冲区满中断使能
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	USART_Cmd(UART4, ENABLE);
	USART_DMACmd(UART4, USART_DMAReq_Rx, ENABLE);
	DMA_Cmd(DMA1_Stream2, ENABLE);
}
void DMA1_Stream2_IRQHandler()
{
//	u8 Tail4;
	if (DMA_GetFlagStatus(DMA1_Stream2, DMA_FLAG_TCIF2) != RESET)
	{
//		Tail4 = BUFFER_SIZE_U4 - DMA1_Stream2->NDTR; // 获取接收到的数据长度
		Write_BUFF_P(0, &U4_buffer);
	}
	DMA_ClearFlag(DMA1_Stream2, DMA_FLAG_TCIF2);
}
void UART4_IRQHandler(void) // 串口1中断服务程序
{
	if (USART_GetITStatus(UART4, USART_IT_IDLE) != RESET) // 串口空闲中断
	{
		Write_BUFF_P(BUFFER_SIZE_U4 - DMA1_Stream2->NDTR, U4_buffer_handle);
		USART_ReceiveData(UART4); // 清除空闲中断标志位（接收函数有清标志位的作用）
	}
}
void Init_UART4_All()
{
	Iinitial_BUFF(&U4_buffer, BUFFER_SIZE_U4);
	UART4_Config();
}
#endif
#ifdef NORMAL_UART4
static void NVIC_USART4_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* 嵌套向量中断控制器组选择 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* 配置USART为中断源 */
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	/* 抢断优先级*/
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	/* 子优先级 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	/* 使能中断 */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/* 初始化配置NVIC */
	NVIC_Init(&NVIC_InitStructure);
}

void USART4_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// 打开串口GPIO的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	// 打开串口外设的时钟
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_UART4);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_UART4);

	// 将USART Tx的GPIO配置为推挽复用模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// 将USART Rx的GPIO配置为浮空输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// 配置串口的工作参数
	// 配置波特率
	USART_InitStructure.USART_BaudRate = 115200;
	// 配置 针数据字长
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// 配置停止位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// 配置校验位
	USART_InitStructure.USART_Parity = USART_Parity_No;
	// 配置硬件流控制
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// 配置工作模式，收发一起
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// 完成串口的初始化配置
	USART_Init(UART4, &USART_InitStructure);

	// 串口中断优先级配置
	NVIC_USART4_Configuration();

	//	// 使能串口接收中断
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);

	// 使能串口
	USART_Cmd(UART4, ENABLE);
}
void UART4_IRQHandler()
{
	u8 abn;

	if (USART_GetFlagStatus(UART4, USART_FLAG_RXNE) == 1)
	{
		abn = USART_ReceiveData(UART4);
		Write_BUFF(&abn, &U4_buffer);
	}
	USART_ClearFlag(UART4, USART_FLAG_RXNE);
}
void Init_UART4_All()
{
	Iinitial_BUFF(&U4_buffer, BUFFER_SIZE_U4);
	USART4_Config();
}
#endif
#ifdef DMA_UART5_RX
static void NVIC_UART5_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
void UART5_Config(void)
{
	USART_InitTypeDef USART_InitStructure;
	USART_StructInit(&USART_InitStructure);
	GPIO_InitTypeDef GPIO_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE); // 开启DMA时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_UART5);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_UART5);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART5, &USART_InitStructure);
	USART_ITConfig(UART5, USART_IT_IDLE, ENABLE);
	NVIC_UART5_Configuration();
	DMA_DeInit(DMA1_Stream0);
	while (DMA_GetCmdStatus(DMA1_Stream0) != DISABLE)
		; // 等待DMA可配置
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32) & (UART5->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)U5_buffer.Data;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = BUFFER_SIZE_U5;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;				// 不开启FIFO模式
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;		// FIFO阈值
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;			// 存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; // 外设突发单次传输
	DMA_Init(DMA1_Stream0, &DMA_InitStructure);							// DMA1_Channel3 配置
	// DMA_ITConfig(DMA1_Stream0, DMA_IT_TC, ENABLE);                      // DMA接收缓冲区满中断使能
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	USART_Cmd(UART5, ENABLE);
	USART_DMACmd(UART5, USART_DMAReq_Rx, ENABLE);
	DMA_Cmd(DMA1_Stream0, ENABLE);
}

void DMA1_Stream0_IRQHandler()
{
	if (DMA_GetFlagStatus(DMA1_Stream0, DMA_FLAG_TCIF0) != RESET)
	{
		Write_BUFF_P(0, &U5_buffer);
	}
	DMA_ClearFlag(DMA1_Stream0, DMA_FLAG_TCIF0);
}
void UART5_IRQHandler(void) // 串口1中断服务程序
{
	u8 Tail5;
	if (USART_GetITStatus(UART5, USART_IT_IDLE) != RESET) // 串口空闲中断
	{
		Tail5 = BUFFER_SIZE_U5 - DMA1_Stream0->NDTR; // 获取接收到的数据长度
		Write_BUFF_P(Tail5, &U5_buffer);
		USART_ReceiveData(UART5); // 清除空闲中断标志位（接收函数有清标志位的作用）
	}
}
void Init_UART5_All()
{
	Iinitial_BUFF(&U5_buffer, BUFFER_SIZE_U5);
	UART5_Config();
}
#endif
#ifdef NORMAL_UART5
static void NVIC_UART5_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
void UART5_Config(void)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_UART5);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_UART5);
	// 将USART Tx的GPIO配置为推挽复用模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// 将USART Rx的GPIO配置为浮空输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	// 配置串口的工作参数
	// 配置波特率
	USART_InitStructure.USART_BaudRate = 115200;
	// 配置 针数据字长
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// 配置停止位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// 配置校验位
	USART_InitStructure.USART_Parity = USART_Parity_No;
	// 配置硬件流控制
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// 配置工作模式，收发一起
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// 完成串口的初始化配置
	USART_Init(UART5, &USART_InitStructure);
	NVIC_UART5_Configuration();

	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);

	USART_Cmd(UART5, ENABLE);
}
void UART5_IRQHandler()
{
	u8 abn;

	if (USART_GetFlagStatus(UART5, USART_FLAG_RXNE) == 1)
	{
		abn = USART_ReceiveData(UART5);
		Write_BUFF(&abn, &U5_buffer);
	}
	USART_ClearFlag(UART5, USART_FLAG_RXNE);
}
void Init_UART5_All()
{
	Iinitial_BUFF(&U5_buffer, BUFFER_SIZE_U5);
	UART5_Config();
}
#endif
union AH
{
	float fdata;
	char ldata[4];
} FloatLongType;
void VOFA_Send_float(float *Data, u8 b)
{
	u8 i;
	const u8 tail[4] = {0x00, 0x00, 0x80, 0x7f};
	while (b--)
	{
		FloatLongType.fdata = *Data;
		for (i = 0; i < 4; i++)
		{
			Usart_SendByte(USART1, FloatLongType.ldata[i]);
		}
		Data++;
	}

	for (i = 0; i < 4; i++)
	{
		Usart_SendByte(USART1, tail[i]);
	}
}
/*****************  发送一个字节 **********************/
void Usart_SendByte(USART_TypeDef *pUSARTx, uint8_t ch)
{
	/* 发送一个字节数据到USART */
	USART_SendData(pUSARTx, ch);

	/* 等待送数据寄存器为空 */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET)
		;
}
/****************** 发送8位的数组 ************************/
void Usart_SendArray(USART_TypeDef *pUSARTx, const uint8_t *array, uint16_t num)
{
	uint8_t i;

	for (i = 0; i < num; i++)
	{
		/* 发送一个字节数据到USART */
		Usart_SendByte(pUSARTx, array[i]);
	}
	/* 等待发送完成 */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TC) == RESET)
		;
}
/*****************  发送字符串 **********************/
void Usart_SendString(USART_TypeDef *pUSARTx, char *str)
{
	unsigned int k = 0;
	do
	{
		Usart_SendByte(pUSARTx, *(str + k));
		k++;
	} while (*(str + k) != '\0');

	/* 等待发送完成 */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TC) == RESET)
	{
	}
}
/*****************  发送一个16位数 **********************/
void Usart_SendHalfWord(USART_TypeDef *pUSARTx, uint16_t ch)
{
	uint8_t temp_h, temp_l;

	/* 取出高八位 */
	temp_h = (ch & 0XFF00) >> 8;
	/* 取出低八位 */
	temp_l = ch & 0XFF;

	/* 发送高八位 */
	USART_SendData(pUSARTx, temp_h);
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET)
		;

	/* 发送低八位 */
	USART_SendData(pUSARTx, temp_l);
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET)
		;
}
/// 重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
	/* 发送一个字节数据到串口 */
	USART_SendData(USART1, (uint8_t)ch);

	/* 等待发送完毕 */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
		;

	return (ch);
}
/// 重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
	/* 等待串口输入数据 */
	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
		;

	return (int)USART_ReceiveData(USART1);
}
