/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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
#include "unitpropertiesprivate.h"

#include "defines.h"
#include "../bomemory/bodummymemory.h"
#include "speciestheme.h"
#include "pluginproperties.h"
#include "bosonweapon.h"
#include "bosonconfig.h"
#include "bodebug.h"
#include "bosonprofiling.h"
#include "upgradeproperties.h"

#include <ksimpleconfig.h>
#include <klocale.h>
#include <kmdcodec.h>

#include <qfile.h>

UnitProperties::UnitProperties(SpeciesTheme* theme, bool fullMode)
	: BoBaseValueCollection(),
	mHealth(this, "Health", "MaxValue"),
	mSightRange(this, "SightRange", "MaxValue"),
	mProductionTime(this, "ProductionTime", "MaxValue"),
	mMineralCost(this, "MineralCost", "MaxValue"),
	mOilCost(this, "OilCost", "MaxValue"),
	mArmor(this, "Armor", "MaxValue"),
	mShields(this, "Shields", "MaxValue"),
	mSpeed(this, "Speed", "MaxValue"),
	mAccelerationSpeed(this, "AccelerationSpeed", "MaxValue"),
	mDecelerationSpeed(this, "DecelerationSpeed", "MaxValue")
{
 init();
 mTheme = theme;
 mFullMode = fullMode;
}

void UnitProperties::init()
{
 d = new UnitPropertiesPrivate;
 mFullMode = true;
 mTheme = 0;
 mIsFacility = false;
 d->mPlugins.setAutoDelete(true);
}

UnitProperties::~UnitProperties()
{
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
 mUnitDepth = conf.readDoubleNumEntry("UnitDepth", 1.0);
 d->mName = conf.readEntry("Name", i18n("Unknown"));
 d->mDescription = conf.readEntry("Description", QString::null);
 insertULongBaseValue(conf.readUnsignedLongNumEntry("Health", 100), "Health", "MaxValue");
 insertULongBaseValue(conf.readUnsignedLongNumEntry("SightRange", 5), "SightRange", "MaxValue");
 // We convert this from seconds to advance calls
 insertULongBaseValue((unsigned long int)(conf.readDoubleNumEntry("ProductionTime", 5) * 20.0f), "ProductionTime", "MaxValue");
 insertULongBaseValue(conf.readUnsignedLongNumEntry("MineralCost", 100), "MineralCost", "MaxValue");
 insertULongBaseValue(conf.readUnsignedLongNumEntry("OilCost", 0), "OilCost", "MaxValue");
 insertULongBaseValue(conf.readUnsignedLongNumEntry("Armor", 0), "Armor", "MaxValue");
 insertULongBaseValue(conf.readUnsignedLongNumEntry("Shield", 0), "Shields", "MaxValue");
 mSupportMiniMap = conf.readBoolEntry("SupportMiniMap", false);
 isFacility = conf.readBoolEntry("IsFacility", false);
 d->mRequirements = BosonConfig::readUnsignedLongNumList(&conf, "Requirements");

 mExplodingDamage = conf.readLongNumEntry("ExplodingDamage", 0);
 mExplodingDamageRange = conf.readDoubleNumEntry("ExplodingDamageRange", 0);
 mExplodingFragmentCount = conf.readUnsignedNumEntry("ExplodingFragmentCount", 0);
 mExplodingFragmentDamage = conf.readLongNumEntry("ExplodingFragmentDamage", 10);
 mExplodingFragmentDamageRange = conf.readDoubleNumEntry("ExplodingFragmentDamageRange", 0.5);
 mRemoveWreckageImmediately = conf.readBoolEntry("RemoveWreckageImmediately", false);
 d->mExplodingFragmentFlyEffectIds = BosonConfig::readUnsignedLongNumList(&conf, "ExplodingFragmentFlyEffects");
 d->mExplodingFragmentHitEffectIds = BosonConfig::readUnsignedLongNumList(&conf, "ExplodingFragmentHitEffects");
 d->mHitPoint = BosonConfig::readBoVector3FixedEntry(&conf, "HitPoint", BoVector3Fixed(0, 0, mUnitDepth / 2.0f));  // FIXME: better name

 d->mDestroyedEffectIds = BosonConfig::readUnsignedLongNumList(&conf, "DestroyedEffects");
 d->mConstructedEffectIds = BosonConfig::readUnsignedLongNumList(&conf, "ConstructedEffects");

 mIsFacility = isFacility;

 // AB: note that we need to load _both_ in order to initialize all variables.
 loadFacilityProperties(&conf);
 loadMobileProperties(&conf);

 if (isFacility) {
	mProducer = conf.readUnsignedNumEntry("Producer", (unsigned int)CommandBunker);
 } else {
	mProducer = conf.readUnsignedNumEntry("Producer", (unsigned int)mTerrain);
 }

 loadAllPluginProperties(&conf);
 loadTextureNames(&conf);
 loadSoundNames(&conf);
 loadWeapons(&conf);

 if (!loadActions(&conf)) {
	boError() << k_funcinfo << "loading actions failed" << endl;
	return false;
 }

 return true;
}

bool UnitProperties::loadMobileProperties(KSimpleConfig* conf)
{
 conf->setGroup("Boson Mobile Unit");
 // We divide speeds with 20, because speeds in config files are cells/second,
 //  but we want cells/advance call
 insertBoFixedBaseValue(conf->readDoubleNumEntry("Speed", 0) / 20.0f, "Speed", "MaxValue");
 if (bofixedBaseValue("Speed") < 0) {
	boWarning() << k_funcinfo << "Invalid Speed value: " << bofixedBaseValue("Speed") <<
			" for unit " << typeId() << ", defaulting to 0" << endl;
	insertBoFixedBaseValue(0, "Speed", "MaxValue");
 }
 insertBoFixedBaseValue(conf->readDoubleNumEntry("AccelerationSpeed", 1) / 20.0f / 20.0f, "AccelerationSpeed", "MaxValue");
 insertBoFixedBaseValue(conf->readDoubleNumEntry("DecelerationSpeed", 2) / 20.0f / 20.0f, "DecelerationSpeed", "MaxValue");
 mRotationSpeed = (int)(conf->readNumEntry("RotationSpeed", (int)(bofixedBaseValue("Speed") * 20.0f * 90.0f)) / 20.0f);
 mCanGoOnLand = conf->readBoolEntry("CanGoOnLand", (isLand() || isAircraft()));
 mCanGoOnWater = conf->readBoolEntry("CanGoOnWater", (isShip() || isAircraft()));
 mCrushDamage = conf->readUnsignedLongNumEntry("CrushDamage", 0);
 mMaxSlope = conf->readDoubleNumEntry("MaxSlope", 30);
 mWaterDepth = conf->readDoubleNumEntry("WaterDepth", 0.25);

 // Those are relevant only for aircrafts
 mIsHelicopter = conf->readBoolEntry("IsHelicopter", false);
 mTurnRadius = conf->readDoubleNumEntry("TurnRadius", 5);
 mPreferredAltitude = conf->readDoubleNumEntry("PreferredAltitude", 3);
 return true;
}

bool UnitProperties::loadFacilityProperties(KSimpleConfig* conf)
{
 conf->setGroup("Boson Facility");
 mConstructionFrames = conf->readUnsignedNumEntry("ConstructionSteps", 20);
 return true;
}

bool UnitProperties::loadAllPluginProperties(KSimpleConfig* conf)
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
	loadPluginProperties(new RefineryProperties(this), conf);
 }
 if (conf->hasGroup(ResourceMineProperties::propertyGroup())) {
	loadPluginProperties(new ResourceMineProperties(this), conf);
 }
 return true;
}

bool UnitProperties::loadPluginProperties(PluginProperties* prop, KSimpleConfig* conf)
{
 if (!prop || !conf) {
	boError() << k_funcinfo << "oops" << endl;
	return false;
 }
 prop->loadPlugin(conf);
 d->mPlugins.append(prop);
 return true;
}

bool UnitProperties::loadTextureNames(KSimpleConfig* conf)
{
 if (!conf->hasGroup("Textures")) {
	return true;
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
 return true;
}

bool UnitProperties::loadSoundNames(KSimpleConfig* conf)
{
 d->mSounds.clear();
 conf->setGroup("Sounds");
 d->mSounds.insert(SoundOrderMove, conf->readEntry("OrderMove", "order_move"));
 d->mSounds.insert(SoundOrderAttack, conf->readEntry("OrderAttack", "order_attack"));
 d->mSounds.insert(SoundOrderSelect, conf->readEntry("OrderSelect", "order_select"));
 d->mSounds.insert(SoundReportProduced, conf->readEntry("ReportProduced", "report_produced"));
 d->mSounds.insert(SoundReportDestroyed, conf->readEntry("ReportDestroyed", "report_destroyed"));
 d->mSounds.insert(SoundReportUnderAttack, conf->readEntry("ReportUnderAttack", "report_underattack"));
 return true;
}

bool UnitProperties::loadWeapons(KSimpleConfig* conf)
{
 conf->setGroup("Boson Unit");
 int num = conf->readNumEntry("Weapons", 0);
 mCanShootAtAirUnits = false;
 mCanShootAtLandUnits = false;
 for (int i = 0; i < num; i++) {
	conf->setGroup(QString("Weapon_%1").arg(i));
	BosonWeaponProperties* p = new BosonWeaponProperties(this, i + 1);
	p->loadPlugin(conf);
	d->mPlugins.append(p);
	if (!p->autoUse()) {
		continue;
	}
	if (p->canShootAtAirUnits()) {
		mCanShootAtAirUnits = true;
	}
	if (p->canShootAtLandUnits()) {
		mCanShootAtLandUnits = true;
	}
 }
 return true;
}

bool UnitProperties::loadActions(KSimpleConfig* conf)
{
 BosonProfiler prof("LoadActions");
 // Produce action first
 // Produce is special because it uses little overview as pixmap and it's
 //  tooltip text is auto-generated. Only thing we load here is hotkey
 // TODO: load hotkey
 conf->setGroup("Actions");
 if (canShoot()) {
	d->mActionStrings.insert(ActionAttack, conf->readEntry("ActionAttack", "ActionAttack"));
 }
 if (isMobile()) {
	d->mActionStrings.insert(ActionMove, conf->readEntry("ActionMove", "ActionMove"));
	d->mActionStrings.insert(ActionFollow, conf->readEntry("ActionFollow", "ActionFollow"));
 }
 if (properties(PluginProperties::Harvester)) {
	d->mActionStrings.insert(ActionHarvest, conf->readEntry("ActionHarvest", "ActionHarvest"));
 }
 if (properties(PluginProperties::Repair)) {
	d->mActionStrings.insert(ActionHarvest, conf->readEntry("ActionRepair", "ActionRepair"));
 }
 if (!d->mActionStrings.isEmpty()) {
	d->mActionStrings.insert(ActionStop, conf->readEntry("ActionStop", "ActionStop"));
 }
 return true;
}

const QCString& UnitProperties::md5() const
{
 return d->mMD5;
}

const QString& UnitProperties::name() const
{
 return d->mName;
}

const QString& UnitProperties::description() const
{
 return d->mDescription;
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

QString UnitProperties::actionString(UnitAction type) const
{
 if (!d->mActionStrings.contains(type)) {
	return QString::null;
 }
 return d->mActionStrings[type];
}

const QMap<int, QString>* UnitProperties::allActionStrings() const
{
 return &d->mActionStrings;
}

bofixed UnitProperties::maxSpeed() const
{
 if (!isMobile()) {
	return bofixed(0);
 }
 return mSpeed.value(upgradesCollection());
}

bofixed UnitProperties::maxAccelerationSpeed() const
{
 if (!isMobile()) {
	return bofixed(0);
 }
 return mAccelerationSpeed.value(upgradesCollection());
}

bofixed UnitProperties::maxDecelerationSpeed() const
{
 if (!isMobile()) {
	return bofixed(0);
 }
 return mDecelerationSpeed.value(upgradesCollection());
}

int UnitProperties::rotationSpeed() const
{
 if (!isMobile()) {
	return 0;
 }
 return mRotationSpeed;
}

bool UnitProperties::isHelicopter() const
{
 if (!isMobile() || !isAircraft()) {
	return false;
 }
 return mIsHelicopter;
}

bofixed UnitProperties::turnRadius() const
{
 if (!isMobile() || !isAircraft()) {
	return 0;
 }
 return mTurnRadius;
}

bofixed UnitProperties::preferredAltitude() const
{
 if (!isMobile() || !isAircraft()) {
	return 0;
 }
 return mPreferredAltitude;
}

bool UnitProperties::canGoOnLand() const
{
 if (!isMobile()) {
	return true; // even facilities can go there.
 }
 return mCanGoOnLand;
}

bool UnitProperties::canGoOnWater() const
{
 if (!isMobile()) {
	return false;
 }
 return mCanGoOnWater;
}

unsigned int UnitProperties::constructionSteps() const
{
 if (!isFacility()) {
	return 0;
 }
 return mConstructionFrames;
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

const QValueList<unsigned long int>& UnitProperties::destroyedEffectIds() const
{
 return d->mDestroyedEffectIds;
}

const QValueList<unsigned long int>& UnitProperties::constructedEffectIds() const
{
 return d->mConstructedEffectIds;
}

const QValueList<unsigned long int>& UnitProperties::explodingFragmentFlyEffectIds() const
{
 return d->mExplodingFragmentFlyEffectIds;
}

const QValueList<unsigned long int>& UnitProperties::explodingFragmentHitEffectIds() const
{
 return d->mExplodingFragmentHitEffectIds;
}

const BoVector3Fixed& UnitProperties::hitPoint() const
{
 return d->mHitPoint;
}

BosonWeaponProperties* UnitProperties::nonConstWeaponProperties(unsigned long int id) const
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
 return 0;
}

const BosonWeaponProperties* UnitProperties::weaponProperties(unsigned long int id) const
{
 return nonConstWeaponProperties(id);
}

const BoUpgradesCollection& UnitProperties::upgradesCollection() const
{
 return d->mUpgradesCollection;
}

bool UnitProperties::saveUpgradesAsXML(QDomElement& root) const
{
 return upgradesCollection().saveAsXML(root);
}

bool UnitProperties::loadUpgradesFromXML(const QDomElement& root)
{
 if (!mTheme) {
	BO_NULL_ERROR(mTheme);
	return false;
 }
 return d->mUpgradesCollection.loadFromXML(mTheme, root);
}

void UnitProperties::clearUpgrades()
{
 d->mUpgradesCollection.clearUpgrades();
}

void UnitProperties::addUpgrade(const UpgradeProperties* upgrade)
{
 d->mUpgradesCollection.addUpgrade(upgrade);
}

void UnitProperties::removeUpgrade(const UpgradeProperties* upgrade)
{
 d->mUpgradesCollection.removeUpgrade(upgrade);
}

void UnitProperties::removeUpgrade(unsigned long int id)
{
 removeUpgrade(d->mUpgradesCollection.findUpgrade(id));
}

bool UnitProperties::canGo(int x, int y)
{
 if (isAircraft()) {
	// Aircrafts can go everywhere
	return true;
 } else {
	// TODO: implement this (we could use map's width here...Z)
	return true;
	//return moveData()->cellPassable[...];
 }
}

