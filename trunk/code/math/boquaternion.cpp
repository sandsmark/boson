/*
    This file is part of the Boson game
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "math/boquaternion.h"

#include "math/bomatrix.h"
#include "math/bo3dtoolsbase.h"

/*****  BoQuaternion  *****/

BoMatrix BoQuaternion::matrix() const
{
 BoMatrix m;
 const float x = mV[0];
 const float y = mV[1];
 const float z = mV[2];
 const float xx = x * x;
 const float yy = y * y;
 const float zz = z * z;
 const float xy = x * y;
 const float xz = x * z;
 const float xw = mW * x;
 const float yz = y * z;
 const float yw = mW * y;
 const float zw = mW * z;
 m.setElement(0, 0, 1.0f - 2.0f * (yy + zz));
 m.setElement(1, 0,        2.0f * (xy + zw));
 m.setElement(2, 0,        2.0f * (xz - yw));

 m.setElement(0, 1,        2.0f * (xy - zw));
 m.setElement(1, 1, 1.0f - 2.0f * (xx + zz));
 m.setElement(2, 1,        2.0f * (yz + xw));

 m.setElement(0, 2,        2.0f * (xz + yw));
 m.setElement(1, 2,        2.0f * (yz - xw));
 m.setElement(2, 2, 1.0f - 2.0f * (xx + yy));

 return m;
}

float BoQuaternion::length() const
{
 return (float)sqrt(mW * mW + mV[0] * mV[0] + mV[1] * mV[1] + mV[2] * mV[2]);
}

void BoQuaternion::setRotation(const BoVector3Float& direction_, const BoVector3Float& up_)
{
 BoVector3Float dir(direction_);
 BoVector3Float up(up_);
 dir.normalize();

 BoVector3Float x = BoVector3Float::crossProduct(up, dir);
 BoVector3Float y = BoVector3Float::crossProduct(dir, x);
 x.normalize();
 y.normalize();

 BoMatrix M;
 M.setElement(0, 0, x[0]);
 M.setElement(0, 1, x[1]);
 M.setElement(0, 2, x[2]);

 M.setElement(1, 0, y[0]);
 M.setElement(1, 1, y[1]);
 M.setElement(1, 2, y[2]);

 M.setElement(2, 0, dir[0]);
 M.setElement(2, 1, dir[1]);
 M.setElement(2, 2, dir[2]);

 setRotation(M);
}

void BoQuaternion::setRotation(float angle, const BoVector3Float& axis)
{
 BoVector3Float normAxis = axis;
 normAxis.normalize();
 float sina = sin(Bo3dToolsBase::deg2rad(angle / 2));
 mW = cos(Bo3dToolsBase::deg2rad(angle / 2));
 mV.set(normAxis[0] * sina, normAxis[1] * sina, normAxis[2] * sina);
 normalize();
}

void BoQuaternion::setRotation(float angleX, float angleY, float angleZ)
{
 BoQuaternion x, y, z;
 // one quaternion per axis
 x.set((float)cos(Bo3dToolsBase::deg2rad(angleX/2)), BoVector3Float((float)sin(Bo3dToolsBase::deg2rad(angleX/2)), 0.0f, 0.0f));
 y.set((float)cos(Bo3dToolsBase::deg2rad(angleY/2)), BoVector3Float(0.0f, (float)sin(Bo3dToolsBase::deg2rad(angleY/2)), 0.0f));
 z.set((float)cos(Bo3dToolsBase::deg2rad(angleZ/2)), BoVector3Float(0.0f, 0.0f, (float)sin(Bo3dToolsBase::deg2rad(angleZ/2))));
 x.multiply(y);
 x.multiply(z);
 set(x);
 normalize();
}

void BoQuaternion::setRotation(const BoMatrix& rotationMatrix)
{
 // See Q55 in the quat faq on http://www.j3d.org/matrix_faq/matrfaq_latest.html
 // WARNING: although they refer to a column order matrix in Q54, they use _row_
 // order here!
 float x, y, z, w;
 const float* m = rotationMatrix.data();
 float t = 1.0f + m[0] + m[5] + m[10];
 if (t > 0.0f)
 {
   float s = sqrtf(t) * 2.0f;
   x = (m[6] - m[9]) / s;
   y = (m[8] - m[2]) / s;
   z = (m[1] - m[4]) / s;
   w = 0.25f * s;
 }
 else if (m[0] > m[5] && m[0] > m[10])
 {
   float s = sqrtf(1.0 + m[0] - m[5] - m[10]) * 2.0f;
   x = 0.25f * s;
   y = (m[1] + m[4]) / s;
   z = (m[8] + m[2]) / s;
   w = (m[6] - m[9]) / s;
 }
 else if (m[5] > m[10])
 {
   float s = sqrtf(1.0 + m[5] - m[0] - m[10]) * 2.0f;
   x = (m[1] + m[4]) / s;
   y = 0.25f * s;
   z = (m[6] + m[9]) / s;
   w = (m[8] - m[2]) / s;
 }
 else
 {
   float s = sqrtf(1.0 + m[10] - m[0] - m[5]) * 2.0f;
   x = (m[8] + m[2]) / s;
   y = (m[6] + m[9]) / s;
   z = 0.25f * s;
   w = (m[1] - m[4]) / s;
 }
 mW = w;
 mV.set(x, y, z);
}

void BoQuaternion::toRotation(float* angle, BoVector3Float* axis)
{
 // see Q 57 in quat faq on http://www.j3d.org/matrix_faq/matrfaq_latest.html
 if (!angle || !axis)
 {
   return;
 }
 normalize();
 const float cosa = mW;
 *angle = acos(cosa) * 2;
 *angle = Bo3dToolsBase::rad2deg(*angle);
 float sina = (float)sqrt(1.0 - cosa * cosa);
 if (fabsf(sina) < 0.0005)
 {
   sina = 1.0f;
 }
 axis->set(mV.x() / sina, mV.y() / sina, mV.z() / sina);
}

void BoQuaternion::toRotation(float* alpha, float* beta, float* gamma)
{
 if (!alpha || !beta || !gamma)
 {
   return;
 }
 BoMatrix m = matrix();
 m.toRotation(alpha, beta, gamma);
}


void BoQuaternion::transform(BoVector3Float* v, const BoVector3Float* input) const
{
 BoQuaternion q = BoQuaternion(0, *input);
 BoQuaternion tmp = BoQuaternion::multiply(*this, q);
 // we assume this is a unit quaternion, then the inverse is equal to the
 // conjugate
 tmp.multiply(conjugate());
 v->set(tmp.mV);
}

/*
 * vim:et sw=2
 */
