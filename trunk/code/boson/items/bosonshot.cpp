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

#include "bosonshot.h"

#include "../unit.h"
#include "../unitproperties.h"
#include "../speciestheme.h"
#include "../bosonmodel.h"
#include "../player.h"
#include "../global.h"
#include "../bosoncanvas.h"
#include "../bosonparticlesystem.h"
#include "../bosonparticlemanager.h"
#include "../bo.h"
#include "../bosonmodel.h"

#include <kdebug.h>
#include <ksimpleconfig.h>

#include <GL/gl.h>

#include <math.h>

/*****  BosonShotProperties  *****/
BosonShotProperties::BosonShotProperties(SpeciesTheme* theme, KSimpleConfig* cfg)
{
  mTheme = theme;
  mId = cfg->readUnsignedLongNumEntry("Id", 0);
  if(mId == 0)
  {
    kdError() << k_funcinfo << "Invalid id in group " << cfg->group() << endl;
  }
  mDamage = cfg->readUnsignedLongNumEntry("Damage", 0);
  mSpeed = cfg->readLongNumEntry("Speed", 0);
  mDamageRange = (float)(cfg->readDoubleNumEntry("DamageRange", 1));
  mFlyParticleSystems = Bo::loadParticleSystemProperties(cfg, "FlyParticles", theme);
  kdDebug() << "    " << k_funcinfo << "There are " << mFlyParticleSystems.count() << " particle systems in fly list" << endl;
  mHitParticleSystems = Bo::loadParticleSystemProperties(cfg, "HitParticles", theme);
  kdDebug() << "    " << k_funcinfo << "There are " << mHitParticleSystems.count() << " particle systems in hit list" << endl;
  // We need to have some kind of model even for bullet (though it won't be shown),
  //  because BosonItem will crash otherwise
  mModel = theme->objectModel(cfg->readEntry("Model", "missile.3ds"));
}

BosonShotProperties::~BosonShotProperties()
{
}

BosonShot* BosonShotProperties::newShot(Unit* attacker, float x, float y, float z, float tx, float ty, float tz)
{
  return new BosonShot(this, attacker, x, y, z, tx, ty, tz);
}

QPtrList<BosonParticleSystem> BosonShotProperties::newFlyParticleSystems(float x, float y, float z) const
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

QPtrList<BosonParticleSystem> BosonShotProperties::newHitParticleSystems(float x, float y, float z) const
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


/*****  BosonShot  *****/
BosonShot::BosonShot(BosonShotProperties* prop, Unit* attacker, float x, float y, float z, float tx, float ty, float tz) :
    BosonItem(prop->model(), attacker->canvas())
{
  kdDebug() << "MISSILE: " << k_funcinfo << "Creating new shot" << endl;
  mOwner = attacker->owner();
  mProp = prop;
  if(prop->speed() == 0)
  {
    // This shot is bullet, not missile - it has infinite speed and it reaches
    //  it's target immideately. No need to calculate anything.
    kdDebug() << "MISSILE: " << k_funcinfo << "    Attacker's shot is bullet (infinite speed). Returning" << endl;
    move(tx, ty, tz);
    mActive = false;
    return;
  }
  mVelo.set(tx - x, ty - y, tz - z);
  mLength = mVelo.length();
  //kdDebug() << "MISSILE: " << k_funcinfo << "    Length of trip: " << length << endl;
  mTotalSteps = (int)ceilf(mLength / prop->speed());
  mStep = 0;
  //kdDebug() << "MISSILE: " << k_funcinfo << "    Steps: " << mSteps << endl;
  mVelo.scale(prop->speed() / mLength);
  //kdDebug() << "MISSILE: " << k_funcinfo << "    Normalized & scaled (final) velocity: (" << mVelo[0] << "; " << mVelo[1] << "; " << mVelo[2] << ")" << endl;
  mActive = true;
  move(x, y, z);
  setAnimated(true);
  setRotation(rotationToPoint(mVelo[0], mVelo[1]));
  mFlyParticleSystems = prop->newFlyParticleSystems(x, y, z);
  attacker->canvas()->addParticleSystems(mFlyParticleSystems);
  mZ = z;
}

void BosonShot::advance(unsigned int phase)
{
  BosonItem::advance(phase);
  float factor = mStep / (float)mTotalSteps - 0.5;
  float newZ = (-4 * (factor * factor) + 1) * BO_TILE_SIZE;
  //cout << k_funcinfo << "newZ: " << newZ << "; factor: " << factor << "; mStep: " << mStep << "; mTotalSteps: " << mTotalSteps << endl;
  moveBy(mVelo[0], mVelo[1], mVelo[2] + (newZ - mZ));
  // Move all "fly" particles.
  BoVector3 move(mVelo[0], -(mVelo[1]), mVelo[2] + (newZ - mZ));
  move.scale(1 / (float)BO_TILE_SIZE);
  mZ = newZ;
  QPtrListIterator<BosonParticleSystem> it(mFlyParticleSystems);
  while(it.current())
  {
    it.current()->moveParticles(move);
    ++it;
  }
  mStep++;
  if(mStep >= mTotalSteps)
  {
    mActive = false;
  }
}

float BosonShot::rotationToPoint(float x, float y)
{
  float add = 0;
  float arg = 0;
  if(x > 0)
  {
    if(y < 0)
    {
      add = 0;
      arg = x / -y;
    }
    else
    {
      add = 90;
      arg = y / x;
    }
  }
  else
  {
    if(y > 0)
    {
      add = 180;
      arg = -x / y;
    }
    else
    {
      add = 270;
      arg = -y / -x;
    }
  }

  return (atan(arg) * (360 / 6.2831853)) + add;
}
