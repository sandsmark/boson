/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Rivo Laks (rivolaks@hot.ee)
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "math/bo3dtoolsbase.h"

#include <math.h>
#include <stdio.h>

// Degrees to radians conversion (AB: from mesa/src/macros.h)
#define DEG2RAD (M_PI/180.0)
// And radians to degrees conversion
#define RAD2DEG (180.0/M_PI)

bofixed Bo3dToolsBase::rotationToPoint(bofixed x, bofixed y)
{
  bofixed add = 0;
  bofixed arg = 0;
  if(x > 0)
  {
    if(y < 0)
    {
      add = 0;
      arg = x / -y;
    }
    else
    {
      add = 90;
      arg = y / x;
    }
  }
  else
  {
    if(y > 0)
    {
      add = 180;
      arg = -x / y;
    }
    else if(x < 0)
    {
      add = 270;
      arg = -y / -x;
    }
    else
    {
      return 0;
    }
  }

  return (atan(arg) * RAD2DEG) + add;
}

void Bo3dToolsBase::pointByRotation(bofixed* x, bofixed* y, const bofixed& angle, const bofixed& radius)
{
  // Some quick tests
  if(angle == 0)
  {
    *x = 0;
    *y = -radius;
    return;
  }
  else if(angle == 90)
  {
    *x = radius;
    *y = 0;
    return;
  }
  else if(angle == 180)
  {
    *x = 0;
    *y = radius;
    return;
  }
  else if(angle == 270)
  {
    *x = -radius;
    *y = 0;
    return;
  }
  // AB: WARNING: to avoid overflows in bofixed (max value around 16383) we use
  //     float here.
  float tmpx = tanf(angle / RAD2DEG);
  float tmpy = 1;
  float length = sqrtf(tmpx * tmpx + tmpy * tmpy);
  tmpx = tmpx / length * radius;
  tmpy = tmpy / length * radius;
  if(angle < 90)
  {
    *x = tmpx;
    *y = -tmpy;
  }
  else if(angle < 180)
  {
    *x = -tmpx;
    *y = tmpy;
  }
  else if(angle < 270)
  {
    *x = -tmpx;
    *y = tmpy;
  }
  else
  {
    *x = tmpx;
    *y = -tmpy;
  }
}

void Bo3dToolsBase::pointByRotation(float* _x, float* _y, const float _angle, const float _radius)
{
  // AB: we use a bofixed implementation by default.
  // the float one lacks some precision because of that, but I believe that
  // doesn't hurt.
  // if that precision is required, we need to implement this method twice, but
  // we must not use a float implementation, because of network stability
  // (floating point operations can have different results on differenct
  // computers)
  bofixed x, y;
  bofixed angle(_angle);
  bofixed radius(_radius);
  pointByRotation(&x, &y, angle, radius);
  *_x = x;
  *_y = y;
}


/*
 * vim:et sw=2
 */
