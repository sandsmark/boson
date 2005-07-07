/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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


#include "boupgradeableproperty.h"

#include "../bomemory/bodummymemory.h"
#include <bodebug.h>
#include "unitproperties.h"
#include "bosonweapon.h"
#include "upgradeproperties.h"
#include "speciestheme.h"

#include <qdom.h>


UpgradesCollection::UpgradesCollection()
{
 mUpgrades = new QValueList<const UpgradeProperties*>();

 // all properties' counters are initialized with 0, so when we
 // set this counter to 1, all properties are initiall dirty
 mUpgradesCacheCounter = 1;
}

UpgradesCollection::~UpgradesCollection()
{
 delete mUpgrades;
}

void UpgradesCollection::addUpgrade(const UpgradeProperties* upgrade)
{
 BO_CHECK_NULL_RET(upgrade);
 mUpgrades->append(upgrade);

 // AB: the counter is increased whenever the upgrades of the player change in
 // any way. this causes all values that depend on upgrades to be recalculated
 // when required.
 mUpgradesCacheCounter++;
}

void UpgradesCollection::clearUpgrades()
{
 while (!upgrades()->isEmpty()) {
	removeUpgrade(upgrades()->first());
 }
}


const UpgradeProperties* UpgradesCollection::findUpgrade(unsigned long int id) const
{
 QValueList<const UpgradeProperties*>::const_iterator it;
 for (it = upgrades()->begin(); it != upgrades()->end(); ++it) {
	if ((*it)->id() == id) {
		return *it;
	}
 }
 return 0;
}

bool UpgradesCollection::removeUpgrade(const UpgradeProperties* upgrade)
{
 if (!upgrade) {
	return false;
 }
 bool ret = false;
 if (mUpgrades->remove(upgrade) > 0) {
	ret = true;
	mUpgradesCacheCounter++;
 }

 return ret;
}

bool UpgradesCollection::saveAsXML(QDomElement& root) const
{
 if (root.isNull()) {
	return false;
 }
 QDomDocument doc = root.ownerDocument();
 QValueList<const UpgradeProperties*>::const_iterator it;
 for (it = upgrades()->begin(); it != upgrades()->end(); ++it) {
	QDomElement e = doc.createElement("Upgrade");
	e.setAttribute("Id", QString::number((*it)->id()));
	root.appendChild(e);
 }
 return true;
}

bool UpgradesCollection::loadFromXML(const SpeciesTheme* speciesTheme, const QDomElement& root)
{
 clearUpgrades();
 if (!speciesTheme) {
	BO_NULL_ERROR(speciesTheme);
	return false;
 }
 if (root.isNull()) {
	return false;
 }
 QDomDocument doc = root.ownerDocument();
 for (QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
	QDomElement e = n.toElement();
	if (e.isNull()) {
		continue;
	}
	if (e.tagName() != "Upgrade") {
		continue;
	}
	bool ok = false;
	unsigned long int id = e.attribute("Id").toULong(&ok);
	if (!ok) {
		boError() << k_funcinfo << "invalid Id attribute" << endl;
		return false;
	}

#warning FIXME: only technologies supported atm
	// TODO: atm we support technologies only. should be more general.
	const UpgradeProperties* upgrade = speciesTheme->technology(id);
	if (!upgrade) {
		boError() << k_funcinfo << "cannot find upgrade " << id << endl;
		return false;
	}
	addUpgrade(upgrade);
 }
 return true;
}



bool BoUpgradeablePropertyBase::loadBaseValue(unsigned long int* v) const
{
 if (!unitProperties() && !weaponProperties()) {
	boError() << k_funcinfo << "dont know where to retrieve base value from" << endl;
	return false;
 }
 if (unitProperties() && weaponProperties()) {
	boError() << k_funcinfo << "more than one source for base value available" << endl;
	return false;
 }

 if (unitProperties()) {
	unitProperties()->getUpgradeableBaseValue(v, name(), type());
 } else if (weaponProperties()) {
	weaponProperties()->getUpgradeableBaseValue(v, name(), type());
 }
 return true;
}

bool BoUpgradeablePropertyBase::loadBaseValue(long int* v) const
{
 if (!unitProperties() && !weaponProperties()) {
	boError() << k_funcinfo << "dont know where to retrieve base value from" << endl;
	return false;
 }
 if (unitProperties() && weaponProperties()) {
	boError() << k_funcinfo << "more than one source for base value available" << endl;
	return false;
 }

 if (unitProperties()) {
	unitProperties()->getUpgradeableBaseValue(v, name(), type());
 } else if (weaponProperties()) {
	weaponProperties()->getUpgradeableBaseValue(v, name(), type());
 }
 return true;
}

bool BoUpgradeablePropertyBase::loadBaseValue(bofixed* v) const
{
 if (!unitProperties() && !weaponProperties()) {
	boError() << k_funcinfo << "dont know where to retrieve base value from" << endl;
	return false;
 }
 if (unitProperties() && weaponProperties()) {
	boError() << k_funcinfo << "more than one source for base value available" << endl;
	return false;
 }

 if (unitProperties()) {
	unitProperties()->getUpgradeableBaseValue(v, name(), type());
 } else if (weaponProperties()) {
	weaponProperties()->getUpgradeableBaseValue(v, name(), type());
 }
 return true;
}

bool BoUpgradeablePropertyBase::upgradeValue(const QValueList<const UpgradeProperties*>* list, unsigned long int* v) const
{
 QValueList<const UpgradeProperties*>::const_iterator it;
 for (it = list->begin(); it != list->end(); ++it) {
	if (!(*it)->upgradeValue(name(), v, type())) {
		boError() << k_funcinfo << "upgrade failed" << endl;
		return false;
	}
 }
 return true;
}

bool BoUpgradeablePropertyBase::upgradeValue(const QValueList<const UpgradeProperties*>* list, long int* v) const
{
 QValueList<const UpgradeProperties*>::const_iterator it;
 for (it = list->begin(); it != list->end(); ++it) {
	if (!(*it)->upgradeValue(name(), v, type())) {
		boError() << k_funcinfo << "upgrade failed" << endl;
		return false;
	}
 }
 return true;
}

bool BoUpgradeablePropertyBase::upgradeValue(const QValueList<const UpgradeProperties*>* list, bofixed* v) const
{
 QValueList<const UpgradeProperties*>::const_iterator it;
 for (it = list->begin(); it != list->end(); ++it) {
	if (!(*it)->upgradeValue(name(), v, type())) {
		boError() << k_funcinfo << "upgrade failed" << endl;
		return false;
	}
 }
 return true;
}


