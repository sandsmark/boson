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

class UnitPropertiesPrivate
{
public:
	UnitPropertiesPrivate()
	{
		mMobileProperties = 0;
		mFacilityProperties = 0;
	}

	QString mName;
	QString mUnitPath; // path to unit files
	int mTypeId; // note: -1 is invalid!
	unsigned long int mArmor;
	unsigned long int mShields;
	unsigned long int mHealth;
	unsigned long int mPrize;
	unsigned long int mRange;
	long int mDamage;
	unsigned int mReload;
	UnitProperties::TerrainType mTerrain;

	MobileProperties* mMobileProperties;
	FacilityProperties* mFacilityProperties;
};

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
 d->mUnitPath = fileName.left(fileName.length() - QString("index.desktop").length());
 isFacility = conf.readBoolEntry("IsFacility", false);
 d->mName = conf.readEntry("Name", i18n("Unknown"));
 d->mTypeId = conf.readNumEntry("Id", -1); // -1 == invalid // Note: Id == Unit::type() , NOT Unit::id() !
 d->mHealth = conf.readUnsignedLongNumEntry("Health", 100);
 d->mShields = conf.readUnsignedLongNumEntry("Shield", 0); 
 d->mArmor = conf.readUnsignedLongNumEntry("Armor", 0); 
 d->mPrize = conf.readUnsignedLongNumEntry("Prize", 0); 
 d->mDamage = conf.readLongNumEntry("Damage", 0); 
 d->mRange = conf.readUnsignedLongNumEntry("Range", 0); 
 d->mReload = conf.readUnsignedNumEntry("Reload", 0); 
 d->mTerrain = (TerrainType)conf.readNumEntry("TerrainType", 0);
 if (d->mTerrain < 0 || d->mTerrain > 2) {
	d->mTerrain = (TerrainType)0;
 }
 if (d->mTypeId < 0) {
	kdError() << "Invalid TypeId: " << d->mTypeId << " in unit file " << fileName << endl;
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
 d->mFacilityProperties->mCanProduce = conf->readBoolEntry("CanProduce", (isShip() || isAircraft()));
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

const QString& UnitProperties::name() const
{
 return d->mName;
}

unsigned long int UnitProperties::armor() const
{
 return d->mArmor;
}

unsigned long int UnitProperties::shields() const
{
 return d->mShields;
}

unsigned long int UnitProperties::health() const
{
 return d->mHealth;
}

unsigned long int UnitProperties::prize() const
{
 return d->mPrize;
}

long int UnitProperties::damage() const
{
 return d->mDamage;
}

unsigned long int UnitProperties::range() const
{
 return d->mRange;
}

unsigned int UnitProperties::reload() const
{
 return d->mReload;
}

int UnitProperties::typeId() const
{
 return d->mTypeId;
}

bool UnitProperties::isShip() const
{
 return d->mTerrain == Water;
}

bool UnitProperties::isAircraft() const
{
 return d->mTerrain == Air;
}

bool UnitProperties::isLand() const
{
 return d->mTerrain == Land;
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

const QString& UnitProperties::unitPath() const
{
 return d->mUnitPath;
}
