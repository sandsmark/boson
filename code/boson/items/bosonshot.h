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
class SpeciesTheme;
class KSimpleConfig;
class BosonParticleSystemProperties;
class BosonParticleSystem;
class BosonModel;

class BosonShot;

/**
 * @short Property class for shots
 * It contains properties for specific shot type
 * Properties include damage, speed and damaging range of shot
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonShotProperties
{
  public:
    /**
     * Constructs new BosonShotProperties and loads properties from KConfig.
     * Group for KConfig must be set before
     * @param theme speciestheme for this shot
     * @ cfg KSimpleConfig object which will be used to load values
     **/
    BosonShotProperties(SpeciesTheme* theme, KSimpleConfig* cfg);
    ~BosonShotProperties();

    /**
     * @return Damage range of missile of this unit, e.g. range in what units will be damaged
     **/
    float damageRange() const { return mDamageRange; };
    /**
     * The damage this unit makes to other units. Negative values means
     * repairing
    **/
    long int damage() const { return mDamage; };
    /**
     * @return Speed of missile of this unit (per second) or 0 if speed is infinite
     **/
    unsigned long int speed() const  { return mSpeed; };

    inline SpeciesTheme* theme() const  { return mTheme; };

    inline long unsigned int id() const  { return mId; };
    
    inline BosonModel* model()  { return mModel; };

    BosonShot* newShot(Unit* attacker, float x, float y, float z, float tx, float ty, float tz);

    QPtrList<BosonParticleSystem> newFlyParticleSystems(float x, float y, float z) const;
        //{ if(mFlyParticleSystem) return mFlyParticleSystem->newSystem(x, y, z); };
    QPtrList<BosonParticleSystem> newHitParticleSystems(float x, float y, float z) const;
        //{ if(mHitParticleSystem) return mHitParticleSystem->newSystem(x, y, z); };

  private:
    float mDamageRange;
    long int mDamage;
    unsigned long int mSpeed;
    unsigned long int mId;
    SpeciesTheme* mTheme;
    QPtrList<BosonParticleSystemProperties> mFlyParticleSystems;
    QPtrList<BosonParticleSystemProperties> mHitParticleSystems;
    BosonModel* mModel;
};


/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonShot : public BosonItem
{
  public:
    BosonShot(BosonShotProperties* prop, Unit* attacker, float x, float y, float z, float tx, float ty, float tz);

    virtual void advance(unsigned int phase);

//    inline BoVector3 pos()  { return mPos; }

    inline Player* owner()  { return mOwner; };
    inline const BosonShotProperties* properties() const  { return mProp; };
    
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
    const BosonShotProperties* mProp;
    QPtrList<BosonParticleSystem> mFlyParticleSystems;
};

#endif // BOSONSHOT_H
