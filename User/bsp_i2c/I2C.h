#ifndef __I2C_H
#define __I2C_H
#include "sys.h" 
 
 
// I2C2初始化函数
void I2C2_Init(void);
 
// I2C2发送数据函数
// 参数：
//   address: 目标设备地址
//   reg: 寄存器地址
//   data: 要发送的数据
void I2C2_WriteData(uint8_t address, uint8_t reg, uint8_t data);
 
// I2C2接收数据函数
// 参数：
//   address: 目标设备地址
//   reg: 寄存器地址
// 返回值：
//   读取到的数据
uint8_t I2C2_ReadData(uint8_t address, uint8_t reg);

// I2C2发送多个数据函数
// 参数：
//   address: 目标设备地址
//   reg: 寄存器地址
//   data: 要发送的数据
//   length: 要发送的数据长度
void I2C2_WriteMultiData(uint8_t address, uint8_t reg, const uint8_t *data, uint8_t length);
 
 
 
 
#endif

