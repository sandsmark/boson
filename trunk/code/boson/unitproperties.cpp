/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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
};

class FacilityProperties
{
public:
	FacilityProperties()
	{
	}

	bool mCanProduce;
	QValueList<int> mProduceList;
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
	unsigned long int mPrize;

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
 if (d->mMobileProperties) {
	delete d->mMobileProperties;
 }
 if (d->mFacilityProperties) {
	delete d->mFacilityProperties;
 }
 delete d;
}

void UnitProperties::loadUnitType(const QString& fileName)
{
 bool isFacility;
 KSimpleConfig conf(fileName);
 conf.setGroup(QString::fromLatin1("Boson Unit"));
 mUnitPath = fileName.left(fileName.length() - QString("index.desktop").length());
 isFacility = conf.readBoolEntry("IsFacility", false);
 mName = conf.readEntry("Name", i18n("Unknown"));
 mTypeId = conf.readNumEntry("Id", -1); // -1 == invalid // Note: Id == Unit::type() , NOT Unit::id() !
 mHealth = conf.readUnsignedLongNumEntry("Health", 100);
 d->mShields = conf.readUnsignedLongNumEntry("Shield", 0); 
 d->mArmor = conf.readUnsignedLongNumEntry("Armor", 0); 
 d->mPrize = conf.readUnsignedLongNumEntry("Prize", 0); 
 mDamage = conf.readLongNumEntry("Damage", 0); 
 mRange = conf.readUnsignedLongNumEntry("Range", 0); 
 mReload = conf.readUnsignedNumEntry("Reload", 0); 
 mTerrain = (TerrainType)conf.readNumEntry("TerrainType", 0);
 if (mTerrain < 0 || mTerrain > 2) {
	mTerrain = (TerrainType)0;
 }
 if (typeId() < 0) {
	kdError() << "Invalid TypeId: " << typeId() << " in unit file " << fileName << endl;
	// we continue - but we'll crash soon
 }

 if (isFacility) {
	loadFacilityProperties(&conf);
 } else {
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
}

void UnitProperties::loadFacilityProperties(KSimpleConfig* conf)
{
 conf->setGroup("Boson Facility");
 d->mFacilityProperties = new FacilityProperties;
 d->mFacilityProperties->mCanProduce = conf->readBoolEntry("CanProduce", false);
 d->mFacilityProperties->mProduceList = conf->readIntListEntry("ProduceList");
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

unsigned long int UnitProperties::prize() const
{
 return d->mPrize;
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

QValueList<int> UnitProperties::produceList() const
{
 if (!d->mFacilityProperties) {
	return QValueList<int>();
 }
 return d->mFacilityProperties->mProduceList;
}

