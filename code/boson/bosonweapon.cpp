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
#include "bosonparticlemanager.h"
#include "unit.h"
#include "global.h"
#include "bosoncanvas.h"

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

void BosonWeaponProperties::loadPlugin(KSimpleConfig* cfg)
{
  mRange = cfg->readUnsignedLongNumEntry("Range", 0);
  mReload = cfg->readUnsignedNumEntry("Reload", 0);
  mSpeed = cfg->readLongNumEntry("Speed", 0);
  mDamage = cfg->readUnsignedLongNumEntry("Damage", 0);
  mDamageRange = (float)(cfg->readDoubleNumEntry("DamageRange", 1));
  mCanShootAtAirUnits = cfg->readBoolEntry("CanShootAtAirUnits", false);
  mCanShootAtLandUnits = cfg->readBoolEntry("CanShootAtLandUnits", false);
  mMaxHeight = (float)(cfg->readDoubleNumEntry("MaxHeight", 1));
  mShootParticleSystems = BosonParticleSystemProperties::loadParticleSystemProperties(cfg, "ShootParticles", speciesTheme());
  mFlyParticleSystems = BosonParticleSystemProperties::loadParticleSystemProperties(cfg, "FlyParticles", speciesTheme());
  mHitParticleSystems = BosonParticleSystemProperties::loadParticleSystemProperties(cfg, "HitParticles", speciesTheme());
  // We need to have some kind of model even for bullet (though it won't be shown),
  //  because BosonShot will crash otherwise (actually it's BosonItem)
  mModel = speciesTheme()->objectModel(cfg->readEntry("Model", "missile.3ds"));
}

void BosonWeaponProperties::savePlugin(KSimpleConfig* cfg)
{
  /// TODO!!!
}

BosonShot* BosonWeaponProperties::newShot(Unit* attacker, float x, float y, float z, float tx, float ty, float tz) const
{
  return new BosonShot(this, attacker, x, y, z, tx, ty, tz);
}

QPtrList<BosonParticleSystem> BosonWeaponProperties::newShootParticleSystems(float x, float y, float z) const
{
  QPtrList<BosonParticleSystem> list;
  QPtrListIterator<BosonParticleSystemProperties> it(mShootParticleSystems);
  while(it.current())
  {
    list.append(it.current()->newSystem(x, y, z));
    ++it;
  }
  return list;
}

QPtrList<BosonParticleSystem> BosonWeaponProperties::newFlyParticleSystems(float x, float y, float z) const
{
  QPtrList<BosonParticleSystem> list;
  QPtrListIterator<BosonParticleSystemProperties> it(mFlyParticleSystems);
  while(it.current())
  {
    list.append(it.current()->newSystem(x, y, z));
    ++it;
  }
  return list;
}

QPtrList<BosonParticleSystem> BosonWeaponProperties::newHitParticleSystems(float x, float y, float z) const
{
  QPtrList<BosonParticleSystem> list;
  QPtrListIterator<BosonParticleSystemProperties> it(mHitParticleSystems);
  while(it.current())
  {
    list.append(it.current()->newSystem(x, y, z));
    ++it;
  }
  return list;
}


/*****  BosonWeapon  *****/
BosonWeapon::BosonWeapon(BosonWeaponProperties* prop, Unit* unit)
{
  mProp = prop;
  mUnit = unit;
  mReloadCounter = 0;
}

BosonWeapon::~BosonWeapon()
{
}

bool BosonWeapon::canShootAt(Unit* u)
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
  shoot(u->x() + u->width() / 2, u->y() + u->height() / 2, u->z());
}

void BosonWeapon::shoot(float x, float y, float z)
{
  BoVector3 pos(mUnit->x() + mUnit->width() / 2, mUnit->y() + mUnit->height() / 2, mUnit->z());
  mUnit->canvas()->newShot(mProp->newShot(mUnit, pos[0], pos[1], pos[2], x, y, z));
  mUnit->canvas()->addParticleSystems(mProp->newShootParticleSystems(pos[0], pos[1], pos[2]));
  mUnit->playSound(SoundShoot);  // TODO: weapon-specific sounds
  mReloadCounter = mProp->reloadingTime();
}
