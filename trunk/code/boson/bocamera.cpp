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

  mLookAtDiff = c.mLookAtDiff;
  mCommitTime = c.mCommitTime;
  mRemainingTime = c.mRemainingTime;
  mMoveMode = c.mMoveMode;
  mMovedAmount = c.mMovedAmount;

  setPositionDirty();

  return *this;
}

void BoCamera::init()
{
  mLookAt.set(0.0f, 0.0f, 0.0f);
  mMoveMode = Linear;

  resetDifferences();

  setPositionDirty();
}

void BoCamera::setGluLookAt(const BoVector3& lookAt, const BoVector3& cameraPos, const BoVector3& up)
{
  mLookAt = lookAt;
  mCameraPos = cameraPos;
  mUp = up;
  // checkPosition();
}

void BoCamera::changeLookAt(const BoVector3& diff, bool now)
{
  if(now)
  {
    mLookAt += diff;
    checkPosition();
    setPositionDirty();
  }
  else
  {
    mLookAtDiff = diff;
  }
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
  if(positionDirty())
  {
    updatePosition();
  }
  return mCameraPos;
}

const BoVector3& BoCamera::up()
{
  if(positionDirty())
  {
    updatePosition();
  }
  return mUp;
}

void BoCamera::setLookAt(const BoVector3& pos, bool now)
{
  if(now)
  {
    mLookAt = pos;
    checkPosition();
    setPositionDirty();
  }
  else
  {
    mLookAtDiff = pos - mLookAt;
  }
}

void BoCamera::commitChanges(int ticks)
{
  mCommitTime = ticks;
  mRemainingTime = ticks;
  mMovedAmount = 0.0f;
  if(ticks <= 0)
  {
    // Advance immediately. commitTime still has to be at least 1
    mCommitTime = 1;
    mRemainingTime = 1;
    advance();
  }
}

void BoCamera::setMoveMode(MoveMode mode)
{
  boDebug(230) << k_funcinfo << "mode: " << mode << endl;
  mMoveMode = mode;
}

float BoCamera::moveFactor() const
{
  float factor = 1.0f;
  if(moveMode() == Sinusoidal)
  {
    // FIXME: make this more simple!
    factor = (-cos((mCommitTime - remainingTime() + 1) / (float)mCommitTime * M_PI) + 1) / 2 - mMovedAmount;
    boDebug(230) << k_funcinfo << "Sinusoidal movement; mCommitTime: " << mCommitTime << "; factor: " << factor << endl;
  }
  else if(moveMode() == SinusoidEnd)
  {
    // FIXME: make this more simple!
    factor = -cos(M_PI_2 + (mCommitTime - remainingTime() + 1) / (float)mCommitTime * M_PI_2) - mMovedAmount;
    boDebug(230) << k_funcinfo << "Sinusoidal movement; mCommitTime: " << mCommitTime << "; factor: " << factor << endl;
  }
  else
  {
    factor = 1.0 / mCommitTime;
    boDebug(230) << k_funcinfo << "Linear movement; mCommitTime: " << mCommitTime << "; factor: " << factor << endl;
  }
  return factor;
}

void BoCamera::advance()
{
  if(remainingTime() <= 0)
  {
    return;
  }
  bool changed = advance2();

  if(!changed)
  {
//    boError(230) << k_funcinfo << "remainingTime: " << reaminingTime() << ", but no changes ?!" << endl;
    mRemainingTime = 0;
    mCommitTime = 0;
    mMovedAmount = 0.0f;
  }
  else
  {
    setPositionDirty();
    mRemainingTime--;
    if(mRemainingTime == 0)
    {
      // Failsafe
      resetDifferences();
    }
  }
}

bool BoCamera::advance2()
{
  if(remainingTime() <= 0)
  {
    return false;
  }

  boDebug(230) << k_funcinfo << "mRemainingTime: " << remainingTime() << endl;

  // How much of differences to add
  float factor = moveFactor();
  mMovedAmount += factor;
  boDebug(230) << k_funcinfo << "factor: " << factor << ";  movedAmount: " << mMovedAmount << endl;
  bool changed = false;

  if(!mLookAtDiff.isNull())
  {
    // How much lookAt point will move
    BoVector3 lookAtChange(mLookAtDiff * factor);
    // Change lookAt point and difference
    mLookAt.add(lookAtChange);
    checkPosition();
    changed = true;
  }

  return changed;
}

void BoCamera::resetDifferences()
{
  mLookAtDiff.reset();
  mCommitTime = 0;
  mRemainingTime = 0;
  mMovedAmount = 0.0f;
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

  mPosZDiff = c.mPosZDiff;
  mRotationDiff = c.mRotationDiff;
  mRadiusDiff = c.mRadiusDiff;

  mMinX = c.mMinX;
  mMaxX = c.mMaxX;
  mMinY = c.mMinY;
  mMaxY = c.mMaxY;
  return *this;
}

void BoGameCamera::init()
{
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

void BoGameCamera::changeZ(GLfloat diff, bool now)
{
  // FIXME: z limits should depend on the ground height
  // Make sure new z-coordinate is within limits
  float newz = mPosZ + diff;
  if(newz < CAMERA_MIN_Z)
  {
    newz = CAMERA_MIN_Z;
  }
  else if(newz > CAMERA_MAX_Z)
  {
    newz = CAMERA_MAX_Z;
  }
  // Change radius too, so that camera angle will remain same
  // TODO: maybe provide another method for that, e.g. changeZWithRadius().
  //  Then changeZ() would change _only_ z
  float factor = newz / mPosZ;
  if(now)
  {
    mPosZ = newz;
    mRadius = radius() * factor;
    setPositionDirty();
  }
  else
  {
    mPosZDiff = newz;
    mRadiusDiff = radius() * factor;
  }
}

void BoGameCamera::changeRadius(GLfloat diff, bool now)
{
  // How much radius is changed depends on z position
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
  // Update
  if(now)
  {
    mRadius = radius;
    setPositionDirty();
  }
  else
  {
    mRadiusDiff = radius;
  }
}

void BoGameCamera::changeRotation(GLfloat diff, bool now)
{
  if(now)
  {
    mRotation += diff;
    checkRotation();
    setPositionDirty();
  }
  else
  {
    mRotationDiff = diff;
  }
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
  bool ret = loadFromXML(root);

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

void BoGameCamera::setRadius(GLfloat r, bool now)
{
  if(now)
  {
    mRadius = r;
    setPositionDirty();
  }
  else
  {
    mRadiusDiff = r - mRadius;
  }
}

void BoGameCamera::setRotation(GLfloat r, bool now)
{
  boDebug(230) << k_funcinfo << endl;
  if(now)
  {
    mRotation = r;
    checkRotation();
    setPositionDirty();
  }
  else
  {
    mRotationDiff = r - mRotation;
    boDebug(230) << k_funcinfo << "rotation diff set to: " << mRotationDiff << endl;
  }
}

void BoGameCamera::setZ(GLfloat z, bool now)
{
  if(now)
  {
    mPosZ = z;
    setPositionDirty();
  }
  else
  {
    mPosZDiff = z - mPosZ;
  }
}

void BoGameCamera::setMoveRect(GLfloat minX, GLfloat maxX, GLfloat minY, GLfloat maxY)
{
  boDebug(230) << k_funcinfo << "(" << minX << "; " << maxX << ";  " << minY << "; " << maxY << ")" << endl;
  mMinX = minX;
  mMaxX = maxX;
  mMinY = minY;
  mMaxY = maxY;
}

bool BoGameCamera::advance2()
{
  bool changed = BoCamera::advance2();
  if(remainingTime() <= 0)
  {
    return changed;
  }

  // How much of differences to add
  float factor = moveFactor();

  if(mPosZDiff)
  {
    float diff = (mPosZDiff * factor);
    mPosZ += diff;
    changed = true;
  }

  if(mRotationDiff)
  {
    float diff = (mRotationDiff * factor);
    mRotation += diff;
    checkRotation();
    changed = true;
  }

  if(mRadiusDiff)
  {
    float diff = (mRadiusDiff * factor);
    mRadius += diff;
    changed = true;
  }
  return changed;
}

void BoGameCamera::resetDifferences()
{
  mPosZDiff = 0;
  mRotationDiff = 0;
  mRadiusDiff = 0;
}



/*
 * vim: et sw=2
 */
