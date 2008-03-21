/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Rivo Laks (rivolaks@hot.ee)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef BOVECTOR_H
#define BOVECTOR_H

#include <math.h>

#include "math/bomath.h"

template<class T> class BoVector2;
template<class T> class BoVector3;
template<class T> class BoVector4;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoVector2<float> BoVector2Float;
typedef BoVector3<bofixed> BoVector3Fixed;
typedef BoVector3<float> BoVector3Float;
typedef BoVector4<bofixed> BoVector4Fixed;
typedef BoVector4<float> BoVector4Float;

/**
 * @short Vector with 2 components.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
template<class T> class BoVector2
{
  public:
    BoVector2()  { reset(); }
    BoVector2(T x, T y)  { set(x, y); }
    BoVector2(const T* data)  { set(data); }
    BoVector2(const BoVector2& v)  { set(v); }
    ~BoVector2()  {}

    BoVector2<float> toFloat() const
    {
      return BoVector2<float>(mData[0], mData[1]);
    }
    BoVector2<bofixed> toFixed() const
    {
      return BoVector2<bofixed>(mData[0], mData[1]);
    }

    /**
     * Make this vector a null vector.
     **/
    inline void reset()  { mData[0] = mData[1] = 0.0f; }

    /**
     * @return The first (x) coordinate of the vector.
     **/
    inline T x() const  { return mData[0]; }
    /**
     * @return The second (y) coordinate of the vector.
     **/
    inline T y() const  { return mData[1]; }

    /**
     * Assign the values @p x, @p y to the vector.
     **/
    inline void set(T x, T y)
    {
      mData[0] = x;  mData[1] = y;
    }
    /**
     * @overload
     **/
    inline void set(const BoVector2<T>& v)  { set(v.data()); }
    /**
     * @overload
     **/
    inline void set(const T* v)  { set(v[0], v[1]); }

    /**
     * Assign the x coordinate to the vector.
     **/
    inline void setX(T x)  { mData[0] = x; }
    /**
     * Assign the y coordinate to the vector.
     **/
    inline void setY(T y)  { mData[1] = y; }

    inline T dotProduct() const
    {
      return mData[0] * mData[0] + mData[1] * mData[1];
    }
    inline T dotProduct(const BoVector2<T>& v)
    {
      return mData[0] * v.mData[0] + mData[1] * v.mData[1];
    }
    T length() const
    {
      return sqrt(dotProduct());
    }
    inline void normalize()
    {
      T l = length();
      if (l != 0.0f) {
        scale(1.0f / l);
      }
    }
    inline void scale(T s)
    {
      mData[0] = mData[0] * s;  mData[1] = mData[1] * s;
    }

    inline T operator[](int i) const  { return mData[i]; }
    inline void operator=(const BoVector2& v)  { set(v); }
    inline bool operator==(const BoVector2& v) const
    {
      return isEqual(mData, v.data());
    }
    static bool isEqual(const T* v1, const T* v2, T diff = 0.001)
    {
      // avoid fabsf() as we don't include math.h
      T d1 = v1[0] - v2[0];
      T d2 = v1[1] - v2[1];
      if (d1 < 0)
      {
        d1 = -d1;
      }
      if (d2 < 0)
      {
        d2 = -d2;
      }
      if (d1 > diff)
      {
        return false;
      }
      if (d2 > diff)
      {
        return false;
      }
      return true;
    }

    inline BoVector2 operator+(const BoVector2& v) const
    {
      return BoVector2(mData[0] + v[0], mData[1] + v[1]);
    }
    inline BoVector2 operator-(const BoVector2& v) const
    {
      return BoVector2(mData[0] - v[0], mData[1] - v[1]);
    }
    inline BoVector2 operator*(T f) const
    {
      return BoVector2(mData[0] * f, mData[1] * f);
    }
    inline BoVector2 operator/(T f) const
    {
      return BoVector2(mData[0] / f, mData[1] / f);
    }
    inline void operator+=(const BoVector2& v)
    {
      mData[0] += v[0]; mData[1] += v[1];
    }
    inline void operator-=(const BoVector2& v)
    {
      mData[0] -= v[0]; mData[1] -= v[1];
    }

    inline const T* data() const { return mData; }


  private:
    T mData[2];
};

/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
template<class T> class BoVector3
{
  public:
    BoVector3()  { reset(); }
    BoVector3(T x, T y, T z)  { set(x, y, z); }
    BoVector3(const T* data) { set(data[0], data[1], data[2]); }
    BoVector3(const BoVector3<T>& v) { set(v[0], v[1], v[2]); }
    ~BoVector3() {}

    BoVector3<float> toFloat() const
    {
      return BoVector3<float>(mData[0], mData[1], mData[2]);
    }
    BoVector3<bofixed> toFixed() const
    {
      return BoVector3<bofixed>(mData[0], mData[1], mData[2]);
    }

    /**
     * Make this vector a null vector.
     **/
    inline void reset()  { mData[0] = mData[1] = mData[2] = 0; }

    /**
     * @return The first (x) coordinate of the vector.
     **/
    inline T x() const { return mData[0]; }
    /**
     * @return The second (y) coordinate of the vector.
     **/
    inline T y() const { return mData[1]; }
    /**
     * @return The third (z) coordinate of the vector.
     **/
    inline T z() const { return mData[2]; }

    /**
     * Assign the values @p x, @p y, @p z to the vector.
     **/
    inline void set(T x, T y, T z)
    {
      mData[0] = x;  mData[1] = y;  mData[2] = z;
    }
    /**
     * @overload
     **/
    inline void set(const BoVector3<T>& v) { set(v.data()); }
    /**
     * @overload
     **/
    inline void set(const T* v) { set(v[0], v[1], v[2]); }

    /**
     * Assign the x coordinate to the vector.
     **/
    inline void setX(T x) { mData[0] = x; }
    /**
     * Assign the y coordinate to the vector.
     **/
    inline void setY(T y) { mData[1] = y; }
    /**
     * Assign the z coordinate to the vector.
     **/
    inline void setZ(T z) { mData[2] = z; }

    /**
     * Scale @p v by s and then add it to this vector.
     **/
    inline void addScaled(const BoVector3<T>& v, T s)
    {
      mData[0] += v.mData[0] * s;  mData[1] += v.mData[1] * s;  mData[2] += v.mData[2] * s;
    }

    inline void setBlended(const BoVector3<T>& a, T af, const BoVector3<T>& b, T bf)
    {
      mData[0] = a.mData[0] * af + b.mData[0] * bf;
      mData[1] = a.mData[1] * af + b.mData[1] * bf;
      mData[2] = a.mData[2] * af + b.mData[2] * bf;
    }

    /**
     * Add @p v to this vector.
     **/
    inline void add(const BoVector3<T>& v)
    {
      mData[0] += v.mData[0]; mData[1] += v.mData[1]; mData[2] += v.mData[2];
    }

    /**
     * Normalize this vector.
     *
     * Normalizing a vector means to make it a so-called "unit-vector", that is
     * a vector with @ref length 1.
     *
     * Practically this means dividing all elements in the vector by the @ref
     * length of the vector.
     **/
    inline void normalize()
    {
      T l = length();
      if (l != 0.0f) {
        scale(1.0f / l);
      }
    }

    /**
     * Scale the vector by @p s. This is just scalar multiplication, i.e. all
     * elements/coordinates of the vector are multiplied by @p s.
     **/
    inline void scale(T s)
    {
      mData[0] = mData[0] * s;  mData[1] = mData[1] * s;  mData[2] = mData[2] * s;
    }


    //AB: this calls sqrt() and therefore is slow!
    /**
     * @return The length (aka magnitude) of the vector.
     *
     * The length of a vector v is defined as sqrt(v[0]^2 + v[1]^2 + v[2]^2) (in
     * case of 3d).
     *
     * Notice that this function actually uses sqrt(), so it is slow in
     * situations where you use it often!
     **/
    T length() const
    {
      return sqrt(dotProduct());
    }

    /**
     * @return The dot product of the two vectors @p v and @p w.
     *
     * The dot product v*w is equal to |v|*|w|*cos(alpha), where alpha is the
     * angle between both vectors and |v| is the length of v.
     **/
    static inline T dotProduct(const BoVector3<T>& v, const BoVector3<T>& w)
    {
      return v[0] * w[0] + v[1] * w[1] + v[2] * w[2];
    }

    inline T dotProduct(const BoVector3<T>& v)
    {
      return dotProduct(*this, v);
    }

    /**
     * @return The dot product of this vector with itself, i.e. (v * v).
     *
     * The dot product is also equivalent to the square of the @ref length. This
     * can be important sometimes, as it might be sufficient to use the square
     * of the length (which is calculated very fast) instead of the actual
     * length (which needs a call to sqrt()).
     **/
    inline T dotProduct() const
    {
      return dotProduct(*this, *this);
    }

    /**
     * The cross product of v and w is a vector that is perpendicular to the
     * surface that is perpendicular to the surface made up by v and w.
     * @return The cross product of v and w.
     **/
    static BoVector3<T> crossProduct(const BoVector3<T>& v, const BoVector3<T>& w)
    {
      BoVector3<T> r;
      r.setX((v.y() * w.z()) - (v.z() * w.y()));
      r.setY((v.z() * w.x()) - (v.x() * w.z()));
      r.setZ((v.x() * w.y()) - (v.y() * w.x()));
      return r;
    }

    inline BoVector3<T> crossProduct(const BoVector3<T>& v) const
    {
      return crossProduct(*this, v);
    }

    /**
     * @return A pointer to the internal array.
     **/
    inline const T* data() const { return mData; }

    /**
     * See @ref set
     **/
    inline void operator=(const BoVector3<T>& v)
    {
      set(v);
    }

    /**
     * @overload
     **/
    inline void operator=(const T* v)
    {
      set(v);
    }

    /**
     * See @ref add
     **/
    inline void operator+=(const BoVector3<T>& v)
    {
      add(v);
    }

    /**
     * @return The component / coordinate at @p i of this vector
     **/
    inline T operator[](int i) const
    {
      return mData[i];
    }

    /**
     * @return A copy of this vector with @p v added.
     **/
    inline BoVector3<T> operator+(const BoVector3<T>& v) const
    {
      return BoVector3(mData[0] + v.mData[0], mData[1] + v.mData[1], mData[2] + v.mData[2]);
    }

    /**
     * @return A copy of this vector, @p v subtracted.
     **/
    inline BoVector3<T> operator-(const BoVector3<T>& v) const
    {
      return BoVector3(mData[0] - v.mData[0], mData[1] - v.mData[1], mData[2] - v.mData[2]);
    }

    /**
     * @return A copy of this vector, scaled by @p f.
     **/
    inline BoVector3<T> operator*(T f) const
    {
      return BoVector3(mData[0] * f, mData[1] * f, mData[2] * f);
    }

    /**
     * @return See @ref crossProduct. Cross product of this vector with @p v
     **/
    inline BoVector3<T> operator*(const BoVector3<T>& v) const
    {
      return crossProduct(*this, v);
    }

    /**
     * @return A copy of this vector, scaled by 1 / @p f
     **/
    inline BoVector3<T> operator/(T f) const
    {
      return BoVector3(mData[0] / f, mData[1] / f, mData[2] / f);
    }

    /**
     * @return A copy of this vector scaled by -1
     **/
    inline BoVector3<T> operator-() const
    {
      return BoVector3(-mData[0], -mData[1], -mData[2]);
    }

    /**
     * @return Whether all components of this vector are zeros
     **/
    inline bool isNull() const
    {
      return ((mData[0] == 0) && (mData[1] == 0) && (mData[2] == 0));
    }

    /**
     * @param v1 An array of 3 vectors (i.e. one triangle)
     * @param v2 An array of 3 vectors (i.e. one triangle)
     * @return TRUE if the two triangles are adjacent, i.e. if they share at
     * least two points. also returns TRUE if the triangles are equal!
     **/
    static bool isAdjacent(const BoVector3<T>* v1, const BoVector3<T>* v2)
    {
      if (!v1 || !v2)
      {
        return false;
      }
      int equal = 0;
      for (int i = 0; i < 3; i++)
      {
        if (v1[i].isEqual(v2[0]) || v1[i].isEqual(v2[1]) || v1[i].isEqual(v2[2]))
        {
          equal++;
        }
      }

      // v1 is adjacent to v2 if at least 2 points are equal.
      // equal vectors (i.e. all points are equal) are possible, too.
      return (equal >= 2);
    }

    /**
     * @param point The point to search for
     * @param array An array of size 3 (one face/triangle)
     * @return the index of the point @p point in @p array, or -1 if @p point is
     * not in @p array.
     **/
    static int findPoint(const BoVector3<T>& point, const BoVector3<T>* array)
    {
      for (int i = 0; i < 3; i++)
      {
        if (array[i].isEqual(point))
        {
          return i;
        }
      }
      return -1;
    }

    /**
     * Convenience method for BoVector3::findPoint(*this, array);
     **/
    int findPoint(const BoVector3<T>* array) const
    {
      return findPoint(*this, array);
    }

    /**
     * Convenience method for BoVector3::isAdjacent(this, v);
     **/
    bool isAdjacent(const BoVector3<T>* v) const
    {
      return isAdjacent(this, v);
    }

    /**
     * @return TRUE when the coordinates of this vector equal x,y and z,
     * otherwise FALSE.
     * @param diff The maximal difference that the elements may have to be
     * treated as "equal". note that 0.0 is a bad idea, since rounding errors
     * are _very_ probable!
     **/
    inline bool isEqual(T x, T y, T z, T diff = 0.001) const
    {
      T v2[3];
      v2[0] = x;
      v2[1] = y;
      v2[2] = z;
      return BoVector3<T>::isEqual(mData, v2, diff);
      return true;
    }

    /**
     * @overload
     *
     * Same as above, except that it takes an array of 3 floats, such as e.g.
     * Lib3dsVector.
     **/
    inline bool isEqual(const T* v, T diff = 0.001) const { return isEqual(v[0], v[1], v[2], diff); }

    /**
     * @overload
     **/
    inline bool isEqual(const BoVector3<T>& v, T diff = 0.001) const
    {
      return isEqual(v.data(), diff);
    }

    inline bool operator==(const BoVector3<T>& v) const { return isEqual(v); }
    /**
     * @overload
     *
     * Same as above, except that it takes 2 separate float arrays. You can use
     * this static method without a BoVector3 instance - useful for comparing
     * Lib3dsVectors.
     **/
    static bool isEqual(const T* v1, const T* v2, T diff = 0.001)
    {
      // avoid fabsf() as we don't include math.h
      T d1 = v1[0] - v2[0];
      T d2 = v1[1] - v2[1];
      T d3 = v1[2] - v2[2];
      if (d1 < 0.0f)
      {
        d1 = -d1;
      }
      if (d2 < 0.0f)
      {
        d2 = -d2;
      }
      if (d3 < 0.0f)
      {
        d3 = -d3;
      }
      if (d1 > diff)
      {
        return false;
      }
      if (d2 > diff)
      {
        return false;
      }
      if (d3 > diff)
      {
        return false;
      }
      return true;
    }

    // Conversion from one coordinate system to another.
    inline void canvasToWorld()
    {
      mData[1] = -mData[1];
    }


  private:
    friend class BoMatrix;

    T mData[3];
};

/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
template<class T> class BoVector4
{
  public:

    BoVector4()  { reset(); };
    BoVector4(T x, T y, T z, T w)  { set(x, y, z, w); };
    BoVector4(const T* data) { set(data[0], data[1], data[2], data[3]); }
    BoVector4(const BoVector4<T>& v) { set(v[0], v[1], v[2], v[3]); }
    ~BoVector4() {};

    BoVector4<float> toFloat() const
    {
      return BoVector4<float>(mData[0], mData[1], mData[2], mData[3]);
    }
    BoVector4<bofixed> toFixed() const
    {
      return BoVector4<bofixed>(mData[0], mData[1], mData[2], mData[3]);
    }

    /**
     * Make this vector a null vector.
     **/
    inline void reset()
    {
      mData[0] = mData[1] = mData[2] = mData[3] = 0;
    }

    /**
     * @return The first (x) coordinate of the vector.
     **/
    inline T x() const { return mData[0]; }
    /**
     * @return The second (y) coordinate of the vector.
     **/
    inline T y() const { return mData[1]; }
    /**
     * @return The third (z) coordinate of the vector.
     **/
    inline T z() const { return mData[2]; }
    /**
     * @return The fourth (w) coordinate of the vector.
     **/
    inline T w() const { return mData[3]; }


    /**
     * Assign the values @p x, @p y, @p z, @p w to the vector.
     **/
    inline void set(T x, T y, T z, T w)
    {
      mData[0] = x;  mData[1] = y;  mData[2] = z; mData[3] = w;
    }
    /**
     * @overload
     **/
    inline void set(const T* v)
    {
      set(v[0], v[1], v[2], v[3]);
    }
    /**
     * @overload
     **/
    inline void set(const BoVector4<T>& v)
    {
      set(v.data());
    }

    /**
     * Assign the x coordinate to the vector.
     **/
    inline void setX(T x) { mData[0] = x; }
    /**
     * Assign the y coordinate to the vector.
     **/
    inline void setY(T y) { mData[1] = y; }
    /**
     * Assign the z coordinate to the vector.
     **/
    inline void setZ(T z) { mData[2] = z; }
    /**
     * Assign the w coordinate to the vector.
     **/
    inline void setW(T w) { mData[3] = w; }

    /**
     * Scale @p v by s and then add it to this vector.
     **/
    inline void addScaled(const BoVector4<T>& v, T s)
    {
      mData[0] += v.mData[0] * s;
      mData[1] += v.mData[1] * s;
      mData[2] += v.mData[2] * s;
      mData[3] += v.mData[3] * s;
    }

    inline void setBlended(const BoVector4<T>& a, T af, const BoVector4<T>& b, T bf)
    {
      mData[0] = a.mData[0] * af + b.mData[0] * bf;
      mData[1] = a.mData[1] * af + b.mData[1] * bf;
      mData[2] = a.mData[2] * af + b.mData[2] * bf;
      mData[3] = a.mData[3] * af + b.mData[3] * bf;
    }

    /**
     * Add @p v to this vector.
     **/
    inline void add(const BoVector4<T>& v)
    {
      mData[0] += v.mData[0];
      mData[1] += v.mData[1];
      mData[2] += v.mData[2];
      mData[3] += v.mData[3];
    }

    /**
     * Scale the vector by @p s. This is just scalar multiplication, i.e. all
     * elements/coordinates of the vector are multiplied by @p s.
     **/
    inline void scale(T s)
    {
      mData[0] *= s; mData[1] *= s; mData[2] *= s; mData[3] *= s;
    }

    inline void scale(const BoVector4<T>& v)
    {
      mData[0] *= v.mData[0]; mData[1] *= v.mData[1]; mData[2] *= v.mData[2]; mData[3] *= v.mData[3];
    }

    /**
     * @return A pointer to the internal array.
     **/
    inline const T * data() const { return mData; }

    /**
     * See @ref set
     **/
    inline void operator=(const BoVector4<T>& v)
    {
      set(v);
    }
    /**
     * See @ref set
     **/
    inline void operator=(const T* v)
    {
      set(v);
    }
    inline BoVector4<T> operator/(const BoVector4<T>& v) const
    {
      return BoVector4<T>(mData[0] / v.mData[0], mData[1] / v.mData[1], mData[2] / v.mData[2], mData[3] / v.mData[3]);
    }
    inline BoVector4<T> operator+(const BoVector4<T>& v) const
    {
      return BoVector4<T>(mData[0] + v.mData[0], mData[1] + v.mData[1], mData[2] + v.mData[2], mData[3] + v.mData[3]);
    }
    inline BoVector4<T> operator-(const BoVector4<T>& v) const
    {
      return BoVector4<T>(mData[0] - v.mData[0], mData[1] - v.mData[1], mData[2] - v.mData[2], mData[3] - v.mData[3]);
    }

    inline BoVector4<T> operator/(float f) const
    {
      return BoVector4<T>(mData[0] / f, mData[1] / f, mData[2] / f, mData[3] / f);
    }
    inline BoVector4<T> operator*(float f) const
    {
      return BoVector4<T>(mData[0] * f, mData[1] * f, mData[2] * f, mData[3] * f);
    }

    /**
     * @return The component / coordinate at @p i of this vector
     **/
    inline T operator[](int i) const { return mData[i]; }

    inline bool operator==(const BoVector4<T>& v) const { return isEqual(v); }

    inline bool isEqual(const BoVector4<T>& v, T diff = 0.001) const
    {
      return isEqual(v.data(), mData, diff);
    }
    static bool isEqual(const T* v1, const T* v2, T diff = 0.001)
    {
      // avoid fabsf() as we don't include math.h
      T d1 = v1[0] - v2[0];
      T d2 = v1[1] - v2[1];
      T d3 = v1[2] - v2[2];
      T d4 = v1[3] - v2[3];
      if (d1 < 0.0f)
      {
        d1 = -d1;
      }
      if (d2 < 0.0f)
      {
        d2 = -d2;
      }
      if (d3 < 0.0f)
      {
        d3 = -d3;
      }
      if (d4 < 0.0f)
      {
        d4 = -d4;
      }
      if (d1 > diff)
      {
        return false;
      }
      if (d2 > diff)
      {
        return false;
      }
      if (d3 > diff)
      {
        return false;
      }
      if (d4 > diff)
      {
        return false;
      }
      return true;
    }


  private:
    friend class BoMatrix;
    T mData[4];
};

#endif
/*
 * vim:et sw=2
 */
