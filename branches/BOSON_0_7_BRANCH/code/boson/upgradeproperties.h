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

#include "pluginproperties.h"

class Player;
class QString;
class KSimpleConfig;
class UnitProperties;
template<class T> class QValueList;

/**
 * @short Base class for properties of upgrades and technologies
 *
 * In Boson, there are 2 types of upgrades: upgrades and technologies.
 * Upgrades are unit-specific, they only affect single unit, while technology
 * can affect many units at once
 *
 * This class stores all properties of upgrade or technology.
 * It has methods to check if upgrade can be researched, to apply changed
 * values to UnitProperties etc.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class UpgradePropertiesBase
{
  public:
    /**
     * Constructs UpgradePropertiesBase
     * @param isTechnology If it is technology or upgrade
     * @param id Id of this _upgrade_. Technologies will get id when they're loaded
     **/
    UpgradePropertiesBase(bool isTechnology, unsigned long int id = 0);
    virtual ~UpgradePropertiesBase();

    /**
     * Load upgrade or tech. properties
     * Note that if it's technology, you _MUST_ have set right group before
     **/
    virtual void load(KSimpleConfig* cfg);

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
     * @return Whether it is technology, not upgrade
     **/
    bool isTechnology() const { return mTechnology; };

    /**
     * One of the most important methods in this class
     * It applies upgrade to units
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
    QString upgradeName() const { return mName; };
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
     * @return Filename of pixmap of this upgrade
     **/
    QString pixmapName() const { return mPixmapName; };
    /**
     * @return List of units required by this upgrade
     **/
    QValueList<unsigned long int> requiredUnits() const;
    /**
     * @return List of technologies required by this upgrade
     **/
    QValueList<unsigned long int> requiredTechnologies() const;

  protected:
    QValueList<unsigned long int> readUIntList(KSimpleConfig* cfg, const char* key) const;

  protected:
    class UpgradePropertiesBasePrivate;
    UpgradePropertiesBasePrivate* d;

    unsigned long int mHealth;
    unsigned long int mWeaponRange;
    unsigned int mSightRange;
    long int mWeaponDamage;
    unsigned int mReload;
    unsigned int mUnitProductionTime;
    unsigned long int mUnitMineralCost;
    unsigned long int mUnitOilCost;
    unsigned long int mArmor;
    unsigned long int mShields;
    float mSpeed;
    unsigned long int mMaxResources;

    bool mHealthSpecified;
    bool mWeaponRangeSpecified;
    bool mSightRangeSpecified;
    bool mWeaponDamageSpecified;
    bool mReloadSpecified;
    bool mUnitProductionTimeSpecified;
    bool mUnitMineralCostSpecified;
    bool mUnitOilCostSpecified;
    bool mArmorSpecified;
    bool mShieldsSpecified;
    bool mSpeedSpecified;
    bool mMaxResourcesSpecified;

    bool mResearched;
    bool mTechnology;

    unsigned long int mId;
    QString mName;
    unsigned long int mMineralCost;
    unsigned long int mOilCost;
    unsigned int mProducer;
    unsigned int mProductionTime;
    QString mPixmapName;
    bool mApplyToFacilities;
    bool mApplyToMobiles;
};

/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class UpgradeProperties : public UpgradePropertiesBase, public PluginProperties
{
  public:
    UpgradeProperties(const UnitProperties* parent, unsigned long int id);
    virtual ~UpgradeProperties();

    virtual QString name() const;
    virtual int pluginType() const { return Upgrade; };
    virtual void loadPlugin(KSimpleConfig* cfg);
    virtual void savePlugin(KSimpleConfig* cfg);
};

class TechnologyProperties : public UpgradePropertiesBase
{
  public:
    TechnologyProperties();
    virtual ~TechnologyProperties();
};

#endif // UPGRADEPROPERTIES_H
