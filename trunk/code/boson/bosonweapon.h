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
    inline unsigned long int range()  { return mRange; };
    /**
     * @return Whether this unit can shoot at aircrafts.
     **/
    inline bool canShootAtAirUnits()  { return mCanShootAtAirUnits; };
    /**
     * @return Whether this unit can shoot at land units
     **/
    inline bool canShootAtLandUnits()  { return mCanShootAtLandUnits; };
    /**
     * @return The number of advance calls until the weapon is reloaded
     **/
    inline unsigned int reloadingTime()  { return mReload; };

    inline unsigned long int id()  { return mId; };

    BosonShot* newShot(Unit* attacker, float x, float y, float z, float tx, float ty, float tz)
        { return mShotProp->newShot(attacker, x, y, z, tx, ty, tz); };
    QPtrList<BosonParticleSystem> newShootParticleSystems(float x, float y, float z);
        //{ if(mShootParticleSystem) return mShootParticleSystem->newSystem(x, y, z); };

  private:
    unsigned long int mRange;
    bool mCanShootAtAirUnits;
    bool mCanShootAtLandUnits;
    unsigned int mReload;
    SpeciesTheme* mTheme;  // NOTE: this may change!
    unsigned long int mId;
    BosonShotProperties* mShotProp;
    QPtrList<BosonParticleSystemProperties> mShootParticleSystems;
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
    inline bool reloaded()  { return (mReloadCounter == 0); };
    
    inline BosonWeaponProperties* properties()  { return mProp; };

    void shoot(Unit* u);
    void shoot(float x, float y, float z);

  private:
    BosonWeaponProperties* mProp;
    Unit* mUnit;
    int mReloadCounter;
};

#endif // BOSONWEAPON_H
