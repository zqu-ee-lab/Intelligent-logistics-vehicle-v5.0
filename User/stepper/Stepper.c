#include "Stepper.h" // User/stepper/Stepper.h
#include "usart.h"	 // User/usart.h
#include "FreeRTOS.h"
#include "delay.h"
#include "buffer.h"

void Stepper_Send_Instruction(struct Steeper_t *this, uint8_t *data, uint32_t dataLen)
{
	Usart_SendArray(this->pUSARTx, data, dataLen);
}

void Stepper_Achieve_Distance(struct Steeper_t *this, enum Stepper_Direction_t direction, uint32_t distance, uint32_t speed, bool synchronizate)
{
	// calculate speed according to the period

	if (this->FOC_VERSION == Stepper_FOC_Version_5_0)
	{
		uint8_t data[13] = {0};
		data[0] = this->address;
		data[1] = 0xFD; // Distance controlled instruction
		data[2] = direction;

		data[3] = speed >> 8;
		data[4] = speed;

		data[5] = this->acceleration;

		data[6] = distance >> 24;
		data[7] = distance >> 16;
		data[8] = distance >> 8;
		data[9] = distance;

		data[10] = 0x00;
		data[11] = 0x00; // multiple instructions
		if(synchronizate)
		{
			data[11] = 0x01;
		}

		if (this->check_way == Stepper_Check_Way_XOR)
		{
			uint8_t check = 0;
			for (int i = 0; i < 12; i++)
			{
				check ^= data[i];
			}
			data[12] = check;
		}
		else if (this->check_way == Stepper_Check_Way_0X6B)
		{
			data[12] = 0x6B;
		}
		else
		{
			// error
			App_Printf("Stepper_Achieve_Distance: check way error!\n");
			return;
		}

		this->Send_Instruction(this, data, 13);
		return;
	}
	else if (this->FOC_VERSION == Stepper_FOC_Version_4_2)
	{
		uint8_t data[9] = {0};

		data[0] = this->address;
		data[1] = 0xFD; // Distance controlled instruction

		if (speed > 0x4FF)
		{
			speed = 0x4FF;
		}
		speed = 0x4FF;
		*(uint16_t *)(data + 2) = speed;
		if (direction == Stepper_Forward)
		{
			data[2] |= 0x10;
		}
		else
		{
			data[2] &= 0x0F;
		}

		// data[4] = this->acceleration;
		data[4] = 0xFF;

		if (distance > 0xFFFFFF)
		{
			distance = 0xFFFFFF;
		}
		data[5] = distance >> 16;
		data[6] = distance >> 8;
		data[7] = distance;

		if (this->check_way == Stepper_Check_Way_XOR)
		{
			uint8_t check = 0;
			for (int i = 0; i < 8; i++)
			{
				check ^= data[i];
			}
			data[8] = check;
		}
		else if (this->check_way == Stepper_Check_Way_0X6B)
		{
			data[8] = 0x6B;
		}
		else
		{
			// error
			App_Printf("Stepper_Achieve_Distance: check way error!\n");
			return;
		}

		this->Send_Instruction(this, data, 9);
		return;
	}
}



void Stepper_Speed_Control(struct Steeper_t *this, enum Stepper_Direction_t direction, uint32_t speed, bool synchronizate)
{
	if(this->FOC_VERSION == Stepper_FOC_Version_4_2)
	{
		App_Printf("Stepper_Speed_Control: FOC version error!\n");
		return;
	}
	
	uint8_t data[8] = {0};
	data[0] = this->address;
	data[1] = 0xF6; // Speed controlled instruction
	data[2] = direction^(this->direction_invert);

	data[3] = speed >> 8;
	data[4] = speed;

	data[5] = 0x00;	// acceleration

	if(synchronizate)
	{
		data[6] = 0x01;
	}

	if (this->check_way == Stepper_Check_Way_XOR)
	{
		uint8_t check = 0;
		for (int i = 0; i < 7; i++)
		{
			check ^= data[i];
		}
		data[7] = check;
	}
	else if (this->check_way == Stepper_Check_Way_0X6B)
	{
		data[7] = 0x6B;
	}
	else
	{
		// error
		App_Printf("Stepper_Speed_Control: check way error!\n");
		return;
	}

	this->Send_Instruction(this, data, 8);
}

void Stepper_synchronization(USART_TypeDef * pUSARTx)
{
	uint8_t data[4];
	data[0] = 0x00;
	data[1] = 0xFF;
	data[2] = 0x66;

	// data[3] = 0xFF ^ 0x66;
	data[3] = 0x6B;
	Usart_SendArray(pUSARTx, data, 4);
}

int32_t Stepper_Read_Current_Position(struct Steeper_t *this)
{
	uint8_t data[3] = {0};
	data[0] = this->address;
	data[1] = 0x36; // Read current position instruction

	if (this->check_way == Stepper_Check_Way_XOR)
	{
		uint8_t check = 0;
		for (int i = 0; i < 2; i++)
		{
			check ^= data[i];
		}
		data[2] = check;
	}
	else if (this->check_way == Stepper_Check_Way_0X6B)
	{
		data[2] = 0x6B;
	}
	else
	{
		// error
		App_Printf("Stepper_Read_Current_Position: check way error!\n");
		return 0;
	}

	this->Send_Instruction(this, data, 3);
	Delayms(3); // wait for the response

	int32_t current_position = 0;
	uint8_t data_res[5] = {0};
	char head[2] = {this->address, 0x36};
	// App_Printf("sizeof_buff: %d\n", this->BUFF->buffer_used);
	if (this->FOC_VERSION == Stepper_FOC_Version_5_0)
	{
		if (BUFF_pop_by_Protocol(this->BUFF, head, 2, data_res, 5) == 5)
		{
			current_position = (data_res[1] << 24) | (data_res[2] << 16) | (data_res[3] << 8) | data_res[4];
			// current_position = current_position / 65535 * 200 * 256;
			current_position = data_res[0] ? -current_position : current_position;
			return current_position;
		}
		else
		{
			App_Printf("Stepper_Read_Current_Position: pop error!\n");
			return 0;
		}
	}

	else if (this->FOC_VERSION == Stepper_FOC_Version_4_2)
	{
		if (BUFF_pop_by_Protocol(this->BUFF, head, 1, data_res, 4) == 4)
		{
			current_position = (data_res[0] << 24) | (data_res[1] << 16) | (data_res[2] << 8) | data_res[3];
			// current_position = current_position * 360 / 65536.;
			return current_position;
		}
		else
		{
			App_Printf("Stepper_Read_Current_Position: pop error!\n");
			return 0;
		}
	}
	return 0;
}

int32_t Stepper_get_current_position_from_buff(struct Steeper_t *this)
{
	int32_t current_position = 0;
	uint8_t data_res[5] = {0};
	char head[2] = {this->address, 0x36};
	// App_Printf("sizeof_buff: %d\n", this->BUFF->buffer_used);
	if (this->FOC_VERSION == Stepper_FOC_Version_5_0)
	{
		if (BUFF_pop_by_Protocol(this->BUFF, head, 2, data_res, 5) == 5)
		{
			current_position = (data_res[1] << 24) | (data_res[2] << 16) | (data_res[3] << 8) | data_res[4];
			// current_position = current_position / 65535 * 200 * 256;
			current_position = data_res[0] ? -current_position : current_position;
			return current_position;
		}
		else
		{
			//            App_Printf("Stepper_Read_Current_Position: pop error!\n");
			return -0x3f3f3f3f;
		}
	}

	else if (this->FOC_VERSION == Stepper_FOC_Version_4_2)
	{
		if (BUFF_pop_by_Protocol(this->BUFF, head, 1, data_res, 4) == 4)
		{
			current_position = (data_res[0] << 24) | (data_res[1] << 16) | (data_res[2] << 8) | data_res[3];
			// current_position = current_position / 65536. * 360;
			return current_position;
		}
		else
		{
			//            App_Printf("Stepper_Read_Current_Position: pop error!\n");
			return -0x3f3f3f3f;
		}
	}
	return 0;
}

void Stepper_unInit(struct Steeper_t *this)
{
	vPortFree(this);
}

struct Steeper_t *Stepper_Init(USART_TypeDef *pUSARTx, uint8_t address, struct Buff *BUFF, enum Stepper_Check_Way_t check_way, enum Stepper_FOC_Version_t FOC_VERSION)
{
	struct Steeper_t *this = NULL;
	this = (struct Steeper_t *)pvPortMalloc(sizeof(struct Steeper_t));
	if (this == NULL)
	{
		App_Printf("Stepper_Init: pvPortMalloc error!\n");
		while (1)
		{
			;
		}
	}
	this->pUSARTx = pUSARTx;
	this->address = address;
	this->direction_invert = 0x01;	 // default direction invert
	this->BUFF = BUFF;
	this->acceleration = 0x00;		 // default acceleration
	this->period = 10;				 // unit : ms
	this->check_way = check_way;	 // default check way
	this->FOC_VERSION = FOC_VERSION; // default FOC version

	this->Init = Stepper_Init;
	this->unInit = Stepper_unInit;
	this->Read_Current_Position = Stepper_Read_Current_Position;
	this->Achieve_Distance = Stepper_Achieve_Distance;
	this->Send_Instruction = Stepper_Send_Instruction;
	this->get_current_position_from_buff = Stepper_get_current_position_from_buff;

	return this;
}
