/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2003-2005 Rivo Laks (rivolaks@hot.ee)

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
#include <bogl.h>

class QDomElement;
class BoAutoCamera;
class BoAutoGameCamera;
class BoLight;
class BoContext;
class BosonCanvas;

/**
 * Base camera class for Boson
 *
 * This is very basic camera, supporting only lookAt, cameraPos and up vectors.
 * It doesn't check for collisions with terrain/units either.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoCamera
{
  public:
    enum CameraType
    {
      Camera = 0,
      GameCamera = 1,
      LightCamera = 2
    };

    BoCamera();
    BoCamera(const BoCamera& c)
    {
      *this = c;
    }

    virtual ~BoCamera();

    BoCamera& operator=(const BoCamera& c);


    virtual int cameraType() const { return Camera; }

    /**
     * Set the auto camera. A BoCamera object has always exactly one auto
     * camera. The previously set auto camera will get deleted, if existing.
     *
     * Note that multiple calls to this are perfectly valid (and necessary for
     * derived classes, as the c'tor of BoCamera sets a default auto camera)
     *
     * This class takes ownership of the auto camera and will delete it on
     * destruction.
     **/
    void setAutoCamera(BoAutoCamera*);
    BoAutoCamera* autoCamera() const { return mAutoCamera; }

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
     * Set the gluLookAt() paremeters directly.
     **/
    virtual void setGluLookAt(const BoVector3Float& cameraPos, const BoVector3Float& lookAt, const BoVector3Float& up);


    /**
     * @return The point we are looking at. This is the lookAt vector, as it
     * can get used by gluLookAt().
     **/
    const BoVector3Float& lookAt() const  { return mLookAt; }

    /**
     * @return The eye vector (camera position), as it can get used by
     * gluLookAt().
     **/
    virtual const BoVector3Float& cameraPos();

    /**
     * @return The up vector, as it can get used by gluLookAt(). The up
     * vector is the vector pointing straight "up" from the position of the
     * camera. it can change when the camera is rotated.
     **/
    virtual const BoVector3Float& up();


    // These will _move_ given things by given values
    void changeCameraPos(const BoVector3Float& diff);
    void changeLookAt(const BoVector3Float& diff);
    void changeUp(const BoVector3Float& diff);

    // these will change the up and cameraPos vectors!
    /**
     * Set lookAt point of camera (where the camera is looking at)
     **/
    virtual void setLookAt(const BoVector3Float& lookat);
    /**
     * Set position of the camera
     **/
    virtual void setCameraPos(const BoVector3Float& pos);
    /**
     * Set up vector of the camera (vector pointing straight up in the viewport)
     **/
    virtual void setUp(const BoVector3Float& up);


    virtual bool loadFromXML(const QDomElement& root);
    virtual bool saveAsXML(QDomElement& root);


    // FIXME: make it const!! (->make cameraPos() const)
    /**
     * The rotation matrix defines, together with the @ref cameraPos, the
     * complete camera.
     * @return The rotation matrix that is used for the current camera rotation.
     **/
    BoMatrix rotationMatrix();

    /**
     * Same as @ref rotationMatrix.
     **/
    BoQuaternion quaternion();


    /**
     * @return Whether camera has changed since it was last applied to scene.
     * Whenever you change any camera parameters (e.g. lookat position), this
     *  will be set to true. In display class, before rendering, if camera is
     *  changed, it will be reapplied.
     **/
    bool isCameraChanged()  { return mChanged; }
    /**
     * Set changed status of camera.
     * Note that normally you shouldn't use this! It may be made protected later.
     **/
    void setCameraChanged(bool changed)  { mChanged = changed; }


  protected:
    void setPositionDirty(bool dirty = true) { mPosDirty = dirty; }
    bool positionDirty() const { return mPosDirty; }

  private:
    void init();

  private:
    friend class BoAutoCamera;
    BoAutoCamera* mAutoCamera;

    BoVector3Float mLookAt;
    BoVector3Float mUp;
    BoVector3Float mCameraPos;

    bool mPosDirty;
    bool mChanged;
};

/**
 * Game camera class for Boson
 *
 * Game camera differs from base camera in that it supports setting rotation,
 * rotation around x axis and distance from the lookAt point for camera and
 * then recalculates camera's position and lookAt vector itself, so you don't
 * have to bother
 *
 * Game camera also takes care of collision detection between camera and ground
 * to make sure that camera is always above the ground.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoGameCamera : public BoCamera
{
  public:
    BoGameCamera();
    /**
     * Construct camera
     **/
    BoGameCamera(const BosonCanvas* canvas);

    BoGameCamera(const BoGameCamera& c)
      : BoCamera()
    {
      *this = c;
    }
    virtual ~BoGameCamera()
    {
    }

    virtual int cameraType() const { return GameCamera; }

    /**
     * @return @ref autoCamera casted to a @ref BoAutoGameCamera.
     **/
    BoAutoGameCamera* autoGameCamera() const
    {
      return (BoAutoGameCamera*)autoCamera();
    }

    BoGameCamera& operator=(const BoGameCamera& c);


    // These will _move_ given things by given values
    void changeDistance(GLfloat diff);
    void changeRotation(GLfloat diff);
    void changeXRotation(GLfloat diff);

    virtual void setLookAt(const BoVector3Float& pos);
    virtual void setCameraPos(const BoVector3Float& pos);
    virtual const BoVector3Float& cameraPos();
    virtual const BoVector3Float& up();

    // these will change the up and cameraPos vectors!
    /**
     * Set camera's rotation (in degrees). Camera is rotated around z-axis, so
     * if it's 0, camera will look along y-axis.
     **/
    void setRotation(GLfloat r);
    /**
     * Set camera's rotation around x-axis (in degrees). If it's 0, the camera
     * will be looking straight from above, if it's 45, it will look diagonally
     * and if it's 90, it will look at the horizon.
     **/
    void setXRotation(GLfloat r);
    /**
     * Set distance between lookAt point and camera's position.
     **/
    void setDistance(GLfloat z);

    GLfloat distance() const  { return mDistance; }
    GLfloat rotation() const  { return mRotation; }
    GLfloat xRotation() const  { return mXRotation; }

    void setCanvas(BosonCanvas* canvas)  { mCanvas = canvas; }


    virtual bool loadFromXML(const QDomElement& root);
    virtual bool saveAsXML(QDomElement& root);


    /**
     * Set whether camera is in free movement mode.
     * In free mode, BoGameCamera acts like BoCamera: z, radius and rotation
     * settings have no effect and you have to manually set lookat/position/up
     * vectors to change camera.
     * Note that no limits are applied to camera in free mode, so you can do
     * whatever you want there.
     * This is intended to be used only in cutscenes! Do not use it in game and
     * always set it back to false at the end of cutscene!
     **/
    // TODO: maybe check for limits if mode is set to non-free?
    void setFreeMovement(bool free);

    /**
     * Set whether limits are checked for when camera changes.
     * Limits include checking camera's height and radius and make sure that
     * camera doesn't e.g. go into the ground.
     * This is intended only to be used in cutscenes in case you want to do some
     * "crazy stuff" with camera that would't be allowed otherwise. Always set
     * it back to true at the end of cutscene!
     **/
    // TODO: maybe check for limits if limits are set to true?
    void setUseLimits(bool use);

  protected:
    /**
     * Checks if camera's lookat point is on the map.
     * If it's not, it will be moved onto it.
     **/
    void checkLookAtPosition();

    /**
     * Makes sure camera is above the ground.
     * If it's not, it will be moved upwards, so that it will be.
     **/
    void checkCameraPosition();

    /**
     * Update the parameters for gluLookAt() (@ref cameraPos
     * and @ref up) according to the new values from @ref radius,
     * @ref rotation and @ref lookAt.
     * Don't call this manually, call @ref setPositionDirty instead. This will
     * be automatically called by @ref cameraPos and @ref up, if it's dirty.
     **/
    void updateCamera();

    void checkRotation();
    void checkXRotation();


  private:
    void init();

  private:
    friend class BoAutoGameCamera;
    GLfloat mRotation;
    GLfloat mXRotation;
    GLfloat mDistance;
    GLfloat mActualDistance;

    const BosonCanvas* mCanvas;

    bool mFree, mLimits;
};


/**
 * This is not an actual camera, but rather a wrapper around the light position.
 * This class allows you to configure the light position using existing camera
 * configuration widgets.
 *
 * @ref setGluLookAt does the main work. The @ref lookAt vector is used for the
 * position of the light (due to the way OpenGL maths works, the correct vector
 * is NOT the cameraPos vector!)
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoLightCamera : public BoCamera
{
  public:
    BoLightCamera(BoLight* light, BoContext* context);
    ~BoLightCamera();
    virtual int cameraType() const { return LightCamera; }

    void setLightPos(const BoVector3Float& pos);
    virtual void setGluLookAt(const BoVector3Float& c, const BoVector3Float& l, const BoVector3Float& u);

  private:
    BoContext* mContext;
    BoLight* mLight;
};

#endif

/*
 * vim: et:sw=2
 */
