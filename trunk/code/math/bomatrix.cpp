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

#include "math/bomatrix.h"

#include "math/bo3dtoolsbase.h"

#include <math.h>
#include <stdio.h>

void BoMatrix::loadMatrix(const float* m)
{
 for (int i = 0; i < 16; i++)
 {
   mData[i] = m[i];
 }
}

void BoMatrix::loadMatrix(const BoVector3Float& x, const BoVector3Float& y, const BoVector3Float& z)
{
  setElement(0, 0, x[0]);
  setElement(0, 1, x[1]);
  setElement(0, 2, x[2]);
  setElement(0, 3, 0.0f);

  setElement(1, 0, y[0]);
  setElement(1, 1, y[1]);
  setElement(1, 2, y[2]);
  setElement(1, 3, 0.0f);

  setElement(2, 0, z[0]);
  setElement(2, 1, z[1]);
  setElement(2, 2, z[2]);
  setElement(2, 3, 0.0f);

  setElement(3, 0, 0.0f);
  setElement(3, 1, 0.0f);
  setElement(3, 2, 0.0f);
  setElement(3, 3, 1.0f);
}

void BoMatrix::transform(BoVector3Float* vector, const BoVector3Float* input) const
{
 // v = m * i, m is a 4x4 OpenGL matrix, r and v are both a 3x1 column vector.
 // the forth element is unused in boson and therefore we use silently 0.
#define M(row, col) mData[col * 4 + row] // AB: shamelessy stolen from mesa's  math subdir
#define v(element) vector->mData[element]
#define i(element) input->mData[element]
 v(0) = M(0, 0) * i(0) + M(0, 1) * i(1) + M(0, 2) * i(2) + M(0, 3);
 v(1) = M(1, 0) * i(0) + M(1, 1) * i(1) + M(1, 2) * i(2) + M(1, 3);
 v(2) = M(2, 0) * i(0) + M(2, 1) * i(1) + M(2, 2) * i(2) + M(2, 3);
#undef i
#undef v
#undef M
}

void BoMatrix::transform(BoVector4Float* vector, const BoVector4Float* input) const
{
 // v = m * i, m is a 4x4 OpenGL matrix, r and v are both a 3x1 column vector.
 // the forth element is unused in boson and therefore we use silently 0.
#define M(row, col) mData[col * 4 + row] // AB: shamelessy stolen from mesa's  math subdir
#define v(element) vector->mData[element]
#define i(element) input->mData[element]
 v(0) = M(0, 0) * i(0) + M(0, 1) * i(1) + M(0, 2) * i(2) + M(0, 3);
 v(1) = M(1, 0) * i(0) + M(1, 1) * i(1) + M(1, 2) * i(2) + M(1, 3);
 v(2) = M(2, 0) * i(0) + M(2, 1) * i(1) + M(2, 2) * i(2) + M(2, 3);
 v(3) = M(3, 0) * i(0) + M(3, 1) * i(1) + M(3, 2) * i(2) + M(3, 3);
#undef i
#undef v
#undef M
}

bool BoMatrix::invert(BoMatrix* inverse) const
{
 // shamelessy stolen from mesa/src/math/m_math.c
 // invert_matrix_general

#define SWAP_ROWS(a, b) { float *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(c)*4+(r)]
 const float *m = mData;
 float *out = inverse->mData;
 float wtmp[4][8];
 float m0, m1, m2, m3, s;
 float *r0, *r1, *r2, *r3;

 r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

 r0[0] = MAT(m,0,0), r0[1] = MAT(m,0,1),
 r0[2] = MAT(m,0,2), r0[3] = MAT(m,0,3),
 r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,

 r1[0] = MAT(m,1,0), r1[1] = MAT(m,1,1),
 r1[2] = MAT(m,1,2), r1[3] = MAT(m,1,3),
 r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,

 r2[0] = MAT(m,2,0), r2[1] = MAT(m,2,1),
 r2[2] = MAT(m,2,2), r2[3] = MAT(m,2,3),
 r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,

 r3[0] = MAT(m,3,0), r3[1] = MAT(m,3,1),
 r3[2] = MAT(m,3,2), r3[3] = MAT(m,3,3),
 r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

 /* choose pivot - or die */
 if (fabs(r3[0])>fabs(r2[0])) { SWAP_ROWS(r3, r2); }
 if (fabs(r2[0])>fabs(r1[0])) { SWAP_ROWS(r2, r1); }
 if (fabs(r1[0])>fabs(r0[0])) { SWAP_ROWS(r1, r0); }
 if (0.0 == r0[0])  return false;

 /* eliminate first variable     */
 m1 = r1[0]/r0[0]; m2 = r2[0]/r0[0]; m3 = r3[0]/r0[0];
 s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;
 s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;
 s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;
 s = r0[4];
 if (s != 0.0) { r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s; }
 s = r0[5];
 if (s != 0.0) { r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s; }
 s = r0[6];
 if (s != 0.0) { r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s; }
 s = r0[7];
 if (s != 0.0) { r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s; }

 /* choose pivot - or die */
 if (fabs(r3[1])>fabs(r2[1])) { SWAP_ROWS(r3, r2); }
 if (fabs(r2[1])>fabs(r1[1])) { SWAP_ROWS(r2, r1); }
 if (0.0 == r1[1]) { return false; }

 /* eliminate second variable */
 m2 = r2[1]/r1[1]; m3 = r3[1]/r1[1];
 r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];
 r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];
 s = r1[4]; if (0.0 != s) { r2[4] -= m2 * s; r3[4] -= m3 * s; }
 s = r1[5]; if (0.0 != s) { r2[5] -= m2 * s; r3[5] -= m3 * s; }
 s = r1[6]; if (0.0 != s) { r2[6] -= m2 * s; r3[6] -= m3 * s; }
 s = r1[7]; if (0.0 != s) { r2[7] -= m2 * s; r3[7] -= m3 * s; }

 /* choose pivot - or die */
 if (fabs(r3[2])>fabs(r2[2])) { SWAP_ROWS(r3, r2); }
 if (0.0 == r2[2]) { return false; }

 /* eliminate third variable */
 m3 = r3[2]/r2[2];
 r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
 r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6],
 r3[7] -= m3 * r2[7];

 /* last check */
 if (0.0 == r3[3]) { return false; }

 s = 1.0F/r3[3];             /* now back substitute row 3 */
 r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;

 m2 = r2[3];                 /* now back substitute row 2 */
 s  = 1.0F/r2[2];
 r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
 r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
 m1 = r1[3];
 r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
 r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
 m0 = r0[3];
 r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
 r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

 m1 = r1[2];                 /* now back substitute row 1 */
 s  = 1.0F/r1[1];
 r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
 r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
 m0 = r0[2];
 r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
 r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

 m0 = r0[1];                 /* now back substitute row 0 */
 s  = 1.0F/r0[0];
 r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
 r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

 MAT(out,0,0) = r0[4]; MAT(out,0,1) = r0[5],
 MAT(out,0,2) = r0[6]; MAT(out,0,3) = r0[7],
 MAT(out,1,0) = r1[4]; MAT(out,1,1) = r1[5],
 MAT(out,1,2) = r1[6]; MAT(out,1,3) = r1[7],
 MAT(out,2,0) = r2[4]; MAT(out,2,1) = r2[5],
 MAT(out,2,2) = r2[6]; MAT(out,2,3) = r2[7],
 MAT(out,3,0) = r3[4]; MAT(out,3,1) = r3[5],
 MAT(out,3,2) = r3[6]; MAT(out,3,3) = r3[7];

 return true;

#undef MAT
#undef SWAP_ROWS
}

void BoMatrix::translate(float x, float y, float z)
{
 // shamelessy stolen from mesa/src/math/m_math.c
 mData[12] = mData[0] * x + mData[4] * y + mData[8]  * z + mData[12];
 mData[13] = mData[1] * x + mData[5] * y + mData[9]  * z + mData[13];
 mData[14] = mData[2] * x + mData[6] * y + mData[10] * z + mData[14];
 mData[15] = mData[3] * x + mData[7] * y + mData[11] * z + mData[15];
}

void BoMatrix::scale(float x, float y, float z)
{
 // shamelessy stolen from mesa/src/math/m_math.c
 mData[0] *= x;   mData[4] *= y;   mData[8]  *= z;
 mData[1] *= x;   mData[5] *= y;   mData[9]  *= z;
 mData[2] *= x;   mData[6] *= y;   mData[10] *= z;
 mData[3] *= x;   mData[7] *= y;   mData[11] *= z;
}

void BoMatrix::multiply(const float* mat)
{
 // shamelessy stolen from mesa/src/math/m_math.c
 // we use matmul4() from mesa only, not matmul34(). this means we are slower
 // than mesa! (and also less complex).
 // AB: this function multiplies mData by mat and places the result into mData.
#define B(row,col)  mat[indexAt(row, col)]
 int i;
 for (i = 0; i < 4; i++)
 {
   const float ai0=element(i,0),  ai1=element(i,1),  ai2=element(i,2),  ai3=element(i,3);
   mData[indexAt(i, 0)] = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
   mData[indexAt(i, 1)] = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
   mData[indexAt(i, 2)] = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
   mData[indexAt(i, 3)] = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
 }
#undef B
}

void BoMatrix::rotate(float angle, float x, float y, float z)
{
 // shamelessy stolen from mesa/src/math/m_math.c
 float mag, s, c;
 float xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;
 float m[16];

 s = (float) sin( Bo3dToolsBase::deg2rad(angle) );
 c = (float) cos( Bo3dToolsBase::deg2rad(angle) );

 mag = (float) sqrt( x*x + y*y + z*z ); // AB: mesa uses GL_SQRT here

 if (mag <= 1.0e-4)
 {
   // generate an identity matrix and return
   loadIdentity();
   return;
 }

 x /= mag;
 y /= mag;
 z /= mag;

#define M(row,col)  m[col*4+row]

 xx = x * x;
 yy = y * y;
 zz = z * z;
 xy = x * y;
 yz = y * z;
 zx = z * x;
 xs = x * s;
 ys = y * s;
 zs = z * s;
 one_c = 1.0F - c;

 M(0,0) = (one_c * xx) + c;
 M(0,1) = (one_c * xy) - zs;
 M(0,2) = (one_c * zx) + ys;
 M(0,3) = 0.0F;

 M(1,0) = (one_c * xy) + zs;
 M(1,1) = (one_c * yy) + c;
 M(1,2) = (one_c * yz) - xs;
 M(1,3) = 0.0F;

 M(2,0) = (one_c * zx) - ys;
 M(2,1) = (one_c * yz) + xs;
 M(2,2) = (one_c * zz) + c;
 M(2,3) = 0.0F;

 M(3,0) = 0.0F;
 M(3,1) = 0.0F;
 M(3,2) = 0.0F;
 M(3,3) = 1.0F;

#undef M

 multiply(m);

}

void BoMatrix::setLookAtRotation(const BoVector3Float& cameraPos, const BoVector3Float& lookAt, const BoVector3Float& up)
{
  BoVector3Float z = cameraPos - lookAt;
  z.normalize();
  BoVector3Float x = BoVector3<float>::crossProduct(up, z);
  BoVector3Float y = BoVector3<float>::crossProduct(z, x);
  x.normalize();
  y.normalize();
  loadMatrix(x, y, z);
}

bool BoMatrix::isEqual(const BoMatrix& matrix, float diff) const
{
  for (int i = 0; i < 16; i++)
  {
    if (fabsf(mData[i] - matrix.mData[i]) > diff)
    {
      return false;
    }
  }
  return true;
}

void BoMatrix::debugMatrix(const float* m)
{
 for (int i = 0; i < 4; i++)
 {
   printf("%f %f %f %f\n", m[i], m[i + 4], m[i + 8], m[i + 12]);
 }
}

void BoMatrix::toRotation(float* alpha, float* beta, float* gamma)
{
 // see also docs/glrotate.lyx
 if (!alpha || !beta || !gamma)
 {
   return;
 }
 // AB: asin() is not unique. See docs/glrotate.lyx on why we can use this
 // anyway.
 *beta = asin(mData[8]);

 float cosb = cos(*beta);
 if (fabsf(cosb) > 0.0001)
 {
   float cosa = mData[10] / cosb;
   float sina = -mData[9] / cosb;
   *alpha = atan2(sina, cosa);

   float cosc = mData[0] / cosb;
   float sinc = -mData[4] / cosb;
   *gamma = atan2(sinc, cosc);
 }
 else
 {
   *alpha = 0.0f;

   float sinc = mData[1];
   float cosc = mData[5];

   *gamma = atan2(sinc, cosc);
 }

 *alpha = Bo3dToolsBase::rad2deg(*alpha);
 *beta = Bo3dToolsBase::rad2deg(*beta);
 *gamma = Bo3dToolsBase::rad2deg(*gamma);
}

void BoMatrix::toGluLookAt(BoVector3Float* lookAt, BoVector3Float* up, const BoVector3Float& cameraPos) const
{
 BoVector3Float x, z;
 x.setX(element(0, 0));
 x.setY(element(0, 1));
 x.setZ(element(0, 2));
 z.setX(element(2, 0));
 z.setY(element(2, 1));
 z.setZ(element(2, 2));

 *lookAt = cameraPos - z;
 extractUp(*up, x, z);
}

void BoMatrix::extractUp(BoVector3Float& up, const BoVector3Float& x, const BoVector3Float& z) const
{
// keep these formulas in mind:
// (you can get them from x := up cross z , (we assume that no normalizing necessary!)
// up[2] = (x[1] + (x[0] * z[0]) / z[1] + (x[2] * z[2] / z[1])) / (z[0] + z[1]);
// up[1] = (x[0] + up[2] * z[1]) / z[2];
// up[0] = ((x[0] + up[2] * z[1] * z[0]) / z[2] + x[2]) / z[1];

 // AB: note that every component of z can actually become zero. i tested all three.
 if (fabsf(z[1]) <= 0.0001f) {
	// we can use
	// x[0] := up[1] * z[2] - up[2] * z[1] => x[0] = up[1] * z[2]
	// ==> up[1] = x[0] / z[2]
	// or
	// x[2] := up[0] * z[1] - up[1] * z[0] => x[2] = -up[1] * z[0]
	// ==> up[1] = x[2] / z[0]
	if (fabsf(z[0]) <= 0.0001f && fabsf(z[2]) <= 0.0001f) {
		// is this possible? where to fall back to?
		printf("oops - x[0] != 0, x[2] != 0, z[0] == z[2] == 0  !\n");
		up.setY(0.0f);
	} else if (fabs(z[2]) > 0.0001f) {
		up.setY(x[0] / z[2]);
	} else { // fabs(z[0]) > 0.0001
		up.setY(-x[2] / z[0]);
	}

	// only one equation for two variables left :-(
	// x[1] := up[2] * z[0] - up[0] * z[2];
	if (fabsf(z[2]) <= 0.0001f && fabs(z[0]) <= 0.0001f) {
		// all of z are zero. this is probably invalid anyway.
		// AB: to be proven.
		up.setX(0.0f);
		up.setX(0.0f);
	} else if (fabsf(z[2]) <= 0.0001f) {
		up.setZ(x[1] / z[0]);
		// up[0] doesn't influence the matrix anyway
		up.setX(0.0f);
	} else if (fabsf(z[0]) <= 0.0001f) {
		up.setX(-x[1] / z[2]);
		// up[2] doesn't influence the matrix anyway
		up.setZ(0.0f);
	} else {
		// multiple solutions possible.
		up.setX(0.0f);
		up.setZ(x[1] / z[0]);
	}
	return;
 } else if (fabsf(z[2]) <= 0.0001f) {
	// here we can assume that z[1] != 0, as we already checked above

	// we can use
	// x[0] := up[1] * z[2] - up[2] * z[1] => x[0] = -up[2] * z[1]
	// ==> up[2] = -x[0] / z[1]
	// or
	// x[1] := up[2] * z[0] - up[0] * z[2] => x[1] = up[2] * z[0]
	// ==> up[2] = x[1] / z[0]

	// we use the first, as we already know that z[1] != 0
	up.setZ(-x[0] / z[1]);

	// one equation left:
	// x[2] := up[0] * z[1] - up[1] * z[0]
	if (fabsf(z[0]) <= 0.0001f) {
		up.setX(x[2] / z[1]);
		// up[1] does't influence the matrix anyway
		up.setY(0.0f);
	} else {
		// multiple solutions possible
		up.setY(0.0f);
		up.setX(x[2] / z[1]);
	}
	return;
 } else if (fabs(z[0]) <= 0.0001f) {
	// here we can assume that z[1] != 0 and z[2] != 0

	// we can use
	// x[1] := up[2] * z[0] - up[0] * z[2] => x[1] = -up[0] * z[2]
	// ==> up[0] = -x[1] / z[2]
	// or
	// x[2] := up[0] * z[1] - up[1] * z[0] => x[2] = up[0] * z[1]
	// ==> up[0] = x[2] / z[1]

	up.setX(x[2] / z[1]);

	// one equation left:
	// x[0] := up[1] * z[2] - up[2] * z[1]
	// multiple solutions possible, as z[1] and z[2] are both != 0

	up.setZ(0.0f);
	up.setY(x[0] / z[2]);
	return;
 }

 double foo1;
 double foo2;
 double foo3;

 foo3 = 0;

 // this code depends on z[0] != 0, z[1] != 0 and z[2] != 0 !
 foo1 = (x[2] * z[2]) / (2 * z[1] * z[0]);
 foo2 = x[0] / (2 * z[1]);

 up.setZ(foo1 - foo2);

 foo1 = x[0] + up.z() * z[1];
 up.setY(foo1 / z[2]);

 up.setX((up.y() * z[0] + x[2]) / z[1]);

}

/*
 * vim:et sw=2
 */
