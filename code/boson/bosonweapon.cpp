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

#include "bosonweapon.h"

#include "speciestheme.h"
#include "bosoneffect.h"
#include "bosoneffectproperties.h"
#include "unit.h"
#include "global.h"
#include "bosoncanvas.h"
#include "bodebug.h"
#include "bosonconfig.h"
#include "unitproperties.h"
#include "boaction.h"

#include <ksimpleconfig.h>

/*****  BosonWeaponProperties  *****/
BosonWeaponProperties::BosonWeaponProperties(const UnitProperties* prop, unsigned long int id) :
    PluginProperties(prop)
{
  mId = id;
}

BosonWeaponProperties::~BosonWeaponProperties()
{
}

QString BosonWeaponProperties::name() const
{
  return "Weapon";
}

void BosonWeaponProperties::loadPlugin(KSimpleConfig* cfg, bool full)
{
  // FIXME: don't load all values for all weapon types
  mName = cfg->readEntry("Name", "");
  mShotType = (BosonShot::Type)(cfg->readNumEntry("Type", (int)BosonShot::Missile));
  mRange = cfg->readUnsignedLongNumEntry("Range", 0);
  mReload = cfg->readUnsignedNumEntry("Reload", 0);
  mSpeed = (cfg->readDoubleNumEntry("Speed", 0)) / 48.0f;
  if(mSpeed == 0 && mShotType == BosonShot::Missile)
  {
    boWarning() << k_funcinfo << "Type is missile, but speed is 0, setting type to bullet" << endl;
    mShotType = BosonShot::Bullet;
  }
  mAccelerationSpeed = (cfg->readDoubleNumEntry("AccelerationSpeed", 0.2)) / 48.0f;
  mDamage = cfg->readUnsignedLongNumEntry("Damage", 0);
  mDamageRange = cfg->readDoubleNumEntry("DamageRange", 1);
  mFullDamageRange = cfg->readDoubleNumEntry("FullDamageRange", 0.25 * mDamageRange);
  if(mFullDamageRange > mDamageRange)
  {
    boWarning() << k_funcinfo << "FullDamageRange must not be bigger than DamageRange!" << endl;
    mFullDamageRange = mDamageRange;
  }
  mCanShootAtAirUnits = cfg->readBoolEntry("CanShootAtAirUnits", false);
  mCanShootAtLandUnits = cfg->readBoolEntry("CanShootAtLandUnits", false);
  mHeight = cfg->readDoubleNumEntry("Height", 0.25);
  mOffset = BosonConfig::readBoVector3Entry(cfg, "Offset");
  mAutoUse = cfg->readBoolEntry("AutoUse", true);
  if(mAutoUse && (mShotType == BosonShot::Mine || mShotType == BosonShot::Bomb))
  {
    boWarning() << k_funcinfo << "AutoUse=true doesn't make sense for mines and bombs" << endl;
  }
  mShootEffectIds = BosonConfig::readUnsignedLongNumList(cfg, "ShootEffects");
  mFlyEffectIds = BosonConfig::readUnsignedLongNumList(cfg, "FlyEffects");
  mHitEffectIds = BosonConfig::readUnsignedLongNumList(cfg, "HitEffects");
  if(full)
  {
    mShootEffects = BosonEffectProperties::loadEffectProperties(mShootEffectIds);
    mFlyEffects = BosonEffectProperties::loadEffectProperties(mFlyEffectIds);
    mHitEffects = BosonEffectProperties::loadEffectProperties(mHitEffectIds);
  }
  // We need to have some kind of model even for bullet (though it won't be shown),
  //  because BosonShot will crash otherwise (actually it's BosonItem)
  mModelFileName = cfg->readEntry("Model", "missile");
  if(full)
  {
    mModel = speciesTheme()->objectModel(mModelFileName);
  }
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
  cfg->writeEntry("Name", mName);
  cfg->writeEntry("Range", mRange);
  cfg->writeEntry("Reload", mReload);
  cfg->writeEntry("Speed", mSpeed * 48.0f);
  cfg->writeEntry("AccelerationSpeed", mAccelerationSpeed * 48.0f);
  cfg->writeEntry("Damage", mDamage);
  cfg->writeEntry("DamageRange", mDamageRange);
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
  mRange = 0;
  mReload = 0;
  mSpeed = 0;
  mAccelerationSpeed = 0;
  mDamage = 0;
  mDamageRange = 1;
  mFullDamageRange = 0.25 * mDamageRange;
  mCanShootAtAirUnits = false;
  mCanShootAtLandUnits = false;
  mHeight = 0.25;
  mOffset = BoVector3();
  mShootEffectIds.clear();
  mFlyEffectIds.clear();
  mHitEffectIds.clear();
  mModelFileName = "missile.3ds";
  mSounds.clear();
  mSounds.insert(SoundWeaponShoot, "shoot");
  mSounds.insert(SoundWeaponFly, "missile_fly");
  mSounds.insert(SoundWeaponHit, "hit");
}

BosonShot* BosonWeaponProperties::newShot(Unit* attacker, BoVector3 pos, BoVector3 target) const
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
  if(shotType() == BosonShot::Bullet)
  {
    ItemType type(BosonShot::Bullet, unitProperties()->typeId(), id());
    s = (BosonShot*)canvas->createNewItem(RTTI::Shot, attacker->owner(), type, pos);
    ((BosonShotBullet*)s)->setTarget(target);
    s->explode();
  }
  else if(shotType() == BosonShot::Missile)
  {
    BoVector3 realpos;
    BoMatrix m;
    m.rotate(attacker->rotation(), 0, 0, 1);
    m.transform(&realpos, &mOffset);
    realpos += pos;
    ItemType type(BosonShot::Missile, unitProperties()->typeId(), id());
    s = (BosonShot*)canvas->createNewItem(RTTI::Shot, attacker->owner(), type, realpos);
    ((BosonShotMissile*)s)->init(realpos, target);
  }
  else if(shotType() == BosonShot::Mine)
  {
    ItemType type(BosonShot::Mine, unitProperties()->typeId(), id());
    s = (BosonShot*)canvas->createNewItem(RTTI::Shot, attacker->owner(), type, pos);
    ((BosonShotMine*)s)->init(pos);
  }
  else if(shotType() == BosonShot::Bomb)
  {
    BoVector3 realpos;
    BoMatrix m;
    m.rotate(attacker->rotation(), 0, 0, 1);
    m.transform(&realpos, &mOffset);
    realpos += pos;
    ItemType type(BosonShot::Bomb, unitProperties()->typeId(), id());
    s = (BosonShot*)canvas->createNewItem(RTTI::Shot, attacker->owner(), type, pos);
    ((BosonShotBomb*)s)->init(realpos);
  }
  else
  {
    boError() << k_funcinfo << "Invalid shotType: " << shotType() << endl;
    return 0;
  }
  return s;
}

QPtrList<BosonEffect> BosonWeaponProperties::newShootEffects(BoVector3 pos, bofixed rotation) const
{
  return BosonEffectProperties::newEffects(&mShootEffects, pos, BoVector3(0, 0, rotation));
}

QPtrList<BosonEffect> BosonWeaponProperties::newFlyEffects(BoVector3 pos, bofixed rotation) const
{
  return BosonEffectProperties::newEffects(&mFlyEffects, pos, BoVector3(0, 0, rotation));
}

QPtrList<BosonEffect> BosonWeaponProperties::newHitEffects(BoVector3 pos) const
{
  return BosonEffectProperties::newEffects(&mHitEffects, pos);
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
BosonWeapon::BosonWeapon(int weaponNumber, BosonWeaponProperties* prop, Unit* _unit) : UnitPlugin(_unit)
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
  shoot(BoVector3(u->centerX(), u->centerY(), u->z()) + u->unitProperties()->hitPoint());
}

void BosonWeapon::shoot(const BoVector3& target)
{
  if (!unit())
  {
    boError() << k_funcinfo << "NULL unit" << endl;
    return;
  }
  shoot(BoVector3(unit()->centerX(), unit()->centerY(), unit()->z()), target);
}

void BosonWeapon::shoot(const BoVector3& pos, const BoVector3& target)
{
  if (!unit())
  {
    boError() << k_funcinfo << "NULL unit" << endl;
    return;
  }
  mProp->newShot(unit(), pos, target);
  canvas()->addEffects(mProp->newShootEffects(pos, unit()->rotation()));
  mProp->playSound(SoundWeaponShoot);
  mReloadCounter = mProp->reloadingTime();
}

bool BosonWeapon::layMine()
{
  boDebug() << k_funcinfo << "" << endl;
  if (properties()->shotType() != BosonShot::Mine || !reloaded()) {
    boDebug() << k_funcinfo << "weapon is not minelayer or not reloaded" << endl;
    return false;
  }
  BoVector3 pos(unit()->centerX(), unit()->centerY(), 0);
  pos.setZ(unit()->canvas()->heightAtPoint(pos.x(), pos.y()));
  // Substract half the object size from pos, so that mine's center will be at pos
  pos += BoVector3(-0.25, -0.25, 0);  // FIXME: use real object size
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
  BoVector3 pos(unit()->centerX(), unit()->centerY(), unit()->z());
  // Substract half the object size from pos, so that bomb's center will be at pos
  pos += BoVector3(-0.25, -0.25, 0);  // FIXME: use real object size
  shoot(pos, pos);
  boDebug() << k_funcinfo << "done" << endl;
  return true;
}

/*
 * vim: et sw=2
 */
