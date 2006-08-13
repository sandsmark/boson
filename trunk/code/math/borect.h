/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Rivo Laks (rivolaks@hot.ee)
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)

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

#ifndef BORECT_H
#define BORECT_H

#include "math/bomath.h"
#include "math/bovector.h"

template<class T> class BoRect2;
template<class T> class BoRect3;
typedef BoRect2<bofixed> BoRect2Fixed;
typedef BoRect2<float> BoRect2Float;
typedef BoRect3<bofixed> BoRect3Fixed;
typedef BoRect3<float> BoRect3Float;

/**
 * @short Rectangle class
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
template<class T> class BoRect2
{
  public:
    BoRect2()
    {
      set(0, 0, 1, 1);
    }
    BoRect2(const BoVector2<T>& topLeft, const BoVector2<T>& bottomRight)
    {
      set(topLeft, bottomRight);
    }
    BoRect2(const T left, const T top, const T right, const T bottom)
    {
      set(left, top, right, bottom);
    }
    BoRect2(const BoRect2<T>& r)
    {
      *this = r;
    }
    BoRect2<T>& operator=(const BoRect2<T>& r)
    {
      set(r.topLeft(), r.bottomRight());
      return *this;
    }

    inline void set(const BoVector2<T>& topLeft, const BoVector2<T>& bottomRight)
    {
      mTopLeft = topLeft;
      mBottomRight = bottomRight;
      if (left() > right())
      {
        float tmp = mTopLeft.x();
        mTopLeft.setX(mBottomRight.x());
        mBottomRight.setX(tmp);
      }
      if (top() > bottom())
      {
        float tmp = mTopLeft.y();
        mTopLeft.setY(mBottomRight.y());
        mBottomRight.setY(tmp);
      }
    }
    inline void set(const T left, const T top, const T right, const T bottom)
    {
      set(BoVector2<T>(left, top), BoVector2<T>(right, bottom));
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

    inline BoVector2<T> center() const
    {
      return BoVector2<T>((left() + right()) / 2, (top() + bottom()) / 2);
    }

    bool contains(const BoVector2<T>& p) const
    {
      if (left() > p.x() || right() < p.x())
      {
        return false;
      }
      if (top() > p.y() || bottom() < p.y())
      {
        return false;
      }
      return true;
    }


  private:
    BoVector2<T> mTopLeft;
    BoVector2<T> mBottomRight;
};

template<class T> class BoRect3
{
  public:
    BoRect3()
    {
      set(0, 0, 0, 1, 1, 1);
    }
    BoRect3(const BoVector3<T>& topLeftBack, const BoVector3<T>& bottomRightFront)
    {
      set(topLeftBack, bottomRightFront);
    }
    BoRect3(const T left, const T top, const T back, const T right, const T bottom, const T front)
    {
      set(BoVector3<T>(left, top, back), BoVector3<T>(right, bottom, front));
    }
    BoRect3(const BoRect3<T>& r)
    {
      *this = r;
    }
    BoRect3<T>& operator=(const BoRect3<T>& r)
    {
      set(r.topLeftBack(), r.bottomRightFront());
      return *this;
    }

    inline void set(const BoVector3<T>& topLeftBack, const BoVector3<T>& bottomRightFront)
    {
      mTopLeftBack = topLeftBack;
      mBottomRightFront = bottomRightFront;
      if (left() > right())
      {
        float tmp = mTopLeftBack.x();
        mTopLeftBack.setX(mBottomRightFront.x());
        mBottomRightFront.setX(tmp);
      }
      if (top() > bottom())
      {
        float tmp = mTopLeftBack.y();
        mTopLeftBack.setY(mBottomRightFront.y());
        mBottomRightFront.setY(tmp);
      }
      if (back() > front())
      {
        float tmp = mTopLeftBack.z();
        mTopLeftBack.setZ(mBottomRightFront.z());
        mBottomRightFront.setZ(tmp);
      }
    }
    inline void set(const T left, const T top, const T front, const T right, const T bottom, const T back)
    {
      mTopLeftBack = BoVector3<T>(left, top, back);
      mBottomRightFront = BoVector3<T>(right, bottom, front);
    }

    inline T left() const  { return mTopLeftBack.x(); }
    inline T top() const  { return mTopLeftBack.y(); }
    inline T back() const { return mTopLeftBack.z(); }
    inline T right() const  { return mBottomRightFront.x(); }
    inline T bottom() const  { return mBottomRightFront.y(); }
    inline T front() const { return mBottomRightFront.z(); }

    inline const BoVector3<T>& topLeftBack() const { return mTopLeftBack; }
    inline const BoVector3<T>& bottomRightFront() const { return mBottomRightFront; }

    inline T x() const  { return mTopLeftBack.x(); }
    inline T y() const  { return mTopLeftBack.y(); }
    inline T z() const  { return mTopLeftBack.z(); }

    inline T width() const  { return right() - left(); }
    inline T height() const  { return bottom() - top(); }
    inline T depth() const  { return front() - back(); }

    inline BoVector3<T> center() const
    {
      return BoVector3<T>((left() + right()) / 2,
          (top() + bottom()) / 2,
          (back() + front()) / 2);
    }

    bool contains(const BoVector3<T>& p) const
    {
      if (left() > p.x() || right() < p.x())
      {
        return false;
      }
      if (top() > p.y() || bottom() < p.y())
      {
        return false;
      }
      if (back() > p.z() || front() < p.z())
      {
        return false;
      }
      return true;
    }


  private:
    BoVector3<T> mTopLeftBack;
    BoVector3<T> mBottomRightFront;
};

#endif
/*
 * vim:et sw=2
 */
