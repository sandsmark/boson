/*
    This file is part of the Boson game
    Copyright (C) 2003 Rivo Laks (rivolaks@hot.ee)

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

#include <qvaluelist.h>

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
    /**
     * Movement mode of the camera.
     * @li Immediate the camera will immediately move when you call e.g.
     *      @ref setLookAt(). Use this if you want camera immediately to look
     *      at the new position.
     * @li SegmentInterpolation interpolates between two positions. Use this to
     *      move camera from onew position to another.
     * @li FullInterpolation interpolates between given set of points. Use this
     *      for cinematics where you want fluid camera movement.
     **/
    enum MoveMode
    {
      Immediate = 1,
      SegmentInterpolation = 10,
      FullInterpolation = 11
    };
    enum InterpolationMode
    {
      Linear = 1,
      Sinusoidal = 2,
      SinusoidStart = 3,
      SinusoidEnd = 4,
      Cubic = 10
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
    virtual bool advanceVectors();

    /**
     * Commits changes made to camera in ticks game ticks. If ticks <= 0, then
     * changes will take effect immediately.
     **/
    void commitChanges(float ticks = 0);

    void setMoveMode(MoveMode mode);
    void setInterpolationMode(InterpolationMode mode);


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

    void addLookAtPoint(const BoVector3Float& pos, float time);
    void addUpPoint(const BoVector3Float& pos, float time);
    void addCameraPosPoint(const BoVector3Float& pos, float time);

    virtual bool loadFromXML(const QDomElement& root);
    virtual bool saveAsXML(QDomElement& root);


  protected:
    class InterpolationData
    {
      public:
        InterpolationData()  { time = -1.0f; }
        InterpolationData(const BoVector3Float& pos, float time);
        BoVector3Float pos;
        float time;
    };
    class InterpolationDataFloat
    {
      public:
        InterpolationDataFloat()  { time = -1.0f; }
        InterpolationDataFloat(float value, float time);
        float value;
        float time;
    };

    virtual void moveCompleted();
    virtual void prepareSegmentInterpolation(float endtime);

    bool getCurrentVector(QValueList<InterpolationData>& points, float time, BoVector3Float& result);
    bool getCurrentValue(QValueList<InterpolationDataFloat>& values, float time, float& result);

    void setPositionDirty(bool d = true);
    float currentTime()  { return mCurrentTime; }

  protected:
    MoveMode moveMode() const { return mMoveMode; }
    InterpolationMode interpolationMode() const { return mInterpolationMode; }
    float moveFactor() const;

  private:
    void init();

  private:

    BoCamera* mCamera;
    float mCurrentTime;
    MoveMode mMoveMode;
    InterpolationMode mInterpolationMode;
    QValueList<InterpolationData> mLookAtPoints;
    QValueList<InterpolationData> mPosPoints;
    QValueList<InterpolationData> mUpPoints;
    bool mMoving;
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

    virtual bool advanceVectors();

    // These will _move_ given things by given values
    // Also, they don't commit changes
    // If now is true, value is changed immediately (only this value, if other
    //  changes are being committed at the same time, they won't be cancelled)
    void changeDistance(GLfloat diff);
    void changeRotation(GLfloat diff);
    void changeXRotation(GLfloat diff);

    /**
     * Set camera's rotation (in degrees). Camera is rotated around z-axis, so
     * if it's 0, camera will look along y-axis.
     * Changes are not commited
     **/
    void setRotation(GLfloat r);
    /**
     * Set camera's rotation around x-axis (in degrees). If it's 0, the camera
     * will be looking straight from above, if it's 45, it will look diagonally
     * and if it's 90, it will look at the horizon.
     * Changes are not commited
     **/
    void setXRotation(GLfloat r);
    /**
     * Set distance between lookAt point and camera's position.
     * Changes are not commited
     **/
    void setDistance(GLfloat z);

    virtual bool loadFromXML(const QDomElement& root);
    virtual bool saveAsXML(QDomElement& root);

  protected:
    virtual void moveCompleted();
    virtual void prepareSegmentInterpolation(float endtime);

  private:
    void init();

  private:
    QValueList<InterpolationDataFloat> mDistancePoints;
    QValueList<InterpolationDataFloat> mRotationPoints;
    QValueList<InterpolationDataFloat> mXRotationPoints;
};

#endif

/*
 * vim: et:sw=2
 */
