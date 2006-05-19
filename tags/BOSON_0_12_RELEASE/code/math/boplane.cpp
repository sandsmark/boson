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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "math/boplane.h"

#include <math.h>
#include <stdio.h>

void BoPlane::debugPlane(const BoVector3Float& normal, float distance)
{
 printf("((%f %f %f) %f\n", normal[0], normal[1], normal[2], distance);
}

bool BoPlane::intersectPlane(const BoPlane& plane1, const BoPlane& plane2, BoVector3Float* intersectionPoint, BoVector3Float* intersectionVector)
{
 const BoVector3Float& n1 = plane1.normal();
 const BoVector3Float& n2 = plane2.normal();
 BoVector3Float cross = BoVector3Float::crossProduct(n1, n2);
 if (cross.isEqual(0.0f, 0.0f, 0.0f)) {
	// AB: note that intersections are still possible, if planes are equal
	// -> see isEqual()
	return false;
 }

 *intersectionVector = cross;

 // AB: the cross product is already 50% of the work.
 // The cross product gives a vector that is perpendicular to both, n1 and n2.
 // -> cross is perpendicular to n1, meaning it is on plane1.
 // -> cross is perpendicular to n2, meaning it is on plane2.
 // => cross is a vector that describes the direction of a line that is on both,
 //    plane1 and plane2. it is that line that we want.
 //    Now all we need is the starting point of the line.


 // A 3d plane can be described in several ways - e.g. using a normal vector "n"
 // and a point "p0" on the plane (p0 can be retrieved from our planes using
 // n*distanceFromOrigin).
 // Another way is the equation "n * ((x,y,z) - p0) = 0", where the plane is the
 // set of all solutions to that equation. Note that by "*" we mean
 // dotproduct here.
 //
 // What we have: 2 equations of the form "n * ((x,y,z) - p0) = 0" (i.e. 2 planes)
 // What we want: one solution to these equations, i.e. one point that is on both
 // planes. This is also one point of the intersection line.
 //
 // 2 equations and 3 variables.
 // -> the vector "cross" describes the direction of the intersection line.
 //    It must also intersect at least one of the x-/y-/z-planes.
 //    If a component of "cross" is non-zero, the line must intersect with the
 //    corresponding plane, so if e.g. the z component is non-zero, we can
 //    assume z=0 and have only 2 equations and 2 variables left.
 //    However due to rounding errors with float variables, we cannot safely
 //    check for a component "!= 0.0f", so we just pick the one with the largest
 //    absolute value (which is of course guaranteed to be non-zero, as the
 //    vector is non-zero).


 // AB: note that our normals are directed to the _inside_ of the frustum,
 // therefore we need to multiply be the negative normal (or by the negative
 // distance) to get a point on the plane
 //
 // (this is because this code is intended for use in conjunction with
 // viewFrustum planes)
 BoVector3Float p0Plane1 = n1 * (-plane1.distanceFromOrigin());
 BoVector3Float p0Plane2 = n2 * (-plane2.distanceFromOrigin());

 float absx = fabsf(cross[0]);
 float absy = fabsf(cross[1]);
 float absz = fabsf(cross[2]);

 // "n1 * p0Plane1" and "n2 * p0Plane2" is used in all three cases below
 float n1DotP0Plane1 = BoVector3Float::dotProduct(n1, p0Plane1);
 float n2DotP0Plane2 = BoVector3Float::dotProduct(n2, p0Plane2);

 if (absx >= absy && absx >= absz) {
	// "n * ((x,y,z) - p0) = 0" simplifies to "n * ((0,y,z) - p0) = 0" for
	// both planes, i.e.
	// "n1 * ((x,y,0) - p0Plane1) = 0" and "n2 * ((0,y,z) - p0Plane2) = 0"
	// this is a 2 equations with 2 variables which is solved below.

	intersectionPoint->setX(0.0f);
	intersectionPoint->setY((n2DotP0Plane2 * n1.z() - n1DotP0Plane1 * n2.z()) / (n1.z() * n2.y() - n2.z() * n1.y()));
	intersectionPoint->setZ((n2DotP0Plane2 * n1.y() - n1DotP0Plane1 * n2.y()) / (n1.y() * n2.z() - n2.y() * n1.z()));
 } else if (absy >= absx && absy >= absz) {
	intersectionPoint->setY(0.0f);
	intersectionPoint->setX((n2DotP0Plane2 * n1.z() - n1DotP0Plane1 * n2.z()) / (n1.z() * n2.x() - n2.z() * n1.x()));
	intersectionPoint->setZ((n2DotP0Plane2 * n1.x() - n1DotP0Plane1 * n2.x()) / (n1.x() * n2.z() - n2.x() * n1.z()));
 } else {
	intersectionPoint->setZ(0.0f);
	intersectionPoint->setX((n2DotP0Plane2 * n1.y() - n1DotP0Plane1 * n2.y()) / (n1.y() * n2.x() - n2.y() * n1.x()));
	intersectionPoint->setY((n2DotP0Plane2 * n1.x() - n1DotP0Plane1 * n2.x()) / (n1.x() * n2.y() - n2.x() * n1.y()));
 }
 return true;
}

bool BoPlane::intersectLine(const BoVector3Float& linePoint, const BoVector3Float& lineVector, BoVector3Float* intersection) const
{
 float factor;
 if (!intersectLineInternal(linePoint, lineVector, &factor)) {
	return false;
 }
 *intersection = linePoint + lineVector * factor;
 return true;
}

bool BoPlane::intersectLineSegment(const BoVector3Float& linePoint1, const BoVector3Float& linePoint2, BoVector3Float* intersection) const
{
 float factor;
 BoVector3Float lineVector = (linePoint2 - linePoint1);
 if (!intersectLineInternal(linePoint1, lineVector, &factor)) {
	return false;
 }
 if (factor < 0.0f || factor > 1.0f) {
	return false;
 }
 *intersection = linePoint1 + lineVector * factor;
 return true;
}

bool BoPlane::intersectLineInternal(const BoVector3Float& linePoint, const BoVector3Float& lineVector, float* factor) const
{
 // http://geometryalgorithms.com/Archive/algorithm_0104/algorithm_0104B.htm#intersect3D_SegPlane()
 // provides a nice explanation of the maths in here

 float NdotLine = BoVector3Float::dotProduct(normal(), lineVector);
 if (fabsf(NdotLine) <= 0.001) {
	// intersection still possible, if the line is on the plane.
	return false;
 }

 // now we know the line _does_ intersect with the plane (let's call the point p)
 // the vector from a point on the plane to p is perpendicular to the plane
 // normal.
 // That means their dot product is 0.
 //
 // i.e. normal * (point_on_plane - p) = 0
 // "p" can also be written as linePoint + a * lineVector, with a being a
 // certain real number. this makes:
 // normal * (point_on_plane - (linePoint + a * lineVector)) = 0
 // =>
 // normal * (point_on_plane - linePoint) + a * normal * lineVector = 0
 // =>
 // a = (normal * (point_on_plane - linePoint)) / (normal * lineVector)

 float foo = BoVector3Float::dotProduct(normal(), pointOnPlane() - linePoint);

 float a = foo / NdotLine;

 *factor = a;
 return true;
}

