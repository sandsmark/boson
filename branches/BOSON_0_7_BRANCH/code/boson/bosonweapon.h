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

#include "items/bosonshot.h"
#include "pluginproperties.h"
#include "unitplugins.h"
#include "bo3dtools.h"

#include <qptrlist.h>

class KSimpleConfig;
class SpeciesTheme;
class BosonParticleSystem;
class BosonParticleSystemProperties;
class Unit;
class QString;


/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonWeaponProperties : public PluginProperties
{
  public:
    BosonWeaponProperties(const UnitProperties* prop);
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
    long int damage() const  { return mDamage; };
    /**
     * @return Damage range of missile of this unit, e.g. range in what units will be damaged
     **/
    float damageRange() const  { return mDamageRange; };
    /**
     * @return Speed of missile of this unit (per second) or 0 if speed is infinite
     **/
    unsigned long int speed() const  { return mSpeed; };
    QString modelFileName() const  { return mModelFileName; };
    QString weaponName() const  { return mName; };
    BoVector3 offset() const  { return mOffset;};
    float maxHeight() const  { return mMaxHeight; };

    inline SpeciesTheme* theme() const  { return mTheme; };
    inline BosonModel* model() const  { return mModel; };

    BosonShot* newShot(Unit* attacker, BoVector3 pos, BoVector3 target) const;

    QPtrList<BosonParticleSystem> newShootParticleSystems(BoVector3 pos, float rotation) const;
    QPtrList<BosonParticleSystem> newFlyParticleSystems(BoVector3 pos) const;
    QPtrList<BosonParticleSystem> newHitParticleSystems(BoVector3 pos) const;

    QValueList<unsigned long int> shootParticleSystemIds() const  { return mShootParticleSystemIds; };
    QValueList<unsigned long int> flyParticleSystemIds() const  { return mFlyParticleSystemIds; };
    QValueList<unsigned long int> hitParticleSystemIds() const  { return mHitParticleSystemIds; };

    virtual QString name() const;
    virtual void loadPlugin(KSimpleConfig* config)  { loadPlugin(config, true); };
    virtual void loadPlugin(KSimpleConfig* config, bool full = true);
    virtual void savePlugin(KSimpleConfig* config);
    virtual int pluginType() const  { return Weapon; };
    
  protected:
    void setWeaponName(QString str)  { mName = str; };
    void setDamage(long int damage)  { mDamage = damage; };
    void setDamageRange(float range)  { mDamageRange = range; };
    void setReloadingTime(unsigned int reload)  { mReload = reload; };
    void setRange(unsigned long int range)  { mRange = range; };
    void setCanShootAtAirUnits(bool can)  { mCanShootAtAirUnits = can; };
    void setCanShootAtLandUnits(bool can)  { mCanShootAtLandUnits = can; };
    void setSpeed(unsigned long int speed)  { mSpeed = speed; };
    void setModelFileName(QString file)  { mModelFileName = file; };
    void setShootParticleSystemIds(QValueList<unsigned long int> ids)  { mShootParticleSystemIds = ids; };
    void setFlyParticleSystemIds(QValueList<unsigned long int> ids)  { mFlyParticleSystemIds = ids; };
    void setHitParticleSystemIds(QValueList<unsigned long int> ids)  { mHitParticleSystemIds = ids; };
    void setOffset(BoVector3 o)  { mOffset = o; };
    void setMaxHeight(float maxheight)  { mMaxHeight = maxheight; };

    void reset();

    friend class BoUnitEditor;

  private:
    unsigned long int mRange;
    long int mDamage;
    float mDamageRange;
    bool mCanShootAtAirUnits;
    bool mCanShootAtLandUnits;
    unsigned int mReload;
    SpeciesTheme* mTheme;
    unsigned long int mSpeed;
    float mMaxHeight;
    BosonModel* mModel;
    QString mModelFileName;
    QString mName;
    QPtrList<BosonParticleSystemProperties> mShootParticleSystems;
    QPtrList<BosonParticleSystemProperties> mFlyParticleSystems;
    QPtrList<BosonParticleSystemProperties> mHitParticleSystems;
    // FIXME: these are only needed in editor mode. In normal mode, they only waste memory
    QValueList<unsigned long int> mShootParticleSystemIds;
    QValueList<unsigned long int> mFlyParticleSystemIds;
    QValueList<unsigned long int> mHitParticleSystemIds;
    BoVector3 mOffset;
};


/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonWeapon : public UnitPlugin
{
  public:
    enum PropertyIds {
      IdReloadCounter = KGamePropertyBase::IdUser + 0,

      LastPropertyId
    };

    /**
     * @param weaponNumber The number of the weapon of the unit.
     **/
    BosonWeapon(int weaponNumber, BosonWeaponProperties* prop, Unit* unit);
    ~BosonWeapon();

    /**
     * Implemented for internal resons only. Not used.
     * @return The plugin type for @ref UnitPlugin. Note that this is not used
     * and note that this value is <em>not</em> unique for the weapon! All
     * weapons have the same pluginType!
     **/
    virtual int pluginType() const { return UnitPlugin::Weapon; }

    /**
     * Reload the weapon.
     **/
    virtual void advance(unsigned int advanceCount)
    {
      Q_UNUSED(advanceCount);
      reload();
    }

    bool canShootAt(Unit* u) const;
    inline bool reloaded() const  { return (mReloadCounter == 0); };

    inline const BosonWeaponProperties* properties() const  { return mProp; };

    void shoot(Unit* u);
    void shoot(BoVector3 target);

  protected:
    inline void reload()  { if(mReloadCounter > 0) { mReloadCounter = mReloadCounter - 1; } }

    void registerWeaponData(int weaponNumber, KGamePropertyBase* prop, int id, bool local = true);

  private:
    const BosonWeaponProperties* mProp;
    KGameProperty<int> mReloadCounter;
};

#endif // BOSONWEAPON_H

/*
 * vim: et sw=2
 */
