/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2002-2006 Rivo Laks (rivolaks@hot.ee)

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
#include "ammunitionstorageplugin.h"

#include "../../bomemory/bodummymemory.h"
#include "../defines.h"
#include "unit.h"
#include "player.h"
#include "playerio.h"
#include "pluginproperties.h"
#include "bodebug.h"

#include <qdom.h>

#include <math.h>

AmmunitionStoragePlugin::AmmunitionStoragePlugin(Unit* owner)
		: UnitPlugin(owner)
{
}
AmmunitionStoragePlugin::~AmmunitionStoragePlugin()
{
}

bool AmmunitionStoragePlugin::saveAsXML(QDomElement& root) const
{
 QDomDocument doc = root.ownerDocument();
 for (QMap<QString, unsigned long int>::const_iterator it = mAmmunitionStorage.begin(); it != mAmmunitionStorage.end(); ++it) {
	QDomElement e = doc.createElement("AmmunitionStorage");
	e.setAttribute("Type", it.key());
	e.setAttribute("Value", QString::number(it.data()));
 }
 return true;
}

bool AmmunitionStoragePlugin::loadFromXML(const QDomElement& root)
{
 mAmmunitionStorage.clear();
 for (QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
	QDomElement e = n.toElement();
	if (e.isNull()) {
		continue;
	}
	if (e.tagName() != "AmmunitionStorage") {
		continue;
	}
	QString type = e.attribute("Type");
	if (type.isEmpty()) {
		boError() << k_funcinfo << "empty type attribute" << endl;
		return false;
	}
	if (!canStore(type)) {
		boError() << k_funcinfo << "cannot store type " << type << endl;
		return false;
	}
	bool ok;
	unsigned long int value = e.attribute("Value").toULong(&ok);
	if (!ok) {
		boError() << k_funcinfo << "invalid Value attribute for Type=" << type << endl;
		return false;
	}
	mAmmunitionStorage.insert(type, value);
 }
 return true;
}

void AmmunitionStoragePlugin::advance(unsigned int)
{
}

bool AmmunitionStoragePlugin::mustBePickedUp(const QString& type) const
{
 const AmmunitionStorageProperties * prop = (AmmunitionStorageProperties*)unit()->properties(PluginProperties::AmmunitionStorage);
 if (!prop) {
	return false;
 }
 return prop->mustBePickedUp(type);
}

bool AmmunitionStoragePlugin::canStore(const QString& type) const
{
 const AmmunitionStorageProperties * prop = (AmmunitionStorageProperties*)unit()->properties(PluginProperties::AmmunitionStorage);
 if (!prop) {
	return false;
 }
 return prop->canStore(type);
}

unsigned long int AmmunitionStoragePlugin::requestAmmunitionGlobally(const QString& type, unsigned long int requested)
{
 if (mustBePickedUp(type)) {
	return 0;
 }
 return giveAmmunition(type, requested);
}

unsigned long int AmmunitionStoragePlugin::pickupAmmunition(Unit* picksUp, const QString& type, unsigned long int requested, bool* denied)
{
 if (denied) {
	*denied = false;
 }
 if (!mustBePickedUp(type)) {
	return requestAmmunitionGlobally(type, requested);
 }
 if (!picksUp) {
	if (denied) {
		*denied = true;
	}
	return 0;
 }
 if (!isNextTo(picksUp)) {
	if (denied) {
		*denied = true;
	}
	return 0;
 }
 return giveAmmunition(type, requested);
}

unsigned long int AmmunitionStoragePlugin::giveAmmunition(const QString& type, unsigned long int requested)
{
 if (!canStore(type)) {
	return 0;
 }
 if (unit()->isDestroyed()) {
	boWarning(610) << k_funcinfo << unit()->id() << " is destroyed" << endl;
	return 0;
 }
 int change = -((int)requested);
 change = changeAmmunition(type, change);
 if (change > 0) {
	boError() << k_funcinfo << "ammunition storage increased!" << endl;
	return 0;
 }
 return (unsigned long int)(-change);
}

unsigned long int AmmunitionStoragePlugin::ammunitionStored(const QString& type) const
{
 if (!canStore(type)) {
	return 0;
 }
 if (!mAmmunitionStorage.contains(type)) {
	return 0;
 }
 return mAmmunitionStorage[type];
}

unsigned long int AmmunitionStoragePlugin::tryToFillStorage(const QString& type, unsigned long int ammo)
{
 if (!canStore(type)) {
	return 0;
 }
 if (!mAmmunitionStorage.contains(type)) {
	mAmmunitionStorage.insert(type, 0);
 }
 // AB: atm there is always unlimited capacity.
 int change = changeAmmunition(type, ammo);
 if (change < 0) {
	boError() << k_funcinfo << "ammo has been reduced!" << endl;
	return 0;
 }
 return (unsigned long int)(change);
}

void AmmunitionStoragePlugin::unitDestroyed(Unit*)
{
}

void AmmunitionStoragePlugin::itemRemoved(BosonItem*)
{
}

int AmmunitionStoragePlugin::changeAmmunition(const QString& type, int change)
{
 if (change == 0) {
	return 0;
 }
 if (!canStore(type)) {
	return 0;
 }
 if (!mAmmunitionStorage.contains(type)) {
	mAmmunitionStorage.insert(type, 0);
 }
 unsigned long int stored = mAmmunitionStorage[type];
 if (change > 0) {
	// AB: atm there is no capacity limitation
	stored = stored + change;

	change = change;
 } else {
	unsigned long int subtracted = -change;
	if (subtracted > stored) {
		subtracted = stored;
	}
	stored = stored - subtracted;

	change = -((int)subtracted);
 }

 mAmmunitionStorage.insert(type, stored);

 // AB: we might cache the sum of ammunition stored by the units.
// player()->changeAmmunitionStorage(type, change);

 return change;
}

