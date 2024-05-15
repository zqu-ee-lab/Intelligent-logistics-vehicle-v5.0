#ifndef STEPPER_H
#define STEPPER_H

#include "stdbool.h"
#include "stm32f4xx.h" // Device header
#include "buffer.h"

enum Stepper_Direction_t
{
	Stepper_Forward = 0x00,
	Stepper_Backward = 0x01
};

enum Stepper_Check_Way_t
{
	Stepper_Check_Way_XOR = 0x00,
	Stepper_Check_Way_0X6B = 0x01
};

enum Stepper_FOC_Version_t
{
	Stepper_FOC_Version_4_2 = 0x01,
	Stepper_FOC_Version_5_0 = 0x02
};

struct Steeper_t *Stepper_Init(USART_TypeDef *pUSARTx, uint8_t address, struct Buff *BUFF, enum Stepper_Check_Way_t check_way, enum Stepper_FOC_Version_t FOC_VERSION);

struct Steeper_t
{
	USART_TypeDef *pUSARTx;
	uint8_t address;
	struct Buff *BUFF;
	uint8_t acceleration;
	uint8_t period;
	enum Stepper_Check_Way_t check_way;
	enum Stepper_FOC_Version_t FOC_VERSION;

	struct Steeper_t *(*Init)(USART_TypeDef *pUSARTx, uint8_t address, struct Buff *BUFF, enum Stepper_Check_Way_t check_way, enum Stepper_FOC_Version_t FOC_VERSION);
	void (*unInit)(struct Steeper_t *this);
	void (*Send_Instruction)(struct Steeper_t *this, uint8_t *data, uint32_t dataLen);
	int32_t (*Read_Current_Position)(struct Steeper_t *this);
	void (*Achieve_Distance)(struct Steeper_t *this, enum Stepper_Direction_t direction, uint32_t distance, uint32_t speed, bool synchronizate);
	int32_t (*get_current_position_from_buff)(struct Steeper_t *this);
	void (*Speed_Control)(struct Steeper_t *this, enum Stepper_Direction_t direction, uint32_t speed, bool synchronizate);
};

void Stepper_Achieve_Distance(struct Steeper_t *this, enum Stepper_Direction_t direction, uint32_t distance, uint32_t speed, bool synchronizate);
void Stepper_synchronization(USART_TypeDef * USART);
int32_t Stepper_get_current_position_from_buff(struct Steeper_t *this);

#endif /* STEPPER_H */
