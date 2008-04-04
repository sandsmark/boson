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

#ifndef BOQUATERNION_H
#define BOQUATERNION_H

#include <math.h>

#include "math/bomath.h"
#include "math/bovector.h"

class BoMatrix;

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

    BoQuaternion(float w, const BoVector3Float& v)
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
    const BoVector3Float& v() const
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
     *
     * Note that this quaternion, as well as @p quat should be normalized
     * quaternions! See also @ref normalize. The resulting quaternion will be
     * normalized as well.
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

    /**
     * @return The conjugate of the quaternion, which is the quaternion with the
     * vector part negated.
     **/
    inline BoQuaternion conjugate() const
    {
      return BoQuaternion(mW, BoVector3Float(-mV[0], -mV[1], -mV[2]));
    }

    /**
     * @return The inverse quaternion. This is equal to the @ref conjugate, if
     * the quaternion is normalized (see @ref normalize). You should prefer @ref
     * conjugate if you know that the quat is normalized (i.e. always)!
     **/
    BoQuaternion inverse() const
    {
      // we assume that the quat is normalized.
      // If it is not, we would have to new_quat.mW /= quat.length()
      BoQuaternion q = conjugate();
      float l = length();
      q.mW /= l;
      return q;
    }

    /**
     * Rotate the vector @input and return the result into @p v.
     *
     * Of course we assume that this quaternion is normalized (see @ref
     * normalize), as only normalized quaternions represent rotations.
     **/
    void transform(BoVector3Float* v, const BoVector3Float* input) const;

    float length() const;

    /**
     * Normalize the quaternion. Note that only a normalized quaternion
     * represents a rotation, meaning that non-normalized quaternions are
     * useless for us!
     **/
    void normalize()
    {
      float l = length();
      mW /= l;
      mV.scale(1.0f / l);
    }

    bool isEqual(const BoQuaternion& quat, float diff = 0.001) const
    {
      // avoid fabsf() as we don't include math.h
      float d = mW - quat.mW;
      if (d < 0)
      {
        d = -d;
      }
      if (d > diff)
      {
        return false;
      }
      return mV.isEqual(quat.mV, diff);
    }

    bool operator==(const BoQuaternion& quat) const
    {
      return isEqual(quat);
    }

    void set(float w, const BoVector3Float& v)
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
    void setRotation(float angle, const BoVector3Float& axis);

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
    void setRotation(const BoVector3Float& direction, const BoVector3Float& up);

    /**
     * Convenience method for the version above. This takes exactly the
     * arguments that gluLookAt() takes.
    **/
    void setRotation(const BoVector3Float& cameraPos, const BoVector3Float& lookAt, const BoVector3Float& up)
    {
      setRotation(cameraPos - lookAt, up);
    }

    void toRotation(float* angle, BoVector3Float* axis); // see Q 57 in quat faq

    /**
     * See @ref BoMatrix::toRotation
    **/
    void toRotation(float* angleX, float* angleY, float* angleZ);

  private:
    float mW;
    BoVector3Float mV;
};

#endif
/*
 * vim:et sw=2
 */
