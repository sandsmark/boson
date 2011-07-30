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


#include "unitbase.h"

#include "../bomemory/bodummymemory.h"
#include "unitproperties.h"
#include "bosonitempropertyhandler.h"
#include "player.h"
#include "speciestheme.h"
#include "bosoncanvas.h"
#include "upgradeproperties.h"
#include "bosonpropertyxml.h"
#include "bodebug.h"

#include <k3staticdeleter.h>

#include <qdom.h>
#include <qmap.h>
#include <Q3IntDict>
//Added by qt3to4:
#include <Q3ValueList>

UnitBase::UnitBase(const UnitProperties* prop, Player* owner, BosonCanvas* canvas)
	: BosonItem(owner, canvas),
	mMaxHealth(prop, "Health", "MaxValue"),
	mMaxArmor(prop, "Armor", "MaxValue"),
	mMaxShields(prop, "Shields", "MaxValue"),
	mMaxSightRange(prop, "SightRange", "MaxValue"),
	mPowerGenerated(prop, "PowerGenerated", "MaxValue"),
	mPowerConsumed(prop, "PowerConsumed", "MaxValue")
{
 initStatic();
 mWeaponProperties = 0; // created on the fly in weaponDataHandler()
 mUnitProperties = prop; // WARNING: this might be 0 at this point! MUST be != 0 for Unit, but ScenarioUnit uses 0 here
 mAdvanceWasChargedThisAdvanceCall = false;

 mScheduledForSightUpdate = false;
 mScheduledForRadarUpdate = false;
 for (int i = 0; i < BOSON_MAX_PLAYERS; i++) {
	mRadarSignalStrength[i] = 0;
	mVisibleStatus[i] = VS_Never;
 }
 if (owner->isActiveGamePlayer()) {
	setVisibleStatus(owner->bosonId(), VS_Visible);
 }

 registerData(&mShieldReloadCounter, IdShieldReloadCounter);
 registerData(&mDeletionTimer, IdDeletionTimer);
 registerData(&mAdvanceWork, IdAdvanceWork);
 registerData(&mMovingStatus, IdMovingStatus);
 registerData(&mHealthFactor, IdHealthFactor);
 registerData(&mArmorFactor, IdArmorFactor);
 registerData(&mShieldsFactor, IdShieldsFactor);
 registerData(&mSightRangeFactor, IdSightRangeFactor);
 registerData(&mPowerChargeForAdvance, IdPowerChargeForAdvance);
 registerData(&mPowerChargeForReload, IdPowerChargeForReload);

 // these properties are fully internal (i.e. nothing is displayed in any
 // widget) and they change very often. So we increase speed greatly by not
 // emitting any signal.
 mDeletionTimer.setEmittingSignal(false);
 mShieldReloadCounter.setEmittingSignal(false);

 mAdvanceWork.setLocal((int)WorkIdle);
 mMovingStatus.setLocal((int)Standing);
 mShieldReloadCounter.setLocal(0);
 mDeletionTimer.setLocal(0);
 mHealthFactor.setLocal(0); // initially destroyed
 mArmorFactor.setLocal(0); // doesn't have any armor
 mShieldsFactor.setLocal(0); // doesn't have any shields
 mSightRangeFactor.setLocal(0);
 mPowerChargeForAdvance.setLocal(0);
 mPowerChargeForReload.setLocal(0);
}

UnitBase::~UnitBase()
{
// boDebug() << k_funcinfo << endl;
 if (mWeaponProperties) {
	// don't call weaponDataHandler() in d'tor, if it wasn't called before
	// (will create a datahandler)
	weaponDataHandler()->clear();
 }
 delete mWeaponProperties;
// boDebug() << k_funcinfo << " done" << endl;
}

QString UnitBase::getModelIdForItem() const
{
 if (!owner()) {
	BO_NULL_ERROR(owner());
	return QString();
 }
 if (!owner()->speciesTheme()) {
	BO_NULL_ERROR(owner()->speciesTheme());
	return QString();
 }
 if (!unitProperties()) {
	BO_NULL_ERROR(unitProperties());
	return QString();
 }
 return QString("%1:%2").arg("unit").arg(QString::number(unitProperties()->typeId()));
}

void UnitBase::initStatic()
{
 static bool initialized = false;
 if (initialized) {
	return;
 }
 addPropertyId(IdAdvanceWork, QString::fromLatin1("AdvanceWork"));
 addPropertyId(IdDeletionTimer, QString::fromLatin1("DeletionTimer"));
 addPropertyId(IdShieldReloadCounter, QString::fromLatin1("ShieldReloadCounter"));
 addPropertyId(IdMovingStatus, QString::fromLatin1("MovingStatus"));
 addPropertyId(IdHealthFactor, QString::fromLatin1("HealthFactor"));
 addPropertyId(IdArmorFactor, QString::fromLatin1("ArmorFactor"));
 addPropertyId(IdShieldsFactor, QString::fromLatin1("ShieldsFactor"));
 addPropertyId(IdSightRangeFactor, QString::fromLatin1("SightRangeFactor"));
 addPropertyId(IdPowerChargeForAdvance, QString::fromLatin1("PowerChargeForAdvance"));
 addPropertyId(IdPowerChargeForReload, QString::fromLatin1("PowerChargeForReload"));
 initialized = true;
}

const QString& UnitBase::name() const
{
 return unitProperties()->name();
}

quint32 UnitBase::maxSightRange() const
{
 return mMaxSightRange.value(upgradesCollection());
}

quint32 UnitBase::sightRange() const
{
 return (quint32)(sightRangeFactor() * maxSightRange());
}

void UnitBase::setSightRange(quint32 r)
{
 if (r > maxSightRange()) {
	r = maxSightRange();
 }
 if (maxSightRange() > 0) {
	setSightRangeFactor((bofixed)r / maxSightRange());
 } else {
	setSightRangeFactor(bofixed(1.0f));
 }
}

quint32 UnitBase::maxArmor() const
{
 return mMaxArmor.value(upgradesCollection());
}

quint32 UnitBase::armor() const
{
 return (quint32)(armorFactor() * maxArmor());
}

void UnitBase::setArmor(quint32 a)
{
 if (a > maxArmor()) {
	a = maxArmor();
 }
 if (maxArmor() > 0) {
	setArmorFactor((bofixed)a / maxArmor());
 } else {
	setArmorFactor(bofixed(1.0f));
 }
}

quint32 UnitBase::maxShields() const
{
 return mMaxShields.value(upgradesCollection());
}

quint32 UnitBase::shields() const
{
 return (quint32)(shieldsFactor() * maxShields());
}

void UnitBase::setShields(quint32 s)
{
 if (s > maxShields()) {
	s = maxShields();
 }
 if (maxShields() > 0) {
	setShieldsFactor((bofixed)s / maxShields());
 } else {
	setShieldsFactor(bofixed(1.0f));
 }
}

quint32 UnitBase::maxHealth() const
{
 return mMaxHealth.value(upgradesCollection());
}

quint32 UnitBase::health() const
{
 return (quint32)(healthFactor() * maxHealth());
}

void UnitBase::setHealth(quint32 h)
{
 if (h > maxHealth()) {
	h = maxHealth();
 }
 if (maxHealth() > 0) {
	setHealthFactor((bofixed)h / maxHealth());
 } else {
	setHealthFactor(bofixed(1.0f));
 }
}

quint32 UnitBase::powerConsumedByUnit() const
{
 return mPowerConsumed.value(upgradesCollection());
}

quint32 UnitBase::powerGeneratedByUnit() const
{
 return mPowerGenerated.value(upgradesCollection());
}

void UnitBase::chargePowerForAdvance(bofixed factor)
{
 if (mPowerChargeForAdvance >= 1) {
	boError() << k_funcinfo << "power charge (advance) already >= 1. must not happen." << endl;
	return;
 }
 mPowerChargeForAdvance = mPowerChargeForAdvance + factor;
}

void UnitBase::chargePowerForReload(bofixed factor)
{
 if (mPowerChargeForReload >= 1) {
	boError() << k_funcinfo << "power charge (reload) already >= 1. must not happen." << endl;
	return;
 }
 mPowerChargeForReload = mPowerChargeForReload + factor;
}

void UnitBase::unchargePowerForAdvance()
{
 if (mPowerChargeForAdvance >= 1) {
	mPowerChargeForAdvance = mPowerChargeForAdvance - 1;
 }
 mAdvanceWasChargedThisAdvanceCall = false;
}

void UnitBase::unchargePowerForReload()
{
 if (mPowerChargeForReload >= 1) {
	mPowerChargeForReload = mPowerChargeForReload - 1;
 }
}

bool UnitBase::requestPowerChargeForAdvance()
{
 if (mAdvanceWasChargedThisAdvanceCall) {
	// AB: in theory it would be invalid to call this more than once per
	// advance call. however sometimes units may call this indirectly, e.g.
	// a harvester calls "refinery->requestPowerChargeForAdvance()" to find
	// out whether the refinery is fully charged.
	// -> in such a case we may have multiple calls per advance call and in
	//    these cases it must be valid. therefore we do not emit an error.
	return isChargedForAdvance();
 }
 mAdvanceWasChargedThisAdvanceCall = true;
 chargePowerForAdvance(owner()->powerChargeForCurrentAdvanceCall());
 return isChargedForAdvance();
}

quint32 UnitBase::type() const
{
 return unitProperties()->typeId();
}

bool UnitBase::saveAsXML(QDomElement& root)
{
 if (!BosonItem::saveAsXML(root)) {
	return false;
 }
 QDomDocument doc = root.ownerDocument();
 root.setAttribute(QString::fromLatin1("Type"), (unsigned int)type());
 root.setAttribute(QString::fromLatin1("Group"), 0);
 root.setAttribute(QString::fromLatin1("GroupType"), 0);

 QDomElement upgrades = doc.createElement("Upgrades");
 root.appendChild(upgrades);
 if (!upgradesCollection().saveAsXML(upgrades)) {
	boError() << k_funcinfo << "Unable to save Upgrades" << endl;
	return false;
 }

 // save the weapon datahandler, if present
 if (mWeaponProperties) {
	BosonCustomPropertyXML propertyXML;
	QDomElement weaponHandler = doc.createElement(QString::fromLatin1("WeaponDataHandler"));
	if (!propertyXML.saveAsXML(weaponHandler, weaponDataHandler())) {
		boError() << k_funcinfo << "Unable to save weapon datahandler of unit " << id() << endl;
		return false;
	}
	root.appendChild(weaponHandler);
 }

 return true;
}

bool UnitBase::loadFromXML(const QDomElement& root)
{
 if (!BosonItem::loadFromXML(root)) {
	return false;
 }
 if (root.isNull()) {
	boError() << k_funcinfo << "NULL root node" << endl;
	return false;
 }

 clearUpgrades();
 QDomElement upgrades = root.namedItem("Upgrades").toElement();
 if (upgrades.isNull()) {
	boError(260) << k_funcinfo << "NULL Upgrades tag" << endl;
	return false;
 }
 if (!mUpgradesCollection.loadFromXML(speciesTheme(), upgrades)) {
	boError() << k_funcinfo << "Unable to save Upgrades" << endl;
	return false;
 }

 // the weapon data handler
 QDomElement weaponHandler = root.namedItem(QString::fromLatin1("WeaponDataHandler")).toElement();
 if (!weaponHandler.isNull()) {
	BosonCustomPropertyXML propertyXML;
	if (!propertyXML.loadFromXML(weaponHandler, weaponDataHandler())) {
		boError(260) << k_funcinfo << "unable to load unit weapon data handler (unit=" << this->id() << ")" << endl;
		return false;
	}
 }

 return true;
}

SpeciesTheme* UnitBase::speciesTheme() const
{
 if (!owner()) {
	boWarning() << k_funcinfo << "NULL owner" << endl;
	return 0;
 }
 return owner()->speciesTheme();
}

bool UnitBase::isFacility() const
{
 return unitProperties()->isFacility();
}

bool UnitBase::isMobile() const
{
 return unitProperties()->isMobile();
}

void UnitBase::increaseDeletionTimer()
{
 mDeletionTimer = mDeletionTimer + 1;
}

unsigned int UnitBase::deletionTimer() const
{
 return mDeletionTimer;
}

const PluginProperties* UnitBase::properties(int pluginType) const
{
 return unitProperties()->properties(pluginType);
}

bool UnitBase::saveScenario(QDomElement& unit)
{
 // FIXME: the unit ID still is a KGameProperty. Should be a normal integer (or
 // we just don't save it here!)

 bool ret = true;
 QMultiHash<int, KGamePropertyBase*> dict = dataHandler()->dict();
 QHashIterator<int, KGamePropertyBase*> it(dict);
 while (it.hasNext()) {
	it.next();
	QString s = dataHandler()->propertyValue(it.value());
	if (s.isNull()) {
		// AB: we need to connect to
		// KGamePropertyHandler::signalRequestValue if this ever
		// happens!
		boWarning() << k_funcinfo << "Cannot save property "
				<< it.value()->id() << "="
				<< dataHandler()->propertyName(it.value()->id())
				<< " to XML" << endl;
		ret = false; // saving basically failed. we continue anyway, maybe we can use the rest
		continue;
	}
	// AB: note that we mustn't use KGamePropertyHandler::propertyName() in
	// the XML! We would save a lot of memory by clearing the names out of
	// all properties
//	QDomElement unit = parent.ownerDocument().createElement("Unit");
	QDomElement property = unit.ownerDocument().createElement("Property");
	property.setAttribute(QString::fromLatin1("Id"), QString::number(it.value()->id()));
	// TODO: add an attribute with "name=..." - when loading first use the
	// Id, and if it's not present use the name. would make files more
	// readable. we need to write a propertyId->propertyName fuction, as we
	// can't use propertyName() (see above)
//	property.setAttribute(QString::fromLatin1("Name"), propertyName);

	property.setAttribute(QString::fromLatin1("Value"), s);
 }
 return ret;
}

void UnitBase::reloadShields(int by)
{
 // AB: the other way would be more clever - start with mShieldReloadCounter at
 // 10, decrease by "by" and actually reload shields when it reaches 0. the we
 // reset to 10 and so on.
 // would allow configurable reload times more easily.
 if (mShieldReloadCounter >= MAX_SHIELD_RELOAD_COUNT) {
	if (shields() < maxShields()) {
		setShields(shields() + 2);
	}
	mShieldReloadCounter = 0;
 } else {
	mShieldReloadCounter = mShieldReloadCounter + by;
 }
}

KGamePropertyHandler* UnitBase::weaponDataHandler()
{
 if (mWeaponProperties) {
	return (KGamePropertyHandler*)mWeaponProperties;
 }
 mWeaponProperties = new BosonItemPropertyHandler(this);
 mWeaponProperties->setPolicy(KGamePropertyBase::PolicyLocal); // fallback
 return (KGamePropertyHandler*)mWeaponProperties;
}

PlayerIO* UnitBase::ownerIO() const
{
 if (!owner()) {
	return 0;
 }
 return owner()->playerIO();
}

const Q3ValueList<const UpgradeProperties*>* UnitBase::upgrades() const
{
 return upgradesCollection().upgrades();
}

void UnitBase::clearUpgrades()
{
 mUpgradesCollection.clearUpgrades();
}

void UnitBase::addUpgrade(const UpgradeProperties* upgrade)
{
 BO_CHECK_NULL_RET(upgrade);
 mUpgradesCollection.addUpgrade(upgrade);

 if (!upgrade->upgradeUnit(this)) {
	boError() << k_funcinfo << "error while applying upgrade to Unit " << id() << endl;
	return;
 }
}

void UnitBase::removeUpgrade(const UpgradeProperties* upgrade)
{
 if (!upgrade) {
	return;
 }
 if (!mUpgradesCollection.removeUpgrade(upgrade)) {
	// nothing changed
	return;
 }

 if (!upgrade->downgradeUnit(this)) {
	boError() << k_funcinfo << "error while un-applying upgrade from Unit " << id() << endl;
	return;
 }
}
