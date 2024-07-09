/*
 * @Author: error: git config user.name && git config user.email & please set dead value or install git
 * @Date: 2023-01-06 20:38:59
 * @LastEditors: JAR_CHOW
 * @LastEditTime: 2024-07-09 20:13:30
 * @FilePath: \RVMDK（uv5）c:\Users\mrchow\Desktop\vscode_repo\Intelligent-logistics-vehicle-v5.0\User\PWM.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef TIME

#define TIME

#include "stm32f4xx.h"
void PWM_TIM1_config(uint16_t arr, uint16_t psc, uint16_t CCR1_Val, uint16_t CCR2_Val, uint16_t CCR3_Val, uint16_t CCR4_Val);
void SetCompare1(TIM_TypeDef *TIMX, u32 CCRX_Val, uint8_t X);
void PWM_TIM8_config(uint16_t arr, uint16_t psc, uint16_t CCR1_Val, uint16_t CCR2_Val, uint16_t CCR3_Val, uint16_t CCR4_Val);
void PWM_TIM9_config(uint16_t arr, uint16_t psc, uint16_t CCR1_Val, uint16_t CCR2_Val);
void ControledMonitor(u8 channel, u16 Count);
#endif
