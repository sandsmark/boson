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

#ifndef BO3DTOOLSBASE_H
#define BO3DTOOLSBASE_H

#include <math.h>

#include "math/bomath.h"
#include "math/bovector.h"
#include "math/bomatrix.h"

// Degrees to radians conversion (AB: from mesa/src/macros.h)
#define DEG2RAD (M_PI/180.0)
// And radians to degrees conversion
#define RAD2DEG (180.0/M_PI)

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 * @short collection class for misc 3d functions
 **/
class Bo3dToolsBase
{
  public:
    Bo3dToolsBase()
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
    inline static bofixed deg2rad(bofixed deg)  { return deg * DEG2RAD; }
    /**
     * Convert @p rad, given in radians, into degree.
     * @return @p rad as degree.
     **/
    inline static bofixed rad2deg(bofixed rad)  { return rad * RAD2DEG; }
};

#undef DEG2RAD
#undef RAD2DEG

#endif
/*
 * vim:et sw=2
 */
