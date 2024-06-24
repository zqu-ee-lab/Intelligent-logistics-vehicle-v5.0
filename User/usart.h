/*
 * @Author: JAR_CHOW
 * @Date: 2024-05-14 20:47:46
 * @LastEditors: JAR_CHOW
 * @LastEditTime: 2024-06-20 22:31:51
 * @FilePath: \RVMDK（uv5）c:\Users\mrchow\Desktop\vscode_repo\Intelligent-logistics-vehicle-v5.0\User\usart.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#ifndef __USART_H
#define __USART_H

#include "stm32f4xx.h"
#include <stdio.h>

// 串口1-USART1
#define DMA_USART1_RX_TX
#define USART1_TX_BUF_SIZE  64
void USART1_DMA_ReceData(uint8_t *data);
void USART1_DMA_TranData(uint8_t *data,uint32_t dataLen);
void App_Printf(char *format, ...);
void Init_USART1_All(void);

// 串口2-USART1
#define DMA_USART2_RX //*使用DMA接受数据#define DMA_USART2_RX  #define DMA_USART2_RX和#define NORMAL_USART2不能同时出现 下同
void Init_USART2_All(void);
// 串口3-USART3
#define NORMAL_USART3
void Init_USART3_All(void);
// 串口4-UART4
#define DMA_UART4_RX
void Init_UART4_All(void);
// 串口5-UART5
#define DMA_UART5_RX
void Init_UART5_All(void);

void VOFA_Send_float(float *Data, uint8_t b);
void Usart_SendByte(USART_TypeDef *pUSARTx, uint8_t ch);
void Usart_SendString(USART_TypeDef *pUSARTx, char *str);
void Usart_SendHalfWord(USART_TypeDef *pUSARTx, uint16_t ch);
void Usart_SendArray(USART_TypeDef *pUSARTx, const uint8_t *array, uint16_t num);
#endif /* __USART_H */
