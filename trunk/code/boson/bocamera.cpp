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

#include <qdom.h>

// Camera limits
#define CAMERA_MIN_Z BO_GL_NEAR_PLANE + 3
#define CAMERA_MAX_Z BO_GL_FAR_PLANE - 50
#define CAMERA_MAX_RADIUS 80

#include <GL/glu.h>

#include <math.h>

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
  mLookAt.set(0.0f, 0.0f, 0.0f);

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

void BoCamera::setGluLookAt(const BoVector3& lookAt, const BoVector3& cameraPos, const BoVector3& up)
{
  mLookAt = lookAt;
  mCameraPos = cameraPos;
  mUp = up;
}

void BoCamera::changeLookAt(const BoVector3& diff)
{
  setLookAt(lookAt() + diff);
}

bool BoCamera::loadFromXML(const QDomElement& root)
{
  bool ok;
  float lookatx, lookaty, lookatz;
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
  boDebug(260) << k_funcinfo << "Setting lookat to (" << lookatx << ", " << lookaty << ", " << lookatz << ")" << endl;
  mLookAt.set(lookatx, lookaty, lookatz);
  boDebug(260) << k_funcinfo << "lookat is now (" << lookAt().x() << ", " << lookAt().y() << ", " << lookAt().z() << ")" << endl;
  setPositionDirty();
  return true;
 }

bool BoCamera::saveAsXML(QDomElement& root)
{
 root.setAttribute("LookAtX", lookAt().x());
 root.setAttribute("LookAtY", lookAt().y());
 root.setAttribute("LookAtZ", lookAt().z());
 // TODO: save diffs too?
 return true;
}

void BoCamera::applyCameraToScene()
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  gluLookAt(cameraPos().x(), cameraPos().y(), cameraPos().z(),
      lookAt().x(), lookAt().y(), lookAt().z(),
      up().x(), up().y(), up().z());
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


BoGameCamera::BoGameCamera()
        : BoCamera()
{
  init();
}

BoGameCamera::BoGameCamera(GLfloat minX, GLfloat maxX, GLfloat minY, GLfloat maxY)
        : BoCamera()
{
  init();
  setMoveRect(minX, maxX, minY, maxY);
}


BoGameCamera& BoGameCamera::operator=(const BoGameCamera& c)
{
  BoCamera::operator=(c);
  mPosZ = c.mPosZ;
  mRotation = c.mRotation;
  mRadius = c.mRadius;

  mMinX = c.mMinX;
  mMaxX = c.mMaxX;
  mMinY = c.mMinY;
  mMaxY = c.mMaxY;
  setAutoCamera(new BoAutoGameCamera(*c.autoGameCamera()));

  return *this;
}

void BoGameCamera::init()
{
  setAutoCamera(new BoAutoGameCamera(this));
  initStatic();
  setMoveRect(-100000, 100000, -100000, 100000); // just very big values
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
  boConfig->addDynamicEntry(new BoConfigDoubleEntry(boConfig, "CameraMaxRadius",
      CAMERA_MAX_RADIUS));
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

float BoGameCamera::maxCameraRadius()
{
  initStatic();
  return (float)boConfig->doubleValue("CameraMaxRadius");
}

void BoGameCamera::updatePosition()
{
  float diffX = 0.0f;
  float diffY = 0.0f;
  float radius = this->radius();
  if(radius <= 0.02f)
  {
    // If radius is 0, up vector will be wrong so we change it
    radius = 0.02f;
  }
  Bo3dTools::pointByRotation(&diffX, &diffY, rotation(), radius);

  BoVector3 cameraPos(lookAt().x() + diffX, lookAt().y() + diffY, lookAt().z() + z());
  BoVector3 up(-diffX, -diffY, 0.0f);
  setGluLookAt(lookAt(), cameraPos, up);

  setPositionDirty(false);
}

float BoGameCamera::calculateNewZ(GLfloat diff) const
{
  // FIXME: z limits should depend on the ground height
  float newz = mPosZ + diff;
  if(newz < CAMERA_MIN_Z)
  {
    newz = CAMERA_MIN_Z;
  }
  else if(newz > CAMERA_MAX_Z)
  {
    newz = CAMERA_MAX_Z;
  }
  return newz;
}

void BoGameCamera::changeZ(GLfloat diff)
{
  // FIXME: z limits should depend on the ground height
  // Make sure new z-coordinate is within limits
  float newz = calculateNewZ(diff);

  // Change radius too, so that camera angle will remain same
  // TODO: maybe provide another method for that, e.g. changeZWithRadius().
  //  Then changeZ() would change _only_ z
  float factor = newz / mPosZ;
  setZ(newz);
  setRadius(radius() * factor);
}

float BoGameCamera::calculateNewRadius(GLfloat diff) const
{
  float radius = this->radius() + mPosZ / CAMERA_MAX_RADIUS * diff;
  // Make sure radius is within limits
  if(radius < 0.0f)
  {
    radius = 0.0f;
  }
  else if(radius > mPosZ)
  {
    radius = mPosZ;
  }
  return radius;
}

void BoGameCamera::changeRadius(GLfloat diff)
{
  // How much radius is changed depends on z position
  float radius = calculateNewRadius(diff);
  // Make sure radius is within limits
  if(radius < 0.0f)
  {
    radius = 0.0f;
  }
  else if(radius > mPosZ)
  {
    radius = mPosZ;
  }
  // Update
  setRadius(radius);
}

void BoGameCamera::changeRotation(GLfloat diff)
{
  setRotation(rotation() + diff);
}

void BoGameCamera::checkPosition()
{
  // note that we use this in setGluLooKAt() as well, so don't use radius() and
  // rotation() here!
  bool changed = false;

  // workaround. AB: i believe cameraPos() and up() must be const.
  bool dirty = positionDirty();
  setPositionDirty(false);
  BoVector3 cameraPos_ = cameraPos();
  BoVector3 lookAt_ = lookAt();
  BoVector3 up_ = up();
  setPositionDirty(dirty);

  if(lookAt().x() < mMinX)
  {
    lookAt_.setX(mMinX);
    changed = true;
  }
  else if(lookAt().x() > mMaxX)
  {
    lookAt_.setX(mMaxX);
    changed = true;
  }
  if(lookAt().y() < mMinY)
  {
    lookAt_.setY(mMinY);
    changed = true;
  }
  else if(lookAt().y() > mMaxY)
  {
    lookAt_.setY(mMaxY);
    changed = true;
  }
  if(changed)
  {
    setGluLookAt(lookAt_, cameraPos_, up_);
    setPositionDirty();
  }
}

void BoGameCamera::checkRotation()
{
  if(mRotation > 360.0f)
  {
    mRotation -= 360.0f;
    setPositionDirty();
  }
  else if(mRotation < 0.0f)
  {
    mRotation += 360.0f;
    setPositionDirty();
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
    updatePosition();
  }
  return BoCamera::cameraPos();
}

const BoVector3& BoGameCamera::up()
{
  if(positionDirty())
  {
    updatePosition();
  }
  return BoCamera::up();
}

void BoGameCamera::setLookAt(const BoVector3& pos)
{
  BoCamera::setLookAt(pos);
  checkPosition();
}

void BoGameCamera::setRadius(GLfloat r)
{
  mRadius = r;
  setPositionDirty();
}

void BoGameCamera::setRotation(GLfloat r)
{
  boDebug(230) << k_funcinfo << endl;
  mRotation = r;
  checkRotation();
  setPositionDirty();
}

void BoGameCamera::setZ(GLfloat z)
{
  mPosZ = z;
  setPositionDirty();
}

void BoGameCamera::setMoveRect(GLfloat minX, GLfloat maxX, GLfloat minY, GLfloat maxY)
{
  boDebug(230) << k_funcinfo << "(" << minX << "; " << maxX << ";  " << minY << "; " << maxY << ")" << endl;
  mMinX = minX;
  mMaxX = maxX;
  mMinY = minY;
  mMaxY = maxY;
}


/*
 * vim: et sw=2
 */
