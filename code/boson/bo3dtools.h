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
     * @return The length of the vector.
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
     * Loads BoVector3 from KConfig
     **/
    static BoVector3 load(const KConfig* cfg, const QString key, const BoVector3& aDefault = BoVector3());

    /**
     * Saves BoVector3 to KConfig
     **/
    void save(KConfig* cfg, const QString key) const;

    /**
     * @return TRUE when the coordinates of this vector equal x,y and z,
     * otherwise FALSE.
     **/
    inline bool isEqual(float x, float y, float z) const
    {
      return mData[0] == x && mData[1] == y && mData[2] == z;
    }

    /**
     * @overload
     *
     * Same as above, except that it takes an array of 3 floats, such as e.g.
     * Lib3dsVector.
     **/
    inline bool isEqual(const float* v) const { return isEqual(v[0], v[1], v[2]); }

    /**
     * @overload
     **/
    inline bool isEqual(const BoVector3& v) const
    {
      return isEqual(v.data());
    }

    /**
     * @overload
     *
     * Same as above, except that it takes 2 separate float arrays. You can use
     * this static method without a BoVector3 instance - useful for comparing
     * Lib3dsVectors.
     **/
    static bool isEqual(const float* v1, const float* v2)
    {
      return v1[0] == v2[0] && v1[1] == v2[1] && v1[2] == v2[2];
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
     * Load a BoVector4 from @ref KConfig
     **/
    static BoVector4 load(const KConfig* cfg, const QString key, const BoVector4& aDefault = BoVector4());

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
     **/
    bool isEqual(const BoMatrix& matrix) const;

    /**
     * @return The element at index @p i in the internal array. See @ref
     * indexAt.
     **/
    inline GLfloat operator[](int i) const { return mData[i]; }

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
};



#endif // BO3DTOOLS_H
/*
 * vim:et sw=2
 */
