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

#include "bosonmissile.h"

#include "unit.h"
#include "unitproperties.h"
#include "speciestheme.h"
#include "bosonmodel.h"
#include "player.h"

#include <kdebug.h>

#include <GL/gl.h>

#include <math.h>

BosonMissile::BosonMissile(Unit* attacker, Unit* target)
{
  init(attacker, (target->x() + target->width() / 2) * BO_GL_CELL_SIZE / BO_TILE_SIZE,
      -((target->y() + target->height() / 2) * BO_GL_CELL_SIZE / BO_TILE_SIZE),
      target->z() * BO_GL_CELL_SIZE / BO_TILE_SIZE);
}

BosonMissile::BosonMissile(Unit* attacker, float x, float y, float z)
{
  init(attacker, x, y, z);
}

void BosonMissile::init(Unit* attacker, float x, float y, float z)
{
  kdDebug() << "MISSILE: " << k_funcinfo << "Creating new missile" << endl;
  mDamageRange = attacker->unitProperties()->missileDamageRange();
  mDamage = attacker->unitProperties()->weaponDamage();
  mOwner = attacker->owner();
  if(attacker->unitProperties()->missileSpeed() == 0)
  {
    kdDebug() << "MISSILE: " << k_funcinfo << "    Attacker's missile is bullet (infinite speed). Returning" << endl;
    mPos.set(x, y, z);
    //kdDebug() << "MISSILE: " << k_funcinfo << "    Missile's final pos: (" << mPos[0] << "; " << mPos[1] << "; " << mPos[2] << ")" << endl;
    mActive = false;
    return;
  }
  //kdDebug() << "MISSILE: " << k_funcinfo << "    Attacker's pos: (" << attacker->x() / BO_TILE_SIZE << "; " << -(attacker->y() / BO_TILE_SIZE) << "; " << attacker->z() / BO_TILE_SIZE << ")" <<
      //";; target's pos: (" << x << "; " << y << "; " << z << ")" << endl;
  mPos.set((attacker->x() + attacker->width() / 2) * BO_GL_CELL_SIZE / BO_TILE_SIZE,
      -((attacker->y() + attacker->height() / 2) * BO_GL_CELL_SIZE / BO_TILE_SIZE),
      attacker->z() * BO_GL_CELL_SIZE / BO_TILE_SIZE);
  //kdDebug() << "MISSILE: " << k_funcinfo << "    Missile's initial pos: (" << mPos[0] << "; " << mPos[1] << "; " << mPos[2] << ")" << endl;
  mVelo.set(x - mPos[0], y - mPos[1], z - mPos[2]);
  float length = mVelo.length();
  //kdDebug() << "MISSILE: " << k_funcinfo << "    Length of trip: " << length << endl;
  float a = length / (attacker->unitProperties()->missileSpeed() / (float)BO_TILE_SIZE);
  //kdDebug() << "MISSILE: " << k_funcinfo << "    " << length << " / ( " << attacker->unitProperties()->missileSpeed() << " / " << BO_TILE_SIZE << ") = " << a << endl;
  mSteps = (int)ceilf(a);
  //kdDebug() << "MISSILE: " << k_funcinfo << "    (int)ceilf(" << a << ") = " << mSteps << endl;
  //kdDebug() << "MISSILE: " << k_funcinfo << "    Steps: " << mSteps << endl;
  mVelo.scale((attacker->unitProperties()->missileSpeed() / (float)BO_TILE_SIZE) / length);
  //kdDebug() << "MISSILE: " << k_funcinfo << "    Normalized & scaled (final) velocity: (" << mVelo[0] << "; " << mVelo[1] << "; " << mVelo[2] << ")" << endl;
  mActive = true;
  //kdDebug() << "MISSILE: " << k_funcinfo << "    All done. Returning." << endl;
}

void BosonMissile::update()
{
  //kdDebug() << "MISSILE: " << k_funcinfo << "Updating missile" << endl;
  mPos.add(mVelo);
  mSteps--;
  //kdDebug() << "MISSILE: " << k_funcinfo << "    Steps is now " << mSteps << endl;
  if(mSteps <= 0)
  {
    //kdDebug() << "MISSILE: " << k_funcinfo << "    Missile is now unactive!" << endl;
    mActive = false;
  }
}

void BosonMissile::draw()
{
  //kdDebug() << "MISSILE: " << k_funcinfo << "  Drawing missile" << endl;
  glPushMatrix();
  glTranslatef(mPos[0], mPos[1], mPos[2]);
  //kdDebug() << "MISSILE: " << k_funcinfo << "  Frames of model: " << mAttacker->speciesTheme()->objectModel(ObjectMissile)->frames() << endl;
  //kdDebug() << "MISSILE: " << k_funcinfo << "  Calling display list" << endl;
  glCallList(mOwner->speciesTheme()->objectModel(ObjectMissile)->frame(0)->displayList()); // Long call ;-)
  //kdDebug() << "MISSILE: " << k_funcinfo << "  DL called" << endl;
  glPopMatrix();
  //kdDebug() << "MISSILE: " << k_funcinfo << "  Drawing missile DONE" << endl;
}
