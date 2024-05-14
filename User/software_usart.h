#ifndef software_usart__
#define software_usart__
#include "stm32f4xx.h"
#define BAUDRATE 104

#define SUSART_GPIOX_TX GPIOC
#define SUSART_GPIOX_PIN_TX GPIO_Pin_3
#define SUSART_GPIOX_RCC_TX RCC_AHB1Periph_GPIOC

#define SUSART_GPIOX_RX GPIOB
#define SUSART_GPIOX_PIN_RX GPIO_Pin_14
#define SUSART_GPIOX_RCC_RX RCC_AHB1Periph_GPIOB
//!  4 -> 256000  9->115200
void Software_USART_TXD(u8 Data);
void USART_Send(u8 *buf, u8 len);
void Software_USART_IOConfig(void);
void TIM7_Int_Init(u16 arr, u16 psc);
#endif
