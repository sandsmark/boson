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
#include "../player.h"
#include "../global.h"
#include "../bosoncanvas.h"
#include "../bosonparticlesystem.h"
#include "../bosonweapon.h"
#include "../bodebug.h"

#include <ksimpleconfig.h>

#include <GL/gl.h>

#include <math.h>


BosonShot::BosonShot(const BosonWeaponProperties* prop, Unit* attacker, float x, float y, float z, float tx, float ty, float tz) :
    BosonItem(prop->model(), attacker->canvas())
{
  boDebug() << "MISSILE: " << k_funcinfo << "Creating new shot" << endl;
  mOwner = attacker->owner();
  mProp = prop;
  if(prop->speed() == 0)
  {
    // This shot is bullet, not missile - it has infinite speed and it reaches
    //  it's target immideately. No need to calculate anything.
    boDebug() << "MISSILE: " << k_funcinfo << "    Attacker's shot is bullet (infinite speed). Returning" << endl;
    move(tx, ty, tz);
    mActive = false;
    return;
  }
  mVelo.set(tx - x, ty - y, tz - z);
  mLength = mVelo.length();
  //boDebug() << "MISSILE: " << k_funcinfo << "    Length of trip: " << length << endl;
  mTotalSteps = (int)ceilf(mLength / prop->speed());
  mStep = 0;
  //boDebug() << "MISSILE: " << k_funcinfo << "    Steps: " << mSteps << endl;
  mVelo.scale(prop->speed() / mLength);
  //boDebug() << "MISSILE: " << k_funcinfo << "    Normalized & scaled (final) velocity: (" << mVelo[0] << "; " << mVelo[1] << "; " << mVelo[2] << ")" << endl;
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
  float factor = mStep / (float)mTotalSteps - 0.5;  // Factor will be in range -0.25 to 0.25
  float newZ = (-4 * (factor * factor) + 1) * BO_TILE_SIZE;
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
