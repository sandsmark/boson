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

#ifndef BOSONSHOT_H
#define BOSONSHOT_H

#include "../bo3dtools.h"
#include "../rtti.h"
#include "bosonitem.h"
#include <qptrlist.h>

class Unit;
class Player;
class BosonParticleSystem;
class BosonWeaponProperties;


/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonShot : public BosonItem
{
  public:
    BosonShot(const BosonWeaponProperties* prop, Unit* attacker, float x, float y, float z, float tx, float ty, float tz);

    virtual void advance(unsigned int phase);

//    inline BoVector3 pos()  { return mPos; }

    inline Player* owner()  { return mOwner; };
    inline const BosonWeaponProperties* properties() const  { return mProp; };
    
    inline QPtrList<BosonParticleSystem>* flyParticleSystems()  { return &mFlyParticleSystems; };

    inline bool isActive() const  { return mActive; };

    inline virtual int rtti() const  { return RTTI::Shot; }
    
    float rotationToPoint(float x, float y);

  protected:
    BoVector3 mVelo;
    unsigned int mStep;
    unsigned int mTotalSteps;
    bool mActive;
    float mLength;
    float mZ;
    Player* mOwner;
    const BosonWeaponProperties* mProp;
    QPtrList<BosonParticleSystem> mFlyParticleSystems;
};

#endif // BOSONSHOT_H
