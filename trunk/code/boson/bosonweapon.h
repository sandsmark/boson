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

#ifndef BOSONWEAPON_H
#define BOSONWEAPON_H

#include <qptrlist.h>
#include "items/bosonshot.h"

class KSimpleConfig;
class SpeciesTheme;
class BosonParticleSystem;
class BosonParticleSystemProperties;
class Unit;


/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonWeaponProperties
{
  public:
    BosonWeaponProperties(KSimpleConfig* cfg, SpeciesTheme* theme);
    ~BosonWeaponProperties();

    /**
     * @return The weapon range of this unit. It's a number of cells, so multiply
     *  with BO_TILE_SIZE to use it on the canvas.
    **/
    inline unsigned long int range() const  { return mRange; };
    /**
     * @return Whether this unit can shoot at aircrafts.
     **/
    inline bool canShootAtAirUnits() const  { return mCanShootAtAirUnits; };
    /**
     * @return Whether this unit can shoot at land units
     **/
    inline bool canShootAtLandUnits() const  { return mCanShootAtLandUnits; };
    /**
     * @return The number of advance calls until the weapon is reloaded
     **/
    inline unsigned int reloadingTime() const  { return mReload; };
    /**
     * The damage this unit makes to other units. Negative values means
     * repairing
    **/
    long int damage() const { return mDamage; };
    /**
     * @return Damage range of missile of this unit, e.g. range in what units will be damaged
     **/
    float damageRange() const { return mDamageRange; };
    /**
     * @return Speed of missile of this unit (per second) or 0 if speed is infinite
     **/
    unsigned long int speed() const  { return mSpeed; };

    inline SpeciesTheme* theme() const  { return mTheme; };
    inline BosonModel* model() const  { return mModel; };

    BosonShot* newShot(Unit* attacker, float x, float y, float z, float tx, float ty, float tz) const;

    QPtrList<BosonParticleSystem> newShootParticleSystems(float x, float y, float z) const;
    QPtrList<BosonParticleSystem> newFlyParticleSystems(float x, float y, float z) const;
    QPtrList<BosonParticleSystem> newHitParticleSystems(float x, float y, float z) const;

  private:
    unsigned long int mRange;
    long int mDamage;
    float mDamageRange;
    bool mCanShootAtAirUnits;
    bool mCanShootAtLandUnits;
    unsigned int mReload;
    SpeciesTheme* mTheme;
    unsigned long int mSpeed;
    BosonModel* mModel;
    QPtrList<BosonParticleSystemProperties> mShootParticleSystems;
    QPtrList<BosonParticleSystemProperties> mFlyParticleSystems;
    QPtrList<BosonParticleSystemProperties> mHitParticleSystems;


};


/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonWeapon
{
  public:
    BosonWeapon(BosonWeaponProperties* prop, Unit* unit);
    ~BosonWeapon();

    inline void reload()  { if(mReloadCounter > 0) mReloadCounter--; };

    bool canShootAt(Unit* u);
    inline bool reloaded() const  { return (mReloadCounter == 0); };

    inline const BosonWeaponProperties* properties() const  { return mProp; };

    void shoot(Unit* u);
    void shoot(float x, float y, float z);

  private:
    const BosonWeaponProperties* mProp;
    Unit* mUnit;
    int mReloadCounter;
};

#endif // BOSONWEAPON_H
