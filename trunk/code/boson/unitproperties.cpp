/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "unitproperties.h"
#include "defines.h"

#include "speciestheme.h"
#include "pluginproperties.h"
#include "upgradeproperties.h"
#include "bosonparticlesystem.h"
#include "bosonparticlemanager.h"
#include "bosonweapon.h"
#include "bosonconfig.h"
#include "bodebug.h"

#include <ksimpleconfig.h>
#include <klocale.h>

class UnitProperties::MobileProperties
{
public:
	MobileProperties()
	{
	}

	float mSpeed;
	bool mCanGoOnLand;
	bool mCanGoOnWater;
};

class UnitProperties::FacilityProperties
{
public:
	FacilityProperties()
	{
	}

	bool mCanRefineMinerals;
	bool mCanRefineOil;
	unsigned int mConstructionFrames;
};


UnitProperties::UnitProperties()
{
 mTheme = 0;
 mMobileProperties = 0;
 mFacilityProperties = 0;
}

UnitProperties::UnitProperties(SpeciesTheme* theme)
{
 mTheme = theme;
 mMobileProperties = 0;
 mFacilityProperties = 0;
}

UnitProperties::UnitProperties(SpeciesTheme* theme, const QString& fileName)
{
 mTheme = theme;
 mMobileProperties = 0;
 mFacilityProperties = 0;

 loadUnitType(fileName, true);
}

UnitProperties::~UnitProperties()
{
 delete mMobileProperties;
 delete mFacilityProperties;
}

void UnitProperties::loadUnitType(const QString& fileName, bool full)
{
 bool isFacility;
 KSimpleConfig conf(fileName);
 conf.setGroup(QString::fromLatin1("Boson Unit"));

 mUnitPath = fileName.left(fileName.length() - QString("index.unit").length());
 /// FIXME: maybe rename to Type or TypeID (Id is confusing IMHO) - rivol
 mTypeId = conf.readUnsignedLongNumEntry("Id", 0); // 0 == invalid // Note: Id == Unit::type() , NOT Unit::id() !
 if (typeId() <= 0) {
	boError() << "Invalid TypeId: " << typeId() << " in unit file " << fileName << endl;
	// we continue - but we'll crash soon
 }
 mTerrain = (TerrainType)conf.readNumEntry("TerrainType", 0);
 if (mTerrain < 0 || mTerrain > 2) {
	boWarning() << k_funcinfo << "Invalid TerrainType value: " << mTerrain << " for unit " << typeId() << ", defaulting to 0" << endl;
	mTerrain = (TerrainType)0;
 }
 mUnitWidth = (unsigned int)(conf.readDoubleNumEntry("UnitWidth", 1.0) * BO_TILE_SIZE);
 mUnitHeight = (unsigned int)(conf.readDoubleNumEntry("UnitHeight", 1.0) * BO_TILE_SIZE);
 mUnitDepth = (unsigned int)(conf.readDoubleNumEntry("UnitDepth", 1.0) * BO_TILE_SIZE);
 mName = conf.readEntry("Name", i18n("Unknown"));
 mHealth = conf.readUnsignedLongNumEntry("Health", 100);
 mMineralCost = conf.readUnsignedLongNumEntry("MineralCost", 100);
 mOilCost = conf.readUnsignedLongNumEntry("OilCost", 0);
 mSightRange = conf.readUnsignedLongNumEntry("SightRange", 5);
 mProductionTime = conf.readUnsignedNumEntry("ProductionTime", 100);
 mShields = conf.readUnsignedLongNumEntry("Shield", 0);
 mArmor = conf.readUnsignedLongNumEntry("Armor", 0);
 mSupportMiniMap = conf.readBoolEntry("SupportMiniMap", false);
 isFacility = conf.readBoolEntry("IsFacility", false);
 mRequirements = BosonConfig::readUnsignedLongNumList(&conf, "Requirements");

 if (full) {
	mDestroyedParticleSystems = BosonParticleSystemProperties::loadParticleSystemProperties(&conf, "DestroyedParticles", mTheme);
 }

 if (isFacility) {
	mProducer = conf.readUnsignedNumEntry("Producer", (unsigned int)CommandBunker);
	loadFacilityProperties(&conf);
 } else {
	mProducer = conf.readUnsignedNumEntry("Producer", (unsigned int)mTerrain);
	loadMobileProperties(&conf);
 }

 loadAllPluginProperties(&conf);
 loadTextureNames(&conf);
 loadSoundNames(&conf);
 loadUpgrades(&conf);
 loadWeapons(&conf);
}

void UnitProperties::saveUnitType(const QString& fileName)
{
 KSimpleConfig conf(fileName);
 conf.setGroup(QString::fromLatin1("Boson Unit"));

 conf.writeEntry("Id", typeId());
 conf.writeEntry("TerrainType", (int)mTerrain);
 conf.writeEntry("UnitWidth", (double)mUnitWidth / BO_TILE_SIZE);
 conf.writeEntry("UnitHeight", (double)mUnitHeight / BO_TILE_SIZE);
 conf.writeEntry("UnitDepth", (double)mUnitDepth / BO_TILE_SIZE);
 conf.writeEntry("Name", mName);
 conf.writeEntry("Health", mName);
 conf.writeEntry("MineralCost", mMineralCost);
 conf.writeEntry("OilCost", mOilCost);
 conf.writeEntry("SightRange", mSightRange);
 conf.writeEntry("ProductionTime", mProductionTime);
 conf.writeEntry("Shield", mShields);
 conf.writeEntry("Armor", mArmor);
 conf.writeEntry("SupportMiniMap", mSupportMiniMap);
 conf.writeEntry("IsFacility", isFacility());
 BosonConfig::writeUnsignedLongNumList(&conf, "Requirements", mRequirements);
 conf.writeEntry("Producer", mProducer);

 /// TODO
//	mDestroyedParticleSystems = BosonParticleSystemProperties::loadParticleSystemProperties(&conf, "DestroyedParticles", mTheme);

 if (isFacility()) {
	saveFacilityProperties(&conf);
 } else {
	saveMobileProperties(&conf);
 }

 saveAllPluginProperties(&conf);  // This saves weapons too
 saveTextureNames(&conf);
 saveSoundNames(&conf);
// saveUpgrades(&conf); // TODO
}

void UnitProperties::loadMobileProperties(KSimpleConfig* conf)
{
 conf->setGroup("Boson Mobile Unit");
 mMobileProperties = new MobileProperties;
 mMobileProperties->mSpeed = (float)conf->readDoubleNumEntry("Speed", 0);
 if(mMobileProperties->mSpeed < 0) {
	boWarning() << k_funcinfo << "Invalid Speed value: " << mMobileProperties->mSpeed <<
			" for unit " << typeId() << ", defaulting to 0" << endl;
	mMobileProperties->mSpeed = 0;
 }
 mMobileProperties->mCanGoOnLand = conf->readBoolEntry("CanGoOnLand",
		(isLand() || isAircraft()));
 mMobileProperties->mCanGoOnWater = conf->readBoolEntry("CanGoOnWater",
		(isShip() || isAircraft()));
}

void UnitProperties::loadFacilityProperties(KSimpleConfig* conf)
{
 conf->setGroup("Boson Facility");
 mFacilityProperties = new FacilityProperties;
 mFacilityProperties->mCanRefineMinerals = conf->readBoolEntry("CanRefineMinerals",
		false);
 mFacilityProperties->mCanRefineOil = conf->readBoolEntry("CanRefineOil", false);
 mFacilityProperties->mConstructionFrames = conf->readUnsignedNumEntry("ConstructionSteps", 20);
}

void UnitProperties::loadAllPluginProperties(KSimpleConfig* conf)
{
 if (conf->hasGroup(ProductionProperties::propertyGroup())) {
	loadPluginProperties(new ProductionProperties(this), conf);
 }
 if (conf->hasGroup(RepairProperties::propertyGroup())) {
	loadPluginProperties(new RepairProperties(this), conf);
 }
 if (conf->hasGroup(HarvesterProperties::propertyGroup())) {
	loadPluginProperties(new HarvesterProperties(this), conf);
 }
}

void UnitProperties::loadPluginProperties(PluginProperties* prop, KSimpleConfig* conf)
{
 if (!prop || !conf) {
	boError() << k_funcinfo << "oops" << endl;
	return;
 }
 prop->loadPlugin(conf);
 mPlugins.append(prop);
}

void UnitProperties::loadTextureNames(KSimpleConfig* conf)
{
 if (!conf->hasGroup("Textures")) {
	return;
 }
 mTextureNames.clear();
 conf->setGroup("Textures");
 QStringList textures = conf->readListEntry("Textures");
 for (unsigned int i = 0; i < textures.count(); i++) {
	QString longName = conf->readEntry(textures[i], QString::null);
	if (!longName.isEmpty()) {
		mTextureNames.insert(textures[i], longName);
		boDebug() << "mapping " << textures[i] << "->" << longName << endl;
	}
 }
}

void UnitProperties::loadSoundNames(KSimpleConfig* conf)
{
 mSounds.clear();
 conf->setGroup("Sounds");
 mSounds.insert(SoundShoot, conf->readEntry("Shoot", "shoot"));
 mSounds.insert(SoundOrderMove, conf->readEntry("OrderMove", "order_move"));
 mSounds.insert(SoundOrderAttack, conf->readEntry("OrderAttack", "order_attack"));
 mSounds.insert(SoundOrderSelect, conf->readEntry("OrderSelect", "order_select"));
 mSounds.insert(SoundReportProduced, conf->readEntry("ReportProduced", "report_produced"));
 mSounds.insert(SoundReportDestroyed, conf->readEntry("ReportDestroyed", "report_destroyed"));
 mSounds.insert(SoundReportUnderAttack, conf->readEntry("ReportUnderAttack", "report_underattack"));
}

void UnitProperties::loadWeapons(KSimpleConfig* conf)
{
 conf->setGroup("Boson Unit");
 int num = conf->readNumEntry("Weapons", 0);
 mCanShootAtAirUnits = false;
 mCanShootAtLandUnits = false;
 for (int i = 0; i < num; i++) {
	conf->setGroup(QString("Weapon_%1").arg(i));
	BosonWeaponProperties* p = new BosonWeaponProperties(this);
	p->loadPlugin(conf);
	mPlugins.append(p);
	if(p->canShootAtAirUnits()) {
		mCanShootAtAirUnits = true;
	}
	if(p->canShootAtLandUnits()) {
		mCanShootAtLandUnits = true;
	}
 }
}

void UnitProperties::saveMobileProperties(KSimpleConfig* conf)
{
 conf->setGroup("Boson Mobile Unit");
 conf->writeEntry("Speed", (double)mMobileProperties->mSpeed);
 conf->writeEntry("CanGoOnLand", mMobileProperties->mCanGoOnLand);
 conf->writeEntry("CanGoOnWater", mMobileProperties->mCanGoOnWater);
}

void UnitProperties::saveFacilityProperties(KSimpleConfig* conf)
{
 conf->setGroup("Boson Facility");
 conf->writeEntry("CanRefineMinerals", mFacilityProperties->mCanRefineMinerals);
 conf->writeEntry("CanRefineOil", mFacilityProperties->mCanRefineOil);
 conf->writeEntry("ConstructionSteps", mFacilityProperties->mConstructionFrames);
}

void UnitProperties::saveAllPluginProperties(KSimpleConfig* conf)
{
 QPtrListIterator<PluginProperties> it(mPlugins);
 while (it.current()) {
	it.current()->savePlugin(conf);
 }
}

void UnitProperties::saveTextureNames(KSimpleConfig* conf)
{
 if (mTextureNames.count() == 0) {
	return;
 }
 conf->setGroup("Textures");
 QMap<QString, QString>::Iterator it;
 QStringList textures;
 for(it = mTextureNames.begin(); it != mTextureNames.end(); ++it) {
	textures.append(it.key());
	conf->writeEntry(it.key(), it.data());
 }
 conf->writeEntry("Textures", textures);
}

void UnitProperties::saveSoundNames(KSimpleConfig* conf)
{
 mSounds.clear();
 conf->setGroup("Sounds");
 conf->writeEntry("Shoot", mSounds[SoundShoot]);
 conf->writeEntry("OrderMove", mSounds[SoundOrderMove]);
 conf->writeEntry("OrderAttack", mSounds[SoundOrderAttack]);
 conf->writeEntry("OrderSelect", mSounds[SoundOrderSelect]);
 conf->writeEntry("ReportProduced", mSounds[SoundReportProduced]);
 conf->writeEntry("ReportDestroyed", mSounds[SoundReportDestroyed]);
 conf->writeEntry("ReportUnderAttack", mSounds[SoundReportUnderAttack]);
}

bool UnitProperties::isMobile() const
{
 return (mMobileProperties != 0);
}

bool UnitProperties::isFacility() const
{
 return (mFacilityProperties != 0);
}

float UnitProperties::speed() const
{
 if (!mMobileProperties) {
	return 0;
 }
 return mMobileProperties->mSpeed;
}

bool UnitProperties::canGoOnLand() const
{
 if (!mMobileProperties) {
	return true; // even facilities can go there.
 }
 return mMobileProperties->mCanGoOnLand;
}

bool UnitProperties::canGoOnWater() const
{
 if (!mMobileProperties) {
	return false;
 }
 return mMobileProperties->mCanGoOnWater;
}

bool UnitProperties::canRefineMinerals() const
{
 if (!mFacilityProperties) {
	return false;
 }
 return mFacilityProperties->mCanRefineMinerals;
}

bool UnitProperties::canRefineOil() const
{
 if (!mFacilityProperties) {
	return false;
 }
 return mFacilityProperties->mCanRefineOil;
}

unsigned int UnitProperties::constructionSteps() const
{
 if (!mFacilityProperties) {
	return 0;
 }
 return mFacilityProperties->mConstructionFrames;
}

const PluginProperties* UnitProperties::properties(int pluginType) const
{
 QPtrListIterator<PluginProperties> it(mPlugins);
 for (; it.current(); ++it) {
	if (it.current()->pluginType() == pluginType) {
		return it.current();
	}
 }
 return 0;
}

void UnitProperties::loadUpgrades(KSimpleConfig* conf)
{
// boDebug() << k_funcinfo << endl;
 conf->setGroup("Boson Unit");
 int count = conf->readNumEntry("Upgrades", 0);
 if (count == 0) {
	return;
 }
 boDebug() << k_funcinfo << "Loading " << count << " upgrades for unit " << typeId() << endl;
 for (int i = 1; i <= count; i++) {
	UpgradeProperties* upgrade = new UpgradeProperties(this, i);
	upgrade->loadPlugin(conf);
	mUpgrades.append(upgrade);
	mNotResearchedUpgrades.append(upgrade);
 }
 boDebug() << k_funcinfo << "DONE" << endl;
}

void UnitProperties::setSpeed(float speed)
{
 if (!mMobileProperties) {
	return;
 }
 mMobileProperties->mSpeed = speed;
}

QPtrList<BosonParticleSystem> UnitProperties::newDestroyedParticleSystems(float x, float y, float z) const
{
 QPtrList<BosonParticleSystem> list;
 QPtrListIterator<BosonParticleSystemProperties> it(mDestroyedParticleSystems);
 while(it.current())
 {
	list.append(it.current()->newSystem(x, y, z));
	++it;
 }
 return list;
}
