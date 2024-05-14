/****************************************************************************
 *  Copyright (C) 2020 RoboMaster.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/

#ifndef __MECANUM_H__
#define __MECANUM_H__

#include <stdint.h>

typedef struct {
    int32_t y_speed;
    int32_t x_speed;
    int32_t w_speed;
} mecanum_car_speed_t;

typedef struct {
    int32_t L_H;
    int32_t R_H;
    int32_t L_B;
    int32_t R_B;
} mecanum_wheel_speed_t;

void mecanum_calculate(mecanum_car_speed_t* car_speed, int32_t* wheel_speed);

#endif // __MECANUM_H__
