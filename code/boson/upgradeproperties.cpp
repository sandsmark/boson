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

#include <ksimpleconfig.h>
#include <klocale.h>

#include <qstring.h>
#include <qvaluelist.h>


/**********  UpgradePropertiesBase  **********/
class UpgradePropertiesBase::UpgradePropertiesBasePrivate
{
public:
  UpgradePropertiesBasePrivate()
  {
  }

  QValueList<unsigned long int> mRequireUnits;
  QValueList<unsigned long int> mRequireTechnologies;
  QValueList<unsigned long int> mApplyToTypes;
};

UpgradePropertiesBase::UpgradePropertiesBase(bool isTechnology, unsigned long int id)
{
  d = new UpgradePropertiesBasePrivate;
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
  delete d;
}

bool UpgradePropertiesBase::canBeResearched(Player* player)
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
    mMineralCost = cfg->readUnsignedLongNumEntry("MineralCost", 0);
    mOilCost = cfg->readUnsignedLongNumEntry("OilCost", 0);
    mProducer = cfg->readUnsignedNumEntry("Producer", 0);
    mProductionTime = cfg->readUnsignedNumEntry("ProductionTime", 100);
    mPixmapName = cfg->readEntry("Pixmap", "none.png");
    d->mRequireUnits = BosonConfig::readUnsignedLongNumList(cfg, "RequireUnits");
    d->mApplyToTypes = BosonConfig::readUnsignedLongNumList(cfg, "ApplyToTypes");
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
  d->mRequireTechnologies = BosonConfig::readUnsignedLongNumList(cfg, "RequireTechnologies");

  boDebug() << "    " << k_funcinfo << "Loading upgrade properties" << endl;
  // Load upgrade properties
  mHealth.loadValue(cfg, "Health");
  mArmor.loadValue(cfg, "Armor");
  mShields.loadValue(cfg, "Shields");
  mUnitMineralCost.loadValue(cfg, "UnitMineralCost");
  mUnitOilCost.loadValue(cfg, "UnitOilCost");
  mSightRange.loadValue(cfg, "SightRange");
  mUnitProductionTime.loadValue(cfg, "UnitProductionTime");
  mSpeed.loadValue(cfg, "Speed");
}


void UpgradePropertiesBase::apply(Player* player)
{
  boDebug() << k_funcinfo << "isTech: " << isTechnology() << "; id: " << id() << endl;
  if(!isResearched())
  {
    boError() << k_funcinfo << "Trying to apply non-researched upgrade" << endl;
  }

  // Add unit types to list
  QValueList<unsigned long int> list = d->mApplyToTypes;
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
  QValueList<unsigned long int>::Iterator it;
  QString addingTo = "Adding to types:";
  for(it = list.begin(); it != list.end(); it++)
  {
    addingTo += " ";
    addingTo += QString::number(*it);
  }
  boDebug() << "  " << k_funcinfo << addingTo << endl;
  // TODO: check for double typeIDs

  mHealth.applyProperty(&list, player, Health);
  mArmor.applyProperty(&list, player, Armor);
  mShields.applyProperty(&list, player, Shields);
  mUnitMineralCost.applyProperty(&list, player, MineralCost);
  mUnitOilCost.applyProperty(&list, player, OilCost);
  mSightRange.applyProperty(&list, player, SightRange);
  mUnitProductionTime.applyProperty(&list, player, ProductionTime);
  mSpeed.applyProperty(&list, player, Speed);
}


QValueList<unsigned long int> UpgradePropertiesBase::requiredUnits() const
{
  return d->mRequireUnits;
}


QValueList<unsigned long int> UpgradePropertiesBase::requiredTechnologies() const
{
  return d->mRequireTechnologies;
}

/**********  UpgradeProperties*Value  **********/

template<class TYPE> QString UpgradePropertiesValue<TYPE>::loadBaseValue(KSimpleConfig* cfg, QString key)
{
  QString str;
  specified = cfg->hasKey(key);
  if(specified)
  {
    str = cfg->readEntry(key, "0");
    boDebug() << "          " << k_funcinfo << "Loaded string: '" << str << "' (key was: " << key << ")" << endl;
    if(str.left(1) == "+" || str.left(1) == "-")
    {
      type = Relative;
    }
    else if(str.right(1) == "%")
    {
      type = Percent;
      str = str.left(str.length() - 1);
    }
    else
    {
      type = Absolute;
    }
  }
  return str;
}

void UpgradePropertiesUIntValue::loadValue(KSimpleConfig* cfg, QString key)
{
  QString str = loadBaseValue(cfg, key);
  boDebug() << "      " << k_funcinfo << "Key: " << key << "; was specified: " << specified << endl;
  if(specified)
  {
    value = str.toULong();
    boDebug() << "        " << k_funcinfo << "Value as string: " << str << "; converted: " << value << "; type: " << type << endl;
  }
}

void UpgradePropertiesFloatValue::loadValue(KSimpleConfig* cfg, QString key)
{
  QString str = loadBaseValue(cfg, key);
  boDebug() << k_funcinfo << "Key: " << key << "; was specified: " << specified << endl;
  if(specified)
  {
    value = str.toFloat();
    boDebug() << "    " << k_funcinfo << "Value as string: " << str << "; converted: " << value << "; type: " << type << endl;
  }
}

template<class TYPE> void UpgradePropertiesValue<TYPE>::applyProperty(QValueList<unsigned long int>* typeIds,
    Player* player, UpgradeType type)
{
  if(!specified)
  {
    return;
  }
  boDebug() << "  " << k_funcinfo << "Applying property (type: " << type << ") to " << typeIds->count() << " props" << endl;
  QValueList<unsigned long int>::Iterator it;
#warning oldvalue might be used initialized. FIX THIS !!
  TYPE oldvalue;
  for(it = typeIds->begin(); it != typeIds->end(); it++)
  {
    boDebug() << "    " << k_funcinfo << "Applying to prop with id " << *it << endl;
    UnitProperties* prop = player->speciesTheme()->nonConstUnitProperties(*it);
    boDebug() << "      " << k_funcinfo << "Unit id: " << prop->typeId() << "; name: " << prop->name() << endl;
    switch(type)
    {
      case Health:
      {
        oldvalue = prop->health();
        boDebug() << "        " << k_funcinfo << "Applying health; old value: " << oldvalue << endl;
        prop->setHealth(applyValue(oldvalue));
        boDebug() << "        " << k_funcinfo << "New value: " << prop->health() << endl;
        break;
      }
      case Armor:
      {
        oldvalue = prop->armor();
        prop->setArmor(applyValue(oldvalue));
        break;
      }
      case Shields:
      {
        oldvalue = prop->shields();
        prop->setShields(applyValue(oldvalue));
        break;
      }
      case MineralCost:
      {
        prop->setMineralCost(applyValue(prop->mineralCost()));
        break;
      }
      case OilCost:
      {
        prop->setOilCost(applyValue(prop->oilCost()));
        break;
      }
      case SightRange:
      {
        oldvalue = prop->sightRange();
        prop->setSightRange(applyValue(oldvalue));
        break;
      }
      case ProductionTime:
      {
        prop->setProductionTime(applyValue(prop->productionTime()));
        break;
      }
      case Speed:
      {
        prop->setSpeed(applyValue(prop->speed()));
        break;
      }
    }
    // Apply to already existing units
    boDebug() << "    " << k_funcinfo << "Applying to existing units with typeid " << *it << endl;
    applyPropertyToUnits(oldvalue, *it, player, type);
  }
}

template<class TYPE> void UpgradePropertiesValue<TYPE>::applyPropertyToUnits(TYPE oldvalue,
    unsigned long int typeId, Player* player, UpgradeType type)
{
  boDebug() << "      " << k_funcinfo << "PARAMS: oldvalue: " << oldvalue << "; typeId: " << typeId <<
      "; player: " << player << "; type: " << type << endl;
  // First check if this upgrade should applied to already existing units
  switch(type)
  {
    // These must be updated:
    case Health:
    case Armor:
    case Shields:
    case SightRange:
    {
      break;
    }
    default:
    {
      // Other types are not stored in UnitBase nor Unit
      return;
    }
  }
  boDebug() << "      " << k_funcinfo << "Starting to apply" << endl;
  QPtrListIterator<Unit> it(*(player->allUnits()));
  boDebug() << "        " << k_funcinfo << "Unit count: " << it.count() << endl;
  Unit* u;
  while(it.current())
  {
    u = it.current();
    if(u->type() == typeId)
    {
      if(type == Health)
      {
        // Health is special: we have to take old health into account as well
        //int oldhealth = u->health();
        u->setHealth((u->health() / (float)oldvalue) * u->unitProperties()->health());
        /*boDebug() << "        " << k_funcinfo << "Applying to unit with id " << u->id() <<
            "; old: " << oldhealth << "; old max: " << oldvalue << "; new max: " <<
            u->unitProperties()->health() << "; new health: " << u->health() << endl;*/
      }
      else if(type == Shields)
      {
        // Shields are also quite special, but not that special, so we use simpler approach here
        u->setShields(u->unitProperties()->shields() - (oldvalue - u->shields()));
      }
      else if(type == Armor)
      {
        // New value will just replace the old one
        u->setArmor(u->unitProperties()->armor());
      }
      else if(type == SightRange)
      {
        u->setSightRange(u->unitProperties()->sightRange());
      }
    }
    ++it;
  }
}

template<class TYPE> TYPE UpgradePropertiesValue<TYPE>::applyValue(TYPE applyTo)
{
  if(!specified)
  {
    boError() << "    " << k_funcinfo << "Shouldn't call applyValue() when value isn't specified!!!" << endl;
    return applyTo;
  }
  TYPE newvalue;
  if(type == Absolute)
  {
    newvalue = value;
  }
  else if(type == Relative)
  {
    newvalue = applyTo + value;
  }
  else if(type == Percent)
  {
    newvalue = (TYPE)(((float)value) * applyTo / 100.0);
  }
  else
  {
    // Shouldn't happen
    boError() << k_funcinfo << "Invalid type: " << type << endl;
    newvalue = applyTo;
  }
  boDebug() << "        " << k_funcinfo << "Applied " << value << " (type " << type << ") to " << applyTo << "; result: " << newvalue << endl;
  return newvalue;
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
  d->mApplyToTypes.append(unitProperties()->typeId());
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


/*
 * vim:et sw=2
 */
