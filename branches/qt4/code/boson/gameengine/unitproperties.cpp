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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

#include <klocale.h>
#include <kcodecs.h>
#include <KConfig>
#include <KConfigGroup>

#include <qfile.h>
//Added by qt3to4:
#include <Q3CString>
#include <Q3ValueList>
#include <Q3PtrList>

UnitProperties::UnitProperties(SpeciesTheme* theme)
	: BoBaseValueCollection(),
	mHealth(this, "Health", "MaxValue"),
	mSightRange(this, "SightRange", "MaxValue"),
	mProductionTime(this, "ProductionTime", "MaxValue"),
	mMineralCost(this, "MineralCost", "MaxValue"),
	mOilCost(this, "OilCost", "MaxValue"),
	mArmor(this, "Armor", "MaxValue"),
	mShields(this, "Shields", "MaxValue"),
	mPowerConsumed(this, "PowerConsumed", "MaxValue"),
	mPowerGenerated(this, "PowerGenerated", "MaxValue"),
	mSpeed(this, "Speed", "MaxValue"),
	mAccelerationSpeed(this, "AccelerationSpeed", "MaxValue"),
	mDecelerationSpeed(this, "DecelerationSpeed", "MaxValue")
{
 init();
 mTheme = theme;
}

void UnitProperties::init()
{
 d = new UnitPropertiesPrivate;
 mTheme = 0;
 mIsFacility = false;
 d->mPlugins.setAutoDelete(true);
}

UnitProperties::~UnitProperties()
{
 delete d;
}

bool UnitProperties::loadUnitType(const QString& fileName)
{
 bool isFacility;
 QFile file(fileName);
 if (!file.open(QIODevice::ReadOnly)) {
	boError() << k_funcinfo << "could not open " << fileName << endl;
	return false;
 }
 KMD5 md5(file.readAll());
 d->mMD5 = md5.hexDigest();
 KConfig conf_(fileName, KConfig::SimpleConfig);
 KConfigGroup unitGroup = conf_.group("Boson Unit");

 d->mUnitPath = fileName.left(fileName.length() - QString("index.unit").length());
 /// FIXME: maybe rename to Type or TypeID (Id is confusing IMHO) - rivol
 mTypeId = unitGroup.readEntry("Id", (quint32)0); // 0 == invalid // Note: Id == Unit::type() , NOT Unit::id() !
 if (typeId() <= 0) {
	boError() << "Invalid TypeId: " << typeId() << " in unit file " << fileName << endl;
	return false;
 }
 mTerrain = (TerrainType)unitGroup.readEntry("TerrainType", (int)TerrainLand);
 switch (mTerrain) {
	case TerrainLand:
	case TerrainWater:
	case TerrainAirPlane:
	case TerrainAirHelicopter:
		break;
	default:
		// AB: atm only _one_ terrain type per unit is allowed.
		//     so e.g. mTerrain = (TerrainLand | TerrainWater) is not
		//     allowed.
		boWarning() << k_funcinfo << "Invalid TerrainType value: " << mTerrain << " for unit " << typeId() << ", defaulting to " << (int)TerrainLand << endl;
		mTerrain = TerrainLand;
		break;
 }
 mUnitWidth = unitGroup.readEntry("UnitWidth", 1.0);
 mUnitHeight = (unitGroup.readEntry("UnitHeight", 1.0));
 mUnitDepth = unitGroup.readEntry("UnitDepth", 1.0);
 d->mName = unitGroup.readEntry("Name", i18n("Unknown"));
 d->mDescription = unitGroup.readEntry("Description", QString());
 insertULongBaseValue(unitGroup.readEntry("Health", (quint32)100), "Health", "MaxValue");
 if (ulongBaseValue("Health") == 0) {
	boError() << k_funcinfo << "health of unit " << mTypeId << " (" << d->mName << ") is 0" << endl;
 }
 insertULongBaseValue(unitGroup.readEntry("SightRange", (quint32)5), "SightRange", "MaxValue");
 // We convert this from seconds to advance calls
 insertULongBaseValue((quint32)(unitGroup.readEntry("ProductionTime", 5.0) * 20.0f), "ProductionTime", "MaxValue");
 insertULongBaseValue(unitGroup.readEntry("MineralCost", (quint32)100), "MineralCost", "MaxValue");
 insertULongBaseValue(unitGroup.readEntry("OilCost", (quint32)0), "OilCost", "MaxValue");
 insertULongBaseValue(unitGroup.readEntry("Armor", (quint32)0), "Armor", "MaxValue");
 insertULongBaseValue(unitGroup.readEntry("Shields", (quint32)0), "Shields", "MaxValue");
 quint32 powerGenerated = unitGroup.readEntry("PowerGenerated", (quint32)0);
 quint32 powerConsumed = unitGroup.readEntry("PowerConsumed", (quint32)0);
 if (powerGenerated > 0 && powerConsumed > 0) {
	boWarning() << k_funcinfo << "both, PowerGenerated and PowerConsumed > 0" << endl;
	if (powerGenerated > powerConsumed) {
		powerGenerated -= powerConsumed;
		powerConsumed = 0;
	} else {
		powerConsumed -= powerGenerated;
		powerGenerated = 0;
	}
 }
 insertULongBaseValue(powerGenerated, "PowerGenerated", "MaxValue");
 insertULongBaseValue(powerConsumed, "PowerConsumed", "MaxValue");
 mSupportMiniMap = unitGroup.readEntry("SupportMiniMap", false);
 isFacility = unitGroup.readEntry("IsFacility", false);
 d->mRequirements = BosonConfig::readUnsignedLongNumList(&unitGroup, "Requirements");

 mExplodingDamage = unitGroup.readEntry("ExplodingDamage", (qint32)0);
 mExplodingDamageRange = unitGroup.readEntry("ExplodingDamageRange", 0.0);
 mExplodingFragmentCount = unitGroup.readEntry("ExplodingFragmentCount", (quint32)0);
 mExplodingFragmentDamage = unitGroup.readEntry("ExplodingFragmentDamage", (qint32)10);
 mExplodingFragmentDamageRange = unitGroup.readEntry("ExplodingFragmentDamageRange", 0.5);
 mRemoveWreckageImmediately = unitGroup.readEntry("RemoveWreckageImmediately", false);
 d->mExplodingFragmentFlyEffectIds = BosonConfig::readUnsignedLongNumList(&unitGroup, "ExplodingFragmentFlyEffects");
 d->mExplodingFragmentHitEffectIds = BosonConfig::readUnsignedLongNumList(&unitGroup, "ExplodingFragmentHitEffects");
 d->mHitPoint = BosonConfig::readBoVector3FixedEntry(&unitGroup, "HitPoint", BoVector3Fixed(0, 0, mUnitDepth / 2.0f));  // FIXME: better name

 d->mDestroyedEffectIds = BosonConfig::readUnsignedLongNumList(&unitGroup, "DestroyedEffects");
 d->mConstructedEffectIds = BosonConfig::readUnsignedLongNumList(&unitGroup, "ConstructedEffects");

 mIsFacility = isFacility;

 // AB: note that we need to load _both_ in order to initialize all variables.
 loadFacilityProperties(&conf_);
 loadMobileProperties(&conf_);

 if (isFacility) {
	mProducer = unitGroup.readEntry("Producer", (quint32)ProducerCommandBunker);
 } else {
	quint32 defaultProducer = ProducerWarFactory;
	if (mTerrain & TerrainLand) {
		defaultProducer = ProducerWarFactory;
	} else if (mTerrain & TerrainWater) {
		defaultProducer = ProducerShipyard;
	} else if (mTerrain & TerrainMaskAir) {
		defaultProducer = ProducerAirport;
	}
	mProducer = unitGroup.readEntry("Producer", defaultProducer);
 }

 if (!loadAllPluginProperties(&conf_)) {
	boError() << k_funcinfo << fileName << ": failed loading plugin properties" << endl;
	return false;
 }
 if (!loadTextureNames(&conf_)) {
	boError() << k_funcinfo << fileName << ": failed loading texture names" << endl;
	return false;
 }
 if (!loadSoundNames(&conf_)) {
	boError() << k_funcinfo << fileName << ": failed loading sound names" << endl;
	return false;
 }
 if (!loadWeapons(&conf_)) {
	boError() << k_funcinfo << fileName << ": failed loading weapons" << endl;
	return false;
 }

 if (!loadActions(&conf_)) {
	boError() << k_funcinfo << "loading actions failed" << endl;
	return false;
 }

 return true;
}

bool UnitProperties::loadMobileProperties(KConfig* conf_)
{
 KConfigGroup group = conf_->group("Boson Mobile Unit");
 // We divide speeds with 20, because speeds in config files are cells/second,
 //  but we want cells/advance call
 insertBoFixedBaseValue(group.readEntry("Speed", 0.0) / 20.0f, "Speed", "MaxValue");
 if (bofixedBaseValue("Speed") < 0) {
	boWarning() << k_funcinfo << "Invalid Speed value: " << bofixedBaseValue("Speed") <<
			" for unit " << typeId() << ", defaulting to 0" << endl;
	insertBoFixedBaseValue(0, "Speed", "MaxValue");
 }
 insertBoFixedBaseValue(group.readEntry("AccelerationSpeed", 1.0) / 20.0f / 20.0f, "AccelerationSpeed", "MaxValue");
 insertBoFixedBaseValue(group.readEntry("DecelerationSpeed", 2.0) / 20.0f / 20.0f, "DecelerationSpeed", "MaxValue");
 // Different default values for aircrafts
 if (isAircraft()) {
	mRotationSpeed = group.readEntry("RotationSpeed", (float)((bofixedBaseValue("Speed") * 20.0f * 15.0f))) / 20.0f;
 } else {
	mRotationSpeed = group.readEntry("RotationSpeed", (float)((bofixedBaseValue("Speed") * 20.0f * 90.0f))) / 20.0f;
 }
 mCanGoOnLand = group.readEntry("CanGoOnLand", (isLand() || isAircraft()));
 mCanGoOnWater = group.readEntry("CanGoOnWater", (isShip() || isAircraft()));
 mCrushDamage = group.readEntry("CrushDamage", (quint32)0);
 mMaxSlope = group.readEntry("MaxSlope", 30.0);
 mWaterDepth = group.readEntry("WaterDepth", 0.25);

 // Those are relevant only for aircrafts
 mPreferredAltitude = group.readEntry("PreferredAltitude", 3.0);
 return true;
}

bool UnitProperties::loadFacilityProperties(KConfig* conf)
{
 KConfigGroup group = conf->group("Boson Facility");
 if (!group.isValid()) {
	return false;
 }
 mConstructionFrames = group.readEntry("ConstructionSteps", (quint32)20);
 return true;
}

bool UnitProperties::loadAllPluginProperties(KConfig* conf)
{
#define TRY_LOAD_PROPERTIES(x) \
	{ \
		KConfigGroup group = conf->group(x::propertyGroup()); \
		if (group.isValid()) { \
			if (!loadPluginProperties(new x(this), group)) { \
				return false; \
			} \
		} \
	}

 TRY_LOAD_PROPERTIES(ProductionProperties);
 TRY_LOAD_PROPERTIES(RepairProperties);
 TRY_LOAD_PROPERTIES(HarvesterProperties);
 TRY_LOAD_PROPERTIES(RefineryProperties);
 TRY_LOAD_PROPERTIES(ResourceMineProperties);
 TRY_LOAD_PROPERTIES(AmmunitionStorageProperties);
 TRY_LOAD_PROPERTIES(RadarProperties);
 TRY_LOAD_PROPERTIES(RadarJammerProperties);

#undef TRY_LOAD_PROPERTIES


 if (conf->hasGroup(UnitStorageProperties::propertyGroup())) {
	if (isMobile()) {
		boError() << k_funcinfo << "atm mobile units are not allowed to have a UnitStorage plugin: first we must ensure that while a unit is entering or leaving, the storage unit cannot move!" << endl;
		boError() << k_funcinfo << "also if units are inside and the storage moves, the stored units must be moved along (and probably not be visible on the screen)" << endl;
		return false;
	}
	KConfigGroup group = conf->group(UnitStorageProperties::propertyGroup());
	if (!loadPluginProperties(new UnitStorageProperties(this), group)) {
		return false;
	}
 }
 return true;
}

// takes ownership of prop!
bool UnitProperties::loadPluginProperties(PluginProperties* prop, const KConfigGroup& conf)
{
 if (!prop) {
	BO_NULL_ERROR(prop);
	return false;
 }
 if (!conf.isValid()) {
	BO_NULL_ERROR(prop);
	delete prop;
	return false;
 }
 if (!prop->loadPlugin(conf)) {
	boError() << k_funcinfo << "unable to load PluginProperties " << prop->pluginType() << endl;
	delete prop;
	return false;
 }
 d->mPlugins.append(prop);
 return true;
}

bool UnitProperties::loadTextureNames(KConfig* conf)
{
 if (!conf->hasGroup("Textures")) {
	return true;
 }
 KConfigGroup group = conf->group("Textures");
 d->mTextureNames.clear();
 QStringList textures = group.readEntry("Textures", QStringList());
 for (int i = 0; i < textures.count(); i++) {
	QString longName = group.readEntry(textures[i], QString());
	if (!longName.isEmpty()) {
		d->mTextureNames.insert(textures[i], longName);
		boDebug() << "mapping " << textures[i] << "->" << longName << endl;
	}
 }
 return true;
}

bool UnitProperties::loadSoundNames(KConfig* conf)
{
 d->mSounds.clear();
 if (!conf->hasGroup("Sounds")) {
	return true;
 }
 KConfigGroup group = conf->group("Sounds");
 d->mSounds.insert(SoundOrderMove, group.readEntry("OrderMove", "order_move"));
 d->mSounds.insert(SoundOrderAttack, group.readEntry("OrderAttack", "order_attack"));
 d->mSounds.insert(SoundOrderSelect, group.readEntry("OrderSelect", "order_select"));
 d->mSounds.insert(SoundReportProduced, group.readEntry("ReportProduced", "report_produced"));
 d->mSounds.insert(SoundReportDestroyed, group.readEntry("ReportDestroyed", "report_destroyed"));
 d->mSounds.insert(SoundReportUnderAttack, group.readEntry("ReportUnderAttack", "report_underattack"));
 return true;
}

bool UnitProperties::loadWeapons(KConfig* conf)
{
 KConfigGroup group = conf->group("Boson Unit");
 qint32 num = group.readEntry("Weapons", (qint32)0);
 mCanShootAtAirUnits = false;
 mCanShootAtLandUnits = false;
 for (qint32 i = 0; i < num; i++) {
	KConfigGroup weaponGroup = conf->group(QString("Weapon_%1").arg(i));
	if (!weaponGroup.isValid()) {
		return false;
	}
	BosonWeaponProperties* p = new BosonWeaponProperties(this, i + 1);
	p->loadPlugin(weaponGroup);
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

bool UnitProperties::loadActions(KConfig* conf)
{
 BosonProfiler prof("LoadActions");
 // Produce action first
 // Produce is special because it uses little overview as pixmap and it's
 //  tooltip text is auto-generated. Only thing we load here is hotkey
 // TODO: load hotkey
 KConfigGroup group = conf->group("Actions");
 if (canShoot()) {
	d->mActionStrings.insert(ActionAttack, group.readEntry("ActionAttack", "ActionAttack"));
 }
 if (isMobile()) {
	d->mActionStrings.insert(ActionMove, group.readEntry("ActionMove", "ActionMove"));
	d->mActionStrings.insert(ActionFollow, group.readEntry("ActionFollow", "ActionFollow"));
 }
 if (properties(PluginProperties::Harvester)) {
	d->mActionStrings.insert(ActionHarvest, group.readEntry("ActionHarvest", "ActionHarvest"));
 }
 if (properties(PluginProperties::Repair)) {
	d->mActionStrings.insert(ActionHarvest, group.readEntry("ActionRepair", "ActionRepair"));
 }
 if (!d->mActionStrings.isEmpty()) {
	d->mActionStrings.insert(ActionStop, group.readEntry("ActionStop", "ActionStop"));
 }
 return true;
}

const Q3CString& UnitProperties::md5() const
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

const Q3PtrList<PluginProperties>* UnitProperties::plugins() const
{
 return &d->mPlugins;
}

QMap<QString, QString> UnitProperties::longTextureNames() const
{
 return d->mTextureNames;
}

Q3ValueList<quint32> UnitProperties::requirements() const
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
	return QString();
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

bofixed UnitProperties::rotationSpeed() const
{
 if (!isMobile()) {
	return 0;
 }
 return mRotationSpeed;
}

bool UnitProperties::isHelicopter() const
{
 return (mTerrain & TerrainAirHelicopter);
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
 Q3PtrListIterator<PluginProperties> it(d->mPlugins);
 for (; it.current(); ++it) {
	if (it.current()->pluginType() == pluginType) {
		return it.current();
	}
 }
 return 0;
}

const Q3ValueList<quint32>& UnitProperties::destroyedEffectIds() const
{
 return d->mDestroyedEffectIds;
}

const Q3ValueList<quint32>& UnitProperties::constructedEffectIds() const
{
 return d->mConstructedEffectIds;
}

const Q3ValueList<quint32>& UnitProperties::explodingFragmentFlyEffectIds() const
{
 return d->mExplodingFragmentFlyEffectIds;
}

const Q3ValueList<quint32>& UnitProperties::explodingFragmentHitEffectIds() const
{
 return d->mExplodingFragmentHitEffectIds;
}

const BoVector3Fixed& UnitProperties::hitPoint() const
{
 return d->mHitPoint;
}

BosonWeaponProperties* UnitProperties::nonConstWeaponProperties(quint32 id) const
{
 Q3PtrListIterator<PluginProperties> it(d->mPlugins);
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

const BosonWeaponProperties* UnitProperties::weaponProperties(quint32 id) const
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

void UnitProperties::removeUpgrade(quint32 id)
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

