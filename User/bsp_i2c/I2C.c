#include "stm32f4xx.h" // Device header
#include "I2C.h"

// I2C2初始化函数
void I2C2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

    // 打开I2C2和GPIOB的时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    // 配置GPIOB的引脚为I2C2功能
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 配置GPIO引脚复用功能
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_I2C2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_I2C2);

    // I2C2配置
    I2C_InitStructure.I2C_ClockSpeed = 400000; // 设置时钟速度为400kHz
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C2, &I2C_InitStructure);

    // 使能I2C2
    I2C_Cmd(I2C2, ENABLE);
}

// I2C2发送数据函数
void I2C2_WriteData(uint8_t address, uint8_t reg, uint8_t data)
{
    int timeout = 0;
    // 等待I2C2处于空闲状态
    timeout = 0x47;
    while ((--timeout) && I2C_GetFlagStatus(I2C2, I2C_FLAG_BUSY))
        ;
    I2C_ClearFlag(I2C2, I2C_FLAG_BUSY);

    // 发送START信号
    I2C_GenerateSTART(I2C2, ENABLE);
    timeout = 0x47;
    while ((--timeout) && !I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))
        ;

    // 发送目标设备地址和写指令
    I2C_Send7bitAddress(I2C2, address, I2C_Direction_Transmitter);
    timeout = 0x47;
    while ((--timeout) && !I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        ;

    // 发送寄存器地址
    I2C_SendData(I2C2, reg);
    timeout = 0x47;
    while ((--timeout) && !I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        ;

    // 发送数据
    I2C_SendData(I2C2, data);
    timeout = 0x47;
    while ((--timeout) && !I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        ;

    // 发送STOP信号
    I2C_GenerateSTOP(I2C2, ENABLE);
}

void I2C2_WriteMultiData(uint8_t address, uint8_t reg, const uint8_t *data, uint8_t length)
{
    int timeout = 0;
    // 等待I2C2处于空闲状态
    timeout = 0x47;
    while ((--timeout) && I2C_GetFlagStatus(I2C2, I2C_FLAG_BUSY))
        ;
    I2C_ClearFlag(I2C2, I2C_FLAG_BUSY);
    // 发送START信号
    I2C_GenerateSTART(I2C2, ENABLE);
    timeout = 0x47;
    while ((--timeout) && !I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))
        ;

    // 发送目标设备地址和写指令
    I2C_Send7bitAddress(I2C2, address, I2C_Direction_Transmitter);
    timeout = 0x47;
    while ((--timeout) && !I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        ;

    // 发送寄存器地址
    I2C_SendData(I2C2, reg);
    timeout = 0x47;
    while ((--timeout) && !I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        ;

    // 发送数据
    for (uint8_t i = 0; i < length; i++)
    {
        I2C_SendData(I2C2, data[i]);
        timeout = 0x47;
        while ((--timeout) && !I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
            ;
    }

    // 发送STOP信号
    I2C_GenerateSTOP(I2C2, ENABLE);
}

// I2C2接收数据函数
uint8_t I2C2_ReadData(uint8_t address, uint8_t reg)
{
    uint8_t data;

    // 等待I2C2处于空闲状态
    while (I2C_GetFlagStatus(I2C2, I2C_FLAG_BUSY))
        ;

    // 发送START信号
    I2C_GenerateSTART(I2C2, ENABLE);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))
        ;

    // 发送目标设备地址和写指令
    I2C_Send7bitAddress(I2C2, address, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        ;

    // 发送寄存器地址
    I2C_SendData(I2C2, reg);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        ;

    // 重新发送START信号，切换到接收模式
    I2C_GenerateSTART(I2C2, ENABLE);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))
        ;

    // 发送目标设备地址和读指令
    I2C_Send7bitAddress(I2C2, address, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
        ;

    // 开启ACK
    I2C_AcknowledgeConfig(I2C2, ENABLE);

    // 接收数据
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED))
        ;
    data = I2C_ReceiveData(I2C2);

    // 发送STOP信号
    I2C_GenerateSTOP(I2C2, ENABLE);

    return data;
}
