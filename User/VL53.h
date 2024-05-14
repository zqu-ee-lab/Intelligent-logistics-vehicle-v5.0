#ifndef VL53__
#define VL53__

#include "stm32f4xx.h"

#define VL53_USARTX USART2                      // 激光测距用到的串口
#define VL53_USARTX_IRQn USART2_IRQn            // 中断源
#define VL53_GPIO_CLOCK RCC_AHB1Periph_GPIOA    // USART的GPIO
#define VL53_USARTX_CLOCK RCC_APB1Periph_USART2 // USART时钟源   //注：注意时钟总线是挂载APB1，还是APB2
#define VL53_TX GPIO_Pin_2
#define VL53_RX GPIO_Pin_3
#define VL53_GPIOX GPIOA
#define VL53_BAUDRATE 921600 // 波特率

#define VL53_USARTX_IRQHandler USART2_IRQHandler // 中断服务函数
extern struct Buff VL53_USARTX_Buff;
void VL53_Initial(void);
void VL53_Read_Data(u16 *distance);
void VL53_Send_Agrement(void);
#endif
