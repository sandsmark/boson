/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bo3dtools.h"
#include "bodebug.h"

#include <qstring.h>
#include <qptrlist.h>
#include <qdatastream.h>

#include <math.h>

#include <lib3ds/mesh.h>

// Degrees to radians conversion (AB: from mesa/src/macros.h)
#define DEG2RAD (M_PI/180.0)
// And radians to degrees conversion
#define RAD2DEG (180.0/M_PI)


/*****  BoVector*  *****/

float BoVector3::length() const
{
  return sqrt(dotProduct());
}

BoVector3 BoVector3::crossProduct(const BoVector3& v, const BoVector3& w)
{
  BoVector3 r;
  r.setX((v.y() * w.z()) - (v.z() * w.y()));
  r.setY((v.z() * w.x()) - (v.x() * w.z()));
  r.setZ((v.x() * w.y()) - (v.y() * w.x()));
  return r;
}

void BoVector3::makeVectors(BoVector3* v, const Lib3dsMesh* mesh, const Lib3dsFace* face)
{
  // Lib3dsFace stores only the position (index) of the
  // actual point. the actual points are in mesh->pointL
  v[0].set(mesh->pointL[ face->points[0] ].pos);
  v[1].set(mesh->pointL[ face->points[1] ].pos);
  v[2].set(mesh->pointL[ face->points[2] ].pos);
}

bool BoVector3::isAdjacent(const BoVector3* v1, const BoVector3* v2)
{
  if (!v1 || !v2)
  {
    return false;
  }
  int equal = 0;
  for (int i = 0; i < 3; i++)
  {
    if (v1[i].isEqual(v2[0]) || v1[i].isEqual(v2[1]) || v1[i].isEqual(v2[2]))
    {
      equal++;
    }
  }

  // v1 is adjacent to v2 if at least 2 points are equal.
  // equal vectors (i.e. all points are equal) are possible, too.
  return (equal >= 2);
}

int BoVector3::findPoint(const BoVector3& point, const BoVector3* array)
{
  for (int i = 0; i < 3; i++)
  {
    if (array[i].isEqual(point))
    {
      return i;
    }
  }
  return -1;
}

QString BoVector3::debugString(const BoVector3& v)
{
  return QString("%1,%2,%3").arg(v.x()).arg(v.y()).arg(v.z());
}

QString BoVector3::debugString() const
{
  return BoVector3::debugString(*this);
}

void BoVector3::debugVector(const BoVector3& v)
{
  boDebug() << "vector: " << debugString(v) << endl;
}

QDataStream& operator<<(QDataStream& s, const BoVector3& v)
{
  return s << (float)v.mData[0] << (float)v.mData[1] << (float)v.mData[2];
}

QDataStream& operator>>(QDataStream& s, BoVector3& v)
{
  float x, y, z;
  s >> x >> y >> z;
  v.mData[0] = x;
  v.mData[1] = y;
  v.mData[2] = z;
  return s;
}

QString BoVector4::debugString(const BoVector4& v)
{
  return QString("%1,%2,%3,%4").arg(v.x()).arg(v.y()).arg(v.z()).arg(v.w());
}

void BoVector4::debugVector(const BoVector4& v)
{
  boDebug() << "vector: " << debugString(v) << endl;
}

QString BoVector4::debugString() const
{
  return BoVector4::debugString(*this);
}



/*****  BoMatrix  *****/

void BoMatrix::loadMatrix(const GLfloat* m)
{
 for (int i = 0; i < 16; i++)
 {
   mData[i] = m[i];
 }
}

void BoMatrix::loadMatrix(GLenum matrix)
{
 switch (matrix)
 {
   case GL_MODELVIEW_MATRIX:
   case GL_PROJECTION_MATRIX:
   case GL_TEXTURE_MATRIX:
     break;
   default:
     boError() << k_funcinfo << "Invalid matrix enum " << (int)matrix << endl;
 }
 glGetFloatv(matrix, mData);
}

void BoMatrix::transform(BoVector3* vector, const BoVector3* input) const
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

void BoMatrix::transform(BoVector4* vector, const BoVector4* input) const
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

#define SWAP_ROWS(a, b) { GLfloat *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(c)*4+(r)]
 const GLfloat *m = mData;
 GLfloat *out = inverse->mData;
 GLfloat wtmp[4][8];
 GLfloat m0, m1, m2, m3, s;
 GLfloat *r0, *r1, *r2, *r3;

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

void BoMatrix::translate(GLfloat x, GLfloat y, GLfloat z)
{
 // shamelessy stolen from mesa/src/math/m_math.c
 mData[12] = mData[0] * x + mData[4] * y + mData[8]  * z + mData[12];
 mData[13] = mData[1] * x + mData[5] * y + mData[9]  * z + mData[13];
 mData[14] = mData[2] * x + mData[6] * y + mData[10] * z + mData[14];
 mData[15] = mData[3] * x + mData[7] * y + mData[11] * z + mData[15];
}

void BoMatrix::scale(GLfloat x, GLfloat y, GLfloat z)
{
 // shamelessy stolen from mesa/src/math/m_math.c
 mData[0] *= x;   mData[4] *= y;   mData[8]  *= z;
 mData[1] *= x;   mData[5] *= y;   mData[9]  *= z;
 mData[2] *= x;   mData[6] *= y;   mData[10] *= z;
 mData[3] *= x;   mData[7] *= y;   mData[11] *= z;
}

void BoMatrix::multiply(const GLfloat* mat)
{
 // shamelessy stolen from mesa/src/math/m_math.c
 // we use matmul4() from mesa only, not matmul34(). this means we are slower
 // than mesa! (and also less complex).
 // AB: this function multiplies mData by mat and places the result into mData.
#define B(row,col)  mat[indexAt(row, col)]
 GLint i;
 for (i = 0; i < 4; i++)
 {
   const GLfloat ai0=element(i,0),  ai1=element(i,1),  ai2=element(i,2),  ai3=element(i,3);
   mData[indexAt(i, 0)] = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
   mData[indexAt(i, 1)] = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
   mData[indexAt(i, 2)] = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
   mData[indexAt(i, 3)] = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
 }
#undef B
}

void BoMatrix::rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
 // shamelessy stolen from mesa/src/math/m_math.c
 GLfloat mag, s, c;
 GLfloat xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;
 GLfloat m[16];

 s = (GLfloat) sin( angle * DEG2RAD );
 c = (GLfloat) cos( angle * DEG2RAD );

 mag = (GLfloat) sqrt( x*x + y*y + z*z ); // AB: mesa uses GL_SQRT here

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

bool BoMatrix::isEqual(const BoMatrix& matrix) const
{
  for (int i = 0; i < 16; i++)
  {
    if (mData[i] != matrix.mData[i])
    {
      return false;
    }
  }
  return true;
}

void BoMatrix::debugMatrix(const GLfloat* m)
{
 boDebug() << k_funcinfo << endl;
 for (int i = 0; i < 4; i++)
 {
   boDebug() << QString("%1 %2 %3 %4").arg(m[i]).arg(m[i + 4]).arg(m[i + 8]).arg(m[i + 12]) << endl;
 }
 boDebug() << k_funcinfo << "done" << endl;
}


/*****  Misc methods  *****/

float Bo3dTools::rotationToPoint(float x, float y)
{
  float add = 0;
  float arg = 0;
  if(x > 0)
  {
    if(y < 0)
    {
      add = 0;
      arg = x / -y;
    }
    else
    {
      add = 90;
      arg = y / x;
    }
  }
  else
  {
    if(y > 0)
    {
      add = 180;
      arg = -x / y;
    }
    else if(x < 0)
    {
      add = 270;
      arg = -y / -x;
    }
    else
    {
      return 0;
    }
  }

  return (atan(arg) * RAD2DEG) + add;
}

void Bo3dTools::pointByRotation(float* x, float* y, const float angle, const float radius)
{
  // Some quick tests
  if(angle == 0)
  {
    *x = 0;
    *y = -radius;
    return;
  }
  else if(angle == 90)
  {
    *x = radius;
    *y = 0;
    return;
  }
  else if(angle == 180)
  {
    *x = 0;
    *y = radius;
    return;
  }
  else if(angle == 270)
  {
    *x = -radius;
    *y = 0;
    return;
  }
  double tmpx, tmpy;
  tmpy = 1.0;
  tmpx = tan(angle / RAD2DEG);
  double length = sqrt(tmpx * tmpx + tmpy * tmpy);
  tmpx = tmpx / length * radius;
  tmpy = tmpy / length * radius;
  if(angle < 90)
  {
    *x = tmpx;
    *y = -tmpy;
  }
  else if(angle < 180)
  {
    *x = -tmpx;
    *y = tmpy;
  }
  else if(angle < 270)
  {
    *x = -tmpx;
    *y = tmpy;
  }
  else
  {
    *x = tmpx;
    *y = -tmpy;
  }
}

float Bo3dTools::deg2rad(float deg)
{
  return deg * DEG2RAD;
}

float Bo3dTools::rad2deg(float rad)
{
  return rad * RAD2DEG;
}

float Bo3dTools::sphereInFrustum(const double* viewFrustum, const BoVector3& pos, float radius)
{
  // FIXME: performance: we might unroll the loop and then make this function
  // inline. We call it pretty often!
  float distance;
  for(int p = 0; p < 6; p++)
  {
    distance = viewFrustum[p * 4 + 0] * pos[0] + viewFrustum[p * 4 + 1] * pos[1] +
        viewFrustum[p * 4 + 2] * pos[2] + viewFrustum[p * 4 + 3];
    if(distance <= -radius)
    {
      return 0;
    }
  }
  return distance + radius;
}

bool Bo3dTools::boProject(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, GLfloat x, GLfloat y, GLfloat z, QPoint* pos)
{
  // AB: once again - most credits go to mesa :)
  BoVector4 v;
  v.setX(x);
  v.setY(y);
  v.setZ(z);
  v.setW(1.0f);

  BoVector4 v2;
  modelviewMatrix.transform(&v2, &v);
  projectionMatrix.transform(&v, &v2);

  if(v[3] == 0.0f)
  {
    boError() << k_funcinfo << "Can't divide by zero" << endl;
    return false;
  }
  v2.setX(v[0] / v[3]);
  v2.setY(v[1] / v[3]);
  v2.setZ(v[2] / v[3]);

  pos->setX((int)(viewport[0] + (1 + v2[0]) * viewport[2] / 2));
  pos->setY((int)(viewport[1] + (1 + v2[1]) * viewport[3] / 2));

  // return the actual window y
  pos->setY(viewport[3] - pos->y());
  return true;
}

bool Bo3dTools::boUnProject(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, const QPoint& pos, BoVector3* ret, float z)
{
  // AB: most code is from mesa's gluUnProject().
  BoMatrix A(projectionMatrix);
  BoMatrix B;

  // A = A x Modelview (== Projection x Modelview)
  A.multiply(&modelviewMatrix);

  // B = A^(-1)
  if(!A.invert(&B))
  {
    boError() << k_funcinfo << "Could not invert (Projection x Modelview)" << endl;
    return false;
  }

  // AB: we could calculate the inverse whenever camera changes!
  // --> less inverses to be calculated.

  GLfloat depth = 0.0f;
  GLint realy = viewport[3] - (GLint)pos.y() - 1;
  if (z == -1.0f)
  {
    glReadPixels(pos.x(), realy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
  }
  else
  {
    depth = z;
  }


  BoVector4 v;
  BoVector4 result;
  v.setX( ((GLfloat)((pos.x() - viewport[0]) * 2)) / viewport[2] - 1.0f);
  v.setY( ((GLfloat)((realy - viewport[1]) * 2)) / viewport[3] - 1.0f);
#if 0
  // mesa uses this
  v.setX( (pos.x() - viewport[0]) * 2 / viewport[2] - 1.0f);
  v.setY( (realy - viewport[1]) * 2 / viewport[3] - 1.0f);
#endif
  v.setZ(2 * depth - 1.0f);
  v.setW(1.0f);
  B.transform(&result, &v);

  if(result[3] == 0.0f)
  {
    boError() << k_funcinfo << "Can't divide by zero" << endl;
    return false;
  }

  ret->set(result[0] / result[3], result[1] / result[3], result[2] / result[3]);

  return true;
}

bool Bo3dTools::mapCoordinates(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, const QPoint& pos, GLfloat* posX, GLfloat* posY, GLfloat* posZ, bool useRealDepth)
{
  GLint realy = viewport[3] - (GLint)pos.y() - 1;
  // we basically calculate a line here .. nearX/Y/Z is the starting point,
  // farX/Y/Z is the end point. From these points we can calculate a direction.
  // using this direction and the points nearX(Y)/farX(Y) you can build triangles
  // and then find the point that is on z=0.0
  GLdouble nearX, nearY, nearZ;
  GLdouble farX, farY, farZ;
  BoVector3 near, far;
  if(!boUnProject(modelviewMatrix, projectionMatrix, viewport, pos, &near, 0.0f))
  {
    return false;
  }
  if(!boUnProject(modelviewMatrix, projectionMatrix, viewport, pos, &far, 1.0f))
  {
    return false;
  }
  nearX = near[0];
  nearY = near[1];
  nearZ = near[2];
  farX = far[0];
  farY = far[1];
  farZ = far[2];

  GLdouble zAtPoint = 0.0f;

  // we need to find out which z position is at the point pos. this is important
  // for mapping 2d values (screen coordinates) to 3d (world coordinates)
  GLfloat depth = 0.0;
  glReadPixels(pos.x(), realy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

  // AB: 0.0f is reached when we have a point that is outside the actual window!
  if(useRealDepth && depth != 1.0f && depth != 0.0f) {
    // retrieve z
    BoVector3 v;
    if(!boUnProject(modelviewMatrix, projectionMatrix, viewport, pos, &v))
    {
      return false;
    }
    zAtPoint = v[2];
  }
  else
  {
    // assume we're using z = 0.0f
    zAtPoint = 0.0f;
  }

  // simple maths .. however it took me pretty much time to do this.. I haven't
  // done this for way too long time!
  GLdouble dist = (nearZ - zAtPoint); // distance from nearZ to our actual z. for z=0.0 this is equal to nearZ.
  GLdouble tanAlphaX = (nearX - farX) / (nearZ - farZ);
  *posX = (GLfloat)(nearX - tanAlphaX * dist);

  GLdouble tanAlphaY = (nearY - farY) / (nearZ - farZ);
  *posY = (GLfloat)(nearY - tanAlphaY * dist);

  *posZ = zAtPoint;
  return true;
}

bool Bo3dTools::mapDistance(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, int windx, int windy, GLfloat* dx, GLfloat* dy)
{
  GLfloat moveZ; // unused
  GLfloat moveX1, moveY1;
  GLfloat moveX2, moveY2;
  if(windx >= viewport[2]) // viewport[2] == width
  {
    boError() << k_funcinfo << "windx (" << windx <<") must be < " << viewport[2] << endl;
    return false;
  }
  if(windy >= viewport[3]) // viewport[3] == height
  {
    boError() << k_funcinfo << "windy (" << windy <<") must be < " << viewport[3] << endl;
    return false;
  }
  if(!mapCoordinates(modelviewMatrix, projectionMatrix, viewport,
              QPoint(viewport[2] / 2 - windx / 2, viewport[3] / 2 - windy / 2),
              &moveX1, &moveY1, &moveZ, false))
  {
    boError() << k_funcinfo << "Cannot map coordinates" << endl;
    return false;
  }
  if(!mapCoordinates(modelviewMatrix, projectionMatrix, viewport,
              QPoint(viewport[2] / 2 + windx / 2, viewport[3] / 2 + windy / 2),
              &moveX2, &moveY2, &moveZ, false))
  {
    boError() << k_funcinfo << "Cannot map coordinates" << endl;
    return false;
  }
  *dx = moveX2 - moveX1;
  *dy = moveY2 - moveY1;
  return true;
}


/*
 * vim:et sw=2
 */
