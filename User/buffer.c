/*
 * @Date: 2023-10-16 11:45:55
 * @LastEditors: zjc & jar chou
 * @LastEditTime: 2023-12-26 16:21:39
 * @FilePath: \RVMDK（uv5）f:\桌面\F407_test\User\buffer.c
 * @Verson:V0.3
 * V0.2增加每个buffer可以指定大小
 * V0.3增加了BUFF_pop_by_Protocol函数，可以按照协议弹出数据
 * V0.4改成自己动态分配内存
 */
#include "buffer.h"
#include "string.h"
#include "stdio.h"

/**
 * @description: 初始化环形缓冲区
 * @param {Buff} *BUFF
 * @return {*}
 */
void Iinitial_BUFF(struct Buff *BUFF, u8 size_)
{
	BUFF->Data = (u8 *)BUFF_MALLOC(size_);
	BUFF->head_p = BUFF->Data;
	BUFF->end_p = BUFF->Data + size_;
	BUFF->write_p = BUFF->Data;
	BUFF->read_p = BUFF->Data;
	BUFF->size = size_;
	BUFF->buffer_used = 0;
}
/**
 * @description: 把数据写入缓冲区
 * @param {u8} *P
 * @param {Buff} *BUFF
 * @return {*}
 */
void Write_BUFF(u8 *P, struct Buff *BUFF)
{
	u8 ch = *P;
	if (BUFF->buffer_used == BUFF->size)
	{
		// even if buffer is full, we can still push, but we will lose the oldest data
		// so we need to move the read_p
		*(char *)(BUFF->write_p) = ch;
		BUFF->write_p = (char *)(BUFF->write_p) + 1;
		if ((BUFF->write_p) == BUFF->end_p)
		{
			BUFF->write_p = BUFF->head_p;
		}
		(BUFF->read_p) = (char *)(BUFF->read_p) + 1;
		if ((BUFF->read_p) == (BUFF->end_p))
		{
			BUFF->read_p = BUFF->head_p;
		}
		// !!! the buffer_used is not changed ,function must be return on here
		return;
	}
	*(char *)(BUFF->write_p) = ch;
	(BUFF->write_p) = (char *)(BUFF->write_p) + 1;
	BUFF->buffer_used++;
	if ((BUFF->write_p) == BUFF->end_p)
	{
		BUFF->write_p = BUFF->head_p;
	}
}

void Write_BUFF_P(u8 P, struct Buff *BUFF)
{
	// BUFF->write_p = BUFF->head_p + P;
	void *eventually_write_p = (char *)BUFF->head_p + P;

	if (BUFF->buffer_used == BUFF->size)
	{
		// even if buffer is full, we can still push, but we will lose the oldest data
		// so we need to move the read_p
		(BUFF->write_p) = (eventually_write_p);
		if ((BUFF->write_p) == BUFF->end_p)
		{
			BUFF->write_p = BUFF->head_p;
		}
		(BUFF->read_p) = (char *)eventually_write_p + 1;
		if ((BUFF->read_p) >= (BUFF->end_p))
		{
			BUFF->read_p = BUFF->head_p;
		}
		// !!! the buffer_used is not changed ,function must be return on here
		return;
	}
	else
	{
		if ((BUFF->write_p) < (eventually_write_p))
		{
			BUFF->buffer_used = (int)(BUFF->buffer_used + (char *)(eventually_write_p) - (char *)(BUFF->write_p));
		}
		else
		{
			BUFF->buffer_used += (int)((char *)(BUFF->end_p) - (char *)(BUFF->write_p) + (char *)(eventually_write_p) - (char *)(BUFF->head_p));
		}

		(BUFF->write_p) = (eventually_write_p);
		if ((BUFF->write_p) == BUFF->end_p)
		{
			BUFF->write_p = BUFF->head_p;
		}

		if (BUFF->buffer_used > BUFF->size)
		{
			BUFF->buffer_used = BUFF->size;
			BUFF->read_p = (char *)BUFF->write_p + 1;
			if ((BUFF->read_p) == (BUFF->end_p))
			{
				BUFF->read_p = BUFF->head_p;
			}
		}
	}
}

/**
 * @description: 从缓冲区读取数据
 * @param {Buff} *BUFF
 * @return {*}
 */
int Read_BUFF(struct Buff *BUFF)
{
	if ((BUFF->buffer_used) == 0)
	{
		return IS_EMPTY;
	}
	char ch = *(char *)(BUFF->read_p);
	(BUFF->read_p) = (char *)(BUFF->read_p) + 1;
	(BUFF->buffer_used)--;
	if (BUFF->read_p == BUFF->end_p)
	{
		BUFF->read_p = BUFF->head_p;
	}
	return ch;
}
/**
 * @description: 从缓冲区寻找指定字符串
 * @param {Buff} *BUFF
 * @param {char} *p
 * @return {*}
 */
u8 Find_Char(struct Buff *BUFF, char *p)
{
	u8 i, n, count = 0;
	n = BUFFER_SIZE;

	for (i = 0; i < strlen(p); i++)
	{
		while (n--)
		{
			if (((u8)Read_BUFF(BUFF) == (u8)p[i]))
			{
				count++;
				break;
			}
			else
				i = 0, count = 0;
			if (n == 0)
				return 0;
		}
	}
	if (count == strlen(p))
	{
		return 1;
	}
	else
		return 0;
}

/*
* @param ch: the char array to be found
* @param len: the length of the char array to be found		range: 0 ~ buffer_size

* @return: the position of the char array in the buffer
* */
int BUFF_find(struct Buff *BUFF, const char *ch, int len)
{
	if (BUFF->buffer_used == 0)
	{
		return IS_EMPTY;
	}
	else
	{
		const char *p = (const char *)BUFF->read_p;
		for (int i = 0; i < BUFF->buffer_used; i++)
		{
			if (*p == *ch)
			{
				int j = 1;
				const char *p1 = p + 1;
				const char *p2 = ch + 1;
				for (; j < len; j++)
				{
					if (*p1 != *p2)
					{
						break;
					}
					p1++;
					p2++;
					if (p1 == BUFF->end_p)
					{
						p1 = BUFF->head_p;
					}
				}
				if (j == len)
				{
					return i;
				}
			}
			p++;
			if (p == BUFF->end_p)
			{
				p = BUFF->head_p;
			}
		}
		return -1;
	}
//	return -1;
}

int BUFF_is_empty(struct Buff *BUFF)
{
	if ((BUFF->buffer_used) == 0)
	{
		return IS_EMPTY;
	}
	else
	{
		return 0;
	}
}

static int set_read_p(struct Buff *BUFF, int offset)
{
	if (offset > BUFF->buffer_used)
	{
		//		std::cout << "The offset is too large" << std::endl;
		return -1;
	}
	else
	{
		(BUFF->read_p) = (char *)(BUFF->read_p) + offset;
		if (BUFF->read_p >= BUFF->end_p)
		{
			(BUFF->read_p) = (char *)(BUFF->read_p) - BUFF->size;
		}
		BUFF->buffer_used -= offset;
		return 0;
	}
}

/**
 * @brief: pop the data by the protocol
 * @param head: the head of the protocol
 * @param head_len: the length of the head of the protocol
 * @param data: the data of the protocol
 * @param data_len: the length of the data of the protocol
 * @return: the length of the data of the protocol
 */
int BUFF_pop_by_Protocol(struct Buff *BUFF, const char *head, int head_len, void *data, int data_len)
{
	int head_position = BUFF_find(BUFF, head, head_len);
	if (head_position == -1)
	{
		//		std::cout << "Can not find the head in the buffer" << std::endl;
		return -1;
	}
	else
	{
		int data_position = head_position + head_len;
		if (data_position + data_len > BUFF->buffer_used)
		{
			//			std::cout << "Can not find the data in the buffer" << std::endl;
			return -1;
		}
		else
		{
			// first, move the read_p to the data_position
			set_read_p(BUFF, data_position);
			// second, pop the data
			for (int i = 0; i < data_len; i++)
			{
				((char *)data)[i] = Read_BUFF(BUFF);
			}
			return data_len;
		}
	}
}

/**
 * @brief: pop the data by the protocol
 * @param head: the head of the protocol
 * @param head_len: the length of the head of the protocol
 * @param data: the data of the protocol
 * @param data_len: the length of the data of the protocol
 * @param big_endian: the data is big_endian or not
 * @return: if succeed,return the length of the data of the protocol,
 *         if have not data, return -1,
 *        if the check_sum is wrong, return -2
 */

int BUFF_pop_with_check_by_Protocol(struct Buff *BUFF, const char *head, int head_len, void *data, int data_size, int big_endian, int data_len)
{
	int head_position = BUFF_find(BUFF, head, head_len);
	if (head_position == -1)
	{
		return -1;
	}
	else
	{
		if (data_size % 8 != 0)
		{
			return -1;
		}
		int data_position = head_position + head_len;
		if (data_position + data_len * data_size / 8 > BUFF->buffer_used)
		{
			// std::cout << "Can not find the data in the buffer" << std::endl;

			return -1;
		}
		else
		{
			// first, move the read_p to the data_position
			set_read_p(BUFF, data_position);
			// second, pop the data
			if (data_size == 8)
			{
				for (int i = 0; i < data_len; i++)
				{
					((char *)data)[i] = Read_BUFF(BUFF);
				}
				// return data_len;
			}
			else if (data_size == 16)
			{
				if (big_endian == 1)
				{
					for (int i = 0; i < data_len; i++)
					{
						int one = Read_BUFF(BUFF);
						int two = Read_BUFF(BUFF);
						((int16_t *)data)[i] = one + (two << 8);
					}
				}
				else
				{
					for (int i = 0; i < data_len; i++)
					{
						int one = Read_BUFF(BUFF);
						int two = Read_BUFF(BUFF);
						((int16_t *)data)[i] = (one << 8) + two;
					}
				}
				// return data_len;
			}
			else if (data_size == 32)
			{
				if (big_endian)
				{
					for (int i = 0; i < data_len; i++)
					{
						int32_t one = Read_BUFF(BUFF);
						int32_t two = Read_BUFF(BUFF);
						int32_t three = Read_BUFF(BUFF);
						int32_t four = Read_BUFF(BUFF);
						((int32_t *)data)[i] = one + (two << 8) + (three << 16) + (four << 24);
					}
				}
				else
				{
					for (int i = 0; i < data_len; i++)
					{
						int32_t one = Read_BUFF(BUFF);
						int32_t two = Read_BUFF(BUFF);
						int32_t three = Read_BUFF(BUFF);
						int32_t four = Read_BUFF(BUFF);
						((int32_t *)data)[i] = four + (three << 8) + (two << 16) + (one << 24);
					}
				}
				// return data_len;
			}
			// check the check_sum
			char check_sum = 0;
			for (int i = 0; i < data_len * data_size / 8; i++)
			{
				check_sum += ((char *)data)[i];
			}
			for (int i = 0; i < head_len; i++)
			{
				check_sum += head[i];
			}
			if (check_sum != Read_BUFF(BUFF))
			{
				return -2;
			}
			return data_len;
		}
	}
}
