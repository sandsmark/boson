/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonshot.h"

#include "../global.h"
#include "../bosoncanvas.h"
#include "../bosonparticlesystem.h"
#include "../bosonweapon.h"
#include "../player.h"
#include "../unitproperties.h"
#include "bodebug.h"

#include <ksimpleconfig.h>

#include <qptrlist.h>
#include <qdom.h>

#include <GL/gl.h>

#include <math.h>


BosonShot::BosonShot(const BosonWeaponProperties* prop, Player* owner, BosonCanvas* canvas, BoVector3 pos, BoVector3 target) :
    BosonItem(prop ? prop->model() : 0, canvas)
{
  boDebug(350) << "MISSILE: " << k_funcinfo << "Creating new shot" << endl;
  mOwner = owner;
  mProp = prop;
  mFlyParticleSystems = 0;
  setSize(BO_TILE_SIZE / 2, BO_TILE_SIZE / 2); // AB: pretty much a random value
  if (!mProp)
  {
    boError(350) << k_funcinfo << "NULL weapon properties!" << endl;
    return;
  }
  if (!mOwner)
  {
    boError(350) << k_funcinfo << "NULL owner!" << endl;
    return;
  }
  if (!canvas)
  {
    boError(350) << k_funcinfo << "NULL canvas" << endl;
    return;
  }
  if(!canvas->onCanvas((int)pos[0], (int)pos[1]))
  {
    boError(350) << k_funcinfo << "invalid start position" << endl;
    mActive = false;
    return;
  }
  if(prop->speed() == 0)
  {
    // This shot is bullet, not missile - it has infinite speed and it reaches
    //  it's target immideately. No need to calculate anything.
    boDebug(350) << "MISSILE: " << k_funcinfo << "    Attacker's shot is bullet (infinite speed). Returning" << endl;
    move(target[0], target[1], target[2]);
    mActive = false;
    return;
  }
  // First set the velocity to length of whole trip (for calculations)
  mVelo = target - pos;
  mLength = mVelo.length();
  mMaxHeight = prop->height() * (mLength / BO_TILE_SIZE);
  //boDebug(350) << "MISSILE: " << k_funcinfo << "    Length of trip: " << length << endl;
  // Calculate number of steps
  mTotalSteps = (int)ceilf(mLength / prop->speed()) - 1;
  // Current step
  mStep = 0;
  //boDebug(350) << "MISSILE: " << k_funcinfo << "    Steps: " << mSteps << endl;
  // Set velocity
  mVelo.scale(prop->speed() / mLength);
  //boDebug(350) << "MISSILE: " << k_funcinfo << "    Normalized & scaled (final) velocity: (" << mVelo[0] << "; " << mVelo[1] << "; " << mVelo[2] << ")" << endl;
  // Particle systems
  mFlyParticleSystems = new QPtrList<BosonParticleSystem>(prop->newFlyParticleSystems(pos, 0.0));
  canvas->addParticleSystems(*mFlyParticleSystems);
  mParticleVelo = sqrt(mVelo[0] * mVelo[0] + mVelo[1] * mVelo[1]) / (float)BO_TILE_SIZE;
  // Initialization
  mActive = true;
  move(pos[0], pos[1], pos[2]);
  setAnimated(true);
  setRotation(rotationToPoint(mVelo[0], mVelo[1]));
  mZ = 0; // For parable calculations only, must be 0 at the beginning
}

BosonShot::BosonShot(const BosonWeaponProperties* prop, Player* owner, BosonCanvas* canvas) :
    BosonItem(prop ? prop->model() : 0, canvas)
{
  mOwner = owner;
  mProp = prop;
  setSize(BO_TILE_SIZE / 2, BO_TILE_SIZE / 2); // AB: pretty much a random value
  mFlyParticleSystems = 0;
}

// move the shot by one step
// (actually only set the velocity - it is moved by BosonCanvas::slotAdvance())
void BosonShot::advanceMoveInternal()
{
  mStep++;
  // Calculate parable height at current step
  float factor = mStep / (float)mTotalSteps - 0.5;  // Factor will be in range -0.5 to 0.5
  factor = -4 * (factor * factor) + 1;  // Factor is now  0 ... 1 ... 0  depending of current step
  // How much will be added to current z position
  float addZ = (mMaxHeight * factor);
  float zvelo = mVelo[2] + (addZ - mZ);
  mZ = addZ;
  setVelocity(mVelo[0], mVelo[1], zvelo);
  setXRotation(rotationToPoint(mParticleVelo, zvelo) - 90 );
  if(mStep >= mTotalSteps)
  {
    mActive = false;
  }
}

//AB: note this this is primary optimized for SAFETY, NOT for performance. it is
//optimized for performance in the second place!
void BosonShot::advanceMoveCheck()
{
  float velocityX = xVelocity();
  float velocityY = yVelocity();
  float xPos = x() + xVelocity();
  float yPos = y() + yVelocity();
  // ensure that the next position will be valid
  if(xPos < 0 || xPos >= canvas()->mapWidth() * BO_TILE_SIZE)
  {
    velocityX = 0.0f;
    xPos = x();
    if(xPos < 0 || xPos >= canvas()->mapWidth() * BO_TILE_SIZE)
    {
      boError() << k_funcinfo << "Internal error! xPos is still invalid: " << xPos << endl;
    }
  }
  if(yPos < 0 || yPos >= canvas()->mapHeight() * BO_TILE_SIZE)
  {
    velocityY = 0.0f;
    yPos = y();
    if(yPos < 0 || yPos >= canvas()->mapHeight() * BO_TILE_SIZE)
    {
      boError() << k_funcinfo << "Internal error! yPos is still invalid: " << yPos << endl;
    }
  }
  setVelocity(velocityX, velocityY, zVelocity());
}

const QColor* BosonShot::teamColor() const
{
  return mOwner ? &mOwner->teamColor() : 0;
}

bool BosonShot::saveAsXML(QDomElement& root)
{
  root.setAttribute("UnitType", (unsigned int)mProp->unitProperties()->typeId());
  root.setAttribute("WeaponType", (unsigned int)mProp->id());
  root.setAttribute("Owner", (unsigned int)mOwner->id());

  // Too many attributes?
  root.setAttribute("Step", mStep);
  root.setAttribute("TotalSteps", mTotalSteps);
  root.setAttribute("mZ", mZ);
  root.setAttribute("x", x());
  root.setAttribute("y", y());
  root.setAttribute("z", z());
  root.setAttribute("xVelo", mVelo.x());
  root.setAttribute("yVelo", mVelo.y());
  root.setAttribute("zVelo", mVelo.z());
  root.setAttribute("ParticleVelo", mParticleVelo);
  root.setAttribute("MaxHeight", mMaxHeight);
  root.setAttribute("Rotation", rotation());
  return true;
}

bool BosonShot::loadFromXML(const QDomElement& root)
{
  bool ok;
  float x;
  float y;
  float z;
  float xvelo, yvelo, zvelo;
  float rotation;
  x = root.attribute("x").toFloat(&ok);
  if (!ok) {
    boError() << k_funcinfo << "Invalid value for x tag" << endl;
    return false;
  }
  y = root.attribute("y").toFloat(&ok);
  if (!ok) {
    boError() << k_funcinfo << "Invalid value for y tag" << endl;
    return false;
  }
  z = root.attribute("z").toFloat(&ok);
  if (!ok) {
    boError() << k_funcinfo << "Invalid value for z tag" << endl;
    return false;
  }
  xvelo = root.attribute("xVelo").toFloat(&ok);
  if (!ok) {
    boError() << k_funcinfo << "Invalid value for xVelo tag" << endl;
    return false;
  }
  yvelo = root.attribute("yVelo").toFloat(&ok);
  if (!ok) {
    boError() << k_funcinfo << "Invalid value for yVelo tag" << endl;
    return false;
  }
  zvelo = root.attribute("zVelo").toFloat(&ok);
  if (!ok) {
    boError() << k_funcinfo << "Invalid value for zVelo tag" << endl;
    return false;
  }
  rotation = root.attribute("Rotation").toFloat(&ok);
  if (!ok) {
    boError() << k_funcinfo << "Invalid value for Rotation tag" << endl;
    rotation = 0;
    // don't stop (will be broken, but unit won't get deleted)
  }
  mStep = root.attribute("Step").toUInt(&ok);
  if (!ok) {
    boError() << k_funcinfo << "Invalid value for Step tag" << endl;
    return false;
  }
  mTotalSteps = root.attribute("TotalSteps").toUInt(&ok);
  if (!ok) {
    boError() << k_funcinfo << "Invalid value for TotalSteps tag" << endl;
    return false;
  }
  mZ = root.attribute("mZ").toFloat(&ok);
  if (!ok) {
    boError() << k_funcinfo << "Invalid value for mZ tag" << endl;
    return false;
  }
  mParticleVelo = root.attribute("ParticleVelo").toFloat(&ok);
  if (!ok) {
    boError() << k_funcinfo << "Invalid value for ParticleVelo tag" << endl;
    return false;
  }
  mMaxHeight = root.attribute("MaxHeight").toFloat(&ok);
  if (!ok) {
    boError() << k_funcinfo << "Invalid value for MaxHeight tag" << endl;
    return false;
  }

  mVelo.set(xvelo, yvelo, zvelo);
  mActive = true; // Inactive shots won't be saved
  move(x, y, z);
  boDebug() << k_funcinfo << "Moving to (" << x << "; " << y << "; " << z << ")" << endl;
  setRotation(rotation);
  setAnimated(true);
  canvas()->newShot(this);
  return true;
}

void BosonShot::save(QDataStream& stream)
{
  stream << (float)x();
  stream << (float)y();
  stream << (float)z();
  stream << mVelo;
  stream << (Q_UINT32)mStep;
  stream << (Q_UINT32)mTotalSteps;
  stream << (float)mLength;
  stream << (float)mZ;
  stream << (float)mParticleVelo;
  stream << (float)rotation();
}

void BosonShot::load(QDataStream& stream)
{
  Q_UINT32 step, totalsteps;
  float x, y, z;
  float rot;

  stream >> x >> y >> z;
  stream >> mVelo;
  stream >> step >> totalsteps;
  stream >> mLength >> mZ >> mParticleVelo;
  stream >> rot;

  mStep = step;
  mTotalSteps = totalsteps;
  mActive = (mStep < mTotalSteps);
  move(x, y, z);
  boDebug() << k_funcinfo << "Moving to (" << x << "; " << y << "; " << z << ")" << endl;
  setRotation(rot);
  setAnimated(true);
}


/*
 * vim: et sw=2
 */
