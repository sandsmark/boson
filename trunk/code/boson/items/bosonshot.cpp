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
#include "../boitemlist.h"
#include "bodebug.h"

#include <ksimpleconfig.h>

#include <qptrlist.h>
#include <qdom.h>

#include <GL/gl.h>

#include <math.h>

/*****  BosonShot  *****/

BosonShot::BosonShot(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop) :
    BosonItem(prop ? prop->model() : 0, canvas)
{
  boDebug(350) << "MISSILE: " << k_funcinfo << "Creating new shot" << endl;
  mOwner = owner;
  mProp = prop;
  // FIXME: can't we use values from objects config file here?
  setSize(BO_TILE_SIZE / 2, BO_TILE_SIZE / 2, BO_GL_CELL_SIZE / 2); // AB: pretty much a random value

  // At first shot is invisible. It will be set visible when it's advanced. This
  //  is needed because x rotation isn't calculated in constructor and it would
  //  be wrong until missile is advanced.
  setVisible(false);

  if(!mOwner)
  {
    boError(350) << k_funcinfo << "NULL owner!" << endl;
    setActive(false);
    return;
  }
  if(!canvas)
  {
    boError(350) << k_funcinfo << "NULL canvas" << endl;
    setActive(false);
    return;
  }

  // Initialization
  mActive = true;
  setAnimated(true);
}

//AB: note this this is primary optimized for SAFETY, NOT for performance. it is
//optimized for performance in the second place!
void BosonShot::advanceMoveCheck()
{
  float velocityX = xVelocity();
  float velocityY = yVelocity();
  if(!velocityX && !velocityY)
  {
    return;
  }
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
  root.setAttribute("x", x());
  root.setAttribute("y", y());
  root.setAttribute("z", z());

  root.setAttribute("Owner", (unsigned int)owner()->id());
  root.setAttribute("Type", type());
  if(properties())
  {
    root.setAttribute("UnitType", (unsigned int)properties()->unitProperties()->typeId());
    root.setAttribute("WeaponType", (unsigned int)properties()->id());
  }
  return true;
}

bool BosonShot::loadFromXML(const QDomElement& root)
{
  bool ok;
  float x;
  float y;
  float z;
  x = root.attribute("x").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for x tag" << endl;
    return false;
  }
  y = root.attribute("y").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for y tag" << endl;
    return false;
  }
  z = root.attribute("z").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for z tag" << endl;
    return false;
  }

  mActive = true; // Inactive shots won't be saved
  move(x, y, z);

  // Is it ok to do this here?
  setAnimated(true);
  return true;
}

void BosonShot::save(QDataStream& stream)
{
  stream << (float)x();
  stream << (float)y();
  stream << (float)z();
}

void BosonShot::load(QDataStream& stream)
{
  float x, y, z;
  stream >> x >> y >> z;

  mActive = true; // Inactive shots won't be saved
  move(x, y, z);

  // Is it ok to do these here?
  setAnimated(true);
}

void BosonShot::explode()
{
 mActive = false;
 moveToTarget();
 setVelocity(0, 0, 0);
 setVisible(false);
 canvas()->shotHit(this);
}

long int BosonShot::damage() const
{
  return properties()->damage();
};

float BosonShot::damageRange() const
{
  return properties()->damageRange();
};

float BosonShot::fullDamageRange() const
{
  return properties()->fullDamageRange();
};



/*****  BosonShotBullet  *****/

BosonShotBullet::BosonShotBullet(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop, BoVector3 target) :
    BosonShot(owner, canvas, prop)
{
  if(!properties())
  {
    boError(350) << k_funcinfo << "NULL weapon properties!" << endl;
    setActive(false);
    return;
  }

  mTarget = target;

  explode();
}

BosonShotBullet::BosonShotBullet(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop) :
    BosonShot(owner, canvas, prop)
{
  if(!properties())
  {
    boError(350) << k_funcinfo << "NULL weapon properties!" << endl;
    setActive(false);
    return;
  }
}

void BosonShotBullet::moveToTarget()
{
  move(mTarget.x(), mTarget.y(), mTarget.z());
}


/*****  BosonShotMissile  *****/

BosonShotMissile::BosonShotMissile(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop, BoVector3 pos, BoVector3 target) :
    BosonShot(owner, canvas, prop)
{
  boDebug(350) << "MISSILE: " << k_funcinfo << "Creating new shot" << endl;
  //boDebug(350) << "    pos: (" << pos.x() << "; " << pos.y() << "; " << pos.z() << ");  target: (" <<
  //    target.x() << "; " << target.y() << "; " << target.z() << "); this=" << this << endl;
  mFlyParticleSystems = 0;
  mTarget = target;
  if(!properties())
  {
    boError(350) << k_funcinfo << "NULL weapon properties!" << endl;
    setActive(false);
    return;
  }
  if(!canvas->onCanvas((int)pos[0], (int)pos[1]))
  {
    boError(350) << k_funcinfo << "invalid start position" << endl;
    setActive(false);
    return;
  }

  // First set the velocity to length of whole trip (for calculations)
  mVelo = target - pos;
  mVelo.setZ(0.0);

  mTotalDist = mVelo.length();

  // Speeds
  setAccelerationSpeed(properties()->accelerationSpeed());
  setMaxSpeed(properties()->speed());

  // Maximum height missile can have
  mMaxHeight = properties()->height() * (mTotalDist / BO_TILE_SIZE);
  // Set correct base velocity. Note that this must be multiplied with speed()
  //  to get actual velocity for given speed
  mVelo.setZ(target.z() - pos.z());
  mVelo.scale(1 / mTotalDist);

  // Particle systems
  mFlyParticleSystems = new QPtrList<BosonParticleSystem>(properties()->newFlyParticleSystems(pos, 0.0));
  canvas->addParticleSystems(*mFlyParticleSystems);
  // FIXME: name: it has nothing to do with particles anymore
  mParticleVelo = sqrt(mVelo[0] * mVelo[0] + mVelo[1] * mVelo[1]) / (float)BO_TILE_SIZE;

  // Initialization
  move(pos[0], pos[1], pos[2]);
  setRotation(rotationToPoint(mVelo[0], mVelo[1]));
  mZ = 0; // For parable calculations only, must be 0 at the beginning
  mPassedDist = 0;
}

BosonShotMissile::BosonShotMissile(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop) :
    BosonShot(owner, canvas, prop)
{
  if(!properties())
  {
    boError(350) << k_funcinfo << "NULL weapon properties!" << endl;
    setActive(false);
    return;
  }
  mFlyParticleSystems = 0;
}

// move the shot by one step
// (actually only set the velocity - it is moved by BosonCanvas::slotAdvance())
void BosonShotMissile::advanceMoveInternal()
{
  setVisible(true);
  // Always accelerate
  accelerate();
  // Increase distance that missile has flied
  mPassedDist += speed();
  // Calculate parable height at current step
  float factor = mPassedDist / mTotalDist - 0.5;  // Factor will be in range -0.5 to 0.5
  factor = -4 * (factor * factor) + 1;  // Factor is now  0 ... 1 ... 0  depending of current step
  // How much will be added to current z position
  float addZ = (mMaxHeight * factor);
  float zvelo = mVelo[2] * speed() + (addZ - mZ);
  mZ = addZ;
  setVelocity(mVelo[0] * speed(), mVelo[1] * speed(), zvelo);
  setXRotation(rotationToPoint(mParticleVelo * speed(), zvelo) - 90 );

  // Check if missile is still active
  BoVector3 dist = mTarget - BoVector3(x() + xVelocity(), y() + yVelocity(), z() + zVelocity());
  if(dist.dotProduct() <= speed() * speed())
  {
    explode();
  }
}

bool BosonShotMissile::saveAsXML(QDomElement& root)
{
  if(!BosonShot::saveAsXML(root))
  {
    boError() << k_funcinfo << "Error saving BosonShot" << endl;
    return false;
  }

  // Too many attributes?
  root.setAttribute("xVelo", mVelo.x());
  root.setAttribute("yVelo", mVelo.y());
  root.setAttribute("zVelo", mVelo.z());
  root.setAttribute("Targetx", mTarget.x());
  root.setAttribute("Targety", mTarget.y());
  root.setAttribute("Targetz", mTarget.z());
  root.setAttribute("TotalDist", mTotalDist);
  root.setAttribute("PassedDist", mPassedDist);
  root.setAttribute("mZ", mZ);
  root.setAttribute("MaxHeight", mMaxHeight);
  root.setAttribute("Speed", speed());
  return true;
}

bool BosonShotMissile::loadFromXML(const QDomElement& root)
{
  if(!BosonShot::loadFromXML(root))
  {
    boError() << k_funcinfo << "Error loading BosonShot" << endl;
    return false;
  }

  bool ok;
  float xvelo, yvelo, zvelo;
  float targetx, targety, targetz;
  float speed;

  xvelo = root.attribute("xVelo").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for xVelo tag" << endl;
    return false;
  }
  yvelo = root.attribute("yVelo").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for yVelo tag" << endl;
    return false;
  }
  zvelo = root.attribute("zVelo").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for zVelo tag" << endl;
    return false;
  }
  targetx = root.attribute("Targetx").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for Targetx tag" << endl;
    return false;
  }
  targety = root.attribute("Targety").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for Targety tag" << endl;
    return false;
  }
  targetz = root.attribute("Targetz").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for Targetz tag" << endl;
    return false;
  }

  mTotalDist = root.attribute("TotalDist").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for TotalDist tag" << endl;
    return false;
  }
  mPassedDist = root.attribute("PassedDist").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for PassedDist tag" << endl;
    return false;
  }
  mZ = root.attribute("mZ").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for mZ tag" << endl;
    return false;
  }
  mMaxHeight = root.attribute("MaxHeight").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for MaxHeight tag" << endl;
    return false;
  }
  speed = root.attribute("Speed").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for Speed tag" << endl;
    return false;
  }

  mVelo.set(xvelo, yvelo, zvelo);
  mTarget.set(targetx, targety, targetz);
  mParticleVelo = sqrt(mVelo[0] * mVelo[0] + mVelo[1] * mVelo[1]) / (float)BO_TILE_SIZE;
  setRotation(rotationToPoint(mVelo[0], mVelo[1]));
  setSpeed(speed);
  setAccelerationSpeed(properties()->accelerationSpeed());
  setMaxSpeed(properties()->speed());
  setVisible(true);
  return true;
}

void BosonShotMissile::save(QDataStream& stream)
{
  BosonShot::save(stream);

  stream << mVelo;
  stream << mTarget;
  stream << (float)mTotalDist;
  stream << (float)mPassedDist;
  stream << (float)mZ;
  stream << (float)mMaxHeight;
  stream << speed();
}

void BosonShotMissile::load(QDataStream& stream)
{
  BosonShot::load(stream);

  float speed;

  stream >> mVelo;
  stream >> mTarget;
  stream >> mTotalDist >> mPassedDist;
  stream >> mZ >> mMaxHeight;
  stream >> speed;

  mParticleVelo = sqrt(mVelo[0] * mVelo[0] + mVelo[1] * mVelo[1]) / (float)BO_TILE_SIZE;
  setRotation(rotationToPoint(mVelo[0], mVelo[1]));
  setSpeed(speed);
  setAccelerationSpeed(properties()->accelerationSpeed());
  setMaxSpeed(properties()->speed());
  setVisible(true);
}

void BosonShotMissile::moveToTarget()
{
  move(mTarget.x(), mTarget.y(), mTarget.z());
}


/*****  BosonShotExplosion  *****/

BosonShotExplosion::BosonShotExplosion(Player* owner, BosonCanvas* canvas, BoVector3 pos, long int damage, float damagerange, float fulldamagerange, int delay) :
    BosonShot(owner, canvas, 0)
{
  mDamage = damage;
  mDamageRange = damagerange;
  mFullDamageRange = fulldamagerange;
  mDelay = delay;
  move(pos.x(), pos.y(), pos.z());
}

BosonShotExplosion::BosonShotExplosion(Player* owner, BosonCanvas* canvas) :
    BosonShot(owner, canvas, 0)
{
}

bool BosonShotExplosion::saveAsXML(QDomElement& root)
{
  if(!BosonShot::saveAsXML(root))
  {
    boError() << k_funcinfo << "Error while saving BosonShot" << endl;
    return false;
  }

  root.setAttribute("Damage", (int)mDamage);
  root.setAttribute("DamageRange", mDamageRange);
  root.setAttribute("FullDamageRange", mFullDamageRange);
  root.setAttribute("Delay", mDelay);
  return true;
}

bool BosonShotExplosion::loadFromXML(const QDomElement& root)
{
  if(!BosonShot::loadFromXML(root))
  {
    boError() << k_funcinfo << "Error while loading BosonShot" << endl;
    return false;
  }

  bool ok;

  mDamage = (long int)root.attribute("Damage").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for Damage tag" << endl;
    return false;
  }
  mDamageRange = root.attribute("DamageRange").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for DamageRange tag" << endl;
    return false;
  }
  mFullDamageRange = root.attribute("FullDamageRange").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for FullDamageRange tag" << endl;
    return false;
  }
  mDelay = root.attribute("Delay").toInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for Delay tag" << endl;
    return false;
  }

  return true;
}

void BosonShotExplosion::save(QDataStream& stream)
{
  BosonShot::save(stream);
  stream << mDamage;
  stream << mDamageRange;
  stream << mFullDamageRange;
  stream << mDelay;
}

void BosonShotExplosion::load(QDataStream& stream)
{
  BosonShot::load(stream);
  stream >> mDamage;
  stream >> mDamageRange;
  stream >> mFullDamageRange;
  stream >> mDelay;
}

void BosonShotExplosion::advanceMoveInternal()
{
  mDelay--;
  if(mDelay <= 0)
  {
    explode();
  }
}


/*****  BosonShotMine  *****/

BosonShotMine::BosonShotMine(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop, BoVector3 pos) :
    BosonShot(owner, canvas, prop)
{
  mActivated = false;
  move(pos.x(), pos.y(), pos.z());
  setVisible(true);
}

BosonShotMine::BosonShotMine(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop) :
    BosonShot(owner, canvas, prop)
{
}

bool BosonShotMine::saveAsXML(QDomElement& root)
{
  if(!BosonShot::saveAsXML(root))
  {
    boError() << k_funcinfo << "Error while saving BosonShot" << endl;
    return false;
  }

  root.setAttribute("Activated", mActivated ? "true" : "false");
  return true;
}

bool BosonShotMine::loadFromXML(const QDomElement& root)
{
  if(!BosonShot::loadFromXML(root))
  {
    boError() << k_funcinfo << "Error while loading BosonShot" << endl;
    return false;
  }

  QString activated;

  activated = root.attribute("Activated");
  if(activated == "true")
  {
    mActivated = true;
  }
  else if(activated == "false")
  {
    mActivated = false;
  }
  else
  {
    boError() << k_funcinfo << "Invalid value for Activated tag" << endl;
    return false;
  }

  setVisible(true);

  return true;
}

void BosonShotMine::save(QDataStream& stream)
{
  BosonShot::save(stream);
  stream << (Q_UINT8)mActivated;
}

void BosonShotMine::load(QDataStream& stream)
{
  BosonShot::load(stream);
  stream >> (Q_UINT8)mActivated;
  setVisible(true);
}

void BosonShotMine::advanceMoveInternal()
{
  boDebug() << "MINE: " << k_funcinfo << endl;
  BoItemList* contacts = collisions()->collisions(boundingRect(), this, true);
  if(!contacts->isEmpty())
  {
    // Somebody is touching the mine. If mine is activated, explode
    if(mActivated)
    {
      boDebug() << "MINE: " << k_funcinfo << "contact. BOOM" << endl;
      explode();
    }
  }
  else
  {
    // Nobody is touching the mine. If mine is unactivated, activate it
    if(!mActivated)
    {
      boDebug() << "MINE: " << k_funcinfo << "Mine activated" << endl;
      mActivated = true;
    }
  }
}



/*****  BosonShotBomb  *****/

BosonShotBomb::BosonShotBomb(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop, BoVector3 pos) :
    BosonShot(owner, canvas, prop)
{
  // Speeds
  setAccelerationSpeed(properties()->accelerationSpeed());
  setMaxSpeed(properties()->speed());

  // TODO: BosonShotBomb and BosonShotMine share quite a lot code. Maybe make
  //  bomb inherit from mine (bomb is basically a falling mine)
  mActivated = false;
  move(pos.x(), pos.y(), pos.z());
  setVisible(true);
}

BosonShotBomb::BosonShotBomb(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop) :
    BosonShot(owner, canvas, prop)
{
  // Speeds
  setAccelerationSpeed(properties()->accelerationSpeed());
  setMaxSpeed(properties()->speed());
}

bool BosonShotBomb::saveAsXML(QDomElement& root)
{
  if(!BosonShot::saveAsXML(root))
  {
    boError() << k_funcinfo << "Error while saving BosonShot" << endl;
    return false;
  }

  root.setAttribute("Activated", mActivated ? "true" : "false");
  root.setAttribute("Speed", speed());
  return true;
}

bool BosonShotBomb::loadFromXML(const QDomElement& root)
{
  if(!BosonShot::loadFromXML(root))
  {
    boError() << k_funcinfo << "Error loading BosonShot" << endl;
    return false;
  }

  bool ok;
  float speed;

  speed = root.attribute("Speed").toFloat(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for Speed tag" << endl;
    return false;
  }

  QString activated;
  activated = root.attribute("Activated");
  if(activated == "true")
  {
    mActivated = true;
  }
  else if(activated == "false")
  {
    mActivated = false;
  }
  else
  {
    boError() << k_funcinfo << "Invalid value for Activated tag" << endl;
    return false;
  }

  setSpeed(speed);
  setAccelerationSpeed(properties()->accelerationSpeed());
  setMaxSpeed(properties()->speed());
  setVisible(true);

  return true;
}

void BosonShotBomb::save(QDataStream& stream)
{
  BosonShot::save(stream);
  stream << (Q_UINT8)mActivated;
  stream << speed();
}

void BosonShotBomb::load(QDataStream& stream)
{
  BosonShot::load(stream);
  float speed;
  stream >> (Q_UINT8)mActivated;
  stream >> speed;
  setSpeed(speed);
  setAccelerationSpeed(properties()->accelerationSpeed());
  setMaxSpeed(properties()->speed());
  setVisible(true);
}

void BosonShotBomb::advanceMoveInternal()
{
  // First check if bomb touches terrain
  // TODO: maybe put item/terrain collison check to generic BosonItem method
  // Maybe check all corners intead of center point
  if(z() <= canvas()->heightAtPoint(x() + width() / 2, y() + height() / 2))
  {
    boDebug() << "BOMB: " << k_funcinfo << "terrain contact. BOOM" << endl;
    explode();
    return;
  }
  // Then check if it collides with items
  BoItemList* contacts = collisions()->collisions(boundingRect(), this, true);
  if(!contacts->isEmpty())
  {
    // We use same trigger mechanism as for mine to prevent it from being
    //  triggered by a collision with the unit that dropped it
    if(mActivated)
    {
      boDebug() << "BOMB: " << k_funcinfo << "item contact. BOOM" << endl;
      explode();
    }
  }
  else
  {
    if(!mActivated)
    {
      boDebug() << "BOMB: " << k_funcinfo << "Bomb activated" << endl;
      mActivated = true;
    }
  }

  accelerate();
  boDebug() << "BOMB: " << k_funcinfo << "accelerated (a. speed: " << accelerationSpeed() <<
      "); speed is now: " << speed() << "; z: " << z() << endl;
  setVelocity(0, 0, -speed());
}


/*
 * vim: et sw=2
 */
