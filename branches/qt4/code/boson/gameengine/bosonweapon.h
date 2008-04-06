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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef BOSONWEAPON_H
#define BOSONWEAPON_H

#include "bosonshot.h"
#include "pluginproperties.h"
#include "unitplugins/unitplugin.h"
#include "../bo3dtools.h"

#include <q3ptrlist.h>
#include <q3intdict.h>
#include <qmap.h>
//Added by qt3to4:
#include <Q3ValueList>

class KSimpleConfig;
class SpeciesTheme;
class Unit;
class QString;
template<class T> class Q3IntDict;
template<class T1, class T2> class QMap;

class BosonWeaponTurretProperties;
class BosonWeaponTurret;

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
    /**
     * The damage this unit makes to other units. Negative values means
     * repairing
     **/
    inline long int damage() const  { return mDamage.value(upgradesCollection()); }
    /**
     * @return Damage range of missile of this unit, e.g. range in what units will be damaged
     **/
    inline bofixed damageRange() const  { return mDamageRange.value(upgradesCollection()); }
    /**
     * @return Full damage range of missile of this unit, e.g. range in what
     *  units will be damaged by damage value (farther they'll be damaged less)
     **/
    inline bofixed fullDamageRange() const  { return mFullDamageRange.value(upgradesCollection()); }
    /**
     * @return Maximum speed that missile of this weapon can have or 0 if speed is infinite
     **/
    inline bofixed speed() const  { return mSpeed.value(upgradesCollection()); }
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
     * @return The amount of ammunition required for one shot of this
     * weapon.
     **/
    inline unsigned long int requiredAmmunition() const
    {
      return mRequiredAmmunition.value(upgradesCollection());
    }
    const QString& ammunitionType() const
    {
      return mAmmunitionType;
    }

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

    /**
     * @return First part of the sound filename - e.g. "shoot" if the file
     * name should be "shoot_00.ogg". the _00 is added dynamically (randomly)
     * by @ref BosonSound
     **/
    QString sound(int soundEvent) const;

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
     * @param weapon The weapon that fired this shot
     * @param pos Position (center point) of attacker. Note that offset is added to this value
     * @param target Position of shot's target point
     * @return Created shot. Note that it's not added to canvas.
     * Note that pos or target may not be used depending on shot's type
     **/
    BosonShot* newShotAtTopLeftPos(Unit* attacker, const BosonWeapon* weapon, BoVector3Fixed pos, BoVector3Fixed target) const;
    BosonShot* newShotAtTopLeftPos(Unit* attacker, const BosonWeapon* weapon, BoVector3Fixed pos, Unit* target) const;

    Q3ValueList<unsigned long int> shootEffectIds() const  { return mShootEffectIds; }
    Q3ValueList<unsigned long int> flyEffectIds() const  { return mFlyEffectIds; }
    Q3ValueList<unsigned long int> hitEffectIds() const  { return mHitEffectIds; }

    virtual QString name() const;
    virtual bool loadPlugin(KSimpleConfig* config);
    virtual bool savePlugin(KSimpleConfig* config);
    virtual int pluginType() const  { return Weapon; }

    const QMap<int, QString>* actionStrings() const  { return &mActionStrings; }

    const BosonWeaponTurretProperties* turret() const
    {
      return mTurret;
    }



  protected:

    void reset();
    void loadAction(UnitAction type, KSimpleConfig* cfg, const QString& key, bool useDefault = false);

    /**
     * Calls @ref insertULongWeaponBaseValue. @p name is converted to be a valid
     * weapon name, i.e. to "Weapon_ID:name", where ID is @ref id - 1.
     **/
    bool insertULongWeaponBaseValue(unsigned long int v, const QString& name, const QString& type = "MaxValue");
    bool insertLongWeaponBaseValue(long int v, const QString& name, const QString& type = "MaxValue");
    bool insertBoFixedWeaponBaseValue(bofixed v, const QString& name, const QString& type = "MaxValue");

    /**
     * @return @ref ulongBaseValue, but with @p name modified so, that it is a
     * valid weapon property name (see also @ref insertULongWeaponBaseValue).
     **/
    unsigned long int ulongWeaponBaseValue(const QString& name, const QString& type = "MaxValue", unsigned long int defaultValue = 0) const;
    long int longWeaponBaseValue(const QString& name, const QString& type = "MaxValue", long int defaultValue = 0) const;
    bofixed bofixedWeaponBaseValue(const QString& name, const QString& type = "MaxValue", bofixed defaultValue = 0) const;

    friend class BosonWeaponPropertiesEditor;

  private:

    unsigned long int mId;
    BoUpgradeableProperty<unsigned long int> mRange;
    BoUpgradeableProperty<long int> mDamage;
    BoUpgradeableProperty<bofixed> mDamageRange;
    BoUpgradeableProperty<bofixed> mFullDamageRange;
    BoUpgradeableProperty<unsigned long int> mReloadingTime;
    BoUpgradeableProperty<bofixed> mSpeed;
    BoUpgradeableProperty<unsigned long int> mRequiredAmmunition;
    bool mCanShootAtAirUnits;
    bool mCanShootAtLandUnits;
    bofixed mAccelerationSpeed;
    bofixed mHeight;
    BosonShot::Type mShotType;
    QString mModelFileName;
    QString mName;
    Q3ValueList<unsigned long int> mShootEffectIds;
    Q3ValueList<unsigned long int> mFlyEffectIds;
    Q3ValueList<unsigned long int> mHitEffectIds;
    BoVector3Fixed mOffset;
    QMap<int, QString> mSounds;
    QMap<int, QString> mActionStrings;
    bool mAutoUse;
    bool mTakeTargetVeloIntoAccount;
    bofixed mMaxFlyDistance;
    bofixed mTurningSpeed;
    bofixed mStartAngle;
    QString mAmmunitionType;
    BosonWeaponTurretProperties* mTurret;
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
      // AB: up to 100 IDs supported per weapon
      IdReloadCounter = KGamePropertyBase::IdUser + 0,
      IdAmmunition = KGamePropertyBase::IdUser + 1,

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

    virtual void advance(unsigned int advanceCallsCount)
    {
      Q_UNUSED(advanceCallsCount);
    }

    void reload(Unit* owner, unsigned int reloadBy)
    {
      if(mReloadCounter > (int)reloadBy)
      {
        mReloadCounter = mReloadCounter - reloadBy;
      }
      else
      {
        mReloadCounter = 0;
      }
      if(mReloadCounter == 0 && mAmmunition < requiredAmmunition())
      {
        refillAmmunition(owner);
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
    inline bool reloaded() const
    {
      if(mReloadCounter != 0)
      {
        return false;
      }
      if(mAmmunition < requiredAmmunition())
      {
        return false;
      }
      return true;
    }

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
     * @param hvelocity initial horizontal velocity of the bomb
     **/
    bool dropBomb(const BoVector2Fixed& hvelocity);

    virtual void unitDestroyed(Unit*) {}
    virtual void itemRemoved(BosonItem*) {}

    /**
     * @return The weapon range of this unit. It's a number of cells
     **/
    inline unsigned long int range() const  { return mRange.value(upgradesCollection()); }
    /**
     * @return The number of advance calls until the weapon is reloaded
     **/
    inline unsigned int reloadingTime() const  { return mReloadingTime.value(upgradesCollection()); }
    /**
     * The damage this unit makes to other units. Negative values means
     * repairing
     **/
    inline long int damage() const  { return mDamage.value(upgradesCollection()); }
    /**
     * @return Damage range of missile of this unit, e.g. range in what units will be damaged
     **/
    inline bofixed damageRange() const  { return mDamageRange.value(upgradesCollection()); }
    /**
     * @return Full damage range of missile of this unit, e.g. range in what
     *  units will be damaged by damage value (farther they'll be damaged less)
     **/
    inline bofixed fullDamageRange() const  { return mFullDamageRange.value(upgradesCollection()); }
    /**
     * @return Maximum speed that missile of this weapon can have or 0 if speed is infinite
     **/
    inline bofixed speed() const  { return mSpeed.value(upgradesCollection()); }

    /**
     * @return The amount of ammunition required for one shot of this
     * weapon.
     **/
    inline unsigned long int requiredAmmunition() const
    {
      return mRequiredAmmunition.value(upgradesCollection());
    }

    const QString& ammunitionType() const
    {
      return mProp->ammunitionType();
    }

    BosonWeaponTurret* turret() const
    {
      return mTurret;
    }


  protected:
    void shoot(const BoVector3Fixed& pos, const BoVector3Fixed& target);

    void registerWeaponData(int weaponNumber, KGamePropertyBase* prop, int id, bool local = true);

    void refillAmmunition(Unit* owner);

  private:
    const BosonWeaponProperties* mProp;
    KGameProperty<qint32> mReloadCounter;
    KGameProperty<quint32> mAmmunition;

    BoUpgradeableProperty<unsigned long int> mRange;
    BoUpgradeableProperty<long int> mDamage;
    BoUpgradeableProperty<bofixed> mDamageRange;
    BoUpgradeableProperty<bofixed> mFullDamageRange;
    BoUpgradeableProperty<unsigned long int> mReloadingTime;
    BoUpgradeableProperty<bofixed> mSpeed;
    BoUpgradeableProperty<unsigned long int> mRequiredAmmunition;

    BosonWeaponTurret* mTurret;
};


class BosonWeaponTurretPropertiesPrivate;
/**
 * This class contains the data for a weapon turret that is stored in the
 * index.unit file. All data in here are constant, they should not change while
 * the game is running.
 *
 * This class is a helper class for @ref BosonWeaponTurret.
 *
 * See @ref BosonWeaponTurret for the actual turret class.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonWeaponTurretProperties
{
  public:
    BosonWeaponTurretProperties();
    ~BosonWeaponTurretProperties();

    bool loadTurret(KSimpleConfig* config);
    bool saveTurret(KSimpleConfig* config) const;

    /**
     * @return The names of the meshes that are part of this turret. See also
     * @ref BoMesh::name and @ref isMeshPartOfTurret.
     **/
    const QStringList& meshNames() const;

    /**
     * @return Whether @p meshName is meant to be rotated by this weapon turret,
     * i.e. whether it is contained in @ref meshNames. See also @ref
     * BoMesh::name
     **/
    bool isMeshPartOfTurret(const QString& meshName) const;

    /**
     * @return The matrix that is applied to the meshes in their unrotated
     * state.
     **/
    const BoMatrix& initialMeshMatrix() const;

  private:
    BosonWeaponTurretPropertiesPrivate* d;
};

/**
 * This class represents a turret in the gameengine, i.e. a mesh that is rotated
 * towards the direction the weapon fires to.
 *
 * The @ref meshMatrix should be used to control the rotation of the
 * corresponding @ref BoMesh in the @ref BosonModel.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonWeaponTurret
{
  public:
    BosonWeaponTurret(const BosonWeaponTurretProperties* prop);
    ~BosonWeaponTurret();

    bool saveAsXML(QDomElement& root) const;
    bool loadFromXML(const QDomElement& root);

    const QStringList& meshNames() const;

    void setMeshMatrix(BoMatrix* matrix);

    /**
     * @return The matrix that describes how the corresponding meshes of this
     * turret should be rotated. Note that this matrix depends on the gameengine
     * only, i.e. it can not only be used for rendering, but also for game
     * relevant tasks.
     **/
    const BoMatrix& meshMatrix() const
    {
      return mMeshMatrix;
    }

    /**
     * See @ref BosonWeaponTurretProperties::isMeshPartOfTurret
     **/
    bool isMeshPartOfTurret(const QString& meshName) const;


    /**
     * Applies a rotation to the @ref meshMatrix so that the turret points into
     * the direction contained in @p direction. This method assumes that @p
     * direction is actually a direction, i.e. the turret is at position
     * (0,0,0).
     **/
    void pointTo(const BoVector3Fixed& direction, const Unit* unit);

  private:
    const BosonWeaponTurretProperties* mProperties;
    BoMatrix mMeshMatrix;
};

#endif // BOSONWEAPON_H

/*
 * vim: et sw=2
 */
