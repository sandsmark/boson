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

#ifndef BOAUTOCAMERA_H
#define BOAUTOCAMERA_H

#include "bo3dtools.h"

class BoCamera;
class BoGameCamera;
class QDomElement;

/**
 * Auto camera. This class takes a certain movement diff (or a final position)
 * and moves the camera in a certain period of time to that position.
 *
 * This can be used e.g. in scripts to provide automatic camera movement.
 *
 * Note that auto camera will _not_ do _any_ validation or limits checking on
 * the coordinates that you throw at it.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoAutoCamera
{
  public:
    enum MoveMode {
      Linear = 1,
      Sinusoidal = 2,
      SinusoidStart = 3,
      SinusoidEnd = 4
    };

    BoAutoCamera(BoCamera* camera);
    BoAutoCamera(const BoAutoCamera& c)
    {
      *this = c;
    }

    virtual ~BoAutoCamera()
    {
    }

    void setCamera(BoCamera* c) { mCamera = c; }
    BoCamera* camera() const { return mCamera; }

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


    BoAutoCamera& operator=(const BoAutoCamera& c);


    // These will _move_ given things by given values
    // Also, they don't commit changes
    // If now is true, value is changed immediately (only this value, if other
    //  changes are being committed at the same time, they won't be cancelled)
    void changeLookAt(const BoVector3Float& diff);
    void changeUp(const BoVector3Float& pos);
    void changeCameraPos(const BoVector3Float& pos);

    // these will change the up and cameraPos vectors!
    /**
     * Set lookAt point of camera
     * Changes are not commited
     **/
    void setLookAt(const BoVector3Float& pos);
    void setUp(const BoVector3Float& pos);
    void setCameraPos(const BoVector3Float& pos);

    virtual bool loadFromXML(const QDomElement& root);
    virtual bool saveAsXML(QDomElement& root);


  protected:
    virtual void resetDifferences();

    void setPositionDirty(bool d = true);

  protected:
    MoveMode moveMode() const { return mMoveMode; }
    float moveFactor() const;

  private:
    void init();

  private:
    BoCamera* mCamera;
    BoVector3Float mLookAtDiff, mUpDiff, mCameraPosDiff;
    int mCommitTime, mRemainingTime;
    MoveMode mMoveMode;
    float mMovedAmount;
};

class BoAutoGameCamera : public BoAutoCamera
{
  public:
    BoAutoGameCamera(BoGameCamera* camera);

    BoAutoGameCamera(const BoAutoGameCamera& c)
      : BoAutoCamera(c)
    {
      *this = c;
    }
    virtual ~BoAutoGameCamera()
    {
    }

    BoAutoGameCamera& operator=(const BoAutoGameCamera& c);
    BoGameCamera* gameCamera() const { return (BoGameCamera*)camera(); }

    /**
     * Advances camera. This smoothly (and linearly) applies changes made to
     * camera.
     **/
    virtual bool advance2();

    // These will _move_ given things by given values
    // Also, they don't commit changes
    // If now is true, value is changed immediately (only this value, if other
    //  changes are being committed at the same time, they won't be cancelled)
    void changeZ(GLfloat diff);
    void changeRadius(GLfloat diff);
    void changeRotation(GLfloat diff);

    /**
     * Set camera's rotation (in degrees). Rotation is measured from y-axis, if
     * it's 0, camera will look along y-axis.
     * Changes are not commited
     **/
    void setRotation(GLfloat r);
    /**
     * Set distance between look-at point and camera's position on xy-plane.
     * It means that if there would be a cylinder which lower center point would
     * be at lookAt point and it's radius would be r, then camera would be
     * somewhere along the edge of upper cap of this cylinder.
     * Changes are not commited
     **/
    void setRadius(GLfloat r);
    /**
     * Set distance between lookAt point and camera in z-axis
     * Changes are not commited
     **/
    void setZ(GLfloat z);

    virtual bool loadFromXML(const QDomElement& root);
    virtual bool saveAsXML(QDomElement& root);

  protected:
   virtual void resetDifferences();

  private:
    void init();

  private:
    GLfloat mPosZDiff;
    GLfloat mRotationDiff;
    GLfloat mRadiusDiff;
};

#endif

/*
 * vim: et:sw=2
 */
