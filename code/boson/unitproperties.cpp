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

#include <ksimpleconfig.h>
#include <klocale.h>
#include <kdebug.h>

class MobileProperties
{
public:
	MobileProperties()
	{
	}

	double mSpeed;
	bool mCanGoOnLand; // a nice candidate for bitfields...
	bool mCanGoOnWater;
	bool mCanMineMinerals;
	bool mCanMineOil;
};

class FacilityProperties
{
public:
	FacilityProperties()
	{
	}

	bool mCanProduce;
	QValueList<int> mProducerList;
};

class UnitProperties::UnitPropertiesPrivate
{
public:
	UnitPropertiesPrivate()
	{
		mMobileProperties = 0;
		mFacilityProperties = 0;
	}

	unsigned long int mArmor;
	unsigned long int mShields;

	MobileProperties* mMobileProperties;
	FacilityProperties* mFacilityProperties;
};

UnitProperties::UnitProperties()
{
 d = new UnitPropertiesPrivate;
}

UnitProperties::UnitProperties(const QString& fileName)
{
 d = new UnitPropertiesPrivate;
 loadUnitType(fileName);
}

UnitProperties::~UnitProperties()
{
 delete d->mMobileProperties;
 delete d->mFacilityProperties;
 delete d;
}

void UnitProperties::loadUnitType(const QString& fileName)
{
 bool isFacility;
 KSimpleConfig conf(fileName);
 conf.setGroup(QString::fromLatin1("Boson Unit"));

 mUnitPath = fileName.left(fileName.length() - QString("index.desktop").length());
 mTypeId = conf.readNumEntry("Id", -1); // -1 == invalid // Note: Id == Unit::type() , NOT Unit::id() !
 if (typeId() < 0) {
	kdError() << "Invalid TypeId: " << typeId() << " in unit file " << fileName << endl;
	// we continue - but we'll crash soon
 }
 mTerrain = (TerrainType)conf.readNumEntry("TerrainType", 0);
 if (mTerrain < 0 || mTerrain > 2) {
	mTerrain = (TerrainType)0;
 }
 mName = conf.readEntry("Name", i18n("Unknown"));
 mHealth = conf.readUnsignedLongNumEntry("Health", 100);
 mMineralCost= conf.readUnsignedLongNumEntry("MineralCost", 0); 
 mOilCost = conf.readUnsignedLongNumEntry("OilCost", 0); 
 mDamage = conf.readLongNumEntry("Damage", 0); 
 mRange = conf.readUnsignedLongNumEntry("Range", 0); 
 mSightRange = conf.readUnsignedLongNumEntry("SightRange", 5); 
 mReload = conf.readUnsignedNumEntry("Reload", 0); 
 mProductionTime = conf.readUnsignedNumEntry("ProductionTime", 100);
 d->mShields = conf.readUnsignedLongNumEntry("Shield", 0); 
 d->mArmor = conf.readUnsignedLongNumEntry("Armor", 0);
 mCanShootAtAirUnits = conf.readBoolEntry("CanShootAtAirUnits", isAircraft());
 mCanShootAtLandUnits = conf.readBoolEntry("CanShootAtLandUnits", (isLand() || isShip()));
 isFacility = conf.readBoolEntry("IsFacility", false);

 if (isFacility) {
	mProducer = conf.readUnsignedNumEntry("Producer", (unsigned int)CommandBunker);
	loadFacilityProperties(&conf);
 } else {
	mProducer = conf.readUnsignedNumEntry("Producer", (unsigned int)mTerrain);
	loadMobileProperties(&conf);
 }
}

void UnitProperties::loadMobileProperties(KSimpleConfig* conf)
{
 conf->setGroup("Boson Mobile Unit");
 d->mMobileProperties = new MobileProperties;
 d->mMobileProperties->mSpeed = conf->readDoubleNumEntry("Speed", 0);
 d->mMobileProperties->mCanGoOnLand = conf->readBoolEntry("CanGoOnLand", (isLand() || isAircraft()));
 d->mMobileProperties->mCanGoOnWater = conf->readBoolEntry("CanGoOnWater", (isShip() || isAircraft()));
 d->mMobileProperties->mCanMineMinerals = conf->readBoolEntry("CanMineMinerals", false);
 d->mMobileProperties->mCanMineOil= conf->readBoolEntry("CanMineOil", false);
}

void UnitProperties::loadFacilityProperties(KSimpleConfig* conf)
{
 conf->setGroup("Boson Facility");
 d->mFacilityProperties = new FacilityProperties;
 d->mFacilityProperties->mCanProduce = conf->readBoolEntry("CanProduce", false);
 d->mFacilityProperties->mProducerList = conf->readIntListEntry("ProducerList");
}

bool UnitProperties::isMobile() const
{
 return (d->mMobileProperties != 0);
}

bool UnitProperties::isFacility() const
{
 return (d->mFacilityProperties != 0);
}

unsigned long int UnitProperties::armor() const
{
 return d->mArmor;
}

unsigned long int UnitProperties::shields() const
{
 return d->mShields;
}

unsigned long int UnitProperties::mineralCost() const
{
 return mMineralCost;
}

unsigned long int UnitProperties::oilCost() const
{
 return mOilCost;
}

double UnitProperties::speed() const
{
 if (!d->mMobileProperties) {
	return 0;
 }
 return d->mMobileProperties->mSpeed;
}

bool UnitProperties::canGoOnLand() const
{
 if (!d->mMobileProperties) {
	return true; // even facilities can go there.
 }
 return d->mMobileProperties->mCanGoOnLand;
}

bool UnitProperties::canGoOnWater() const
{
 if (!d->mMobileProperties) {
	return false;
 }
 return d->mMobileProperties->mCanGoOnWater;
}

bool UnitProperties::canProduce() const
{
 if (!d->mFacilityProperties) {
	return false;
 }
 return d->mFacilityProperties->mCanProduce;
}

QValueList<int> UnitProperties::producerList() const
{
 if (!d->mFacilityProperties) {
	return QValueList<int>();
 }
 return d->mFacilityProperties->mProducerList;
}

unsigned int UnitProperties::productionTime() const
{
 return mProductionTime;
}

bool UnitProperties::canMineMinerals() const
{
 if (!d->mMobileProperties) {
	return false;
 }
 return d->mMobileProperties->mCanMineMinerals;
}

bool UnitProperties::canMineOil() const
{
 if (!d->mMobileProperties) {
	return false;
 }
 return d->mMobileProperties->mCanMineOil;
}

