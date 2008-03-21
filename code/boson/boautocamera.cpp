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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "boautocamera.h"

#include "../bomemory/bodummymemory.h"
#include "bocamera.h"
#include "defines.h"
#include "bodebug.h"

#include <qdom.h>

#include <math.h>


BoAutoCamera::InterpolationData::InterpolationData(const BoVector3Float& _pos, float _time)
{
  pos = _pos;
  time = _time;
}

BoAutoCamera::InterpolationDataFloat::InterpolationDataFloat(float _value, float _time)
{
  value = _value;
  time = _time;
}



BoAutoCamera::BoAutoCamera(BoCamera* camera)
{
  init();
  mCamera = camera;
}

BoAutoCamera& BoAutoCamera::operator=(const BoAutoCamera& c)
{
  mCamera = c.mCamera;
  mCurrentTime = c.mCurrentTime;
  mMoveMode = c.mMoveMode;
  mInterpolationMode = c.mInterpolationMode;
  mPosPoints = c.mPosPoints;
  mUpPoints = c.mUpPoints;
  mLookAtPoints = c.mLookAtPoints;
  mMoving = c.mMoving;

  return *this;
}

void BoAutoCamera::init()
{
  mCamera = 0;
  mMoveMode = Immediate;
  mInterpolationMode = Linear;
  mCurrentTime = 0;
  mMoving = false;
}

void BoAutoCamera::changeLookAt(const BoVector3Float& diff)
{
  InterpolationData d;
  d.pos = camera()->lookAt() + diff;
  if(mMoveMode == Immediate)
  {
    camera()->setLookAt(d.pos);
    return;
  }
  mLookAtPoints.append(d);
}

void BoAutoCamera::changeUp(const BoVector3Float& diff)
{
  InterpolationData d;
  d.pos = camera()->up() + diff;
  if(mMoveMode == Immediate)
  {
    camera()->setUp(d.pos);
    return;
  }
  mUpPoints.append(d);
}

void BoAutoCamera::changeCameraPos(const BoVector3Float& diff)
{
  InterpolationData d;
  d.pos = camera()->cameraPos() + diff;
  if(mMoveMode == Immediate)
  {
    camera()->setCameraPos(d.pos);
    return;
  }
  mPosPoints.append(d);
}

void BoAutoCamera::setLookAt(const BoVector3Float& pos)
{
  InterpolationData d;
  d.pos = pos;
  if(mMoveMode == Immediate)
  {
    camera()->setLookAt(d.pos);
    return;
  }
  mLookAtPoints.append(d);
}

void BoAutoCamera::setUp(const BoVector3Float& pos)
{
  InterpolationData d;
  d.pos = pos;
  if(mMoveMode == Immediate)
  {
    camera()->setUp(d.pos);
    return;
  }
  mUpPoints.append(d);
}

void BoAutoCamera::setCameraPos(const BoVector3Float& pos)
{
  InterpolationData d;
  d.pos = pos;
  if(mMoveMode == Immediate)
  {
    camera()->setCameraPos(d.pos);
    return;
  }
  mPosPoints.append(d);
}

void BoAutoCamera::commitChanges(float ticks)
{
  if(moveMode() == Immediate)
  {
    boWarning() << k_funcinfo << "commitChanges() has no effect in immediate mode!" << endl;
    moveCompleted();
    mMoving = false;
    return;
  }
  else if(moveMode() == SegmentInterpolation)
  {
    float endtime = mCurrentTime + ticks;
    prepareSegmentInterpolation(endtime);
  }

  mMoving = true;
}

void BoAutoCamera::prepareSegmentInterpolation(float endtime)
{
  // Make sure there's either 0 or 2 (start and final) points in each list
  if(mPosPoints.count())
  {
    BoVector3Float final = mPosPoints.last().pos;
    mPosPoints.clear();
    mPosPoints.append(InterpolationData(camera()->cameraPos(), mCurrentTime));
    mPosPoints.append(InterpolationData(final, endtime));
  }
  if(mUpPoints.count())
  {
    BoVector3Float final = mUpPoints.last().pos;
    mUpPoints.clear();
    mUpPoints.append(InterpolationData(camera()->up(), mCurrentTime));
    mUpPoints.append(InterpolationData(final, endtime));
  }
  if(mLookAtPoints.count())
  {
    BoVector3Float final = mLookAtPoints.last().pos;
    mLookAtPoints.clear();
    mLookAtPoints.append(InterpolationData(camera()->lookAt(), mCurrentTime));
    mLookAtPoints.append(InterpolationData(final, endtime));
  }
}

void BoAutoCamera::setMoveMode(MoveMode mode)
{
  boDebug(230) << k_funcinfo << "mode: " << mode << endl;
  mMoveMode = mode;
}

void BoAutoCamera::setInterpolationMode(InterpolationMode mode)
{
  boDebug(230) << k_funcinfo << "mode: " << mode << endl;
  mInterpolationMode = mode;
}

bool BoAutoCamera::getCurrentVector(QValueList<InterpolationData>& points, float time, BoVector3Float& result)
{
  // Make sure there are any points in the list
  if(points.count() == 0)
  {
    return false;
  }

  // If we have a single point in the list, then the resulting vector will be
  //  constant.
  if(points.count() == 1)
  {
    result = points.first().pos;
    return true;
  }

  // If time of the last data in the list is in past (i.e. it has already been
  //  used), then the resulting vector is now constant
  if(points.last().time < (time + 1))
  {
    return false;
  }

  // The points for interpolation (we interpolate between a and b).
  // Note that beforea and afterb are used by only cubic interpolation.
  // Init all datas to first element in the list.
  InterpolationData beforea = points.first();
  InterpolationData a = beforea;
  InterpolationData b = beforea;
  InterpolationData afterb = beforea;

  // Get points a and b from the list
  QValueList<InterpolationData>::Iterator it;
  for(it = points.begin(); it != points.end(); ++it)
  {
    if((*it).time >= time)
    {
      // That's point b
      b = *it;
      // Get point afterb
      ++it;
      if(it != points.end())
      {
        afterb = *it;
      }
      else
      {
        // b is already at the end of the list
        afterb = b;
      }
      break;
    }
    else
    {
      // Update points a and beforea
      beforea = a;
      a = *it;
    }
  }

  // Calculate factor.
  // If factor is 0, we're at point a, if it's 0.5 we're between a and b, if
  //  it's 1, we're at b, etc
  float factor = -1.0f;
  if(time == a.time)
  {
    factor = 0;
  }
  else if(time == b.time)
  {
    factor = 1;
  }
  else
  {
    factor = (time - a.time) / (b.time - a.time);
  }

  // Do the interpolation
  if(mInterpolationMode == Linear)
  {
    result = a.pos * (1-factor) + b.pos * factor;
  }
  else if(mInterpolationMode == Sinusoidal)
  {
    float f = (1 - cos(factor * 3.1415927)) * 0.5;
    result = a.pos * (1-f) + b.pos * f;
  }
  else if(mInterpolationMode == SinusoidStart)
  {
    float f = (1 - cos(factor * 3.1415927 * 2)) * 0.5;
    result = a.pos * (1-f) + b.pos * f;
  }
  else if(mInterpolationMode == SinusoidEnd)
  {
    float f = (1 - cos(factor * 3.1415927 * 2)) * 0.5;
    result = a.pos * (1-f) + b.pos * f;
  }
  else if(mInterpolationMode == Cubic)
  {
    BoVector3Float p = (afterb.pos - b.pos) - (beforea.pos - a.pos);
    BoVector3Float q = (beforea.pos - a.pos) - p;
    BoVector3Float r = b.pos - beforea.pos;
    BoVector3Float s = a.pos;
    result = p * (factor*factor*factor) + q * (factor*factor) + r * factor + s;
  }
  return true;
}

bool BoAutoCamera::getCurrentValue(QValueList<InterpolationDataFloat>& values, float time, float& result)
{
  // FIXME: LOTS of code duplication (with the getCurrentVector() method above)

  // Make sure there are any points in the list
  if(values.count() == 0)
  {
    return false;
  }

  // If we have a single point in the list, then the resulting vector will be
  //  constant.
  if(values.count() == 1 && values.first().time >= time)
  {
    result = values.first().value;
    return true;
  }

  // If time of the last data in the list is in past (i.e. it has already been
  //  used), then the resulting vector is now constant
  if(values.last().time < time)
  {
    return false;
  }

  // The points for interpolation (we interpolate between a and b).
  // Note that beforea and afterb are used by only cubic interpolation.
  // Init all datas to first element in the list.
  InterpolationDataFloat beforea = values.first();
  InterpolationDataFloat a = beforea;
  InterpolationDataFloat b = beforea;
  InterpolationDataFloat afterb = beforea;

  // Get points a and b from the list
  QValueList<InterpolationDataFloat>::Iterator it;
  for(it = values.begin(); it != values.end(); ++it)
  {
    if((*it).time >= time)
    {
      // That's point b
      b = *it;
      // Get point afterb
      ++it;
      if(it != values.end())
      {
        afterb = *it;
      }
      else
      {
        // b is already at the end of the list
        afterb = b;
      }
      break;
    }
    else
    {
      // Update points a and beforea
      beforea = a;
      a = *it;
    }
  }

  // Calculate factor.
  // If factor is 0, we're at point a, if it's 0.5 we're between a and b, if
  //  it's 1, we're at b, etc
  float factor = -1.0f;
  if(time == a.time)
  {
    factor = 0;
  }
  else if(time == b.time)
  {
    factor = 1;
  }
  else
  {
    factor = (time - a.time) / (time - b.time);
  }

  // Do the interpolation
  if(mInterpolationMode == Linear)
  {
    result = a.value * (1-factor) + b.value * factor;
  }
  else if(mInterpolationMode == Sinusoidal)
  {
    float f = (1 - cos(factor * 3.1415927)) * 0.5;
    result = a.value * (1-f) + b.value * f;
  }
  else if(mInterpolationMode == SinusoidStart)
  {
    float f = (1 - cos(factor * 3.1415927 * 2)) * 0.5;
    result = a.value * (1-f) + b.value * f;
  }
  else if(mInterpolationMode == SinusoidEnd)
  {
    float f = (1 - cos(factor * 3.1415927 * 2)) * 0.5;
    result = a.value * (1-f) + b.value * f;
  }
  else if(mInterpolationMode == Cubic)
  {
    float p = (afterb.value - b.value) - (beforea.value - a.value);
    float q = (beforea.value - a.value) - p;
    float r = b.value - beforea.value;
    float s = a.value;
    result = p * factor*factor*factor + q * factor*factor + r * factor + s;
  }
  return true;
}

void BoAutoCamera::advance()
{
  mCurrentTime += 1;

  if(!mMoving)
  {
    return;
  }

  if(advanceVectors())
  {
    setPositionDirty();
  }
  else
  {
    mMoving = false;
    moveCompleted();
  }
}

bool BoAutoCamera::advanceVectors()
{
  bool moving = false;
  BoVector3Float pos;
  BoVector3Float up;
  BoVector3Float lookat;

  if(getCurrentVector(mPosPoints, mCurrentTime, pos))
  {
    moving = true;
    camera()->setCameraPos(pos);
  }
  if(getCurrentVector(mUpPoints, mCurrentTime, up))
  {
    moving = true;
    camera()->setUp(up);
  }
  if(getCurrentVector(mLookAtPoints, mCurrentTime, lookat))
  {
    moving = true;
    camera()->setLookAt(lookat);
  }

  return moving;
}

void BoAutoCamera::moveCompleted()
{
  mLookAtPoints.clear();
  mUpPoints.clear();
  mPosPoints.clear();
}

void BoAutoCamera::setPositionDirty(bool d)
{
  camera()->setPositionDirty(d);
}

void BoAutoCamera::addLookAtPoint(const BoVector3Float& pos, float time)
{
  InterpolationData d;
  d.pos = pos;
  d.time = time;
  mLookAtPoints.append(d);
}

void BoAutoCamera::addUpPoint(const BoVector3Float& pos, float time)
{
  InterpolationData d;
  d.pos = pos;
  d.time = time;
  mUpPoints.append(d);
}

void BoAutoCamera::addCameraPosPoint(const BoVector3Float& pos, float time)
{
  InterpolationData d;
  d.pos = pos;
  d.time = time;
  mPosPoints.append(d);
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

  mDistancePoints = c.mDistancePoints;
  mRotationPoints = c.mRotationPoints;
  mXRotationPoints = c.mXRotationPoints;

  return *this;
}

void BoAutoGameCamera::init()
{
}

void BoAutoGameCamera::changeDistance(GLfloat diff)
{
  InterpolationDataFloat d;
  d.value = gameCamera()->distance() + diff;
  if(moveMode() == Immediate)
  {
    gameCamera()->setDistance(d.value);
    return;
  }
  mDistancePoints.append(d);
}

void BoAutoGameCamera::changeRotation(GLfloat diff)
{
  InterpolationDataFloat d;
  d.value = gameCamera()->rotation() + diff;
  if(moveMode() == Immediate)
  {
    gameCamera()->setRotation(d.value);
    return;
  }
  mRotationPoints.append(d);
}

void BoAutoGameCamera::changeXRotation(GLfloat diff)
{
  InterpolationDataFloat d;
  d.value = gameCamera()->xRotation() + diff;
  if(moveMode() == Immediate)
  {
    gameCamera()->setXRotation(d.value);
    return;
  }
  mXRotationPoints.append(d);
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

void BoAutoGameCamera::setRotation(GLfloat r)
{
  InterpolationDataFloat d;
  d.value = r;
  if(moveMode() == Immediate)
  {
    gameCamera()->setRotation(d.value);
    return;
  }
  mRotationPoints.append(d);
}

void BoAutoGameCamera::setXRotation(GLfloat r)
{
  InterpolationDataFloat d;
  d.value = r;
  if(moveMode() == Immediate)
  {
    gameCamera()->setXRotation(d.value);
    return;
  }
  mXRotationPoints.append(d);
}

void BoAutoGameCamera::setDistance(GLfloat dist)
{
  InterpolationDataFloat d;
  d.value = dist;
  if(moveMode() == Immediate)
  {
    gameCamera()->setDistance(d.value);
    return;
  }
  mDistancePoints.append(d);
}

bool BoAutoGameCamera::advanceVectors()
{
  bool moving = false;
  if(BoAutoCamera::advanceVectors())
  {
    moving = true;
  }

  float distance;
  float rotation;
  float xrotation;

  if(getCurrentValue(mDistancePoints, currentTime(), distance))
  {
    moving = true;
    gameCamera()->setDistance(distance);
  }
  if(getCurrentValue(mRotationPoints, currentTime(), rotation))
  {
    moving = true;
    gameCamera()->setRotation(rotation);
  }
  if(getCurrentValue(mXRotationPoints, currentTime(), xrotation))
  {
    moving = true;
    gameCamera()->setXRotation(xrotation);
  }

  return moving;
}

void BoAutoGameCamera::moveCompleted()
{
  BoAutoCamera::moveCompleted();

  mDistancePoints.clear();
  mRotationPoints.clear();
  mXRotationPoints.clear();
}

void BoAutoGameCamera::prepareSegmentInterpolation(float endtime)
{
  BoAutoCamera::prepareSegmentInterpolation(endtime);

  // Make sure there's either 0 or 2 (start and final) points in each list
  if(mDistancePoints.count())
  {
    float final = mDistancePoints.last().value;
    mDistancePoints.clear();
    mDistancePoints.append(InterpolationDataFloat(gameCamera()->distance(), currentTime()));
    mDistancePoints.append(InterpolationDataFloat(final, endtime));
  }
  if(mRotationPoints.count())
  {
    float final = mRotationPoints.last().value;
    mRotationPoints.clear();
    mRotationPoints.append(InterpolationDataFloat(gameCamera()->rotation(), currentTime()));
    mRotationPoints.append(InterpolationDataFloat(final, endtime));
  }
  if(mXRotationPoints.count())
  {
    float final = mXRotationPoints.last().value;
    mXRotationPoints.clear();
    mXRotationPoints.append(InterpolationDataFloat(gameCamera()->xRotation(), currentTime()));
    mXRotationPoints.append(InterpolationDataFloat(final, endtime));
  }
}



/*
 * vim: et sw=2
 */
