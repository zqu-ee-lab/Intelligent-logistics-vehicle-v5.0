/*
 * @Author: jar-chou 2722642511@qq.com
 * @Date: 2023-09-09 17:15:34
 * @LastEditors: jar-chou 2722642511@qq.com
 * @LastEditTime: 2023-09-09 22:36:49
 * @FilePath: \delivering_car\User\oled\oled_buffer.c
 * @Description: cn:这个文件用于操作屏幕缓冲区
 * 			 en:This file is used to operate the screen buffer
 */

#include "oled_buffer.h"
#include "string.h"
#include "OLED.h"

// 定义缓冲 屏幕缓冲区和临时缓冲区
uint8_t ScreenBuffer[8][128] = {0}; // 屏幕缓冲

#define BUFFERSIZE sizeof(ScreenBuffer)

///////////////////////////////////////////////////////////////////

// 功能:清除屏幕缓冲数据
void ClearScreenBuffer(unsigned char val)
{
	memset(ScreenBuffer, val, sizeof(ScreenBuffer));
}

///////////////////////////////////////////////////////////////////////////////////////////
// 读取选择的缓冲区的8位数据
unsigned char ReadByteBuffer(int x, int y)
{
	return ScreenBuffer[x][y];
}
// 写入读取选择的缓冲区8位数据
void WriteByteBuffer(int x, int y, unsigned char byte)
{
	ScreenBuffer[x][y] = byte;
}

void WriteMultByteBuffer(int x, int y, const uint8_t *bytearray, size_t num)
{
	memcpy(&ScreenBuffer[x][y], bytearray, num);
}

// 刷新屏幕显示
void UpdateScreenDisplay(void)
{
	OLED_FILL(ScreenBuffer);
}
