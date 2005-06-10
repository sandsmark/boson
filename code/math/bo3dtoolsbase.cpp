/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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
  bofixed tmpx = tan(angle / RAD2DEG);
  bofixed tmpy = 1;
  bofixed length = sqrt(tmpx * tmpx + tmpy * tmpy);
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

bofixed Bo3dToolsBase::deg2rad(bofixed deg)
{
  return deg * DEG2RAD;
}

bofixed Bo3dToolsBase::rad2deg(bofixed rad)
{
  return rad * RAD2DEG;
}

float Bo3dToolsBase::distanceFromPlane(const float* plane, const BoVector3Float& pos)
{
  return pos.x() * plane[0] + pos.y() * plane[1] + pos.z() * plane[2] + plane[3];
}

float Bo3dToolsBase::sphereInFrustum(const float* viewFrustum, const BoVector3Float& pos, float radius)
{
  // FIXME: performance: we might unroll the loop and then make this function
  // inline. We call it pretty often!
  float distance;
  for(int p = 0; p < 6; p++)
  {
    distance = viewFrustum[p * 4 + 0] * pos[0] + viewFrustum[p * 4 + 1] * pos[1] +
        viewFrustum[p * 4 + 2] * pos[2] + viewFrustum[p * 4 + 3];
    if(distance <= -radius)
    {
      return 0;
    }
  }

  // we return distance from near plane + radius, which is how far the object is
  // away from the camera!
  return distance + radius;
}

bofixed Bo3dToolsBase::sphereInFrustum(const float* viewFrustum, const BoVector3Fixed& pos, bofixed radius)
{
  // FIXME: performance: we might unroll the loop and then make this function
  // inline. We call it pretty often!
  bofixed distance;
  for(int p = 0; p < 6; p++)
  {
    distance = viewFrustum[p * 4 + 0] * pos[0] + viewFrustum[p * 4 + 1] * pos[1] +
        viewFrustum[p * 4 + 2] * pos[2] + viewFrustum[p * 4 + 3];
    if(distance <= -radius)
    {
      return 0;
    }
  }

  // we return distance from near plane + radius, which is how far the object is
  // away from the camera!
  return distance + radius;
}

int Bo3dToolsBase::sphereCompleteInFrustum(const float* viewFrustum, const BoVector3Float& pos, float radius)
{
  float distance;
  int c = 0;
  for(int p = 0; p < 6; p++)
  {
    distance = viewFrustum[p * 4 + 0] * pos[0] + viewFrustum[p * 4 + 1] * pos[1] +
        viewFrustum[p * 4 + 2] * pos[2] + viewFrustum[p * 4 + 3];
    if(distance <= -radius)
    {
      return 0;
    }
    if(distance > radius)
    {
      c++;
    }
  }
  if(c == 6)
  {
    return 2;
  }
  return 1;
}

int Bo3dToolsBase::sphereCompleteInFrustum(const float* viewFrustum, const BoVector3Fixed& pos, bofixed radius)
{
  bofixed distance;
  int c = 0;
  for(int p = 0; p < 6; p++)
  {
    distance = viewFrustum[p * 4 + 0] * pos[0] + viewFrustum[p * 4 + 1] * pos[1] +
        viewFrustum[p * 4 + 2] * pos[2] + viewFrustum[p * 4 + 3];
    if(distance <= -radius)
    {
      return 0;
    }
    if(distance > radius)
    {
      c++;
    }
  }
  if(c == 6)
  {
    return 2;
  }
  return 1;
}

bool Bo3dToolsBase::boxInFrustum(const float* viewFrustum, const BoVector3Float& min, const BoVector3Float& max)
{
  for(int p = 0; p < 6; p++)
  {
    if(viewFrustum[p*4 + 0] * min.x() + viewFrustum[p*4 + 1] * min.y() +
        viewFrustum[p*4 + 2] * min.z() + viewFrustum[p*4 + 3] > 0)
    {
      continue;
    }
    if(viewFrustum[p*4 + 0] * max.x() + viewFrustum[p*4 + 1] * min.y() +
        viewFrustum[p*4 + 2] * min.z() + viewFrustum[p*4 + 3] > 0)
    {
      continue;
    }
    if(viewFrustum[p*4 + 0] * min.x() + viewFrustum[p*4 + 1] * max.y() +
        viewFrustum[p*4 + 2] * min.z() + viewFrustum[p*4 + 3] > 0)
    {
      continue;
    }
    if(viewFrustum[p*4 + 0] * max.x() + viewFrustum[p*4 + 1] * max.y() +
        viewFrustum[p*4 + 2] * min.z() + viewFrustum[p*4 + 3] > 0)
    {
      continue;
    }
    if(viewFrustum[p*4 + 0] * min.x() + viewFrustum[p*4 + 1] * min.y() +
        viewFrustum[p*4 + 2] * max.z() + viewFrustum[p*4 + 3] > 0)
    {
      continue;
    }
    if(viewFrustum[p*4 + 0] * max.x() + viewFrustum[p*4 + 1] * min.y() +
        viewFrustum[p*4 + 2] * max.z() + viewFrustum[p*4 + 3] > 0)
    {
      continue;
    }
    if(viewFrustum[p*4 + 0] * min.x() + viewFrustum[p*4 + 1] * max.y() +
        viewFrustum[p*4 + 2] * max.z() + viewFrustum[p*4 + 3] > 0)
    {
      continue;
    }
    if(viewFrustum[p*4 + 0] * max.x() + viewFrustum[p*4 + 1] * max.y() +
        viewFrustum[p*4 + 2] * max.z() + viewFrustum[p*4 + 3] > 0)
    {
      continue;
    }
    return false;
  }
  return true;
}



/*
 * vim:et sw=2
 */
