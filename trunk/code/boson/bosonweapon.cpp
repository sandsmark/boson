/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bosonparticlesystem.h"
#include "bosonparticlesystemproperties.h"
#include "unit.h"
#include "global.h"
#include "bosoncanvas.h"
#include "bodebug.h"
#include "bosonconfig.h"

#include <ksimpleconfig.h>

/*****  BosonWeaponProperties  *****/
BosonWeaponProperties::BosonWeaponProperties(const UnitProperties* prop) :
    PluginProperties(prop)
{
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
  mName = cfg->readEntry("Name", "");
  mRange = cfg->readUnsignedLongNumEntry("Range", 0);
  mReload = cfg->readUnsignedNumEntry("Reload", 0);
  mSpeed = cfg->readLongNumEntry("Speed", 0);
  mDamage = cfg->readUnsignedLongNumEntry("Damage", 0);
  mDamageRange = (float)(cfg->readDoubleNumEntry("DamageRange", 1));
  mCanShootAtAirUnits = cfg->readBoolEntry("CanShootAtAirUnits", false);
  mCanShootAtLandUnits = cfg->readBoolEntry("CanShootAtLandUnits", false);
  mMaxHeight = (float)(cfg->readDoubleNumEntry("MaxHeight", 1));
  mOffset = BoVector3::load(cfg, "Offset");
  mOffset.scale(BO_TILE_SIZE);
  mShootParticleSystemIds = BosonConfig::readUnsignedLongNumList(cfg, "ShootParticles");
  mFlyParticleSystemIds = BosonConfig::readUnsignedLongNumList(cfg, "FlyParticles");
  mHitParticleSystemIds = BosonConfig::readUnsignedLongNumList(cfg, "HitParticles");
  if(full)
  {
    mShootParticleSystems = BosonParticleSystemProperties::loadParticleSystemProperties(mShootParticleSystemIds, speciesTheme());
    mFlyParticleSystems = BosonParticleSystemProperties::loadParticleSystemProperties(mFlyParticleSystemIds, speciesTheme());
    mHitParticleSystems = BosonParticleSystemProperties::loadParticleSystemProperties(mHitParticleSystemIds, speciesTheme());
  }
  // We need to have some kind of model even for bullet (though it won't be shown),
  //  because BosonShot will crash otherwise (actually it's BosonItem)
  mModelFileName = cfg->readEntry("Model", "missile.3ds");
  if(full) 
  {
    mModel = speciesTheme()->objectModel(mModelFileName);
  }
}

void BosonWeaponProperties::savePlugin(KSimpleConfig* cfg)
{
  // Group must have been set before
  cfg->writeEntry("Name", mName);
  cfg->writeEntry("Range", mRange);
  cfg->writeEntry("Reload", mReload);
  cfg->writeEntry("Speed", mSpeed);
  cfg->writeEntry("Damage", mDamage);
  cfg->writeEntry("DamageRange", mDamageRange);
  cfg->writeEntry("CanShootAtAirUnits", mCanShootAtAirUnits);
  cfg->writeEntry("CanShootAtLandUnits", mCanShootAtLandUnits);
  cfg->writeEntry("MaxHeight", (double)mMaxHeight);
  mOffset.save(cfg, "Offset");
  BosonConfig::writeUnsignedLongNumList(cfg, "ShootParticles", mShootParticleSystemIds);
  BosonConfig::writeUnsignedLongNumList(cfg, "FlyParticles", mFlyParticleSystemIds);
  BosonConfig::writeUnsignedLongNumList(cfg, "HitParticles", mHitParticleSystemIds);
}

void BosonWeaponProperties::reset()
{
 mName = "";
 mRange = 0;
 mReload = 0;
 mSpeed = 0;
 mDamage = 0;
 mDamageRange = 1;
 mCanShootAtAirUnits = false;
 mCanShootAtLandUnits = false;
 mMaxHeight = 1;
 mOffset = BoVector3();
 mShootParticleSystemIds.clear();
 mFlyParticleSystemIds.clear();
 mHitParticleSystemIds.clear();
 mModelFileName = "missile.3ds";
}

BosonShot* BosonWeaponProperties::newShot(Unit* attacker, BoVector3 pos, BoVector3 target) const
{
  if(!attacker)
  {
    boError() << k_funcinfo << "NULL attacker" << endl;
    return 0;
  }
  return new BosonShot(this, attacker->owner(), attacker->canvas(), pos + mOffset, target);
}

QPtrList<BosonParticleSystem> BosonWeaponProperties::newShootParticleSystems(BoVector3 pos, float rotation) const
{
  BoVector3 realpos = pos + mOffset;
  QPtrList<BosonParticleSystem> list;
  QPtrListIterator<BosonParticleSystemProperties> it(mShootParticleSystems);
  while(it.current())
  {
    BosonParticleSystem* s = it.current()->newSystem(pos);
    s->setRotation(BoVector3(0, 0, -rotation));
    list.append(s);
    ++it;
  }
  return list;
}

QPtrList<BosonParticleSystem> BosonWeaponProperties::newFlyParticleSystems(BoVector3 pos) const
{
  QPtrList<BosonParticleSystem> list;
  QPtrListIterator<BosonParticleSystemProperties> it(mFlyParticleSystems);
  while(it.current())
  {
    list.append(it.current()->newSystem(pos));
    ++it;
  }
  return list;
}

QPtrList<BosonParticleSystem> BosonWeaponProperties::newHitParticleSystems(BoVector3 pos) const
{
  QPtrList<BosonParticleSystem> list;
  QPtrListIterator<BosonParticleSystemProperties> it(mHitParticleSystems);
  while(it.current())
  {
    list.append(it.current()->newSystem(pos));
    ++it;
  }
  return list;
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
  shoot(BoVector3(u->x() + u->width() / 2, u->y() + u->height() / 2, u->z()));
}

void BosonWeapon::shoot(BoVector3 target)
{
  if (!unit())
  {
    boError() << k_funcinfo << "NULL unit" << endl;
    return;
  }
  BoVector3 pos(unit()->x() + unit()->width() / 2, unit()->y() + unit()->height() / 2, unit()->z());
  canvas()->newShot(mProp->newShot(unit(), pos, target));
  canvas()->addParticleSystems(mProp->newShootParticleSystems(pos, unit()->rotation()));
  unit()->playSound(SoundShoot);  // TODO: weapon-specific sounds
  mReloadCounter = mProp->reloadingTime();
}

/*
 * vim: et sw=2
 */
