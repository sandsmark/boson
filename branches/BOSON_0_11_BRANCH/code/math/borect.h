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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef BORECT_H
#define BORECT_H

#include "math/bomath.h"
#include "math/bovector.h"

template<class T> class BoRect;
typedef BoRect<bofixed> BoRectFixed;
typedef BoRect<float> BoRectFloat;

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

#endif
/*
 * vim:et sw=2
 */
