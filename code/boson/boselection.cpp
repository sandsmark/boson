/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "boselection.h"
#include "boselection.moc"

#include "unit.h"
#include "unitproperties.h"
#include "pluginproperties.h"

BoSelection::BoSelection(QObject* parent) : QObject(parent)
{
 mIsActivated = false;
}

BoSelection::~BoSelection()
{
}

void BoSelection::copy(BoSelection* selection)
{
 clear();
 if (!selection) {
	return;
 }
 QPtrList<Unit> list = selection->allUnits();
 QPtrListIterator<Unit> it(list);
 for (; it.current(); ++it) {
	add(it.current());
 }
}

void BoSelection::clear()
{
 if (isEmpty()) {
	return;
 }
 QPtrListIterator<Unit> it(mSelection);
 while (it.current()) {
	remove(it.current());
 }
 mSelection.clear();

 emit signalSingleUnitSelected(0);
}

void BoSelection::add(Unit* unit)
{
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 if (unit->isDestroyed()) {
	kdDebug() << k_funcinfo << "unit destroyed" << endl;
	return;
 }
 if (mSelection.containsRef(unit)) {
	return;
 }
 if (mSelection.count() > 0) {
	if (mSelection.first()->owner() != unit->owner()) {
		return;
	}
 }
 mSelection.append(unit);
 unit->select();
}

void BoSelection::selectUnit(Unit* unit)
{
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit" << endl;
 }
 clear();
 add(unit);
 emit signalSingleUnitSelected(unit);
}

void BoSelection::selectUnits(QPtrList<Unit> list)
{
 if (!list.count()) {
	return;
 }
 if (list.count() == 1) {
	selectUnit(list.first());
	return;
 }
 clear();
 Player* p = 0;
 QPtrListIterator<Unit> it(list);
 while (it.current() && it.current()->owner()) {
	if (it.current()->owner() != p) {
		if (p == 0) {
			p = it.current()->owner();
		} else {
			continue;
		}
	
	}
	add(it.current());
	emit signalSelectUnit(it.current());
	++it;
 }
}

void BoSelection::removeUnit(Unit* unit)
{
 remove(unit);
 emit signalUnselectUnit(unit);
}

void BoSelection::remove(Unit* unit)
{
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 mSelection.removeRef(unit);
 unit->unselect();
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
	emit signalSingleUnitSelected(0);
 }
}
