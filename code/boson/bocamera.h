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
    virtual void setGluLookAt(const BoVector3& cameraPos, const BoVector3& lookAt, const BoVector3& up);


    /**
     * @return The point we are looking at. This is the lookAt vector, as it
     * can get used by gluLookAt().
     **/
    const BoVector3& lookAt() const  { return mLookAt; }

    /**
     * @return The eye vector (camera position), as it can get used by
     * gluLookAt().
     **/
    virtual const BoVector3& cameraPos();

    /**
     * @return The up vector, as it can get used by gluLookAt(). The up
     * vector is the vector pointing straight "up" from the position of the
     * camera. it can change when the camera is rotated.
     **/
    virtual const BoVector3& up();


    // These will _move_ given things by given values
    void changeCameraPos(const BoVector3& diff);
    void changeLookAt(const BoVector3& diff);
    void changeUp(const BoVector3& diff);

    // these will change the up and cameraPos vectors!
    /**
     * Set lookAt point of camera (where the camera is looking at)
     **/
    virtual void setLookAt(const BoVector3& lookat);
    /**
     * Set position of the camera
     **/
    virtual void setCameraPos(const BoVector3& pos);
    /**
     * Set up vector of the camera (vector pointing straight up in the viewport)
     **/
    virtual void setUp(const BoVector3& up);


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


  protected:
    void setPositionDirty(bool dirty = true) { mPosDirty = dirty; }
    bool positionDirty() const { return mPosDirty; }

  private:
    void init();

  private:
    friend class BoAutoCamera;
    BoAutoCamera* mAutoCamera;

    BoVector3 mLookAt;
    BoVector3 mUp;
    BoVector3 mCameraPos;

    bool mPosDirty;
};

/**
 * Game camera class for Boson
 *
 * Game camera differs from base camera in that it supports setting radius,
 * rotation, z, etc for camera and then recalculates camera's position and
 * lookAt vector itself, so you don't have to bother
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
    void changeZ(GLfloat diff);
    void changeRadius(GLfloat diff);
    void changeRotation(GLfloat diff);

    virtual void setLookAt(const BoVector3& pos);
    virtual void setCameraPos(const BoVector3& pos);
    virtual const BoVector3& cameraPos();
    virtual const BoVector3& up();

    // these will change the up and cameraPos vectors!
    /**
     * Set camera's rotation (in degrees). Rotation is measured from y-axis, if
     * it's 0, camera will look along y-axis.
     **/
    void setRotation(GLfloat r);
    /**
     * Set distance between look-at point and camera's position on xy-plane.
     * It means that if there would be a cylinder which lower center point would
     * be at lookAt point and it's radius would be r, then camera would be
     * somewhere along the edge of upper cap of this cylinder.
     **/
    void setRadius(GLfloat r);
    /**
     * Set distance between lookAt point and camera in z-axis
     * Note that it can also be < 0.0, e.g. when camera's z-coordinate is below 0
     **/
    void setZ(GLfloat z);

    /**
     * This specifies how much camera is above the ground. Not that it's not
     * distance between camera position and terrain's height at this point, but
     * between camera's lookat point and terrain's height at this point.
     **/
    GLfloat z() const  { return mPosZ; }
    GLfloat rotation() const  { return mRotation; }
    GLfloat radius() const  { return mRadius; }

    void setCanvas(BosonCanvas* canvas)  { mCanvas = canvas; }


    virtual bool loadFromXML(const QDomElement& root);
    virtual bool saveAsXML(QDomElement& root);


    float minCameraZ();
    float maxCameraZ();

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

    /**
     * Makes sure camera's readius is within limits (e.g. camera's angle is not
     *  less than minimum angle)
     **/
    void checkRadius();

  private:
    void init();
    static void initStatic();

  private:
    friend class BoAutoGameCamera;
    GLfloat mPosZ;
    GLfloat mRotation;
    GLfloat mRadius;

    const BosonCanvas* mCanvas;
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

    void setLightPos(const BoVector3& pos);
    virtual void setGluLookAt(const BoVector3& c, const BoVector3& l, const BoVector3& u);

  private:
    BoContext* mContext;
    BoLight* mLight;
};

#endif

/*
 * vim: et:sw=2
 */
