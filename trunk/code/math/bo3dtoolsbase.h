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

#ifndef BO3DTOOLSBASE_H
#define BO3DTOOLSBASE_H

#include <math.h>

#include "math/bomath.h"
#include "math/bovector.h"
#include "math/bomatrix.h"

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
    static bofixed deg2rad(bofixed deg);
    /**
     * Convert @p rad, given in radians, into degree.
     * @return @p rad as degree.
     **/
    static bofixed rad2deg(bofixed rad);

    /**
     * @param plane An array of size 4 representing a plane. The first 3 floats
     * are the normal vector, the 4th value the distance.
     * @return The distance of @pos from the @p plane. Negative if @p pos is
     * behind the @p plane.
     **/
    static float distanceFromPlane(const float* plane, const BoVector3Float& pos);

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
    static bofixed sphereInFrustum(const float* viewFrustum, const BoVector3Fixed&, bofixed radius);

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
    static int sphereCompleteInFrustum(const float* viewFrustum, const BoVector3Fixed&, bofixed radius);

    /**
     * @overload
     **/
    inline static float sphereInFrustum(const float* viewFrustum, float x, float y, float z, float radius)
    {
      BoVector3Float pos(x,y,z);
      return sphereInFrustum(viewFrustum, pos, radius);
    }

};


#endif
/*
 * vim:et sw=2
 */
