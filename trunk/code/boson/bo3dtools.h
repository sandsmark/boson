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

/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoVector3
{
  public:
    BoVector3()  { reset(); };
    BoVector3(GLfloat x, GLfloat y, GLfloat z)  { set(x, y, z); };
    ~BoVector3() {};

    inline void reset()  { mData[0] = mData[1] = mData[2] = 0; };

    inline void set(GLfloat x, GLfloat y, GLfloat z)  { mData[0] = x;  mData[1] = y;  mData[2] = z; };
    inline void setX(GLfloat x) { mData[0] = x; }
    inline void setY(GLfloat y) { mData[1] = y; }
    inline void setZ(GLfloat z) { mData[2] = z; }

    inline void addScaled(BoVector3 v, GLfloat s)  { mData[0] += v.mData[0] * s;  mData[1] += v.mData[1] * s;  mData[2] += v.mData[2] * s; };
    inline void setScaledSum(BoVector3 a, BoVector3 b,  GLfloat s)
        { mData[0] = a.mData[0] + b.mData[0] * s;   mData[1] = a.mData[1] + b.mData[1] * s;   mData[2] = a.mData[2] + b.mData[2] * s; };
    inline void add(BoVector3 v)  { mData[0] += v.mData[0]; mData[1] += v.mData[1]; mData[2] += v.mData[2]; };
    inline void normalize()  { scale(1 / length()); };
    inline void scale(float s)  {mData[0] = mData[0] * s;  mData[1] = mData[1] * s;  mData[2] = mData[2] * s; };
    float length();

    inline GLfloat* data() { return mData; }
    inline void operator=(BoVector3 v)  { mData[0] = v.mData[0];  mData[1] = v.mData[1];  mData[2] = v.mData[2]; };
    inline void operator=(const GLfloat* v)  { mData[0] = v[0];  mData[1] = v[1];  mData[2] = v[2]; };
    inline GLfloat operator[](int i)  { return mData[i]; };


  private:
    friend class BoMatrix;
    GLfloat mData[3];
};

/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoVector4
{
  public:
    BoVector4()  { reset(); };
    BoVector4(GLfloat x, GLfloat y, GLfloat z, GLfloat w)  { set(x, y, z, w); };
    ~BoVector4() {};

    inline void reset()  {  mData[0] = mData[1] = mData[2] = mData[3] = 0; };

    inline void set(GLfloat x, GLfloat y, GLfloat z, GLfloat w)  { mData[0] = x;  mData[1] = y;  mData[2] = z; mData[3] = w; };
    inline void setX(GLfloat x) { mData[0] = x; }
    inline void setY(GLfloat y) { mData[1] = y; }
    inline void setZ(GLfloat z) { mData[2] = z; }
    inline void setW(GLfloat w) { mData[3] = w; }

    inline void addScaled(BoVector4 v, GLfloat s)
        { mData[0] += v.mData[0] * s;  mData[1] += v.mData[1] * s;  mData[2] += v.mData[2] * s;  mData[3] += v.mData[3] * s; };
    inline void setScaledSum(BoVector4 a, BoVector4 b,  GLfloat s)
        { mData[0] = a.mData[0] + b.mData[0] * s;   mData[1] = a.mData[1] + b.mData[1] * s;   mData[2] = a.mData[2] + b.mData[2] * s;   mData[3] = a.mData[3] + b.mData[3] * s; };
    inline void setBlended(BoVector4 a, float af, BoVector4 b, float bf)
        { mData[0] = a.mData[0] * af + b.mData[0] * bf;   mData[1] = a.mData[1] * af + b.mData[1] * bf;
        mData[2] = a.mData[2] * af + b.mData[2] * bf;   mData[3] = a.mData[3] * af + b.mData[3] * af; };
    inline void add(BoVector4 v)  { mData[0] += v.mData[0]; mData[1] += v.mData[1]; mData[2] += v.mData[2]; mData[3] += v.mData[3]; };

    inline GLfloat* data() { return mData; }
    inline void operator=(BoVector4 v)  { mData[0] = v.mData[0];  mData[1] = v.mData[1];  mData[2] = v.mData[2];  mData[3] = v.mData[3]; };
    inline void operator=(const GLfloat* v)  { mData[0] = v[0];  mData[1] = v[1];  mData[2] = v[2];  mData[3] = v[3]; };
    inline GLfloat operator[](int i)  { return mData[i]; };

  private:
    friend class BoMatrix;
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
    BoMatrix(GLenum matrix)
    {
      loadMatrix(matrix);
    }
    ~BoMatrix() { }

    void loadIdentity()
    {
      int i;
      for (i = 0; i < 16; i++) {
        mData[i] = 0.0;
      }
      mData[0] = mData[5] = mData[10] = mData[15] = 1.0;
    }

    /**
     * Load the specified OpenGL matrix.
     * @param GL_MODELVIEW_MATRIX, GL_PROJECTION_MATRIX or GL_TEXTURE_MATRIX.
     * Note that all other values (also e.g. GL_TEXTURE) will result in the
     * identity matrix and generate an error
     **/
    void loadMatrix(GLenum matrix);
    void loadMatrix(const GLfloat* m);

    const GLfloat* data() const { return mData; }
    void debugMatrix()
    {
      debugMatrix(data());
    }

    /**
     * @return TRUE if <em>all</em> elements of this matrix are 0. Otherwise
     * FALSE.
     **/
    bool isNull() const
    {
      for (int i = 0; i < 16; i++) {
        if (mData[i] != 0.0) {
          return false;
        }
      }
      return true;
    }
    /**
     * @return TRUE if this is the identity matrix, otherwise FALSE.
     **/
    bool isIdentity() const
    {
      for (int i = 0; i < 16; i++) {
        if (mData[i] != 0.0) {
          if (mData[i] != 1.0 || i % 5 != 0) {
            return false;
          }
        }
      }
      return true;
    }

    /**
     * Transform the vector @p input according to this matrix and put the result
     * into @p v.
     *
     * This calculates simply does v = M * input, where M is this matrix.
     **/
    void transform(BoVector3* v, BoVector3* input);
    void transform(BoVector4* v, BoVector4* input);

    /**
     * Invert this matrix and place the result into @p inverse
     **/
    bool invert(BoMatrix* inverse) const;

    static void debugMatrix(const GLfloat* matrix);

  private:
    GLfloat mData[16];
};

#endif // BO3DTOOLS_H
/*
 * vim:et sw=2
 */
