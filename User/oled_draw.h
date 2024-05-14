/*** 
 * @Author: jar-chou 2722642511@qq.com
 * @Date: 2023-09-09 17:15:48
 * @LastEditors: jar-chou 2722642511@qq.com
 * @LastEditTime: 2023-09-09 22:39:18
 * @FilePath: \delivering_car\User\oled\oled_draw.h
 * @Description: cn:这个文件用于在屏幕上画文字或图形
 */

#ifndef OLED_DRAW_H
#define OLED_DRAW_H
#include "stdint.h"

void DrawChar(int x, int y, unsigned char c);
void DrawString(int x, int y,char *str);
void DrawNum(unsigned char x,unsigned char y,unsigned int num,unsigned char len);
//void OLED_ShowCHinese(uint8_t x,uint8_t y,uint8_t *cn);
void DrawPicture(int x, int y, int high, int width, const unsigned char *p);

// this is the size of one char
// 5*8  :2
// 6*8  :1
// 8*16 :0
#define sizeofonechar 1


#endif

