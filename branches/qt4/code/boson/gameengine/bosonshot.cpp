/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 Rivo Laks (rivolaks@hot.ee)

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

#include "bosonshot.h"

#include "../../bomemory/bodummymemory.h"
#include "../global.h"
#include "bosoncanvas.h"
#include "bosonweapon.h"
#include "player.h"
#include "unitproperties.h"
#include "boitemlist.h"
#include "boson.h"
#include "speciestheme.h"
#include "unit.h"
#include <bogl.h>
#include "bodebug.h"

#include <ksimpleconfig.h>
#include <krandomsequence.h>

#include <q3ptrlist.h>
#include <qdom.h>

#include <math.h>

// Degrees to radians conversion (AB: from mesa/src/macros.h)
#define DEG2RAD (M_PI/180.0)
// And radians to degrees conversion
#define RAD2DEG (180.0/M_PI)


/*****  BosonShot  *****/

BosonShot::BosonShot(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop) :
    BosonItem(owner, canvas)
{
  initPrivate();
  mProp = prop;

  if(properties())
  {
    mWeaponDamage.setLocal(properties()->damage());
    mWeaponDamageRange.setLocal(properties()->damageRange());
    mWeaponFullDamageRange.setLocal(properties()->fullDamageRange());
    mWeaponSpeed.setLocal(properties()->speed());
  }
}

BosonShot::BosonShot(Player* owner, BosonCanvas* canvas) :
    BosonItem(owner, canvas)
{
  initPrivate();
}

void BosonShot::initPrivate()
{
  initStatic();
  mProp = 0;
  registerData(&mWeaponDamage, IdWeaponDamage);
  registerData(&mWeaponDamageRange, IdWeaponDamageRange);
  registerData(&mWeaponFullDamageRange, IdWeaponFullDamageRange);
  registerData(&mWeaponSpeed, IdWeaponSpeed);

  mWeaponDamage.setLocal(0);
  mWeaponDamageRange.setLocal(0);
  mWeaponFullDamageRange.setLocal(0);
  mWeaponSpeed.setLocal(0);
}

BosonShot::~BosonShot()
{
}

void BosonShot::applyWeapon(const BosonWeapon* weapon)
{
  if(!weapon)
  {
    BO_NULL_ERROR(weapon);
    return;
  }
  mWeaponDamage.setLocal(weapon->damage());
  mWeaponDamageRange.setLocal(weapon->damageRange());
  mWeaponFullDamageRange.setLocal(weapon->fullDamageRange());
  mWeaponSpeed.setLocal(weapon->speed());
}

QString BosonShot::getModelIdForItem() const
{
  if(!properties())
  {
    return QString::null;
  }
  if(!owner())
  {
    BO_NULL_ERROR(owner());
    return QString::null;
  }
  if(!owner()->speciesTheme())
  {
    BO_NULL_ERROR(owner()->speciesTheme());
    return QString::null;
  }
  return QString("%1:%2").arg("shot").arg(properties()->modelFileName());
}

void BosonShot::initStatic()
{
  static bool initialized = false;
  if (initialized)
  {
    return;
  }
  addPropertyId(IdWeaponDamage, "WeaponDamage");
  addPropertyId(IdWeaponDamageRange, "WeaponDamageRange");
  addPropertyId(IdWeaponFullDamageRange, "WeaponFullDamageRange");
  addPropertyId(IdWeaponSpeed, "WeaponSpeed");
  initialized = true;
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
    setInactive();
    return false;
  }
  if(!canvas())
  {
    boError(350) << k_funcinfo << "NULL canvas" << endl;
    setInactive();
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
  bofixed xPos = centerX() + xVelocity();
  bofixed yPos = centerY() + yVelocity();
  // ensure that the next position will be valid
  if(xPos < 0 || xPos >= canvas()->mapWidth())
  {
    velocityX = 0;
    xPos = centerX();
    if(xPos < 0 || xPos >= canvas()->mapWidth())
    {
      boError(350) << k_funcinfo << "Internal error! xPos is still invalid: " << xPos << endl;
    }
  }
  if(yPos < 0 || yPos >= canvas()->mapHeight())
  {
    velocityY = 0;
    yPos = centerY();
    if(yPos < 0 || yPos >= canvas()->mapHeight())
    {
      boError(350) << k_funcinfo << "Internal error! yPos is still invalid: " << yPos << endl;
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
    boError(350) << k_funcinfo << "Error saving BosonItem" << endl;
    return false;
  }

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
    boError(350) << k_funcinfo << "Error loading BosonItem" << endl;
    return false;
  }
  mActive = true; // Inactive shots won't be saved

  return true;
}

void BosonShot::explode()
{
 mActive = false;
 moveToTarget();
 setVelocity(0, 0, 0);
 setVisible(false);
 canvas()->shotHit(this);
 setInactive();
}

qint32 BosonShot::damage() const
{
  return mWeaponDamage;
}

bofixed BosonShot::damageRange() const
{
  return mWeaponDamageRange;
}

bofixed BosonShot::fullDamageRange() const
{
  return mWeaponFullDamageRange;
}

void BosonShot::setInactive()
{
  mActive = false;
  setVelocity(0, 0, 0);
  setRotation(0);
  setXRotation(0);
  setYRotation(0);
}



/*****  BosonShotBullet  *****/

BosonShotBullet::BosonShotBullet(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop) :
    BosonShot(owner, canvas, prop)
{
  initStatic();
}

void BosonShotBullet::initStatic()
{
  static bool initialized = false;
  if (initialized)
  {
    return;
  }
  initialized = true;
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
    setInactive();
    return false;
  }
  return true;
}

void BosonShotBullet::moveToTarget()
{
  moveCenterTo(mTarget.x(), mTarget.y(), mTarget.z());
}

void BosonShotBullet::setTarget(const BoVector3Fixed& target)
{
  mTarget = target;
}




/*****  BosonShotRocket  *****/

BosonShotRocket::BosonShotRocket(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop) :
    BosonShot(owner, canvas, prop)
{
  initStatic();
  registerData(&mTotalDist, IdTotalDist);
  registerData(&mPassedDist, IdPassedDist);
  registerData(&mZ, IdZ);
  registerData(&mMaxHeight, IdMaxHeight);
}

BosonShotRocket::~BosonShotRocket()
{
}

void BosonShotRocket::initStatic()
{
  static bool initialized = false;
  if (initialized)
  {
    return;
  }
  initialized = true;
  addPropertyId(IdTotalDist, "TotalDist");
  addPropertyId(IdPassedDist, "PassedDist");
  addPropertyId(IdZ, "Z");
  addPropertyId(IdMaxHeight, "MaxHeight");
}

// AB: "pos" is the _center_ of the rocket
void BosonShotRocket::init(const BoVector3Fixed& pos, const BoVector3Fixed& target)
{
  boDebug(350) << "MISSILE: " << k_funcinfo << "Creating new shot" << endl;
  mTarget = target;
  if(!properties())
  {
    boError(350) << k_funcinfo << "NULL weapon properties!" << endl;
    setInactive();
    return;
  }
  if(!canvas()->onCanvas((int)pos[0], (int)pos[1]))
  {
    boError(350) << k_funcinfo << "invalid start position" << endl;
    setInactive();
    return;
  }

  // First set the velocity to length of whole trip (for calculations)
  mVelo = target - pos;
  mVelo.setZ(0.0f);

  mTotalDist = mVelo.length();

  // Speeds
  setAccelerationSpeed(properties()->accelerationSpeed());
  setMaxSpeed(mWeaponSpeed);

  // Maximum height missile can have
  mMaxHeight = properties()->height() * (mTotalDist);
  // Set correct base velocity. Note that this must be multiplied with speed()
  //  to get actual velocity for given speed
  mVelo.setZ(target.z() - pos.z());
  mVelo.scale(1 / mTotalDist);

  // FIXME: name: it has nothing to do with effects anymore
  mEffectVelo = sqrt(mVelo[0] * mVelo[0] + mVelo[1] * mVelo[1]);

  // Initialization
  moveCenterTo(pos[0], pos[1], pos[2]);
  setRotation(Bo3dTools::rotationToPoint(mVelo[0], mVelo[1]));
  mZ = 0; // For parable calculations only, must be 0 at the beginning
  mPassedDist = 0;
  mLastDistToTarget = (target - pos).dotProduct();
}

// move the shot by one step
// (actually only set the velocity - it is moved by BosonCanvas::slotAdvance())
void BosonShotRocket::advanceMoveInternal()
{
  setVisible(true);
  // Always accelerate
  accelerate();
  // Increase distance that missile has flied
  mPassedDist = mPassedDist + speed();
  // Calculate parable height at current step
  bofixed factor;
  if (mTotalDist == 0)
  {
    boError(350) << k_funcinfo << "mTotalDist == 0 is not allowed" << endl;
    factor = 0;
  }
  else
  {
    factor = mPassedDist / mTotalDist - 0.5;  // Factor will be in range -0.5 to 0.5
  }
  factor = -4 * (factor * factor) + 1;  // Factor is now  0 ... 1 ... 0  depending of current step
  // How much will be added to current z position
  bofixed addZ = (mMaxHeight * factor);
  bofixed zvelo = mVelo[2] * speed() + (addZ - mZ);
  mZ = addZ;
  setVelocity(mVelo[0] * speed(), mVelo[1] * speed(), zvelo);
  setXRotation(Bo3dTools::rotationToPoint(mEffectVelo * speed(), zvelo) - 90 );

  // Check if missile is still active
  bofixed newdist = (mTarget - BoVector3Fixed(centerX() + xVelocity(), centerY() + yVelocity(), z() + zVelocity())).dotProduct();
  if(newdist > mLastDistToTarget)
  {
    explode();
  }
  mLastDistToTarget = newdist;
}

bool BosonShotRocket::saveAsXML(QDomElement& root)
{
  if(!BosonShot::saveAsXML(root))
  {
    boError(350) << k_funcinfo << "Error saving BosonShot" << endl;
    return false;
  }

  // Too many attributes?
  root.setAttribute("xVelocity", mVelo.x());
  root.setAttribute("yVelocity", mVelo.y());
  root.setAttribute("zVelocity", mVelo.z());
  root.setAttribute("Targetx", mTarget.x());
  root.setAttribute("Targety", mTarget.y());
  root.setAttribute("Targetz", mTarget.z());
  root.setAttribute("Speed", speed());
  return true;
}

bool BosonShotRocket::loadFromXML(const QDomElement& root)
{
  if(!BosonShot::loadFromXML(root))
  {
    boError(350) << k_funcinfo << "Error loading BosonShot" << endl;
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
      boError(350) << k_funcinfo << "Invalid value for xVelocity tag" << endl;
      return false;
    }
  }
  yvelo = root.attribute("yVelocity").toFloat(&ok);
  if(!ok)
  {
    yvelo = root.attribute("yVelo").toFloat(&ok);
    if (!ok)
    {
      boError(350) << k_funcinfo << "Invalid value for yVelocity tag" << endl;
      return false;
    }
  }
  zvelo = root.attribute("zVelocity").toFloat(&ok);
  if(!ok)
  {
    zvelo = root.attribute("zVelo").toFloat(&ok);
    if (!ok)
    {
      boError(350) << k_funcinfo << "Invalid value for zVelocity tag" << endl;
      return false;
    }
  }
  targetx = root.attribute("Targetx").toFloat(&ok);
  if(!ok)
  {
    boError(350) << k_funcinfo << "Invalid value for Targetx tag" << endl;
    return false;
  }
  targety = root.attribute("Targety").toFloat(&ok);
  if(!ok)
  {
    boError(350) << k_funcinfo << "Invalid value for Targety tag" << endl;
    return false;
  }
  targetz = root.attribute("Targetz").toFloat(&ok);
  if(!ok)
  {
    boError(350) << k_funcinfo << "Invalid value for Targetz tag" << endl;
    return false;
  }

  speed = root.attribute("Speed").toFloat(&ok);
  if(!ok)
  {
    boError(350) << k_funcinfo << "Invalid value for Speed tag" << endl;
    return false;
  }

  mVelo.set(xvelo, yvelo, zvelo);
  mTarget.set(targetx, targety, targetz);
  mLastDistToTarget = (mTarget - BoVector3Fixed(centerX(), centerY(), z())).dotProduct();
  mEffectVelo = sqrt(mVelo[0] * mVelo[0] + mVelo[1] * mVelo[1]);
  setRotation(Bo3dTools::rotationToPoint(mVelo[0], mVelo[1]));
  setSpeed(speed);
  setAccelerationSpeed(properties()->accelerationSpeed());
  setMaxSpeed(mWeaponSpeed);
  setVisible(true);
  return true;
}

void BosonShotRocket::moveToTarget()
{
  moveCenterTo(mTarget.x(), mTarget.y(), mTarget.z());
}



/*****  BosonShotMissile  *****/

BosonShotMissile::BosonShotMissile(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop) :
    BosonShot(owner, canvas, prop)
{
  initStatic();
  registerData(&mPassedDist, IdPassedDist);
}

BosonShotMissile::~BosonShotMissile()
{
}

void BosonShotMissile::initStatic()
{
  static bool initialized = false;
  if (initialized)
  {
    return;
  }
  initialized = true;
  addPropertyId(IdPassedDist, "PassedDist");
}

// AB: "pos" is the _center_ of the shot
void BosonShotMissile::init(const BoVector3Fixed& pos, Unit* target)
{
  boDebug(350) << "MISSILE: " << k_funcinfo << "Creating new shot" << endl;
  mTarget = target;
  mTargetPos.set(mTarget->centerX(), mTarget->centerY(), mTarget->z());
  if(!properties())
  {
    boError(350) << k_funcinfo << "NULL weapon properties!" << endl;
    setInactive();
    return;
  }
  if(!canvas()->onCanvas((int)pos[0], (int)pos[1]))
  {
    boError(350) << k_funcinfo << "invalid start position" << endl;
    setInactive();
    return;
  }

  // Speeds
  setAccelerationSpeed(properties()->accelerationSpeed());
  setMaxSpeed(mWeaponSpeed);

  // Initialization
  moveCenterTo(pos[0], pos[1], pos[2]);
  //setRotation(Bo3dTools::rotationToPoint(target->x() - pos.x(), target->y() - pos.y()));

  mPassedDist = 0;

  // Initial velocity
  if(properties()->startAngle() == -1)
  {
    mVelo.set(mTarget->centerX() - centerX(), mTarget->centerY() - centerY(), mTarget->z() - z());
    mVelo.normalize();
  }
  else
  {
    mVelo.set(mTarget->centerX() - centerX(), mTarget->centerY() - centerY(), 0);
    mVelo.normalize();
    mVelo.scale(cos(properties()->startAngle() * DEG2RAD));
    mVelo.setZ(sin(properties()->startAngle() * DEG2RAD));
    // mVelo is already normalized
  }
}

// move the shot by one step
// (actually only set the velocity - it is moved by BosonCanvas::slotAdvance())
void BosonShotMissile::advanceMoveInternal()
{
  if(mTarget && !mTarget->isDestroyed())
  {
    // Update target pos
    mTargetPos.set(mTarget->centerX(), mTarget->centerY(), mTarget->z());
  }

  setVisible(true);
  // Always accelerate
  accelerate();
  // Increase distance that missile has flied
  mPassedDist = mPassedDist + speed();
  if(mPassedDist > properties()->maxFlyDistance())
  {
    // TODO: wait e.g. 0.5 or 1 second before exploding
    explode();
    return;
  }

  //boDebug(350) << k_funcinfo << id() << ": target pos: (" << mTargetPos.x() << "; " << mTargetPos.y() << "; " << mTargetPos.z() << ")" << endl;
  //boDebug(350) << k_funcinfo << id() << ": my pos: (" << x() << "; " << y() << "; " << z() << ")" << endl;
  // Calculate velocity
  // Missile always moves towards it's target
  BoVector3Fixed totarget(mTargetPos.x() - centerX(), mTargetPos.y() - centerY(), mTargetPos.z() - z());
  bofixed totargetlen = totarget.length();
  // We need check this here to avoid division by 0 later
  if(totargetlen <= speed())
  {
    //boDebug(350) << k_funcinfo << id() << ": near target (totargetlen = " << totargetlen << "), exploding" << endl;
    explode();
    return;
  }

  // Normalize totarget. totarget vector now shows direction to target
  totarget.scale(1.0f / totargetlen);

  // Difference between missile's current direction and direction to target
  BoVector3Fixed diff = totarget - mVelo;
  bofixed difflen = diff.length();
  if(difflen != 0)
  {
    //boDebug(350) << k_funcinfo << id() << ": difflen = " << difflen << endl;
    // Missile is not flying towards the target atm
    // Calculate new velocity vector
    if(properties()->turningSpeed() < difflen)
    {
      diff.scale(properties()->turningSpeed() / difflen);
    }
    // Alter velocity direction so that it's more towards the target
    mVelo += diff;
    mVelo.normalize();
  }

  // This is final velocity
  BoVector3Fixed velo(mVelo * speed());

  setVelocity(velo.x(), velo.y(), velo.z());
  setRotation(Bo3dTools::rotationToPoint(velo.x(), velo.y()));
  setXRotation(Bo3dTools::rotationToPoint(sqrt(1 - velo.z() * velo.z()), velo.z()) - 90);
}

bool BosonShotMissile::saveAsXML(QDomElement& root)
{
  if(!BosonShot::saveAsXML(root))
  {
    boError(350) << k_funcinfo << "Error saving BosonShot" << endl;
    return false;
  }

  root.setAttribute("xVelocity", mVelo.x());
  root.setAttribute("yVelocity", mVelo.y());
  root.setAttribute("zVelocity", mVelo.z());
  root.setAttribute("Speed", speed());
  root.setAttribute("Target", mTarget->id());
  root.setAttribute("TargetPosX", mTargetPos.x());
  root.setAttribute("TargetPosY", mTargetPos.y());
  root.setAttribute("TargetPosZ", mTargetPos.z());
  return true;
}

bool BosonShotMissile::loadFromXML(const QDomElement& root)
{
  if(!BosonShot::loadFromXML(root))
  {
    boError(350) << k_funcinfo << "Error loading BosonShot" << endl;
    return false;
  }

  bool ok;
  bofixed speed;
  bofixed xvelo, yvelo, zvelo;
  unsigned int targetid;
  bofixed targetposx, targetposy, targetposz;

  xvelo = root.attribute("xVelocity").toFloat(&ok);
  if(!ok)
  {
    boError(350) << k_funcinfo << "Invalid value for xVelocity tag" << endl;
    return false;
  }
  yvelo = root.attribute("yVelocity").toFloat(&ok);
  if(!ok)
  {
    boError(350) << k_funcinfo << "Invalid value for yVelocity tag" << endl;
    return false;
  }
  zvelo = root.attribute("zVelocity").toFloat(&ok);
  if(!ok)
  {
    boError(350) << k_funcinfo << "Invalid value for zVelocity tag" << endl;
    return false;
  }
  speed = root.attribute("Speed").toFloat(&ok);
  if(!ok)
  {
    boError(350) << k_funcinfo << "Invalid value for Speed tag" << endl;
    return false;
  }
  targetid = root.attribute("Target").toUInt(&ok);
  if(!ok)
  {
    boError(350) << k_funcinfo << "Invalid value for Target tag" << endl;
    return false;
  }
  targetposx = root.attribute("TargetPosX").toFloat(&ok);
  if(!ok)
  {
    boError(350) << k_funcinfo << "Invalid value for TargetPosX tag" << endl;
    return false;
  }
  targetposy = root.attribute("TargetPosY").toFloat(&ok);
  if(!ok)
  {
    boError(350) << k_funcinfo << "Invalid value for TargetPosY tag" << endl;
    return false;
  }
  targetposz = root.attribute("TargetPosZ").toFloat(&ok);
  if(!ok)
  {
    boError(350) << k_funcinfo << "Invalid value for TargetPosZ tag" << endl;
    return false;
  }

  mTarget = boGame->findUnit(targetid, 0);
  mVelo.set(xvelo, yvelo, zvelo);
  mTargetPos.set(targetposx, targetposy, targetposz);
  setRotation(Bo3dTools::rotationToPoint(mVelo.x(), mVelo.y()));
  setXRotation(Bo3dTools::rotationToPoint(mVelo.y(), mVelo.z()) + 90);
  setSpeed(speed);
  setAccelerationSpeed(properties()->accelerationSpeed());
  setMaxSpeed(mWeaponSpeed);
  setVisible(true);
  return true;
}

void BosonShotMissile::moveToTarget()
{
  //move(mTarget->x(), mTarget->y(), mTarget->z());
}


/*****  BosonShotExplosion  *****/

BosonShotExplosion::BosonShotExplosion(Player* owner, BosonCanvas* canvas) :
    BosonShot(owner, canvas)
{
  initStatic();
  registerData(&mDamage, IdDamage);
  registerData(&mDamageRange, IdDamageRange);
  registerData(&mFullDamageRange, IdFullDamageRange);
  registerData(&mDelay, IdDelay);

  mDamage = 0;
  mDamageRange = 0;
  mFullDamageRange = 0;
  mDelay = 0;
}

void BosonShotExplosion::initStatic()
{
  static bool initialized = false;
  if (initialized)
  {
    return;
  }
  initialized = true;
  addPropertyId(IdDamage, "Damage");
  addPropertyId(IdDamageRange, "DamageRange");
  addPropertyId(IdFullDamageRange, "FullDamageRange");
  addPropertyId(IdDelay, "Delay");
}

void BosonShotExplosion::activate(const BoVector3Fixed& pos, qint32 damage, bofixed damagerange, bofixed fulldamagerange, int delay)
{
  mDamage = damage;
  mDamageRange = damagerange;
  mFullDamageRange = fulldamagerange;
  mDelay = delay;
  moveCenterTo(pos.x(), pos.y(), pos.z());
}

bool BosonShotExplosion::saveAsXML(QDomElement& root)
{
  if(!BosonShot::saveAsXML(root))
  {
    boError(350) << k_funcinfo << "Error while saving BosonShot" << endl;
    return false;
  }
  return true;
}

bool BosonShotExplosion::loadFromXML(const QDomElement& root)
{
  if(!BosonShot::loadFromXML(root))
  {
    boError(350) << k_funcinfo << "Error while loading BosonShot" << endl;
    return false;
  }
  return true;
}

void BosonShotExplosion::advanceMoveInternal()
{
  mDelay = mDelay - 1;
  if(mDelay <= 0)
  {
    explode();
  }
}


/*****  BosonShotMine  *****/

BosonShotMine::BosonShotMine(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop) :
    BosonShot(owner, canvas, prop)
{
  initStatic();
  registerData(&mActivated, IdActivated);
}

void BosonShotMine::initStatic()
{
  static bool initialized = false;
  if (initialized)
  {
    return;
  }
  initialized = true;
  addPropertyId(IdActivated, "Activated");
}

// AB: "pos" is the _center_ of the mine
void BosonShotMine::init(const BoVector3Fixed& pos)
{
  mActivated = false;
  moveCenterTo(pos.x(), pos.y(), pos.z());
  setVisible(true);
}

bool BosonShotMine::saveAsXML(QDomElement& root)
{
  if(!BosonShot::saveAsXML(root))
  {
    boError(350) << k_funcinfo << "Error while saving BosonShot" << endl;
    return false;
  }
  return true;
}

bool BosonShotMine::loadFromXML(const QDomElement& root)
{
  if(!BosonShot::loadFromXML(root))
  {
    boError(350) << k_funcinfo << "Error while loading BosonShot" << endl;
    return false;
  }

  setVisible(true);

  return true;
}

void BosonShotMine::advanceMoveInternal()
{
  boDebug(350) << "MINE: " << k_funcinfo << endl;
  BoItemList* contacts = collisions()->collisions(boundingRect(), this, true);
  if(!contacts->isEmpty())
  {
    // Somebody is touching the mine. If mine is activated, explode
    if(mActivated)
    {
      boDebug(350) << "MINE: " << k_funcinfo << "contact. BOOM" << endl;
      explode();
    }
  }
  else
  {
    // Nobody is touching the mine. If mine is unactivated, activate it
    if(!mActivated)
    {
      boDebug(350) << "MINE: " << k_funcinfo << "Mine activated" << endl;
      mActivated = true;
    }
  }
}



/*****  BosonShotBomb  *****/

BosonShotBomb::BosonShotBomb(Player* owner, BosonCanvas* canvas, const BosonWeaponProperties* prop) :
    BosonShot(owner, canvas, prop)
{
  initStatic();
  registerData(&mActivated, IdActivated);
  // Speeds
  setAccelerationSpeed(properties()->accelerationSpeed());
  setMaxSpeed(mWeaponSpeed);
}

void BosonShotBomb::initStatic()
{
  static bool initialized = false;
  if (initialized)
  {
    return;
  }
  initialized = true;
  addPropertyId(IdActivated, "Activated");
}

// AB: "pos" is the _center_ of the bomb
void BosonShotBomb::init(const BoVector3Fixed& pos)
{
  // TODO: BosonShotBomb and BosonShotMine share quite a lot code. Maybe make
  //  bomb inherit from mine (bomb is basically a falling mine)
  mActivated = false;
  moveCenterTo(pos.x(), pos.y(), pos.z());
  setVisible(true);
}

bool BosonShotBomb::saveAsXML(QDomElement& root)
{
  if(!BosonShot::saveAsXML(root))
  {
    boError(350) << k_funcinfo << "Error while saving BosonShot" << endl;
    return false;
  }

  root.setAttribute("Speed", speed());
  root.setAttribute("HorizontalVelocityX", mHorizontalVelocity.x());
  root.setAttribute("HorizontalVelocityY", mHorizontalVelocity.y());
  return true;
}

bool BosonShotBomb::loadFromXML(const QDomElement& root)
{
  if(!BosonShot::loadFromXML(root))
  {
    boError(350) << k_funcinfo << "Error loading BosonShot" << endl;
    return false;
  }

  bool ok;
  bofixed speed;
  bofixed hvelox, hveloy;

  speed = root.attribute("Speed").toFloat(&ok);
  if(!ok)
  {
    boError(350) << k_funcinfo << "Invalid value for Speed tag" << endl;
    return false;
  }
  hvelox = root.attribute("HorizontalVelocityX").toFloat(&ok);
  if(!ok)
  {
    boError(350) << k_funcinfo << "Invalid value for HorizontalVelocityX tag" << endl;
    return false;
  }
  hveloy = root.attribute("HorizontalVelocityY").toFloat(&ok);
  if(!ok)
  {
    boError(350) << k_funcinfo << "Invalid value for HorizontalVelocityY tag" << endl;
    return false;
  }

  setSpeed(speed);
  setHorizontalVelocity(BoVector2Fixed(hvelox, hveloy));
  setAccelerationSpeed(properties()->accelerationSpeed());
  setMaxSpeed(mWeaponSpeed);
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
    boDebug(350) << "BOMB: " << k_funcinfo << "terrain contact. BOOM" << endl;
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
      boDebug(350) << "BOMB: " << k_funcinfo << "item contact. BOOM" << endl;
      explode();
    }
  }
  else
  {
    if(!mActivated)
    {
      boDebug(350) << "BOMB: " << k_funcinfo << "Bomb activated" << endl;
      mActivated = true;
    }
  }

  accelerate();
  boDebug(350) << "BOMB: " << k_funcinfo << "accelerated (a. speed: " << accelerationSpeed() <<
      "); speed is now: " << speed() << "; z: " << z() << endl;
  setVelocity(mHorizontalVelocity.x(), mHorizontalVelocity.y(), -speed());
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
  initStatic();
  mUnitProperties = 0;
}

BosonShotFragment::~BosonShotFragment()
{
}

void BosonShotFragment::initStatic()
{
  static bool initialized = false;
  if (initialized)
  {
    return;
  }
  initialized = true;
}

QString BosonShotFragment::getModelIdForItem() const
{
  if(!owner())
  {
    BO_NULL_ERROR(owner());
    return QString::null;
  }
  if(!owner()->speciesTheme())
  {
    BO_NULL_ERROR(owner()->speciesTheme());
    return QString::null;
  }
  return QString("%1:%2").arg("shot").arg("fragment");
}

void BosonShotFragment::activate(const BoVector3Fixed& pos, const UnitProperties* unitproperties)
{
  boDebug(350) << k_funcinfo << "id: " << id() << endl;
  mUnitProperties = unitproperties;

  KRandomSequence* r = owner()->game()->random();
  mVelo.set(r->getDouble() - 0.5, r->getDouble() - 0.5, 0);
  mVelo.normalize();
  mVelo.scale(FRAGMENT_MIN_SPEED + (r->getDouble() * (FRAGMENT_MAX_SPEED - FRAGMENT_MIN_SPEED)));
  mVelo.setZ(FRAGMENT_MIN_Z_SPEED + (r->getDouble() * (FRAGMENT_MAX_Z_SPEED - FRAGMENT_MIN_Z_SPEED)));
  boDebug(350) << k_funcinfo << "Velocity is: (" << mVelo.x() << "; " << mVelo.y() << "; " << mVelo.z() << ")" << endl;

  moveCenterTo(pos.x(), pos.y(), pos.z() + 0.2);  // +0.2 prevents immediate contact with the terrain
  setRotation(Bo3dTools::rotationToPoint(mVelo.x(), mVelo.y()));
  setVisible(true);
}

bool BosonShotFragment::saveAsXML(QDomElement& root)
{
  if(!BosonShot::saveAsXML(root))
  {
    boError(350) << k_funcinfo << "Error while saving BosonShot" << endl;
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
    boError(350) << k_funcinfo << "Error loading BosonShot" << endl;
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
      boError(350) << k_funcinfo << "Invalid value for Velocityx tag" << endl;
      return false;
    }
  }
  veloy = root.attribute("Velocityy").toFloat(&ok);
  if(!ok)
  {
    veloy = root.attribute("Veloy").toFloat(&ok);
    if (!ok)
    {
      boError(350) << k_funcinfo << "Invalid value for Velocityy tag" << endl;
      return false;
    }
  }
  veloz = root.attribute("Velocityz").toFloat(&ok);
  if(!ok)
  {
    veloz = root.attribute("Veloz").toFloat(&ok);
    if (!ok)
    {
      boError(350) << k_funcinfo << "Invalid value for Velocityz tag" << endl;
      return false;
    }
  }

  props = root.attribute("UnitProperties").toUInt(&ok);
  if(!ok)
  {
    boError(350) << k_funcinfo << "Invalid value for UnitProperties tag" << endl;
    return false;
  }

  mVelo.set(velox, veloy, veloz);
  setRotation(Bo3dTools::rotationToPoint(mVelo.x(), mVelo.y()));
  mUnitProperties = owner()->speciesTheme()->unitProperties((quint32)props);
  if(!mUnitProperties)
  {
    boError(350) << k_funcinfo << "NULL properties for " << props << endl;
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
    boDebug(350) << "FRAGMENT: " << k_funcinfo << "terrain contact. BOOM" << endl;
    explode();
    return;
  }
  // Check for items too?

  mVelo.setZ(mVelo.z() + FRAGMENT_GRAVITY);
  setVelocity(mVelo.x(), mVelo.y(), mVelo.z());
}

qint32 BosonShotFragment::damage() const
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
