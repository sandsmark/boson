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

class BoSelection::BoSelectionPrivate
{
public:
	QPtrList<Unit> mSelection;
	bool mIsActivated;
};

BoSelection::BoSelection(QObject* parent) : QObject(parent)
{
 d = new BoSelectionPrivate;
 d->mIsActivated = false;
}

BoSelection::~BoSelection()
{
 delete d;
}

void BoSelection::clear()
{
 QPtrListIterator<Unit> it(d->mSelection);
 while (it.current()) {
	remove(it.current());
 }
 d->mSelection.clear();

 // FIXME: don't emit if selection was already empty
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
 if (d->mSelection.containsRef(unit)) {
	return;
 }
 if (d->mSelection.count() > 0) {
	if (d->mSelection.first()->owner() != unit->owner()) {
		return;
	}
 }
 d->mSelection.append(unit);
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
 d->mSelection.removeRef(unit);
 unit->unselect();
}

uint BoSelection::count() const
{
 return d->mSelection.count();
}

bool BoSelection::isEmpty() const
{
 return d->mSelection.isEmpty();
}

bool BoSelection::hasMobileUnit() const
{
 QPtrListIterator<Unit> it(d->mSelection);
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
 QPtrListIterator<Unit> it(d->mSelection);
 while (it.current()) {
	if (it.current()->unitProperties()->canMineMinerals()) {
		return true;
	}
	++it;
 }
 return false;
}

bool BoSelection::hasOilHarvester() const
{
 QPtrListIterator<Unit> it(d->mSelection);
 while (it.current()) {
	if (it.current()->unitProperties()->canMineOil()) {
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
 return d->mSelection.first();
}

QPtrList<Unit> BoSelection::allUnits() const
{
 return d->mSelection;
}

bool BoSelection::contains(Unit* unit) const
{
 return d->mSelection.containsRef(unit);
}

bool BoSelection::canShoot() const
{
 QPtrListIterator<Unit> it(d->mSelection);
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
 QPtrListIterator<Unit> it(d->mSelection);
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
 if (d->mIsActivated == on) {
	return;
 }
 d->mIsActivated = on;
 if (on) {
	selectUnits(d->mSelection);
 } else {
	QPtrListIterator<Unit> it(d->mSelection);
	while (it.current()) {
		it.current()->unselect();
		++it;
	}
	emit signalSingleUnitSelected(0);
 }
}
