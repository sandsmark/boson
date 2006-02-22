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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "upgradeproperties.h"

#include "../bomemory/bodummymemory.h"
#include "player.h"
#include "unitproperties.h"
#include "speciestheme.h"
#include "unit.h"
#include "bodebug.h"
#include "bosonconfig.h"
#include "bosonweapon.h"
#include "bosoncanvas.h"

#include <ksimpleconfig.h>
#include <klocale.h>

#include <qstring.h>
#include <qvaluelist.h>

class UpgradeApplyer
{
  public:
    UpgradeApplyer(const UpgradeProperties* prop)
    {
      mProp = prop;
    }

    // AB: valid types for T currently: unsigned long int, bofixed
    // note that only numerical values are valid!
    template<class T> bool upgradeValue(const QString& data, T* value, const QString& type) const;

  protected:
    unsigned long int applyValue(const QString& data, unsigned long int oldvalue) const;
    bofixed applyValue(const QString& data, bofixed oldvalue) const;

    bool parseEntryType(const QString& typeString, UpgradeProperties::UpgradeType* type, int* weaponid) const;
    void parseEntry(const QString& entry, UpgradeProperties::ValueType& type, QString& value) const;

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
    template<class T> T applyValueInternal(UpgradeProperties::ValueType type, T oldvalue, T value) const;

  private:
    const UpgradeProperties* mProp;
};


class UpgradePropertiesPrivate
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

UpgradeProperties::UpgradeProperties(const QString& type, const SpeciesTheme* theme)
{
  d = new UpgradePropertiesPrivate;

  mType = type;
  mId = 0;
  mMineralCost = 0;
  mOilCost = 0;
  mProducer = 0;
  mProductionTime = 0;
  mApplyToFacilities = false;
  mApplyToMobiles = false;
}

UpgradeProperties::~UpgradeProperties()
{
  delete d;
}

bool UpgradeProperties::appliesTo(const UnitProperties* prop) const
{
  if(!prop)
  {
    BO_NULL_ERROR(prop);
    return false;
  }
  if(mApplyToFacilities && prop->isFacility())
  {
    return true;
  }
  if(mApplyToMobiles && prop->isMobile())
  {
    return true;
  }
  if(d->mApplyToTypes.contains(prop->typeId()))
  {
    return true;
  }
  return false;
}

bool UpgradeProperties::appliesTo(const UnitBase* unit) const
{
  if(!unit)
  {
    BO_NULL_ERROR(unit);
    return false;
  }
  return appliesTo(unit->unitProperties());
}

bool UpgradeProperties::appliesTo(const BosonWeaponProperties* prop) const
{
  if(!prop)
  {
    BO_NULL_ERROR(prop);
    return false;
  }
  return appliesTo(prop->unitProperties());
}

bool UpgradeProperties::appliesTo(const BosonWeapon* weapon) const
{
  if(!weapon)
  {
    BO_NULL_ERROR(weapon);
    return false;
  }
  return weapon->properties();
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
  mDescription = cfg->readEntry("Description");
  mMineralCost = cfg->readUnsignedLongNumEntry("MineralCost", 0);
  mOilCost = cfg->readUnsignedLongNumEntry("OilCost", 0);
  mProducer = cfg->readUnsignedNumEntry("Producer", 0);
  mProductionTime = (int)(cfg->readDoubleNumEntry("ProductionTime", 5) * 20.0f);
  mProduceActionString = cfg->readEntry("ProduceAction", "");
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

  convertEntries();

  return true;
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

bool UpgradeProperties::upgradeUnit(UnitBase* unit) const
{
  if(!appliesTo(unit))
  {
    return true;
  }

#warning TODO

  return true;
}

bool UpgradeProperties::downgradeUnit(UnitBase* unit) const
{
  if(!appliesTo(unit))
  {
    return true;
  }

#warning TODO

  return true;
}

bool UpgradeProperties::upgradeValue(const QString& name, unsigned long int* v, const QString& type) const
{
  if(!d->mEntryList.contains(name))
  {
    return true;
  }
  UpgradeApplyer a(this);
  return a.upgradeValue(d->mEntryList[name], v, type);
}

bool UpgradeProperties::upgradeValue(const QString& name, long int* v, const QString& type) const
{
  if(!d->mEntryList.contains(name))
  {
    boDebug() << k_funcinfo << "dont have " << name << endl;
    return true;
  }
  UpgradeApplyer a(this);
  return a.upgradeValue(d->mEntryList[name], v, type);
}

bool UpgradeProperties::upgradeValue(const QString& name, bofixed* v, const QString& type) const
{
  if(!d->mEntryList.contains(name))
  {
    return true;
  }
  UpgradeApplyer a(this);
  return a.upgradeValue(d->mEntryList[name], v, type);
}

void UpgradeProperties::convertEntries()
{
  // Iterate through entries map and convert needed ones.
  QMap<QString, QString>::Iterator it;
  for(it = d->mEntryList.begin(); it != d->mEntryList.end(); it++)
  {
    QString key = it.key();
    // If it's weapon's key, we have to do some preprocessing:
    if(key.left(7) == "Weapon_")
    {
      int i = key.find(':');
      if(i < 1)
      {
        // Invalid entry
        continue;
      }
      // Extract "weapon key", e.g. Damage, Range etc. from key
      key = key.right(key.length() - i - 1);  // -1 is ":"
    }

    // Convert from cells/second to cells/adv.call  (divide by 20)
    if(key == "Speed")
    {
      // Can be both unit's or weapon's key
      it.data() = QString::number(it.data().toFloat() / 20.0f);
    }
    // Convert from seconds to adv.calls  (multiply by 20)
    else if(key == "UnitProductionTime")
    {
      it.data() = QString::number(it.data().toFloat() * 20.0f);
    }
    else if(key == "Reload")
    {
      it.data() = QString::number(it.data().toFloat() * 20.0f);
    }
  }
}

unsigned long int UpgradeApplyer::applyValue(const QString& data, unsigned long int oldvalue) const
{
  UpgradeProperties::ValueType type;
  QString valuestr;
  parseEntry(data, type, valuestr);
  unsigned long int value = valuestr.toULong();
  return applyValueInternal(type, oldvalue, value);
}

bofixed UpgradeApplyer::applyValue(const QString& data, bofixed oldvalue) const
{
  UpgradeProperties::ValueType type;
  QString valuestr;
  parseEntry(data, type, valuestr);
  bofixed value = valuestr.toFloat();
  return applyValueInternal(type, oldvalue, value);
}

template<class T> T UpgradeApplyer::applyValueInternal(UpgradeProperties::ValueType type, T oldvalue, T value) const
{
  if(type == UpgradeProperties::Absolute)
  {
    return value;
  }
  else if(type == UpgradeProperties::Relative)
  {
    return oldvalue + value;
  }
  else if(type == UpgradeProperties::Percent)
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

#if 0
bool UpgradeApplyer::parseEntryType(const QString& typeString, UpgradeProperties::UpgradeType* type, int* weaponid) const
{
  *weaponid = -1;
  if(typeString == "Health")
  {
    boDebug(600) << k_funcinfo << "Found entry: Health" << endl;
    *type = UpgradeProperties::Health;
  }
  else if(typeString == "Armor")
  {
    boDebug(600) << k_funcinfo << "Found entry: Armor" << endl;
    *type = UpgradeProperties::Armor;
  }
  else if(typeString == "Shields")
  {
    boDebug(600) << k_funcinfo << "Found entry: Shields" << endl;
    *type = UpgradeProperties::Shields;
  }
  else if(typeString == "UnitMineralCost")
  {
    boDebug(600) << k_funcinfo << "Found entry: UnitMineralCost" << endl;
    *type = UpgradeProperties::MineralCost;
  }
  else if(typeString == "UnitOilCost")
  {
    boDebug(600) << k_funcinfo << "Found entry: UnitOilCost" << endl;
    *type = UpgradeProperties::OilCost;
  }
  else if(typeString == "SightRange")
  {
    boDebug(600) << k_funcinfo << "Found entry: SightRange" << endl;
    *type = UpgradeProperties::SightRange;
  }
  else if(typeString == "UnitProductionTime")
  {
    boDebug(600) << k_funcinfo << "Found entry: UnitProductionTime" << endl;
    *type = UpgradeProperties::ProductionTime;
  }
  else if(typeString == "Speed")
  {
    boDebug(600) << k_funcinfo << "Found entry: Speed" << endl;
    *type = UpgradeProperties::Speed;
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
      *type = UpgradeProperties::WeaponDamage;
    }
    else if(str == "DamageRange")
    {
      boDebug(600) << k_funcinfo << "Found entry: WeaponDamageRange" << endl;
      *type = UpgradeProperties::WeaponDamageRange;
    }
    else if(str == "FullDamageRange")
    {
      boDebug(600) << k_funcinfo << "Found entry: WeaponFullDamageRange" << endl;
      *type = UpgradeProperties::WeaponFullDamageRange;
    }
    else if(str == "Range")
    {
      boDebug(600) << k_funcinfo << "Found entry: WeaponRange" << endl;
      *type = UpgradeProperties::WeaponRange;
    }
    else if(str == "Reload")
    {
      boDebug(600) << k_funcinfo << "Found entry: WeaponReload" << endl;
      *type = UpgradeProperties::WeaponReload;
    }
    else if(str == "Speed")
    {
      boDebug(600) << k_funcinfo << "Found entry: WeaponSpeed" << endl;
      *type = UpgradeProperties::WeaponSpeed;
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
#endif

void UpgradeApplyer::parseEntry(const QString& entry, UpgradeProperties::ValueType& type, QString& value) const
{
  if(entry.left(1) == "+" || entry.left(1) == "-")
  {
    type = UpgradeProperties::Relative;
    value = entry;
  }
  else if(entry.right(1) == "%")
  {
    type = UpgradeProperties::Percent;
    value = entry.left(entry.length() - 1);
  }
  else
  {
    type = UpgradeProperties::Absolute;
    value = entry;
  }
}

template<class T> bool UpgradeApplyer::upgradeValue(const QString& data, T* value, const QString& type) const
{
  if(data.isEmpty())
  {
    boError(600) << k_funcinfo << "empty data string" << endl;
    return false;
  }
  if(type == "MaxValue")
  {
    *value = applyValue(data, *value);
    return true;
  }
  else if(type == "MinValue")
  {
    boError(600) << k_funcinfo << "MinValue not yet supported" << endl;
    return false;
  }
  else
  {
    boError(600) << k_funcinfo << "invalid type " << type << endl;
    return false;
  }
  return false;
}




/*
 * vim:et sw=2
 */
