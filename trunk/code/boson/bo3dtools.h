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

#ifndef BO3DTOOLS_H
#define BO3DTOOLS_H

#include <GL/gl.h>

#include <lib3ds/types.h>

#include "defines.h"

class QString;
class KConfig;
class QDataStream;
class QPoint;


/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoVector3
{
  public:
    BoVector3()  { reset(); }
    BoVector3(GLfloat x, GLfloat y, GLfloat z)  { set(x, y, z); }
    BoVector3(const GLfloat* data) { set(data[0], data[1], data[2]); }
    ~BoVector3() {}

    /**
     * Make this vector a null vector.
     **/
    inline void reset()  { mData[0] = mData[1] = mData[2] = 0.0f; }

    /**
     * @return The first (x) coordinate of the vector.
     **/
    inline GLfloat x() const { return mData[0]; }
    /**
     * @return The second (y) coordinate of the vector.
     **/
    inline GLfloat y() const { return mData[1]; }
    /**
     * @return The third (z) coordinate of the vector.
     **/
    inline GLfloat z() const { return mData[2]; }

    /**
     * Assign the values @p x, @p y, @p z to the vector.
     **/
    inline void set(GLfloat x, GLfloat y, GLfloat z)
    {
      mData[0] = x;  mData[1] = y;  mData[2] = z; 
    }
    /**
     * @overload
     **/
    inline void set(const BoVector3& v) { set(v.data()); }
    /**
     * @overload
     **/
    inline void set(const float* v) { set(v[0], v[1], v[2]); }

    /**
     * Assign the x coordinate to the vector.
     **/
    inline void setX(GLfloat x) { mData[0] = x; }
    /**
     * Assign the y coordinate to the vector.
     **/
    inline void setY(GLfloat y) { mData[1] = y; }
    /**
     * Assign the z coordinate to the vector.
     **/
    inline void setZ(GLfloat z) { mData[2] = z; }

    /**
     * Scale @p v by s and then add it to this vector.
     **/
    inline void addScaled(const BoVector3& v, GLfloat s)
    {
      mData[0] += v.mData[0] * s;  mData[1] += v.mData[1] * s;  mData[2] += v.mData[2] * s;
    }

#if 0
    // AB: this function is NOT used all over boson's code and I find it very
    // confusing. The name looks like it would scale both vertices and add the
    // result together, but it scales b only.
    // So I am removing this code for now - do we actually need it? I don't
    // think so!
    /**
     * Scale the vector @p b by @p s, add the vector @p a to the result and
     * assign the result to this vector.
     **/
    inline void setScaledSum(const BoVector3& a, const BoVector3& b,  GLfloat s)
    {
      mData[0] = a.mData[0] + b.mData[0] * s;   mData[1] = a.mData[1] + b.mData[1] * s;   mData[2] = a.mData[2] + b.mData[2] * s;
    }
#endif

    /**
     * Add @p v to this vector.
     **/
    inline void add(const BoVector3& v)
    {
      mData[0] += v.mData[0]; mData[1] += v.mData[1]; mData[2] += v.mData[2];
    }

    /**
     * Normalize this vector.
     *
     * Normalizing a vector means to make it a so-called "unit-vector", that is
     * a vector with @ref length 1.
     *
     * Practically this means dividing all elements in the vector by the @ref
     * length of the vector.
     **/
    inline void normalize()
    {
      float l = length();
      if (l != 0.0f) {
        scale(1.0f / l);
      }
    }

    /**
     * Scale the vector by @p s. This is just scalar multiplication, i.e. all
     * elements/coordinates of the vector are multiplied by @p s.
     **/
    inline void scale(float s)
    {
      mData[0] = mData[0] * s;  mData[1] = mData[1] * s;  mData[2] = mData[2] * s;
    }


    //AB: this calls sqrt() and therefore is slow!
    /**
     * @return The length (aka magnitude) of the vector.
     *
     * The length of a vector v is defined as sqrt(v[0]^2 + v[1]^2 + v[2]^2) (in
     * case of 3d).
     *
     * Notice that this function actually uses sqrt(), so it is slow in
     * situations where you use it often!
     **/
    float length() const;

    /**
     * @return The dot product of the two vectors @p v and @p w.
     *
     * The dot product v*w is equal to |v|*|w|*cos(alpha), where alpha is the
     * angle between both vectors and |v| is the length of v.
     **/
    static inline float dotProduct(const BoVector3& v, const BoVector3& w)
    {
      return v[0] * w[0] + v[1] * w[1] + v[2] * w[2];
    }

    /**
     * @return The dot product of this vector with itself, i.e. (v * v).
     **/
    inline float dotProduct() const
    {
      return dotProduct(*this, *this);
    }

    /**
     * @return The cross product of v and w.
     **/
    static BoVector3 crossProduct(const BoVector3& v, const BoVector3& w);

    /**
     * @return A pointer to the internal array.
     **/
    inline const GLfloat* data() const { return mData; }

    /**
     * See @ref set
     **/
    inline void operator=(const BoVector3& v)
    {
      set(v);
    }

    /**
     * @overload
     **/
    inline void operator=(const GLfloat* v)
    {
      set(v);
    }

    /**
     * See @ref add
     **/
    inline void operator+=(const BoVector3& v)
    {
      add(v);
    }

    /**
     * @return The component / coordinate at @p i of this vector
     **/
    inline GLfloat operator[](int i) const
    {
      return mData[i];
    }

    /**
     * @return A copy of this vector with @p v added.
     **/
    inline BoVector3 operator+(const BoVector3& v) const
    {
      return BoVector3(mData[0] + v.mData[0], mData[1] + v.mData[1], mData[2] + v.mData[2]);
    }

    /**
     * @return A copy of this vector, @p v subtracted.
     **/
    inline BoVector3 operator-(const BoVector3& v) const
    {
      return BoVector3(mData[0] - v.mData[0], mData[1] - v.mData[1], mData[2] - v.mData[2]);
    }

    /**
     * @return A copy of this vector, scaled by @p f.
     **/
    inline BoVector3 operator*(float f) const
    {
      return BoVector3(mData[0] * f, mData[1] * f, mData[2] * f);
    }

    /**
     * @return See @ref crossProduct. Cross product of this vector with @p v
     **/
    inline BoVector3 operator*(const BoVector3& v) const
    {
      return crossProduct(*this, v);
    }

    /**
     * @return A copy of this vector, scaled by 1 / @p f
     **/
    inline BoVector3 operator/(float f) const
    {
      return BoVector3(mData[0] / f, mData[1] / f, mData[2] / f);
    }

    /**
     * @return A copy of this vector scaled by -1
     **/
    inline BoVector3 operator-() const
    {
      return BoVector3(-mData[0], -mData[1], -mData[2]);
    }

    /**
     * @return Whether all components of this vector are zeros
     **/
    inline bool isNull() const
    {
      return ((mData[0] == 0.0f) && (mData[1] == 0.0f) && (mData[2] == 0.0f));
    }

    /**
     * Create 3 vectors from @p face in @p mesh and place them into @p v.
     * @param v An array of size 3 which will contain the vectors of the face.
     **/
    static void makeVectors(BoVector3* v, const Lib3dsMesh* mesh, const Lib3dsFace* face);

    /**
     * @param v1 An array of 3 vectors (i.e. one triangle)
     * @param v2 An array of 3 vectors (i.e. one triangle)
     * @return TRUE if the two triangles are adjacent, i.e. if they share at
     * least two points. also returns TRUE if the triangles are equal!
     **/
    static bool isAdjacent(const BoVector3* v1, const BoVector3* v2);

    /**
     * @param point The point to search for
     * @param array An array of size 3 (one face/triangle)
     * @return the index of the point @p point in @p array, or -1 if @p point is
     * not in @p array.
     **/
    static int findPoint(const BoVector3& point, const BoVector3* array);

    /**
     * Convenience method for BoVector3::findPoint(*this, array);
     **/
    int findPoint(const BoVector3* array) const
    {
      return findPoint(*this, array);
    }

    /**
     * Convenience method for BoVector3::isAdjacent(this, v);
     **/
    bool isAdjacent(const BoVector3* v) const
    {
      return isAdjacent(this, v);
    }

    /**
     * @return TRUE when the coordinates of this vector equal x,y and z,
     * otherwise FALSE.
     * @param diff The maximal difference that the elements may have to be
     * treated as "equal". note that 0.0 is a bad idea, since rounding errors
     * are _very_ probable!
     **/
    inline bool isEqual(float x, float y, float z, float diff = 0.001) const
    {
      float v2[3];
      v2[0] = x;
      v2[1] = y;
      v2[2] = z;
      return BoVector3::isEqual(mData, v2);
      return true;
    }

    /**
     * @overload
     *
     * Same as above, except that it takes an array of 3 floats, such as e.g.
     * Lib3dsVector.
     **/
    inline bool isEqual(const float* v, float diff = 0.001) const { return isEqual(v[0], v[1], v[2], diff); }

    /**
     * @overload
     **/
    inline bool isEqual(const BoVector3& v, float diff = 0.001) const
    {
      return isEqual(v.data(), diff);
    }

    /**
     * @overload
     *
     * Same as above, except that it takes 2 separate float arrays. You can use
     * this static method without a BoVector3 instance - useful for comparing
     * Lib3dsVectors.
     **/
    static bool isEqual(const float* v1, const float* v2, float diff = 0.001)
    {
      // avoid fabsf() as we don't include math.h
      float d1 = v1[0] - v2[0];
      float d2 = v1[1] - v2[1];
      float d3 = v1[2] - v2[2];
      if (d1 < 0.0f)
      {
        d1 = -d1;
      }
      if (d2 < 0.0f)
      {
        d2 = -d2;
      }
      if (d3 < 0.0f)
      {
        d3 = -d3;
      }
      if (d1 > diff)
      {
        return false;
      }
      if (d2 > diff)
      {
        return false;
      }
      if (d3 > diff)
      {
        return false;
      }
      return true;
    }

    /**
     * @return A string that contains the vector @p v. This string can be used
     * for debugging.
     **/
    static QString debugString(const BoVector3& v);

    /**
     * Dump this vector onto the console. See also @ref debugString
     **/
    static void debugVector(const BoVector3& v);

    /**
     * Convenience method for BoVector3::debugString(*this)
     **/
    QString debugString() const;


    // Conversion from one coordinate system to another. Should we use BO_GL_CELL_SIZE here?
    inline void canvasToCell()
    {
      mData[0] /= (float)BO_TILE_SIZE; mData[1] /= (float)BO_TILE_SIZE;
    }
    inline void cellToCanvas()
    {
      mData[0] *= (float)BO_TILE_SIZE; mData[1] *= (float)BO_TILE_SIZE;
    }
    inline void canvasToWorld()
    {
      mData[0] /= (float)BO_TILE_SIZE; mData[1] = -(mData[1] / (float)BO_TILE_SIZE);
    }


  private:
    friend class BoMatrix;
    friend QDataStream& operator<<(QDataStream& s, const BoVector3& v);
    friend QDataStream& operator>>(QDataStream& s, BoVector3& v);

    GLfloat mData[3];
};

QDataStream& operator<<(QDataStream& s, const BoVector3& v);
QDataStream& operator>>(QDataStream& s, BoVector3& v);

/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoVector4
{
  public:

    BoVector4()  { reset(); };
    BoVector4(GLfloat x, GLfloat y, GLfloat z, GLfloat w)  { set(x, y, z, w); };
    BoVector4(const GLfloat* data) { set(data[0], data[1], data[2], data[3]); }
    ~BoVector4() {};

    /**
     * Make this vector a null vector.
     **/
    inline void reset()
    {
      mData[0] = mData[1] = mData[2] = mData[3] = 0.0f;
    }

    /**
     * @return The first (x) coordinate of the vector.
     **/
    inline GLfloat x() const { return mData[0]; }
    /**
     * @return The second (y) coordinate of the vector.
     **/
    inline GLfloat y() const { return mData[1]; }
    /**
     * @return The third (z) coordinate of the vector.
     **/
    inline GLfloat z() const { return mData[2]; }
    /**
     * @return The fourth (w) coordinate of the vector.
     **/
    inline GLfloat w() const { return mData[3]; }


    /**
     * Assign the values @p x, @p y, @p z, @p w to the vector.
     **/
    inline void set(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
    {
      mData[0] = x;  mData[1] = y;  mData[2] = z; mData[3] = w;
    }
    /**
     * @overload
     **/
    inline void set(const float* v)
    {
      set(v[0], v[1], v[2], v[3]);
    }
    /**
     * @overload
     **/
    inline void set(const BoVector4& v)
    {
      set(v.data());
    }

    /**
     * Assign the x coordinate to the vector.
     **/
    inline void setX(GLfloat x) { mData[0] = x; }
    /**
     * Assign the y coordinate to the vector.
     **/
    inline void setY(GLfloat y) { mData[1] = y; }
    /**
     * Assign the z coordinate to the vector.
     **/
    inline void setZ(GLfloat z) { mData[2] = z; }
    /**
     * Assign the w coordinate to the vector.
     **/
    inline void setW(GLfloat w) { mData[3] = w; }

    /**
     * Scale @p v by s and then add it to this vector.
     **/
    inline void addScaled(const BoVector4& v, GLfloat s)
    {
      mData[0] += v.mData[0] * s;  mData[1] += v.mData[1] * s;  mData[2] += v.mData[2] * s;  mData[3] += v.mData[3] * s;
    }

#if 0
    // AB: see BoVector3::setScaledSum()
    inline void setScaledSum(BoVector4 a, BoVector4 b,  GLfloat s)
        { mData[0] = a.mData[0] + b.mData[0] * s;   mData[1] = a.mData[1] + b.mData[1] * s;   mData[2] = a.mData[2] + b.mData[2] * s;   mData[3] = a.mData[3] + b.mData[3] * s; };
#endif

    inline void setBlended(const BoVector4& a, float af, const BoVector4& b, float bf)
    {
      mData[0] = a.mData[0] * af + b.mData[0] * bf;
      mData[1] = a.mData[1] * af + b.mData[1] * bf;
      mData[2] = a.mData[2] * af + b.mData[2] * bf;
      mData[3] = a.mData[3] * af + b.mData[3] * af;
    }

    /**
     * Add @p v to this vector.
     **/
    inline void add(const BoVector4& v)
    {
      mData[0] += v.mData[0]; mData[1] += v.mData[1]; mData[2] += v.mData[2]; mData[3] += v.mData[3];
    }

    /**
     * @return A pointer to the internal array.
     **/
    inline const GLfloat* data() const { return mData; }

    /**
     * See @ref set
     **/
    inline void operator=(const BoVector4& v)
    {
      set(v);
    }
    /**
     * See @ref set
     **/
    inline void operator=(const GLfloat* v)
    {
      set(v);
    }
    /**
     * @return The component / coordinate at @p i of this vector
     **/
    inline GLfloat operator[](int i) const { return mData[i]; }

    /**
     * @return A string that contains the vector @p v. This string can be used
     * for debugging.
     **/
    static QString debugString(const BoVector4& v);

    /**
     * Dump this vector onto the console. See also @ref debugString
     **/
    static void debugVector(const BoVector4& v);

    /**
     * Convenience method for BoVector4::debugString(*this)
     **/
    QString debugString() const;

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
 * @short A 4x4 matrix as used by boson and OpenGL
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoMatrix
{
  public:
    /**
     * Construct an (identitiy) matrix. See @ref loadIdentity.
     **/
    BoMatrix()
    {
      loadIdentity();
    }

    /**
     * Construct a matrix that is a copy of @p matrix. See @ref loadMatrix
     **/
    BoMatrix(const GLfloat* matrix)
    {
      loadMatrix(matrix);
    }

    /**
     * Construct a matrix that is a copy of @p matrix. See @ref loadMatrix
     **/
    BoMatrix(const BoMatrix& matrix)
    {
      loadMatrix(matrix);
    }

    /**
     * Load @p matrix from OpenGL. See @ref loadMatrix.
     **/
    BoMatrix(GLenum matrix)
    {
      loadMatrix(matrix);
    }

    /**
     * Load the identity matrix (the "1" for matrices - M * identity = M)
     **/
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

    /**
     * @overload
     **/
    void loadMatrix(const GLfloat* m);

    /**
     * @overload
     **/
    void loadMatrix(const BoMatrix& m) { loadMatrix(m.data()); }

    /**
     * Change the element at @p row, @p column to @p value. See also @ref
     * element and @ref indexAt
     **/
    void setElement(int row, int column, float value)
    {
      mData[indexAt(row, column)] = value;
    }

    /**
     * See also @ref indexAt
     * @param row 0..3 -> specifies the row (aka line) of the matrix
     * @param column 0..3 -> specifies the column of the matrix (what a
     * surprise)
     * @return The element of the matrix at the specified position
     **/
    inline float element(int row, int column) const
    {
      return mData[indexAt(row, column)];
    }

    /**
     * @return A pointer to the internal array. See also @ref element, @ref indexAt,
     * @ref setElement
     **/
    const GLfloat* data() const { return mData; }

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
     * Translate (i.e. move) the matrix by x,y,z.
     **/
    void translate(GLfloat x, GLfloat y, GLfloat z);

    /**
     * @overload
     **/
    inline void translate(const BoVector3& v)
    {
      translate(v.x(), v.y(), v.z());
    }

    /**
     * Scale the matrix by x,y,z.
     *
     * Note that if one of x,y,z is 0.0 the result will probably an invalid
     * matrix. Don't do that unless you really know what you're doing.
     **/
    void scale(GLfloat x, GLfloat y, GLfloat z);

    /**
     * Multiply the matrix by @p mat.
     * @param mat An array as returned by @ref data and as used by OpenGL.
     **/
    void multiply(const GLfloat* mat);

    /**
     * @overload
     **/
    inline void multiply(const BoMatrix* mat)
    {
      multiply(mat->data());
    }

    /**
     * Rotate around a specified axis. @p angle specifies the angle, i.e. how
     * much it is rotated and x,y,z specify the axis.
     *
     * See also the OpenGL glRotate() which uses the same syntax.
     **/
    void rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);

    /**
     * Transform the vector @p input according to this matrix and put the result
     * into @p v.
     *
     * This calculates simply does v = M * input, where M is this matrix.
     **/
    void transform(BoVector3* v, const BoVector3* input) const;

    /**
     * @overload
     **/
    void transform(BoVector4* v, const BoVector4* input) const;

    /**
     * Invert this matrix and place the result into @p inverse.
     * @return TRUE on success or FALSE if this is a not invertible matrix.
     **/
    bool invert(BoMatrix* inverse) const;

    /**
     * @return TRUE when.. well, when this matrix is equal to @p matrix
     * @param diff The maximal difference that the elements may have to be
     * treated as "equal". note that 0.0 is a bad idea, since rounding errors
     * are _very_ probable!
     **/
    bool isEqual(const BoMatrix& matrix, float diff = 0.001) const;

    /**
     * @return The element at index @p i in the internal array. See @ref
     * indexAt.
     **/
    inline GLfloat operator[](int i) const { return mData[i]; }

    /**
     * Convert the rotation matrix to 3 angles. If you combine these angles in
     * the following way (the order is important!)
     * <pre>
     * glRotatef(*angleX, 1.0, 0.0, 0.0);
     * glRotatef(*angleY, 0.0, 1.0, 0.0);
     * glRotatef(*angleZ, 0.0, 0.0, 1.0);
     * </pre>
     * You will get this rotation matrix.
     *
     * Note that the results of this function are totally undefined if this is
     * not a rotation matrix (i.e. a mtrix that was rotated only)
     *
     * These angles are often referred to as euler angles.
     **/
    void toRotation(float* angleX, float* angleY, float* angleZ);

    /**
     * @return The index of the element @p row, @p column of the matrix in the
     * internal array. The array can be organized in two different ways, which
     * both are used out there in the world. We are preferring the organization
     * that is used by OpenGL/mesa
     **/
    static inline int indexAt(int row, int column) { return (column << 2) + row; }

    /**
     * Dump this matrix to the console as debug output.
     **/
    void debugMatrix()
    {
      debugMatrix(data());
    }

    /**
     * Dump @p matrix onto the console as debug output.
     **/
    static void debugMatrix(const GLfloat* matrix);

  private:
    GLfloat mData[16];
};

// most has been stolen from
// http://www.gamedev.net/reference/articles/article1095.asp
// and also from
// http://www.j3d.org/matrix_faq/matrfaq_latest.html
// (it seems both articles share some code :))
// note at least in the matrix/quat faq there are some errors! (version 1.20)

/**
 * A rotation can be represented in several ways, a quaternion is one of them.
 *
 * The by far easiest and most popular way is to store 3 different rotation
 * values, one for each axis. In OpenGL code this will look like this:
 * <pre>
 * glRotatef(angleX, 1.0f, 0.0f, 0.0f);
 * glRotatef(angleY, 0.0f, 1.0f, 0.0f);
 * glRotatef(angleZ, 0.0f, 0.0f, 1.0f);
 * </pre>
 * These angles are called euler angles.
 * They suffer from the so-called * "gimbal-lock".
 *
 * Use google to find a description on what this "gimbal lock" is  - i am not
 * qualified enough to give a correct description. It is enough to say that the
 * rotation will not occur as you want it to.
 *
 * Another representation is the angle axis representation. Here you do only one
 * rotation, by an arbitrary axis. Code:
 * <pre>
 * glRotatef(angle, axisX, axisY, axisZ);
 * </pre>
 * This does not suffer from gimbal lock but (according to some
 * tutorials/howtos) it suffers from other things, such as when you interpolate
 * between two rotations. This problem does not matter for us, as we do not
 * (yet?) use it. But it is imho hard to use and to calculate.
 *
 * The third, and probably most popular among big 3d projects, way of
 * representing rotations are quaternions.
 *
 * I will not try to explain to you what exactly quaternions are - i am not
 * qualified enough to do this. Use google and e.g.
 * http://www.gamedev.net/reference/articles/article1095.asp
 *
 * A quaternion consists of 4 floating point values - a scalar (w) and a vector
 * (v).
 * You can get a rotation matrix (see @ref matrix) from a quaternion and
 * therefore you can easily use it in glMultMatrix.
 *
 * You can convert all major means of rotation into a quaternion,
 * see @ref setRotation.
 *
 * If you @ref multiply a quat by another one you get a similar effect as
 * if you had multiplied both rotation matrices, i.e. the two rotations are
 * combined.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoQuaternion
{
public:
	BoQuaternion()
	: mW(1.0f)
	{
	}
	BoQuaternion(const BoQuaternion& quat)
	{
		*this = quat;
	}
	BoQuaternion(float w, const BoVector3& v)
	{
		set(w, v);
	}
	BoQuaternion& operator=(const BoQuaternion& quat)
	{
		set(quat);
		return *this;
	}

	void loadIdentity()
	{
		mW = 1.0f;
		mV.set(0.0f, 0.0f, 0.0f);
	}

	/**
	 * @return The scalar part of this quaternion
	 **/
	float w() const
	{
		return mW;
	}

	/**
	 * @return The vector part of this quaternion
	 **/
	const BoVector3& v() const
	{
		return mV;
	}

	/**
	 * @return The quaternion converted into a rotation matrix
	 **/
	BoMatrix matrix() const;

	/**
	 * Multiply this quaternion by another one. The identity quaternion
	 * (i.e. a quat doesnt get changed if you multiply it by this one) is
	 * (1,(0,0,0)), i.e. w is 1 and the vector is (0,0,0).
	 *
	 * This way of combining two rotations does not suffer from gimbal lock.
	 **/
	void multiply(const BoQuaternion& quat)
	{
		float w = mW * quat.mW    - mV[0] * quat.mV[0] - mV[1] * quat.mV[1] - mV[2] * quat.mV[2];
		float x = mW * quat.mV[0] + mV[0] * quat.mW    + mV[1] * quat.mV[2] - mV[2] * quat.mV[1];
		float y = mW * quat.mV[1] + mV[1] * quat.mW    + mV[2] * quat.mV[0] - mV[0] * quat.mV[2];
		float z = mW * quat.mV[2] + mV[2] * quat.mW    + mV[0] * quat.mV[1] - mV[1] * quat.mV[0];
		mW = w;
		mV.set(x, y, z);
	}
	static BoQuaternion multiply(const BoQuaternion& q1, const BoQuaternion& q2)
	{
		BoQuaternion q(q1);
		q.multiply(q2);
		return q;
	}
	void operator*=(const BoQuaternion& quat)
	{
		multiply(quat);
	}
	BoQuaternion operator+(const BoQuaternion& q) const
	{
		return BoQuaternion(mW + q.mW, mV + q.mV);
	}

	float length() const;
	void normalize()
	{
		float l = length();
		mW /= l;
		mV.scale(1.0f / l);
	}

	bool isEqual(const BoQuaternion& quat) const
	{
		return ((mW == quat.mW) && (mV.isEqual(quat.mV)));
	}
	bool operator==(const BoQuaternion& quat) const
	{
		return isEqual(quat);
	}

	void set(float w, const BoVector3& v)
	{
		mW = w;
		mV = v;
	}
	inline void set(const BoQuaternion& quat)
	{
		set(quat.mW, quat.mV);
	}

	/**
	 * @param angle The angle around @p axis, given in degree.
	 **/
	void setRotation(float angle, const BoVector3& axis);

	/**
	 * The so-called "euler rotation". This creates a quaternion for as if
	 * <pre>
	 * glRotatef(angleX, 1, 0, 0);
	 * glRotatef(angleY, 0, 1, 0);
	 * glRotatef(angleZ, 0, 0, 1);
	 * </pre>
	 * was called (in this order).
	 * @param angleX The (euler-)angle around the x-axis. given in degree.
	 * @param angleY The (euler-)angle around the y-axis. given in degree.
	 * @param angleZ The (euler-)angle around the z-axis. given in degree.
	 **/
	void setRotation(float angleX, float angleY, float angleZ);

	/**
	 * Convert a rotation matrix to the quaternion. A rotation matrix is
	 * simply a matrix that describes a rotation.
	 **/
	void setRotation(const BoMatrix& rotationMatrix); // See Q55 in the quat faq

	/**
	 * Set the rotation according to the @p direction and the @p up vector.
	 * These are compatible to the parameters used in gluLookAt, but note
	 * that the @p direction differs from the lookat (aka center) vector.
	 *
	 * The direction is the (camera - lookat) vector.
	 **/
	void setRotation(const BoVector3& direction, const BoVector3& up);
	/**
	 * Convenience method for the version above. This takes exactly the
	 * arguments that gluLookAt() takes.
	 **/
	void setRotation(const BoVector3& cameraPos, const BoVector3& lookAt, const BoVector3& up)
	{
		setRotation(cameraPos - lookAt, up);
	}

	void toRotation(float* angle, BoVector3* axis); // see Q 57 in quat faq

        /**
         * See @ref BoMatrix::toRotation
         **/
	void toRotation(float* angleX, float* angleY, float* angleZ);

private:
	float mW;
	BoVector3 mV;
};

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 * @short collection class for misc 3d functions
 **/
class Bo3dTools
{
  public:
    Bo3dTools()
    {
    }

    /**
     * @return How many degrees you have to rotate around z-axis for y-axis to go
     * through (x, y). (i.e. angle between (0, 1) and (x, y) when (x, y) is a
     * normalized vector)
     * @author Rivo Laks <rivolaks@hot.ee>
     **/
    static float rotationToPoint(float x, float y);

    /**
     * This is the inverse operation to @ref rotationToPoint.
     * It calculates point (x, y) which is at intersection of circle with @p radius
     * and line which is rotated by @p angle around z-axis.
     * @author Rivo Laks <rivolaks@hot.ee>
     **/
    static void pointByRotation(float* x, float* y, const float angle, const float radius);

    /**
     * Convert @p deg, given in degree, into radians.
     * @return @p deg as radians.
     **/
    static float deg2rad(float deg);
    /**
     * Convert @p rad, given in radians, into degree.
     * @return @p rad as degree.
     **/
    static float rad2deg(float rad);

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
    // FIXME: we should use float* instead of double*
    static float sphereInFrustum(const double* viewFrustum, const BoVector3&, float radius);

    /**
     * @overload
     **/
    inline static float sphereInFrustum(const double* viewFrustum, float x, float y, float z, float radius)
    {
      BoVector3 pos(x,y,z);
      return sphereInFrustum(viewFrustum, pos, radius);
    }

    /**
     * @param modelviewMatrix A reference to the modelview matrix. You can use
     * @ref BoMatrix::loadMatrix with GL_MODELVIEW_MATRIX for this.
     * @param projectionMatrix A reference to the projection matrix. You can use
     * @ref BoMatrix::loadMatrix with GL_PROJECTION_MATRIX for this.
     * @ref viewport The viewport. Use glGetIntegerv(GL_VIEWPORT, viewport) for
     * this.
     * @param x The x-coordinate (world-, aka OpenGL- coordinates) of the point
     * that is to be projected.
     * @param y The y-coordinate (world-, aka OpenGL- coordinates) of the point
     * that is to be projected.
     * @param z The z-coordinate (world-, aka OpenGL- coordinates) of the point
     * that is to be projected.
     * @param pos Here the result will get returned. It will be in
     * window-coordinates (relative to the OpenGL widget of course).
     **/
    static bool boProject(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, GLfloat x, GLfloat y, GLfloat z, QPoint* pos);

    static bool boUnProject(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, const QPoint& pos, BoVector3* v, float z = -1.0);

    /**
     * This is a frontend to @ref boUnProject. It calculates the world-(aka
     * OpenGL-) coordinates (@p posX, @p posY, @p posZ) of given window
     * coordinates @p pos.
     *
     * This function takes the correct z value at @p pos into account (usually
     * the z/depth at the mouse cursor).
     * @param useRealDepth If TRUE this function will calculate the real
     * coordinates at @p pos, if FALSE it will calculate the coordinate at
     * @p pos with z=0.0. This is useful for e.g. @ref mapDistance, where
     * different z values could deliver wrong values.
     **/
    static bool mapCoordinates(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, const QPoint& pos, GLfloat* posX, GLfloat* posY, GLfloat* posZ, bool useRealDepth = true);

    /**
     * Map distances from window to world coordinates.
     *
     * Sometimes you need to know how much a certain amount of pixels (from a
     * widget) is in world-coordinates. This is e.g. the case for mouse
     * scrolling - the player moved the mouse by a certain distance and you need
     * to scroll the scene by a certain distance.
     **/
    static bool mapDistance(const BoMatrix& modelviewMatrix, const BoMatrix& projectionMatrix, const int* viewport, int windx, int windy, GLfloat* dx, GLfloat* dy);
};


#endif // BO3DTOOLS_H
/*
 * vim:et sw=2
 */
