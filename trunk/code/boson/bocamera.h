/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef BOCAMERA_H
#define BOCAMERA_H

#include "bo3dtools.h"

#include <GL/gl.h>

class QDomElement;

/**
 * Camera class for Boson
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoCamera
{
  public:
    enum MoveMode { Linear = 1, Sinusoidal = 2, SinusoidStart = 3, SinusoidEnd = 4 };

    BoCamera();
    BoCamera(const BoCamera& c)
    {
      *this = c;
    }

    virtual ~BoCamera()
    {
    }

   /**
    * Apply the camera to the scene by doing the necessary OpenGL
    * transformation on the modelview matrix.
    *
    * This will first load the identity matrix, so any previous changes are
    * lost. use glPushMatrix()/glPopMatrix() if you need your old settings
    * back at a later point.
    **/
    void applyCameraToScene();


    /**
     * Advances camera. This smoothly (and linearly) applies changes made to
     * camera.
     **/
    void advance();
    virtual bool advance2();

    /**
     * Commits changes made to camera in ticks game ticks. If ticks <= 0, then
     * changes will take effect immediately.
     **/
    void commitChanges(int ticks);

    void setMoveMode(MoveMode mode);

    int commitTime() const { return mCommitTime; }
    int remainingTime() const { return mRemainingTime; }
    float movedAmount() const { return mMovedAmount; }


    BoCamera& operator=(const BoCamera& c);


    /**
    * Set the gluLookAt() paremeters directly. Note that when you use
    * this @ref radius and @ref rotation will remain undefined.
    **/
    void setGluLookAt(const BoVector3& lookAt, const BoVector3& cameraPos, const BoVector3& up);

    /**
    * @return The eye vector (camera position), as it can get used by
    * gluLookAt().
    **/
    const BoVector3& cameraPos();

    /**
    * @return The up vector, as it can get used by gluLookAt(). The up
    * vector is the vector pointing straight "up" from the position of the
    * camera. it can change when the camera is rotated.
    **/
    const BoVector3& up();


    // These will _move_ given things by given values
    // Also, they don't commit changes
    // If now is true, value is changed immediately (only this value, if other
    //  changes are being committed at the same time, they won't be cancelled)
    void changeLookAt(const BoVector3& diff, bool now = false);

    // these will change the up and cameraPos vectors!
    /**
     * Set lookAt point of camera
     * Changes are not commited
     **/
    void setLookAt(const BoVector3& pos, bool now = false);

    /**
    * @return The point we are looking at. This is the lookAt vector, as it
    * can get used by gluLookAt().
    **/
    const BoVector3& lookAt() const  { return mLookAt; }

    virtual bool loadFromXML(const QDomElement& root);
    virtual bool saveAsXML(QDomElement& root);


  protected:
    /**
     * Checks if camera is inside rectangle set by setMoveRect method.
     * If it's not inside this rectangle, it will be moved into it.
     **/
    virtual void checkPosition() = 0;

    /**
    * Update the parameters for gluLookAt() (@ref cameraPos
    * and @ref up) according to the new values from @ref radius,
    * @ref rotation and @ref lookAt.
    * Don't call this manually, call @ref setPositionDirty instead. This will
    * be automatically called by @ref cameraPos and @ref up, if it's dirty.
    **/
    virtual void updatePosition() = 0;
    virtual void resetDifferences();

//    void checkRotation();

    void setPositionDirty(bool dirty = true) { mPosDirty = dirty; }
    bool positionDirty() const { return mPosDirty; }

  protected:
    MoveMode moveMode() const { return mMoveMode; }
    float moveFactor() const;
  private:
    void init();

  private:
    BoVector3 mLookAt;
    BoVector3 mUp;
    BoVector3 mCameraPos;
    bool mPosDirty;

    BoVector3 mLookAtDiff;
    int mCommitTime, mRemainingTime;
    MoveMode mMoveMode;
    float mMovedAmount;
};

/**
 * Camera class for Boson
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoGameCamera : public BoCamera
{
  public:
    BoGameCamera();
    /**
     * Construct camera which will move only in given rectangle
     **/
    BoGameCamera(GLfloat minX, GLfloat maxX, GLfloat minY, GLfloat maxY);

    BoGameCamera(const BoGameCamera& c)
      : BoCamera()
    {
      *this = c;
    }
    virtual ~BoGameCamera()
    {
    }

    /**
     * Advances camera. This smoothly (and linearly) applies changes made to
     * camera.
     **/
    virtual bool advance2();


    BoGameCamera& operator=(const BoGameCamera& c);


    static float minCameraZ();
    static float maxCameraZ();
    static float maxCameraRadius();


    // These will _move_ given things by given values
    // Also, they don't commit changes
    // If now is true, value is changed immediately (only this value, if other
    //  changes are being committed at the same time, they won't be cancelled)
    void changeZ(GLfloat diff, bool now = false);
    void changeRadius(GLfloat diff, bool now = false);
    void changeRotation(GLfloat diff, bool now = false);

    // these will change the up and cameraPos vectors!
    /**
     * Set camera's rotation (in degrees). Rotation is measured from y-axis, if
     * it's 0, camera will look along y-axis.
     * Changes are not commited
     **/
    void setRotation(GLfloat r, bool now = false);
    /**
     * Set distance between look-at point and camera's position on xy-plane.
     * It means that if there would be a cylinder which lower center point would
     * be at lookAt point and it's radius would be r, then camera would be
     * somewhere along the edge of upper cap of this cylinder.
     * Changes are not commited
     **/
    void setRadius(GLfloat r, bool now = false);
    /**
     * Set distance between lookAt point and camera in z-axis
     * Changes are not commited
     **/
    void setZ(GLfloat z, bool now = false);

    GLfloat z() const  { return mPosZ; }
    GLfloat rotation() const  { return mRotation; }
    GLfloat radius() const  { return mRadius; }

    /**
    * Set limits for the camera. The camera tries not to move beyond the
    * given rectangle.
    **/
    void setMoveRect(GLfloat minX, GLfloat maxX, GLfloat minY, GLfloat maxY);

    virtual bool loadFromXML(const QDomElement& root);
    virtual bool saveAsXML(QDomElement& root);


  protected:
    /**
     * Checks if camera is inside rectangle set by setMoveRect method.
     * If it's not inside this rectangle, it will be moved into it.
     **/
    virtual void checkPosition();

    /**
    * Update the parameters for gluLookAt() (@ref cameraPos
    * and @ref up) according to the new values from @ref radius,
    * @ref rotation and @ref lookAt.
    * Don't call this manually, call @ref setPositionDirty instead. This will
    * be automatically called by @ref cameraPos and @ref up, if it's dirty.
    **/
    virtual void updatePosition();
    virtual void resetDifferences();

    void checkRotation();

  private:
    void init();
    static void initStatic();

  private:
    GLfloat mPosZ;
    GLfloat mRotation;
    GLfloat mRadius;

    GLfloat mMinX, mMaxX, mMinY, mMaxY;

    GLfloat mPosZDiff;
    GLfloat mRotationDiff;
    GLfloat mRadiusDiff;
};

#endif

/*
 * vim: et:sw=2
 */
