/*
    This file is part of the Boson game
    Copyright (C) 2002 Andreas Beckermann (b_mann@gmx.de)

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

#include "boselection.h"
#include "boselection.moc"

#include "../../bomemory/bodummymemory.h"
#include "../gameengine/unit.h"
#include "../gameengine/unitproperties.h"
#include "../gameengine/pluginproperties.h"
#include "bodebug.h"
#include "../gameengine/boson.h"

#include <qdom.h>
#include <qintdict.h>

BoSelection::BoSelection(QObject* parent) : QObject(parent)
{
 mIsActivated = false;
}

BoSelection::~BoSelection()
{
}

void BoSelection::copy(BoSelection* selection, bool replace)
{
 if (replace) {
	clear(false);
 }
 if (!selection) {
	return;
 }
 QPtrList<Unit> list = selection->allUnits();
 QPtrListIterator<Unit> it(list);
 for (; it.current(); ++it) {
	add(it.current());
 }
 emit signalSelectionChanged(this);
}

void BoSelection::clear(bool emitSignal)
{
 if (isEmpty()) {
	return;
 }
 QPtrListIterator<Unit> it(mSelection);
 while (it.current()) {
	remove(it.current());
 }
 mSelection.clear();
 if (emitSignal) {
	emit signalSelectionChanged(this);
 }
}

void BoSelection::add(Unit* unit)
{
 if (!unit) {
	boError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 if (unit->isDestroyed()) {
	boDebug() << k_funcinfo << "unit destroyed" << endl;
	return;
 }
 if (mSelection.containsRef(unit)) {
	return;
 }
 mSelection.append(unit);
 unit->select();
}

void BoSelection::selectUnit(Unit* unit, bool replace)
{
 if (!unit) {
	boError() << k_funcinfo << "NULL unit" << endl;
 }
 if (replace) {
	clear(false);
 }
 add(unit);
 emit signalSelectionChanged(this);
}

void BoSelection::selectUnits(const QPtrList<Unit>& list, bool replace)
{
 if (!list.count()) {
	return;
 }
 if (list.count() == 1) {
	selectUnit(list.getFirst(), replace);
	return;
 }
 if (replace) {
	clear(false);
 }
 QPtrListIterator<Unit> it(list);
 while (it.current()) {
	add(it.current());
	++it;
 }
 emit signalSelectionChanged(this);
}

void BoSelection::removeUnit(Unit* unit)
{
 bool ret = remove(unit);
 if (ret) {
	emit signalSelectionChanged(this);
 }
}

bool BoSelection::remove(Unit* unit)
{
 if (!unit) {
	boError() << k_funcinfo << "NULL unit" << endl;
	return false;
 }
 bool ret = mSelection.removeRef(unit);
 unit->unselect();
 return ret;
}

bool BoSelection::hasMobileUnit() const
{
 QPtrListIterator<Unit> it(mSelection);
 while (it.current()) {
	if (it.current()->isMobile()) {
		return true;
	}
	++it;
 }
 return false;
}

bool BoSelection::hasMineralHarvester() const
{
 QPtrListIterator<Unit> it(mSelection);
 while (it.current()) {
	HarvesterProperties* p = (HarvesterProperties*)it.current()->properties(PluginProperties::Harvester);
	if (p && p->canMineMinerals()) {
		return true;
	}
	++it;
 }
 return false;
}

bool BoSelection::hasOilHarvester() const
{
 QPtrListIterator<Unit> it(mSelection);
 while (it.current()) {
	HarvesterProperties* p = (HarvesterProperties*)it.current()->properties(PluginProperties::Harvester);
	if (p && p->canMineOil()) {
		return true;
	}
	++it;
 }
 return false;
}


Unit* BoSelection::leader() const
{
 if (count() == 0) {
	return 0;
 }
 return mSelection.getFirst();
}

bool BoSelection::canShoot() const
{
 QPtrListIterator<Unit> it(mSelection);
 while (it.current()) {
	if (it.current()->unitProperties()->canShoot()) {
		return true;
	}
	++it;
 }
 return false;
}

bool BoSelection::canShootAt(Unit* unit) const
{
 QPtrListIterator<Unit> it(mSelection);
 while (it.current()) {
	const UnitProperties* prop = it.current()->unitProperties();
	if (unit->isFlying() && prop->canShootAtAirUnits()) {
		return true;
	} else if (!unit->isFlying() && prop->canShootAtLandUnits()) {
		return true;
	}
	++it;
 }
 return false;
}

void BoSelection::activate(bool on)
{
// TODO: config option:
// it would also be nice if the selection in one display remains when we switch
// to another display, i.e. we have only one global selection. the user should
// be able to choose one behaviour.
 if (mIsActivated == on) {
	return;
 }
 mIsActivated = on;
 if (on) {
	selectUnits(mSelection);
 } else {
	QPtrListIterator<Unit> it(mSelection);
	while (it.current()) {
		it.current()->unselect();
		++it;
	}
	emit signalSelectionChanged(this);
 }
}

void BoSelection::saveAsXML(QDomElement& root)
{
 QDomDocument doc = root.ownerDocument();
 QDomElement units = doc.createElement(QString::fromLatin1("Units"));

 QPtrListIterator<Unit> it(mSelection);
 while (it.current()) {
	QDomElement unit = doc.createElement("Unit");
	unit.setAttribute("Id", (unsigned int)it.current()->id());
	units.appendChild(unit);
	++it;
 }

 root.appendChild(units);
}

void BoSelection::loadFromXML(const QDomElement& root, bool activate)
{
 QDomElement units = root.namedItem(QString::fromLatin1("Units")).toElement();
 if (units.isNull()) {
	boError(260) << k_funcinfo << "no units" << endl;
	return;
 }

 QDomNodeList list = units.elementsByTagName(QString::fromLatin1("Unit"));
 if (list.count() == 0) {
	// It's ok to have no units in selection
	return;
 }
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement e = list.item(i).toElement();
	if (e.isNull()) {
		boError(260) << k_funcinfo << i << " is not an element" << endl;
		return;
	}
	if (!e.hasAttribute(QString::fromLatin1("Id"))) {
		boError(260) << k_funcinfo << "missing attribute: Id for Unit " << i << endl;
		continue;
	}

	bool ok = false;
	unsigned long int id;
	id = e.attribute(QString::fromLatin1("Id")).toULong(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "Invalid Id number for Unit " << i << endl;
		continue;
	}

	Unit* u = boGame->findUnit(id, 0);
	if (!u) {
		boError(260) << "Unit " << id << " doesn't exist" << endl;
		continue;
	}
	// We can't use add() here because it would bring problems, so we duplicate some code
	if (u->isDestroyed()) {
		boDebug(260) << k_funcinfo << "unit destroyed" << endl;
		continue;
	}
	if (mSelection.containsRef(u)) {
		continue;
	}
	mSelection.append(u);
	if(activate) {
		u->select();
	}
 }

 if(activate) {
	emit signalSelectionChanged(this);
 }
}

void BoSelection::slotRemoveItem(BosonItem* item)
{
 BO_CHECK_NULL_RET(item);
 // AB: do _not_ call remove()/removeUnit(), as it calls unit->unselect() which
 // doesn't exist anymore, when called from BosonItem d'tor
 // also note that we cast to Unit, but it might NOT be a Unit!
 // -> we can't access rtti() anymore
 mSelection.removeRef((Unit*)item);
}



BoSelectionGroup::BoSelectionGroup(int count, QObject* parent)
	: QObject(parent)
{
 mCount = count;
 mSelection = 0;
 mSelectionGroups = new QIntDict<BoSelection>();
 mSelectionGroups->setAutoDelete(true);
 for (int i = 0; i < mCount; i++) {
	BoSelection* s = new BoSelection(this);
	mSelectionGroups->insert(i, s);
 }
}

BoSelectionGroup::~BoSelectionGroup()
{
 mSelectionGroups->clear();
 delete mSelectionGroups;
}

void BoSelectionGroup::clearGroups()
{
 for (int i = 0; i < count(); i++) {
	slotClearSelectionGroup(i);
 }
}

void BoSelectionGroup::slotRemoveItem(BosonItem* item)
{
 QIntDictIterator<BoSelection> it(*mSelectionGroups);
 for (; it.current(); ++it) {
	it.current()->slotRemoveItem(item);
 }
}

void BoSelectionGroup::slotRemoveUnit(Unit* u)
{
 QIntDictIterator<BoSelection> it(*mSelectionGroups);
 for (; it.current(); ++it) {
	it.current()->removeUnit(u);
 }
}

void BoSelectionGroup::slotSelectSelectionGroup(int number)
{
 BO_CHECK_NULL_RET(selection());
 if (number < 0 || number >= count()) {
	boError() << k_funcinfo << "Invalid group " << number << endl;
	return;
 }
 if (!(*mSelectionGroups)[number]) {
	boError() << k_funcinfo << "NULL group " << number << endl;
	return;
 }
 selection()->copy((*mSelectionGroups)[number]);
}

void BoSelectionGroup::slotCreateSelectionGroup(int number)
{
 BO_CHECK_NULL_RET(selection());
 if (number < 0 || number >= count()) {
	boError() << k_funcinfo << "Invalid group " << number << endl;
	return;
 }
 if (!(*mSelectionGroups)[number]) {
	boError() << k_funcinfo << "NULL group " << number << endl;
	return;
 }
 (*mSelectionGroups)[number]->copy(selection());
}

void BoSelectionGroup::slotClearSelectionGroup(int number)
{
 if (number < 0 || number >= count()) {
	boError() << k_funcinfo << "Invalid group " << number << endl;
	return;
 }
 if (!(*mSelectionGroups)[number]) {
	boError() << k_funcinfo << "NULL group " << number << endl;
	return;
 }
 (*mSelectionGroups)[number]->clear();
}

bool BoSelectionGroup::loadFromXML(const QDomElement& root)
{
 if (root.isNull()) {
	return false;
 }
 QDomNodeList list = root.elementsByTagName(QString::fromLatin1("Group"));
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement e = list.item(i).toElement();
	if (e.isNull()) {
		boError(260) << k_funcinfo << i << " is not an element" << endl;
		return false;
	}
	if (!e.hasAttribute("Id")) {
		boError(260) << k_funcinfo << "missing attribute: Id for Group " << i << endl;
		continue;
	}
	int id;
	bool ok;
	id = e.attribute("Id").toInt(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "Invalid Id for Group " << i << endl;
		continue;
	}
	if (!(*mSelectionGroups)[id]) {
		boError(260) <<k_funcinfo << "no unitgroup with id=" << id << endl;
		continue;
	}
	(*mSelectionGroups)[id]->loadFromXML(e);
 }
 return true;
}

bool BoSelectionGroup::saveAsXML(QDomElement& root) const
{
 QDomDocument doc = root.ownerDocument();
 for(int i = 0; i < count(); i++) {
	QDomElement group = doc.createElement(QString::fromLatin1("Group"));
	group.setAttribute("Id", i);
	(*mSelectionGroups)[i]->saveAsXML(group);
	root.appendChild(group);
 }
 return true;
}



