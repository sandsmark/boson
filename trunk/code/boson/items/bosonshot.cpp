/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "../bosoneffect.h"
#include "../bosonweapon.h"
#include "../player.h"
#include "../unitproperties.h"
#include "../boitemlist.h"
#include "../bosoneffectproperties.h"
#include "../boson.h"
#include "../speciestheme.h"
#include "bodebug.h"

#include <ksimpleconfig.h>
#include <krandomsequence.h>

#include <qptrlist.h>
#include <qdom.h>

#include <GL/gl.h>

#include <math.h>

/*****  BosonShot  *****/

BosonShot::BosonShot(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop) :
    BosonItem(owner, canvas)
{
  mProp = prop;
}

BosonShot::BosonShot(Player* owner, BosonCanvas* canvas) :
    BosonItem(owner, canvas)
{
  mProp = 0;
}

BosonShot::~BosonShot()
{
}

BosonModel* BosonShot::getModelForItem() const
{
  if(!mProp)
  {
    return 0;
  }
  return mProp->model();
}

bool BosonShot::init()
{
  bool ret = BosonItem::init();
  if(!ret)
  {
    return ret;
  }
  boDebug(350) << "MISSILE: " << k_funcinfo << "Creating new shot" << endl;
  // FIXME: can't we use values from objects config file here?
  setSize(1.0f / 2, 1.0f / 2, 1.0f / 2); // AB: pretty much a random value

  // At first shot is invisible. It will be set visible when it's advanced. This
  //  is needed because x rotation isn't calculated in constructor and it would
  //  be wrong until missile is advanced.
  setVisible(false);

  if(!owner())
  {
    boError(350) << k_funcinfo << "NULL owner!" << endl;
    setActive(false);
    return false;
  }
  if(!canvas())
  {
    boError(350) << k_funcinfo << "NULL canvas" << endl;
    setActive(false);
    return false;
  }

  // Initialization
  mActive = true;
  return true;
}

//AB: note this this is primary optimized for SAFETY, NOT for performance. it is
//optimized for performance in the second place!
void BosonShot::advanceMoveCheck()
{
  bofixed velocityX = xVelocity();
  bofixed velocityY = yVelocity();
  if(!velocityX && !velocityY)
  {
    return;
  }
  bofixed xPos = x() + xVelocity();
  bofixed yPos = y() + yVelocity();
  // ensure that the next position will be valid
  if(xPos < 0 || xPos >= canvas()->mapWidth())
  {
    velocityX = 0;
    xPos = x();
    if(xPos < 0 || xPos >= canvas()->mapWidth())
    {
      boError() << k_funcinfo << "Internal error! xPos is still invalid: " << xPos << endl;
    }
  }
  if(yPos < 0 || yPos >= canvas()->mapHeight())
  {
    velocityY = 0;
    yPos = y();
    if(yPos < 0 || yPos >= canvas()->mapHeight())
    {
      boError() << k_funcinfo << "Internal error! yPos is still invalid: " << yPos << endl;
    }
  }
  setVelocity(velocityX, velocityY, zVelocity());
}

const QColor* BosonShot::teamColor() const
{
  return owner() ? &owner()->teamColor() : 0;
}

bool BosonShot::saveAsXML(QDomElement& root)
{
  if (!BosonItem::saveAsXML(root))
  {
    boError() << k_funcinfo << "Error saving BosonItem" << endl;
    return false;
  }
  root.setAttribute("x", x());
  root.setAttribute("y", y());
  root.setAttribute("z", z());

  root.setAttribute("Rtti", rtti());
  root.setAttribute("Type", type());

  // the unittype defines the group of the shot (through the weapons of that
  // unit), the weapon ID defines the type inside the group.
  unsigned int unitType = 0;
  unsigned int weaponId = 0;
  if(properties())
  {
    unitType = properties()->unitProperties()->typeId();
    weaponId = properties()->id();
  }
  root.setAttribute("Group", unitType);
  root.setAttribute("GroupType", weaponId);
  return true;
}

bool BosonShot::loadFromXML(const QDomElement& root)
{
  if (!BosonItem::loadFromXML(root))
  {
    boError() << k_funcinfo << "Error loading BosonItem" << endl;
    return false;
  }
  bool ok;
  bofixed x;
  bofixed y;
  bofixed z;
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

  return true;
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
  return properties() ? properties()->damage() : 0;
}

bofixed BosonShot::damageRange() const
{
  return properties() ? properties()->damageRange() : bofixed(0);
}

bofixed BosonShot::fullDamageRange() const
{
  return properties() ? properties()->fullDamageRange() : bofixed(0);
}



/*****  BosonShotBullet  *****/

BosonShotBullet::BosonShotBullet(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop) :
    BosonShot(owner, canvas, prop)
{
}

bool BosonShotBullet::init()
{
  bool ret = BosonShot::init();
  if(!ret)
  {
    return ret;
  }
  if(!properties())
  {
    boError(350) << k_funcinfo << "NULL weapon properties!" << endl;
    setActive(false);
    return false;
  }
  return true;
}

void BosonShotBullet::moveToTarget()
{
  move(mTarget.x(), mTarget.y(), mTarget.z());
}

void BosonShotBullet::setTarget(const BoVector3Fixed& target)
{
  mTarget = target;
}

void BosonShotBullet::explode()
{
  // We need to create fly effects here, because atm, our position is shooter's
  //  (weapon's) position and BosonShot::explode() moves us to target position,
  //  so fly effects will then get correctly moved as well.
  setEffects(properties()->newFlyEffects(BoVector3Fixed(x(), y(), z()), 0));

  BosonShot::explode();

  clearEffects();
}


/*****  BosonShotMissile  *****/

BosonShotMissile::BosonShotMissile(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop) :
    BosonShot(owner, canvas, prop)
{
}

BosonShotMissile::~BosonShotMissile()
{
}

void BosonShotMissile::init(const BoVector3Fixed& pos, const BoVector3Fixed& target)
{
  boDebug(350) << "MISSILE: " << k_funcinfo << "Creating new shot" << endl;
  mTarget = target;
  if(!properties())
  {
    boError(350) << k_funcinfo << "NULL weapon properties!" << endl;
    setActive(false);
    return;
  }
  if(!canvas()->onCanvas((int)pos[0], (int)pos[1]))
  {
    boError(350) << k_funcinfo << "invalid start position" << endl;
    setActive(false);
    return;
  }

  // First set the velocity to length of whole trip (for calculations)
  mVelo = target - pos;
  mVelo.setZ(0.0f);

  mTotalDist = mVelo.length();

  // Speeds
  setAccelerationSpeed(properties()->accelerationSpeed());
  setMaxSpeed(properties()->speed());

  // Maximum height missile can have
  mMaxHeight = properties()->height() * (mTotalDist);
  // Set correct base velocity. Note that this must be multiplied with speed()
  //  to get actual velocity for given speed
  mVelo.setZ(target.z() - pos.z());
  mVelo.scale(1 / mTotalDist);

  // Effects
  setEffects(properties()->newFlyEffects(pos, 0.0));
  // FIXME: name: it has nothing to do with effects anymore
  mEffectVelo = sqrt(mVelo[0] * mVelo[0] + mVelo[1] * mVelo[1]);

  // Initialization
  move(pos[0], pos[1], pos[2]);
  setRotation(Bo3dTools::rotationToPoint(mVelo[0], mVelo[1]));
  mZ = 0; // For parable calculations only, must be 0 at the beginning
  mPassedDist = 0;
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
  bofixed factor = mPassedDist / mTotalDist - 0.5;  // Factor will be in range -0.5 to 0.5
  factor = -4 * (factor * factor) + 1;  // Factor is now  0 ... 1 ... 0  depending of current step
  // How much will be added to current z position
  bofixed addZ = (mMaxHeight * factor);
  bofixed zvelo = mVelo[2] * speed() + (addZ - mZ);
  mZ = addZ;
  setVelocity(mVelo[0] * speed(), mVelo[1] * speed(), zvelo);
  setXRotation(Bo3dTools::rotationToPoint(mEffectVelo * speed(), zvelo) - 90 );

  // Check if missile is still active
  BoVector3Fixed dist = mTarget - BoVector3Fixed(x() + xVelocity(), y() + yVelocity(), z() + zVelocity());
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
  root.setAttribute("xVelocity", mVelo.x());
  root.setAttribute("yVelocity", mVelo.y());
  root.setAttribute("zVelocity", mVelo.z());
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
  bofixed xvelo, yvelo, zvelo;
  bofixed targetx, targety, targetz;
  bofixed speed;

  xvelo = root.attribute("xVelocity").toFloat(&ok);
  if(!ok)
  {
    xvelo = root.attribute("xVelo").toFloat(&ok);
    if (!ok)
    {
      boError() << k_funcinfo << "Invalid value for xVelocity tag" << endl;
      return false;
    }
  }
  yvelo = root.attribute("yVelocity").toFloat(&ok);
  if(!ok)
  {
    yvelo = root.attribute("yVelo").toFloat(&ok);
    if (!ok)
    {
      boError() << k_funcinfo << "Invalid value for yVelocity tag" << endl;
      return false;
    }
  }
  zvelo = root.attribute("zVelocity").toFloat(&ok);
  if(!ok)
  {
    zvelo = root.attribute("zVelo").toFloat(&ok);
    if (!ok)
    {
      boError() << k_funcinfo << "Invalid value for zVelocity tag" << endl;
      return false;
    }
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
  mEffectVelo = sqrt(mVelo[0] * mVelo[0] + mVelo[1] * mVelo[1]);
  setRotation(Bo3dTools::rotationToPoint(mVelo[0], mVelo[1]));
  setSpeed(speed);
  setAccelerationSpeed(properties()->accelerationSpeed());
  setMaxSpeed(properties()->speed());
  setVisible(true);
  return true;
}

void BosonShotMissile::moveToTarget()
{
  move(mTarget.x(), mTarget.y(), mTarget.z());
}


/*****  BosonShotExplosion  *****/

BosonShotExplosion::BosonShotExplosion(Player* owner, BosonCanvas* canvas) :
    BosonShot(owner, canvas)
{
  mDamage = 0;
  mDamageRange = 0;
  mFullDamageRange = 0;
  mDelay = 0;
}

void BosonShotExplosion::activate(const BoVector3Fixed& pos, long int damage, bofixed damagerange, bofixed fulldamagerange, int delay)
{
  mDamage = damage;
  mDamageRange = damagerange;
  mFullDamageRange = fulldamagerange;
  mDelay = delay;
  move(pos.x(), pos.y(), pos.z());
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

void BosonShotExplosion::advanceMoveInternal()
{
  mDelay--;
  if(mDelay <= 0)
  {
    explode();
  }
}


/*****  BosonShotMine  *****/

BosonShotMine::BosonShotMine(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop) :
    BosonShot(owner, canvas, prop)
{
}

void BosonShotMine::init(const BoVector3Fixed& pos)
{
  mActivated = false;
  move(pos.x(), pos.y(), pos.z());
  setVisible(true);
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

BosonShotBomb::BosonShotBomb(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop) :
    BosonShot(owner, canvas, prop)
{
  // Speeds
  setAccelerationSpeed(properties()->accelerationSpeed());
  setMaxSpeed(properties()->speed());
}

void BosonShotBomb::init(const BoVector3Fixed& pos)
{
  // TODO: BosonShotBomb and BosonShotMine share quite a lot code. Maybe make
  //  bomb inherit from mine (bomb is basically a falling mine)
  mActivated = false;
  move(pos.x(), pos.y(), pos.z());
  setVisible(true);
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
  bofixed speed;

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

void BosonShotBomb::advanceMoveInternal()
{
  // First check if bomb touches terrain
  // TODO: maybe put item/terrain collison check to generic BosonItem method
  // Maybe check all corners intead of center point
  if(z() <= canvas()->heightAtPoint(centerX(), centerY()))
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



/*****  BosonShotFragment  *****/

#define FRAGMENT_MIN_SPEED (3 / 48.0f)
#define FRAGMENT_MAX_SPEED (8 / 48.0f)
#define FRAGMENT_MIN_Z_SPEED 0.03
#define FRAGMENT_MAX_Z_SPEED 0.1
#define FRAGMENT_GRAVITY -0.006

BosonShotFragment::BosonShotFragment(Player* owner, BosonCanvas* canvas) :
    BosonShot(owner, canvas)
{
  mUnitProperties = 0;
}

BosonShotFragment::~BosonShotFragment()
{
}

BosonModel* BosonShotFragment::getModelForItem() const
{
  BO_CHECK_NULL_RET0(owner());
  BO_CHECK_NULL_RET0(owner()->speciesTheme());
  return owner()->speciesTheme()->objectModel("fragment");
}

void BosonShotFragment::activate(const BoVector3Fixed& pos, const UnitProperties* unitproperties)
{
  mUnitProperties = unitproperties;

  KRandomSequence* r = owner()->game()->random();
  mVelo.set(r->getDouble() - 0.5, r->getDouble() - 0.5, 0);
  mVelo.normalize();
  mVelo.scale(FRAGMENT_MIN_SPEED + (r->getDouble() * (FRAGMENT_MAX_SPEED - FRAGMENT_MIN_SPEED)));
  mVelo.setZ(FRAGMENT_MIN_Z_SPEED + (r->getDouble() * (FRAGMENT_MAX_Z_SPEED - FRAGMENT_MIN_Z_SPEED)));
  boDebug() << k_funcinfo << "Velocity is: (" << mVelo.x() << "; " << mVelo.y() << "; " << mVelo.z() << ")" << endl;

  setEffects(mUnitProperties->newExplodingFragmentFlyEffects(pos));

  move(pos.x(), pos.y(), pos.z() + 0.2);  // +0.2 prevents immediate contact with the terrain
  setRotation(Bo3dTools::rotationToPoint(mVelo.x(), mVelo.y()));
  setVisible(true);
}

bool BosonShotFragment::saveAsXML(QDomElement& root)
{
  if(!BosonShot::saveAsXML(root))
  {
    boError() << k_funcinfo << "Error while saving BosonShot" << endl;
    return false;
  }

  root.setAttribute("Velocityx", mVelo.x());
  root.setAttribute("Velocityy", mVelo.y());
  root.setAttribute("Velocityz", mVelo.z());
  root.setAttribute("UnitProperties", (unsigned int)mUnitProperties->typeId());

  return true;
}

bool BosonShotFragment::loadFromXML(const QDomElement& root)
{
  if(!BosonShot::loadFromXML(root))
  {
    boError() << k_funcinfo << "Error loading BosonShot" << endl;
    return false;
  }

  bool ok;
  bofixed velox, veloy, veloz;
  unsigned int props;

  velox = root.attribute("Velocityx").toFloat(&ok);
  if(!ok)
  {
    velox = root.attribute("Velox").toFloat(&ok);
    if (!ok)
    {
      boError() << k_funcinfo << "Invalid value for Velocityx tag" << endl;
      return false;
    }
  }
  veloy = root.attribute("Velocityy").toFloat(&ok);
  if(!ok)
  {
    veloy = root.attribute("Veloy").toFloat(&ok);
    if (!ok)
    {
      boError() << k_funcinfo << "Invalid value for Velocityy tag" << endl;
      return false;
    }
  }
  veloz = root.attribute("Velocityz").toFloat(&ok);
  if(!ok)
  {
    veloz = root.attribute("Veloz").toFloat(&ok);
    if (!ok)
    {
      boError() << k_funcinfo << "Invalid value for Velocityz tag" << endl;
      return false;
    }
  }

  props = root.attribute("UnitProperties").toUInt(&ok);
  if(!ok)
  {
    boError() << k_funcinfo << "Invalid value for UnitProperties tag" << endl;
    return false;
  }

  mVelo.set(velox, veloy, veloz);
  setRotation(Bo3dTools::rotationToPoint(mVelo.x(), mVelo.y()));
  mUnitProperties = owner()->speciesTheme()->unitProperties((unsigned long int)props);
  if(!mUnitProperties)
  {
    boError() << k_funcinfo << "NULL properties for " << props << endl;
    return false;
  }

  setVisible(true);

  return true;
}

void BosonShotFragment::advanceMoveInternal()
{
  // TODO: maybe put item/terrain collison check to generic BosonItem method
  if(z() <= canvas()->heightAtPoint(centerX(), centerY()))
  {
    boDebug() << "FRAGMENT: " << k_funcinfo << "terrain contact. BOOM" << endl;
    explode();
    return;
  }
  // Check for items too?

  mVelo.setZ(mVelo.z() + FRAGMENT_GRAVITY);
  setVelocity(mVelo.x(), mVelo.y(), mVelo.z());
}

void BosonShotFragment::explode()
{
  BosonShot::explode();

  BoVector3Fixed pos(centerX(), centerY(), z());
  canvas()->addEffects(mUnitProperties->newExplodingFragmentHitEffects(pos));
}

long int BosonShotFragment::damage() const
{
  return mUnitProperties->explodingFragmentDamage();
}

bofixed BosonShotFragment::damageRange() const
{
  return mUnitProperties->explodingFragmentDamageRange();
}

bofixed BosonShotFragment::fullDamageRange() const
{
  return 0.25 * damageRange();
}



/*
 * vim: et sw=2
 */
