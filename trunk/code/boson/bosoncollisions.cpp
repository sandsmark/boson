/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosoncollisions.h"

#include "cell.h"
#include "unit.h"
#include "bosonmap.h"
#include "bo3dtools.h"
#include "boitemlist.h"
#include "bodebug.h"
#include "defines.h"

#include <qptrvector.h>

#include <math.h>

BosonCollisions::BosonCollisions()
{
 init();
}

void BosonCollisions::init()
{
 mMap = 0;
}

BosonCollisions::~BosonCollisions()
{
}

Cell* BosonCollisions::cell(int x, int y) const
{
 BO_CHECK_NULL_RET0(map());
 return map()->cell(x, y);
}

Unit* BosonCollisions::findUnitAtCell(int x, int y, bofixed z) const
{
 return (Unit*)findItemAtCell(x, y, z, true);
}

BosonItem* BosonCollisions::findItemAtCell(int x, int y, bofixed z, bool unitOnly) const
{
 BoItemList* list = collisionsAtCell(x, y);
 BoItemList::Iterator it;

 // AB: about unitOnly: we could improve performance slightly by using a
 // separate function instead of this additional check. BUT:
 // a) we won't gain much (probably a few ns only)
 // b) we shouldn't do a bad design for that little speedups.
 //    good design is more important than optimizing, cause good
 //    design leads usually to faster code

 BosonItem* ret = 0;
 bofixed zDist = 100; // use a very high value - will be thrown away anyway.
 for (it = list->begin(); it != list->end(); ++it) {
	if (RTTI::isUnit((*it)->rtti())) {
		Unit* u = (Unit*)*it;
		// only living units are rlevant
		if (u->isDestroyed()) {
			continue;
		}
	} else if (unitOnly) {
		// ignore all normal other items
		continue;
	}

	// distance of item's z to desired z
	bofixed dist = (*it)->z() - z;
	if (dist < 0) {
		dist = -dist;
	}
	if (!ret || dist < zDist) {
		// new distance is lower than previous.
		ret = *it;
		zDist = dist;
	}
 }
 return ret;
}

BosonItem* BosonCollisions::findItemAt(const BoVector3Fixed& pos) const
{
 return findItemAtCell((int)(pos.x()), (int)(pos.y()), pos.z(), false);
}

Unit* BosonCollisions::findUnitAt(const BoVector3Fixed& pos) const
{
 return findUnitAtCell((int)(pos.x()), (int)(pos.y()), pos.z());
}

QValueList<Unit*> BosonCollisions::unitCollisionsInRange(const BoVector2Fixed& pos, bofixed radius) const
{
 BoItemList* l = collisions(BoRectFixed(QMAX(pos.x() - radius, bofixed(0)), QMAX(pos.y() - radius, bofixed(0)),
		pos.x() + radius, pos.y() + radius));

 QValueList<Unit*> list;
 BoItemList::Iterator it;
 for (it = l->begin(); it != l->end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		// this item is not important for us here
		continue;
	}
	Unit* u = (Unit*)*it;
	if (u->isDestroyed()) {
		// this item is not important for us here
		continue;
	}
//	boDebug(310) << "unit at x=" << u->x() << ",y=" << u->y() << ",pos=" << pos.x() << "," << pos.y() << endl;

	if ((pos - u->center()).dotProduct() <= radius * radius) {
//		boDebug(310) << "adding " << u->id() << endl;
		list.append(u);
	}
 }
 return list;
}

QValueList<Unit*> BosonCollisions::unitCollisionsInSphere(const BoVector3Fixed& pos, bofixed radius) const
{
 // FIXME: code duplicated from unitCollisionsInRange
 boDebug(310) << k_funcinfo << endl;
 BoItemList* l = collisions(BoRectFixed(QMAX(pos.x() - radius, bofixed(0)), QMAX(pos.y() - radius, bofixed(0)),
		pos.x() + radius, pos.y() + radius));

 QValueList<Unit*> list;
 BoItemList::Iterator it;
 for (it = l->begin(); it != l->end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		// this item is not important for us here
		continue;
	}
	Unit* u = (Unit*)*it;
	if (u->isDestroyed()) {
		// this item is not important for us here
		continue;
	}
//	boDebug(310) << "unit at x=" << u->x() << ",y=" << u->y() << ",pos=" << pos.x() << "," << pos.y() << endl;

	if (u->distance(pos) <= radius * radius) {
		list.append(u);
	}
 }
 return list;
}

bool BosonCollisions::cellOccupied(int x, int y) const
{
 if (!cell(x, y)) {
	return true;
 }
 return cell(x, y)->isOccupied();
}

bool BosonCollisions::cellOccupied(int x, int y, Unit* unit, bool excludeMoving) const
{
 if (!unit) {
	return cellOccupied(x, y);
 }
 if (!cell(x, y)) {
	boError(310) << k_funcinfo << "NULL cell at " << x << "," << y << endl;
	return true;
 }
 bool includeMoving = !excludeMoving; // FIXME: replace exclude by include in parameter
 return cell(x, y)->isOccupied(unit, includeMoving);
}

bool BosonCollisions::cellsOccupied(const BoRectFixed& rect) const
{
 int right = lround(rect.right());
 int bottom = lround(rect.bottom());
 for (int x = (int)rect.left(); x < right; x++) {
	for (int y = (int)rect.top(); y < bottom; y++) {
		if (cellOccupied(x, y)) {
			return true;
		}
	}
 }
 return false;
}

// this is an extremely time-critical function!
BoItemList* BosonCollisions::collisionsAtCells(const QPtrVector<Cell>* cells, const BosonItem* item, bool exact) const
{
 // FIXME: if exact is true we assume that cells == item->cells() !!
// AB: item can be NULL, too!
 BoItemList* collisions = new BoItemList(); // will get deleted by BoItemListHandler
 const BoItemList* cellItems;
 BoItemList::ConstIterator it;
 BosonItem* s;
 if (cells->count() == 0) {
	return collisions;
 }
 if (!map()) {
	BO_NULL_ERROR(map());
	return collisions;
 }
 for (unsigned int i = 0; i < cells->count(); i++) {
	Cell* c = cells->at(i);
	if (!c) {
		boError(310) << "invalid cell at " << i << endl;
		continue;
	}
	cellItems = c->items();
	for (it = cellItems->begin(); it != cellItems->end(); ++it) {
		s = *it;
		if (s != item) {
			if (collisions->findIndex(s) < 0 && (!item || !exact || item->bosonCollidesWith(s))) {
				collisions->append(s);
			}
		}
	}
 }
 return collisions;
}

BoItemList* BosonCollisions::collisions(const BoRectFixed& rect, const BosonItem* item, bool exact) const
{
 return collisionsAtCells(rect, item, exact);
}

BoItemList* BosonCollisions::collisionsAtCells(const BoRectFixed& rect, const BosonItem* item, bool exact) const
{
 if (!map()) {
	BO_NULL_ERROR(map());
	return new BoItemList();
 }
 int left, right, top, bottom;
 left = QMAX((int)rect.left(), 0);
 right = QMIN(lround(rect.right()), (int)map()->width());
 top = QMAX((int)rect.top(), 0);
 bottom = QMIN(lround(rect.bottom()), (int)map()->height());
 int size = (right - left + 1) * (bottom - top + 1);
 if (size <= 0) {
	return new BoItemList();
 }
 QPtrVector<Cell> cells(size);
 int n = 0;
 Cell* allCells = map()->cells();
 for (int i = left; i < right; i++) {
	for (int j = top; j < bottom; j++) {
		if (!map()->isValidCell(i, j)) {
			boError(310) << k_funcinfo << "not a valid cell: " << i << "," << j << endl;
			continue;
		}
		Cell* c = &allCells[map()->cellArrayPos(i, j)];
		if (!c) {
			boError(310) << k_funcinfo << "NULL cell (although the coordinates should be valid: " << i << "," << j << ")" << endl;
			continue;
		}
		cells.insert(n, c);
		n++;
	}
 }
 return collisionsAtCells(&cells, item, exact);
}

BoItemList* BosonCollisions::collisionsAtCell(int x, int y) const
{
 QPtrVector<Cell> cells(1);
 Cell* c = cell(x, y);
 if (!c) {
	boWarning(310) << k_funcinfo << "NULL cell: " << x << "," << y << endl;
	return new BoItemList();
 }
 cells.insert(0, c);
 boDebug(310) << k_funcinfo << c->x() << " " << c->y() << endl;
 return collisionsAtCells(&cells, 0, true); // FIXME: exact = true has no effect
}

BoItemList* BosonCollisions::collisions(const BoVector2Fixed& pos) const
{
 return collisionsAtCell((int)pos.x(), (int)pos.y());
}

QValueList<Unit*> BosonCollisions::collisionsInBox(const BoVector3Fixed& v1, const BoVector3Fixed& v2, BosonItem* exclude) const
{
 boDebug() << k_funcinfo << "v1: (" << v1.x() << "; " << v1.y() << "; " << v1.z() <<
		");  v2: (" << v2.x() << "; " << v2.y() << "; " << v2.z() << ")" << endl;
 QValueList<Unit*> units;
 if (!map()) {
	BO_NULL_ERROR(map());
	return units;
 }

 // Calculate rect in cell coordinates
 int left, right, top, bottom;
 left = QMAX((int)v1.x(), 0);
 right = QMIN(lround(v2.x()), (int)map()->width());
 top = QMAX((int)v1.y(), 0);
 bottom = QMIN(lround(v2.y()), (int)map()->height());
 boDebug() << k_funcinfo << "Cell rect: (" << left << ";" << top << ")-(" << right << ";" << bottom <<
		")" << endl;

 // Make list of cells
 int size = (right - left + 1) * (bottom - top + 1);
 if (size <= 0) {
	return units;
 }
 QPtrVector<Cell> cells(size);
 int n = 0;
 Cell* allCells = map()->cells();
 for (int i = left; i < right; i++) {
	for (int j = top; j < bottom; j++) {
		Cell* c = &allCells[map()->cellArrayPos(i, j)];
		if (!c) {
			boError(310) << k_funcinfo << "NULL cell (although the coordinates should be valid: " << i << "," << j << ")" << endl;
			continue;
		}
		cells.insert(n, c);
		n++;
	}
 }
 boDebug() << k_funcinfo << "Got " << cells.count() << " cells" << endl;

 // Check for collisions
 const BoItemList* cellItems;
 BoItemList::ConstIterator it;
 BosonItem* s;

 for (unsigned int i = 0; i < cells.count(); i++) {
	Cell* c = cells[i];
	if (!c) {
		boError(310) << "invalid cell at " << i << endl;
		continue;
	}
	cellItems = c->items();
	//boDebug() << "    " << k_funcinfo << "Cell " << i << " has " << cellItems->count() << " items" << endl;
	for (it = cellItems->begin(); it != cellItems->end(); ++it) {
		s = *it;
		if ((s != exclude) && RTTI::isUnit(s->rtti())) {
			//boDebug() << "        " << k_funcinfo << "Checking for collision with item " << s << " with id " << ((Unit*)s)->id() << endl;
			if (s->bosonCollidesWith(v1, v2)) {
				// We have a collision
				units.append((Unit*)s);
			}
		}
	}
 }
 return units;
}

