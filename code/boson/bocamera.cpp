/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bocamera.h"

#include "../bomemory/bodummymemory.h"
#include "boautocamera.h"
#include "defines.h"
#include "bosonconfig.h"
#include "bodebug.h"
#include "bosonglwidget.h" // BoContext
#include "bolight.h"
#include "gameengine/bosoncanvas.h"

#include <qdom.h>

// Camera limits
// These are min/max distances between camera's position and lookat point.
// Note that min distance MUST be > 0
#define CAMERA_MIN_DISTANCE 2
#define CAMERA_MAX_DISTANCE 200
// Maximum allowed angle between z-axis and camera (0 = )
#define CAMERA_MAX_XROTATION 50

// Height of the lookat point (from the ground)
#define CAMERA_LOOKAT_HEIGHT 1
// Minimum allowed z-distance between camera and ground
#define CAMERA_MIN_HEIGHT 0.5


#include <bogl.h>

#include <math.h>


/*****  BoCamera  *****/

BoCamera::BoCamera()
{
  init();
}

BoCamera& BoCamera::operator=(const BoCamera& c)
{
  mLookAt = c.mLookAt;
  mUp = c.mUp;
  mCameraPos = c.mCameraPos;

  setAutoCamera(new BoAutoCamera(*c.autoCamera()));

  setPositionDirty();
  setCameraChanged(true);

  return *this;
}

BoCamera::~BoCamera()
{
  delete mAutoCamera;
}

void BoCamera::init()
{
  mAutoCamera = 0;

  setPositionDirty();
  setCameraChanged(true);
  setAutoCamera(new BoAutoCamera(this));
}

void BoCamera::setAutoCamera(BoAutoCamera* a)
{
  delete mAutoCamera;
  mAutoCamera = a;
  if (mAutoCamera)
  {
    mAutoCamera->setCamera(this);
  }
}

void BoCamera::setGluLookAt(const BoVector3Float& cameraPos, const BoVector3Float& lookAt, const BoVector3Float& up)
{
  mLookAt = lookAt;
  mCameraPos = cameraPos;
  mUp = up;
  setCameraChanged(true);
}

void BoCamera::changeLookAt(const BoVector3Float& diff)
{
  setLookAt(lookAt() + diff);
}

void BoCamera::changeCameraPos(const BoVector3Float& diff)
{
  setCameraPos(cameraPos() + diff);
}

void BoCamera::changeUp(const BoVector3Float& diff)
{
  setUp(up() + diff);
}

bool BoCamera::loadFromXML(const QDomElement& root)
{
  bool ok;
  float lookatx, lookaty, lookatz;
  float cameraposx, cameraposy, cameraposz;
  float upx, upy, upz;

  lookatx = root.attribute("LookAtX").toFloat(&ok);
  if(!ok)
  {
    boError(230) << k_funcinfo << "Invalid value for LookAtX tag" << endl;
    lookatx = 0.0f;
  }
  lookaty = root.attribute("LookAtY").toFloat(&ok);
  if(!ok)
  {
    boError(230) << k_funcinfo << "Invalid value for LookAtY tag" << endl;
    lookaty = 0.0f;
  }
  lookatz = root.attribute("LookAtZ").toFloat(&ok);
  if(!ok)
  {
    boError(230) << k_funcinfo << "Invalid value for LookAtZ tag" << endl;
    lookatz = 0.0f;
  }
  mLookAt.set(lookatx, lookaty, lookatz);

  cameraposx = root.attribute("CameraPosX").toFloat(&ok);
  if(!ok)
  {
    boError(230) << k_funcinfo << "Invalid value for CameraPosX tag" << endl;
    cameraposx = 0.0f;
  }
  cameraposy = root.attribute("CameraPosY").toFloat(&ok);
  if(!ok)
  {
    boError(230) << k_funcinfo << "Invalid value for CameraPosY tag" << endl;
    cameraposy = 0.0f;
  }
  cameraposz = root.attribute("CameraPosZ").toFloat(&ok);
  if(!ok)
  {
    boError(230) << k_funcinfo << "Invalid value for CameraPosZ tag" << endl;
    cameraposz = 0.0f;
  }
  mCameraPos.set(cameraposx, cameraposy, cameraposz);

  upx = root.attribute("UpX").toFloat(&ok);
  if(!ok)
  {
    boError(230) << k_funcinfo << "Invalid value for UpX tag" << endl;
    upx = 0.0f;
  }
  upy = root.attribute("UpY").toFloat(&ok);
  if(!ok)
  {
    boError(230) << k_funcinfo << "Invalid value for UpY tag" << endl;
    upy = 0.0f;
  }
  upz = root.attribute("UpZ").toFloat(&ok);
  if(!ok)
  {
    boError(230) << k_funcinfo << "Invalid value for UpZ tag" << endl;
    upz = 0.0f;
  }
  mUp.set(upx, upy, upz);

  setPositionDirty();
  setCameraChanged(true);
  return true;
 }

bool BoCamera::saveAsXML(QDomElement& root)
{
  root.setAttribute("LookAtX", lookAt().x());
  root.setAttribute("LookAtY", lookAt().y());
  root.setAttribute("LookAtZ", lookAt().z());

  root.setAttribute("CameraPosX", cameraPos().x());
  root.setAttribute("CameraPosY", cameraPos().y());
  root.setAttribute("CameraPosZ", cameraPos().z());

  root.setAttribute("UpX", up().x());
  root.setAttribute("UpY", up().y());
  root.setAttribute("UpZ", up().z());

  return true;
}

// FIXME: make it const!
BoMatrix BoCamera::rotationMatrix()
{
  // emulate gluLookAt()
  BoMatrix m;
  m.setLookAtRotation(cameraPos(), lookAt(), up());
  return m;
}

// FIXME: make it const!
BoQuaternion BoCamera::quaternion()
{
  // emulate gluLookAt()
  BoQuaternion q;
  q.setRotation(cameraPos(), lookAt(), up());
  return q;
}

void BoCamera::applyCameraToScene()
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glMultMatrixf(rotationMatrix().data());
  glTranslatef(-cameraPos().x(), -cameraPos().y(), -cameraPos().z());
  setCameraChanged(false);
}

const BoVector3Float& BoCamera::cameraPos()
{
  return mCameraPos;
}

const BoVector3Float& BoCamera::up()
{
  return mUp;
}

void BoCamera::setLookAt(const BoVector3Float& pos)
{
  mLookAt = pos;
  setPositionDirty();
  setCameraChanged(true);
}

void BoCamera::setCameraPos(const BoVector3Float& pos)
{
  mCameraPos = pos;
  setPositionDirty();
  setCameraChanged(true);
}

void BoCamera::setUp(const BoVector3Float& up)
{
  mUp = up;
  setPositionDirty();
  setCameraChanged(true);
}


/*****  BoGameCamera  *****/

BoGameCamera::BoGameCamera()
        : BoCamera()
{
  init();
}

BoGameCamera::BoGameCamera(const BosonCanvas* canvas)
        : BoCamera()
{
  init();
  mCanvas = canvas;
}


BoGameCamera& BoGameCamera::operator=(const BoGameCamera& c)
{
  BoCamera::operator=(c);
  mDistance = c.mDistance;
  mRotation = c.mRotation;
  mXRotation = c.mXRotation;

  mActualDistance = c.mActualDistance;

  mFree = c.mFree;
  mLimits = c.mLimits;

  mCanvas = c.mCanvas;
  setAutoCamera(new BoAutoGameCamera(*c.autoGameCamera()));

  return *this;
}

void BoGameCamera::init()
{
  setAutoCamera(new BoAutoGameCamera(this));
  mCanvas = 0;

  mDistance = 8.0f;
  mActualDistance = 8.0f;
  mRotation = 0.0f;
  mXRotation = 30.0f;
  mFree = false;
  mLimits = true;
}

void BoGameCamera::updateCamera()
{
  if(mFree)
  {
    // In free mode, rotation, radius, etc aren't used and all gluLookAt params
    // are set directly, so return here
    return;
  }
  BoMatrix matrix;
  matrix.rotate(mRotation, 0, 0, 1);
  matrix.rotate(mXRotation, 1, 0, 0);
  matrix.translate(0, 0, mActualDistance);
  BoVector3Float in(0, 0, 0), out;
  matrix.transform(&out, &in);
  float diffX, diffY;
  Bo3dTools::pointByRotation(&diffX, &diffY, 0, 0);


  // We don't want any validation here (we'd get infinite loops otherwise)
  BoCamera::setCameraPos(lookAt() + out);

  BoVector3Float upvector(0, 1, 0);
  matrix.transform(&out, &upvector);
  BoCamera::setUp(out);

  setPositionDirty(false);
  setCameraChanged(true);
}

void BoGameCamera::checkLookAtPosition()
{
  if(mFree || !mLimits)
  {
    // No restrictions
    return;
  }
  // We use canvas for checking here, so if we don't have it, return
  if(!mCanvas)
  {
    boDebug() << k_funcinfo << "NULL canvas" << endl;
    return;
  }

  // note that we use this in setGluLooKAt() as well, so don't use radius() and
  // rotation() here!
  bool changed = false;

  // workaround. AB: i believe cameraPos() and up() must be const.
  // We must first update lookAt position by calling
//  bool dirty = positionDirty();
//  setPositionDirty(false);
  BoVector3Float lookAt_ = lookAt();
//  setPositionDirty(dirty);

  if(lookAt_.x() < 0.0f)
  {
    lookAt_.setX(0.0f);
    changed = true;
  }
  else if(lookAt_.x() > (float)mCanvas->mapWidth())
  {
    lookAt_.setX((float)mCanvas->mapWidth());
    changed = true;
  }
  if(lookAt_.y() > 0.0f)
  {
    lookAt_.setY(0.0f);
    changed = true;
  }
  else if(lookAt_.y() < -((float)mCanvas->mapHeight()))
  {
    lookAt_.setY(-((float)mCanvas->mapHeight()));
    changed = true;
  }
  if(changed)
  {
    // We can't use setLookAt here, because it calls this method and we'd get
    //  infinite loop. So we use setGluLookAt();
    setGluLookAt(cameraPos(), lookAt_, up());
  }
}

void BoGameCamera::checkCameraPosition()
{
  if(mFree || !mLimits)
  {
    // No restrictions
    return;
  }
  // Maybe rename to checkCameraHeight() as it only checks height
  //  (z-coordinate)? Note that camera's height != z(), because z() only gives
  //  height measured from lookat point, so to get real height you'll have to do
  //  lookAt.z() + z()  (which should always be equal to just cameraPos.z())
  if(mCanvas)
  {
    float camposx = cameraPos().x();
    float camposy = -cameraPos().y();
    camposx = qMax(0.0f, qMin(camposx, (float)mCanvas->mapWidth()));
    camposy = qMax(0.0f, qMin(camposy, (float)mCanvas->mapHeight()));
    float groundz = mCanvas->heightAtPoint(camposx, camposy);
    if(cameraPos().z() < groundz + CAMERA_MIN_HEIGHT)
    {
      // Shorten the distance from the lookat point until the camera is enough
      //  above the ground.
      const int iterations = 25;
      const float step = 1 / (float)iterations;
      for(float factor = 1 - step; factor >= 0; factor -= step)
      {
        mActualDistance = qMax(CAMERA_MIN_DISTANCE, mDistance * factor);
        // Recalc camera pos
        setPositionDirty();
        BoVector3Float newcamerapos = cameraPos();
        // Find out ground height at the new camera pos
        camposx = cameraPos().x();
        camposy = -cameraPos().y();
        camposx = qMax(0.0f, qMin(camposx, (float)mCanvas->mapWidth()));
        camposy = qMax(0.0f, qMin(camposy, (float)mCanvas->mapHeight()));
        groundz = mCanvas->heightAtPoint(camposx, camposy);
        if(cameraPos().z() >= groundz + CAMERA_MIN_HEIGHT)
        {
          // Valid point found.
          break;
        }
      }
    }
    else
    {
      mActualDistance = mDistance;
    }
  }
}

void BoGameCamera::checkRotation()
{
  if(mFree || !mLimits)
  {
    // No restrictions
    return;
  }
  if(mRotation > 360.0f)
  {
    setRotation(mRotation - 360.0f);
  }
  else if(mRotation < 0.0f)
  {
    setRotation(mRotation + 360.0f);
  }
}

void BoGameCamera::checkXRotation()
{
  if(mFree || !mLimits)
  {
    // No restrictions
    return;
  }
  mXRotation = qMax(0, qMin(CAMERA_MAX_XROTATION, mXRotation));
}


bool BoGameCamera::saveAsXML(QDomElement& root)
{
 BoCamera::saveAsXML(root);
 root.setAttribute("Distance", distance());
 root.setAttribute("Rotation", rotation());
 root.setAttribute("XRotation", xRotation());
 return true;
}

bool BoGameCamera::loadFromXML(const QDomElement& root)
{
  bool ret = BoCamera::loadFromXML(root);

  bool ok;
  mRotation = root.attribute("Rotation").toFloat(&ok);
  if(!ok)
  {
    boError(230) << k_funcinfo << "Invalid value for Rotation tag" << endl;
    mRotation = 0.0f;
  }
  if(root.hasAttribute("Distance"))
  {
    mDistance = root.attribute("Distance").toFloat(&ok);
    if(!ok)
    {
      boError(230) << k_funcinfo << "Invalid value for Distance tag" << endl;
      mDistance = 8.0f;
    }
    mXRotation = root.attribute("XRotation").toFloat(&ok);
    if(!ok)
    {
      boError(230) << k_funcinfo << "Invalid value for XRotation tag" << endl;
      mXRotation = 30.0f;
    }
  }
  else
  {
    // Revert to defaults
    mDistance = 8.0f;
    mXRotation = 30.0f;
  }

  setPositionDirty();
  return ret;
}

const BoVector3Float& BoGameCamera::cameraPos()
{
  if(positionDirty())
  {
    updateCamera();
  }
  return BoCamera::cameraPos();
}

const BoVector3Float& BoGameCamera::up()
{
  if(positionDirty())
  {
    updateCamera();
  }
  return BoCamera::up();
}

void BoGameCamera::changeDistance(GLfloat diff)
{
  setDistance(mDistance + diff);
}

void BoGameCamera::changeXRotation(GLfloat diff)
{
  setXRotation(mXRotation + diff);
}

void BoGameCamera::changeRotation(GLfloat diff)
{
  setRotation(rotation() + diff);
}

void BoGameCamera::setLookAt(const BoVector3Float& pos)
{
  BO_CHECK_NULL_RET(mCanvas);
  // We must set lookAt point in 2 passes: first we set it using setLookAt(),
  //  then we validate it (this may change lookAt) and finally we calculate
  //  ground height (z) at validated lookAt point.
  BoCamera::setLookAt(pos);
  if(mFree || !mLimits)
  {
    // No restrictions
    return;
  }

  // Validate lookat point (this may change lookat)
  checkLookAtPosition();
  // Calculate new z
  float groundz = mCanvas->heightAtPoint(lookAt().x(), -lookAt().y());
  // Set lookat again. Note that this time, we don't do any validation, because
  //  only z-coordinate changed.
  BoCamera::setLookAt(BoVector3Float(lookAt().x(), lookAt().y(), groundz + CAMERA_LOOKAT_HEIGHT));
  checkCameraPosition();
  setCameraChanged(true);
}

void BoGameCamera::setCameraPos(const BoVector3Float& pos)
{
  BoCamera::setCameraPos(pos);

  // Validate position
  checkCameraPosition();
  setCameraChanged(true);
}

void BoGameCamera::setXRotation(GLfloat r)
{
  mXRotation = qMax(0, qMin(CAMERA_MAX_XROTATION, r));
  setPositionDirty();

  checkXRotation();

  // Validate camera's position
  checkCameraPosition();
  setCameraChanged(true);
}

void BoGameCamera::setRotation(GLfloat r)
{
  mRotation = r;
  setPositionDirty();

  // Makes sure 0 <= rotation < 360
  checkRotation();

  // Validates camera's position
  checkCameraPosition();
  setCameraChanged(true);
}

void BoGameCamera::setDistance(GLfloat dist)
{
  mDistance = qMin(CAMERA_MAX_DISTANCE, qMax(CAMERA_MIN_DISTANCE, dist));
  mActualDistance = mDistance;

  setPositionDirty();

  // Validate camera pos
  checkCameraPosition();
  setCameraChanged(true);
}

void BoGameCamera::setFreeMovement(bool free)
{
  if(mFree == free)
  {
    return;
  }

  mFree = free;

  if(!mFree)
  {
    // Update camera
    // TODO: maybe also check for limits?
    //  Actually, in free mode, camera should not accept any input, but until
    //  this will be implemented, player can set camera parameters (such as z)
    //  to higher values than allowed, and when camera comes out of free mode,
    //  those values won't be checked for.
    updateCamera();
    setCameraChanged(true);
  }
}

void BoGameCamera::setUseLimits(bool use)
{
  if(mLimits == use)
  {
    return;
  }

  mLimits = use;

  if(mLimits)
  {
    // Check for limits
    checkRotation();
    checkXRotation();
    checkLookAtPosition();
    checkCameraPosition();
    setCameraChanged(true);
  }
}


/*****  BoLightCamera  *****/

BoLightCamera::BoLightCamera(BoLight* light, BoContext* ctx) : BoCamera()
{
  mContext = ctx;
  mLight = light;
  if(!mContext)
  {
    BO_NULL_ERROR(mContext);
  }
  if(!mLight)
  {
    BO_NULL_ERROR(mLight);
  }
}

BoLightCamera::~BoLightCamera()
{
}

void BoLightCamera::setLightPos(const BoVector3Float& pos)
{
  setGluLookAt(pos, BoVector3Float(), BoVector3Float());
}

void BoLightCamera::setGluLookAt(const BoVector3Float& c, const BoVector3Float& l, const BoVector3Float& u)
{
  BoCamera::setGluLookAt(c, l, u);
  if(!mContext)
  {
    return;
  }
  if(!mLight)
  {
    return;
  }
  BoContext* old = BoContext::currentContext();
  mContext->makeCurrent();
  mLight->setPosition3(l * -1);

  if(old)
  {
    old->makeCurrent();
  }
}


/*
 * vim: et sw=2
 */
