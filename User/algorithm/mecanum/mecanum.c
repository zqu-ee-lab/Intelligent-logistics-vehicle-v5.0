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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "my_math.h"
#include "mecanum.h"

#ifndef RADIAN_COEF
    #define RADIAN_COEF 57.3f
#endif

#define MAX_WHEEL_RPM 1999.0f

#ifndef VAL_LIMIT
    #define VAL_LIMIT(val, min, max) \
      do                                 \
      {                                  \
        if ((val) <= (min))              \
        {                                \
          (val) = (min);                 \
        }                                \
        else if ((val) >= (max))         \
        {                                \
          (val) = (max);                 \
        }                                \
      } while (0)
#endif

/**
  * @brief mecanum glb_chassis velocity decomposition.F:forword; B:backword; L:left; R:right
  * @param input : ccx=+vx  ccy=+vy  ccw=+vw
  *        output: wheel_speed
  * @note  0=FL 1=BR 2=FR 3=BL
  */
void mecanum_calculate(mecanum_car_speed_t* car_speed, int32_t* wheel_speed)
{
    int32_t wheel_rpm[4] = {0};
    // calculate the ccr register value of the wheel
    wheel_rpm[0] = car_speed->y_speed;
    wheel_rpm[1] = car_speed->y_speed;
    wheel_rpm[2] = car_speed->y_speed;
    wheel_rpm[3] = car_speed->y_speed;

    wheel_rpm[0] += car_speed->x_speed;
    wheel_rpm[1] += car_speed->x_speed;
    wheel_rpm[2] -= car_speed->x_speed;
    wheel_rpm[3] -= car_speed->x_speed;

    wheel_rpm[0] -= car_speed->w_speed;
    wheel_rpm[1] += car_speed->w_speed;
    wheel_rpm[2] += car_speed->w_speed;
    wheel_rpm[3] -= car_speed->w_speed;

    //find max item
    int32_t max = 0;
    for (uint8_t i = 0; i < 4; i++)
    {
        if (abs(wheel_rpm[i]) > max)
        {
            max = abs(wheel_rpm[i]);
        }
    }

    //equal proportion
    if (max > MAX_WHEEL_RPM)
    {
        float rate = (float)MAX_WHEEL_RPM / max;
        for (uint8_t i = 0; i < 4; i++)
        {
            wheel_rpm[i] *= rate;
        }
    }
    memcpy(wheel_speed, wheel_rpm, 4 * sizeof(int32_t));
}
