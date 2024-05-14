

#ifndef Driver__
#define Driver__
#include "stm32f4xx.h"
void Control_PIN(uint16_t GPIO_Pin_X, GPIO_TypeDef *GPIOX);
void Initial_Control_PIN(void);
void Advance(u8 a);
void Back(u8 a);
void PULL_Low(void);
void PULL_High(void);
#endif
