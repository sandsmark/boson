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

  mResearched = false;
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
  if(!isResearched())
  {
    boError(600) << k_funcinfo << "Trying to apply non-researched upgrade" << endl;
    return;
  }

  // Add unit types to list
  QValueList<unsigned long int> list = d->mApplyToTypes;
  if(mApplyToFacilities)
  {
    list += player->speciesTheme()->allFacilities();
  }
  if(mApplyToMobiles)
  {
    list += player->speciesTheme()->allMobiles();
  }
  QValueList<unsigned long int>::Iterator tit;
  QString addingTo = "Adding to types:";
  for(tit = list.begin(); tit != list.end(); tit++)
  {
    addingTo += " ";
    addingTo += QString::number(*tit);
  }
  boDebug(600) << "  " << k_funcinfo << addingTo << endl;
  // TODO: check for double typeIDs

  // Iterate through entries map and apply them
  QMap<QString, QString>::Iterator it;
  UpgradeType type;
  int weaponid;
  for(it = d->mEntryList.begin(); it != d->mEntryList.end(); it++)
  {
    // Reset weaponid
    weaponid = -1;
    if(it.key() == "Health")
    {
      boDebug(600) << k_funcinfo << "Found entry: Health;  value: " << it.data() << endl;
      type = Health;
    }
    else if(it.key() == "Armor")
    {
      boDebug(600) << k_funcinfo << "Found entry: Armor;  value: " << it.data() << endl;
      type = Armor;
    }
    else if(it.key() == "Shields")
    {
      boDebug(600) << k_funcinfo << "Found entry: Shields;  value: " << it.data() << endl;
      type = Shields;
    }
    else if(it.key() == "UnitMineralCost")
    {
      boDebug(600) << k_funcinfo << "Found entry: UnitMineralCost;  value: " << it.data() << endl;
      type = MineralCost;
    }
    else if(it.key() == "UnitOilCost")
    {
      boDebug(600) << k_funcinfo << "Found entry: UnitOilCost;  value: " << it.data() << endl;
      type = OilCost;
    }
    else if(it.key() == "SightRange")
    {
      boDebug(600) << k_funcinfo << "Found entry: SightRange;  value: " << it.data() << endl;
      type = SightRange;
    }
    else if(it.key() == "UnitProductionTime")
    {
      boDebug(600) << k_funcinfo << "Found entry: UnitProductionTime;  value: " << it.data() << endl;
      type = ProductionTime;
    }
    else if(it.key() == "Speed")
    {
      boDebug(600) << k_funcinfo << "Found entry: Speed;  value: " << it.data() << endl;
      type = Speed;
    }
    else if(it.key().left(7) == "Weapon_")
    {
      boDebug(600) << k_funcinfo << "Found weapon entry: " << it.key() << "=" << it.data() << "; parsing..." << endl;
      // Find weapon id
      QString str = it.key().right(it.key().length() - 7);
      int i = str.find(':');
      boDebug(600) << "        " << k_funcinfo << "str: " << str << "; i: " << i << endl;
      if(i < 1)
      {
        boError(600) << k_funcinfo << "Invalid weapon key: " << it.key() << endl;
        continue;
      }
      weaponid = str.left(i).toInt();
      boDebug(600) << "    " << k_funcinfo << "Weapon id as a string: \"" << str.left(i) << "\", as int: "
          << weaponid << endl;
      // Extract "weapon key", e.g. Damage, Range etc. from key
      str = str.right(str.length() - i - 1);  // -1 is ":"
      boDebug(600) << "        " << k_funcinfo << "weapon key: " << str << endl;
      if(str == "Damage")
      {
        boDebug(600) << k_funcinfo << "Found entry: WeaponDamage;  value: " << str << endl;
        type = WeaponDamage;
      }
      else if(str == "DamageRange")
      {
        boDebug(600) << k_funcinfo << "Found entry: WeaponDamageRange;  value: " << str << endl;
        type = WeaponDamageRange;
      }
      else if(str == "FullDamageRange")
      {
        boDebug(600) << k_funcinfo << "Found entry: WeaponFullDamageRange;  value: " << str << endl;
        type = WeaponFullDamageRange;
      }
      else if(str == "Range")
      {
        boDebug(600) << k_funcinfo << "Found entry: WeaponRange;  value: " << str << endl;
        type = WeaponRange;
      }
      else if(str == "Reload")
      {
        boDebug(600) << k_funcinfo << "Found entry: WeaponReload;  value: " << str << endl;
        type = WeaponReload;
      }
      else if(str == "Speed")
      {
        boDebug(600) << k_funcinfo << "Found entry: WeaponSpeed;  value: " << str << endl;
        type = WeaponSpeed;
      }
      else
      {
        boWarning(600) << k_funcinfo << "Unrecogniced weapon key: \"" << str << "\", skipping" << endl;
        continue;
      }
    }
    else
    {
      boWarning(600) << k_funcinfo << "Unrecogniced key: \"" << it.key() << "\", skipping" << endl;
      continue;
    }
    applyProperty(&list, player, it.data(), type, weaponid);
  }
}

void UpgradeProperties::applyProperty(QValueList<unsigned long int>* typeIds,
    Player* player, const QString& data, UpgradeType type, int weaponid) const
{
  // Note that I don't use k_funcinfo here, because I get _very_ long lines with it
  boDebug(600) << "    " << "[UpgradeProperties::applyProperty(...)] " << "Applying property (type: " << type << ") to " << typeIds->count() << " properites. weaponid: " << weaponid << endl;
  QValueList<unsigned long int>::Iterator it;
  unsigned long int oldvalueuint = 0;
  bofixed oldvaluef = 0.0f;
  for(it = typeIds->begin(); it != typeIds->end(); it++)
  {
    UnitProperties* prop = player->speciesTheme()->nonConstUnitProperties(*it);
    boDebug(600) << "        " << "[UpgradeProperties::applyProperty(...)]" << "Applying to prop with id: " << prop->typeId() << "; name: " << prop->name() << endl;
    if(weaponid == -1)
    {
      // Not a weapon upgrade
      switch(type)
      {
        case Health:
        {
          oldvalueuint = prop->health();
          prop->setHealth(applyValue(data, oldvalueuint));
          applyPropertyToUnits((bofixed)oldvalueuint, *it, player, type);
          break;
        }
        case Armor:
        {
          oldvalueuint = prop->armor();
          prop->setArmor(applyValue(data, oldvalueuint));
          applyPropertyToUnits((bofixed)oldvalueuint, *it, player, type);
          break;
        }
        case Shields:
        {
          oldvalueuint = prop->shields();
          prop->setShields(applyValue(data, oldvalueuint));
          applyPropertyToUnits((bofixed)oldvalueuint, *it, player, type);
          break;
        }
        case MineralCost:
        {
          oldvalueuint = prop->mineralCost();
          prop->setMineralCost(applyValue(data, oldvalueuint));
          break;
        }
        case OilCost:
        {
          oldvalueuint = prop->oilCost();
          prop->setOilCost(applyValue(data, oldvalueuint));
          break;
        }
        case SightRange:
        {
          oldvalueuint = prop->sightRange();
          prop->setSightRange(applyValue(data, oldvalueuint));
          applyPropertyToUnits((bofixed)oldvalueuint, *it, player, type);
          break;
        }
        case ProductionTime:
        {
          oldvalueuint = prop->productionTime();
          prop->setProductionTime(applyValue(data, oldvalueuint));
          break;
        }
        case Speed:
        {
          oldvaluef = prop->speed();
          prop->setSpeed(applyValue(data, oldvaluef));
        }
        default:
        {
          oldvalueuint = 0;
          oldvaluef = 0.0f;
          boError(600) << "[UpgradeProperties::applyProperty(...)]" << "Invalid UpgradeType: " << type << endl;
          break;
        }
      }
    }
    else
    {
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
        case WeaponDamage:
        {
          oldvalueuint = wep->damage();
          wep->setDamage(applyValue(data, oldvalueuint));
          break;
        }
        case WeaponDamageRange:
        {
          oldvaluef = wep->damageRange();
          wep->setDamageRange(applyValue(data, oldvalueuint));
          break;
        }
        case WeaponFullDamageRange:
        {
          oldvaluef = wep->fullDamageRange();
          wep->setFullDamageRange(applyValue(data, oldvalueuint));
          break;
        }
        case WeaponReload:
        {
          oldvalueuint = wep->reloadingTime();
          wep->setReloadingTime(applyValue(data, oldvalueuint));
          break;
        }
        case WeaponRange:
        {
          oldvalueuint = wep->range();
          wep->setRange(applyValue(data, oldvalueuint));
          break;
        }
        case WeaponSpeed:
        {
          oldvaluef = wep->speed();
          wep->setSpeed(applyValue(data, oldvaluef));
          break;
        }
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

void UpgradeProperties::applyPropertyToUnits(bofixed oldvalue,
    unsigned long int typeId, Player* player, UpgradeType type) const
{
  boDebug(600) << "          " << "[UpgradeProperties::applyPropertyToUnits(...)]" <<
      "PARAMS: oldvalue: " << oldvalue << "; typeId: " << typeId <<
      "; player: " << player << "; type: " << type << endl;
  //boDebug(600) << "      " << k_funcinfo << "Starting to apply" << endl;
  QPtrListIterator<Unit> it(*(player->allUnits()));
  //boDebug(600) << "        " << k_funcinfo << "Unit count: " << it.count() << endl;
  Unit* u;
  while(it.current())
  {
    u = it.current();
    if(u->type() == typeId)
    {
      if(type == Health)
      {
        // Health is special: we have to take old health into account as well
        u->setHealth((unsigned long int)((u->health() / oldvalue) * u->unitProperties()->health()));
      }
      else if(type == Shields)
      {
        // Shields are also quite special, but not that special, so we use simpler approach here
        u->setShields((unsigned long int)(u->unitProperties()->shields() - (oldvalue - u->shields())));
      }
      else if(type == Armor)
      {
        // New value will just replace the old one
        u->setArmor(u->unitProperties()->armor());
      }
      else if(type == SightRange)
      {
        u->setSightRange(u->unitProperties()->sightRange());
        // Update unit's sight
        u->canvas()->updateSight(u, u->x(), u->y());
      }
      else
      {
        boError(600) << "[UpgradeProperties::applyPropertyToUnits(...)]" << "Invalid UpgradeType: " << type << endl;
        return;
      }
    }
    ++it;
  }
}

QValueList<unsigned long int> UpgradeProperties::requiredUnits() const
{
  return d->mRequireUnits;
}


QValueList<unsigned long int> UpgradeProperties::requiredTechnologies() const
{
  return d->mRequireTechnologies;
}


/*
 * vim:et sw=2
 */
