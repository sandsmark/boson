/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "playerio.h"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "player.h"
#include "cell.h"
#include "unit.h"
#include "bosoncanvas.h"
#include "bosonpath.h"
#include "bosonmap.h"
#include "unitproperties.h"
#include "boson.h"

#include <qptrvector.h>
#include <qptrlist.h>

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

const QString& PlayerIO::name() const
{
 return player()->name();
}

unsigned long int PlayerIO::playerId() const
{
 return player()->bosonId();
}

const Boson* PlayerIO::game() const
{
 return (Boson*)player()->game();
}

const BosonCanvas* PlayerIO::canvas() const
{
 if (!game()) {
	return 0;
 }
 return game()->canvas();
}

bool PlayerIO::hasRtti(int rtti) const
{
 return player()->hasRtti(rtti);
}

KGameIO* PlayerIO::findRttiIO(int rtti) const
{
 return player()->findRttiIO(rtti);
}

bool PlayerIO::addGameIO(KGameIO* io)
{
 return player()->addGameIO(io);
}

SpeciesTheme* PlayerIO::speciesTheme() const
{
 return player()->speciesTheme();
}

const UnitProperties* PlayerIO::unitProperties(unsigned long int type) const
{
 return player()->unitProperties(type);
}

const UpgradeProperties* PlayerIO::technologyProperties(unsigned long int type) const
{
 return player()->technologyProperties(type);
}

bool PlayerIO::isOutOfGame() const
{
 return player()->isOutOfGame();
}

bool PlayerIO::hasLost() const
{
 return player()->hasLost();
}

bool PlayerIO::hasWon() const
{
 return player()->hasWon();
}

QPtrList<Unit> PlayerIO::allUnits() const
{
 QPtrList<Unit> list;
 if (!canvas()) {
	BO_NULL_ERROR(canvas());
	return list;
 }
 for (BoItemList::ConstIterator it = canvas()->allItems()->begin(); it != canvas()->allItems()->end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	Unit* u = (Unit*)*it;
	if (u->isDestroyed()) {
		continue;
	}
  if (!(u->visibleStatus(playerId()) & (UnitBase::VS_Visible | UnitBase::VS_Earlier))) {
		continue;
	}
	list.append(u);
 }
 return list;
}

QPtrList<Unit>* PlayerIO::allMyUnits() const
{
 return player()->allUnits();
}

QPtrList<Unit> PlayerIO::allMyLivingUnits() const
{
 QPtrList<Unit> list;
 for (QPtrListIterator<Unit> it(*allMyUnits()); it.current(); ++it) {
	if (!it.current()->isDestroyed()) {
		list.append(it.current());
	}
 }
 return list;
}

QPtrList<Unit> PlayerIO::allEnemyUnits() const
{
 QPtrList<Unit> list;
 const QPtrList<Unit>& all = allUnits();
 for (QPtrListIterator<Unit> it(all); it.current(); ++it) {
	if (isEnemy(it.current())) {
		list.append(it.current());
	}
 }
 return list;
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

bool PlayerIO::useMinerals(unsigned long int amount)
{
 return player()->useMinerals(amount);
}

bool PlayerIO::useOil(unsigned long int amount)
{
 return player()->useOil(amount);
}

bool PlayerIO::useResources(unsigned long int mineralamount, unsigned long int oilamount)
{
 return player()->useResources(mineralamount, oilamount);
}

unsigned long int PlayerIO::ammunition(const QString& type) const
{
 return player()->ammunition(type);
}

BosonStatistics* PlayerIO::statistics() const
{
 return player()->statistics();
}

bool PlayerIO::isExplored(int x, int y) const
{
 return player()->isExplored(x, y);
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

bool PlayerIO::isFogged(const BoVector3Fixed& vector) const
{
 return isFogged((int)(vector.x()), (int)(vector.y()));
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

bool PlayerIO::hasMiniMap() const
{
 return player()->hasMiniMap();
}

bool PlayerIO::isEnemy(const Player* p) const
{
 return player()->isEnemy(p);
}

bool PlayerIO::isPlayerEnemy(int id) const
{
 return player()->isPlayerEnemy(id);
}

bool PlayerIO::isEnemy(const Unit* unit) const
{
 if (!unit) {
	return false;
 }
 return isEnemy(unit->owner());
}

bool PlayerIO::isNeutral(const Player* p) const
{
 return player()->isNeutral(p);
}

bool PlayerIO::isPlayerNeutral(int id) const
{
 return player()->isPlayerNeutral(id);
}

bool PlayerIO::isNeutral(const Unit* unit) const
{
 if (!unit) {
	return false;
 }
 return isNeutral(unit->owner());
}

bool PlayerIO::isAllied(const Player* p) const
{
 return player()->isAllied(p);
}

bool PlayerIO::isPlayerAllied(int id) const
{
 return player()->isPlayerAllied(id);
}

bool PlayerIO::isAllied(const Unit* unit) const
{
 if (!unit) {
	return false;
 }
 return isAllied(unit->owner());
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
 return QPoint((int)commandCenter->x(),
		(int)commandCenter->y());
}

BosonItem* PlayerIO::findItemAt(const BoVector3Fixed& canvasVector) const
{
 BO_CHECK_NULL_RET0(canvas());
 if (canSee(canvasVector)) {
	return canvas()->findItemAt(canvasVector);
 }
 return 0;
}

Unit* PlayerIO::findUnitAt(const BoVector3Fixed& canvasVector) const
{
 BO_CHECK_NULL_RET0(canvas());
 if (canSee(canvasVector)) {
	return canvas()->findUnitAt(canvasVector);
 }
 return 0;
}

Unit* PlayerIO::findUnit(unsigned long int id) const
{
 // AB: note that player()->findUnit(id) is not correct here!
 // -> id may be of a different player, too!
 BO_CHECK_NULL_RET0(game());
 Unit* u = game()->findUnit(id, 0);
 if (!u) {
	return 0;
 }
 if (!(u->visibleStatus(playerId()) & (UnitBase::VS_Visible | UnitBase::VS_Earlier))) {
	return 0;
 }
 return u;
}

BoItemList* PlayerIO::unitsAtCells(const QPtrVector<const Cell>* cells) const
{
 BoItemList* collisions = new BoItemList(); // will get deleted by BoItemListHandler
 const BoItemList* cellItems;
 BoItemList::ConstIterator it;
 BosonItem* s;
 if (cells->count() == 0) {
	return collisions;
 }
 for (unsigned int i = 0; i < cells->count(); i++) {
	const Cell* c = cells->at(i);
	if (!c) {
		boError() << "invalid cell at " << i << endl;
		continue;
	}
	cellItems = c->items();
	for (it = cellItems->begin(); it != cellItems->end(); ++it) {
		s = *it;
		if (!RTTI::isUnit(s->rtti())) {
			continue;
		}
		if (!(((Unit*)s)->visibleStatus(playerId()) & (UnitBase::VS_Visible | UnitBase::VS_Earlier))) {
			continue;
		}
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

bool PlayerIO::hasTechnology(unsigned long int id) const
{
 return player()->hasTechnology(id);
}

bool PlayerIO::hasUnitWithType(unsigned long int type) const
{
 return player()->hasUnitWithType(type);
}

const QValueList<const Unit*>* PlayerIO::radarUnits() const
{
 return player()->radarUnits();
}


bool PlayerIO::isValidCell(int x, int y) const
{
 if (!player()->map()) {
	return false;
 }
 return player()->map()->isValidCell(x, y);
}

Cell* PlayerIO::cell(int x, int y, bool* valid) const
{
 bool v = true;
 Cell* c = 0;
 if (!player()->map()) {
	v = false;
 } else {
	if (canSee(x, y)) {
		c = player()->map()->cell(x, y);
	}
 }
 if (!v) {
	if (valid) {
		*valid = v;
	}
	return 0;
 }
 if (valid) {
	*valid = v;
 }
 return c;
}

bool PlayerIO::connect(const char* signal, const QObject* receiver, const char* member)
{
 return QObject::connect(player(), signal, receiver, member);
}

bool PlayerIO::disconnect(const char* signal, const QObject* receiver, const char* member)
{
 return QObject::disconnect(player(), signal, receiver, member);
}

QPtrList<KGameIO>* PlayerIO::ioList()
{
 return player()->ioList();
}

bool PlayerIO::removeGameIO(KGameIO* io, bool deleteit)
{
 return player()->removeGameIO(io, deleteit);
}

void PlayerIO::calculatePower(unsigned long int* powerGenerated, unsigned long int* powerConsumed, bool includeUnconstructedFacilities) const
{
 player()->calculatePower(powerGenerated, powerConsumed, includeUnconstructedFacilities);
}


QValueList<BoVector2Fixed> PlayerIO::nearestMineralLocations(int x, int y, unsigned int n, unsigned int radius) const
{
 if (!canvas()) {
	BO_NULL_ERROR(canvas());
	return QValueList<BoVector2Fixed>();
 }
 if (!canvas()->pathFinder()) {
	BO_NULL_ERROR(canvas()->pathFinder());
	return QValueList<BoVector2Fixed>();
 }
 return canvas()->pathFinder()->findLocations(player(), x, y, n, radius, BosonPath::Minerals);
}

QValueList<BoVector2Fixed> PlayerIO::nearestOilLocations(int x, int y, unsigned int n, unsigned int radius) const
{
 if (!canvas()) {
	BO_NULL_ERROR(canvas());
	return QValueList<BoVector2Fixed>();
 }
 if (!canvas()->pathFinder()) {
	BO_NULL_ERROR(canvas->pathFinder());
	return QValueList<BoVector2Fixed>();
 }
 return canvas()->pathFinder()->findLocations(player(), x, y, n, radius, BosonPath::Oil);
}

