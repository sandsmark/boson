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

#include "../global.h"
#include "../bosoncanvas.h"
#include "../bosonparticlesystem.h"
#include "../bosonweapon.h"
#include "bodebug.h"

#include <ksimpleconfig.h>

#include <qptrlist.h>

#include <GL/gl.h>

#include <math.h>


BosonShot::BosonShot(const BosonWeaponProperties* prop, Player* owner, BosonCanvas* canvas, BoVector3 pos, BoVector3 target) :
    BosonItem(prop ? prop->model() : 0, canvas)
{
  boDebug(350) << "MISSILE: " << k_funcinfo << "Creating new shot" << endl;
  mOwner = owner;
  mProp = prop;
  setSize(BO_TILE_SIZE / 2, BO_TILE_SIZE / 2); // AB: pretty much a random value
  if (!mProp)
  {
    boError(350) << k_funcinfo << "NULL weapon properties!" << endl;
    return;
  }
  if (!mOwner)
  {
    boError(350) << k_funcinfo << "NULL owner!" << endl;
    return;
  }
  if (!canvas)
  {
    boError(350) << k_funcinfo << "NULL canvas" << endl;
    return;
  }
  if(prop->speed() == 0)
  {
    // This shot is bullet, not missile - it has infinite speed and it reaches
    //  it's target immideately. No need to calculate anything.
    boDebug(350) << "MISSILE: " << k_funcinfo << "    Attacker's shot is bullet (infinite speed). Returning" << endl;
    move(target[0], target[1], target[2]);
    mActive = false;
    return;
  }
  // First set the velocity to length of whole trip (for calculations)
  mVelo = target - pos;
  mLength = mVelo.length();
  //boDebug(350) << "MISSILE: " << k_funcinfo << "    Length of trip: " << length << endl;
  // Calculate number of steps
  mTotalSteps = (int)ceilf(mLength / prop->speed()) - 1;
  // Current step
  mStep = 0;
  //boDebug(350) << "MISSILE: " << k_funcinfo << "    Steps: " << mSteps << endl;
  // Set velocity
  mVelo.scale(prop->speed() / mLength);
  //boDebug(350) << "MISSILE: " << k_funcinfo << "    Normalized & scaled (final) velocity: (" << mVelo[0] << "; " << mVelo[1] << "; " << mVelo[2] << ")" << endl;
  // Initialization
  mActive = true;
  move(pos[0], pos[1], pos[2]);
  setAnimated(true);
  setRotation(rotationToPoint(mVelo[0], mVelo[1]));
  mZ = 0; // For parable calculations only, must be 0 at the beginning
  // Particle systems
  mFlyParticleSystems = prop->newFlyParticleSystems(pos, -rotation());
  canvas->addParticleSystems(mFlyParticleSystems);
  mParticleVelo = sqrt(mVelo[0] * mVelo[0] + mVelo[1] * mVelo[1]) / (float)BO_TILE_SIZE;
}

// move the shot by one step
// (actually only set the velocity - it is moved by BosonCanvas::slotAdvance())
void BosonShot::advanceMoveInternal()
{
  mStep++;
  // Calculate parable height at current step
  float factor = mStep / (float)mTotalSteps - 0.5;  // Factor will be in range -0.5 to 0.5
  factor = -4 * (factor * factor) + 1;  // Factor is now  0 ... 1 ... 0  depending of current step
  // How much will be added to current z position
  float addZ = (mProp->maxHeight() * factor) * BO_TILE_SIZE;
  setVelocity(mVelo[0], mVelo[1], mVelo[2] + (addZ - mZ));
  setXRotation(rotationToPoint(mLength / mTotalSteps, addZ - mZ + mVelo[2]) - 90 );
  //AB: maybe reimplement moveBy() for this?
  // Move all "fly" particles.
  BoVector3 move(mVelo[0], -(mVelo[1]), mVelo[2] + (addZ - mZ));
  move.scale(1 / (float)BO_TILE_SIZE);
  mZ = addZ;
  QPtrListIterator<BosonParticleSystem> it(mFlyParticleSystems);
  while(it.current())
  {
    if(it.current() == 0)
    {
      mFlyParticleSystems.remove(it);
      break;
    }
    BoVector3 newpos(it.current()->position());
    newpos.add(move);
    it.current()->setPosition(newpos);
    ++it;
  }
  if(mStep >= mTotalSteps)
  {
    mActive = false;
  }
}

//AB: note this this is primary optimized for SAFETY, NOT for performance. it is
//optimized for performance in the second place!
void BosonShot::advanceMoveCheck()
{
  float velocityX = xVelocity();
  float velocityY = yVelocity();
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

/*
 * vim: et sw=2
 */
