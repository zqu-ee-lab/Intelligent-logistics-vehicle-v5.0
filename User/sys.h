#ifndef SY
#define SY

#include "stm32f4xx.h"
#include "FreeRTOS.h"
void Read_RGB(void);
void Forward(void);
void Reverse(void);
void Turn_Right_Founction(void);
void Turn_Left_Founction(void);
void Walking_Right(void);
void Walking_Left(void);
extern struct COLOR RGB;
struct COLOR
{
    u8 R;
    u8 G;
    u8 B;
};

struct angle
{
    uint8_t data[9];
    int16_t z;
};

extern int32_t x_target;
extern int32_t y_target;

extern const uint8_t bigone[4][32];
extern const uint8_t bigtwo[4][32];
extern const uint8_t bigthree[4][32];


/**
 * those following marcos are used to operate the event group
 */
#define WAIT_FOR_EVENT(specific_event_bit) xEventGroupWaitBits(Group_One_Handle, (specific_event_bit), pdTRUE, pdTRUE, portMAX_DELAY)
#define WAIT_FOR_EVENT_UNTIL(specific_event_bit, wait_time) xEventGroupWaitBits(Group_One_Handle, (specific_event_bit), pdTRUE, pdTRUE, (wait_time))
#define CHECK_EVENT(specific_event_bit) (xEventGroupGetBits(Group_One_Handle) & (specific_event_bit))
#define CLEAR_EVENT(specific_event_bit) xEventGroupClearBits(Group_One_Handle, (specific_event_bit))
#define SET_EVENT(specific_event_bit) xEventGroupSetBits(Group_One_Handle, (specific_event_bit))

/**
 * those following marcos are used to define the specific event bit
 */
#define GAME_OVER (1 << 0)
#define GOT_QUADRANGLE (1 << 1)
#define GOT_DOT_RED (1 << 2)
#define GOT_DOT_GREEN (1 << 3)

#define FOLLWING (1 << 4)


/**
 * those following marcos are used to define the specific key bit
 */
#define up___ (uint8_t)KEY_1_DOWN
#define enter___ (uint8_t)KEY_2_DOWN
#define down___ (uint8_t)KEY_3_DOWN
#define back___ (uint8_t)KEY_2_LONG

#endif
