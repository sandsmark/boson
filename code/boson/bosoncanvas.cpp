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

#include "bosoncanvas.h"
#include "player.h"
#include "cell.h"
#include "unit.h"
#include "unitplugins.h"
#include "bosonmap.h"
#include "unitproperties.h"
#include "speciestheme.h"
#include "boitemlist.h"
#include "bosonparticlesystem.h"
#include "bosonparticlemanager.h"
#include "defines.h"
#include "items/bosonshot.h"
#include "bosonweapon.h"
#include "bosonstatistics.h"
#include "bodebug.h"

#include <klocale.h>

#include <qpointarray.h>

#include "bosoncanvas.moc"


class BosonCanvas::BosonCanvasPrivate
{
public:
	BosonCanvasPrivate()
	{
		mMap = 0;

	}
	
	QPtrList<Unit> mDestroyedUnits;

	BosonMap* mMap; // just a pointer - no memory allocated

	QPtrList<BosonItem> mAnimList; // see BosonCanvas::slotAdvance()

	BoItemList mAllItems;

	QPtrList<BosonParticleSystem> mParticles;
};

BosonCanvas::BosonCanvas(QObject* parent)
		: QObject(parent, "BosonCanvas")
{
 init();
}

void BosonCanvas::init()
{
 d = new BosonCanvasPrivate;
 d->mDestroyedUnits.setAutoDelete(false);
 d->mParticles.setAutoDelete(true);
 mAdvanceFunctionLocked = false;
}

BosonCanvas::~BosonCanvas()
{
boDebug()<< k_funcinfo << endl;
 quitGame();
 delete d;
boDebug()<< k_funcinfo <<"done"<< endl;
}

void BosonCanvas::quitGame()
{
 deleteDestroyed(); // already called before
 d->mAnimList.clear();
 d->mParticles.clear();
}

void BosonCanvas::deleteDestroyed()
{
 d->mDestroyedUnits.setAutoDelete(true);
 d->mDestroyedUnits.clear();
 d->mDestroyedUnits.setAutoDelete(false);
}

Cell* BosonCanvas::cell(int x, int y) const
{
 if (!d->mMap) {
	boError() << k_funcinfo << "NULL map" << endl;
	return 0;
 }
 return d->mMap->cell(x, y);
}

void BosonCanvas::slotAddUnit(Unit* unit, int x, int y)
{
 if (!unit) {
	boError() << k_funcinfo << "NULL unit!" << endl;
	return;
 }

 unit->move(x, y, unit->z());
// unit->show();
}

Unit* BosonCanvas::findUnitAt(const QPoint& pos)
{
 BoItemList list = bosonCollisions(pos);
 BoItemList::Iterator it;

 for (it = list.begin(); it != list.end(); ++it) {
	if (RTTI::isUnit((*it)->rtti())) {
		Unit* u = (Unit*)*it;
		if (!u->isDestroyed()) {
			return u;
		}
	}
 }
 return 0;
}

void BosonCanvas::slotAdvance(unsigned int advanceCount, bool advanceFlag)
{
 QPtrListIterator<BosonItem> animIt(d->mAnimList);
 lockAdvanceFunction();
 if (advanceFlag) {
	// note: the advance methods must not change the advanceFunction()s
	// here!
	while (animIt.current()) {
		BosonItem* s = animIt.current();
		s->advance(advanceCount);
		s->advanceFunction(advanceCount); // once this was called this object is allowed to change its advanceFunction()
		// now move *without* collision detection. collision detection should
		// have been done above - especially in advanceMoveCheck() methods.
		// AB: do NOT add something here - if you add something for units then
		// check for isDestroyed() !!
		if (s->xVelocity() || s->yVelocity()) {
			s->moveBy(s->xVelocity(), s->yVelocity(), 0.0);
		}
		++animIt;
	}
 } else {
	// note: the advance methods must not change the advanceFunction2()s
	// here!
	while (animIt.current()) {
		BosonItem* s = animIt.current();
		s->advance(advanceCount);
		s->advanceFunction2(advanceCount); // once this was called this object is allowed to change its advanceFunction2()
		if (s->xVelocity() || s->yVelocity()) {
			s->moveBy(s->xVelocity(), s->yVelocity(), 0.0);
		}
		++animIt;
	}
 }
 unlockAdvanceFunction();

 deleteUnusedShots();
 
 updateParticleSystems(0.05);  // With default game speed, delay between advance messages is 1.0 / 20 = 0.05 sec

 if (advanceCount == MAXIMAL_ADVANCE_COUNT) {
	boDebug() << "MAXIMAL_ADVANCE_COUNT" << endl;
	// there are 2 different timers for deletion of canvas items.
	// The first is done in BosonCanvas - we only delete anything when
	// advanceCount == MAXIMAL_ADVANCE_COUNT.
	// The second is unit based. every MAXIMAL_ADVANCE_COUNT advance calls
	// we increase the deletion timer of the unit and delete it when
	// REMOVE_WRECKAGES_TIME is reached. This way we don't see all wreckages
	// diappear at once...
	QPtrListIterator<Unit> deletionIt(d->mDestroyedUnits);
	QPtrList<Unit> deleteList;
	while (deletionIt.current()) {
		deletionIt.current()->increaseDeletionTimer();
		if (deletionIt.current()->deletionTimer() >= REMOVE_WRECKAGES_TIME) { 
			deleteList.append(deletionIt.current());
		}
		++deletionIt;
	}
 
	while (deleteList.count() > 0) {
		Unit* u = deleteList.first();
		deleteList.removeRef(u);
		d->mDestroyedUnits.removeRef(u);
		delete u;
	}
 }
}

bool BosonCanvas::canGo(const UnitProperties* prop, const QRect& rect) const
{
// boDebug() << k_funcinfo << endl;
 int y = rect.y() / BO_TILE_SIZE; // what about modulu? do we care ?
 do {
	int x = rect.x() / BO_TILE_SIZE;
	do {
		Cell* newCell = cell(x, y);
		if (!newCell) {
			boError() << k_funcinfo << "NULL cell" << endl;
			return false;
		}
		if (!newCell->canGo(prop)) {
			boDebug() << "can  not go on " << x << "," << y << endl;
			return false;
		} else {
//			boDebug() << "can go on " << x << "," << y << endl;
		}
		x++;
	} while (x * BO_TILE_SIZE <= rect.right());
	y++;
 } while (y * BO_TILE_SIZE <= rect.bottom());

 return true;
}

void BosonCanvas::setMap(BosonMap* map)
{
 d->mMap = map;
}

void BosonCanvas::addAnimation(BosonItem* item)
{
 d->mAnimList.append(item);
}

void BosonCanvas::removeAnimation(BosonItem* item)
{
 d->mAnimList.removeRef(item);
}

void BosonCanvas::unitMoved(Unit* unit, float oldX, float oldY)
{
 updateSight(unit, oldX, oldY);
 
// test if any unit has this unit as target. If sou then adjust the destination.
//TODO

// used to adjust the mini map
 emit signalUnitMoved(unit, oldX, oldY);
}

void BosonCanvas::updateSight(Unit* unit, float , float)
{
// TODO: use the float parameters - check whether the player can still see
// these coordinates and if not out fog on them again. Remember to check for -1
// (new unit placed)!

 unsigned int sight = unit->sightRange(); // *cell* number! not pixel number!
 unsigned int x = unit->boundingRect().center().x() / BO_TILE_SIZE;
 unsigned int y = unit->boundingRect().center().y() / BO_TILE_SIZE;

 int left = ((x > sight) ? (x - sight) : 0) - x;
 int top = ((y > sight) ? (y - sight) : 0) - y;
 int right = ((x + sight > d->mMap->width()) ?  d->mMap->width() :
		x + sight) - x;
 int bottom = ((y + sight > d->mMap->height()) ?  d->mMap->height() :
		y + sight) - y;
 
 sight *= sight;
// boDebug() << k_funcinfo << endl;
// boDebug() << "left=" << left << ",right=" << right << endl;
// boDebug() << "top=" << top << ",bottom=" << bottom << endl;

 for (int i = left; i < right; i++) {
	for (int j = top; j < bottom; j++) {
		if (i*i + j*j < (int)sight) {
			if (unit->owner()->isFogged(x + i, y + j)) {
				unit->owner()->unfog(x + i, y + j);
			}
		} else {
			//TODO
			// cell(i, j) is not in sight anymore. Check if any
			// other unit can see it!
			// if (we_cannot_see_this) {
			//	unit->owner()->fog(x + i, y + j);
			// }
		}
	}
 }
}

void BosonCanvas::newShot(BosonShot* shot)
{
 boDebug() << k_funcinfo << endl;

 if (!shot->isActive()) {
	shotHit(shot);
	d->mAnimList.removeRef(shot);
	d->mAllItems.removeItem(shot);
	delete shot;
 }
}

void BosonCanvas::shotHit(BosonShot* s)
{
 if (!s) {
	boError() << k_funcinfo << "NULL shot" << endl;
	return;
 }
 // Set age of flying particle systems (e.g. smoke traces) to 0 so they wont create any new particles
 QPtrListIterator<BosonParticleSystem> it(*(s->flyParticleSystems()));
 while (it.current()) {
	it.current()->setAge(0);
	++it;
 }
 // Add hit particle systems
 addParticleSystems(s->properties()->newHitParticleSystems(s->x(), s->y(), s->z()));
 // Decrease health of all units within damaging range of missile
 QValueList<Unit*> l = unitCollisionsInRange(QPoint(s->x(), s->y()),
		s->properties()->damageRange() * BO_TILE_SIZE - 10);  // - 2 is needed to prevent units on next cells from also being damaged
 for (unsigned int i = 0; i < l.count(); i++) {
	unitHit(l[i], s->properties()->damage());
	if (l[i]->isDestroyed()) {
		if (l[i]->isFacility()) {
			s->owner()->statistics()->addDestroyedFacility(l[i], s->owner());
		} else {
			s->owner()->statistics()->addDestroyedMobileUnit(l[i], s->owner());
		}
	}
 }
}

void BosonCanvas::unitHit(Unit* unit, long int damage)
{
 // Shield
 if (unit->shields() > 0) {
	if (unit->shields() >= (unsigned long int)damage) {
		// Unit will not be damaged (it has enough shields)
		unit->setShields(unit->shields() - damage);
		// TODO: show some shield animation
		return;
	} else {
		damage -= unit->shields();
		unit->setShields(0);
		// Also show shield animation?
	}
 }

 if (damage < 0) {
	unit->setHealth(unit->health() + ((unsigned long)-damage));
 } else {
	// Usually, unit's armor is substracted from attacker's weaponDamage, but
	//  if target has only little health left, then armor doesn't have full effect
	int health = (int)unit->health();
	if (health <= (int)(unit->unitProperties()->health() / 10.0)) {
		// If unit has only 10% or less of it's hitpoint left, armor has no effect (it's probably destroyed)
	} else if (health <= (int)(unit->unitProperties()->health() / 2.5)) {
		// Unit has 40% or less of hitpoints left. Only half of armor is "working"
		damage -= (int)(unit->armor() / 2.0);
	} else {
		damage -= unit->armor();
	}
	if (damage < 0) {
		damage = 0;
	}
	health -= damage;
	unit->setHealth((health >= 0) ? health : 0);
 }
 
 if (unit->isDestroyed()) {
	destroyUnit(unit); // display the explosion ; not the shoot
 } else {
/*	float factor = 2.0 - unit->health() / (unit->unitProperties()->health() / 2.0);
//	if (unit->health() <= (unit->unitProperties()->health() / 2.0)) {
	if (factor >= 1.0) {
		// If unit has less than 50% hitpoints, it's smoking
		BoVector3 pos((unit->x() + unit->width() / 2) * BO_GL_CELL_SIZE / (float)BO_TILE_SIZE,
				-((unit->y() + unit->height() / 2) * BO_GL_CELL_SIZE / (float)BO_TILE_SIZE),
				unit->z() * BO_GL_CELL_SIZE / (float)BO_TILE_SIZE);
		BosonParticleSystem* s;
		if (!unit->smokeParticleSystem()) {
			s = BosonParticleManager::newSmallSmoke(pos);
			unit->setSmokeParticleSystem(s);
			d->mParticles.append(s);
		}
		s = unit->smokeParticleSystem();
		// FIXME: maybe move this to BosonParticleManager?
		s->setCreateRate(factor * 25);
//		s->setVelocity(BoVector3(0, 0, factor * 0.5));  // This is only hint for BosonParticleManager
		float c = 0.8 - factor * 0.4;
		s->setColor(BoVector4(c, c, c, 0.25));

		// Facilities are burning too
		if (unit->isFacility()) {
			if (!((Facility*)unit)->flamesParticleSystem()) {
				s = BosonParticleManager::newFire(pos);
				((Facility*)unit)->setFlamesParticleSystem(s);
				d->mParticles.append(s);
			}
			s = ((Facility*)unit)->flamesParticleSystem();
			// FIXME: maybe move this to BosonParticleManager?
			s->setCreateRate(factor * 30);
			s->setVelocity(BoVector3(0, 0, factor * 0.5));  // This is only hint for BosonParticleManager
		}
	} else {
		// If it has more hitpoints, it's not burning ;-)
		if (unit->isFacility()) {
			if (((Facility*)unit)->flamesParticleSystem()) {
				((Facility*)unit)->flamesParticleSystem()->setAge(0);
			}
		}
		if (unit->smokeParticleSystem()) {
			unit->smokeParticleSystem()->setAge(0);
		}
	}*/
 }
}

void BosonCanvas::destroyUnit(Unit* unit)
{
 // please note: you MUST NOT delete the unit here!!
 // we call it from advance() and items must not be deleted from there!
 if (!unit) {
	return;
 }
 if (!d->mDestroyedUnits.contains(unit)) {
	boDebug() << "destroy unit " << unit->id() << endl;
	Player* owner = unit->owner();
	d->mDestroyedUnits.append(unit);

	if (unit->isFacility()) {
		boDebug() << k_funcinfo << "destoying facility" << endl;
		if (((Facility*)unit)->flamesParticleSystem()) {
			((Facility*)unit)->flamesParticleSystem()->setAge(0);
		}
	}
	if (unit->smokeParticleSystem()) {
		unit->smokeParticleSystem()->setAge(0);
	}

	// the unit is added to a list - now displayed as a wreckage only.
	removeUnit(unit);
	unit->playSound(SoundReportDestroyed);
	// Pos is center of unit
	BoVector3 pos(unit->x() + unit->width() / 2,unit->y() + unit->height() / 2, unit->z());
	addParticleSystems(unit->unitProperties()->newDestroyedParticleSystems(pos[0], pos[1], pos[2]));
	if (owner->checkOutOfGame()) {
		killPlayer(owner);
	}
 }
}

void BosonCanvas::removeUnit(Unit* unit)
{
 // please note: you MUST NOT delete the unit here!!
 // we call it from advance() and items must not be deleted from there!
 if (!unit) {
	return;
 }
 Player* owner = unit->owner();
 unit->setAnimated(false);
 unit->setHealth(0); // in case of an accidental change before
 unit->setWork(UnitBase::WorkDestroyed);
 owner->unitDestroyed(unit); // remove from player without deleting
 emit signalUnitRemoved(unit);

 // note: we don't add unit to any list and we don't delete it here.
 // editor will now delete it, while game mustn't delete it (displays wreckage)
}

Cell* BosonCanvas::cellAt(Unit* unit) const
{
 if (!unit) {
	return 0;
 }
 return cellAt(unit->x() + unit->width() / 2, unit->y() + unit->width() / 2);
}

Cell* BosonCanvas::cellAt(float x, float y) const
{
 return cell((int)(x / BO_TILE_SIZE), (int)(y / BO_TILE_SIZE));
}

BosonMap* BosonCanvas::map() const
{
 return d->mMap;
}

unsigned int BosonCanvas::mapWidth() const
{
 return map() ? map()->width() : 0;
}

unsigned int BosonCanvas::mapHeight() const
{
 return map() ? map()->height() : 0;
}

QValueList<Unit*> BosonCanvas::unitCollisionsInRange(const QPoint& pos, int radius) const
{
boDebug() << k_funcinfo << endl;
 BoItemList l = bosonCollisions(QRect(
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
//		boDebug() << "adding " << u->id() << endl;
		list.append(u);
	}
 }
 return list;
}

QValueList<Unit*> BosonCanvas::unitsAtCell(int x, int y) const
{
 if (!cell(x, y)) {
	return QValueList<Unit*>();
 }
 return cell(x, y)->items()->units(false);
}

bool BosonCanvas::cellOccupied(int x, int y) const
{
 if (!cell(x, y)) {
	return true;
 }
 return cell(x, y)->isOccupied();
}

bool BosonCanvas::cellOccupied(int x, int y, Unit* unit, bool excludeMoving) const
{
 if (!unit) {
	return cellOccupied(x, y);
 }
 if (unit->isFlying()) {
	return false; // even if there are other flying units - different altitudes!
 }
 if (!cell(x, y)) {
	boError() << k_funcinfo << "NULL cell" << endl;
	return true;
 }
 bool includeMoving = !excludeMoving; // FIXME: replace exclude by include in parameter
 return cell(x, y)->isOccupied(unit, includeMoving);
}

bool BosonCanvas::cellsOccupied(const QRect& rect, Unit* unit, bool excludeMoving) const
{
 const int left = rect.left() / BO_TILE_SIZE;
 const int top = rect.top() / BO_TILE_SIZE;
 const int right = rect.right() / BO_TILE_SIZE + ((rect.right() % BO_TILE_SIZE == 0) ? 0 : 1);
 const int bottom = rect.bottom() / BO_TILE_SIZE + ((rect.bottom() % BO_TILE_SIZE == 0) ? 0 : 1);

 for (int x = left; x < right; x++) {
	for (int y = top; y < bottom; y++) {
		if (cellOccupied(x, y, unit, excludeMoving)) {
			return true;
		}
	}
 }
 return false;
}

void BosonCanvas::killPlayer(Player* player)
{
 while (player->allUnits().count() > 0) {
	destroyUnit(player->allUnits().first());
 }
 player->setMinerals(0);
 player->setOil(0);
 boDebug() << "player " << player->id() << " is out of game" << endl;
 emit signalOutOfGame(player);
}

void BosonCanvas::removeFromCells(BosonItem* item)
{
 QPointArray cells = item->cells();
 for (unsigned int i = 0; i < cells.count(); i++) {
	Cell* c = cell(cells[i].x(), cells[i].y());
	if (!c) {
		boError() << k_funcinfo << "NULL cell - x=" << cells[i].x() << ",y=" << cells[i].y() << endl;
		continue;
	}
	c->removeItem(item);
 }
}

void BosonCanvas::addToCells(BosonItem* item)
{
 QPointArray cells = item->cells();
 for (unsigned int i = 0; i < cells.count(); i++) {
	Cell* c = cell(cells[i].x(), cells[i].y());
	if (!c) {
		boError() << k_funcinfo << "NULL cell - x=" << cells[i].x() << ",y=" << cells[i].y() << endl;
		continue;
	}
	c->addItem(item);
 }
}

bool BosonCanvas::canPlaceUnitAt(const UnitProperties* prop, const QPoint& pos, ProductionPlugin* factory) const
{
 int width = prop->unitWidth();
 int height= prop->unitHeight();
 if (!width) {
	boError() << k_funcinfo << "null width for " << prop->typeId() << endl;
	return false;
 }
 if (!height) {
	boError() << k_funcinfo << "null height for " << prop->typeId() << endl;
	return false;
 }
 QRect r(pos.x(), pos.y(), width, height);
 if (!canGo(prop, r)) {
	return false;
 }
 if (cellsOccupied(r)) {
	return false;
 }
 if (!factory) {
	return true;
 }
 if (prop->isMobile()) {
	// must be in BUILD_RANGE of factory
	// not perfect - there is alays a distance from center() to the edge of
	// both units which should also added to this. but this is not a maths
	// contest, so its ok this way
	Unit* factoryUnit = factory->unit();
	if (!factoryUnit) {
		boError() << k_funcinfo << "production plugin has NULL owner" << endl;
		return false;
	}
	int dx = QABS(r.center().x() - factoryUnit->boundingRect().center().x());
	int dy = QABS(r.center().y() - factoryUnit->boundingRect().center().y());
	if (dx * dx + dy * dy <= BUILD_RANGE * BUILD_RANGE) {
		return true;
	}
 } else {
	// must be in BUILD_RANGE of any facility of the player
	QValueList<Unit*> list = unitCollisionsInRange(r.center(), BUILD_RANGE);
	for (unsigned int i = 0; i < list.count(); i++) {
		if (list[i]->isFacility() && list[i]->owner() == factory->player()) {
			return true;
		}
	}
 }
 return false;
}

BosonTiles* BosonCanvas::tileSet() const
{
 if (!d->mMap) {
	return 0;
 }
 return d->mMap->tileSet();
}

BoItemList BosonCanvas::allBosonItems() const
{
 return d->mAllItems;
}

void BosonCanvas::addItem(BosonItem* item)
{
 d->mAllItems.append(item);
}

void BosonCanvas::removeItem(BosonItem* item)
{
 d->mAllItems.remove(item);
}


// this is an extremely time-critical function!
BoItemList BosonCanvas::bosonCollisions(const QPointArray& cells, const BosonItem* item, bool exact) const
{
 // FIXME: if exact is true we assume that cells == item->cells() !!
// AB: item can be NULL, too!
 BoItemList collisions;
 BoItemList seen;
 const BoItemList* cellItems;
 Cell* c = 0;
 for (unsigned int i = 0; i < cells.count(); i++) {
	c = cell(cells[i].x(), cells[i].y());
	if (!c) {
		boWarning(310) << k_funcinfo << "NULL cell " << cells[i].x() << " " << cells[i].y() << endl;
		if (cells[i].x() < 0) {
			boError(310) << k_funcinfo << "x < 0 - please check the calling funktion! this shouldn't happen!" << endl;
		}
		if (cells[i].y() < 0) {
			boError(310) << k_funcinfo << "y < 0 - please check the calling funktion! this shouldn't happen!" << endl;
		}
		continue;
	}
	cellItems = c->items();
	BoItemList::ConstIterator it;
	for (it = cellItems->begin(); it != cellItems->end(); ++it) {
		BosonItem* s = *it;
		if (s != item) {
			if (seen.findIndex(s) < 0 && (!item || !exact || item->bosonCollidesWith(s))) {
				seen.append(s);
				collisions.append(s);
			}
		}
	}
 }
 return collisions;
}

BoItemList BosonCanvas::bosonCollisions(const QRect& r) const
{
 // r is canvas coordinates!
 QPointArray cells;
 int left, right, top, bottom;
 left = QMAX(r.left() / BO_TILE_SIZE, 0);
 right = QMIN(r.right() / BO_TILE_SIZE, QMAX((int)mapWidth() - 1, 0));
 top = QMAX(r.top() / BO_TILE_SIZE, 0);
 bottom = QMIN(r.bottom() / BO_TILE_SIZE, QMAX((int)mapHeight() - 1, 0));
 int size = (right - left + 1) * (bottom - top + 1);
 if (size <= 0) {
	return BoItemList();
 }
 cells.resize(size);
 int n = 0;
 for (int i = left; i <= right; i++) {
	for (int j = top; j <= bottom; j++) {
		cells[n++] = QPoint(i, j);
	}
 }
 return bosonCollisions(cells, 0, true);// FIXME: exact = true has no effect
}

BoItemList BosonCanvas::bosonCollisions(const QPoint& pos) const
{
 // pos is canvas coordinates!
 QPointArray cells(1);
 cells[0] = pos / BO_TILE_SIZE;
 boDebug(310) << k_funcinfo << cells[0].x() << " " << cells[0].y() << endl;
 return bosonCollisions(cells, 0, true); // FIXME: ecact = true has no effect
}

int BosonCanvas::particleSystemsCount()
{
 return d->mParticles.count();
}

QPtrList<BosonParticleSystem>* BosonCanvas::particleSystems()
{
 return &(d->mParticles);
}

void BosonCanvas::updateParticleSystems(float elapsed)
{
/* int count = d->mParticles.count();
 if (count <= 0) {
	return;
 }
	BosonParticleSystem* s;
	for (int i = 0; i < count; i++) {
	s = d->mParticles.at(i);
	s->update(elapsed);
	if (!s->isActive()) {
		boDebug() << k_funcinfo << "**********  REMOVING inactive particle system (particle count: " << s->particleCount() << ")!  *****" << endl;
		d->mParticles.remove();
		i--;
		count--;
	}
 }*/
 for (BosonParticleSystem* s = d->mParticles.first(); s; s = d->mParticles.next()) {
	s->update(elapsed);
	if (!s->isActive()) {
		d->mParticles.removeRef(s);
	}
 }
}

void BosonCanvas::deleteUnusedShots()
{
 for (BosonItem* i = d->mAnimList.first(); i; i = d->mAnimList.next()) {
	if (RTTI::isShot(i->rtti())) {
		if (!((BosonShot*)i)->isActive()) {
			shotHit((BosonShot*)i);
			d->mAnimList.remove();
			d->mAllItems.removeItem(i);
			delete i;
		}
	}
 }
}

void BosonCanvas::addParticleSystem(BosonParticleSystem* s)
{
 d->mParticles.append(s);
}

void BosonCanvas::addParticleSystems(const QPtrList<BosonParticleSystem> systems)
{
 QPtrListIterator<BosonParticleSystem> it(systems);
 BosonParticleSystem* s;
 while ((s = it.current()) != 0) {
	++it;
	addParticleSystem(s);
 }
}
