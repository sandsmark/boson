/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 Rivo Laks (rivolaks@hot.ee)
    Copyright (C) 2002-2003 Andreas Beckermann (b_mann@gmx.de)

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

#ifndef BO3DTOOLS_H
#define BO3DTOOLS_H

#include <math.h>

//#include "defines.h"
//#include "bomath.h"
#define bofixed float

class QString;
class KConfig;
class QDataStream;
class QPoint;
class QDomElement;
template<class T> class BoRect;
template<class T> class BoVector2;
template<class T> class BoVector3;
template<class T> class BoVector4;
typedef BoRect<bofixed> BoRectFixed;
typedef BoRect<float> BoRectFloat;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoVector2<float> BoVector2Float;
typedef BoVector3<bofixed> BoVector3Fixed;
typedef BoVector3<float> BoVector3Float;
typedef BoVector4<bofixed> BoVector4Fixed;
typedef BoVector4<float> BoVector4Float;

QDataStream& operator<<(QDataStream& s, const BoVector2Float& v);
QDataStream& operator>>(QDataStream& s, BoVector2Float& v);
QDataStream& operator<<(QDataStream& s, const BoVector2Fixed& v);
QDataStream& operator>>(QDataStream& s, BoVector2Fixed& v);

QDataStream& operator<<(QDataStream& s, const BoVector3Float& v);
QDataStream& operator<<(QDataStream& s, const BoVector3Fixed& v);
QDataStream& operator>>(QDataStream& s, BoVector3Float& v);
QDataStream& operator>>(QDataStream& s, BoVector3Fixed& v);

QDataStream& operator<<(QDataStream& s, const BoVector4Float& v);
QDataStream& operator<<(QDataStream& s, const BoVector4Fixed& v);
QDataStream& operator>>(QDataStream& s, BoVector4Float& v);
QDataStream& operator>>(QDataStream& s, BoVector4Fixed& v);


bool saveVector2AsXML(const BoVector2Float&, QDomElement& root, const QString& name);
bool saveVector2AsXML(const BoVector2Fixed&, QDomElement& root, const QString& name);
bool loadVector2FromXML(BoVector2Float*, const QDomElement& root, const QString& name);
bool loadVector2FromXML(BoVector2Fixed*, const QDomElement& root, const QString& name);

bool saveVector3AsXML(const BoVector3Float&, QDomElement& root, const QString& name);
bool saveVector3AsXML(const BoVector3Fixed&, QDomElement& root, const QString& name);
bool loadVector3FromXML(BoVector3Float*, const QDomElement& root, const QString& name);
bool loadVector3FromXML(BoVector3Fixed*, const QDomElement& root, const QString& name);

bool saveVector4AsXML(const BoVector4Float&, QDomElement& root, const QString& name);
bool saveVector4AsXML(const BoVector4Fixed&, QDomElement& root, const QString& name);
bool loadVector4FromXML(BoVector4Float*, const QDomElement& root, const QString& name);
bool loadVector4FromXML(BoVector4Fixed*, const QDomElement& root, const QString& name);

// convenience function to convert a BoVector into a string for debugging
QString debugStringVector(const BoVector3Float&, int prec);
QString debugStringVector(const BoVector3Fixed&, int prec);
QString debugStringVector(const BoVector4Float&, int prec);
QString debugStringVector(const BoVector4Fixed&, int prec);

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
    T length() const
    {
      return sqrt(dotProduct());
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
      mData[0] += v[0];
    }
    inline void operator-=(const BoVector2& v)
    {
      mData[0] -= v[0];
    }

    inline const T* data() const { return mData; }


    bool saveAsXML(QDomElement& root, const QString& name) const
    {
      return saveVector2AsXML(*this, root, name);
    }
    bool loadFromXML(const QDomElement& root, const QString& name)
    {
      return loadVector2FromXML(this, root, name);
    }

  private:
    T mData[2];
};


/**
 * @short Rectangle class
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
template<class T> class BoRect
{
  public:
    BoRect(const BoVector2<T>& topLeft, const BoVector2<T>& bottomRight)
    {
      set(topLeft, bottomRight);
    }
    BoRect(const T left, const T top, const T right, const T bottom)
    {
      set(left, top, right, bottom);
    }

    inline void set(const BoVector2<T>& topLeft, const BoVector2<T>& bottomRight)
    {
      mTopLeft = topLeft;
      mBottomRight = bottomRight;
    }
    inline void set(const T left, const T top, const T right, const T bottom)
    {
      mTopLeft = BoVector2<T>(left, top);
      mBottomRight = BoVector2<T>(right, bottom);
    }

    inline T left() const  { return mTopLeft.x(); }
    inline T top() const  { return mTopLeft.y(); }
    inline T right() const  { return mBottomRight.x(); }
    inline T bottom() const  { return mBottomRight.y(); }

    inline const BoVector2<T>& topLeft() const { return mTopLeft; }
    inline const BoVector2<T>& bottomRight() const { return mBottomRight; }

    inline T x() const  { return mTopLeft.x(); }
    inline T y() const  { return mTopLeft.y(); }

    inline T width() const  { return mBottomRight.x() - mTopLeft.x(); }
    inline T height() const  { return mBottomRight.y() - mTopLeft.y(); }

    inline BoVector2<T>center() const
    {
      return BoVector2<T>((left() + right()) / 2, (top() + bottom()) / 2);
    }


  private:
    BoVector2<T> mTopLeft;
    BoVector2<T> mBottomRight;
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

    bool saveAsXML(QDomElement& root, const QString& name) const
    {
      return saveVector3AsXML(*this, root, name);
    }
    bool loadFromXML(const QDomElement& root, const QString& name)
    {
      return loadVector3FromXML(this, root, name);
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

    /**
     * @return A string that contains the vector @p v. This string can be used
     * for debugging.
     **/
    static QString debugString(const BoVector4<T>& v, int prec = 6);

    /**
     * Dump this vector onto the console. See also @ref debugString
     **/
    static void debugVector(const BoVector4<T>& v, int prec = 6);

    bool saveAsXML(QDomElement& root, const QString& name) const
    {
      return saveVector4AsXML(*this, root, name);
    }
    bool loadFromXML(const QDomElement& root, const QString& name)
    {
      return loadVector4FromXML(this, root, name);
    }


  private:
    friend class BoMatrix;
    T mData[4];
};

/**
 * an OpenGL 4x4 matrix. note that we use (just like mesa) column major order to
 * store the matrix elements!
 *
 * This means that a matrix
 * <pre>
 * A11 A12 A13 A14
 * A21 A22 A23 A24
 * A31 A32 A33 A34
 * A41 A42 A43 A44
 * </pre>
 * Will be stored in memory like this:
 * <pre>
 * A11 A21 A31 A41 A12 A22 A32 A42 A13 A23 A33 A43 A41 A42 A43 A44
 * </pre>
 *
 * @short A 4x4 matrix as used by boson and OpenGL
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoMatrix
{
  public:
    /**
     * Construct an (identitiy) matrix. See @ref loadIdentity.
     **/
    BoMatrix()
    {
      loadIdentity();
    }

    /**
     * Construct a matrix that is a copy of @p matrix. See @ref loadMatrix
     **/
    BoMatrix(const float* matrix)
    {
      loadMatrix(matrix);
    }

    /**
     * Construct a matrix that is a copy of @p matrix. See @ref loadMatrix
     **/
    BoMatrix(const BoMatrix& matrix)
    {
      loadMatrix(matrix);
    }

    /**
     * Load the identity matrix (the "1" for matrices - M * identity = M)
     **/
    void loadIdentity()
    {
      int i;
      for (i = 0; i < 16; i++) {
        mData[i] = 0.0;
      }
      mData[0] = mData[5] = mData[10] = mData[15] = 1.0;
    }


    /**
     * @overload
     **/
    void loadMatrix(const float* m);

    /**
     * @overload
     **/
    void loadMatrix(const BoMatrix& m) { loadMatrix(m.data()); }

    /**
     * @overload
     * The three vectors get interpreted as <em>row</em> vectors
     **/
    void loadMatrix(const BoVector3Float& row1, const BoVector3Float& row2, const BoVector3Float& row3);

    /**
     * Change the element at @p row, @p column to @p value. See also @ref
     * element and @ref indexAt
     **/
    void setElement(int row, int column, float value)
    {
      mData[indexAt(row, column)] = value;
    }

    /**
     * See also @ref indexAt
     * @param row 0..3 -> specifies the row (aka line) of the matrix
     * @param column 0..3 -> specifies the column of the matrix (what a
     * surprise)
     * @return The element of the matrix at the specified position
     **/
    inline float element(int row, int column) const
    {
      return mData[indexAt(row, column)];
    }

    /**
     * @return A pointer to the internal array. See also @ref element, @ref indexAt,
     * @ref setElement
     **/
    const float* data() const { return mData; }

    /**
     * @return TRUE if <em>all</em> elements of this matrix are 0. Otherwise
     * FALSE.
     **/
    bool isNull() const
    {
      for (int i = 0; i < 16; i++) {
        if (mData[i] != 0.0) {
          return false;
        }
      }
      return true;
    }
    /**
     * @return TRUE if this is the identity matrix, otherwise FALSE.
     **/
    bool isIdentity() const
    {
      for (int i = 0; i < 16; i++) {
        if (mData[i] != 0.0) {
          if (mData[i] != 1.0 || i % 5 != 0) {
            return false;
          }
        }
      }
      return true;
    }
    bool hasNaN() const
    {
      for (int i = 0; i < 16; i++) {
        if (isnan(mData[i])) {
          return true;
        }
      }
      return false;
    }

    /**
     * Translate (i.e. move) the matrix by x,y,z.
     **/
    void translate(float x, float y, float z);

    /**
     * @overload
     **/
    inline void translate(const BoVector3Float& v)
    {
      translate(v.x(), v.y(), v.z());
    }

    /**
     * Scale the matrix by x,y,z.
     *
     * Note that if one of x,y,z is 0.0 the result will probably an invalid
     * matrix. Don't do that unless you really know what you're doing.
     **/
    void scale(float x, float y, float z);

    /**
     * Multiply the matrix by @p mat.
     * @param mat An array as returned by @ref data and as used by OpenGL.
     **/
    void multiply(const float* mat);

    /**
     * @overload
     **/
    inline void multiply(const BoMatrix* mat)
    {
      multiply(mat->data());
    }

    /**
     * Rotate around a specified axis. @p angle specifies the angle, i.e. how
     * much it is rotated and x,y,z specify the axis.
     *
     * See also the OpenGL glRotate() which uses the same syntax.
     **/
    void rotate(float angle, float x, float y, float z);

    /**
     * Generate a matrix from the three vectors, just as gluLookAt() does. Note
     * that the origin will remain at (0,0,0), it is not translated.
     **/
    void setLookAtRotation(const BoVector3Float& cameraPos, const BoVector3Float& lookAt, const BoVector3Float& up);

    /**
     * Transform the vector @p input according to this matrix and put the result
     * into @p v.
     *
     * This calculates simply does v = M * input, where M is this matrix.
     **/
    void transform(BoVector3Float* v, const BoVector3Float* input) const;

    /**
     * @overload
     **/
    void transform(BoVector4Float* v, const BoVector4Float* input) const;

    /**
     * Invert this matrix and place the result into @p inverse.
     * @return TRUE on success or FALSE if this is a not invertible matrix.
     **/
    bool invert(BoMatrix* inverse) const;

    /**
     * @return TRUE when.. well, when this matrix is equal to @p matrix
     * @param diff The maximal difference that the elements may have to be
     * treated as "equal". note that 0.0 is a bad idea, since rounding errors
     * are _very_ probable!
     **/
    bool isEqual(const BoMatrix& matrix, float diff = 0.001) const;

    /**
     * @return The element at index @p i in the internal array. See @ref
     * indexAt.
     **/
    inline float operator[](int i) const { return mData[i]; }

    /**
     * Convert the rotation matrix to 3 angles. If you combine these angles in
     * the following way (the order is important!)
     * <pre>
     * glRotatef(*angleX, 1.0, 0.0, 0.0);
     * glRotatef(*angleY, 0.0, 1.0, 0.0);
     * glRotatef(*angleZ, 0.0, 0.0, 1.0);
     * </pre>
     * You will get this rotation matrix.
     *
     * Note that the results of this function are totally undefined if this is
     * not a rotation matrix (i.e. a mtrix that was rotated only)
     *
     * These angles are often referred to as euler angles.
     **/
    void toRotation(float* angleX, float* angleY, float* angleZ);

    /**
     * Convert a matrix to @p lookAt and @p up, as it can get used by gluLookAt.
     *
     * Keep in mind that infinite different combinations of these vectors will
     * lead to exactly the same matrix, so you will not receive the same vectors
     * as you have initially specified. But they construct the same matrix.
     *
     * You can do this with any valid matrix (i.e. any matrix that was created
     * by using @ref rotate, @ref transform, @ref scale). You are not limited to
     * rotation matrices. But you will have to provide the cameraPos vector - it
     * may be possible without (haven't tried), but this function was developed
     * to convert euler angles to gluLookAt() and then you'll have it anyway.
     *
     * This function is pretty complex and not very optimized, as I have had to
     * develop the algorithm on my own. A good mathematician may develop a
     * faster way but that doesn't matter for us.
     **/
    void toGluLookAt(BoVector3Float* lookAt, BoVector3Float* up, const BoVector3Float& cameraPos) const;

    /**
     * @return The index of the element @p row, @p column of the matrix in the
     * internal array. The array can be organized in two different ways, which
     * both are used out there in the world. We are preferring the organization
     * that is used by OpenGL/mesa
     **/
    static inline int indexAt(int row, int column) { return (column << 2) + row; }

    /**
     * Dump this matrix to the console as debug output.
     **/
    void debugMatrix()
    {
      debugMatrix(data());
    }

    /**
     * Dump @p matrix onto the console as debug output.
     **/
    static void debugMatrix(const float* matrix);

    /**
     * See @ref loadMatrix
     **/
    inline void operator=(const BoMatrix& m)
    {
      loadMatrix(m);
    }

  private:
    /**
     * Used by @ref toGluLookAt. Extract the up vector from the two row vectors
     * @p x and @p z.
     **/
    void extractUp(BoVector3Float& up, const BoVector3Float& x, const BoVector3Float& z) const;

  private:
    float mData[16];
};

// most has been stolen from
// http://www.gamedev.net/reference/articles/article1095.asp
// and also from
// http://www.j3d.org/matrix_faq/matrfaq_latest.html
// (it seems both articles share some code :))
// note at least in the matrix/quat faq there are some errors! (version 1.20)

/**
 * A rotation can be represented in several ways, a quaternion is one of them.
 *
 * The by far easiest and most popular way is to store 3 different rotation
 * values, one for each axis. In OpenGL code this will look like this:
 * <pre>
 * glRotatef(angleX, 1.0f, 0.0f, 0.0f);
 * glRotatef(angleY, 0.0f, 1.0f, 0.0f);
 * glRotatef(angleZ, 0.0f, 0.0f, 1.0f);
 * </pre>
 * These angles are called euler angles.
 * They suffer from the so-called * "gimbal-lock".
 *
 * Use google to find a description on what this "gimbal lock" is  - i am not
 * qualified enough to give a correct description. It is enough to say that the
 * rotation will not occur as you want it to.
 *
 * Another representation is the angle axis representation. Here you do only one
 * rotation, by an arbitrary axis. Code:
 * <pre>
 * glRotatef(angle, axisX, axisY, axisZ);
 * </pre>
 * This does not suffer from gimbal lock but (according to some
 * tutorials/howtos) it suffers from other things, such as when you interpolate
 * between two rotations. This problem does not matter for us, as we do not
 * (yet?) use it. But it is imho hard to use and to calculate.
 *
 * The third, and probably most popular among big 3d projects, way of
 * representing rotations are quaternions.
 *
 * I will not try to explain to you what exactly quaternions are - i am not
 * qualified enough to do this. Use google and e.g.
 * http://www.gamedev.net/reference/articles/article1095.asp
 *
 * A quaternion consists of 4 floating point values - a scalar (w) and a vector
 * (v).
 * You can get a rotation matrix (see @ref matrix) from a quaternion and
 * therefore you can easily use it in glMultMatrix.
 *
 * You can convert all major means of rotation into a quaternion,
 * see @ref setRotation.
 *
 * If you @ref multiply a quat by another one you get a similar effect as
 * if you had multiplied both rotation matrices, i.e. the two rotations are
 * combined.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoQuaternion
{
  public:
    BoQuaternion()
        : mW(1.0f)
    {
    }

    BoQuaternion(const BoQuaternion& quat)
    {
      *this = quat;
    }

    BoQuaternion(float w, const BoVector3Float& v)
    {
      set(w, v);
    }

    BoQuaternion& operator=(const BoQuaternion& quat)
    {
      set(quat);
      return *this;
    }

    void loadIdentity()
    {
      mW = 1.0f;
      mV.set(0.0f, 0.0f, 0.0f);
    }

    /**
     * @return The scalar part of this quaternion
     **/
    float w() const
    {
      return mW;
    }

    /**
     * @return The vector part of this quaternion
     **/
    const BoVector3Float& v() const
    {
      return mV;
    }

    /**
     * @return The quaternion converted into a rotation matrix
     **/
    BoMatrix matrix() const;

    /**
     * Multiply this quaternion by another one. The identity quaternion
     * (i.e. a quat doesnt get changed if you multiply it by this one) is
     * (1,(0,0,0)), i.e. w is 1 and the vector is (0,0,0).
     *
     * This way of combining two rotations does not suffer from gimbal lock.
     *
     * Note that this quaternion, as well as @p quat should be normalized
     * quaternions! See also @ref normalize. The resulting quaternion will be
     * normalized as well.
     **/
    void multiply(const BoQuaternion& quat)
    {
      float w = mW * quat.mW    - mV[0] * quat.mV[0] - mV[1] * quat.mV[1] - mV[2] * quat.mV[2];
      float x = mW * quat.mV[0] + mV[0] * quat.mW    + mV[1] * quat.mV[2] - mV[2] * quat.mV[1];
      float y = mW * quat.mV[1] + mV[1] * quat.mW    + mV[2] * quat.mV[0] - mV[0] * quat.mV[2];
      float z = mW * quat.mV[2] + mV[2] * quat.mW    + mV[0] * quat.mV[1] - mV[1] * quat.mV[0];
      mW = w;
      mV.set(x, y, z);
    }

    static BoQuaternion multiply(const BoQuaternion& q1, const BoQuaternion& q2)
    {
      BoQuaternion q(q1);
      q.multiply(q2);
      return q;
    }

    void operator*=(const BoQuaternion& quat)
    {
      multiply(quat);
    }

    BoQuaternion operator+(const BoQuaternion& q) const
    {
      return BoQuaternion(mW + q.mW, mV + q.mV);
    }

    /**
     * @return The conjugate of the quaternion, which is the quaternion with the
     * vector part negated.
     **/
    inline BoQuaternion conjugate() const
    {
      return BoQuaternion(mW, BoVector3Float(-mV[0], -mV[1], -mV[2]));
    }

    /**
     * @return The inverse quaternion. This is equal to the @ref conjugate, if
     * the quaternion is normalized (see @ref normalize). You should prefer @ref
     * conjugate if you know that the quat is normalized (i.e. always)!
     **/
    BoQuaternion inverse() const
    {
      // we assume that the quat is normalized.
      // If it is not, we would have to new_quat.mW /= quat.length()
      BoQuaternion q = conjugate();
      float l = length();
      q.mW /= l;
      return q;
    }

    /**
     * Rotate the vector @input and return the result into @p v.
     *
     * Of course we assume that this quaternion is normalized (see @ref
     * normalize), as only normalized quaternions represent rotations.
     **/
    void transform(BoVector3Float* v, const BoVector3Float* input) const;

    float length() const;

    /**
     * Normalize the quaternion. Note that only a normalized quaternion
     * represents a rotation, meaning that non-normalized quaternions are
     * useless for us!
     **/
    void normalize()
    {
      float l = length();
      mW /= l;
      mV.scale(1.0f / l);
    }

    bool isEqual(const BoQuaternion& quat, float diff = 0.001) const
    {
      // avoid fabsf() as we don't include math.h
      float d = mW - quat.mW;
      if (d < 0)
      {
        d = -d;
      }
      if (d > diff)
      {
        return false;
      }
      return mV.isEqual(quat.mV, diff);
    }

    bool operator==(const BoQuaternion& quat) const
    {
      return isEqual(quat);
    }

    void set(float w, const BoVector3Float& v)
    {
      mW = w;
      mV = v;
    }

    inline void set(const BoQuaternion& quat)
    {
      set(quat.mW, quat.mV);
    }

    /**
    * @param angle The angle around @p axis, given in degree.
     **/
    void setRotation(float angle, const BoVector3Float& axis);

    /**
     * The so-called "euler rotation". This creates a quaternion for as if
     * <pre>
     * glRotatef(angleX, 1, 0, 0);
     * glRotatef(angleY, 0, 1, 0);
     * glRotatef(angleZ, 0, 0, 1);
     * </pre>
     * was called (in this order).
     * @param angleX The (euler-)angle around the x-axis. given in degree.
     * @param angleY The (euler-)angle around the y-axis. given in degree.
     * @param angleZ The (euler-)angle around the z-axis. given in degree.
     **/
    void setRotation(float angleX, float angleY, float angleZ);

    /**
     * Convert a rotation matrix to the quaternion. A rotation matrix is
     * simply a matrix that describes a rotation.
     **/
    void setRotation(const BoMatrix& rotationMatrix); // See Q55 in the quat faq

    /**
     * Set the rotation according to the @p direction and the @p up vector.
     * These are compatible to the parameters used in gluLookAt, but note
     * that the @p direction differs from the lookat (aka center) vector.
     *
     * The direction is the (camera - lookat) vector.
    **/
    void setRotation(const BoVector3Float& direction, const BoVector3Float& up);

    /**
     * Convenience method for the version above. This takes exactly the
     * arguments that gluLookAt() takes.
    **/
    void setRotation(const BoVector3Float& cameraPos, const BoVector3Float& lookAt, const BoVector3Float& up)
    {
      setRotation(cameraPos - lookAt, up);
    }

    void toRotation(float* angle, BoVector3Float* axis); // see Q 57 in quat faq

    /**
     * See @ref BoMatrix::toRotation
    **/
    void toRotation(float* angleX, float* angleY, float* angleZ);

    QString debugString(int prec = 6) const;

  private:
    float mW;
    BoVector3Float mV;
};

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 * @short collection class for misc 3d functions
 **/
class Bo3dTools
{
  public:
    Bo3dTools()
    {
    }

    /**
     * @return How many degrees you have to rotate around z-axis for y-axis to go
     * through (x, y). (i.e. angle between (0, 1) and (x, y) when (x, y) is a
     * normalized vector)
     * @author Rivo Laks <rivolaks@hot.ee>
     **/
    static bofixed rotationToPoint(bofixed x, bofixed y);

    /**
     * This is the inverse operation to @ref rotationToPoint.
     * It calculates point (x, y) which is at intersection of circle with @p radius
     * and line which is rotated by @p angle around z-axis.
     * @author Rivo Laks <rivolaks@hot.ee>
     **/
    static void pointByRotation(bofixed* x, bofixed* y, const bofixed& angle, const bofixed& radius);
    static void pointByRotation(float* x, float* y, const float angle, const float radius);

    /**
     * Convert @p deg, given in degree, into radians.
     * @return @p deg as radians.
     **/
    static bofixed deg2rad(bofixed deg);
    /**
     * Convert @p rad, given in radians, into degree.
     * @return @p rad as degree.
     **/
    static bofixed rad2deg(bofixed rad);

    /**
     * See @ref BosonBigDisplayBase::extractFrustum for more information about this stuff.
     *
     * We use a bounding spere so that we can easily rotate it.
     * @return 0 if the object is not in the frustum (i.e. is not visible)
     * otherwise the distance from the near plane. We might use this for the
     * level of detail.
     * @param viewFrustum This is the viewFrustum, as it is used by @ref
     * BosonBigDisplayBase. The view frustum is a 6x4 matrix
     **/
    static float sphereInFrustum(const float* viewFrustum, const BoVector3Float&, float radius);
//    static bofixed sphereInFrustum(const float* viewFrustum, const BoVector3Fixed&, bofixed radius);

    /**
     * See @ref BosonBigDisplayBase::extractFrustum for more information about this stuff.
     *
     * @param viewFrustum This is the viewFrustum, as it is used by @ref
     * BosonBigDisplayBase. The view frustum is a 6x4 matrix
     **/
    static bool boxInFrustum(const float* viewFrustum, const BoVector3Float& min, const BoVector3Float& max);

    /**
     * This is similar to @ref sphereInFrustum, but will test whether the sphere
     * is completely in the frustum.
     *
     * @return 0 if the sphere is not in the frustum at all, 1 if it is
     * partially in the frustum and 2 if the complete sphere is in the frustum.
     **/
    static int sphereCompleteInFrustum(const float* viewFrustum, const BoVector3Float&, float radius);

    /**
     * @overload
     **/
    inline static float sphereInFrustum(const float* viewFrustum, float x, float y, float z, float radius)
    {
      BoVector3Float pos(x,y,z);
      return sphereInFrustum(viewFrustum, pos, radius);
    }

};


#endif // BO3DTOOLS_H
/*
 * vim:et sw=2
 */
