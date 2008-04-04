/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 Rivo Laks (rivolaks@hot.ee)
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef UPGRADEPROPERTIES_H
#define UPGRADEPROPERTIES_H

#include "../bomath.h"
#include <qstring.h>

class Player;
class UnitProperties;
class Unit;
class UnitBase;
class BosonWeaponProperties;
class BosonWeapon;
class SpeciesTheme;

class QString;
class KSimpleConfig;

template<class T> class QValueList;


class UpgradePropertiesPrivate;
/**
 * @short Upgrades class
 *
 * This class stores all properties of an upgrade.
 * It has methods to check if upgrade can be researched, to apply changed
 * values to UnitProperties etc.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class UpgradeProperties
{
  public:
    /**
     * Constructs UpgradeProperties
     * You should call @ref load after that.
     **/
    UpgradeProperties(const QString& type, const SpeciesTheme* theme);
    virtual ~UpgradeProperties();

    bool appliesTo(const UnitProperties* prop) const;
    bool appliesTo(const UnitBase* unit) const;
    bool appliesTo(const BosonWeaponProperties* prop) const;
    bool appliesTo(const BosonWeapon* weapon) const;

    bool upgradeUnit(UnitBase* unit) const;
    bool downgradeUnit(UnitBase* unit) const;

    /**
     * Upgrade @p v. The value is considered to be the "maximum" value of the
     * property.
     * @param type allowed values: "MaxValue" (default), "MinValue". This
     * parameters decides which kind of value is being upgraded. For example a
     * turret may have an upgradeable weaponrange with a minimum and a maximum
     * value. Note that with some properties MinValue does not make sense (such
     * as health).
     * @return The new maximum value of the property
     **/
    bool upgradeValue(const QString& name, unsigned long int* v, const QString& type = "MaxValue") const;

    /**
     * @overload
     **/
    bool upgradeValue(const QString& name, long int* v, const QString& type = "MaxValue") const;

    /**
     * @overload
     **/
    bool upgradeValue(const QString& name, bofixed* v, const QString& type = "MaxValue") const;

    /**
     * Load upgrade properties
     **/
    virtual bool load(KSimpleConfig* cfg, const QString& group);

    /**
     * @return The type of this upgrade. The type together with the @ref id
     * uniquely identify an upgrade.
     **/
    const QString& type() const { return mType; }

    /**
     * @return Id of this upgrade. The Id is unique among different upgrades of
     * the same @ref type.
     **/
    unsigned long int id() const { return mId; }

    /**
     * @return Name of this upgrade
     **/
    const QString& upgradeName() const { return mName; }
    /**
     * @return Description of this upgrade (shown to the player)
     **/
    const QString& upgradeDescription() const { return mDescription; }

    /**
     * @return Mineral cost of this upgrade
     **/
    unsigned long int mineralCost() const { return mMineralCost; }

    /**
     * @return Oil cost of this upgrade
     **/
    unsigned long int oilCost() const { return mOilCost; }

    /**
     * @return producer of this upgrade. Producer ids are used
     **/
    unsigned int producer() const { return mProducer; }

    /**
     * @return How much time it takes to produce this upgrade
     **/
    unsigned int productionTime() const { return mProductionTime; }

    /**
     * @return List of units required by this upgrade
     **/
    QValueList<unsigned long int> requiredUnits() const;

    /**
     * @return List of technologies required by this upgrade
     **/
    QValueList<unsigned long int> requiredTechnologies() const;

    const QString& produceActionString() const { return mProduceActionString; }


  protected:
    enum UpgradeType
    {
        Health = 0,
        Armor,
        Shields,
        MineralCost,
        OilCost,
        SightRange,
        ProductionTime,
        Speed,
        WeaponDamage,
        WeaponDamageRange,
        WeaponFullDamageRange,
        WeaponReload,
        WeaponRange,
        WeaponSpeed,

        LastUpgrade // MUST be last entry in the list!
    };
    enum ValueType { Absolute = 0, Relative, Percent };


    /**
     * @return A list of unit types the upgrade applies to. This can be a fixed
     * list of units or all mobiles/all facilities.
     **/
    QValueList<unsigned long int> appliesToTypes(const Player* player) const;

  private:
    /**
     * Converts entries in entry list from seconds to advance calls if
     *  necessary.
     **/
    void convertEntries();

  private:
    QString mType;
    unsigned long int mId;
    QString mName;
    QString mDescription;
    unsigned long int mMineralCost;
    unsigned long int mOilCost;
    unsigned int mProducer;
    unsigned int mProductionTime;
    bool mApplyToFacilities;
    bool mApplyToMobiles;
    QString mProduceActionString;

    UpgradePropertiesPrivate* d;

    friend class UpgradeApplyer;
};


#endif

/*
 * vim: et sw=2
 */
