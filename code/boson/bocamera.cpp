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

#include "bocamera.h"

#include "boautocamera.h"
#include "defines.h"
#include "bosonconfig.h"
#include "bodebug.h"
#include "bosonglwidget.h" // BoContext
#include "bolight.h"
#include "bosoncanvas.h"

#include <qdom.h>

// Camera limits
// These are min/max height of the camera (between camera's _position_, not
//  lookat point, and ground)
#define CAMERA_MIN_Z 2
#define CAMERA_MAX_Z 100
#define CAMERA_MAX_RADIUS 70
// This is tangens of the minimum camera angle allowed (if angle is 90, camera
//  is looking straight at the ground from top; if angle is 0, camera is looking
//  at the horizon)
// This is tan(30 degrees) atm, i.e. min camera angle is 30 degrees
//#define MIN_CAMERA_ANGLE_TAN 0.57735027
// 30 degrees limit is fatal to performance atm, so we use 45 degrees instead
#define MIN_CAMERA_ANGLE_TAN 1.0
// This helps to avoid tiny rounding errors which would be causing infinite loops
#define CAMERA_Z_ROUNDING_ERROR 0.001

#include <GL/glu.h>

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

void BoCamera::setGluLookAt(const BoVector3& cameraPos, const BoVector3& lookAt, const BoVector3& up)
{
  mLookAt = lookAt;
  mCameraPos = cameraPos;
  mUp = up;
}

void BoCamera::changeLookAt(const BoVector3& diff)
{
  setLookAt(lookAt() + diff);
}

void BoCamera::changeCameraPos(const BoVector3& diff)
{
  setCameraPos(cameraPos() + diff);
}

void BoCamera::changeUp(const BoVector3& diff)
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
  return true;
 }

bool BoCamera::saveAsXML(QDomElement& root)
{
  root.setAttribute("LookAtX", lookAt().x());
  root.setAttribute("LookAtY", lookAt().y());
  root.setAttribute("LookAtZ", lookAt().z());

  root.setAttribute("CameraPosX", cameraPos().x());
  root.setAttribute("CameraPosX", cameraPos().y());
  root.setAttribute("CameraPosX", cameraPos().z());

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
}

const BoVector3& BoCamera::cameraPos()
{
  return mCameraPos;
}

const BoVector3& BoCamera::up()
{
  return mUp;
}

void BoCamera::setLookAt(const BoVector3& pos)
{
  mLookAt = pos;
  setPositionDirty();
}

void BoCamera::setCameraPos(const BoVector3& pos)
{
  mCameraPos = pos;
  setPositionDirty();
}

void BoCamera::setUp(const BoVector3& up)
{
  mUp = up;
  setPositionDirty();
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
  mPosZ = c.mPosZ;
  mRotation = c.mRotation;
  mRadius = c.mRadius;

  mCanvas = c.mCanvas;
  setAutoCamera(new BoAutoGameCamera(*c.autoGameCamera()));

  return *this;
}

void BoGameCamera::init()
{
  setAutoCamera(new BoAutoGameCamera(this));
  initStatic();
  mCanvas = 0;
  mPosZ = 8.0f;
  mRotation = 0.0f;
  mRadius = 5.0f;
}

void BoGameCamera::initStatic()
{
  static bool initialized = false;
  if(initialized)
  {
    return;
  }
  initialized = true;
  boConfig->addDynamicEntry(new BoConfigDoubleEntry(boConfig, "CameraMinZ",
      CAMERA_MIN_Z));
  boConfig->addDynamicEntry(new BoConfigDoubleEntry(boConfig, "CameraMaxZ",
      CAMERA_MAX_Z));
}

float BoGameCamera::minCameraZ()
{
  initStatic();
  return (float)boConfig->doubleValue("CameraMinZ");
}

float BoGameCamera::maxCameraZ()
{
  initStatic();
  return (float)boConfig->doubleValue("CameraMaxZ");
}

void BoGameCamera::updateCamera()
{
  boDebug() << k_funcinfo << endl;
  float diffX = 0.0f;
  float diffY = 0.0f;
  float radius = this->radius();
  if(radius <= 0.02f)
  {
    // If radius is 0, up vector will be wrong so we change it
    radius = 0.02f;
  }
  Bo3dTools::pointByRotation(&diffX, &diffY, rotation(), radius);

  // We don't want any validation here (we'd get infinite loops otherwise)
  BoCamera::setCameraPos(BoVector3(lookAt().x() + diffX, lookAt().y() + diffY, lookAt().z() + z()));
  BoCamera::setUp(BoVector3(-diffX, -diffY, 0.0f));

  setPositionDirty(false);
}

void BoGameCamera::checkLookAtPosition()
{
  boDebug() << k_funcinfo << endl;
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
  BoVector3 lookAt_ = lookAt();
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
  boDebug() << k_funcinfo << endl;
  // Maybe rename to checkCameraHeight() as it only checks height
  //  (z-coordinate)? Note that camera's height != z(), because z() only gives
  //  height measured from lookat point, so to get real height you'll have to do
  //  lookAt.z() + z()  (which should always be equal to just cameraPos.z())
  if(mCanvas)
  {
    float camposx = cameraPos().x() * BO_TILE_SIZE;
    float camposy = -cameraPos().y() * BO_TILE_SIZE;
    camposx = QMAX(0, QMIN(camposx, mCanvas->mapWidth() * BO_TILE_SIZE));
    camposy = QMAX(0, QMIN(camposy, mCanvas->mapHeight() * BO_TILE_SIZE));
    float groundz = mCanvas->heightAtPoint(camposx, camposy);
    boDebug() << k_funcinfo << "ground z at (" << camposx << "; " << camposy << "): " << groundz << endl;
    // FIXME: this assumes that lookAt.z() == 0, which may not always be the case
    // TODO: maybe also change radius to keep camera's angle constant
    if(cameraPos().z() < groundz + minCameraZ())
    {
      // We may have tiny rounding errors here, so we set z to little bigger
      //  number than the limit in order to avoid infinite loop
      //  (otherwise we may have  groundz + minCameraZ() = 0.0000001, which will
      //  be rounded to 0 when calling setZ(), and then setZ() sets z to 0 and
      //  this function compares values and finds out that 0 < 0.0000001, so it
      //  calls setZ() ... )
      setZ(groundz + minCameraZ() + CAMERA_Z_ROUNDING_ERROR);
    }
    else if(cameraPos().z() > groundz + maxCameraZ())
    {
      setZ(groundz + maxCameraZ() - CAMERA_Z_ROUNDING_ERROR);
    }
  }
}

void BoGameCamera::checkRotation()
{
  boDebug() << k_funcinfo << endl;
  if(mRotation > 360.0f)
  {
    setRotation(mRotation - 360.0f);
  }
  else if(mRotation < 0.0f)
  {
    setRotation(mRotation + 360.0f);
  }
}

void BoGameCamera::checkRadius()
{
  boDebug() << k_funcinfo << endl;
  // Make sure radius is within limits
  if(radius() < 0.0f)
  {
    setRadius(0.0f);
  }
  else if(radius() * MIN_CAMERA_ANGLE_TAN > z())
  {
    setRadius(z() / MIN_CAMERA_ANGLE_TAN);
  }
}

bool BoGameCamera::saveAsXML(QDomElement& root)
{
 BoCamera::saveAsXML(root);
 root.setAttribute("PosZ", z());
 root.setAttribute("Rotation", rotation());
 root.setAttribute("Radius", radius());
 // TODO: save diffs too?
 return true;
}

bool BoGameCamera::loadFromXML(const QDomElement& root)
{
  bool ret = BoCamera::loadFromXML(root);

  bool ok;
  mPosZ = root.attribute("PosZ").toFloat(&ok);
  if(!ok)
  {
    boError(230) << k_funcinfo << "Invalid value for PosZ tag" << endl;
    mPosZ = 0.0f;
  }
  float rotation = root.attribute("Rotation").toFloat(&ok);
  if(!ok)
  {
    boError(230) << k_funcinfo << "Invalid value for Rotation tag" << endl;
    rotation = 0.0f;
  }
  mRotation = rotation;
  float radius = root.attribute("Radius").toFloat(&ok);
  if(!ok)
  {
    boError(230) << k_funcinfo << "Invalid value for Radius tag" << endl;
    radius = 0.0f;
  }
  mRadius = radius;
  setPositionDirty();
  return ret;
}

const BoVector3& BoGameCamera::cameraPos()
{
  if(positionDirty())
  {
    updateCamera();
  }
  return BoCamera::cameraPos();
}

const BoVector3& BoGameCamera::up()
{
  if(positionDirty())
  {
    updateCamera();
  }
  return BoCamera::up();
}

void BoGameCamera::changeZ(GLfloat diff)
{
  boDebug() << k_funcinfo << "diff: " << diff << endl;
  // Calculate new z
  float oldz = mPosZ;
  float newz = oldz + diff;
  float factor = newz / oldz;
  boDebug() << k_funcinfo << "oldz: " << oldz << "; newz: " << newz << "; factor: " << factor << endl;

  // Set new z coordinate (also validates camera's position (i.e. makes sure
  //  it's not inside the terrain))
  setZ(newz);

  if(mPosZ != newz)
  {
    boDebug() << k_funcinfo << "z changed to: " << mPosZ << endl;
    if(mPosZ == oldz)
    {
      // Special case - z coordinate didn't change. Probably camera is at it's
      //  max/min height. Return.
      return;
    }
    // checkCameraPosition changed z coordinate (it was set to too low/high).
    // recalculate z change factor
    factor = mPosZ / oldz;
    boDebug() << k_funcinfo << "new factor: " << factor << endl;
  }

  // Change radius too, so that camera angle will remain same
  // TODO: maybe provide another method for that, e.g. changeZWithRadius().
  //  Then changeZ() would change _only_ z
  setRadius(mRadius * factor);
}

void BoGameCamera::changeRadius(GLfloat diff)
{
  boDebug() << k_funcinfo << endl;
  // How much radius is changed depends on z position
  float radius = this->radius() + mPosZ / CAMERA_MAX_RADIUS * diff;
  // Update radius
  setRadius(radius);
}

void BoGameCamera::changeRotation(GLfloat diff)
{
  boDebug() << k_funcinfo << endl;
  setRotation(rotation() + diff);
}

void BoGameCamera::setLookAt(const BoVector3& pos)
{
  boDebug() << k_funcinfo << endl;
  BoCamera::setLookAt(pos);

  // Validate lookat point and camera's position
  checkLookAtPosition();
  checkCameraPosition();
}

void BoGameCamera::setCameraPos(const BoVector3& pos)
{
  BoCamera::setCameraPos(pos);

  // Validate position
  checkCameraPosition();
}

void BoGameCamera::setRadius(GLfloat r)
{
  boDebug() << k_funcinfo << endl;
  mRadius = r;
  setPositionDirty();

  // Make sure new radius is valid
  checkRadius();

  // Validate camera's position
  checkCameraPosition();
}

void BoGameCamera::setRotation(GLfloat r)
{
  boDebug() << k_funcinfo << endl;
  boDebug(230) << k_funcinfo << endl;
  mRotation = r;
  setPositionDirty();

  // Makes sure 0 <= rotation < 360
  checkRotation();

  // Validates camera's position
  checkCameraPosition();
}

void BoGameCamera::setZ(GLfloat z)
{
  boDebug() << k_funcinfo << endl;
  mPosZ = z;
  setPositionDirty();

  // Validate camera pos
  checkCameraPosition();
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

void BoLightCamera::setLightPos(const BoVector3& pos)
{
  setGluLookAt(pos, BoVector3(), BoVector3());
}

void BoLightCamera::setGluLookAt(const BoVector3& c, const BoVector3& l, const BoVector3& u)
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
