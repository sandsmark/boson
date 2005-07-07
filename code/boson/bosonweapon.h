/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 Rivo Laks (rivolaks@hot.ee)

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
#include "boupgradeableproperty.h"

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
     * @return The weapon range of this unit. It's a number of cells
     **/
    inline unsigned long int range() const  { return mRange.value(upgradesCollection()); }
    inline unsigned long int baseRange() const  { return mBaseRange; }
    /**
     * @return Maximum flying distance of the missile of this weapon.
     * If the missile hasn't caught the target after flying this distance, the
     *  missile runs out of fuel and self-detonates.
     * This has effect only for missile weapons.
     **/
    inline bofixed maxFlyDistance() const  { return mMaxFlyDistance; }
    /**
     * @return Starting angle (in degrees) of the missile of this weapon.
     * This is the vertical angle that the missile will get when it's launched.
     *  If it's 0, missile will start to fly horizontally towards the target,
     *  if it's 90, missile will first fly upward and then turn to it's target.
     *  If it's -1, then missile will start flying directly towards it's
     *  target.
     * Must be in range 0-90 or -1.
     * This has effect only for missile weapons.
     **/
    inline bofixed startAngle() const  { return mStartAngle; }
    /**
     * @return The number of advance calls until the weapon is reloaded
     **/
    inline unsigned int reloadingTime() const  { return mReloadingTime.value(upgradesCollection()); }
    inline unsigned int baseReloadingTime() const  { return mBaseReloadingTime; }
    /**
     * The damage this unit makes to other units. Negative values means
     * repairing
     **/
    inline long int damage() const  { return mDamage.value(upgradesCollection()); }
    inline long int baseDamage() const  { return mBaseDamage; }
    /**
     * @return Damage range of missile of this unit, e.g. range in what units will be damaged
     **/
    inline bofixed damageRange() const  { return mDamageRange.value(upgradesCollection()); }
    inline bofixed baseDamageRange() const  { return mBaseDamageRange; }
    /**
     * @return Full damage range of missile of this unit, e.g. range in what
     *  units will be damaged by damage value (farther they'll be damaged less)
     **/
    inline bofixed fullDamageRange() const  { return mFullDamageRange.value(upgradesCollection()); }
    inline bofixed baseFullDamageRange() const  { return mBaseFullDamageRange; }
    /**
     * @return Maximum speed that missile of this weapon can have or 0 if speed is infinite
     **/
    inline bofixed speed() const  { return mSpeed.value(upgradesCollection()); }
    inline bofixed baseSpeed() const  { return mBaseSpeed; }
    /**
     * @return Acceleration speed of missile of this unit
     **/
    inline bofixed accelerationSpeed() const  { return mAccelerationSpeed; }
    /**
     * @return tangens of turning speed of the missile of this weapon.
     * Turning speed specifies how fast the missile can turn.
     * It has effect only for missile weapons.
     **/
    bofixed turningSpeed() const  { return mTurningSpeed; }
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
    inline BoVector3Fixed offset() const  { return mOffset; }

    inline BosonShot::Type shotType() const  { return mShotType; }
    /**
     * @return Height of parable that shot of this weapon flies along
     * Note that this is height per cell, it should be multiplied by distance of
     * shot to get final height.
     **/
    inline bofixed height() const  { return mHeight; }

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

    bool takeTargetVeloIntoAccount() const  { return mTakeTargetVeloIntoAccount; }

    /**
     * Creates new shot
     * @param attacker Unit that fired this shot
     * @param pos Position (center point) of attacker. Note that offset is added to this value
     * @param target Position of shot's target point
     * @return Created shot. Note that it's not added to canvas.
     * Note that pos or target may not be used depending on shot's type
     **/
    BosonShot* newShot(Unit* attacker, BoVector3Fixed pos, BoVector3Fixed target) const;
    BosonShot* newShot(Unit* attacker, BoVector3Fixed pos, Unit* target) const;

    QPtrList<BosonEffect> newShootEffects(BoVector3Fixed pos, bofixed rotation) const;
    QPtrList<BosonEffect> newFlyEffects(BoVector3Fixed pos, bofixed rotation) const;
    QPtrList<BosonEffect> newHitEffects(BoVector3Fixed pos) const;

    QValueList<unsigned long int> shootEffectIds() const  { return mShootEffectIds; }
    QValueList<unsigned long int> flyEffectIds() const  { return mFlyEffectIds; }
    QValueList<unsigned long int> hitEffectIds() const  { return mHitEffectIds; }

    virtual QString name() const;
    virtual void loadPlugin(KSimpleConfig* config)  { loadPlugin(config, true); }
    virtual void loadPlugin(KSimpleConfig* config, bool full = true);
    virtual void savePlugin(KSimpleConfig* config);
    virtual int pluginType() const  { return Weapon; }

    QIntDict<BoAction>* actions()  { return &mActions; }


    bool getBaseValue(unsigned long int* ret, const QString& name, const QString& type) const;
    bool getBaseValue(long int* ret, const QString& name, const QString& type) const;
    bool getBaseValue(bofixed* ret, const QString& name, const QString& type) const;

    const UpgradesCollection& upgradesCollection() const
    {
      return mUpgradesCollection;
    }
    void clearUpgrades();
    void addUpgrade(const UpgradeProperties* prop);
    void removeUpgrade(const UpgradeProperties* prop);
    void removeUpgrade(unsigned long int id);

    bool saveUpgradesAsXML(QDomElement& root) const;
    bool loadUpgradesFromXML(const SpeciesTheme* theme, const QDomElement& root);

  protected:
    void setWeaponName(QString str)  { mName = str; }
    void setCanShootAtAirUnits(bool can)  { mCanShootAtAirUnits = can; }
    void setCanShootAtLandUnits(bool can)  { mCanShootAtLandUnits = can; }
    void setAccelerationSpeed(bofixed speed)  { mAccelerationSpeed = speed; }
    void setModelFileName(QString file)  { mModelFileName = file; }
    void setShootEffectIds(QValueList<unsigned long int> ids)  { mShootEffectIds = ids; }
    void setFlyEffectIds(QValueList<unsigned long int> ids)  { mFlyEffectIds = ids; }
    void setHitEffectIds(QValueList<unsigned long int> ids)  { mHitEffectIds = ids; }
    void setOffset(BoVector3Fixed o)  { mOffset = o; }
    void setHeight(bofixed height)  { mHeight = height; }
    void setSound(int event, QString filename);
    void setAutoUse(bool use)  { mAutoUse = use; }
    void setTakeTargetVeloIntoAccount(bool take)  { mTakeTargetVeloIntoAccount = take; }
    void setMaxFlyDistance(bofixed dist)  { mMaxFlyDistance = dist; }
    void setTurningSpeed(bofixed s)  { mTurningSpeed = s; }
    void setStartAngle(bofixed a)  { mStartAngle = a; }

    void reset();
    void loadAction(UnitAction type, KSimpleConfig* cfg, const QString& key, bool useDefault = false);

    friend class BoUnitEditor;

  private:

#define DECLAREUPGRADEABLE(type, name) \
                void setBase##name(type v) { mBase##name = v; } \
                BoUpgradeableProperty<type> m##name; \
                type mBase##name;


    unsigned long int mId;
    DECLAREUPGRADEABLE(unsigned long int, Range);
    DECLAREUPGRADEABLE(long int, Damage);
    DECLAREUPGRADEABLE(bofixed, DamageRange);
    DECLAREUPGRADEABLE(bofixed, FullDamageRange);
    DECLAREUPGRADEABLE(unsigned long int, ReloadingTime);
    DECLAREUPGRADEABLE(bofixed, Speed);
    bool mCanShootAtAirUnits;
    bool mCanShootAtLandUnits;
    bofixed mAccelerationSpeed;
    bofixed mHeight;
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
    BoVector3Fixed mOffset;
    QMap<int, QString> mSounds;
    QIntDict<BoAction> mActions;
    bool mAutoUse;
    bool mTakeTargetVeloIntoAccount;
    bofixed mMaxFlyDistance;
    bofixed mTurningSpeed;
    bofixed mStartAngle;


#undef DECLAREUPGRADEABLE

    UpgradesCollection mUpgradesCollection;
    QMap<QString, unsigned long int*> mULongProperties;
    QMap<QString, long int*> mLongProperties;
    QMap<QString, bofixed*> mBoFixedProperties;
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
    }

    inline void reload(unsigned int reloadBy)
    {
      if(mReloadCounter > (int)reloadBy)
      {
        mReloadCounter = mReloadCounter - reloadBy;
      }
      else
      {
        mReloadCounter = 0;
      }
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
    void shoot(const BoVector3Fixed& target);

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

    virtual void unitDestroyed(Unit*) {}
    virtual void itemRemoved(BosonItem*) {}

  protected:
    void shoot(const BoVector3Fixed& pos, const BoVector3Fixed& target);

    void registerWeaponData(int weaponNumber, KGamePropertyBase* prop, int id, bool local = true);

  private:
    const BosonWeaponProperties* mProp;
    KGameProperty<int> mReloadCounter;
};

#endif // BOSONWEAPON_H

/*
 * vim: et sw=2
 */
