/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bosoneffect.h"
#include "bosoneffectproperties.h"
#include "bosonweapon.h"
#include "bosonconfig.h"
#include "bodebug.h"
#include "boaction.h"

#include <ksimpleconfig.h>
#include <klocale.h>
#include <kmdcodec.h>

#include <qfile.h>

class UnitProperties::UnitPropertiesPrivate
{
public:
	UnitPropertiesPrivate()
	{
	}

	QCString mMD5;
	QString mName;
	QString mUnitPath; // the path to the unit files
	QValueList<unsigned long int> mRequirements;


	QPtrList<PluginProperties> mPlugins;

	QMap<QString, QString> mTextureNames;
	QMap<int, QString> mSounds;

	QIntDict<BoAction> mActions;

	QPtrList<BosonEffectProperties> mDestroyedEffects;
	QValueList<unsigned long int> mDestroyedEffectIds;
	QPtrList<BosonEffectProperties> mConstructedEffects;
	QValueList<unsigned long int> mConstructedEffectIds;
	QPtrList<BosonEffectProperties> mExplodingFragmentFlyEffects;
	QValueList<unsigned long int> mExplodingFragmentFlyEffectIds;
	QPtrList<BosonEffectProperties> mExplodingFragmentHitEffects;
	QValueList<unsigned long int> mExplodingFragmentHitEffectIds;
	BoVector3 mHitPoint;  // FIXME: better name
};

class UnitProperties::MobileProperties
{
public:
	MobileProperties()
	{
	}

	float mSpeed;
	float mAccelerationSpeed;
	float mDecelerationSpeed;
	int mRotationSpeed;
	bool mCanGoOnLand;
	bool mCanGoOnWater;
};

class UnitProperties::FacilityProperties
{
public:
	FacilityProperties()
	{
	}

	unsigned int mConstructionFrames;
};


UnitProperties::UnitProperties(bool fullmode)
{
 init();
 mFullMode = fullmode;
}

UnitProperties::UnitProperties(SpeciesTheme* theme)
{
 init();
 mTheme = theme;
}

UnitProperties::UnitProperties(SpeciesTheme* theme, const QString& fileName, bool fullload)
{
 init();
 mTheme = theme;
 loadUnitType(fileName, fullload);
}

void UnitProperties::init()
{
 d = new UnitPropertiesPrivate;
 mTheme = 0;
 mMobileProperties = 0;
 mFacilityProperties = 0;
 mProduceAction = 0;
 d->mPlugins.setAutoDelete(true);
}

UnitProperties::~UnitProperties()
{
 delete mMobileProperties;
 delete mFacilityProperties;
 delete mProduceAction;
 delete d;
}

bool UnitProperties::loadUnitType(const QString& fileName, bool fullmode)
{
 mFullMode = fullmode;
 bool isFacility;
 QFile file(fileName);
 if (!file.open(IO_ReadOnly)) {
	boError() << k_funcinfo << "could not open " << fileName << endl;
	return false;
 }
 KMD5 md5(file.readAll());
 d->mMD5 = md5.hexDigest();
 KSimpleConfig conf(fileName);
 conf.setGroup(QString::fromLatin1("Boson Unit"));

 d->mUnitPath = fileName.left(fileName.length() - QString("index.unit").length());
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
 mUnitWidth = conf.readDoubleNumEntry("UnitWidth", 1.0);
 mUnitHeight = (conf.readDoubleNumEntry("UnitHeight", 1.0));
 mUnitDepth = (float)conf.readDoubleNumEntry("UnitDepth", 1.0);
 d->mName = conf.readEntry("Name", i18n("Unknown"));
 mHealth = conf.readUnsignedLongNumEntry("Health", 100);
 mMineralCost = conf.readUnsignedLongNumEntry("MineralCost", 100);
 mOilCost = conf.readUnsignedLongNumEntry("OilCost", 0);
 mSightRange = conf.readUnsignedLongNumEntry("SightRange", 5);
 mProductionTime = conf.readUnsignedNumEntry("ProductionTime", 100);
 mShields = conf.readUnsignedLongNumEntry("Shield", 0);
 mArmor = conf.readUnsignedLongNumEntry("Armor", 0);
 mSupportMiniMap = conf.readBoolEntry("SupportMiniMap", false);
 isFacility = conf.readBoolEntry("IsFacility", false);
 d->mRequirements = BosonConfig::readUnsignedLongNumList(&conf, "Requirements");
 mExplodingDamage = conf.readLongNumEntry("ExplodingDamage", 0);
 mExplodingDamageRange = (float)(conf.readDoubleNumEntry("ExplodingDamageRange", 0));
 mExplodingFragmentCount = conf.readUnsignedNumEntry("ExplodingFragmentCount", 0);
 mExplodingFragmentDamage = conf.readLongNumEntry("ExplodingFragmentDamage", 10);
 mExplodingFragmentDamageRange = (float)conf.readDoubleNumEntry("ExplodingFragmentDamageRange", 0.5);
 mRemoveWreckageImmediately = conf.readBoolEntry("RemoveWreckageImmediately", false);
 d->mExplodingFragmentFlyEffectIds = BosonConfig::readUnsignedLongNumList(&conf, "ExplodingFragmentFlyEffects");
 d->mExplodingFragmentHitEffectIds = BosonConfig::readUnsignedLongNumList(&conf, "ExplodingFragmentHitEffects");
 if (mFullMode) {
	d->mExplodingFragmentFlyEffects = BosonEffectProperties::loadEffectProperties(d->mExplodingFragmentFlyEffectIds);
	d->mExplodingFragmentHitEffects = BosonEffectProperties::loadEffectProperties(d->mExplodingFragmentHitEffectIds);
 }
 d->mHitPoint = BosonConfig::readBoVector3Entry(&conf, "HitPoint");  // FIXME: better name
 d->mHitPoint.cellToCanvas();

 d->mDestroyedEffectIds = BosonConfig::readUnsignedLongNumList(&conf, "DestroyedEffects");
 d->mConstructedEffectIds = BosonConfig::readUnsignedLongNumList(&conf, "ConstructedEffects");
 if (mFullMode) {
	d->mDestroyedEffects = BosonEffectProperties::loadEffectProperties(d->mDestroyedEffectIds);
	d->mConstructedEffects = BosonEffectProperties::loadEffectProperties(d->mConstructedEffectIds);
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
 loadWeapons(&conf);
 return true;
}

void UnitProperties::saveUnitType(const QString& fileName)
{
 d->mUnitPath = fileName.left(fileName.length() - QString("index.unit").length());
 KSimpleConfig conf(fileName);
 conf.setGroup(QString::fromLatin1("Boson Unit"));

 conf.writeEntry("Id", typeId());
 conf.writeEntry("TerrainType", (int)mTerrain);
 conf.writeEntry("UnitWidth", mUnitWidth);
 conf.writeEntry("UnitHeight", mUnitHeight);
 conf.writeEntry("UnitDepth", (double)mUnitDepth);
 conf.writeEntry("Name", d->mName);
 conf.writeEntry("Health", mHealth);
 conf.writeEntry("MineralCost", mMineralCost);
 conf.writeEntry("OilCost", mOilCost);
 conf.writeEntry("SightRange", mSightRange);
 conf.writeEntry("ProductionTime", mProductionTime);
 conf.writeEntry("Shield", mShields);
 conf.writeEntry("Armor", mArmor);
 conf.writeEntry("SupportMiniMap", mSupportMiniMap);
 conf.writeEntry("IsFacility", isFacility());
 BosonConfig::writeUnsignedLongNumList(&conf, "Requirements", d->mRequirements);
 conf.writeEntry("ExplodingDamage", mExplodingDamage);
 conf.writeEntry("ExplodingDamageRange", mExplodingDamageRange);
 BoVector3 tmpHitPoint(d->mHitPoint);
 tmpHitPoint.canvasToCell();
 BosonConfig::writeEntry(&conf, "HitPoint", tmpHitPoint);
 conf.writeEntry("Producer", mProducer);

 BosonConfig::writeUnsignedLongNumList(&conf, "DestroyedEffects", d->mDestroyedEffectIds);
 BosonConfig::writeUnsignedLongNumList(&conf, "ConstructedEffects", d->mConstructedEffectIds);

 if (isFacility()) {
	saveFacilityProperties(&conf);
 } else {
	saveMobileProperties(&conf);
 }

 saveAllPluginProperties(&conf);  // This saves weapons too
 saveTextureNames(&conf);
 saveSoundNames(&conf);
}

void UnitProperties::loadMobileProperties(KSimpleConfig* conf)
{
 conf->setGroup("Boson Mobile Unit");
 createMobileProperties();
 mMobileProperties->mSpeed = (float)conf->readDoubleNumEntry("Speed", 0) / 48.0f;
 if (mMobileProperties->mSpeed < 0) {
	boWarning() << k_funcinfo << "Invalid Speed value: " << mMobileProperties->mSpeed <<
			" for unit " << typeId() << ", defaulting to 0" << endl;
	mMobileProperties->mSpeed = 0;
 }
 mMobileProperties->mAccelerationSpeed = (float)conf->readDoubleNumEntry("AccelerationSpeed", 0.1) / 48.0f;
 mMobileProperties->mDecelerationSpeed = (float)conf->readDoubleNumEntry("DecelerationSpeed", 0.2) / 48.0f;
 mMobileProperties->mRotationSpeed = conf->readNumEntry("RotationSpeed", (int)(mMobileProperties->mSpeed * 48.0f * 2));
 mMobileProperties->mCanGoOnLand = conf->readBoolEntry("CanGoOnLand",
		(isLand() || isAircraft()));
 mMobileProperties->mCanGoOnWater = conf->readBoolEntry("CanGoOnWater",
		(isShip() || isAircraft()));
}

void UnitProperties::loadFacilityProperties(KSimpleConfig* conf)
{
 conf->setGroup("Boson Facility");
 createFacilityProperties();
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
 if (conf->hasGroup(RefineryProperties::propertyGroup())) {
	boDebug() << k_funcinfo << "loading refinery plugin properties" << endl;
	loadPluginProperties(new RefineryProperties(this), conf);
 }
 if (conf->hasGroup(ResourceMineProperties::propertyGroup())) {
	loadPluginProperties(new ResourceMineProperties(this), conf);
 }
}

void UnitProperties::loadPluginProperties(PluginProperties* prop, KSimpleConfig* conf)
{
 if (!prop || !conf) {
	boError() << k_funcinfo << "oops" << endl;
	return;
 }
 prop->loadPlugin(conf);
 d->mPlugins.append(prop);
}

void UnitProperties::loadTextureNames(KSimpleConfig* conf)
{
 if (!conf->hasGroup("Textures")) {
	return;
 }
 d->mTextureNames.clear();
 conf->setGroup("Textures");
 QStringList textures = conf->readListEntry("Textures");
 for (unsigned int i = 0; i < textures.count(); i++) {
	QString longName = conf->readEntry(textures[i], QString::null);
	if (!longName.isEmpty()) {
		d->mTextureNames.insert(textures[i], longName);
		boDebug() << "mapping " << textures[i] << "->" << longName << endl;
	}
 }
}

void UnitProperties::loadSoundNames(KSimpleConfig* conf)
{
 d->mSounds.clear();
 conf->setGroup("Sounds");
 d->mSounds.insert(SoundOrderMove, conf->readEntry("OrderMove", "order_move"));
 d->mSounds.insert(SoundOrderAttack, conf->readEntry("OrderAttack", "order_attack"));
 d->mSounds.insert(SoundOrderSelect, conf->readEntry("OrderSelect", "order_select"));
 d->mSounds.insert(SoundReportProduced, conf->readEntry("ReportProduced", "report_produced"));
 d->mSounds.insert(SoundReportDestroyed, conf->readEntry("ReportDestroyed", "report_destroyed"));
 d->mSounds.insert(SoundReportUnderAttack, conf->readEntry("ReportUnderAttack", "report_underattack"));
}

void UnitProperties::loadWeapons(KSimpleConfig* conf)
{
 conf->setGroup("Boson Unit");
 int num = conf->readNumEntry("Weapons", 0);
 mCanShootAtAirUnits = false;
 mCanShootAtLandUnits = false;
 mMaxAirWeaponRange = 0;
 mMaxLandWeaponRange = 0;
 for (int i = 0; i < num; i++) {
	conf->setGroup(QString("Weapon_%1").arg(i));
	BosonWeaponProperties* p = new BosonWeaponProperties(this, i + 1);
	p->loadPlugin(conf, mFullMode);
	d->mPlugins.append(p);
	if (!p->autoUse()) {
		continue;
	}
	if (p->canShootAtAirUnits()) {
		mCanShootAtAirUnits = true;
		if(p->range() > mMaxAirWeaponRange) {
			mMaxAirWeaponRange = p->range();
		}
	}
	if (p->canShootAtLandUnits()) {
		mCanShootAtLandUnits = true;
		if(p->range() > mMaxLandWeaponRange) {
			mMaxLandWeaponRange = p->range();
		}
	}
 }
 mMaxWeaponRange = QMAX(mMaxAirWeaponRange, mMaxLandWeaponRange);
}

void UnitProperties::loadActions()
{
 KSimpleConfig conf(unitPath() + "index.unit");
 // Produce action first
 // Produce is special because it uses little overview as pixmap and it's
 //  tooltip text is auto-generated. Only thing we load here is hotkey
 // TODO: load hotkey
 conf.setGroup("Actions");
 mProduceAction = new BoAction(QString("ProduceAction-%1").arg(typeId()), theme()->smallOverview(typeId()),
		"Produce"/*, hotkey*/);
 if (canShoot()) {
	d->mActions.insert(ActionAttack, theme()->action(conf.readEntry("ActionAttack", "ActionAttack")));
 }
 if (isMobile()) {
	d->mActions.insert(ActionMove, theme()->action(conf.readEntry("ActionMove", "ActionMove")));
	d->mActions.insert(ActionFollow, theme()->action(conf.readEntry("ActionFollow", "ActionFollow")));
 }
 if (properties(PluginProperties::Harvester)) {
	d->mActions.insert(ActionHarvest, theme()->action(conf.readEntry("ActionHarvest", "ActionHarvest")));
 }
 if (properties(PluginProperties::Repair)) {
	d->mActions.insert(ActionHarvest, theme()->action(conf.readEntry("ActionRepair", "ActionRepair")));
 }
 if (!d->mActions.isEmpty()) {
	d->mActions.insert(ActionStop, theme()->action(conf.readEntry("ActionStop", "ActionStop")));
 }
}

void UnitProperties::saveMobileProperties(KSimpleConfig* conf)
{
 conf->setGroup("Boson Mobile Unit");
 conf->writeEntry("Speed", (double)(mMobileProperties->mSpeed  * 48.0f));
 conf->writeEntry("AccelerationSpeed", (double)(mMobileProperties->mAccelerationSpeed  * 48.0f));
 conf->writeEntry("DecelerationSpeed", (double)(mMobileProperties->mDecelerationSpeed  * 48.0f));
 conf->writeEntry("RotationSpeed", mMobileProperties->mRotationSpeed);
 conf->writeEntry("CanGoOnLand", mMobileProperties->mCanGoOnLand);
 conf->writeEntry("CanGoOnWater", mMobileProperties->mCanGoOnWater);
}

void UnitProperties::saveFacilityProperties(KSimpleConfig* conf)
{
 conf->setGroup("Boson Facility");
 conf->writeEntry("ConstructionSteps", mFacilityProperties->mConstructionFrames);
}

void UnitProperties::saveAllPluginProperties(KSimpleConfig* conf)
{
 int weaponcounter = 0;
 QPtrListIterator<PluginProperties> it(d->mPlugins);
 while (it.current()) {
	if (it.current()->pluginType() == PluginProperties::Weapon)
	{
		conf->setGroup(QString("Weapon_%1").arg(weaponcounter++));
	}
	it.current()->savePlugin(conf);
	++it;
 }
 conf->setGroup("Boson Unit");
 conf->writeEntry("Weapons", weaponcounter);
}

void UnitProperties::saveTextureNames(KSimpleConfig* conf)
{
 if (d->mTextureNames.count() == 0) {
	return;
 }
 conf->setGroup("Textures");
 QMap<QString, QString>::Iterator it;
 QStringList textures;
 for (it = d->mTextureNames.begin(); it != d->mTextureNames.end(); ++it) {
	textures.append(it.key());
	conf->writeEntry(it.key(), it.data());
 }
 conf->writeEntry("Textures", textures);
}

void UnitProperties::saveSoundNames(KSimpleConfig* conf)
{
 conf->setGroup("Sounds");
 conf->writeEntry("OrderMove", d->mSounds[SoundOrderMove]);
 conf->writeEntry("OrderAttack", d->mSounds[SoundOrderAttack]);
 conf->writeEntry("OrderSelect", d->mSounds[SoundOrderSelect]);
 conf->writeEntry("ReportProduced", d->mSounds[SoundReportProduced]);
 conf->writeEntry("ReportDestroyed", d->mSounds[SoundReportDestroyed]);
 conf->writeEntry("ReportUnderAttack", d->mSounds[SoundReportUnderAttack]);
}

const QCString& UnitProperties::md5() const
{
 return d->mMD5;
}

void UnitProperties::setName(const QString& n)
{
 d->mName = n;
}

const QString& UnitProperties::name() const
{
 return d->mName;
}

const QString& UnitProperties::unitPath() const
{
 return d->mUnitPath;
}

const QPtrList<PluginProperties>* UnitProperties::plugins() const
{
 return &d->mPlugins;
}

QMap<QString, QString> UnitProperties::longTextureNames() const
{
 return d->mTextureNames;
}

QValueList<unsigned long int> UnitProperties::requirements() const
{
 return d->mRequirements;
}

QString UnitProperties::sound(int soundEvent) const
{
 return d->mSounds[soundEvent];
}

QMap<int, QString> UnitProperties::sounds() const
{
 return d->mSounds;
}

BoAction* UnitProperties::action(UnitAction type) const
{
 return d->mActions[type];
}

const QIntDict<BoAction>* UnitProperties::allActions() const
{
 return &d->mActions;
}

void UnitProperties::setRequirements(QValueList<unsigned long int> requirements)
{
 d->mRequirements = requirements;
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

float UnitProperties::accelerationSpeed() const
{
 if (!mMobileProperties) {
	return 0;
 }
 return mMobileProperties->mAccelerationSpeed;
}

float UnitProperties::decelerationSpeed() const
{
 if (!mMobileProperties) {
	return 0;
 }
 return mMobileProperties->mDecelerationSpeed;
}

int UnitProperties::rotationSpeed() const
{
 if (!mMobileProperties) {
	return 0;
 }
 return mMobileProperties->mRotationSpeed;
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

unsigned int UnitProperties::constructionSteps() const
{
 if (!mFacilityProperties) {
	return 0;
 }
 return mFacilityProperties->mConstructionFrames;
}

const PluginProperties* UnitProperties::properties(int pluginType) const
{
 QPtrListIterator<PluginProperties> it(d->mPlugins);
 for (; it.current(); ++it) {
	if (it.current()->pluginType() == pluginType) {
		return it.current();
	}
 }
 return 0;
}

QPtrList<BosonEffect> UnitProperties::newDestroyedEffects(float x, float y, float z) const
{
 return BosonEffectProperties::newEffects(&d->mDestroyedEffects, BoVector3(x, y, z));
}

QPtrList<BosonEffect> UnitProperties::newConstructedEffects(float x, float y, float z) const
{
 return BosonEffectProperties::newEffects(&d->mConstructedEffects, BoVector3(x, y, z));
}

QPtrList<BosonEffect> UnitProperties::newExplodingFragmentFlyEffects(BoVector3 pos) const
{
 return BosonEffectProperties::newEffects(&d->mExplodingFragmentFlyEffects, pos);
}

QPtrList<BosonEffect> UnitProperties::newExplodingFragmentHitEffects(BoVector3 pos) const
{
 return BosonEffectProperties::newEffects(&d->mExplodingFragmentHitEffects, pos);
}

void UnitProperties::clearPlugins(bool deleteweapons)
{
 // FIXME: deleteweapons is very ugly hack here. In unit editor, we store
 //  pointers to units, so we must not delete weapons here
 if (!deleteweapons) {
	d->mPlugins.setAutoDelete(false);
	PluginProperties* p = d->mPlugins.first();
	while (p) {
		if (p->pluginType() != PluginProperties::Weapon) {
			delete p;
		}
		d->mPlugins.remove();
		p = d->mPlugins.current();
	}
	d->mPlugins.setAutoDelete(true);
 } else {
	d->mPlugins.clear();
 }
 delete mMobileProperties;
 mMobileProperties = 0;
 delete mFacilityProperties;
 mFacilityProperties = 0;
}

void UnitProperties::createMobileProperties()
{
 mMobileProperties = new MobileProperties;
}

void UnitProperties::createFacilityProperties()
{
 mFacilityProperties = new FacilityProperties;
}

void UnitProperties::addPlugin(PluginProperties* prop)
{
 d->mPlugins.append(prop);
}

void UnitProperties::setConstructionSteps(unsigned int steps)
{
 if (mFacilityProperties) {
	mFacilityProperties->mConstructionFrames = steps;
 }
}

void UnitProperties::setSpeed(float speed)
{
 if (mMobileProperties) {
	mMobileProperties->mSpeed = speed;
 }
}

void UnitProperties::setAccelerationSpeed(float speed)
{
 if (mMobileProperties) {
	mMobileProperties->mAccelerationSpeed = speed;
 }
}

void UnitProperties::setDecelerationSpeed(float speed)
{
 if (mMobileProperties) {
	mMobileProperties->mDecelerationSpeed = speed;
 }
}

void UnitProperties::setRotationSpeed(int speed)
{
 if (mMobileProperties) {
	mMobileProperties->mRotationSpeed = speed;
 }
}

void UnitProperties::setCanGoOnLand(bool c)
{
 if (mMobileProperties) {
	mMobileProperties->mCanGoOnLand = c;
 }
}

void UnitProperties::setCanGoOnWater(bool c)
{
 if (mMobileProperties) {
	mMobileProperties->mCanGoOnWater = c;
 }
}

void UnitProperties::addTextureMapping(QString shortname, QString longname)
{
 d->mTextureNames.insert(shortname, longname);
}

void UnitProperties::addSound(int event, QString filename)
{
 d->mSounds.insert(event, filename);
}

void UnitProperties::setDestroyedEffectIds(QValueList<unsigned long int> ids)
{
 d->mDestroyedEffectIds = ids;
}

QValueList<unsigned long int> UnitProperties::destroyedEffectIds() const
{
 return d->mDestroyedEffectIds;
}

void UnitProperties::setConstructedEffectIds(QValueList<unsigned long int> ids)
{
 d->mConstructedEffectIds = ids;
}

QValueList<unsigned long int> UnitProperties::constructedEffectIds() const
{
 return d->mConstructedEffectIds;
}

void UnitProperties::reset()
{
 if (mFullMode) {
	// UnitProperties should be never reset in full mode (aka game mode)
	boWarning() << k_funcinfo << "Resetting UnitProperties in full mode!!!" << endl;
 }
 clearPlugins(false); // reset() is only used by unit editor (this far), so don't delete weapons
 // Set variables to default values
 d->mUnitPath = "";
 mTypeId = 0;
 mTerrain = (TerrainType)0;
 mUnitWidth = 1.0f;
 mUnitHeight = 1.0f;
 mUnitDepth = 1.0;
 d->mName = i18n("Unknown");
 mHealth = 100;
 mMineralCost = 100;
 mOilCost = 0;
 mSightRange = 5;
 mProductionTime = 100;
 mShields = 0;
 mArmor = 0;
 mSupportMiniMap = false;
 d->mRequirements.clear();
 d->mDestroyedEffectIds.clear();
 d->mConstructedEffectIds.clear();
 d->mHitPoint.reset();
 mProducer = 0;
 mExplodingDamage = 0;
 mExplodingDamageRange = 0;
 // Delete old mobile/facility properties
 delete mMobileProperties;
 mMobileProperties = 0;
 delete mFacilityProperties;
 mFacilityProperties = 0;
 // Mobile stuff (because unit is mobile by default)
 createMobileProperties();
 mMobileProperties->mSpeed = 0 / 48.0f; // Hmm, this doesn't make any sense IMO
 mMobileProperties->mAccelerationSpeed = 0.5 / 48.0f;
 mMobileProperties->mDecelerationSpeed = 1.0 / 48.0f;
 mMobileProperties->mRotationSpeed = (int)(2 * mMobileProperties->mSpeed * 48.0f);
 mMobileProperties->mCanGoOnLand = true;
 mMobileProperties->mCanGoOnWater = false;
 // Sounds
 d->mSounds.clear();
 d->mSounds.insert(SoundOrderMove, "order_move");
 d->mSounds.insert(SoundOrderAttack, "order_attack");
 d->mSounds.insert(SoundOrderSelect, "order_select");
 d->mSounds.insert(SoundReportProduced, "report_produced");
 d->mSounds.insert(SoundReportDestroyed, "report_destroyed");
 d->mSounds.insert(SoundReportUnderAttack, "report_underattack");
 // Clear other lists
 d->mTextureNames.clear();
}

void UnitProperties::setHitPoint(const BoVector3& hitpoint)
{
 d->mHitPoint = hitpoint;
}

const BoVector3& UnitProperties::hitPoint() const
{
 return d->mHitPoint;
}

const BosonWeaponProperties* UnitProperties::weaponProperties(unsigned long int id) const
{
 QPtrListIterator<PluginProperties> it(d->mPlugins);
 while (it.current()) {
	if (it.current()->pluginType() == PluginProperties::Weapon) {
		if (((BosonWeaponProperties*)it.current())->id() == id) {
			return (BosonWeaponProperties*)it.current();
		}
	}
	++it;
 }
 return 0l;
}

