#include "Software_iic.h"
#include "delay.h"
void IIC_Init(void)	// IIC初始化
{
	GPIO_InitTypeDef GPIO_InitStructure;
    // 这一句保留着，不要删掉，禁用掉JTAG
	RCC_AHB1PeriphClockCmd(SCLPinCLK,ENABLE);
	RCC_AHB1PeriphClockCmd(SDAPinCLK,ENABLE);
	
	// 初始化SCL引脚
	GPIO_InitStructure.GPIO_Pin = SCLPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(SCLPort, &GPIO_InitStructure);
	
	// 初始化SDA引脚
	GPIO_InitStructure.GPIO_Pin = SDAPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(SDAPort, &GPIO_InitStructure);
	
	GPIO_SetBits(SCLPort,SCLPin);
	GPIO_SetBits(SDAPort,SDAPin); 	

}
//
void IIC_SDA_In(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = SDAPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(SDAPort, &GPIO_InitStructure);
}
void IIC_SDA_Out(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = SDAPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(SDAPort, &GPIO_InitStructure);
}

void IIC_Start(void)	// IIC起始
{
	IIC_SDA_Out();
	GPIO_WriteBit(SDAPort,SDAPin,Bit_SET);
	GPIO_WriteBit(SCLPort,SCLPin,Bit_SET);
	Delayus(4);
	GPIO_WriteBit(SDAPort,SDAPin,Bit_RESET);
	Delayus(4);
	GPIO_WriteBit(SCLPort,SCLPin,Bit_RESET);
}
void IIC_Stop(void)	// IIC结束
{
	IIC_SDA_Out();
	GPIO_WriteBit(SCLPort,SCLPin,Bit_RESET);
	GPIO_WriteBit(SDAPort,SDAPin,Bit_RESET);
	Delayus(4);
	GPIO_WriteBit(SCLPort,SCLPin,Bit_SET);
	GPIO_WriteBit(SDAPort,SDAPin,Bit_SET);
	Delayus(4);
}

//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)		//等待Ack
{
	u8 ucErrTime=0;
	IIC_SDA_In();
	GPIO_WriteBit(SDAPort,SDAPin,Bit_SET);
	Delayus(1);
	GPIO_WriteBit(SCLPort,SCLPin,Bit_SET);
	Delayus(1);
	while(GPIO_ReadInputDataBit(SDAPort,SDAPin))
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	GPIO_WriteBit(SCLPort,SCLPin,Bit_RESET);
	return 0;
	
}

void IIC_Ack(void)	//IICAck
{
	GPIO_WriteBit(SCLPort,SCLPin,Bit_RESET);
	IIC_SDA_Out();
	GPIO_WriteBit(SDAPort,SDAPin,Bit_RESET);
	Delayus(2);
	GPIO_WriteBit(SCLPort,SCLPin,Bit_SET);
	Delayus(2);
	GPIO_WriteBit(SCLPort,SCLPin,Bit_RESET);
}
void IIC_NAck(void)  //IICNAck
{
	GPIO_WriteBit(SCLPort,SCLPin,Bit_RESET);
	IIC_SDA_Out();
	GPIO_WriteBit(SDAPort,SDAPin,Bit_SET);
	Delayus(2);
	GPIO_WriteBit(SCLPort,SCLPin,Bit_SET);
	Delayus(2);
	GPIO_WriteBit(SCLPort,SCLPin,Bit_RESET);
}

// 写一个字节
void IIC_Send_Byte(uint8_t byte)
{
	uint8_t t;
	IIC_SDA_Out();
	GPIO_WriteBit(SCLPort,SCLPin,Bit_RESET);
	for(t=0;t<8;t++)
  {              
		if((byte&0x80)>>7)
			GPIO_WriteBit(SDAPort,SDAPin,Bit_SET);
		else
			GPIO_WriteBit(SDAPort,SDAPin,Bit_RESET);
		byte<<=1; 	  
		Delayus(2);   //对TEA5767这三个延时都是必须的
		GPIO_WriteBit(SCLPort,SCLPin,Bit_SET);
		Delayus(2); 
		GPIO_WriteBit(SCLPort,SCLPin,Bit_RESET);
		Delayus(2);
  }
}
// 读一个字节
uint8_t IIC_Read_Byte(uint8_t ack)
{
	unsigned char i,receive=0;
	IIC_SDA_In();//SDA设置为输入
   for(i=0;i<8;i++ )
	{
		GPIO_WriteBit(SCLPort,SCLPin,Bit_RESET);
		Delayus(2);
		GPIO_WriteBit(SCLPort,SCLPin,Bit_SET);
		receive<<=1;
        if(GPIO_ReadInputDataBit(SDAPort,SDAPin))receive++;   
		Delayus(1); 
    }					 
    if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK   
    return receive;
}
/**********************************************
函数名称：MPU_Write_Len
函数功能：IIC连续写(写器件地址、寄存器地址、数据)
函数参数：addr:器件地址      reg:寄存器地址
				 len:写入数据的长度  buf:数据区
函数返回值：0,写入成功  其他,写入失败
**********************************************/
u8 IIC_Write_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
	u8 i;
	
	IIC_Start();
	IIC_Send_Byte((addr<<1)|0);      //发送器件地址+写命令(0为写,1为读)	
	if(IIC_Wait_Ack())							 //等待应答
	{
		IIC_Stop();
		return 1;
	}
    IIC_Send_Byte(reg);						 //写寄存器地址
    IIC_Wait_Ack();		             //等待应答
	for(i=0;i<len;i++)
	{
		IIC_Send_Byte(buf[i]);	       //发送数据
		if(IIC_Wait_Ack())		         //等待ACK
		{
			IIC_Stop();
			return 1;
		}
	}
    IIC_Stop();
	return 0;
}

/**********************************************
函数名称：MPU_Read_Len
函数功能：IIC连续读(写入器件地址后,读寄存器地址、数据)
函数参数：addr:器件地址        reg:要读的寄存器地址
				 len:要读取的数据长度  buf:读取到的数据存储区
函数返回值：0,读取成功  其他,读取失败
**********************************************/
u8 IIC_Read_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
		IIC_Start();
		IIC_Send_Byte((addr<<1)|0);		//发送器件地址+写命令
		if(IIC_Wait_Ack())						//等待应答
		{
			IIC_Stop();		 
			return 1;
		}
    IIC_Send_Byte(reg);						//写寄存器地址
    IIC_Wait_Ack();								//等待应答
    IIC_Start();
		IIC_Send_Byte((addr<<1)|1);		//发送器件地址+读命令	
    IIC_Wait_Ack();								//等待应答 
		while(len)
		{
			if(len==1) *buf=IIC_Read_Byte(0);   //读数据,发送nACK 
			else 			 *buf=IIC_Read_Byte(1);		//读数据,发送ACK  
			len--;
			buf++;
		}
    IIC_Stop();	//产生一个停止条件 
		return 0;	
}
