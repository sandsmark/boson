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

#ifndef BOPLANE_H
#define BOPLANE_H

#include "math/bomath.h"
#include "math/bovector.h"

#include <math.h>

class BoPlane
{
public:
	BoPlane()
	{
		mDistanceFromOrigin = 0.0f;
	}

	/**
	 * Construct a plane with normal vector @p normal and a distance of @p distance
	 * from the origin.
	 * See @ref loadPlane
	 **/
	BoPlane(const BoVector3Float& normal, float distance)
	{
		loadPlane(normal, distance);
	}

	/**
	 * Construct a plane with normal vector @p normal and a @p pointOnPlane.
	 * See @ref loadPlane
	 **/
	BoPlane(const BoVector3Float& normal, const BoVector3Float& pointOnPlane)
	{
		loadPlane(normal, pointOnPlane);
	}

	/**
	 * Construct a plane that is a copy of @p plane . See @ref loadPlane
	 * See @ref loadPlane
	 **/
	BoPlane(const float* plane)
	{
		loadPlane(plane);
	}

	/**
	 * Construct a plane with normal vector @p normal and a distance of @p distance
	 * from the origin.
	 * See @ref loadPlane
	 **/
	BoPlane(const float* normal, float distance)
	{
		loadPlane(normal, distance);
	}

	/**
	 * Construct plane matrix that is a copy of @p plane.
	 * See @ref loadPlane
	 **/
	BoPlane(const BoPlane& plane)
	{
		loadPlane(plane);
	}

	/**
	 * Load the plane using its @p normal and its @p distance from the
	 * origin.
	 **/
	void loadPlane(const BoVector3Float& normal, float distance)
	{
		mNormal = normal;
		mDistanceFromOrigin = distance;
		normalize();
	}
	void loadPlane(const BoVector3Float& normal, const BoVector3Float& pointOnPlane)
	{
		// AB:
		// "pointOnPlane" (in the following I'll call it p) can be
		// interpreted as a vector from the origin to that point.
		//
		// The distanceFromOrigin of the plane can then easily be
		// calculated:
		// We have the normal and p interpreted as a vector. together
		// with the plane, they make up a triangle.
		// In that triangle, between p and the normal there is the angle
		// alpha:
		// cos(alpha) = distanceFromOrigin / |p|
		// as "distanceFromOrigin" is the length of the normal from the
		// origin to the plane. It also is the adjacent side in our
		// triangle. |p| is the hyypotenuse.
		// This can be written as
		// distanceFromOrigin = |p| * cos(alpha)
		//
		// Now if the normal is normalized (i.e. |normal| = 1) this is
		// exactly what dotProduct(p, normal) calculates.
		//
		// We need to multiply that with -1 so that the normal in the
		// triangle points into the correct direction
		// (ok, lousy explanation. i dont find a better one - consider
		// to use a piece of paper to make this clear to yourself)

		BoVector3Float normalizedNormal = normal;
		normalizedNormal.normalize();
		float distance = -BoVector3Float::dotProduct(normalizedNormal, pointOnPlane);
		loadPlane(normalizedNormal, distance);
	}
	void loadPlane(const float* plane)
	{
		loadPlane(plane, plane[3]);
	}
	void loadPlane(const float* normal, float distance)
	{
		loadPlane(BoVector3Float(normal), distance);
	}
	void loadPlane(const BoPlane& p)
	{
		loadPlane(p.normal().data(), p.distanceFromOrigin());
	}

	inline const BoVector3Float& normal() const
	{
		return mNormal;
	}

	/**
	 * @return The distance from the origin
	 **/
	inline float distanceFromOrigin() const
	{
		return mDistanceFromOrigin;
	}
	/**
	 * Many algorithms work by using the normal of the plane and a point on
	 * the plane (instead of the @ref distanceFromOrigin). This method
	 * provides such a point - but note that it must be created on the fly,
	 * it is not stored in this class.
	 * @return A point on the plane
	 **/
	BoVector3Float pointOnPlane() const
	{
		return normal() * (-distanceFromOrigin());
	}

	/**
	 * Dump this plane to the console as debug output.
	 **/
	void debugPlane()
	{
		debugPlane(normal(), distanceFromOrigin());
	}

	/**
	 * Dump plane onto the console as debug output.
	 **/
	static void debugPlane(const BoVector3Float& normal, float distanceFromOrigin);

	/**
	 * See @ref loadPlane
	 **/
	inline void operator=(const BoPlane& p)
	{
		loadPlane(p);
	}

	inline float distance(const BoVector3Float& p) const
	{
		return BoVector3Float::dotProduct(normal(), p) + distanceFromOrigin();
	}
	float distance(float x, float y, float z) const
	{
		return distance(BoVector3Float(x, y, z));
	}
	inline bofixed distance(const BoVector3Fixed& p) const
	{
		BoVector3Fixed n(normal().x(), normal().y(), normal().z());
		return BoVector3Fixed::dotProduct(n, p) + distanceFromOrigin();
	}
	bofixed distance(bofixed x, bofixed y, bofixed z) const
	{
		return distance(BoVector3Fixed(x, y, z));
	}

	bool onPlane(float x, float y, float z) const
	{
		return onPlane(BoVector3Float(x, y, z));
	}
	bool onPlane(const BoVector3Float& pos, float epsilon = 0.0001) const
	{
		return (fabsf(distance(pos))) < epsilon;
	}

	bool behindPlane(float x, float y, float z) const
	{
		return behindPlane(BoVector3Float(x, y, z));
	}
	bool behindPlane(const BoVector3Float& pos) const
	{
		return (distance(pos) < 0.0f);
	}

	bool inFrontOfPlane(float x, float y, float z) const
	{
		return inFrontOfPlane(BoVector3Float(x, y, z));
	}
	bool inFrontOfPlane(const BoVector3Float& pos) const
	{
		return (distance(pos) > 0.0f);
	}

	bool isEqual(const BoPlane& plane) const
	{
		if (!normal().isEqual(plane.normal(), 0.001)) {
			return false;
		}
		if (fabsf(distanceFromOrigin() - plane.distanceFromOrigin()) > 0.001) {
			return false;
		}
		return true;
	}

	void normalize()
	{
		float inv_length = 1.0f / normal().length();
		mNormal.scale(inv_length);
		mDistanceFromOrigin *= inv_length;
	}

	/**
	 * This calculates the intersection line of @p plane1 and @p
	 * plane2.
	 *
	 * In 3d space, two planes are either equal (see @ref isEqual), parallel
	 * but not equal, or they intersect in a line.
	 *
	 * The line is represented as a point and a direction vector. Every point on the
	 * line can be calculated using
	 * <code>
	 * BoVector3Float point = *intersectionPoint + a * (*inersectionVector)
	 * </code>
	 * where a is any real number.
	 *
	 * Note that we do not have a dedicated class for points and therefore use the
	 * vector class here for both, however it is important to note that the point is
	 * really a <em>point</em> whereas the vector is actually a vector, i.e. a
	 * direction.
	 *
	 *
	 * @return TRUE if the two planes intersect. In this case @p
	 * intersectionPoint contains one point of the intersection line and @p
	 * intersectionVector contains the direction of that line. Otherwise
	 * (the planes do not intersect) FALSE is returned - the planes are
	 * parallel then. Note that the planes can still intersect, if they are
	 * equal, see @ref isEqual.
	 **/
	static bool intersectPlane(const BoPlane& plane1, const BoPlane& plane2, BoVector3Float* intersectionPoint, BoVector3Float* intersectionVector);

	bool intersectPlane(const BoPlane& plane, BoVector3Float* intersectionPoint, BoVector3Float* intersectionVector) const
	{
		return BoPlane::intersectPlane(*this, plane, intersectionPoint, intersectionVector);
	}

	/**
	* This returns the intersection of a line and a plane in @p intersection. Note
	* that a line always has infinite length - line segments (finite length) are
	* not handled here.
	*
	* This function assumes that an intersection actually exists, as this is what
	* we need in this file. Checking whether an intersection exists, is pretty
	* easy.
	*
	* @param linePoint Just a point on the line
	* @param lineVector The line vector, i.e. the direction of the line. This is
	* returned by @ref planes_intersect or can easily be calculated using (p0-p1)
	* if p0 and p1 are two different points on the line.
	**/
	bool intersectLine(const BoVector3Float& linePoint, const BoVector3Float& lineVector, BoVector3Float* intersection) const;
	bool intersectLineSegment(const BoVector3Float& linePoint1, const BoVector3Float& linePoint2, BoVector3Float* intersection) const;

protected:
	/**
	* @param factor how often the @p lineVector has to be added to @p linePoint to
	* get the intersection point. If @p factor is between 0 and 1, then the
	* intersection point is on the line segment that starts at linePoint and ends
	* with linePoint + lineVector.
	* @return TRUE if the line intersects with the plane
	**/
	bool intersectLineInternal(const BoVector3Float& linePoint, const BoVector3Float& lineVector, float* factor) const;

private:
	BoVector3Float mNormal;
	float mDistanceFromOrigin;
};

#endif
