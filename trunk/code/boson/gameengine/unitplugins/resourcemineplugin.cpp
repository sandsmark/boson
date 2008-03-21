/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2002-2006 Rivo Laks (rivolaks@hot.ee)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include "resourcemineplugin.h"

#include "../../bomemory/bodummymemory.h"
#include "harvesterplugin.h"
#include "unit.h"
#include "pluginproperties.h"
#include "bodebug.h"
#include "../bo3dtools.h"

#include <qdom.h>

#include <math.h>

ResourceMinePlugin::ResourceMinePlugin(Unit* unit)
		: UnitPlugin(unit)
{
 unit->registerData(&mMinerals, Unit::IdResourceMineMinerals);
 unit->registerData(&mOil, Unit::IdResourceMineOil);
 mMinerals.setLocal(0);
 mOil.setLocal(0);
}

ResourceMinePlugin::~ResourceMinePlugin()
{
}

bool ResourceMinePlugin::saveAsXML(QDomElement& root) const
{
 // everything is saved using KGameProperty
 Q_UNUSED(root);
 return true;
}

bool ResourceMinePlugin::loadFromXML(const QDomElement& root)
{
 // everything is already loaded using KGameProperty - we do safety checks here
 ResourceMineProperties* prop = (ResourceMineProperties*)properties(PluginProperties::ResourceMine);
 if (!prop) {
	BO_NULL_ERROR(prop);
	return false;
 }
 if (!prop->canProvideMinerals()) {
	if (mMinerals != 0) {
		boWarning() << k_funcinfo << "unit can't have minerals, but minerals not 0" << endl;
	}
	mMinerals = 0;
 }
 if (!prop->canProvideOil()) {
	if (mOil != 0) {
		boWarning() << k_funcinfo << "unit can't have oil, but oil not 0" << endl;
	}
	mOil = 0;
 }
 return true;
}

void ResourceMinePlugin::advance(unsigned int)
{
}

bool ResourceMinePlugin::canProvideMinerals() const
{
 const ResourceMineProperties* prop = (ResourceMineProperties*)unit()->properties(PluginProperties::ResourceMine);
 if (!prop) {
	return false;
 }
 return prop->canProvideMinerals();
}

bool ResourceMinePlugin::canProvideOil() const
{
 const ResourceMineProperties* prop = (ResourceMineProperties*)unit()->properties(PluginProperties::ResourceMine);
 if (!prop) {
	return false;
 }
 return prop->canProvideOil();
}

bool ResourceMinePlugin::isUsableTo(const HarvesterPlugin* harvester) const
{
 if (!harvester) {
	return false;
 }
 if (harvester->canMineMinerals() && canProvideMinerals() && minerals() != 0) {
	return true;
 }
 if (harvester->canMineOil() && canProvideOil() && oil() != 0) {
	return true;
 }
 return false;
}

unsigned int ResourceMinePlugin::mineMinerals(const HarvesterPlugin* harvester)
{
 if (!harvester) {
	BO_NULL_ERROR(harvester);
	return 0;
 }
 if (!canProvideMinerals()) {
	boDebug() << k_funcinfo << "no minerals available. sorry." << endl;
	return 0;
 }
 if (!isUsableTo(harvester)) {
	boDebug() << k_funcinfo << "harvester cannot mine here" << endl;
	return 0;
 }

 int amount = mineStep(harvester, minerals());
 // When minerals() == -1, then mine's mineral supply is infinite, so we won't change it.
 if (minerals() >= 0) {
	setMinerals(minerals() - amount);
 }
 return amount;
}

unsigned int ResourceMinePlugin::mineOil(const HarvesterPlugin* harvester)
{
 if (!harvester) {
	BO_NULL_ERROR(harvester);
	return 0;
 }
 if (!canProvideOil()) {
	boDebug() << k_funcinfo << "no oil available. sorry." << endl;
	return 0;
 }
 if (!isUsableTo(harvester)) {
	boDebug() << k_funcinfo << "harvester cannot mine here" << endl;
	return 0;
 }

 int amount = mineStep(harvester, oil());
 // When oil() == -1, then mine's oil supply is infinite, so we won't change it.
 if (oil() >= 0) {
	setOil(oil() - amount);
 }
 return amount;
}

unsigned int ResourceMinePlugin::mineStep(const HarvesterPlugin* harvester, int resourcesAvailable) const
{
 if (!harvester) {
	BO_NULL_ERROR(harvester);
	return 0;
 }
 int maxCapacity = QMAX((int)harvester->maxResources() - (int)harvester->resourcesMined(), 0);
 int maxAvailable = resourcesAvailable;
 if (resourcesAvailable < 0) {
	// infinite resources
	maxAvailable = maxCapacity;
 }
 int step = harvester->miningSpeed();
 if (step > maxAvailable) {
	step = maxAvailable;
 }
 if (step > maxCapacity) {
	step = maxCapacity;
 }
 return step;
}

void ResourceMinePlugin::setMinerals(int m)
{
 if (!canProvideMinerals()) {
	mMinerals = 0;
	return;
 }
 mMinerals = m;
}

void ResourceMinePlugin::setOil(int o)
{
 if (!canProvideOil()) {
	mOil = 0;
	return;
 }
 mOil = o;
}

int ResourceMinePlugin::minerals() const
{
 return mMinerals;
}

int ResourceMinePlugin::oil() const
{
 return mOil;
}

void ResourceMinePlugin::unitDestroyed(Unit*)
{
}

void ResourceMinePlugin::itemRemoved(BosonItem*)
{
}


