/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef BO3DTOOLS_H
#define BO3DTOOLS_H

#include <GL/gl.h>

class BoVector3
{
  public:
    BoVector3()  { reset(); };
    BoVector3(GLfloat x, GLfloat y, GLfloat z)  { set(x, y, z); };
    ~BoVector3() {};

    inline void set(GLfloat x, GLfloat y, GLfloat z)  { mData[0] = x;  mData[1] = y;  mData[2] = z; };
    inline void reset()  { mData[0] = mData[1] = mData[2] = 0; };
    inline void addScaled(BoVector3 v, GLfloat s)  { mData[0] += v.mData[0] * s;  mData[1] += v.mData[1] * s;  mData[2] += v.mData[2] * s; };
    inline void setScaledSum(BoVector3 a, BoVector3 b,  GLfloat s)
        { mData[0] = a.mData[0] + b.mData[0] * s;   mData[1] = a.mData[1] + b.mData[1] * s;   mData[2] = a.mData[2] + b.mData[2] * s; };

    inline void operator=(BoVector3 v)  { mData[0] = v.mData[0];  mData[1] = v.mData[1];  mData[2] = v.mData[2]; };
    inline void operator=(const GLfloat* v)  { mData[0] = v[0];  mData[1] = v[1];  mData[2] = v[2]; };
    inline GLfloat operator[](int i)  { return mData[i]; };


    GLfloat mData[3];
};

class BoVector4
{
  public:
    BoVector4()  { reset(); };
    BoVector4(GLfloat x, GLfloat y, GLfloat z, GLfloat w)  { set(x, y, z, w); };
    ~BoVector4() {};

    inline void set(GLfloat x, GLfloat y, GLfloat z, GLfloat w)  { mData[0] = x;  mData[1] = y;  mData[2] = z; mData[3] = w; };
    inline void reset()  {  mData[0] = mData[1] = mData[2] = mData[3] = 0; };
    inline void addScaled(BoVector4 v, GLfloat s)
        { mData[0] += v.mData[0] * s;  mData[1] += v.mData[1] * s;  mData[2] += v.mData[2] * s;  mData[3] += v.mData[3] * s; };
    inline void setScaledSum(BoVector4 a, BoVector4 b,  GLfloat s)
        { mData[0] = a.mData[0] + b.mData[0] * s;   mData[1] = a.mData[1] + b.mData[1] * s;   mData[2] = a.mData[2] + b.mData[2] * s;   mData[3] = a.mData[3] + b.mData[3] * s; };

    inline void operator=(BoVector4 v)  { mData[0] = v.mData[0];  mData[1] = v.mData[1];  mData[2] = v.mData[2];  mData[3] = v.mData[3]; };
    inline void operator=(const GLfloat* v)  { mData[0] = v[0];  mData[1] = v[1];  mData[2] = v[2];  mData[3] = v[3]; };
    inline GLfloat operator[](int i)  { return mData[i]; };


    GLfloat mData[4];
};

/**
 * an OpenGL 4x4 matrix. note that we use (just like mesa) column major order to
 * store the matrix elements!
 * 
 * This means that a matrix
 * <pre>
 * A11 A12 A13 A14
 * A21 A22 A23 A24
 * A31 A32 A33 A34
 * A41 A42 A43 A44
 * </pre>
 * Will be stored in memory like this:
 * <pre>
 * A11 A21 A31 A41 A12 A22 A32 A42 A13 A23 A33 A43 A41 A42 A43 A44
 * </pre>
 *
 * Note that this class is not optimized for speed.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
// AB: note that this is a dummy implementation. we need BoMatrix::debugMatrix()
// only right now. one day we might add rotate() translate() and all that stuff
// here, too.
class BoMatrix
{
  public:
    BoMatrix()
    {
      loadIdentity();
    }
    BoMatrix(const GLfloat* m)
    {
      loadMatrix(m);
    }
    ~BoMatrix() { }

    void loadIdentity()
    {
      int i;
      for (i = 0; i < 16; i++) {
        mData[i] = 0.0;
      }
      mData[i] = mData[5] = mData[10] = mData[15] = 1.0;
    }
    void loadMatrix(const GLfloat* m);

    const GLfloat* data() const { return mData; }
    void debugMatrix()
    {
      debugMatrix(data());
    }
    static void debugMatrix(const GLfloat* matrix);

  private:
    GLfloat mData[16];
};

#endif // BO3DTOOLS_H
/*
 * vim:et sw=2
 */
