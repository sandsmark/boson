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

#include <qcanvas.h>
#include "speciestheme.h"

#include <ksimpleconfig.h>
#include <klocale.h>
#include <kdebug.h>

class UnitProperties::MobileProperties
{
public:
	MobileProperties()
	{
	}

	float mSpeed;
	bool mCanGoOnLand; // a nice candidate for bitfields...
	bool mCanGoOnWater;
	bool mCanMineMinerals;
	bool mCanMineOil;
	unsigned int mMaxResources;
};

class UnitProperties::FacilityProperties
{
public:
	FacilityProperties()
	{
	}

	bool mCanProduce;
	QValueList<int> mProducerList;
	bool mCanRefineMinerals;
	bool mCanRefineOil;
	unsigned int mConstructionFrames;
};


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

 loadUnitType(fileName);
}

UnitProperties::~UnitProperties()
{
 delete mMobileProperties;
 delete mFacilityProperties;
}

void UnitProperties::loadUnitType(const QString& fileName)
{
 bool isFacility;
 KSimpleConfig conf(fileName);
 conf.setGroup(QString::fromLatin1("Boson Unit"));

 mUnitPath = fileName.left(fileName.length() - QString("index.desktop").length());
 /// FIXME: maybe rename to Type or TypeID (Id is confusing IMHO) - rivol
 mTypeId = conf.readUnsignedLongNumEntry("Id", 0); // 0 == invalid // Note: Id == Unit::type() , NOT Unit::id() !
 if (typeId() <= 0) {
	kdError() << "Invalid TypeId: " << typeId() << " in unit file " << fileName << endl;
	// we continue - but we'll crash soon
 }
 mTerrain = (TerrainType)conf.readNumEntry("TerrainType", 0);
 if (mTerrain < 0 || mTerrain > 2) {
	kdWarning() << k_funcinfo << "Invalid TerrainType value: " << mTerrain << " for unit " << typeId() << ", defaulting to 0" << endl;
	mTerrain = (TerrainType)0;
 }
 mName = conf.readEntry("Name", i18n("Unknown"));
 mHealth = conf.readUnsignedLongNumEntry("Health", 100);
 mMineralCost= conf.readUnsignedLongNumEntry("MineralCost", 100);
 mOilCost = conf.readUnsignedLongNumEntry("OilCost", 0);
 mWeaponDamage = conf.readLongNumEntry("WeaponDamage", 0);
 mWeaponRange = conf.readUnsignedLongNumEntry("WeaponRange", 0);
 if (mWeaponDamage == 0) {
	mWeaponRange = 0;
 }
 mSightRange = conf.readUnsignedLongNumEntry("SightRange", 5); 
 mReload = conf.readUnsignedNumEntry("Reload", 0);
 mProductionTime = conf.readUnsignedNumEntry("ProductionTime", 100);
 mShields = conf.readUnsignedLongNumEntry("Shield", 0);
 mArmor = conf.readUnsignedLongNumEntry("Armor", 0);
 mCanShootAtAirUnits = conf.readBoolEntry("CanShootAtAirUnits", isAircraft() && weaponDamage());
 mCanShootAtLandUnits = conf.readBoolEntry("CanShootAtLandUnits", (isLand() || isShip()) && weaponDamage());
 mSupportMiniMap = conf.readBoolEntry("SupportMiniMap", false);
 isFacility = conf.readBoolEntry("IsFacility", false);
 // KConfig doesn't support reading list of _unsigned_ int's so we must cast
 //  them ourselves
 QValueList<int> tmpRequisities = conf.readIntListEntry("Requisities");
 QValueList<int>::Iterator it;
 for(it = tmpRequisities.begin(); it != tmpRequisities.end(); it++) {
	mRequisities.append((unsigned long int)(*it));
 }

 if (isFacility) {
	mProducer = conf.readUnsignedNumEntry("Producer", (unsigned int)CommandBunker);
	loadFacilityProperties(&conf);
 } else {
	mProducer = conf.readUnsignedNumEntry("Producer", (unsigned int)mTerrain);
	loadMobileProperties(&conf);
 }

 loadTextureNames(&conf);
}

void UnitProperties::loadMobileProperties(KSimpleConfig* conf)
{
 conf->setGroup("Boson Mobile Unit");
 mMobileProperties = new MobileProperties;
 mMobileProperties->mSpeed = (float)conf->readDoubleNumEntry("Speed", 0);
 if(mMobileProperties->mSpeed < 0) {
	kdWarning() << k_funcinfo << "Invalid Speed value: " << mMobileProperties->mSpeed <<
			" for unit " << typeId() << ", defaulting to 0" << endl;
	mMobileProperties->mSpeed = 0;
 }
 mMobileProperties->mCanGoOnLand = conf->readBoolEntry("CanGoOnLand",
		(isLand() || isAircraft()));
 mMobileProperties->mCanGoOnWater = conf->readBoolEntry("CanGoOnWater",
		(isShip() || isAircraft()));
 mMobileProperties->mCanMineMinerals = conf->readBoolEntry("CanMineMinerals", 
		false);
 mMobileProperties->mCanMineOil = conf->readBoolEntry("CanMineOil", false);
 mMobileProperties->mMaxResources = conf->readUnsignedNumEntry("MaxResources",
		(canMineMinerals() || canMineOil()) ? 100 : 0);
}

void UnitProperties::loadFacilityProperties(KSimpleConfig* conf)
{
 conf->setGroup("Boson Facility");
 mFacilityProperties = new FacilityProperties;
 mFacilityProperties->mCanProduce = conf->readBoolEntry("CanProduce", false);
 mFacilityProperties->mProducerList = conf->readIntListEntry("ProducerList");
 mFacilityProperties->mCanRefineMinerals = conf->readBoolEntry("CanRefineMinerals",
		false);
 mFacilityProperties->mCanRefineOil= conf->readBoolEntry("CanRefineOil", false);
 mFacilityProperties->mConstructionFrames= conf->readUnsignedNumEntry("ConstructionSteps", FACILITY_CONSTRUCTION_STEPS);
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
		kdDebug() << "mapping " << textures[i] << "->" << longName << endl;
	}
 }
}

bool UnitProperties::isMobile() const
{
 return (mMobileProperties != 0);
}

bool UnitProperties::isFacility() const
{
 return (mFacilityProperties != 0);
}

unsigned long int UnitProperties::mineralCost() const
{
 return mMineralCost;
}

unsigned long int UnitProperties::oilCost() const
{
 return mOilCost;
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

bool UnitProperties::canProduce() const
{
 if (!mFacilityProperties) {
	return false;
 }
 return mFacilityProperties->mCanProduce;
}

QValueList<int> UnitProperties::producerList() const
{
 if (!mFacilityProperties) {
	return QValueList<int>();
 }
 return mFacilityProperties->mProducerList;
}

unsigned int UnitProperties::productionTime() const
{
 return mProductionTime;
}

bool UnitProperties::canMineMinerals() const
{
 if (!mMobileProperties) {
	return false;
 }
 return mMobileProperties->mCanMineMinerals;
}

bool UnitProperties::canMineOil() const
{
 if (!mMobileProperties) {
	return false;
 }
 return mMobileProperties->mCanMineOil;
}

unsigned int UnitProperties::maxResources() const
{
 if (!canMineMinerals() && !canMineOil()) {
	return 0;
 }
 return mMobileProperties->mMaxResources;
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

