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

#include "pluginproperties.h"

#include "unitproperties.h"
#include "speciestheme.h"
#include "bodebug.h"
#include "bosonconfig.h"

#include <klocale.h>
#include <ksimpleconfig.h>

PluginProperties::PluginProperties(const UnitProperties* parent)
{
 mUnitProperties = parent;
}

PluginProperties::~PluginProperties()
{
}

SpeciesTheme* PluginProperties::speciesTheme() const
{
 return unitProperties()->theme();
}


RepairProperties::RepairProperties(const UnitProperties* parent)
		: PluginProperties(parent)
{
}

RepairProperties::~RepairProperties()
{
}

QString RepairProperties::propertyGroup()
{
 return QString::fromLatin1("RepairPlugin");
}

QString RepairProperties::name() const
{
 return i18n("Repair Plugin");
}

void RepairProperties::loadPlugin(KSimpleConfig* config)
{
 if (!config->hasGroup(propertyGroup())) {
	boError() << k_funcinfo << "unit has no repair plugin" << endl;
	return;
 }
 config->setGroup(propertyGroup());
}

void RepairProperties::savePlugin(KSimpleConfig* config)
{
 config->setGroup(propertyGroup());
 // KConfig automatically deletes empty groups
 config->writeEntry("Dummy", "This entry is here just to prevent KConfig from deleting this group");
}

ProductionProperties::ProductionProperties(const UnitProperties* parent)
		: PluginProperties(parent)
{
}

ProductionProperties::~ProductionProperties()
{
}

QString ProductionProperties::propertyGroup()
{
 return QString::fromLatin1("ProductionPlugin");
}

QString ProductionProperties::name() const
{
 return i18n("Production Plugin");
}

void ProductionProperties::loadPlugin(KSimpleConfig* config)
{
 if (!config->hasGroup(propertyGroup())) {
	boError() << k_funcinfo << "unit has no production plugin" << endl;
	return;
 }
 config->setGroup(propertyGroup());
 mProducerList = BosonConfig::readUnsignedLongNumList(config, "ProducerList");
}

void ProductionProperties::savePlugin(KSimpleConfig* config)
{
 config->setGroup(propertyGroup());
 BosonConfig::writeUnsignedLongNumList(config, "ProducerList", mProducerList);
}


HarvesterProperties::HarvesterProperties(const UnitProperties* parent)
		: PluginProperties(parent)
{
 mCanMineMinerals = false;
 mCanMineOil = false;
 mMaxResources = 0;
}

HarvesterProperties::~HarvesterProperties()
{
}

QString HarvesterProperties::propertyGroup()
{
 return QString::fromLatin1("HarvesterPlugin");
}

QString HarvesterProperties::name() const
{
 return i18n("Harvester Plugin");
}

void HarvesterProperties::loadPlugin(KSimpleConfig* config)
{
 if (!config->hasGroup(propertyGroup())) {
	boError() << k_funcinfo << "unit has no harvester plugin" << endl;
	return;
 }
 config->setGroup(propertyGroup());
 mCanMineMinerals = config->readBoolEntry("CanMineMinerals", false);
 mCanMineOil = config->readBoolEntry("CanMineOil", false);
 if (mCanMineMinerals && mCanMineOil) {
	boWarning() << k_funcinfo << "units can't mine minerals *and* oil" << endl;
	mCanMineOil = false;
 }
 mMaxResources = config->readUnsignedNumEntry("MaxResources", 100);
 mMiningSpeed = config->readUnsignedNumEntry("MiningSpeed", 10);
 mUnloadingSpeed = config->readUnsignedNumEntry("UnloadingSpeed", 10);
}

void HarvesterProperties::savePlugin(KSimpleConfig* config)
{
 config->setGroup(propertyGroup());
 config->writeEntry("CanMineMinerals", mCanMineMinerals);
 config->writeEntry("CanMineOil", mCanMineOil);
 config->writeEntry("MaxResources", mMaxResources);
 config->writeEntry("MiningSpeed", mMiningSpeed);
 config->writeEntry("UnloadingSpeed", mUnloadingSpeed);
}


RefineryProperties::RefineryProperties(const UnitProperties* parent)
		: PluginProperties(parent)
{
 mCanRefineMinerals = false;
 mCanRefineOil = false;
}

RefineryProperties::~RefineryProperties()
{
}

QString RefineryProperties::propertyGroup()
{
 return QString::fromLatin1("RefineryPlugin");
}

QString RefineryProperties::name() const
{
 return i18n("Refinery Plugin");
}

void RefineryProperties::loadPlugin(KSimpleConfig* config)
{
 if (!config->hasGroup(propertyGroup())) {
	boError() << k_funcinfo << "unit has no refinery plugin" << endl;
	return;
 }
 config->setGroup(propertyGroup());
 mCanRefineMinerals = config->readBoolEntry("CanRefineMinerals", false);
 mCanRefineOil = config->readBoolEntry("CanRefineOil", false);
 if (mCanRefineMinerals && mCanRefineOil) {
	boWarning() << k_funcinfo << "unit can't refine minerals *and* oil" << endl;
	mCanRefineOil = false;
 }
}

void RefineryProperties::savePlugin(KSimpleConfig* config)
{
 config->setGroup(propertyGroup());
 config->writeEntry("CanRefineMinerals", mCanRefineMinerals);
 config->writeEntry("CanRefineOil", mCanRefineOil);
}


RessourceMineProperties::RessourceMineProperties(const UnitProperties* parent)
	: PluginProperties(parent)
{
 mMinerals = false;
 mOil = false;
}

RessourceMineProperties::~RessourceMineProperties()
{
}

QString RessourceMineProperties::name() const
{
 return i18n("Ressource Mine Plugin");
}

QString RessourceMineProperties::propertyGroup()
{
 return QString::fromLatin1("RessourceMinePlugin");
}

void RessourceMineProperties::loadPlugin(KSimpleConfig* config)
{
 if (!config->hasGroup(propertyGroup())) {
	boError() << k_funcinfo << "unit has no harvester plugin" << endl;
	return;
 }
 config->setGroup(propertyGroup());
 mMinerals = config->readBoolEntry("CanProvideMinerals", false);
 mOil = config->readBoolEntry("CanProvideOil", false);
}

void RessourceMineProperties::savePlugin(KSimpleConfig* config)
{
 if (!config->hasGroup(propertyGroup())) {
	boError() << k_funcinfo << "unit has no harvester plugin" << endl;
	return;
 }
 config->writeEntry("CanProvideMinerals", mMinerals);
 config->writeEntry("CanProvideOil", mOil);
}

