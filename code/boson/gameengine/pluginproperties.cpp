/*
    This file is part of the Boson game
    Copyright (C) 2002 Andreas Beckermann (b_mann@gmx.de)

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

#include "../bomemory/bodummymemory.h"
#include "unitproperties.h"
#include "speciestheme.h"
#include "bodebug.h"
#include "bosonconfig.h"

#include <klocale.h>
#include <ksimpleconfig.h>

PluginProperties::PluginProperties(const UnitProperties* parent)
	: BoBaseValueCollection()
{
 mUnitProperties = parent;
 mEditorObject = 0;
}

PluginProperties::~PluginProperties()
{
 delete mEditorObject;
}

void PluginProperties::setEditorObject(PluginPropertiesEditor* e)
{
 delete mEditorObject;
 mEditorObject = e;
}

SpeciesTheme* PluginProperties::speciesTheme() const
{
 return unitProperties()->theme();
}

const BoUpgradesCollection& PluginProperties::upgradesCollection() const
{
 return unitProperties()->upgradesCollection();
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
	boWarning() << k_funcinfo << "units should't mine minerals *and* oil" << endl;
 }
 mMaxResources = config->readUnsignedNumEntry("MaxResources", 100);
 // Convert speeds from amount/second to amount/adv.call
 mMiningSpeed = (int)(config->readDoubleNumEntry("MiningSpeed", 100) / 20.0f);
 mUnloadingSpeed = (int)(config->readDoubleNumEntry("UnloadingSpeed", 200) / 20.0f);
}

void HarvesterProperties::savePlugin(KSimpleConfig* config)
{
 config->setGroup(propertyGroup());
 config->writeEntry("CanMineMinerals", mCanMineMinerals);
 config->writeEntry("CanMineOil", mCanMineOil);
 config->writeEntry("MaxResources", mMaxResources);
 // Convert speeds from amount/adv.call to amount/second
 config->writeEntry("MiningSpeed", mMiningSpeed * 20.0f);
 config->writeEntry("UnloadingSpeed", mUnloadingSpeed * 20.0f);
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
	boWarning() << k_funcinfo << "unit shouldn't refine minerals *and* oil" << endl;
 }
}

void RefineryProperties::savePlugin(KSimpleConfig* config)
{
 config->setGroup(propertyGroup());
 config->writeEntry("CanRefineMinerals", mCanRefineMinerals);
 config->writeEntry("CanRefineOil", mCanRefineOil);
}


ResourceMineProperties::ResourceMineProperties(const UnitProperties* parent)
	: PluginProperties(parent)
{
 mMinerals = false;
 mOil = false;
}

ResourceMineProperties::~ResourceMineProperties()
{
}

QString ResourceMineProperties::name() const
{
 return i18n("Resource Mine Plugin");
}

QString ResourceMineProperties::propertyGroup()
{
 return QString::fromLatin1("ResourceMinePlugin");
}

void ResourceMineProperties::loadPlugin(KSimpleConfig* config)
{
 if (!config->hasGroup(propertyGroup())) {
	boError() << k_funcinfo << "unit has no harvester plugin" << endl;
	return;
 }
 config->setGroup(propertyGroup());
 mMinerals = config->readBoolEntry("CanProvideMinerals", false);
 mOil = config->readBoolEntry("CanProvideOil", false);
}

void ResourceMineProperties::savePlugin(KSimpleConfig* config)
{
 if (!config->hasGroup(propertyGroup())) {
	boError() << k_funcinfo << "unit has no harvester plugin" << endl;
	return;
 }
 config->writeEntry("CanProvideMinerals", mMinerals);
 config->writeEntry("CanProvideOil", mOil);
}

AmmunitionStorageProperties::AmmunitionStorageProperties(const UnitProperties* parent)
		: PluginProperties(parent)
{
}

AmmunitionStorageProperties::~AmmunitionStorageProperties()
{
}

QString AmmunitionStorageProperties::propertyGroup()
{
 return QString::fromLatin1("AmmunitionStoragePlugin");
}

QString AmmunitionStorageProperties::name() const
{
 return i18n("Ammunition Storage Plugin");
}

void AmmunitionStorageProperties::loadPlugin(KSimpleConfig* config)
{
 if (!config->hasGroup(propertyGroup())) {
	boError() << k_funcinfo << "unit has no production plugin" << endl;
	return;
 }
 config->setGroup(propertyGroup());
 mCanStore = config->readListEntry("CanStore");
 mMustBePickedUp = config->readListEntry("MustBePickedUp");
}

void AmmunitionStorageProperties::savePlugin(KSimpleConfig* config)
{
 config->setGroup(propertyGroup());
 config->writeEntry("CanStore", mCanStore);
 config->writeEntry("MustBePickedUp", mMustBePickedUp);
}

bool AmmunitionStorageProperties::mustBePickedUp(const QString& type) const
{
 if (mMustBePickedUp.contains(type)) {
	return true;
 }
 return false;
}

bool AmmunitionStorageProperties::canStore(const QString& type) const
{
 if (mCanStore.contains(type)) {
	return true;
 }
 return false;
}


RadarProperties::RadarProperties(const UnitProperties* parent)
		: PluginProperties(parent)
{
}

RadarProperties::~RadarProperties()
{
}

QString RadarProperties::propertyGroup()
{
 return QString::fromLatin1("RadarPlugin");
}

QString RadarProperties::name() const
{
 return i18n("Radar Plugin");
}

void RadarProperties::loadPlugin(KSimpleConfig* config)
{
 if (!config->hasGroup(propertyGroup())) {
	boError() << k_funcinfo << "unit has no production plugin" << endl;
	return;
 }
 config->setGroup(propertyGroup());
 mTransmittedPower = config->readDoubleNumEntry("TransmittedPower", 10000.0f);
 mMinReceivedPower = config->readDoubleNumEntry("MinReceivedPower", 0.05f);
 mDetectsLandUnits = config->readBoolEntry("DetectsLandUnits", true);
 mDetectsAirUnits = config->readBoolEntry("DetectsAirUnits", true);
}

void RadarProperties::savePlugin(KSimpleConfig* config)
{
 config->setGroup(propertyGroup());
 config->writeEntry("TransmittedPower", mTransmittedPower);
 config->writeEntry("MinReceivedPower", mMinReceivedPower);
}


RadarJammerProperties::RadarJammerProperties(const UnitProperties* parent)
		: PluginProperties(parent)
{
}

RadarJammerProperties::~RadarJammerProperties()
{
}

QString RadarJammerProperties::propertyGroup()
{
 return QString::fromLatin1("RadarJammerPlugin");
}

QString RadarJammerProperties::name() const
{
 return i18n("Radar Jammer Plugin");
}

void RadarJammerProperties::loadPlugin(KSimpleConfig* config)
{
 if (!config->hasGroup(propertyGroup())) {
	boError() << k_funcinfo << "unit has no production plugin" << endl;
	return;
 }
 config->setGroup(propertyGroup());
 mTransmittedPower = config->readDoubleNumEntry("TransmittedPower", 500.0f);
}

void RadarJammerProperties::savePlugin(KSimpleConfig* config)
{
 config->setGroup(propertyGroup());
 config->writeEntry("TransmittedPower", mTransmittedPower);
}

