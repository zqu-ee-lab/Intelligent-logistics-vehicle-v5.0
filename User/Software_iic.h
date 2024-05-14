#ifndef _IIC_H
#define _IIC_H

#include "stm32f4xx.h"

// 只需要修改下方需要使用的端口，以及时钟即可，注意一定要全部修改好，
#define SCLPort GPIOB		//SCL引脚端口
#define SCLPin GPIO_Pin_4			//SCL引脚
#define SCLPinCLK RCC_AHB1Periph_GPIOB

#define SDAPort GPIOB   //SDA引脚端口
#define SDAPin GPIO_Pin_5			//SDA引脚
#define SDAPinCLK RCC_AHB1Periph_GPIOB


void IIC_Init(void);	// IIC初始化
// 这里，因为SDA需要实现写和读的功能，因此写了两个，需要读的时候就调用in，需要写的时候调用out，默认out
void IIC_SDA_In(void);
void IIC_SDA_Out(void);

void IIC_Start(void);	// IIC起始
void IIC_Stop(void);	// IIC结束

uint8_t IIC_Wait_Ack(void);

void IIC_Ack(void);		//IICAck
void IIC_NAck(void);  //IICNAck

// 写一个字节
void IIC_Send_Byte(uint8_t byte);
// 读一个字节
uint8_t IIC_Read_Byte(uint8_t ack);
u8 IIC_Write_Len(u8 addr,u8 reg,u8 len,u8 *buf);
u8 IIC_Read_Len(u8 addr,u8 reg,u8 len,u8 *buf);
#endif
