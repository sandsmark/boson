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

/**
 * Camera class for Boson
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoCamera
{
  public:
    enum CameraType
    {
      Camera = 0,
      GameCamera = 1
    };
    BoCamera();
    BoCamera(const BoCamera& c)
    {
      *this = c;
    }


    virtual ~BoCamera();

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
    virtual const BoVector3& cameraPos();

    /**
     * @return The up vector, as it can get used by gluLookAt(). The up
     * vector is the vector pointing straight "up" from the position of the
     * camera. it can change when the camera is rotated.
     **/
    virtual const BoVector3& up();


    // These will _move_ given things by given values
    void changeLookAt(const BoVector3& diff);

    // these will change the up and cameraPos vectors!
    /**
     * Set lookAt point of camera
     * Changes are not commited
     **/
    virtual void setLookAt(const BoVector3& pos);

    /**
     * @return The point we are looking at. This is the lookAt vector, as it
     * can get used by gluLookAt().
     **/
    const BoVector3& lookAt() const  { return mLookAt; }

    virtual bool loadFromXML(const QDomElement& root);
    virtual bool saveAsXML(QDomElement& root);


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

    virtual int cameraType() const { return GameCamera; }

    /**
     * @return @ref autoCamera casted to a @ref BoAutoGameCamera.
     **/
    BoAutoGameCamera* autoGameCamera() const
    {
      return (BoAutoGameCamera*)autoCamera();
    }

    BoGameCamera& operator=(const BoGameCamera& c);


    static float minCameraZ();
    static float maxCameraZ();
    static float maxCameraRadius();


    // These will _move_ given things by given values
    void changeZ(GLfloat diff);
    void changeRadius(GLfloat diff);
    void changeRotation(GLfloat diff);

    virtual void setLookAt(const BoVector3& pos);
    virtual const BoVector3& cameraPos();
    virtual const BoVector3& up();

    // these will change the up and cameraPos vectors!
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


    /**
     * @internal
     * Calculate the new z value, according to the camera restrictions.
     *
     * This is used by @ref changeZ as well as by the @ref BoAutoGameCamera.
     **/
    float calculateNewZ(float diff) const;
    float calculateNewRadius(GLfloat diff) const;


  protected:
    /**
     * Checks if camera is inside rectangle set by setMoveRect method.
     * If it's not inside this rectangle, it will be moved into it.
     **/
    void checkPosition();

    /**
     * Update the parameters for gluLookAt() (@ref cameraPos
     * and @ref up) according to the new values from @ref radius,
     * @ref rotation and @ref lookAt.
     * Don't call this manually, call @ref setPositionDirty instead. This will
     * be automatically called by @ref cameraPos and @ref up, if it's dirty.
     **/
    void updatePosition();

    void checkRotation();

  private:
    void init();
    static void initStatic();

  private:
    friend class BoAutoGameCamera;
    GLfloat mPosZ;
    GLfloat mRotation;
    GLfloat mRadius;

    GLfloat mMinX, mMaxX, mMinY, mMaxY;
};

#endif

/*
 * vim: et:sw=2
 */
