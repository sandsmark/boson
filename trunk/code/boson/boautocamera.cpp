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

#include "boautocamera.h"

#include "bocamera.h"
#include "defines.h"
#include "bodebug.h"

#include <qdom.h>

#include <math.h>

BoAutoCamera::BoAutoCamera(BoCamera* camera)
{
  init();
  mCamera = camera;
}

BoAutoCamera& BoAutoCamera::operator=(const BoAutoCamera& c)
{
  mCamera = c.mCamera;
  mLookAtDiff = c.mLookAtDiff;
  mCommitTime = c.mCommitTime;
  mRemainingTime = c.mRemainingTime;
  mMoveMode = c.mMoveMode;
  mMovedAmount = c.mMovedAmount;

  return *this;
}

void BoAutoCamera::init()
{
  mCamera = 0;
  mMoveMode = Linear;

  resetDifferences();
}

void BoAutoCamera::changeLookAt(const BoVector3Float& diff)
{
  mLookAtDiff = diff;
}

void BoAutoCamera::changeUp(const BoVector3Float& diff)
{
  mUpDiff = diff;
}

void BoAutoCamera::changeCameraPos(const BoVector3Float& diff)
{
  mCameraPosDiff = diff;
}

void BoAutoCamera::setLookAt(const BoVector3Float& pos)
{
  mLookAtDiff = pos - camera()->lookAt();
}

void BoAutoCamera::setUp(const BoVector3Float& pos)
{
  mUpDiff = pos - camera()->up();
}

void BoAutoCamera::setCameraPos(const BoVector3Float& pos)
{
  mCameraPosDiff = pos - camera()->cameraPos();
}

void BoAutoCamera::commitChanges(int ticks)
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

void BoAutoCamera::setMoveMode(MoveMode mode)
{
  boDebug(230) << k_funcinfo << "mode: " << mode << endl;
  mMoveMode = mode;
}

float BoAutoCamera::moveFactor() const
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

void BoAutoCamera::advance()
{
  if(remainingTime() <= 0)
  {
    return;
  }
  bool changed = advance2();

  // FIXME: cache factor!
  float factor = moveFactor();
  mMovedAmount += factor;

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

bool BoAutoCamera::advance2()
{
  if(remainingTime() <= 0)
  {
    return false;
  }

  boDebug(230) << k_funcinfo << "mRemainingTime: " << remainingTime() << endl;

  // How much of differences to add
  float factor = moveFactor();
  boDebug(230) << k_funcinfo << "factor: " << factor << ";  movedAmount: " << mMovedAmount << endl;
  bool changed = false;

  if(!mLookAtDiff.isNull())
  {
    // How much lookAt point will move
    BoVector3Float lookAtChange(mLookAtDiff * factor);
    // Change lookAt point and difference
    camera()->setLookAt(camera()->mLookAt + lookAtChange);
    changed = true;
  }
  if(!mUpDiff.isNull())
  {
    // How much lookAt point will move
    BoVector3Float upChange(mUpDiff * factor);
    // Change lookAt point and difference
    camera()->setUp(camera()->mUp + upChange);
    changed = true;
  }
  if(!mCameraPosDiff.isNull())
  {
    // How much lookAt point will move
    BoVector3Float cameraPosChange(mCameraPosDiff * factor);
    // Change lookAt point and difference
    camera()->setCameraPos(camera()->mCameraPos + cameraPosChange);
    changed = true;
  }

  return changed;
}

void BoAutoCamera::resetDifferences()
{
  mLookAtDiff.reset();
  mUpDiff.reset();
  mCameraPosDiff.reset();
  mCommitTime = 0;
  mRemainingTime = 0;
  mMovedAmount = 0.0f;
}

void BoAutoCamera::setPositionDirty(bool d)
{
  camera()->setPositionDirty(d);
}


bool BoAutoCamera::saveAsXML(QDomElement& root)
{
  Q_UNUSED(root);
  return true;
}

bool BoAutoCamera::loadFromXML(const QDomElement& root)
{
  Q_UNUSED(root);
  return true;
}

BoAutoGameCamera::BoAutoGameCamera(BoGameCamera* camera)
        : BoAutoCamera(camera)
{
  init();
}

BoAutoGameCamera& BoAutoGameCamera::operator=(const BoAutoGameCamera& c)
{
  BoAutoCamera::operator=(c);
  mPosZDiff = c.mPosZDiff;
  mRotationDiff = c.mRotationDiff;
  mRadiusDiff = c.mRadiusDiff;
  return *this;
}

void BoAutoGameCamera::init()
{
  mPosZDiff = 0.0f;
  mRotationDiff = 0.0f;
  mRadiusDiff = 0.0f;
}

void BoAutoGameCamera::changeZ(GLfloat diff)
{
  float newz = gameCamera()->z() + diff;

  // Change radius too, so that camera angle will remain same
  // TODO: maybe provide another method for that, e.g. changeZWithRadius().
  //  Then changeZ() would change _only_ z
  float factor = newz / gameCamera()->z();
  mPosZDiff = newz;
  mRadiusDiff = gameCamera()->radius() * factor;
}

void BoAutoGameCamera::changeRadius(GLfloat diff)
{
  float radius = gameCamera()->radius() + diff;
  mRadiusDiff = radius;
}

void BoAutoGameCamera::changeRotation(GLfloat diff)
{
  mRotationDiff = diff;
}

bool BoAutoGameCamera::saveAsXML(QDomElement& root)
{
 bool ret = BoAutoCamera::saveAsXML(root);
 // TODO: save diffs too?
 return ret;
}

bool BoAutoGameCamera::loadFromXML(const QDomElement& root)
{
  bool ret = BoAutoCamera::loadFromXML(root);
  return ret;
}

void BoAutoGameCamera::setRadius(GLfloat r)
{
  mRadiusDiff = r - gameCamera()->radius();
}

void BoAutoGameCamera::setRotation(GLfloat r)
{
  mRotationDiff = r - gameCamera()->rotation();
  boDebug(230) << k_funcinfo << "rotation diff set to: " << mRotationDiff << endl;
}

void BoAutoGameCamera::setZ(GLfloat z)
{
  mPosZDiff = z - gameCamera()->z();
}

bool BoAutoGameCamera::advance2()
{
  bool changed = BoAutoCamera::advance2();
  if(remainingTime() <= 0)
  {
    return changed;
  }

  // How much of differences to add
  float factor = moveFactor();

  if(mPosZDiff)
  {
    float diff = (mPosZDiff * factor);
    gameCamera()->setZ(gameCamera()->mPosZ + diff);
    changed = true;
  }

  if(mRotationDiff)
  {
    float diff = (mRotationDiff * factor);
    gameCamera()->setRotation(gameCamera()->mRotation + diff);
    gameCamera()->checkRotation();
    changed = true;
  }

  if(mRadiusDiff)
  {
    float diff = (mRadiusDiff * factor);
    gameCamera()->setRadius(gameCamera()->mRadius + diff);
    changed = true;
  }
  return changed;
}

void BoAutoGameCamera::resetDifferences()
{
  BoAutoCamera::resetDifferences();
  mPosZDiff = 0;
  mRotationDiff = 0;
  mRadiusDiff = 0;
}



/*
 * vim: et sw=2
 */
