
#ifndef buffer__
#define buffer__
#include "stm32f4xx.h"
#include "FreeRTOS.h"

#define BUFF_MALLOC pvPortMalloc
#define BUFF_FREE vPortFree

#define IS_FULL 1 + 0xFF
#define IS_EMPTY 2 + 0xFF

#define BUFFER_SIZE 128
#define BUFFER_SIZE_U1 30
#define BUFFER_SIZE_U2 30
#define BUFFER_SIZE_U3 30
#define BUFFER_SIZE_U4 128
#define BUFFER_SIZE_U5 30

extern struct Buff *U3_buffer_handle, *U2_buffer_handle, *Soft_Usart_handle, *U1_buffer_handle, *U4_buffer_handle, *U5_buffer_handle, *IIC_buff_handle;
struct Buff
{
    u8* Data;
    int size;
    void *head_p;
    void *end_p;
    void *write_p;
    void *read_p;
    int buffer_used;
};


// extern struct Buff BUFF;
extern struct Buff U3_buffer, U2_buffer, IIC_buff, U4_buffer, U5_buffer;
void Iinitial_BUFF(struct Buff *BUFF, u8 size_);
void Write_BUFF_P(u8 num, struct Buff *BUFF);
void Write_BUFF(u8 *P, struct Buff *BUFF);
int Read_BUFF(struct Buff *BUFF);
u8 Find_Char(struct Buff *BUFF, char *p);
int BUFF_pop_by_Protocol(struct Buff *BUFF, const char* head, int head_len, void* data, int data_len);
int BUFF_pop_with_check_by_Protocol(struct Buff *BUFF, const char *head, int head_len, void *data, int data_size, int big_endian, int data_len);
int BUFF_find(struct Buff *BUFF, const char *ch, int len);
#endif
