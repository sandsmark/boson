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

#include "upgradeproperties.h"

#include "player.h"
#include "unitproperties.h"
#include "speciestheme.h"
#include "unit.h"
#include "bodebug.h"
#include "bosonconfig.h"
#include "bosonweapon.h"
#include "bosoncanvas.h"
#include "boaction.h"

#include <ksimpleconfig.h>
#include <klocale.h>

#include <qstring.h>
#include <qvaluelist.h>

class UpgradeProperties::UpgradePropertiesPrivate
{
public:
  UpgradePropertiesPrivate()
  {
  }

  QValueList<unsigned long int> mRequireUnits;
  QValueList<unsigned long int> mRequireTechnologies;
  QValueList<unsigned long int> mApplyToTypes;

  QMap<QString, QString> mEntryList;
};

UpgradeProperties::UpgradeProperties(const SpeciesTheme* theme)
{
  d = new UpgradePropertiesPrivate;

  mTheme = theme;

  mId = 0;
  mMineralCost = 0;
  mOilCost = 0;
  mProducer = 0;
  mProductionTime = 0;
  mProduceAction = 0;
  mApplyToFacilities = false;
  mApplyToMobiles = false;
}

UpgradeProperties::~UpgradeProperties()
{
  delete d;
}

void UpgradeProperties::resetUpgradeableUnitProperties(Player* player)
{
  if(!player || !player->speciesTheme())
  {
    return;
  }
  QValueList<const UnitProperties*> list = player->speciesTheme()->allUnits();
  QValueList<const UnitProperties*>::Iterator it;
  for(it = list.begin(); it != list.end(); ++it)
  {
    UnitProperties* prop = player->speciesTheme()->nonConstUnitProperties((*it)->typeId());
    for(int i = 0; i < LastUpgrade; i++)
    {
      switch((UpgradeType)i)
      {
        // AB: do NOT add a default!
        // -> I want a compiler warning if something is missing here
#define RESET(name, enumName) case enumName: prop->resetUpgradedValue_##name(); break;

        RESET(health, Health);
        RESET(armor, Armor);
        RESET(shields, Shields);
        RESET(mineralCost, MineralCost);
        RESET(oilCost, OilCost);
        RESET(sightRange, SightRange);
        RESET(productionTime, ProductionTime);
        RESET(speed, Speed);

#undef RESET

        case WeaponDamage:
        case WeaponDamageRange:
        case WeaponFullDamageRange:
        case WeaponReload:
        case WeaponRange:
        case WeaponSpeed:
          break;

        case LastUpgrade:
          break;
      }
    }
  }
}

void UpgradeProperties::resetUpgradeableWeaponProperties(Player* player)
{
  if(!player || !player->speciesTheme())
  {
    return;
  }
  QValueList<const UnitProperties*> list = player->speciesTheme()->allUnits();
  QValueList<const UnitProperties*>::Iterator it;
  for(it = list.begin(); it != list.end(); ++it)
  {
    UnitProperties* prop = player->speciesTheme()->nonConstUnitProperties((*it)->typeId());
    for (int weaponid = 1; true; weaponid++)
    {
      BosonWeaponProperties* wep = const_cast<BosonWeaponProperties*>(prop->weaponProperties(weaponid));
      if(!wep)
      {
        break;
      }
      for(int i = 0; i < LastUpgrade; i++)
      {
        switch((UpgradeType)i)
        {
          // AB: do NOT add a default!
          // -> I want a compiler warning if something is missing here

#define RESET(name, enumName) case enumName: wep->resetUpgradedValue_##name(); break;

          RESET(damage, WeaponDamage);
          RESET(damageRange, WeaponDamageRange);
          RESET(fullDamageRange, WeaponFullDamageRange);
          RESET(reloadingTime, WeaponReload);
          RESET(range, WeaponRange);
          RESET(speed, WeaponSpeed);

#undef RESET

          case Health:
          case Armor:
          case Shields:
          case MineralCost:
          case OilCost:
          case SightRange:
          case ProductionTime:
          case Speed:
            break;
          case LastUpgrade:
            break;
        }
      }
    }
  }
}

bool UpgradeProperties::canBeResearched(Player* player) const
{
  if(!d->mRequireUnits.isEmpty())
  {
    QValueList<unsigned long int>::Iterator it;
    for(it = d->mRequireUnits.begin(); it != d->mRequireUnits.end(); ++it)
    {
      if(!player->hasUnitWithType(*it))
      {
        return false;
      }
    }
  }

  if(!d->mRequireTechnologies.isEmpty())
  {
    QValueList<unsigned long int>::Iterator it;
    for(it = d->mRequireTechnologies.begin(); it != d->mRequireTechnologies.end(); ++it)
    {
      if(!player->hasTechnology(*it))
      {
        return false;
      }
    }
  }
  return true;
}

bool UpgradeProperties::load(KSimpleConfig* cfg, const QString& group)
{
  boDebug(600) << k_funcinfo << "Loading from group " << group << endl;

  // Load entry list
  d->mEntryList = cfg->entryMap(group);

  // Load basic stuff
  cfg->setGroup(group);
  mId = cfg->readUnsignedLongNumEntry("Id", 0);
  if(mId == 0) {
    boError(600) << k_funcinfo << "Invalid id: 0" << endl;
    return false;
  }
  mName = cfg->readEntry("Name", i18n("unknown"));
  mMineralCost = cfg->readUnsignedLongNumEntry("MineralCost", 0);
  mOilCost = cfg->readUnsignedLongNumEntry("OilCost", 0);
  mProducer = cfg->readUnsignedNumEntry("Producer", 0);
  mProductionTime = cfg->readUnsignedNumEntry("ProductionTime", 100);
  mProduceAction = mTheme->action(cfg->readEntry("ProduceAction", ""));
  d->mRequireUnits = BosonConfig::readUnsignedLongNumList(cfg, "RequireUnits");
  d->mRequireTechnologies = BosonConfig::readUnsignedLongNumList(cfg, "RequireTechnologies");
  d->mApplyToTypes = BosonConfig::readUnsignedLongNumList(cfg, "ApplyToTypes");
  mApplyToFacilities = cfg->readBoolEntry("ApplyToFacilities", false);
  mApplyToMobiles = cfg->readBoolEntry("ApplyToMobiles", false);

  // Remove unwanted entries from the entry list
  d->mEntryList.remove("Id");
  d->mEntryList.remove("ApplyToTypes");
  d->mEntryList.remove("ApplyToFacilities");
  d->mEntryList.remove("ApplyToMobiles");
  d->mEntryList.remove("Name");
  d->mEntryList.remove("MineralCost");
  d->mEntryList.remove("OilCost");
  d->mEntryList.remove("ProduceAction");
  d->mEntryList.remove("Producer");
  d->mEntryList.remove("ProductionTime");
  d->mEntryList.remove("Pixmap");
  d->mEntryList.remove("RequireUnits");
  d->mEntryList.remove("RequireTechnologies");
  return true;
}


void UpgradeProperties::apply(Player* player) const
{
  boDebug(600) << k_funcinfo << "id: " << id() << endl;

  // Add unit types to list
  QValueList<unsigned long int> list = appliesToTypes(player);
  QValueList<unsigned long int>::Iterator tit;
  QString addingTo = "Adding to types:";
  for(tit = list.begin(); tit != list.end(); tit++)
  {
    addingTo += " ";
    addingTo += QString::number(*tit);
  }
  boDebug(600) << "  " << k_funcinfo << addingTo << endl;

  // Iterate through entries map and apply them
  QMap<QString, QString>::Iterator it;
  for(it = d->mEntryList.begin(); it != d->mEntryList.end(); it++)
  {
    UpgradeType type;
    int weaponid = -1;

    if(!parseEntryType(it.key(), &type, &weaponid))
    {
      boError(600) << k_funcinfo << "unable to retrieve type from string " << it.key() << endl;
      continue;
    }
    applyProperty(&list, player, it.data(), type, weaponid);
  }
}

void UpgradeProperties::applyToUnits(Player* player) const
{
  boDebug(600) << k_funcinfo << "id: " << id() << endl;

  // Add unit types to list
  QValueList<unsigned long int> list = appliesToTypes(player);

  // Iterate through entries map and apply them
  QMap<QString, QString>::Iterator it;
  for(it = d->mEntryList.begin(); it != d->mEntryList.end(); it++)
  {
    UpgradeType type;
    int weaponid = -1;

    if(!parseEntryType(it.key(), &type, &weaponid))
    {
      boError(600) << k_funcinfo << "unable to retrieve type from string " << it.key() << endl;
      continue;
    }
    applyPropertyToUnits(&list, player, it.data(), type, weaponid);
  }
}

void UpgradeProperties::applyProperty(QValueList<unsigned long int>* typeIds,
    Player* player, const QString& data, UpgradeType type, int weaponid) const
{
  // Note that I don't use k_funcinfo here, because I get _very_ long lines with it
#define my_funcinfo "[UpgradeProperties::applyProperty(...)] "

  boDebug(600) << "    " << my_funcinfo << "Applying property (type: " << type << ") to " << typeIds->count() << " properites. weaponid: " << weaponid << endl;
  QValueList<unsigned long int>::Iterator it;
  for(it = typeIds->begin(); it != typeIds->end(); it++)
  {
    UnitProperties* prop = player->speciesTheme()->nonConstUnitProperties(*it);
    boDebug(600) << "        " << my_funcinfo << "Applying to prop with id: " << prop->typeId() << "; name: " << prop->name() << endl;
    if(weaponid == -1)
    {
      // Not a weapon upgrade
      switch(type)
      {

/**
 * This macro creates the case entries.
 * @param name is the name of the property. We require that there is a method in
 * @ref UnitProperties with that name returning the current value and that
 * a corresponding setUpgradedValue_name exists.
 * @param enumName is the name of the type enum
 **/
#define APPLY(name, enumName) \
          case enumName: \
          { \
            prop->setUpgradedValue_##name(applyValue(data, prop->name())); \
            break; \
          }

        APPLY(health, Health);
        APPLY(armor, Armor);
        APPLY(shields, Shields);
        APPLY(mineralCost, MineralCost);
        APPLY(oilCost, OilCost);
        APPLY(sightRange, SightRange);
        APPLY(productionTime, ProductionTime);
        APPLY(speed, Speed);

#undef APPLY

        default:
        {
          boError(600) << my_funcinfo << "Invalid UpgradeType: " << type << endl;
          break;
        }
      }
    }
    else
    {
      unsigned long int oldvalueuint = 0;
      bofixed oldvaluef = 0;
      // It's a weapon upgrade
      // Weapon's ids actually start from 1. So we add 1 to weaponid here
      BosonWeaponProperties* wep = const_cast<BosonWeaponProperties*>(prop->weaponProperties(weaponid + 1));
      if(!wep)
      {
        boError(600) << "[UpgradeProperties::applyProperty(...)]" << "NULL const weaponproperties (id: " << weaponid << ")" << endl;
        return;
      }
      switch(type)
      {

/**
 * This macro is similar to the one above for non-weapon properties
 **/
#define APPLY(name, enumName) \
          case enumName: \
          { \
            wep->setUpgradedValue_##name(applyValue(data, wep->name())); \
            break; \
          }

        APPLY(damage, WeaponDamage);
        APPLY(damageRange, WeaponDamageRange);
        APPLY(fullDamageRange, WeaponFullDamageRange);
        APPLY(reloadingTime, WeaponReload);
        APPLY(range, WeaponRange);
        APPLY(speed, WeaponSpeed);
        default:
        {
          oldvalueuint = 0;
          oldvaluef = 0.0f;
          boError(600) << "[UpgradeProperties::applyProperty(...)]" << "Invalid UpgradeType: " << type << endl;
          break;
        }
      }
    }
  }

#undef my_funcinfo
}

void UpgradeProperties::applyPropertyToUnits(QValueList<unsigned long int>* typeIds,
    Player* player, const QString& data, UpgradeType type, int weaponid) const
{
  // Note that I don't use k_funcinfo here, because I get _very_ long lines with it
#define my_funcinfo "[UpgradeProperties::applyPropertyToUnits(...)] "

  if(weaponid != -1)
  {
    return;
  }
  // Not a weapon upgrade

  QValueList<unsigned long int>::Iterator it;
  for(it = typeIds->begin(); it != typeIds->end(); it++)
  {
    unsigned long int oldvalueuint = 0;
    bofixed oldvaluef = 0;
    unsigned long int newvalueuint = 0;
    bofixed newvaluef = 0;
    UnitProperties* prop = player->speciesTheme()->nonConstUnitProperties(*it);
    switch(type)
    {
      case Health:
      {
        oldvalueuint = prop->health();
        newvalueuint = applyValue(data, oldvalueuint);
        applyPropertyToUnits((bofixed)oldvalueuint, (bofixed)newvalueuint, *it, player, type);
        break;
      }
      case Armor:
      {
        oldvalueuint = prop->armor();
        newvalueuint = applyValue(data, oldvalueuint);
        applyPropertyToUnits((bofixed)oldvalueuint, (bofixed)newvalueuint, *it, player, type);
        break;
      }
      case Shields:
      {
        oldvalueuint = prop->shields();
        newvalueuint = applyValue(data, oldvalueuint);
        applyPropertyToUnits((bofixed)oldvalueuint, (bofixed)newvalueuint, *it, player, type);
        break;
      }
      case MineralCost:
        break;
      case OilCost:
        break;
      case SightRange:
      {
        oldvalueuint = prop->sightRange();
        newvalueuint = applyValue(data, oldvalueuint);
        applyPropertyToUnits((bofixed)oldvalueuint, (bofixed)newvalueuint, *it, player, type);
        break;
      }
      case ProductionTime:
        break;
      case Speed:
        break;
      default:
      {
        oldvalueuint = 0;
        oldvaluef = 0;
        boError(600) << my_funcinfo << "Invalid UpgradeType: " << type << endl;
        break;
      }
    }
  }

#undef my_funcinfo
}

template<class T> T UpgradeProperties::applyValueInternal(ValueType type, T oldvalue, T value) const
{
  if(type == Absolute)
  {
    return value;
  }
  else if(type == Relative)
  {
    return oldvalue + value;
  }
  else if(type == Percent)
  {
    return (unsigned long int)(value * oldvalue / 100.0);
  }
  else
  {
    // Shouldn't happen
    boError(600) << k_funcinfo << "Invalid type: " << type << endl;
    return oldvalue;
  }
}

unsigned long int UpgradeProperties::applyValue(const QString& data, unsigned long int oldvalue) const
{
  ValueType type;
  QString valuestr;
  parseEntry(data, type, valuestr);
  unsigned long int value = valuestr.toULong();
  return applyValueInternal(type, oldvalue, value);
}

bofixed UpgradeProperties::applyValue(const QString& data, bofixed oldvalue) const
{
  ValueType type;
  QString valuestr;
  parseEntry(data, type, valuestr);
  bofixed value = valuestr.toFloat();
  return applyValueInternal(type, oldvalue, value);
}

bool UpgradeProperties::parseEntryType(const QString& typeString, UpgradeType* type, int* weaponid) const
{
  *weaponid = -1;
  if(typeString == "Health")
  {
    boDebug(600) << k_funcinfo << "Found entry: Health" << endl;
    *type = Health;
  }
  else if(typeString == "Armor")
  {
    boDebug(600) << k_funcinfo << "Found entry: Armor" << endl;
    *type = Armor;
  }
  else if(typeString == "Shields")
  {
    boDebug(600) << k_funcinfo << "Found entry: Shields" << endl;
    *type = Shields;
  }
  else if(typeString == "UnitMineralCost")
  {
    boDebug(600) << k_funcinfo << "Found entry: UnitMineralCost" << endl;
    *type = MineralCost;
  }
  else if(typeString == "UnitOilCost")
  {
    boDebug(600) << k_funcinfo << "Found entry: UnitOilCost" << endl;
    *type = OilCost;
  }
  else if(typeString == "SightRange")
  {
    boDebug(600) << k_funcinfo << "Found entry: SightRange" << endl;
    *type = SightRange;
  }
  else if(typeString == "UnitProductionTime")
  {
    boDebug(600) << k_funcinfo << "Found entry: UnitProductionTime" << endl;
    *type = ProductionTime;
  }
  else if(typeString == "Speed")
  {
    boDebug(600) << k_funcinfo << "Found entry: Speed" << endl;
    *type = Speed;
  }
  else if(typeString.left(7) == "Weapon_")
  {
    boDebug(600) << k_funcinfo << "Found weapon entry: " << typeString << " parsing..." << endl;
    // Find weapon id
    QString str = typeString.right(typeString.length() - 7);
    int i = str.find(':');
    boDebug(600) << "        " << k_funcinfo << "str: " << str << "; i: " << i << endl;
    if(i < 1)
    {
      boError(600) << k_funcinfo << "Invalid weapon key: " << typeString << endl;
      return false;
    }
    *weaponid = str.left(i).toInt();
    boDebug(600) << "    " << k_funcinfo << "Weapon id as a string: \"" << str.left(i) << "\", as int: "
        << *weaponid << endl;
    // Extract "weapon key", e.g. Damage, Range etc. from key
    str = str.right(str.length() - i - 1);  // -1 is ":"
    boDebug(600) << "        " << k_funcinfo << "weapon key: " << str << endl;
    if(str == "Damage")
    {
      boDebug(600) << k_funcinfo << "Found entry: WeaponDamage" << endl;
      *type = WeaponDamage;
    }
    else if(str == "DamageRange")
    {
      boDebug(600) << k_funcinfo << "Found entry: WeaponDamageRange" << endl;
      *type = WeaponDamageRange;
    }
    else if(str == "FullDamageRange")
    {
      boDebug(600) << k_funcinfo << "Found entry: WeaponFullDamageRange" << endl;
      *type = WeaponFullDamageRange;
    }
    else if(str == "Range")
    {
      boDebug(600) << k_funcinfo << "Found entry: WeaponRange" << endl;
      *type = WeaponRange;
    }
    else if(str == "Reload")
    {
      boDebug(600) << k_funcinfo << "Found entry: WeaponReload" << endl;
      *type = WeaponReload;
    }
    else if(str == "Speed")
    {
      boDebug(600) << k_funcinfo << "Found entry: WeaponSpeed" << endl;
      *type = WeaponSpeed;
    }
    else
    {
      boError(600) << k_funcinfo << "Unrecogniced weapon key: \"" << str << "\", skipping" << endl;
      return false;
    }
  }
  else
  {
    boError(600) << k_funcinfo << "Unrecogniced key: \"" << typeString << "\", skipping" << endl;
    return false;
  }
  return true;
}

void UpgradeProperties::parseEntry(const QString& entry, ValueType& type, QString& value) const
{
  if(entry.left(1) == "+" || entry.left(1) == "-")
  {
    type = Relative;
    value = entry;
  }
  else if(entry.right(1) == "%")
  {
    type = Percent;
    value = entry.left(entry.length() - 1);
  }
  else
  {
    type = Absolute;
    value = entry;
  }
}

void UpgradeProperties::applyPropertyToUnits(bofixed oldvalue, bofixed newvalue,
    unsigned long int typeId, Player* player, UpgradeType type) const
{
#define my_funcinfo "[UpgradeProperties::applyPropertyToUnits(...)] "
  boDebug(600) << "          " << my_funcinfo <<
      "PARAMS: oldvalue: " << oldvalue << "; newvalue: " << newvalue << 
      "; typeId: " << typeId <<
      "; player: " << player << "; type: " << type << endl;
  //boDebug(600) << "      " << k_funcinfo << "Starting to apply" << endl;
  QPtrListIterator<Unit> it(*(player->allUnits()));
  //boDebug(600) << "        " << k_funcinfo << "Unit count: " << it.count() << endl;
  while(it.current())
  {
    Unit* u = it.current();
    if(u->type() == typeId)
    {
      switch(type)
      {
        case Health:
          // Health is special: we have to take old health into account as well
          u->setHealth((unsigned long int)((u->health() / oldvalue) * newvalue));
          break;
        case Shields:
          // Shields are also quite special, but not that special, so we use simpler approach here
          u->setShields((unsigned long int)(newvalue - (oldvalue - u->shields())));
          break;
        case Armor:
          // New value will just replace the old one
          u->setArmor(newvalue);
          break;
        case SightRange:
          u->setSightRange(newvalue);
          // Update unit's sight
          u->canvas()->updateSight(u, u->x(), u->y());
          break;
        default:
          boError(600) << my_funcinfo << "Invalid UpgradeType: " << type << endl;
          return;
      }
    }
    ++it;
  }
#undef my_funcinfo
}

QValueList<unsigned long int> UpgradeProperties::requiredUnits() const
{
  return d->mRequireUnits;
}


QValueList<unsigned long int> UpgradeProperties::requiredTechnologies() const
{
  return d->mRequireTechnologies;
}

QValueList<unsigned long int> UpgradeProperties::appliesToTypes(const Player* player) const
{
  QValueList<unsigned long int> tmp = d->mApplyToTypes;
  if(mApplyToFacilities)
  {
    tmp += player->speciesTheme()->allFacilities();
  }
  if(mApplyToMobiles)
  {
    tmp += player->speciesTheme()->allMobiles();
  }

  // make sure that every type is at most once in the list
  QValueList<unsigned long int> list;
  QValueList<unsigned long int>::Iterator it;
  for(it = tmp.begin(); it != tmp.end(); ++it)
  {
    if(!list.contains(*it))
    {
      list.append(*it);
    }
  }
  return list;
}


/*
 * vim:et sw=2
 */
