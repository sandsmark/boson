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

#include "playerio.h"

#include "bodebug.h"
#include "player.h"
#include "cell.h"
#include "unit.h"
#include "bosoncanvas.h"

#include <qptrvector.h>

class PlayerIOPrivate
{
public:
	PlayerIOPrivate()
	{
	}
};

PlayerIO::PlayerIO(Player* player)
{
 d = new PlayerIOPrivate;
 mPlayer = player;
}

PlayerIO::~PlayerIO()
{
 delete d;
}

const QColor& PlayerIO::teamColor() const
{
 return player()->teamColor();
}
unsigned long int PlayerIO::minerals() const
{
 return player()->minerals();
}
unsigned long int PlayerIO::oil() const
{
 return player()->oil();
}

BosonStatistics* PlayerIO::statistics() const
{
 return player()->statistics();
}

bool PlayerIO::isFogged(int x, int y) const
{
 return player()->isFogged(x, y);
}

bool PlayerIO::isFogged(const Cell* c) const
{
 if (!c) {
	return true;
 }
 return isFogged(c->x(), c->y());
}

bool PlayerIO::canSee(BosonItem* item) const
{
 if (!item) {
	return false;
 }
 QPtrVector<Cell>* cells = item->cells();
 for (unsigned int i = 0; i < cells->count(); i++) {
	if (canSee((*cells)[i])) {
		return true;
	}
 }
 return false;
}

bool PlayerIO::ownsUnit(const Unit* unit) const
{
 if (!unit) {
	return false;
 }
 return (unit->owner() == player());
}

bool PlayerIO::isEnemy(Player* p) const
{
 return player()->isEnemy(p);
}

bool PlayerIO::isEnemyUnit(const Unit* unit) const
{
 if (!unit) {
	return false;
 }
 return isEnemy(unit->owner());
}

QPoint PlayerIO::homeBase() const
{
 QPtrList<Unit> units = *(player()->allUnits());
 QPtrListIterator<Unit> it(units);
 Unit* commandCenter = 0;
 for (; it.current() && !commandCenter; ++it) {
	// now we have a problem. what do we need to check for?
	// checking for Unit::type() isn't nice. maybe we need a
	// UnitProperties::isCommandCenter() or so?
	//
	// so for now we get around this problem by picking the first unit and
	// then exiting the loop.
	commandCenter = it.current();
 }
 if (!commandCenter) {
	commandCenter = units.getFirst();
 }
 if (!commandCenter) {
	boWarning() << k_funcinfo << "cannot find a unit for localplayer" << endl;
	// no units for player
	return QPoint(0, 0);
 }
 return QPoint((int)commandCenter->x() / BO_TILE_SIZE,
		(int)commandCenter->y() / BO_TILE_SIZE);
}

#if 0
Unit* PlayerIO::findUnitAt(const BoVector3& canvasVector) const
{
#warning FIXME
 BosonCanvas* canvas = 0;
 return 0;
 if (canSee(canvasVector)) {
	return canvas->findUnitAt(canvasVector);
 }
 return 0;
}
#endif

BoItemList* PlayerIO::unitsAtCells(const QPtrVector<Cell>* cells) const
{
 BoItemList* collisions = new BoItemList(); // will get deleted by BoItemListHandler
 const BoItemList* cellItems;
 BoItemList::ConstIterator it;
 BosonItem* s;
 if (cells->count() == 0) {
	return collisions;
 }
 for (unsigned int i = 0; i < cells->count(); i++) {
	Cell* c = cells->at(i);
	if (!c) {
		boError() << "invalid cell at " << i << endl;
		continue;
	}
	if (!canSee(c)) {
		continue;
	}
	cellItems = c->items();
	for (it = cellItems->begin(); it != cellItems->end(); ++it) {
		s = *it;
		if (collisions->findIndex(s) < 0) {
			collisions->append(s);
		}
	}
 }
 return collisions;
}

bool PlayerIO::canBuild(unsigned long int unitType) const
{
 return player()->canBuild(unitType);
}

bool PlayerIO::canResearchTech(unsigned long int id) const
{
 return player()->canResearchTech(id);
}

bool PlayerIO::canGo(const UnitProperties* prop, const Cell* cell, bool _default) const
{
 if (!prop || !cell) {
	return _default;
 }
 if (!canSee(cell)) {
	return _default;
 }
 return cell->canGo(prop);
}

bool PlayerIO::canGo(const Unit* unit, const Cell* cell, bool _default) const
{
 if (unit) {
	return canGo(unit->unitProperties(), cell, _default);
 }
 return _default;
}

