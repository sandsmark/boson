/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "unitproperties.h"
#include "unitpropertyhandler.h" // not related to unitproperties!
#include "player.h"
#include "speciestheme.h"
#include "bosonpropertyxml.h"
#include "bodebug.h"

#include <kstaticdeleter.h>

#include <qdom.h>
#include <qmap.h>

QMap<int, QString>* UnitBase::mPropertyMap = 0;
static KStaticDeleter< QMap<int, QString> > sd;

UnitBase::UnitBase(const UnitProperties* prop)
{
 if (!mPropertyMap) {
	initStatic();
 }
 mProperties = new UnitPropertyHandler(this);
 mProperties->setPolicy(KGamePropertyBase::PolicyLocal); // fallback
 mWeaponProperties = 0; // created on the fly in weaponDataHandler()
 mOwner = 0;
 mUnitProperties = prop; // WARNING: this might be 0 at this point! MUST be != 0 for Unit, but ScenarioUnit uses 0 here
 mIsMoving = false;

// PolicyLocal?
 registerData(&mHealth, IdHealth);
 registerData(&mArmor, IdArmor);
 registerData(&mShields, IdShields);
 registerData(&mShieldReloadCounter, IdShieldReloadCounter);
 registerData(&mSightRange, IdSightRange);
 registerData(&mWork, IdWork);
 registerData(&mAdvanceWork, IdAdvanceWork);
 registerData(&mDeletionTimer, IdDeletionTimer);

 // these properties are fully internal (i.e. nothing is displayed in any
 // widget) and they change very often. So we increase speed greatly by not
 // emitting any signal.
 mDeletionTimer.setEmittingSignal(false);
 mShieldReloadCounter.setEmittingSignal(false);

 mId = 0;


 mWork.setLocal((int)WorkNone);
 mAdvanceWork.setLocal((int)WorkNone);
 mHealth.setLocal(0); // initially destroyed
 mShields.setLocal(0); // doesn't have any shields
 mShieldReloadCounter.setLocal(0);
 mArmor.setLocal(0); // doesn't have any armor
 mSightRange.setLocal(0);
 mDeletionTimer.setLocal(0);
}

UnitBase::~UnitBase()
{
// boDebug() << k_funcinfo << endl;
 dataHandler()->clear();
 if (mWeaponProperties) {
	// don't call weaponDataHandler() in d'tor, if it wasn't called before
	// (will create a datahandler)
	weaponDataHandler()->clear();
 }
 delete mWeaponProperties;
 delete mProperties;
// boDebug() << k_funcinfo << " done" << endl;
}

void UnitBase::initStatic()
{
 if (mPropertyMap) {
	boError() << k_funcinfo << "Called twice" << endl;
	return;
 }
 delete mPropertyMap;
 mPropertyMap = new QMap<int, QString>;
 sd.setObject(mPropertyMap);
 addPropertyId(IdHealth, QString::fromLatin1("Health"));
 addPropertyId(IdArmor, QString::fromLatin1("Armor"));
 addPropertyId(IdShields, QString::fromLatin1("Shields"));
 addPropertyId(IdSightRange, QString::fromLatin1("SightRange"));
 addPropertyId(IdWork, QString::fromLatin1("Work"));
 addPropertyId(IdAdvanceWork, QString::fromLatin1("AdvanceWork"));
 addPropertyId(IdDeletionTimer, QString::fromLatin1("DeletionTimer"));
 addPropertyId(IdShieldReloadCounter, QString::fromLatin1("ShieldReloadCounter"));
}

void UnitBase::registerData(KGamePropertyBase* prop, int id, bool local)
{
 if (!prop) {
	boError() << k_funcinfo << "NULL property" << endl;
	return;
 }
 if (id < KGamePropertyBase::IdUser) {
	boWarning() << k_funcinfo << "ID < KGamePropertyBase::IdUser" << endl;
	// do not return - might still work
 }
 QString name = propertyName(id);
 if (name.isNull()) {
	boWarning() << k_funcinfo << "Invalid property name for " << id << endl;
	// a name isn't strictly necessary, so don't return
 }
 prop->registerData(id, dataHandler(),
		local ? KGamePropertyBase::PolicyLocal : KGamePropertyBase::PolicyClean,
		name);
}

void UnitBase::addPropertyId(int id, const QString& name)
{
 if (mPropertyMap->contains(id)) {
	boError() << k_funcinfo << "Cannot add " << id << " twice!" << endl;
	boError() << k_funcinfo << "Existing name: " << *mPropertyMap->find(id) << " ; provided name: " << name << endl;
	return;
 }
/* if (mPropertyMap->values().contains(name)) {
	boError() << k_funcinfo << "Cannot add " << name << " twice!" << endl;
	return;
 }*/
 mPropertyMap->insert(id, name);
}

QString UnitBase::propertyName(int id)
{
 if (!mPropertyMap->contains(id)) {
	return QString::null;
 }
 return (*mPropertyMap)[id];
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

unsigned long int UnitBase::type() const
{
 return unitProperties()->typeId();
}

void UnitBase::setOwner(Player* owner)
{
 mOwner = owner;
}

bool UnitBase::saveAsXML(QDomElement& root)
{
 QDomDocument doc = root.ownerDocument();
 root.setAttribute(QString::fromLatin1("UnitType"), (unsigned int)type());
 root.setAttribute(QString::fromLatin1("Id"), (unsigned int)id());

 // the data handler
 BosonCustomPropertyXML propertyXML;
 QDomElement handler = doc.createElement(QString::fromLatin1("DataHandler"));
 if (!propertyXML.saveAsXML(handler, dataHandler())) {
	boError() << k_funcinfo << "Unable to save datahandler of unit " << id() << endl;
	return false;
 }
 root.setAttribute(QString::fromLatin1("DataHandlerId"), dataHandler()->id());
 root.appendChild(handler);


 if (mWeaponProperties) {
	QDomElement weaponHandler = doc.createElement(QString::fromLatin1("WeaponDataHandler"));
	if (!propertyXML.saveAsXML(weaponHandler, weaponDataHandler())) {
		boError() << k_funcinfo << "Unable to save weapon datahandler of unit " << id() << endl;
		return false;
	}
	root.appendChild(weaponHandler);
 }
 return true;
}

bool UnitBase::save(QDataStream& stream)
{
 // TODO: we need to save and load Unit::mCurrentPlugin->pluginType() !!
 // note that multiple plugins of the same type are not *yet* supported! but
 // they might be one day..
 bool ret = dataHandler()->save(stream);
 if (mWeaponProperties) {
	// call weaponDataHandler() only, if it was called in c'tor, as it is
	// created here otherwise
	ret = ret && weaponDataHandler()->save(stream);
 }
 return ret;
}

bool UnitBase::loadFromXML(const QDomElement& root)
{
 if (root.isNull()) {
	boError() << k_funcinfo << "NULL root node" << endl;
	return false;
 }
 // load the data handler
 BosonCustomPropertyXML propertyXML;
 QDomElement handler = root.namedItem(QString::fromLatin1("DataHandler")).toElement();
 if (handler.isNull()) {
	boError(260) << k_funcinfo << "No DataHandler tag found for Unit" << endl;
	return false;
 }
 if (!propertyXML.loadFromXML(handler, dataHandler())) {
	boError(260) << k_funcinfo << "unable to load unit data handler (unit=" << this->id() << ")" << endl;
	return false;
 }

 // the weapon data handler
 QDomElement weaponHandler = root.namedItem(QString::fromLatin1("WeaponDataHandler")).toElement();
 if (!weaponHandler.isNull()) {
	if (!propertyXML.loadFromXML(weaponHandler, weaponDataHandler())) {
		boError(260) << k_funcinfo << "unable to load unit weapon data handler (unit=" << this->id() << ")" << endl;
		return false;
	}
 }
 return true;
}

bool UnitBase::load(QDataStream& stream)
{
 bool ret = dataHandler()->load(stream);
  if (mWeaponProperties) {
	// call weaponDataHandler() only, if it was called in c'tor, as it is
	// created here otherwise
	ret = ret && weaponDataHandler()->load(stream);
 }
 return ret;
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
 if (!dataHandler()) {
	boError() << k_funcinfo << "NULL property handler" << endl;
	return false;
 }
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

KGamePropertyHandler* UnitBase::dataHandler() const
{
 return (KGamePropertyHandler*)mProperties;
}

KGamePropertyHandler* UnitBase::weaponDataHandler()
{
 if (mWeaponProperties) {
	return (KGamePropertyHandler*)mWeaponProperties;
 }
 mWeaponProperties = new UnitPropertyHandler(this);
 mWeaponProperties->setPolicy(KGamePropertyBase::PolicyLocal); // fallback
 return (KGamePropertyHandler*)mWeaponProperties;
}

