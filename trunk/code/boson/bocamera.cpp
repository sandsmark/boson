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

BoCamera::BoCamera(GLfloat minX, GLfloat maxX, GLfloat minY, GLfloat maxY)
{
  init();
  setMoveRect(minX, maxX, minY, maxY);
}

BoCamera& BoCamera::operator=(const BoCamera& c)
{
  mPosZ = c.mPosZ;
  mLookAt = c.mLookAt;
  mRotation = c.mRotation;
  mRadius = c.mRadius;

  mMinX = c.mMinX;
  mMaxX = c.mMaxX;
  mMinY = c.mMinY;
  mMaxY = c.mMaxY;

  mLookAtDiff = c.mLookAtDiff;
  mPosZDiff = c.mPosZDiff;
  mRotationDiff = c.mRotationDiff;
  mRadiusDiff = c.mRadiusDiff;
  mCommitTime = c.mCommitTime;
  mRemainingTime = c.mRemainingTime;
  mMoveMode = c.mMoveMode;
  mMovedAmount = c.mMovedAmount;

  setPositionDirty();

  return *this;
}

void BoCamera::init()
{
  initStatic();
  setMoveRect(-100000, 100000, -100000, 100000); // just very big values
  mLookAt.set(0.0f, 0.0f, 0.0f);
  mPosZ = 8.0f;
  mRotation = 0.0f;
  mRadius = 5.0f;
  mMoveMode = Linear;

  resetDifferences();

  setPositionDirty();
}

void BoCamera::initStatic()
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

float BoCamera::minCameraZ()
{
  initStatic();
  return (float)boConfig->doubleValue("CameraMinZ");
}

float BoCamera::maxCameraZ()
{
  initStatic();
  return (float)boConfig->doubleValue("CameraMaxZ");
}

float BoCamera::maxCameraRadius()
{
  initStatic();
  return (float)boConfig->doubleValue("CameraMaxRadius");
}

void BoCamera::setGluLookAt(const BoVector3& lookAt, const BoVector3& cameraPos, const BoVector3& up)
{
  mLookAt = lookAt;
  mCameraPos = cameraPos;
  mUp = up;
  // checkPosition();
}

void BoCamera::changeZ(GLfloat diff, bool now)
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

void BoCamera::changeRadius(GLfloat diff, bool now)
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

void BoCamera::changeRotation(GLfloat diff, bool now)
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

void BoCamera::loadFromXML(const QDomElement& root)
{
  bool ok;
  float lookatx, lookaty, lookatz;
  lookatx = root.attribute("LookAtX").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for LookAtX tag" << endl;
    lookatx = 0.0f;
  }
  lookaty = root.attribute("LookAtY").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for LookAtY tag" << endl;
    lookaty = 0.0f;
  }
  lookatz = root.attribute("LookAtZ").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for LookAtZ tag" << endl;
    mPosZ = 0.0f;
  }
  mPosZ = root.attribute("PosZ").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for PosZ tag" << endl;
    mPosZ = 0.0f;
  }
  float rotation = root.attribute("Rotation").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for Rotation tag" << endl;
    rotation = 0.0f;
  }
  mRotation = rotation;
  float radius = root.attribute("Radius").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for Radius tag" << endl;
    radius = 0.0f;
  }
  mRadius = radius;
  boDebug(260) << k_funcinfo << "Setting lookat to (" << lookatx << ", " << lookaty << ", " << lookatz << ")" << endl;
  mLookAt.set(lookatx, lookaty, lookatz);
  boDebug(260) << k_funcinfo << "lookat is now (" << lookAt().x() << ", " << lookAt().y() << ", " << lookAt().z() << ")" << endl;
  setPositionDirty();
 }

void BoCamera::saveAsXML(QDomElement& root)
{
 root.setAttribute("LookAtX", lookAt().x());
 root.setAttribute("LookAtY", lookAt().y());
 root.setAttribute("LookAtZ", lookAt().z());
 root.setAttribute("PosZ", z());
 root.setAttribute("Rotation", rotation());
 root.setAttribute("Radius", radius());
 // TODO: save diffs too?
}

void BoCamera::checkPosition()
{
  // note that we use this in setGluLooKAt() as well, so don't use radius() and
  // rotation() here!
  bool changed = false;
  if(lookAt().x() < mMinX)
  {
    mLookAt.setX(mMinX);
    changed = true;
  }
  else if(lookAt().x() > mMaxX)
  {
    mLookAt.setX(mMaxX);
    changed = true;
  }
  if(lookAt().y() < mMinY)
  {
    mLookAt.setY(mMinY);
    changed = true;
  }
  else if(lookAt().y() > mMaxY)
  {
    mLookAt.setY(mMaxY);
    changed = true;
  }
  if(changed)
  {
    setPositionDirty();
  }
}

void BoCamera::checkRotation()
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

void BoCamera::applyCameraToScene()
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  gluLookAt(cameraPos().x(), cameraPos().y(), cameraPos().z(),
      lookAt().x(), lookAt().y(), lookAt().z(),
      up().x(), up().y(), up().z());
}

void BoCamera::updatePosition()
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

  // Position of camera
  mCameraPos.set(lookAt().x() + diffX, lookAt().y() + diffY, lookAt().z() + z());
  // up vector (points straight up in viewport)
  mUp.set(-diffX, -diffY, 0.0f);

  mPosDirty = false;
}

const BoVector3& BoCamera::cameraPos()
{
  if(mPosDirty)
  {
    updatePosition();
  }
  return mCameraPos;
}

const BoVector3& BoCamera::up()
{
  if(mPosDirty)
  {
    updatePosition();
  }
  return mUp;
}

void BoCamera::setRadius(GLfloat r, bool now)
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

void BoCamera::setRotation(GLfloat r, bool now)
{
  boDebug() << k_funcinfo << endl;
  if(now)
  {
    mRotation = r;
    checkRotation();
    setPositionDirty();
  }
  else
  {
    mRotationDiff = r - mRotation;
    boDebug() << k_funcinfo << "rotation diff set to: " << mRotationDiff << endl;
  }
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

void BoCamera::setZ(GLfloat z, bool now)
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

void BoCamera::setMoveRect(GLfloat minX, GLfloat maxX, GLfloat minY, GLfloat maxY)
{
  boDebug() << k_funcinfo << "(" << minX << "; " << maxX << ";  " << minY << "; " << maxY << ")" << endl;
  mMinX = minX;
  mMaxX = maxX;
  mMinY = minY;
  mMaxY = maxY;
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
  boDebug() << k_funcinfo << "mode: " << mode << endl;
  mMoveMode = mode;
}

void BoCamera::advance()
{
  if(mRemainingTime <= 0)
  {
    return;
  }

  boDebug() << k_funcinfo << "mRemainingTime: " << mRemainingTime << endl;

  // How much of differences to add
  float factor;
  if(mMoveMode == Sinusoidal)
  {
    // FIXME: make this more simple!
    factor = (-cos((mCommitTime - mRemainingTime + 1) / (float)mCommitTime * M_PI) + 1) / 2 - mMovedAmount;
    boDebug() << k_funcinfo << "Sinusoidal movement; mCommitTime: " << mCommitTime << "; factor: " << factor << endl;
  }
  else
  {
    factor = 1.0 / mCommitTime;
    boDebug() << k_funcinfo << "Linear movement; mCommitTime: " << mCommitTime << "; factor: " << factor << endl;
  }
  mMovedAmount += factor;
  boDebug() << k_funcinfo << "factor: " << factor << ";  movedAmount: " << mMovedAmount << endl;
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

  if(!changed)
  {
    boError() << k_funcinfo << "remainingTime: " << mRemainingTime << ", but no changes ?!" << endl;
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

void BoCamera::resetDifferences()
{
  mLookAtDiff.reset();
  mPosZDiff = 0;
  mRotationDiff = 0;
  mRadiusDiff = 0;
  mCommitTime = 0;
  mRemainingTime = 0;
  mMovedAmount = 0.0f;
}
