/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef BO3DTOOLS_H
#define BO3DTOOLS_H

class BoVector3
{
  public:
    BoVector3()  { reset(); };
    BoVector3(float x, float y, float z)  { set(x, y, z); };
    ~BoVector3() {};

    inline void set(float x, float y, float z)  { data[0] = x;  data[1] = y;  data[2] = z; };
    inline void reset()  { data[0] = data[1] = data[2] = 0; };
    inline void addScaled(BoVector3 v, float s)  { data[0] += v.data[0] * s;  data[1] += v.data[1] * s;  data[2] += v.data[2] * s; };
    inline void setScaledSum(BoVector3 a, BoVector3 b,  float s)
        { data[0] = a.data[0] + b.data[0] * s;   data[1] = a.data[1] + b.data[1] * s;   data[2] = a.data[2] + b.data[2] * s; };

    inline void operator=(BoVector3 v)  { data[0] = v.data[0];  data[1] = v.data[1];  data[2] = v.data[2]; };
    inline void operator=(float* v)  { data[0] = v[0];  data[1] = v[1];  data[2] = v[2]; };
    inline float operator[](int i)  { return data[i]; };


    float data[3];
};

class BoVector4
{
  public:
    BoVector4()  { reset(); };
    BoVector4(float x, float y, float z, float w)  { set(x, y, z, w); };
    ~BoVector4() {};

    inline void set(float x, float y, float z, float w)  { data[0] = x;  data[1] = y;  data[2] = z; data[3] = w; };
    inline void reset()  {  data[0] = data[1] = data[2] = data[3] = 0; };
    inline void addScaled(BoVector4 v, float s)
        { data[0] += v.data[0] * s;  data[1] += v.data[1] * s;  data[2] += v.data[2] * s;  data[3] += v.data[3] * s; };
    inline void setScaledSum(BoVector4 a, BoVector4 b,  float s)
        { data[0] = a.data[0] + b.data[0] * s;   data[1] = a.data[1] + b.data[1] * s;   data[2] = a.data[2] + b.data[2] * s;   data[3] = a.data[3] + b.data[3] * s; };

    inline void operator=(BoVector4 v)  { data[0] = v.data[0];  data[1] = v.data[1];  data[2] = v.data[2];  data[3] = v.data[3]; };
    inline void operator=(float* v)  { data[0] = v[0];  data[1] = v[1];  data[2] = v[2];  data[3] = v[3]; };
    inline float operator[](int i)  { return data[i]; };


    float data[4];
};

#endif // BO3DTOOLS_H
