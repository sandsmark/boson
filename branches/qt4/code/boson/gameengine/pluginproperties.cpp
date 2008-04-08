/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)

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

#include "pluginproperties.h"

#include "../bomemory/bodummymemory.h"
#include "unitproperties.h"
#include "speciestheme.h"
#include "bodebug.h"
#include "bosonconfig.h"

#include <klocale.h>
#include <KConfigGroup>
#include <KConfig>
//Added by qt3to4:
#include <Q3ValueList>

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

bool RepairProperties::loadPlugin(const KConfigGroup& config)
{
 if (!config.exists()) {
	boError() << k_funcinfo << "config group does not exist";
	return false;
 }
 return true;
}

bool RepairProperties::savePlugin(KConfig* config)
{
 KConfigGroup group = config->group(propertyGroup());
 group.writeEntry("Dummy", "This entry is here just to prevent KConfig from deleting this group");
 return true;
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

bool ProductionProperties::loadPlugin(const KConfigGroup& config)
{
 if (!config.exists()) {
	boError() << k_funcinfo << "config group does not exist";
	return false;
 }
 mProducerList = BosonConfig::readUnsignedLongNumList(&config, "ProducerList");
 return true;
}

bool ProductionProperties::savePlugin(KConfig* config)
{
 KConfigGroup group = config->group(propertyGroup());
 BosonConfig::writeUnsignedLongNumList(&group, "ProducerList", mProducerList);
 return true;
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

bool HarvesterProperties::loadPlugin(const KConfigGroup& config)
{
 if (!config.exists()) {
	boError() << k_funcinfo << "config group does not exist";
	return false;
 }
 mCanMineMinerals = config.readEntry("CanMineMinerals", false);
 mCanMineOil = config.readEntry("CanMineOil", false);
 if (mCanMineMinerals && mCanMineOil) {
	boWarning() << k_funcinfo << "units should't mine minerals *and* oil" << endl;
 }
 mMaxResources = config.readEntry("MaxResources", (quint32)100);
 // Convert speeds from amount/second to amount/adv.call
 mMiningSpeed = (int)(config.readEntry("MiningSpeed", 100.0) / 20.0f);
 mUnloadingSpeed = (int)(config.readEntry("UnloadingSpeed", 200.0) / 20.0f);
 return true;
}

bool HarvesterProperties::savePlugin(KConfig* config)
{
 KConfigGroup group = config->group(propertyGroup());
 group.writeEntry("CanMineMinerals", mCanMineMinerals);
 group.writeEntry("CanMineOil", mCanMineOil);
 group.writeEntry("MaxResources", mMaxResources);
 // Convert speeds from amount/adv.call to amount/second
 group.writeEntry("MiningSpeed", mMiningSpeed * 20.0f);
 group.writeEntry("UnloadingSpeed", mUnloadingSpeed * 20.0f);
 return true;
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

bool RefineryProperties::loadPlugin(const KConfigGroup& config)
{
 if (!config.exists()) {
	boError() << k_funcinfo << "config group does not exist";
	return false;
 }
 mCanRefineMinerals = config.readEntry("CanRefineMinerals", false);
 mCanRefineOil = config.readEntry("CanRefineOil", false);
 if (mCanRefineMinerals && mCanRefineOil) {
	boWarning() << k_funcinfo << "unit shouldn't refine minerals *and* oil" << endl;
 }
 return true;
}

bool RefineryProperties::savePlugin(KConfig* config)
{
 KConfigGroup group = config->group(propertyGroup());
 group.writeEntry("CanRefineMinerals", mCanRefineMinerals);
 group.writeEntry("CanRefineOil", mCanRefineOil);
 return true;
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

bool ResourceMineProperties::loadPlugin(const KConfigGroup& config)
{
 if (!config.exists()) {
	boError() << k_funcinfo << "config group does not exist";
	return false;
 }
 mMinerals = config.readEntry("CanProvideMinerals", false);
 mOil = config.readEntry("CanProvideOil", false);
 return true;
}

bool ResourceMineProperties::savePlugin(KConfig* config)
{
 KConfigGroup group = config->group(propertyGroup());
 group.writeEntry("CanProvideMinerals", mMinerals);
 group.writeEntry("CanProvideOil", mOil);
 return true;
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

bool AmmunitionStorageProperties::loadPlugin(const KConfigGroup& config)
{
 if (!config.exists()) {
	boError() << k_funcinfo << "config group does not exist";
	return false;
 }
 mCanStore = config.readEntry("CanStore", QList<QString>());
 mMustBePickedUp = config.readEntry("MustBePickedUp", QList<QString>());
 return true;
}

bool AmmunitionStorageProperties::savePlugin(KConfig* config)
{
 KConfigGroup group = config->group(propertyGroup());
 group.writeEntry("CanStore", mCanStore);
 group.writeEntry("MustBePickedUp", mMustBePickedUp);
 return true;
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

bool RadarProperties::loadPlugin(const KConfigGroup& config)
{
 if (!config.exists()) {
	boError() << k_funcinfo << "config group does not exist";
	return false;
 }
 mTransmittedPower = config.readEntry("TransmittedPower", 10000.0f);
 mMinReceivedPower = config.readEntry("MinReceivedPower", 0.05f);
 mDetectsLandUnits = config.readEntry("DetectsLandUnits", true);
 mDetectsAirUnits = config.readEntry("DetectsAirUnits", true);
 return true;
}

bool RadarProperties::savePlugin(KConfig* config)
{
 KConfigGroup group = config->group(propertyGroup());
 group.writeEntry("TransmittedPower", mTransmittedPower);
 group.writeEntry("MinReceivedPower", mMinReceivedPower);
 return true;
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

bool RadarJammerProperties::loadPlugin(const KConfigGroup& config)
{
 if (!config.exists()) {
	boError() << k_funcinfo << "config group does not exist";
	return false;
 }
 mTransmittedPower = config.readEntry("TransmittedPower", 500.0f);
 return true;
}

bool RadarJammerProperties::savePlugin(KConfig* config)
{
 KConfigGroup group = config->group(propertyGroup());
 group.writeEntry("TransmittedPower", mTransmittedPower);
 return true;
}


UnitStorageProperties::UnitStorageProperties(const UnitProperties* parent)
		: PluginProperties(parent)
{
}

UnitStorageProperties::~UnitStorageProperties()
{
}

QString UnitStorageProperties::propertyGroup()
{
 return QString::fromLatin1("UnitStoragePlugin");
}

QString UnitStorageProperties::name() const
{
 return i18n("Unit Storage Plugin");
}

bool UnitStorageProperties::loadPlugin(const KConfigGroup& config)
{
 if (!config.exists()) {
	boError() << k_funcinfo << "config group does not exist";
	return false;
 }

 mEnterPaths.clear();
 int enterPaths = config.readEntry("EnterPaths", (quint32)0);
 if (enterPaths < 0 || enterPaths >= 100) {
	boError() << k_funcinfo << "invalid EnterPaths count" << endl;
	return false;
 }
 mEnterPaths.resize(enterPaths);
 for (int i = 0; i < enterPaths; i++) {
	if (!loadEnterPath(i, config.config())) {
		boError() << k_funcinfo << "failed loading enter path " << i << endl;
		return false;
	}
 }

 return true;
}

bool UnitStorageProperties::savePlugin(KConfig* config)
{
 KConfigGroup group = config->group(propertyGroup());
 group.writeEntry("EnterPaths", mEnterPaths.count());
 for (int i = 0; i < mEnterPaths.count(); i++) {
	if (!saveEnterPath(i, config)) {
		boError() << k_funcinfo << "failed saving enter path " << i << endl;
		return false;
	}
 }
 return true;
}

bool UnitStorageProperties::loadEnterPath(int path, const KConfig* config)
{
 if (path < 0 || path >= mEnterPaths.count()) {
	return false;
 }
 QString groupName = propertyGroup() + QString::fromLatin1("_EnterPath_%1").arg(path);
 KConfigGroup group = config->group(groupName);
 if (!group.exists()) {
	boError() << k_funcinfo << "no such group " << groupName << endl;
	return false;
 }
 if (!mEnterPaths[path].loadPath(group)) {
	boError() << k_funcinfo << "Unable to load path " << path << endl;
	return false;
 }
 return true;
}

bool UnitStorageProperties::saveEnterPath(int path, KConfig* config)
{
 if (path < 0 || path >= mEnterPaths.count()) {
	return false;
 }
 KConfigGroup group = config->group(propertyGroup() + QString::fromLatin1("_EnterPath_%1").arg(path));
 if (!mEnterPaths[path].savePath(group)) {
	boError() << k_funcinfo << "Unable to save path " << path << endl;
	return false;
 }
 return true;
}

bool UnitStorageProperties::Path::loadPath(const KConfigGroup& config)
{
 if (!config.exists()) {
	boError() << k_funcinfo << "config group does not exist";
	return false;
 }
 clear();

 int points = config.readEntry("Points", (quint32)0);
 if (points <= 0 || points >= 100) {
	boError() << k_funcinfo << "invalid Points count: " << points << endl;
	return false;
 }
 float x0 = -1.0f;
 float y0 = -1.0f;
 for (int i = 0; i < points; i++) {
	float x = config.readEntry(QString::fromLatin1("Point%1_X").arg(i), 0.0f);
	float y = config.readEntry(QString::fromLatin1("Point%1_Y").arg(i), 0.0f);
	if (i == 0) {
		x0 = x;
		y0 = y;
	}
	mPathPoints.append(BoVector2Float(x, y));
 }

 QString typeString = config.readEntry("UnitType", QString::fromLatin1("Land"));
 mType = PathTypeLand;
 if (typeString == QString::fromLatin1("Land")) {
	mType = PathTypeLand;
 } else if (typeString == QString::fromLatin1("Plane")) {
	mType = PathTypePlane;
 } else if (typeString == QString::fromLatin1("Helicopter")) {
	mType = PathTypeHelicopter;
 } else if (typeString == QString::fromLatin1("Ship")) {
	mType = PathTypeShip;
 } else {
	boError() << k_funcinfo << "invalid UnitType: " << typeString << endl;
	return false;
 }

 if (mType == PathTypeLand || mType == PathTypeShip) {
	if (x0 != 0.0f && x0 != 1.0f && y0 != 0.0f && y0 != 1.0f) {
		boError() << k_funcinfo << "either X or Y coordinate of the first Point of a Path must be on the border (i.e. 0.0 or 1.0)." << endl;
		return false;
	}
 }


 if (mType != PathTypeHelicopter) {
	// helicopters may simply land on their destination. all other units
	// must move to somewhere inside the unit.
	if (points < 2) {
		boError() << k_funcinfo << "non-helicopter paths must have at least 2 points" << endl;
		return false;
	}
 }


 // possible methods might be (atm only "Reverse" is used):
 // * Reverse - use reversed "ingoing" paths for leaving this unit
 //             -> must useful method (simplest)
 // * Dedicated - specify a dedicated path for leaving the unit
 //               -> e.g. a runway for landing planes and one for starting
 //                  planes
 // * OnceCompleted - leave immediately once the "ingoing" path has been
 //                   completed. for this the last point must be a border point,
 //                   too (x or y either 0.0 or 1.0).
 //                   this could be useful for things like bridges - we dont
 //                   want to store the unit inside the unit
 QString leaveMethod = config.readEntry("Leave", QString::fromLatin1("Reverse"));
 if (leaveMethod != QString::fromLatin1("Reverse")) {
	boError() << k_funcinfo << "atm only Leave=Reverse supported" << endl;
	return false;
 }

 return true;
}

bool UnitStorageProperties::Path::savePath(KConfigGroup& config)
{
 config.writeEntry("Points", mPathPoints.count());
 for (int i = 0; i < mPathPoints.count(); i++) {
	BoVector2Float p = mPathPoints[i];
	config.writeEntry(QString::fromLatin1("Point%1_X").arg(i), p.x());
	config.writeEntry(QString::fromLatin1("Point%1_Y").arg(i), p.y());
 }

 switch (mType) {
	case PathTypeLand:
		config.writeEntry("UnitType", QString::fromLatin1("Land"));
		break;
	case PathTypePlane:
		config.writeEntry("UnitType", QString::fromLatin1("Plane"));
		break;
	case PathTypeHelicopter:
		config.writeEntry("UnitType", QString::fromLatin1("Helicopter"));
		break;
	case PathTypeShip:
		config.writeEntry("UnitType", QString::fromLatin1("Ship"));
		break;
	default:
		boError() << k_funcinfo << "unhandled type: " << (int)mType << endl;
		return false;
 }
 config.writeEntry("Leave", QString::fromLatin1("Reverse"));
 return true;
}

unsigned int UnitStorageProperties::enterPathCount() const
{
 return mEnterPaths.count();
}

Q3ValueList<BoVector2Float> UnitStorageProperties::enterPath(unsigned int i) const
{
 if (i >= enterPathCount()) {
	boError() << k_funcinfo << "index out of range: " << i << endl;
	return Q3ValueList<BoVector2Float>();
 }
 return mEnterPaths[i].mPathPoints;
}

Q3ValueList<BoVector2Float> UnitStorageProperties::leavePathForEnterPath(unsigned int i) const
{
 Q3ValueList<BoVector2Float> leave;
 Q3ValueList<BoVector2Float> enter = enterPath(i);

 // TODO: atm we assume "Reverse" as Leave method.
 for (Q3ValueList<BoVector2Float>::iterator it = enter.begin(); it != enter.end(); ++it) {
	leave.prepend(*it);
 }
 return leave;
}

UnitStorageProperties::PathUnitType UnitStorageProperties::enterPathUnitType(unsigned int i) const
{
 if (i >= enterPathCount()) {
	boError() << k_funcinfo << "index out of range: " << i << endl;
	return PathTypeLand;
 }
 return mEnterPaths[i].mType;
}


BoVector2Float UnitStorageProperties::enterDirection(unsigned int i) const
{
 if (i >= enterPathCount()) {
	boError() << k_funcinfo << "index out of range: " << i << endl;
	return BoVector2Float(1.0f, 0.0f);
 }
 if (mEnterPaths[i].mPathPoints.count() < 2) {
	return BoVector2Float(0.0f, 0.0f);
 }
 BoVector2Float dir = mEnterPaths[i].mPathPoints[1] - mEnterPaths[i].mPathPoints[0];
 dir.normalize();
 return dir;
}

