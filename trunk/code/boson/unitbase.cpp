/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "pluginproperties.h"
#include "player.h"
#include "speciestheme.h"
#include "bodebug.h"

#include <kgame/kgamepropertyhandler.h>

#include <qdom.h>
#include <qmap.h>

UnitBase::PropertyMap* UnitBase::mPropertyMap = 0;

// this is done this way in order to prevent including qmap.h in the header
class UnitBase::PropertyMap : public QMap<int, QString>
{
};


UnitBase::UnitBase(const UnitProperties* prop)
{
 if (!mPropertyMap) {
	initStatic();
 }
 mProperties = new KGamePropertyHandler();
 mProperties->setPolicy(KGamePropertyBase::PolicyLocal); // fallback
 mOwner = 0;
 mUnitProperties = prop; // WARNING: this might be 0 at this point! MUST be != 0 for Unit, but ScenarioUnit uses 0 here

// PolicyLocal?
 registerData(&mHealth, IdHealth);
 registerData(&mArmor, IdArmor);
 registerData(&mShields, IdShields);
 registerData(&mSightRange, IdSightRange);
 registerData(&mWork, IdWork);
 registerData(&mAdvanceWork, IdAdvanceWork);
 registerData(&mDeletionTimer, IdDeletionTimer);
 mDeletionTimer.setEmittingSignal(false);

 mId = 0;


 mWork.setLocal((int)WorkNone);
 mAdvanceWork.setLocal((int)WorkNone);
 mHealth.setLocal(0); // initially destroyed
 mShields.setLocal(0); // doesn't have any shields
 mArmor.setLocal(0); // doesn't have any armor
 mSightRange.setLocal(0);
 mDeletionTimer.setLocal(0);
}

UnitBase::~UnitBase()
{
// boDebug() << k_funcinfo << endl;
 dataHandler()->clear();
// boDebug() << k_funcinfo << " done" << endl;
}

void UnitBase::initStatic()
{
 delete mPropertyMap;
 mPropertyMap = new PropertyMap;
 addPropertyId(IdHealth, QString::fromLatin1("Health"));
 addPropertyId(IdArmor, QString::fromLatin1("Armor"));
 addPropertyId(IdShields, QString::fromLatin1("Shields"));
 addPropertyId(IdSightRange, QString::fromLatin1("SightRange"));
 addPropertyId(IdWork, QString::fromLatin1("Work"));
 addPropertyId(IdAdvanceWork, QString::fromLatin1("AdvanceWork"));
 addPropertyId(IdDeletionTimer, QString::fromLatin1("DeletionTimer"));
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

unsigned long int UnitBase::shields() const
{
 return mShields;
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

bool UnitBase::save(QDataStream& stream)
{
 // TODO: we need to save and load Unit::mCurrentPlugin->pluginType() !!
 // note that multiple plugins of the same type are not *yet* supported! but
 // they might be one day..
 stream << (Q_UINT32)unitProperties()->typeId();
 bool ret = dataHandler()->save(stream);
 stream << (Q_UINT32)id();
 return ret;
}

bool UnitBase::load(QDataStream& stream)
{
 Q_UINT32 typeId;
 Q_UINT32 id;
 stream >> typeId;
 if (!speciesTheme()) {
	boError() << k_funcinfo << "NULL speciesTheme" << endl;
	return false;
 }
 mUnitProperties = speciesTheme()->unitProperties(typeId);
 bool ret = dataHandler()->load(stream);
 stream >> id;
 mId = id;
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
	if (s == QString::null) {
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

