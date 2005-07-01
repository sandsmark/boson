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


#include "unitbase.h"

#include "../bomemory/bodummymemory.h"
#include "unitproperties.h"
#include "items/bosonitempropertyhandler.h"
#include "player.h"
#include "speciestheme.h"
#include "bosoncanvas.h"
#include "bosonpropertyxml.h"
#include "bodebug.h"

#include <kstaticdeleter.h>

#include <qdom.h>
#include <qmap.h>

UnitBase::UnitBase(const UnitProperties* prop, Player* owner, BosonCanvas* canvas)
	: BosonItem(owner, canvas)
{
 initStatic();
 mWeaponProperties = 0; // created on the fly in weaponDataHandler()
 mUnitProperties = prop; // WARNING: this might be 0 at this point! MUST be != 0 for Unit, but ScenarioUnit uses 0 here

// PolicyLocal?
 registerData(&mArmor, IdArmor);
 registerData(&mShields, IdShields);
 registerData(&mShieldReloadCounter, IdShieldReloadCounter);
 registerData(&mSightRange, IdSightRange);
 registerData(&mWork, IdWork);
 registerData(&mAdvanceWork, IdAdvanceWork);
 registerData(&mDeletionTimer, IdDeletionTimer);
 registerData(&mMovingStatus, IdMovingStatus);
 registerData(&mHealthPercentage, IdHealthPercentage);

 // these properties are fully internal (i.e. nothing is displayed in any
 // widget) and they change very often. So we increase speed greatly by not
 // emitting any signal.
 mDeletionTimer.setEmittingSignal(false);
 mShieldReloadCounter.setEmittingSignal(false);

 mWork.setLocal((int)WorkIdle);
 mAdvanceWork.setLocal((int)WorkIdle);
 mMovingStatus.setLocal((int)Standing);
 mShields.setLocal(0); // doesn't have any shields
 mShieldReloadCounter.setLocal(0);
 mArmor.setLocal(0); // doesn't have any armor
 mSightRange.setLocal(0);
 mDeletionTimer.setLocal(0);
 mHealthPercentage.setLocal(0); // initially destroyed
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

BosonModel* UnitBase::getModelForItem() const
{
 BO_CHECK_NULL_RET0(owner());
 BO_CHECK_NULL_RET0(owner()->speciesTheme());
 BO_CHECK_NULL_RET0(unitProperties());
 return owner()->speciesTheme()->unitModel(unitProperties()->typeId());
}

void UnitBase::initStatic()
{
 static bool initialized = false;
 if (initialized) {
	return;
 }
 addPropertyId(IdArmor, QString::fromLatin1("Armor"));
 addPropertyId(IdShields, QString::fromLatin1("Shields"));
 addPropertyId(IdSightRange, QString::fromLatin1("SightRange"));
 addPropertyId(IdWork, QString::fromLatin1("Work"));
 addPropertyId(IdAdvanceWork, QString::fromLatin1("AdvanceWork"));
 addPropertyId(IdDeletionTimer, QString::fromLatin1("DeletionTimer"));
 addPropertyId(IdShieldReloadCounter, QString::fromLatin1("ShieldReloadCounter"));
 addPropertyId(IdMovingStatus, QString::fromLatin1("MovingStatus"));
 addPropertyId(IdHealthPercentage, QString::fromLatin1("HealthPercentage"));
 initialized = true;
}

const QString& UnitBase::name() const
{
 return unitProperties()->name();
}

unsigned long int UnitBase::armor() const
{
 return mArmor;
}

void UnitBase::setArmor(unsigned long int a)
{
 mArmor = a;
}

void UnitBase::setShields(unsigned long int s)
{
 mShields = s;
}

unsigned long int UnitBase::health() const
{
 return (int)(healthPercentage() * mUnitProperties->health());
}

void UnitBase::setHealth(unsigned long int h)
{
 setHealthPercentage((bofixed)h / mUnitProperties->health());
}

unsigned long int UnitBase::type() const
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

bool UnitBase::isFlying() const
{
 return (unitProperties() ? unitProperties()->isAircraft() : false);
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
 QIntDict<KGamePropertyBase> dict = dataHandler()->dict();
 QIntDictIterator<KGamePropertyBase> it(dict);
 for (; it.current(); ++it) {
	QString s = dataHandler()->propertyValue(it.current());
	if (s.isNull()) {
		// AB: we need to connect to
		// KGamePropertyHandler::signalRequestValue if this ever
		// happens!
		boWarning() << k_funcinfo << "Cannot save property "
				<< it.current()->id() << "="
				<< dataHandler()->propertyName(it.current()->id())
				<< " to XML" << endl;
		ret = false; // saving basically failed. we continue anyway, maybe we can use the rest
		continue;
	}
	// AB: note that we mustn't use KGamePropertyHandler::propertyName() in
	// the XML! We would save a lot of memory by clearing the names out of
	// all properties
//	QDomElement unit = parent.ownerDocument().createElement("Unit");
	QDomElement property = unit.ownerDocument().createElement("Property");
	property.setAttribute(QString::fromLatin1("Id"), QString::number(it.current()->id()));
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
	if (shields() < unitProperties()->shields()) {
		setShields(shields() + 1);
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

