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

#include <klocale.h>
#include <ksimpleconfig.h>
#include <kdebug.h>

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
	kdError() << k_funcinfo << "unit has no repair plugin" << endl;
	return;
 }
 config->setGroup(propertyGroup());
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
	kdError() << k_funcinfo << "unit has no production plugin" << endl;
	return;
 }
 config->setGroup(propertyGroup());
 mProducerList = config->readIntListEntry("ProducerList");
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
	kdError() << k_funcinfo << "unit has no harvester plugin" << endl;
	return;
 }
 config->setGroup(propertyGroup());
 config->setGroup("HarvesterPlugin");
 mCanMineMinerals = config->readBoolEntry("CanMineMinerals", false);
 mCanMineOil = config->readBoolEntry("CanMineOil", false);
 if (mCanMineMinerals && mCanMineOil) {
	kdWarning() << k_funcinfo << "units can't mine minerals *and* oil" << endl;
	mCanMineOil = false;
 }
 mMaxResources= config->readUnsignedNumEntry("MaxResources", 100);
}

