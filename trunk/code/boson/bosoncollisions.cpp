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

Unit* BosonCollisions::findUnitAtCell(int x, int y)
{
 return (Unit*)findItemAtCell(x, y, true);
}

BosonItem* BosonCollisions::findItemAtCell(int x, int y, bool unitOnly)
{
 BoItemList list = collisionsAtCell(QPoint(x, y));
 BoItemList::Iterator it;

 for (it = list.begin(); it != list.end(); ++it) {
	if (RTTI::isUnit((*it)->rtti())) {
		Unit* u = (Unit*)*it;
		if (!u->isDestroyed()) {
			return *it;
		}
	} else if (!unitOnly) {
		// AB: we could improve performance slightly by using a separate
		// function instead of this additional check. BUT:
		// a) we won't gain much (probably a few ns only)
		// b) we shouldn't do a bad design for that little speedups.
		//    good design is more important than optimizing, cause good
		//    design leads usually to faster code
		return *it;
	}
 }
 return 0;
}

BosonItem* BosonCollisions::findItemAt(const QPoint& pos)
{
 return findItemAtCell(pos.x() / BO_TILE_SIZE, pos.y() / BO_TILE_SIZE, false);
}

Unit* BosonCollisions::findUnitAt(const QPoint& pos)
{
 return findUnitAtCell(pos.x() / BO_TILE_SIZE, pos.y() / BO_TILE_SIZE);
}

QValueList<Unit*> BosonCollisions::unitCollisionsInRange(const QPoint& pos, int radius) const
{
 BoItemList l = collisions(QRect(
		(pos.x() - radius > 0) ? pos.x() - radius : 0,
		(pos.y() - radius > 0) ? pos.y() - radius : 0,
		pos.x() + radius,
		pos.y() + radius));
			
 QValueList<Unit*> list;
 for (unsigned int i = 0; i < l.count(); i++) {
	if (!RTTI::isUnit(l[i]->rtti())) {
		// this item is not important for us here
		continue;
	}
	Unit* u = (Unit*)l[i];
	if (u->isDestroyed()) {
		// this item is not important for us here
		continue;
	}
//	boDebug(310) << "unit at x=" << u->x() << ",y=" << u->y() << ",pos=" << pos.x() << "," << pos.y() << endl;
	int w = pos.x() - (int)(u->x() + u->width() / 2);
	int h = pos.y() - (int)(u->y() + u->height() / 2);
//	boDebug(310) << "w*w=" << w*w << ",h*h=" << h*h << " <= r*r=" << radius*radius<< endl;

	if (w * w + h * h <= radius * radius) {
//		boDebug(310) << "adding " << u->id() << endl;
		list.append(u);
	}
 }
 return list;
}

QValueList<Unit*> BosonCollisions::unitCollisionsInSphere(const BoVector3& pos, int radius) const
{
 radius -= 10;  // hack, but prevents nearby units from getting damaged in some conditions
 // FIXME: code duplicated from unitCollisionsInRange
 boDebug(310) << k_funcinfo << endl;
 BoItemList l = collisions(QRect(
		(pos.x() - radius > 0) ? (int)pos.x() - radius : 0,
		(pos.y() - radius > 0) ? (int)pos.y() - radius : 0,
		(int)pos.x() + radius,
		(int)pos.y() + radius));
			
 QValueList<Unit*> list;
 for (unsigned int i = 0; i < l.count(); i++) {
	if (!RTTI::isUnit(l[i]->rtti())) {
		// this item is not important for us here
		continue;
	}
	Unit* u = (Unit*)l[i];
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
	boError(310) << k_funcinfo << "NULL cell" << endl;
	return true;
 }
 bool includeMoving = !excludeMoving; // FIXME: replace exclude by include in parameter
 return cell(x, y)->isOccupied(unit, includeMoving);
}

bool BosonCollisions::cellsOccupied(const QRect& rect) const
{
 const int left = rect.left() / BO_TILE_SIZE;
 const int top = rect.top() / BO_TILE_SIZE;
 const int right = rect.right() / BO_TILE_SIZE + ((rect.right() % BO_TILE_SIZE == 0) ? 0 : 1);
 const int bottom = rect.bottom() / BO_TILE_SIZE + ((rect.bottom() % BO_TILE_SIZE == 0) ? 0 : 1);

 for (int x = left; x < right; x++) {
	for (int y = top; y < bottom; y++) {
		if (cellOccupied(x, y)) {
			return true;
		}
	}
 }
 return false;
}

// this is an extremely time-critical function!
BoItemList BosonCollisions::collisionsAtCells(const QPtrVector<Cell>* cells, const BosonItem* item, bool exact) const
{
 // FIXME: if exact is true we assume that cells == item->cells() !!
// AB: item can be NULL, too!
 BoItemList collisions;
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
			if (collisions.findIndex(s) < 0 && (!item || !exact || item->bosonCollidesWith(s))) {
				collisions.append(s);
			}
		}
	}
 }
 return collisions;
}

BoItemList BosonCollisions::collisions(const QRect& rect, const BosonItem* item, bool exact) const
{
 // rect is canvas coordinates!
 int w = rect.width() / BO_TILE_SIZE;
 int h = rect.height() / BO_TILE_SIZE;
 if (rect.width() % BO_TILE_SIZE != 0) {
	w++;
 }
 if (rect.height() % BO_TILE_SIZE != 0) {
	h++;
 }
 return collisionsAtCells(QRect(rect.left() / BO_TILE_SIZE, rect.top() / BO_TILE_SIZE, w, h), item, exact);
}

BoItemList BosonCollisions::collisionsAtCells(const QRect& rect, const BosonItem* item, bool exact) const
{
 if (!map()) {
	BO_NULL_ERROR(map());
	return BoItemList();
 }
 int left, right, top, bottom;
 left = QMAX(rect.left(), 0);
 right = QMIN(rect.right(), QMAX((int)map()->width() - 1, 0));
 top = QMAX(rect.top(), 0);
 bottom = QMIN(rect.bottom(), QMAX((int)map()->height() - 1, 0));
 int size = (right - left + 1) * (bottom - top + 1);
 if (size <= 0) {
	return BoItemList();
 }
 QPtrVector<Cell> cells(size);
 int n = 0;
 Cell* allCells = map()->cells();
 for (int i = left; i <= right; i++) {
	for (int j = top; j <= bottom; j++) {
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

BoItemList BosonCollisions::collisionsAtCell(const QPoint& pos) const
{
 QPtrVector<Cell> cells(1);
 Cell* c = cell(pos.x(), pos.y());
 if (!c) {
	boWarning(310) << k_funcinfo << "NULL cell: " << pos.x() << "," << pos.y() << endl;
	return BoItemList();
 }
 cells.insert(0, c);
 boDebug(310) << k_funcinfo << c->x() << " " << c->y() << endl;
 return collisionsAtCells(&cells, 0, true); // FIXME: exact = true has no effect
}

BoItemList BosonCollisions::collisions(const QPoint& pos) const
{
 return collisionsAtCell(pos / BO_TILE_SIZE);
}


