/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include <qintdict.h>
#include <qmap.h>

class KSimpleConfig;
class SpeciesTheme;
class BosonEffect;
class BosonEffectProperties;
class Unit;
class BoAction;
class QString;
template<class T> class QIntDict;
template<class T1, class T2> class QMap;


/**
 * @short This class holds properties for @ref BosonWeapon
 *
 * Properties stored here include weapon's range, damage, what it can shoot at
 * etc.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonWeaponProperties : public PluginProperties
{
  public:
    /**
     * Constructs new BosonWeaponProperties. You must call @ref loadPlugin
     * before doing anything
     **/
    BosonWeaponProperties(const UnitProperties* prop, unsigned long int id);
    ~BosonWeaponProperties();

    /**
     * @return Whether this unit can shoot at aircrafts.
     **/
    inline bool canShootAtAirUnits() const  { return mCanShootAtAirUnits; }
    /**
     * @return Whether this unit can shoot at land units
     **/
    inline bool canShootAtLandUnits() const  { return mCanShootAtLandUnits; }
    /**
     * @return The weapon range of this unit. It's a number of cells, so multiply
     *  with BO_TILE_SIZE to use it on the canvas.
    **/
    inline unsigned long int range() const  { return mRange; }
    /**
     * @return The number of advance calls until the weapon is reloaded
     **/
    inline unsigned int reloadingTime() const  { return mReload; }
    /**
     * The damage this unit makes to other units. Negative values means
     * repairing
     **/
    inline long int damage() const  { return mDamage; }
    /**
     * @return Damage range of missile of this unit, e.g. range in what units will be damaged
     **/
    inline float damageRange() const  { return mDamageRange; }
    /**
     * @return Full damage range of missile of this unit, e.g. range in what
     *  units will be damaged by damage value (farther they'll be damaged less)
     **/
    inline float fullDamageRange() const  { return mFullDamageRange; }
    /**
     * @return Maximum speed that missile of this weapon can have or 0 if speed is infinite
     **/
    inline float speed() const  { return mSpeed; }
    /**
     * @return Acceleration speed of missile of this unit
     **/
    inline float accelerationSpeed() const  { return mAccelerationSpeed; }
    /**
     * @return Filename of 3d model of shot of this weapon.
     * Only used in unit editor
     **/
    inline QString modelFileName() const  { return mModelFileName; }
    /**
     * @return Name of unit. You can show it to user
     **/
    inline QString weaponName() const  { return mName; }
    /**
     * @return Offset of this weapon
     * Offset is relative to the center point of unit and is used when creating
     * new shot.
     **/
    inline BoVector3 offset() const  { return mOffset; }

    inline BosonShot::Type shotType() const  { return mShotType; }
    /**
     * @return Height of parable that shot of this weapon flies along
     * Note that this is height per cell, it should be multiplied by distance of
     * shot to get final height.
     **/
    inline float height() const  { return mHeight; }

    inline BosonModel* model() const  { return mModel; }

    /**
     * @return First part of the sound filename - e.g. "shoot" if the file
     * name should be "shoot_00.ogg". the _00 is added dynamically (randomly)
     * by @ref BosonSound
     **/
    QString sound(int soundEvent) const;

    void playSound(WeaponSoundEvent event) const;

    QMap<int, QString> sounds() const;

    unsigned long int id() const  { return mId; }

    /**
     * @return Whether this weapon can be used automatically by the unit
     * You can set it to false for more powerful weapons, which take a lot of
     *  time to reload, to make sure unit won't "waste" them for weak enemies.
     **/
    bool autoUse() const  { return mAutoUse; }

    /**
     * Creates new shot
     * @param attacker Unit that fired this shot
     * @param pos Position (center point) of attacker. Note that offset is added to this value
     * @param target Position of shot's target point
     * @return Created shot. Note that it's not added to canvas.
     * Note that pos or target may not be used depending on shot's type
     **/
    BosonShot* newShot(Unit* attacker, BoVector3 pos, BoVector3 target) const;

    QPtrList<BosonEffect> newShootEffects(BoVector3 pos, float rotation) const;
    QPtrList<BosonEffect> newFlyEffects(BoVector3 pos, float rotation) const;
    QPtrList<BosonEffect> newHitEffects(BoVector3 pos) const;

    QValueList<unsigned long int> shootEffectIds() const  { return mShootEffectIds; }
    QValueList<unsigned long int> flyEffectIds() const  { return mFlyEffectIds; }
    QValueList<unsigned long int> hitEffectIds() const  { return mHitEffectIds; }

    virtual QString name() const;
    virtual void loadPlugin(KSimpleConfig* config)  { loadPlugin(config, true); }
    virtual void loadPlugin(KSimpleConfig* config, bool full = true);
    virtual void savePlugin(KSimpleConfig* config);
    virtual int pluginType() const  { return Weapon; }

    QIntDict<BoAction>* actions()  { return &mActions; }

  protected:
    void setWeaponName(QString str)  { mName = str; }
    void setDamage(long int damage)  { mDamage = damage; }
    void setDamageRange(float range)  { mDamageRange = range; }
    void setFullDamageRange(float range)  { mFullDamageRange = range; }
    void setReloadingTime(unsigned int reload)  { mReload = reload; }
    void setRange(unsigned long int range)  { mRange = range; }
    void setCanShootAtAirUnits(bool can)  { mCanShootAtAirUnits = can; }
    void setCanShootAtLandUnits(bool can)  { mCanShootAtLandUnits = can; }
    void setSpeed(float speed)  { mSpeed = speed; }
    void setAccelerationSpeed(float speed)  { mAccelerationSpeed = speed; }
    void setModelFileName(QString file)  { mModelFileName = file; }
    void setShootEffectIds(QValueList<unsigned long int> ids)  { mShootEffectIds = ids; }
    void setFlyEffectIds(QValueList<unsigned long int> ids)  { mFlyEffectIds = ids; }
    void setHitEffectIds(QValueList<unsigned long int> ids)  { mHitEffectIds = ids; }
    void setOffset(BoVector3 o)  { mOffset = o; }
    void setHeight(float height)  { mHeight = height; }
    void setSound(int event, QString filename);
    void setAutoUse(bool use)  { mAutoUse = use; }

    void reset();
    void loadAction(UnitAction type, KSimpleConfig* cfg, const QString& key, bool useDefault = false);

    friend class BoUnitEditor;
    friend class UpgradeProperties;

  private:
    unsigned long int mId;
    unsigned long int mRange;
    long int mDamage;
    float mDamageRange;
    float mFullDamageRange;
    bool mCanShootAtAirUnits;
    bool mCanShootAtLandUnits;
    unsigned int mReload;
    float mSpeed;
    float mAccelerationSpeed;
    float mHeight;
    BosonShot::Type mShotType;
    BosonModel* mModel;
    QString mModelFileName;
    QString mName;
    QPtrList<BosonEffectProperties> mShootEffects;
    QPtrList<BosonEffectProperties> mFlyEffects;
    QPtrList<BosonEffectProperties> mHitEffects;
    // FIXME: these are only needed in editor mode. In normal mode, they only waste memory
    QValueList<unsigned long int> mShootEffectIds;
    QValueList<unsigned long int> mFlyEffectIds;
    QValueList<unsigned long int> mHitEffectIds;
    BoVector3 mOffset;
    QMap<int, QString> mSounds;
    QIntDict<BoAction> mActions;
    bool mAutoUse;
};


/**
 * @short Represents a weapon of an unit
 *
 * This class represents one weapon of unit. It's specific to unit, so you have
 * to create one BosonWeapon object for every weapon in every unit.
 *
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
    virtual void advance(unsigned int advanceCallsCount)
    {
      Q_UNUSED(advanceCallsCount);
      reload();
    }

    virtual bool saveAsXML(QDomElement& root) const;
    virtual bool loadFromXML(const QDomElement& root);

    /**
     * @return Whether this weapon can shoot at unit u
     **/
    bool canShootAt(Unit* u) const;
    /**
     * @return Whether this weapon is reloaded (ready to fire)
     **/
    inline bool reloaded() const  { return (mReloadCounter == 0); }

    inline const BosonWeaponProperties* properties() const  { return mProp; }

    void shoot(Unit* u);
    void shoot(const BoVector3& target);

    /**
     * Lay mine at current location of unit
     * If mine is laid, returns true. If weapon is not a minelayer, not reloaded
     * or some other error occurs, returns false
     **/
    bool layMine();

    /**
     * Drop bomb from the current location of unit
     * If bomb is dropped, returns true. If weapon is not a bomb, not reloaded
     * or some other error occurs, returns false
     **/
    bool dropBomb();

    virtual void itemRemoved(BosonItem*) {}

  protected:
    inline void reload()  { if(mReloadCounter > 0) { mReloadCounter = mReloadCounter - 1; } }

    void shoot(const BoVector3& pos, const BoVector3& target);

    void registerWeaponData(int weaponNumber, KGamePropertyBase* prop, int id, bool local = true);

  private:
    const BosonWeaponProperties* mProp;
    KGameProperty<int> mReloadCounter;
};

#endif // BOSONWEAPON_H

/*
 * vim: et sw=2
 */
