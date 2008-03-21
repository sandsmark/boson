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

#ifndef BOMATRIX_H
#define BOMATRIX_H

#include <math.h>

#include "math/bomath.h"
#include "math/bovector.h"

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
    BoMatrix(const float* matrix)
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
     * @overload
     **/
    void loadMatrix(const float* m);

    /**
     * @overload
     **/
    void loadMatrix(const BoMatrix& m) { loadMatrix(m.data()); }

    /**
     * @overload
     * The three vectors get interpreted as <em>row</em> vectors
     **/
    void loadMatrix(const BoVector3Float& row1, const BoVector3Float& row2, const BoVector3Float& row3);

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
    const float* data() const { return mData; }

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
     * @return TRUE if one of the matrix elements is NaN (not a number) and
     * therefore the matrix is invalid. Otherwise FALSE.
     **/
    bool hasNaN() const
    {
      for (int i = 0; i < 16; i++) {
        if (isnan(mData[i])) {
          return true;
        }
      }
      return false;
    }

    /**
     * Translate (i.e. move) the matrix by x,y,z.
     **/
    void translate(float x, float y, float z);

    /**
     * @overload
     **/
    inline void translate(const BoVector3Float& v)
    {
      translate(v.x(), v.y(), v.z());
    }

    /**
     * Scale the matrix by x,y,z.
     *
     * Note that if one of x,y,z is 0.0 the result will probably an invalid
     * matrix. Don't do that unless you really know what you're doing.
     **/
    void scale(float x, float y, float z);

    /**
     * Multiply the matrix by @p mat.
     * @param mat An array as returned by @ref data and as used by OpenGL.
     **/
    void multiply(const float* mat);

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
    void rotate(float angle, float x, float y, float z);

    /**
     * Generate a matrix from the three vectors, just as gluLookAt() does. Note
     * that the origin will remain at (0,0,0), it is not translated.
     **/
    void setLookAtRotation(const BoVector3Float& cameraPos, const BoVector3Float& lookAt, const BoVector3Float& up);

    /**
     * Transform the vector @p input according to this matrix and put the result
     * into @p v.
     *
     * This calculates simply does v = M * input, where M is this matrix.
     **/
    void transform(BoVector3Float* v, const BoVector3Float* input) const;

    /**
     * @overload
     **/
    void transform(BoVector4Float* v, const BoVector4Float* input) const;

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
    inline float operator[](int i) const { return mData[i]; }

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
     * Convert a matrix to @p lookAt and @p up, as it can get used by gluLookAt.
     *
     * Keep in mind that infinite different combinations of these vectors will
     * lead to exactly the same matrix, so you will not receive the same vectors
     * as you have initially specified. But they construct the same matrix.
     *
     * You can do this with any valid matrix (i.e. any matrix that was created
     * by using @ref rotate, @ref transform, @ref scale). You are not limited to
     * rotation matrices. But you will have to provide the cameraPos vector - it
     * may be possible without (haven't tried), but this function was developed
     * to convert euler angles to gluLookAt() and then you'll have it anyway.
     *
     * This function is pretty complex and not very optimized, as I have had to
     * develop the algorithm on my own. A good mathematician may develop a
     * faster way but that doesn't matter for us.
     **/
    void toGluLookAt(BoVector3Float* lookAt, BoVector3Float* up, const BoVector3Float& cameraPos) const;

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
    static void debugMatrix(const float* matrix);

    /**
     * See @ref loadMatrix
     **/
    inline void operator=(const BoMatrix& m)
    {
      loadMatrix(m);
    }

  private:
    /**
     * Used by @ref toGluLookAt. Extract the up vector from the two row vectors
     * @p x and @p z.
     **/
    void extractUp(BoVector3Float& up, const BoVector3Float& x, const BoVector3Float& z) const;

  private:
    float mData[16];
};

#endif
/*
 * vim:et sw=2
 */
