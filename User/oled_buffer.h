/*** 
 * @Author: jar-chou 2722642511@qq.com
 * @Date: 2023-09-09 17:15:34
 * @LastEditors: jar-chou 2722642511@qq.com
 * @LastEditTime: 2023-09-09 18:15:09
 * @FilePath: \delivering_car\User\oled\oled_buffer.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef OLED_BUFFER_H
#define OLED_BUFFER_H

#include <stdlib.h>
#include <stdint.h>

void ClearScreenBuffer(unsigned char val);
void UpdateScreenDisplay(void);

void WriteByteBuffer(int page,int x,unsigned char byte);
void WriteMultByteBuffer(int x,int y, const uint8_t* bytearray, size_t num);
unsigned char ReadByteBuffer(int page,int x);
#endif

