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

#include "upgradeproperties.h"

#include "player.h"
#include "unitproperties.h"
#include "speciestheme.h"
#include "unit.h"
#include "bodebug.h"

#include <ksimpleconfig.h>
#include <klocale.h>

#include <qstring.h>
#include <qvaluelist.h>


/**********  UpgradePropertiesBase  **********/

UpgradePropertiesBase::UpgradePropertiesBase(bool isTechnology, unsigned long int id)
{
  mTechnology = isTechnology;

  mResearched = false;
  mId = id;
  mName = "";
  mMineralCost = 0;
  mOilCost = 0;
  mProducer = 0;
  mProductionTime = 0;
  mPixmapName = "";
}

UpgradePropertiesBase::~UpgradePropertiesBase()
{
}

bool UpgradePropertiesBase::canBeResearched(Player* player)
{
  if(!mRequireUnits.isEmpty())
  {
    QValueList<unsigned long int>::Iterator it;
    for(it = mRequireUnits.begin(); it != mRequireUnits.end(); ++it)
    {
      if(!player->hasUnitWithType(*it))
      {
        return false;
      }
    }
  }

  if(!mRequireTechnologies.isEmpty())
  {
    QValueList<unsigned long int>::Iterator it;
    for(it = mRequireTechnologies.begin(); it != mRequireTechnologies.end(); ++it)
    {
      if(!player->hasTechnology(*it))
      {
        return false;
      }
    }
  }
  return true;
}

void UpgradePropertiesBase::load(KSimpleConfig* cfg)
{
  boDebug() << k_funcinfo << "Loading; isTech: " << isTechnology() << "; id: " << id() << "" << endl;
  // Load basic stuff
  if(mTechnology)
  {
    // Config group is set earlier
    mId = cfg->readUnsignedLongNumEntry("Id", 0);
    if(mId == 0) {
      boError() << k_funcinfo << "Invalid id: 0" << endl;
    }
    mName = cfg->readEntry("Name", i18n("unknown"));
    mMineralCost = cfg->readUnsignedLongNumEntry("MineralCost", 100);
    mOilCost = cfg->readUnsignedLongNumEntry("OilCost", 0);
    mProducer = cfg->readUnsignedNumEntry("Producer", 0);
    mProductionTime = cfg->readUnsignedNumEntry("ProductionTime", 100);
    mPixmapName = cfg->readEntry("Pixmap", "none.png");
    mRequireUnits = readUIntList(cfg, "RequireUnits");
    mApplyToTypes = readUIntList(cfg, "ApplyToTypes");
    mApplyToFacilities = cfg->readBoolEntry("ApplyToFacilities", false);
    mApplyToMobiles = cfg->readBoolEntry("ApplyToMobiles", false);
  }
  else
  {
    if(!cfg->hasGroup(QString("Upgrade_%1").arg(mId)))
    {
      boError() << k_funcinfo << "While loading upgrade with id " << mId << ": No config group for upgrade" << endl;
      return;
    }
    cfg->setGroup(QString("Upgrade_%1").arg(mId));
  }
  mRequireTechnologies = readUIntList(cfg, "RequireTechnologies");

  // Load upgrade properties
  mHealthSpecified = cfg->hasKey("Health");
  if(mHealthSpecified)
  {
    mHealth = cfg->readUnsignedLongNumEntry("Health", 100);
  }
  mWeaponRangeSpecified = cfg->hasKey("WeaponRange");
  if(mWeaponRangeSpecified)
  {
    mWeaponRange = cfg->readUnsignedLongNumEntry("WeaponRange", 0);
  }
  mSightRangeSpecified = cfg->hasKey("SightRange");
  if(mSightRangeSpecified)
  {
    mSightRange = cfg->readUnsignedNumEntry("SightRange", 5);
  }
  mWeaponDamageSpecified = cfg->hasKey("WeaponDamage");
  if(mWeaponDamageSpecified)
  {
    mWeaponDamage = cfg->readLongNumEntry("WeaponDamage", 0);
  }
  mReloadSpecified = cfg->hasKey("Reload");
  if(mReloadSpecified)
  {
    mReload = cfg->readUnsignedNumEntry("Reload", 0);
  }
  mUnitProductionTimeSpecified = cfg->hasKey("UnitProductionTime");
  if(mUnitProductionTimeSpecified)
  {
    mUnitProductionTime = cfg->readUnsignedNumEntry("UnitProductionTime", 0);
  }
  mUnitMineralCostSpecified = cfg->hasKey("UnitMineralCost");
  if(mUnitMineralCostSpecified)
  {
    mUnitMineralCost = cfg->readUnsignedLongNumEntry("UnitMineralCost", 100);
  }
  mUnitOilCostSpecified = cfg->hasKey("UnitOilCost");
  if(mUnitOilCostSpecified)
  {
    mUnitOilCost = cfg->readUnsignedLongNumEntry("UnitOilCost", 0);
  }
  mArmorSpecified = cfg->hasKey("Armor");
  if(mArmorSpecified)
  {
    mArmor = cfg->readUnsignedLongNumEntry("Armor", 0);
  }
  mShieldsSpecified = cfg->hasKey("Shield");
  if(mShieldsSpecified)
  {
    mShields = cfg->readUnsignedLongNumEntry("Shield", 0);
  }
  mSpeedSpecified = cfg->hasKey("Speed");
  if(mSpeedSpecified)
  {
    mSpeed = cfg->readDoubleNumEntry("Speed", 0);
  }
  mMaxResourcesSpecified = cfg->hasKey("MaxResources");
  if(mMaxResourcesSpecified)
  {
    mMaxResources = cfg->readUnsignedLongNumEntry("MaxResources", 100);
  }
}

QValueList<unsigned long int> UpgradePropertiesBase::readUIntList(KSimpleConfig* cfg, const char* key) const
{
  QValueList<unsigned long int> list;
  QValueList<int> tmplist = cfg->readIntListEntry(key);
  QValueList<int>::Iterator it;
  for(it = tmplist.begin(); it != tmplist.end(); it++) {
    list.append((unsigned long int)(*it));
  }
  return list;
}

#warning FIXME
// AB: I HATE macros

// I do not usually like macros, but this is one place where I feel that macros
//  are good. Parameters:
// list: property will be applied to all unitproperties in list
// player: pointer to owner of UnitProperties
// myvar: name of variable in this class
// upvar: name of variable in UnitProperties
#define applyProperty(list,player,myvar,upvar,name) { \
  QValueList<unsigned long int>::Iterator it; \
  for(it = list.begin(); it != list.end(); it++) \
  { \
    UnitProperties* prop = player->speciesTheme()->nonConstUnitProperties(*it); \
    prop->upvar = myvar; \
  } \
}

// Like above, but also changes unitvar to myvar in all units whose id is
//  in list
#define applyPropertyToUnits(list,player,myvar,upvar,unitvar,name) { \
  applyProperty(list,player,myvar,upvar,name); \
  QPtrListIterator<Unit> uit(player->allUnits()); \
  while(uit.current()) \
  { \
    if(list.contains(uit.current()->id())) \
    { \
      uit.current()->unitvar = myvar; \
    } \
  } \
}

void UpgradePropertiesBase::apply(Player* player)
{
  boDebug() << k_funcinfo << "isTech: " << isTechnology() << "; id: " << id() << endl;
  if(!isResearched())
  {
    boError() << k_funcinfo << "Trying to apply non-researched upgrade" << endl;
  }

  // Add unit types to list
  QValueList<unsigned long int> list = mApplyToTypes;
  if(isTechnology())
  {
    if(mApplyToFacilities)
    {
      list += player->speciesTheme()->allFacilities();
    }
    if(mApplyToMobiles)
    {
      list += player->speciesTheme()->allMobiles();
    }
  }

  if(mHealthSpecified)
  {
    applyPropertyToUnits(list, player, mHealth, mHealth, mHealth, "mHealth");
    applyProperty(list, player, mHealth, mHealth, "mHealth");
    QPtrListIterator<Unit> uit(player->allUnits());
    while(uit.current())
    {
      if(list.contains(uit.current()->id()))
      {
        uit.current()->mHealth = mHealth - (uit.current()->unitProperties()->health() - uit.current()->mHealth);
      }
    }
  }
/*  if(mWeaponRangeSpecified)
  {
    applyPropertyToUnits(list, player, mWeaponRange, mWeaponRange, mWeaponRange, "mWeaponRange");
  }*/
  if(mSightRangeSpecified)
  {
    applyPropertyToUnits(list, player, mSightRange, mSightRange, mSightRange, "mSightRange");
  }
/*  if(mWeaponDamageSpecified)
  {
    applyPropertyToUnits(list, player, mWeaponDamage, mWeaponDamage, mWeaponDamage, "mWeaponDamage");
  }
  if(mReloadSpecified)
  {
    applyProperty(list, player, mReload, mReload, "mReload");
  }*/
  if(mUnitProductionTimeSpecified)
  {
    applyProperty(list, player, mUnitProductionTime, mProductionTime, "mProductionTime");
  }
  if(mUnitMineralCostSpecified)
  {
    applyProperty(list, player, mUnitMineralCost, mMineralCost, "mMineralCost");
  }
  if(mUnitOilCostSpecified)
  {
    applyProperty(list, player, mUnitOilCost, mOilCost, "mOilCost");
  }
  if(mArmorSpecified)
  {
    applyPropertyToUnits(list, player, mArmor, mArmor, mArmor, "mArmor");
  }
  if(mShieldsSpecified)
  {
    applyPropertyToUnits(list, player, mShields, mShields, mShields, "mShields");
  }
  if(mSpeedSpecified)
  {
    // This is tricky one
    QValueList<unsigned long int>::Iterator it;
    for(it = list.begin(); it != list.end(); it++)
    {
      UnitProperties* prop = player->speciesTheme()->nonConstUnitProperties(*it);
      prop->setSpeed(mSpeed);
    }
    QPtrListIterator<Unit> uit(player->allUnits());
    while(uit.current())
    {
      if(list.contains(uit.current()->id()))
      {
        uit.current()->setSpeed(mSpeed);
      }
    }
  }
/*  if(mMaxResourcesSpecified)
  {
    applyProperty(list, player, mHealth, mHealth, "mHealth");
  }*/
}


/**********  UpgradeProperties  **********/

UpgradeProperties::UpgradeProperties(const UnitProperties* parent, unsigned long int id) :
    UpgradePropertiesBase(false, id), PluginProperties(parent)
{
}

UpgradeProperties::~UpgradeProperties()
{
}

QString UpgradeProperties::name() const
{
  return i18n("Upgrade plugin");
}

void UpgradeProperties::loadPlugin(KSimpleConfig* cfg)
{
  load(cfg);
  mApplyToTypes.append(unitProperties()->typeId());
}

void UpgradeProperties::savePlugin(KSimpleConfig* cfg)
{
  /// TODO!!!
}

/**********  TechnologyProperties  **********/

TechnologyProperties::TechnologyProperties() : UpgradePropertiesBase(true)
{
}

TechnologyProperties::~TechnologyProperties()
{
}
