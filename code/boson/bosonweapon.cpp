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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "bosonweapon.h"

#include "../bomemory/bodummymemory.h"
#include "speciestheme.h"
#include "unit.h"
#include "global.h"
#include "bosoncanvas.h"
#include "bodebug.h"
#include "bosonconfig.h"
#include "unitproperties.h"
#include "boaction.h"

#include <math.h>

#include <ksimpleconfig.h>

// Degrees to radians conversion (AB: from mesa/src/macros.h)
#define DEG2RAD (M_PI/180.0)
// And radians to degrees conversion
#define RAD2DEG (180.0/M_PI)


/*****  BosonWeaponProperties  *****/
BosonWeaponProperties::BosonWeaponProperties(const UnitProperties* prop, unsigned long int id) :
    PluginProperties(prop),
    // AB: note that ids start with _1_ here, but with _0_ in the config files.
    // so we must subtract one for the strings.
    mRange(          this, QString("Weapon_%1:Range").arg(id-1), "MaxValue"),
    mDamage(         this, QString("Weapon_%1:Damage").arg(id-1), "MaxValue"),
    mDamageRange(    this, QString("Weapon_%1:DamageRange").arg(id-1), "MaxValue"),
    mFullDamageRange(this, QString("Weapon_%1:FullDamageRange").arg(id-1), "MaxValue"),
    mReloadingTime(  this, QString("Weapon_%1:Reload").arg(id-1), "MaxValue"),
    mSpeed(          this, QString("Weapon_%1:Speed").arg(id-1), "MaxValue")
{
  mId = id;

  if(id < 1)
  {
    boError() << k_funcinfo << "weapon IDs must be >= 1 !!" << endl;
  }
}

BosonWeaponProperties::~BosonWeaponProperties()
{
}

QString BosonWeaponProperties::name() const
{
  return "Weapon";
}

bool BosonWeaponProperties::insertULongWeaponBaseValue(unsigned long int v, const QString& name, const QString& type)
{
  return insertULongBaseValue(v, QString("Weapon_%1:%2").arg(id() - 1).arg(name), type);
}

bool BosonWeaponProperties::insertLongWeaponBaseValue(long int v, const QString& name, const QString& type)
{
  return insertLongBaseValue(v, QString("Weapon_%1:%2").arg(id() - 1).arg(name), type);
}

bool BosonWeaponProperties::insertBoFixedWeaponBaseValue(bofixed v, const QString& name, const QString& type)
{
  return insertBoFixedBaseValue(v, QString("Weapon_%1:%2").arg(id() - 1).arg(name), type);
}

unsigned long int BosonWeaponProperties::ulongWeaponBaseValue(const QString& name, const QString& type, unsigned long int defaultValue) const
{
  return ulongBaseValue(QString("Weapon_%1:%2").arg(id() - 1).arg(name), type, defaultValue);
}

long int BosonWeaponProperties::longWeaponBaseValue(const QString& name, const QString& type, long int defaultValue) const
{
  return longBaseValue(QString("Weapon_%1:%2").arg(id() - 1).arg(name), type, defaultValue);
}

bofixed BosonWeaponProperties::bofixedWeaponBaseValue(const QString& name, const QString& type, bofixed defaultValue) const
{
  return bofixedBaseValue(QString("Weapon_%1:%2").arg(id() - 1).arg(name), type, defaultValue);
}

void BosonWeaponProperties::loadPlugin(KSimpleConfig* cfg, bool full)
{
  // FIXME: don't load all values for all weapon types
  mName = cfg->readEntry("Name", "");
  // Find out type of the weapon
  QString shottype = cfg->readEntry("Type", "Rocket");
  if(shottype == "Bullet")
  {
    mShotType = BosonShot::Bullet;
  }
  else if(shottype == "Rocket")
  {
    mShotType = BosonShot::Rocket;
  }
  else if(shottype == "Missile")
  {
    mShotType = BosonShot::Missile;
  }
  else if(shottype == "Mine")
  {
    mShotType = BosonShot::Mine;
  }
  else if(shottype == "Bomb")
  {
    mShotType = BosonShot::Bomb;
  }
  else
  {
    // Default to rocket
    mShotType = BosonShot::Rocket;
  }
  // Other parameters
  insertULongWeaponBaseValue(cfg->readUnsignedLongNumEntry("Range", 0), "Range", "MaxValue");
  mMaxFlyDistance = cfg->readDoubleNumEntry("MaxFlyDistance", ulongWeaponBaseValue("Range", "MaxValue") * 1.5f);
  mStartAngle = cfg->readDoubleNumEntry("StartAngle", -1);
  if(mStartAngle != -1 && (mStartAngle < 0 || mStartAngle > 90))
  {
    boError() << k_funcinfo << "StartAngle must be in range 0-90 or -1" << endl;
    mStartAngle = -1;
  }
  // Reload interval is converted from seconds to advance calls here
  insertULongWeaponBaseValue((unsigned long int)(cfg->readDoubleNumEntry("Reload", 0) * 20.0f), "Reload", "MaxValue");
 // We divide speeds with 20, because speeds in config files are cells/second,
 //  but we want cells/advance call
  insertBoFixedWeaponBaseValue(cfg->readDoubleNumEntry("Speed", 0) / 20.0f, "Speed", "MaxValue");
  if(speed() == 0 && mShotType == BosonShot::Rocket)
  {
    boWarning() << k_funcinfo << "Type is rocket, but speed is 0, setting type to bullet" << endl;
    mShotType = BosonShot::Bullet;
  }
  mAccelerationSpeed = (cfg->readDoubleNumEntry("AccelerationSpeed", 4) / 20.0f / 20.0f);
  bofixed turningspeed = (cfg->readDoubleNumEntry("TurningSpeed", 120) / 20.0f);
  // We convert turning speed for performace reasons
  mTurningSpeed = tan(turningspeed * DEG2RAD);
  insertLongWeaponBaseValue(cfg->readLongNumEntry("Damage", 0), "Damage", "MaxValue");
  insertBoFixedWeaponBaseValue(cfg->readDoubleNumEntry("DamageRange", 1), "DamageRange", "MaxValue");
  insertBoFixedWeaponBaseValue(cfg->readDoubleNumEntry("FullDamageRange", 0.25 * bofixedWeaponBaseValue("DamageRange")), "FullDamageRange", "MaxValue");
  if(bofixedWeaponBaseValue("FullDamageRange") > bofixedWeaponBaseValue("DamageRange"))
  {
    boWarning() << k_funcinfo << "FullDamageRange must not be bigger than DamageRange!" << endl;
    insertBoFixedWeaponBaseValue(bofixedWeaponBaseValue("DamageRange"), "FullDamageRange", "MaxValue");
  }
  mCanShootAtAirUnits = cfg->readBoolEntry("CanShootAtAirUnits", false);
  mCanShootAtLandUnits = cfg->readBoolEntry("CanShootAtLandUnits", false);
  mHeight = cfg->readDoubleNumEntry("Height", 0.25);
  mOffset = BosonConfig::readBoVector3FixedEntry(cfg, "Offset");
  mAutoUse = cfg->readBoolEntry("AutoUse", true);
  if(mAutoUse && (mShotType == BosonShot::Mine || mShotType == BosonShot::Bomb))
  {
    boWarning() << k_funcinfo << "AutoUse=true doesn't make sense for mines and bombs" << endl;
  }
  mTakeTargetVeloIntoAccount = cfg->readBoolEntry("TakeTargetVeloIntoAccount", false);
  mShootEffectIds = BosonConfig::readUnsignedLongNumList(cfg, "ShootEffects");
  mFlyEffectIds = BosonConfig::readUnsignedLongNumList(cfg, "FlyEffects");
  mHitEffectIds = BosonConfig::readUnsignedLongNumList(cfg, "HitEffects");
  // We need to have some kind of model even for bullet (though it won't be shown),
  //  because BosonShot will crash otherwise (actually it's BosonItem)
  mModelFileName = cfg->readEntry("Model", "missile");
  mSounds.clear();
  mSounds.insert(SoundWeaponShoot, cfg->readEntry("SoundShoot", "shoot"));
  mSounds.insert(SoundWeaponFly, cfg->readEntry("SoundFly", "missile_fly"));
  mSounds.insert(SoundWeaponHit, cfg->readEntry("SoundHit", "hit"));
  //loadAction(ActionAttackGround, cfg, "ActionAttackGround");
  if(shotType() == BosonShot::Mine)
  {
    loadAction(ActionLayMine, cfg, "ActionLayMine", true);
  }
  else if(shotType() == BosonShot::Bomb)
  {
    loadAction(ActionDropBomb, cfg, "ActionDropBomb", true);
  }
}

void BosonWeaponProperties::savePlugin(KSimpleConfig* cfg)
{
  // Group must have been set before
  // Save type
  QString shottype;
  if(mShotType == BosonShot::Bullet)
  {
    shottype == "Bullet";
  }
  else if(mShotType == BosonShot::Rocket)
  {
    shottype == "Rocket";
  }
  else if(mShotType == BosonShot::Missile)
  {
    shottype == "Missile";
  }
  else if(mShotType == BosonShot::Mine)
  {
    shottype == "Mine";
  }
  else if(mShotType == BosonShot::Bomb)
  {
    shottype == "Bomb";
  }
  else
  {
    boError() << k_funcinfo << "Invalid shot type: " << mShotType << endl;
    // Default to rocket
    mShotType = BosonShot::Rocket;
  }
  cfg->writeEntry("Type", shottype);
  // Other parameters
  cfg->writeEntry("Name", mName);
  cfg->writeEntry("Range", range());
  // Reload interval is converted from advance calls to seconds here
  cfg->writeEntry("Reload", reloadingTime() / 20.0f);
 // We multiply speeds with 20 because speeds in config files are cells/second,
 //  but here we have cells/advance calls
  cfg->writeEntry("Speed", speed() * 20.0f);
  cfg->writeEntry("AccelerationSpeed", mAccelerationSpeed * 20.0f * 20.0f);
  cfg->writeEntry("TurningSpeed", atan(mTurningSpeed) * RAD2DEG * 20.0f);
  cfg->writeEntry("StartAngle", mStartAngle);
  cfg->writeEntry("Damage", damage());
  cfg->writeEntry("DamageRange", damageRange());
  cfg->writeEntry("CanShootAtAirUnits", mCanShootAtAirUnits);
  cfg->writeEntry("CanShootAtLandUnits", mCanShootAtLandUnits);
  cfg->writeEntry("Height", (double)mHeight);
  BosonConfig::writeEntry(cfg, "Offset", mOffset);
  BosonConfig::writeUnsignedLongNumList(cfg, "ShootEffects", mShootEffectIds);
  BosonConfig::writeUnsignedLongNumList(cfg, "FlyEffects", mFlyEffectIds);
  BosonConfig::writeUnsignedLongNumList(cfg, "HitEffects", mHitEffectIds);
  cfg->writeEntry("SoundShoot", mSounds[SoundWeaponShoot]);
  cfg->writeEntry("SoundFly", mSounds[SoundWeaponFly]);
  cfg->writeEntry("SoundHit", mSounds[SoundWeaponHit]);
}

void BosonWeaponProperties::reset()
{
  mName = "";
  insertULongWeaponBaseValue(0, "Range", "MaxValue");
  insertLongWeaponBaseValue(0, "Damage", "MaxValue");
  insertBoFixedWeaponBaseValue(1, "DamageRange", "MaxValue");
  insertBoFixedWeaponBaseValue(0.25 * bofixedWeaponBaseValue("DamageRange"), "FullDamageRange", "MaxValue");
  insertULongWeaponBaseValue(0, "Reload", "MaxValue");
  insertBoFixedWeaponBaseValue(0, "Speed", "MaxValue");
  mAccelerationSpeed = 0;
  mTurningSpeed = tan(120 * DEG2RAD / 20.0f);
  mStartAngle = -1;
  mShotType = BosonShot::Rocket;
  mCanShootAtAirUnits = false;
  mCanShootAtLandUnits = false;
  mHeight = 0.25;
  mOffset = BoVector3Fixed();
  mShootEffectIds.clear();
  mFlyEffectIds.clear();
  mHitEffectIds.clear();
  mModelFileName = "missile.3ds";
  mSounds.clear();
  mSounds.insert(SoundWeaponShoot, "shoot");
  mSounds.insert(SoundWeaponFly, "missile_fly");
  mSounds.insert(SoundWeaponHit, "hit");
}

BosonShot* BosonWeaponProperties::newShot(Unit* attacker, const BosonWeapon* weapon, BoVector3Fixed pos, BoVector3Fixed target) const
{
  if(!attacker)
  {
    boError() << k_funcinfo << "NULL attacker" << endl;
    return 0;
  }
  BosonShot* s = 0;
  BosonCanvas* canvas = attacker->canvas();
  BO_CHECK_NULL_RET0(canvas);
  BO_CHECK_NULL_RET0(attacker->owner());
  BoVector3Fixed realPos;
  switch(shotType())
  {
    case BosonShot::Bullet:
    {
      ItemType type(BosonShot::Bullet, unitProperties()->typeId(), id());
      s = (BosonShot*)canvas->createNewItem(RTTI::Shot, attacker->owner(), type, pos);
      break;
    }
    case BosonShot::Rocket:
    {
      BoVector3Float realPosFloat;
      BoMatrix m;
      m.rotate(attacker->rotation(), 0, 0, 1);
      BoVector3Float offset = mOffset.toFloat();
      m.transform(&realPosFloat, &offset);
      realPos = realPosFloat.toFixed();
      realPos += pos;
      ItemType type(BosonShot::Rocket, unitProperties()->typeId(), id());
      s = (BosonShot*)canvas->createNewItem(RTTI::Shot, attacker->owner(), type, realPos);
      break;
    }
    case BosonShot::Mine:
    {
      ItemType type(BosonShot::Mine, unitProperties()->typeId(), id());
      s = (BosonShot*)canvas->createNewItem(RTTI::Shot, attacker->owner(), type, pos);
      break;
    }
    case BosonShot::Bomb:
    {
      BoVector3Float realPosFloat;
      BoMatrix m;
      m.rotate(attacker->rotation(), 0, 0, 1);
      BoVector3Float offset = mOffset.toFloat();
      m.transform(&realPosFloat, &offset);
      realPos = realPosFloat.toFixed();
      realPos += pos;
      ItemType type(BosonShot::Bomb, unitProperties()->typeId(), id());
      s = (BosonShot*)canvas->createNewItem(RTTI::Shot, attacker->owner(), type, pos);
      break;
    }
    default:
    {
      boError() << k_funcinfo << "Invalid shotType: " << shotType() << endl;
      return 0;
    }
  }

  if(!s)
  {
    return 0;
  }

  s->applyWeapon(weapon);

  switch(shotType())
  {
    case BosonShot::Bullet:
    {
      ((BosonShotBullet*)s)->setTarget(target);
      s->explode();
      break;
    }
    case BosonShot::Rocket:
    {
      ((BosonShotRocket*)s)->init(realPos, target);
      break;
    }
    case BosonShot::Mine:
    {
      ((BosonShotMine*)s)->init(pos);
      break;
    }
    case BosonShot::Bomb:
    {
      ((BosonShotBomb*)s)->init(realPos);
      break;
    }
    default:
    {
      boError() << k_funcinfo << "Invalid shotType: " << shotType() << endl;
      return 0;
    }
  }

  return s;
}

BosonShot* BosonWeaponProperties::newShot(Unit* attacker, const BosonWeapon* weapon, BoVector3Fixed pos, Unit* target) const
{
  if(!attacker)
  {
    boError() << k_funcinfo << "NULL attacker" << endl;
    return 0;
  }
  BosonShot* s = 0;
  BosonCanvas* canvas = attacker->canvas();
  BO_CHECK_NULL_RET0(canvas);
  BO_CHECK_NULL_RET0(attacker->owner());
  BoVector3Fixed realPos;
  switch(shotType())
  {
    case BosonShot::Missile:
    {
      BoVector3Float realPosFloat;
      BoMatrix m;
      m.rotate(attacker->rotation(), 0, 0, 1);
      BoVector3Float offset = mOffset.toFloat();
      m.transform(&realPosFloat, &offset);
      realPos = realPosFloat.toFixed();
      realPos += pos;
      ItemType type(BosonShot::Missile, unitProperties()->typeId(), id());
      s = (BosonShot*)canvas->createNewItem(RTTI::Shot, attacker->owner(), type, realPos);
      break;
    }
    default:
    {
      boWarning() << k_funcinfo << "Use newShot() which takes target position parameter for shottype " << shotType() << endl;
      BoVector3Fixed targetpos(target->centerX(), target->centerY(), target->z());

      // abort further processing, return.
      return newShot(attacker, weapon, pos, targetpos);
    }
  }

  if(!s)
  {
    return 0;
  }
  s->applyWeapon(weapon);

  switch(shotType())
  {
    case BosonShot::Missile:
      ((BosonShotMissile*)s)->init(realPos, target);
      break;
    default:
      boError() << k_funcinfo << "unhandled shot type " << shotType() << endl;
      break;
  }

  return s;
}

void BosonWeaponProperties::setSound(int event, QString filename)
{
  mSounds.insert(event, filename);
}

QString BosonWeaponProperties::sound(int soundEvent) const
{
  return mSounds[soundEvent];
}

void BosonWeaponProperties::playSound(WeaponSoundEvent event) const
{
  speciesTheme()->playSound(this, event);
}

QMap<int, QString> BosonWeaponProperties::sounds() const
{
  return mSounds;
}

void BosonWeaponProperties::loadAction(UnitAction type, KSimpleConfig* cfg, const QString& key, bool useDefault)
{
  if(!speciesTheme())
  {
    // No speciestheme. We're probably in unit editor
    return;
  }
  if(!cfg->hasKey(key) && !useDefault)
  {
    return;
  }
  mActions.insert(type, speciesTheme()->action(cfg->readEntry(key, key)));
}


/*****  BosonWeapon  *****/
BosonWeapon::BosonWeapon(int weaponNumber, BosonWeaponProperties* prop, Unit* _unit)
    : UnitPlugin(_unit),
    // AB: note that ids start with _1_ here, but with _0_ in the config files.
    // so we must subtract one for the strings.
    mRange(          prop, QString("Weapon_%1:Range").arg(prop->id()-1), "MaxValue"),
    mDamage(         prop, QString("Weapon_%1:Damage").arg(prop->id()-1), "MaxValue"),
    mDamageRange(    prop, QString("Weapon_%1:DamageRange").arg(prop->id()-1), "MaxValue"),
    mFullDamageRange(prop, QString("Weapon_%1:FullDamageRange").arg(prop->id()-1), "MaxValue"),
    mReloadingTime(  prop, QString("Weapon_%1:Reload").arg(prop->id()-1), "MaxValue"),
    mSpeed(          prop, QString("Weapon_%1:Speed").arg(prop->id()-1), "MaxValue")
{
  mProp = prop;
  if (!unit())
  {
    boError() << k_funcinfo << "NULL unit" << endl;
  }
  registerWeaponData(weaponNumber, &mReloadCounter, IdReloadCounter);
  mReloadCounter.setLocal(0);
  mReloadCounter.setEmittingSignal(false);
}

BosonWeapon::~BosonWeapon()
{
}

void BosonWeapon::registerWeaponData(int weaponNumber, KGamePropertyBase* prop, int id, bool local)
{
 if(!unit())
 {
   boError() << k_funcinfo << "NULL unit" << endl;
   return;
 }
 if(!prop)
 {
   boError() << k_funcinfo << "NULL property" << endl;
   return;
 }
 if (id < KGamePropertyBase::IdUser)
 {
   boWarning() << k_funcinfo << "ID < KGamePropertyBase::IdUser" << endl;
   // do not return - might still work
 }
 QString name;
 switch (id)
 {
   // AB: in UnitBase we use propertyName() for this, in order to be able to use
   // the name in the scenario files, too.
   // it is easier to use a simple switch here, but that means that we won't be
   // able to use the weapon properties in the scenario files (not with names at
   // least)!
   // I hope we won't need this anyway for weapons.
   case IdReloadCounter:
     name = QString::fromLatin1("ReloadCounter");
     break;
   default:
     break;
 }
 if (name.isNull())
 {
   boDebug() << k_funcinfo << "No weapon property name for " << id << endl;
   // a name isn't necessary, so don't return
 }
 id += weaponNumber * 100; // we support up to 100 IDs per weapon. we'll never use them.
 prop->registerData(id, unit()->weaponDataHandler(),
		local ? KGamePropertyBase::PolicyLocal : KGamePropertyBase::PolicyClean,
		name);
}

bool BosonWeapon::saveAsXML(QDomElement& /*root*/) const
{
 // AB: nothing to do here.
 return true;
}

bool BosonWeapon::loadFromXML(const QDomElement& /*root*/)
{
 // AB: nothing to do here.
 return true;
}

bool BosonWeapon::canShootAt(Unit* u) const
{
  if(u->isFlying())
  {
    return mProp->canShootAtAirUnits();
  }
  else
  {
    return mProp->canShootAtLandUnits();
  }
}

void BosonWeapon::shoot(Unit* u)
{
  BoVector3Fixed mypos(unit()->centerX(), unit()->centerY(), unit()->z());
  if(mProp->shotType() == BosonShot::Missile)
  {
    BosonShot* shot = mProp->newShot(unit(), this, mypos, u);
    canvas()->shotFired(shot, this);
    mReloadCounter = reloadingTime();
    return;
  }

  BoVector3Fixed targetpos(u->centerX(), u->centerY(), u->z());
  targetpos += u->unitProperties()->hitPoint();
  if(mProp->takeTargetVeloIntoAccount() && mProp->shotType() == BosonShot::Rocket)
  {
    // Calculate how much time it should take to reach target
    // Note that this is just _approximation_! It doesn't take many things,
    //  e.g. acceleration into account.
    float dist = (targetpos - mypos).length();
    float time = dist / speed();
    targetpos += BoVector3Fixed(u->xVelocity(), u->yVelocity(), u->zVelocity()) * time;
  }
  if (!unit())
  {
    boError() << k_funcinfo << "NULL unit" << endl;
    return;
  }
  shoot(mypos, targetpos);
}

void BosonWeapon::shoot(const BoVector3Fixed& target)
{
  if (!unit())
  {
    boError() << k_funcinfo << "NULL unit" << endl;
    return;
  }
  shoot(BoVector3Fixed(unit()->centerX(), unit()->centerY(), unit()->z()), target);
}

void BosonWeapon::shoot(const BoVector3Fixed& pos, const BoVector3Fixed& target)
{
  if (!unit())
  {
    boError() << k_funcinfo << "NULL unit" << endl;
    return;
  }
  BosonShot* shot = mProp->newShot(unit(), this, pos, target);
  canvas()->shotFired(shot, this);
  mReloadCounter = reloadingTime();
}

bool BosonWeapon::layMine()
{
  boDebug() << k_funcinfo << "" << endl;
  if (properties()->shotType() != BosonShot::Mine || !reloaded()) {
    boDebug() << k_funcinfo << "weapon is not minelayer or not reloaded" << endl;
    return false;
  }
  BoVector3Fixed pos(unit()->centerX(), unit()->centerY(), 0);
  pos.setZ(unit()->canvas()->heightAtPoint(pos.x(), pos.y()));
  // Substract half the object size from pos, so that mine's center will be at pos
  pos += BoVector3Fixed(-0.25, -0.25, 0);  // FIXME: use real object size
  shoot(pos, pos);
  boDebug() << k_funcinfo << "done" << endl;
  return true;
}

bool BosonWeapon::dropBomb()
{
  boDebug() << k_funcinfo << "" << endl;
  if (properties()->shotType() != BosonShot::Bomb || !reloaded()) {
    boDebug() << k_funcinfo << "weapon is not bomb or not reloaded" << endl;
    return false;
  }
  BoVector3Fixed pos(unit()->centerX(), unit()->centerY(), unit()->z());
  // Substract half the object size from pos, so that bomb's center will be at pos
  pos += BoVector3Fixed(-0.25, -0.25, 0);  // FIXME: use real object size
  shoot(pos, pos);
  boDebug() << k_funcinfo << "done" << endl;
  return true;
}

/*
 * vim: et sw=2
 */
