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
#ifndef UPGRADEPROPERTIES_H
#define UPGRADEPROPERTIES_H

#include "bomath.h"
#include <qstring.h>

class Player;
class UnitProperties;
class BoAction;
class SpeciesTheme;

class QString;
class KSimpleConfig;

template<class T> class QValueList;


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
    UpgradeProperties(const SpeciesTheme* theme);
    virtual ~UpgradeProperties();

    /**
     * Load upgrade properties
     **/
    virtual bool load(KSimpleConfig* cfg, const QString& group);

    /**
     * @return Whether it is possible by player to research this upgrade (all
     * requirements are met)
     **/
    virtual bool canBeResearched(Player* player) const;

    /**
     * One of the most important methods in this class
     * It applies upgrade to units and UnitProperties
     * @param player owner of this class (and UnitProperties)
     **/
    virtual void apply(Player* player) const;
    virtual void applyToUnits(Player* player) const;


    /**
     * @return Id of this upgrade
     **/
    unsigned long int id() const { return mId; }
    /**
     * @return Name of this upgrade
     **/
    const QString& upgradeName() const { return mName; }
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
     * @return Action to produce this upgrade
     **/
    const BoAction* produceAction() const { return mProduceAction; }
    /**
     * @return List of units required by this upgrade
     **/
    QValueList<unsigned long int> requiredUnits() const;
    /**
     * @return List of technologies required by this upgrade
     **/
    QValueList<unsigned long int> requiredTechnologies() const;


  protected:
    enum UpgradeType { Health = 0, Armor, Shields, MineralCost, OilCost, SightRange,
        ProductionTime, Speed,
        WeaponDamage, WeaponDamageRange, WeaponFullDamageRange, WeaponReload,
        WeaponRange, WeaponSpeed };
    enum ValueType { Absolute = 0, Relative, Percent };

    void applyProperty(QValueList<unsigned long int>* typeIds, Player* player,
        const QString& data, UpgradeType type, int weaponid = -1) const;
    void applyPropertyToUnits(QValueList<unsigned long int>* typeIds, Player* player,
        const QString& data, UpgradeType type, int weaponid = -1) const;
    void applyPropertyToUnits(bofixed oldvalue, bofixed newvalue,
        unsigned long int typeId, Player* player, UpgradeType type) const;


    unsigned long int applyValue(const QString& data, unsigned long int oldvalue) const;
    bofixed applyValue(const QString& data, bofixed oldvalue) const;
    bool parseEntryType(const QString& typeString, UpgradeType* type, int* weaponid) const;
    void parseEntry(const QString& entry, ValueType& type, QString& value) const;

    /**
     * @return A list of unit types the upgrade applies to. This can be a fixed
     * list of units or all mobiles/all facilities.
     **/
    QValueList<unsigned long int> appliesToTypes(const Player* player) const;

  private:
    /**
     * Note: you are meant to use primitive data (int, uint, bofixed, ...) only
     * here!
     * You should avoid classes
     *
     * @param oldvalue The initial value
     * @param value The change-value. What this actually does depends on the
     * @ref ValueType of the value.
     * @return The new value, i.e. @p oldValue changed by @p value according to
     * @p data.
     **/
    template<class T> T applyValueInternal(ValueType type, T oldvalue, T value) const;

  private:
    unsigned long int mId;
    QString mName;
    unsigned long int mMineralCost;
    unsigned long int mOilCost;
    unsigned int mProducer;
    unsigned int mProductionTime;
    bool mApplyToFacilities;
    bool mApplyToMobiles;
    const BoAction* mProduceAction;
    const SpeciesTheme* mTheme;

    class UpgradePropertiesPrivate;
    UpgradePropertiesPrivate* d;
};

#endif // UPGRADEPROPERTIES_H
