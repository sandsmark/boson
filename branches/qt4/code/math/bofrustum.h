/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

#ifndef BOFRUSTUM_H
#define BOFRUSTUM_H

#include "math/bomath.h"
#include "math/bovector.h"
#include "math/boplane.h"
#include "math/bomatrix.h"
#include "math/borect.h"

#include <math.h>

class BoFrustum
{
public:
	BoFrustum()
	{
	}

	/**
	 * Extract the frustum from @p projectionModelview, which is projection
	 * multiplied by modelview.
	 *
	 * Credits for this method partially go to Mark Morley - see
	 * http://www.markmorley.com/opengl/frustumculling.html
	 * (AB: link is broken now, site does not exist anymore)
	 *
	 * We use pretty much of his code examples here so let me quote from the
	 * article: "[...] Unless otherwise noted, you may use any and all code
	 * examples provided herein in any way you want. [...]"
	 *
	 * Also see (even more important than the Mark Morley link):
	 * http://www2.ravensoft.com/users/ggribb/plane_extraction.pdf
	 * This is the original paper that describes this.
	 **/
	void loadViewFrustum(const BoMatrix& projectionModelview);

	/**
	 * @overload
	 * Convenience method. This lets you specify both, the modelview and the
	 * projection matrix and multiplies on its own.
	 **/
	void loadViewFrustum(const BoMatrix& modelview, const BoMatrix& projection)
	{
		BoMatrix m(projection);
		m.multiply(&modelview);
		loadViewFrustum(m);
	}

	/**
	 * Create a viewfrustum for picking, i.e. a viewfrustum where the @p
	 * pickRect (in window coordinates) fill the viewport.
	 *
	 * See also @ref loadViewFrustum
	 *
	 * @param pickRect The rectangle inside the viewport that is to be
	 * picked. This is in Qt/X11 widget coordinates, i.e. y=0 is top!
	 **/
	void loadPickViewFrustum(const BoRect2Float& pickRect, const int* viewport, const BoMatrix& modelview, const BoMatrix& projection);

	const BoPlane& right() const { return mPlanes[0]; }
	const BoPlane& left() const { return mPlanes[1]; }
	const BoPlane& bottom() const { return mPlanes[2]; }
	const BoPlane& top() const { return mPlanes[3]; }
	const BoPlane& far() const { return mPlanes[4]; }
	const BoPlane& near() const { return mPlanes[5]; }


	float sphereInFrustum(const BoVector3Float&, float radius) const;
	bofixed sphereInFrustum(const BoVector3Fixed&, bofixed radius) const;

	bool boxInFrustum(const BoVector3Float& min, const BoVector3Float& max) const;
	bool boxInFrustum(const BoRect3Float& rect) const
	{
		return boxInFrustum(rect.topLeftBack(), rect.bottomRightFront());
	}

	/**
	 * This is similar to @ref sphereInFrustum, but will test whether the sphere
	 * is completely in the frustum.
	 *
	 * @param dist if non-null, distance between object and near plane
	 *
	 * @return 0 if the sphere is not in the frustum at all, 1 if it is
	 * partially in the frustum and 2 if the complete sphere is in the frustum.
	 **/
	int sphereCompleteInFrustum(const BoVector3Float&, float radius, float* dist = 0) const;
	int sphereCompleteInFrustum(const BoVector3Fixed&, bofixed radius, bofixed* dist = 0) const;

	/**
	 * @overload
	 **/
	inline float sphereInFrustum(float x, float y, float z, float radius) const
	{
		return sphereInFrustum(BoVector3Float(x, y, z), radius);
	}

private:
	BoPlane mPlanes[6];
};

#endif
