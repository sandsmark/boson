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
    UpgradeProperties(SpeciesTheme* theme);
    virtual ~UpgradeProperties();

    /**
     * Load upgrade properties
     **/
    virtual void load(KSimpleConfig* cfg, const QString& group);

    /**
     * @return Whether it is possible by player to research this upgrade (all
     * requirements are met)
     **/
    virtual bool canBeResearched(Player* player);

    /**
     * @return Whether this upgrade/tech. is already researched
     **/
     bool isResearched() const { return mResearched; };

    /**
     * Set 'researched' status of this upgrade (whether it's already researched)
     * Use this only in Player class!!!
     **/
     void setResearched(bool r) { mResearched = r; };

    /**
     * One of the most important methods in this class
     * It applies upgrade to units and UnitProperties
     * You must call setResearched(true) before this can have any effect
     * @param player owner of this class (and UnitProperties)
     **/
    virtual void apply(Player* player);


    /**
     * @return Id of this upgrade
     **/
    unsigned long int id() const { return mId; };
    /**
     * @return Name of this upgrade
     **/
    const QString& upgradeName() const { return mName; };
    /**
     * @return Mineral cost of this upgrade
     **/
    unsigned long int mineralCost() const { return mMineralCost; };
    /**
     * @return Oil cost of this upgrade
     **/
    unsigned long int oilCost() const { return mOilCost; };
    /**
     * @return producer of this upgrade. Producer ids are used
     **/
    unsigned int producer() const { return mProducer; };
    /**
     * @return How much time it takes to produce this upgrade
     **/
    unsigned int productionTime() const { return mProductionTime; };
    /**
     * @return Action to produce this upgrade
     **/
    BoAction* produceAction() const { return mProduceAction; };
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
        const QString& data, UpgradeType type, int weaponid = -1);
    void applyPropertyToUnits(float oldvalue, unsigned long int typeId,
        Player* player, UpgradeType type);


    unsigned long int applyValue(const QString& data, unsigned long int oldvalue);
    float applyValue(const QString& data, float oldvalue);
    void parseEntry(const QString& entry, ValueType& type, QString& value);

    class UpgradePropertiesPrivate;
    UpgradePropertiesPrivate* d;

  private:
    /**
     * Note: you are meant to use primitive data (int, uint, float, ...) only
     * here!
     * You should avoid classes
     *
     * @param oldvalue The initial value
     * @param value The change-value. What this actually does depends on the
     * @ref ValueType of the value.
     * @return The new value, i.e. @p oldValue changed by @p value according to
     * @p data.
     **/
    template<class T> T applyValueInternal(ValueType type, T oldvalue, T value);

  private:
    bool mResearched;
    unsigned long int mId;
    QString mName;
    unsigned long int mMineralCost;
    unsigned long int mOilCost;
    unsigned int mProducer;
    unsigned int mProductionTime;
    bool mApplyToFacilities;
    bool mApplyToMobiles;
    BoAction* mProduceAction;
    SpeciesTheme* mTheme;
};

#endif // UPGRADEPROPERTIES_H
