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

#include "math/bofrustum.h"

#include <math.h>
#include <stdio.h>

void BoFrustum::loadViewFrustum(const BoMatrix& m)
{
 // Extract the numbers for the RIGHT plane
 mPlanes[0].loadPlane(BoVector3Float(m[3] - m[0], m[7] - m[4], m[11] - m[8]), m[15] - m[12]);

 // Extract the numbers for the LEFT plane
 mPlanes[1].loadPlane(BoVector3Float(m[3] + m[0], m[7] + m[4], m[11] + m[8]), m[15] + m[12]);

 // Extract the BOTTOM plane
 mPlanes[2].loadPlane(BoVector3Float(m[3] + m[1], m[7] + m[5], m[11] + m[9]), m[15] + m[13]);

 // Extract the TOP plane
 mPlanes[3].loadPlane(BoVector3Float(m[3] - m[1], m[7] - m[5], m[11] - m[9]), m[15] - m[13]);

 // Extract the FAR plane
 mPlanes[4].loadPlane(BoVector3Float(m[3] - m[2], m[7] - m[6], m[11] - m[10]), m[15] - m[14]);

 // Extract the NEAR plane
 mPlanes[5].loadPlane(BoVector3Float(m[3] + m[2], m[7] + m[6], m[11] + m[10]), m[15] + m[14]);
}

void BoFrustum::loadPickViewFrustum(const BoRect2Float& pickRect, const int* viewport, const BoMatrix& modelview, const BoMatrix& _projection)
{
 const BoVector2Float pickCenter = pickRect.center();
 const float viewX = (float)viewport[0];
 const float viewY = (float)viewport[1];
 const float viewW = (float)viewport[2];
 const float viewH = (float)viewport[3];
 const float pickW = qMax(pickRect.width(), 1.0f);
 const float pickH = qMax(pickRect.height(), 1.0f);
 const float pickCenterX = pickCenter.x();
 const float pickCenterY = viewH - pickCenter.y();

 BoMatrix pick;
 const float scaleX = viewW / pickW;
 const float scaleY = viewH / pickH;
 pick.scale(scaleX, scaleY, 1.0f);

 float translateX;
 float translateY;
 translateX = 1.0f - 2.0f * (pickCenterX - viewX) / viewW;
 translateY = 1.0f - 2.0f * (pickCenterY - viewY) / viewH;

 pick.translate(translateX, translateY, 0.0f);

 BoMatrix projection(pick);
 projection.multiply(&_projection);
 loadViewFrustum(modelview, projection);
}

float BoFrustum::sphereInFrustum(const BoVector3Float& pos, float radius) const
{
 // FIXME: performance: we might unroll the loop and then make this function
 // inline. We call it pretty often!
 float distance;
 for (int p = 0; p < 6; p++) {
	distance = mPlanes[p].distance(pos);
	if (distance <= -radius) {
		return 0;
	}
 }

 // we return distance from near plane + radius, which is how far the object is
 // away from the camera!
 return distance + radius;
}

bofixed BoFrustum::sphereInFrustum(const BoVector3Fixed& pos, bofixed radius) const
{
 // FIXME: performance: we might unroll the loop and then make this function
 // inline. We call it pretty often!
 bofixed distance;
 for (int p = 0; p < 6; p++) {
	distance = mPlanes[p].distance(pos);
	if (distance <= -radius) {
		return 0;
	}
 }

 // we return distance from near plane + radius, which is how far the object is
 // away from the camera!
 return distance + radius;
}

int BoFrustum::sphereCompleteInFrustum(const BoVector3Float& pos, float radius, float* dist) const
{
 float distance;
 int c = 0;
 for (int p = 0; p < 6; p++) {
	distance = mPlanes[p].distance(pos);
	if (distance <= -radius) {
		return 0;
	}
	if (distance > radius) {
		c++;
	}
 }
 if (dist) {
	*dist = distance + radius;
 }
 if (c == 6) {
	return 2;
 }
 return 1;
}

int BoFrustum::sphereCompleteInFrustum(const BoVector3Fixed& pos, bofixed radius, bofixed* dist) const
{
 bofixed distance;
 int c = 0;
 for (int p = 0; p < 6; p++) {
	distance = mPlanes[p].distance(pos);
	if (distance <= -radius) {
		return 0;
	}
	if (distance > radius) {
		c++;
	}
 }
 if (dist) {
	*dist = distance + radius;
 }
 if (c == 6) {
	return 2;
 }
 return 1;
}

bool BoFrustum::boxInFrustum(const BoVector3Float& min, const BoVector3Float& max) const
{
 for (int p = 0; p < 6; p++) {
	if (mPlanes[p].distance(min.x(), min.y(), min.z()) > 0.0f) {
		continue;
	}
	if (mPlanes[p].distance(max.x(), min.y(), min.z()) > 0.0f) {
		continue;
	}
	if (mPlanes[p].distance(min.x(), max.y(), min.z()) > 0.0f) {
		continue;
	}
	if (mPlanes[p].distance(max.x(), max.y(), min.z()) > 0.0f) {
		continue;
	}
	if (mPlanes[p].distance(min.x(), min.y(), max.z()) > 0.0f) {
		continue;
	}
	if (mPlanes[p].distance(max.x(), min.y(), max.z()) > 0.0f) {
		continue;
	}
	if (mPlanes[p].distance(min.x(), max.y(), max.z()) > 0.0f) {
		continue;
	}
	if (mPlanes[p].distance(max.x(), max.y(), max.z()) > 0.0f) {
		continue;
	}
	return false;
 }
 return true;
}


