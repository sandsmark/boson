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

#include "unit.h"
#include "player.h"
#include "bosoncanvas.h"
#include "boson.h"
#include "cell.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "bosonpath.h"
#include "selectbox.h"
#include "bosonmessage.h"
#include "bosonstatistics.h"
#include "kspritetooltip.h"
#include "unitplugins.h"
#include "boitemlist.h"
#include "bosonmodel.h"

#include <kgame/kgamepropertylist.h>
#include <kgame/kgame.h>

#include "defines.h"

class Unit::UnitPrivate
{
public:
	UnitPrivate()
	{
		mSelectBox = 0;
		mTarget = 0;
	}
	KGamePropertyInt mDirection;

	KGamePropertyList<QPoint> mWaypoints;
	KGameProperty<int> mMoveDestX;
	KGameProperty<int> mMoveDestY;
	KGameProperty<int> mMoveRange;

	// be *very* careful with those - NewGameDialog uses Unit::save() which
	// saves all KGameProperty objects. If non-KGameProperty properties are
	// changed before all players entered the game we'll have a broken
	// network game.
	Unit* mTarget;

	SelectBox* mSelectBox;
};

Unit::Unit(const UnitProperties* prop, Player* owner, BosonCanvas* canvas) 
		: UnitBase(prop), 
#ifndef NO_OPENGL
		BosonSprite(owner->speciesTheme() ? owner->speciesTheme()->unitModel(prop->typeId()) : 0, canvas)
#else
		BosonSprite(owner->speciesTheme() ? owner->speciesTheme()->pixmapArray(prop->typeId()) : 0, canvas)
#endif
{
 d = new UnitPrivate;
 setOwner(owner);

 d->mDirection.registerData(IdDirection, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Direction");
 d->mWaypoints.registerData(IdWaypoints, dataHandler(),
		KGamePropertyBase::PolicyLocal, "Waypoints");
 d->mMoveDestX.registerData(IdMoveDestX, dataHandler(),
		KGamePropertyBase::PolicyLocal, "MoveDestX");
 d->mMoveDestY.registerData(IdMoveDestY, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "MoveDestY");
 d->mMoveRange.registerData(IdMoveRange, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "MoveRange");

 d->mDirection.setLocal(0); // not yet used
 setAnimated(true);
 d->mMoveDestX.setLocal(0);
 d->mMoveDestY.setLocal(0);
 d->mMoveRange.setLocal(0);

 // TODO: the tooltips do not yet work with OpenGL!!
#ifdef NO_OPENGL
 KSpriteToolTip::add(rtti(), unitProperties()->name());
#else
 if (!model()) {
	kdError() << k_funcinfo << "NULL model - this will most probably crash!" << endl;
	return;
 }
 model()->setFrame(0);
#endif
}

Unit::~Unit()
{
#ifdef NO_OPENGL
 KSpriteToolTip::remove(this);
#endif
 d->mWaypoints.setEmittingSignal(false); // just to prevent warning in Player::slotUnitPropertyChanged()
 d->mWaypoints.clear();
 unselect();
 delete d;
}

void Unit::select(bool markAsLeader)
{
 if (isDestroyed()) {
	return; // shall we really return?
 }
 BosonSprite::select(markAsLeader);
 updateSelectBox();
}

int Unit::destinationX() const
{
 return d->mMoveDestX;
}

int Unit::destinationY() const
{
 return d->mMoveDestY;
}

int Unit::moveRange() const
{
 return d->mMoveRange;
}

Unit* Unit::target() const
{
 return d->mTarget;
}

void Unit::setTarget(Unit* target)
{
 d->mTarget = target;
 if (!d->mTarget) {
	return;
 }
 if (d->mTarget->isDestroyed()) {
	 d->mTarget = 0;
 }
}

void Unit::setHealth(unsigned long int h)
{
 unsigned long int maxHealth = unitProperties()->health();
 if (h > maxHealth) {
	h = maxHealth;
 }
 if (maxHealth == 0) {
	kdError() << "Ooop - maxHealth == 0" << endl;
	return;
 }
 UnitBase::setHealth(h);
 updateSelectBox();
 if (isDestroyed()) {
	unselect();
	setFrame(frameCount() - 1);
	if (unitProperties()->isMobile()) {
		setZ(Z_DESTROYED_MOBILE);
	} else {
		setZ(Z_DESTROYED_FACILITY);
	}
	setAnimated(false);
 }
}

void Unit::updateSelectBox()
{
 if (selectBox()) {
	unsigned long int maxHealth = unitProperties()->health();
	double div = (double)health() / maxHealth;
	selectBox()->update(div);
	selectBox()->setVisible(true);
 }
}

void Unit::moveBy(float moveX, float moveY, float moveZ)
{
// time critical function

 if (!moveX && !moveY && !moveZ) {
	return;
 }

 // QCanvasItem::moveBy() is called from QCanvasItem::advance(1). I finally
 // found out why it is a bad idea (tm) to do collision detection here.
 // QCanvas::collisions() (and all other collisions()) use imageAdvanced() for
 // collision detection.
 // collision detection of item A may found none - so it is moved. but item B
 // may find item A as collision candidate. but it now tests for the *next*
 // advance() pahse, as A has already been moved. so it may happen that item B
 // is ok, too, as item A won't be in the way in the next phase.
 // This means that be will be moved, too, but it mustn't be moved - we have a
 // collision.
 float oldX = x();
 float oldY = y();
 boCanvas()->removeFromCells(this);
 BosonSprite::moveBy(moveX, moveY, moveZ);
#ifdef NO_OPENGL
 if (selectBox()) {
	// im pretty sure we won't need moveZ in the select box.. not yet ;)
	selectBox()->moveBy(moveX, moveY, 0.0);
 }
#endif
 boCanvas()->addToCells(this);
 boCanvas()->unitMoved(this, oldX, oldY);
}

void Unit::advance(int phase)
{ // time critical function !!!
// kdDebug() << k_funcinfo << " id=" << id() << endl;
 if (isDestroyed()) {
	return;
 }
 if (phase == 0) {
	reloadWeapon();
 } else { // phase == 1
	// QCanvasSprite::advance() just moves for phase == 1 ; let's do it
	// here, too. Collision detection is done is phase == 0 - in all of the
	// other advance*() functions.
	if (xVelocity() || yVelocity()) {
		moveBy(xVelocity(), yVelocity(), 0.0);
	}
 }
}

void Unit::advanceNone()
{
// this is called when the unit has nothing specific to do. Usually we just want
// to fire at every enemy in range.

 if (weaponDamage() > 0) {
	BoItemList list = enemyUnitsInRange();
	if (list.count() > 0) {
		BoItemList::Iterator it = list.begin();
		for (; it != list.end(); ++it) {
			if (((Unit*)*it)->unitProperties()->canShoot()) {
				shootAt((Unit*)*it);
				break;
			}
		}
		if (it != list.end()) {
			// no military unit (i.e. unit that can shoot) found
			shootAt((Unit*)*it);
		}
	}
 } else if (weaponDamage() < 0) {
	if (!repairPlugin()) {
		kdWarning() << k_funcinfo << "weaponDamage < 0 but no repair plugin??" << endl;
		return;
	}
	repairPlugin()->repairInRange();
 } else {
	// weaponDamage() == 0 - what can be done here?
 }
}

void Unit::advanceAttack()
{
 kdDebug() << k_funcinfo << endl;
 if (!target()) {
	kdWarning() << k_funcinfo << "cannot attack NULL target" << endl;
	stopAttacking();
	return;
 }
 if (target()->isDestroyed()) {
	kdDebug() << "Target is destroyed!" << endl;
	stopAttacking();
	return;
 }
 if (!inRange(target())) {
	if (!boCanvas()->allBosonItems().contains(target())) {
		kdDebug() << "Target seems to be destroyed!" << endl;
		stopAttacking();
		return;
	}
	kdDebug() << "unit (" << target()->id() << ") not in range - moving..." << endl;
	if (!moveTo(target()->x(), target()->y(), weaponRange())) {
		setWork(WorkNone);
	} else {
		setAdvanceWork(WorkMove);
	}
	return;
 }
 shootAt(target());
 if (target()->isDestroyed()) {
	stopAttacking();
 }

}

void Unit::addWaypoint(const QPoint& pos)
{
 d->mWaypoints.append(pos);
}

void Unit::waypointDone()
{
 d->mWaypoints.remove(d->mWaypoints.at(0));
}

QValueList<QPoint> Unit::waypointList() const
{
 return d->mWaypoints;
}

unsigned int Unit::waypointCount() const
{
 return d->mWaypoints.count();
}

void Unit::moveTo(const QPoint& pos)
{
 d->mTarget = 0;
 if(moveTo(pos.x(), pos.y(), 0)) {
	setWork(WorkMove);
 } else {
	setWork(WorkNone);
 }
}

bool Unit::moveTo(float x, float y, int range)
{
 stopMoving();

 if (range == -1) {
	range = d->mMoveRange;
 }
 if(!owner()->isFogged((int)(x / BO_TILE_SIZE), (int)(y / BO_TILE_SIZE))) {
	Cell* c = boCanvas()->cell((int)(x / BO_TILE_SIZE), (int)(y / BO_TILE_SIZE));
	if (!c) {
		kdError() << k_funcinfo << "NULL cell at " << x << "," << y << endl;
		return false;
	}
	// No pathfinding if goal not reachable or occupied and we can see it
	if(!c->canGo(unitProperties())) {
		return false;
	}
 }

 d->mMoveDestX = (int)x;
 d->mMoveDestY = (int)y;
 d->mMoveRange = range;

 // AB: FIXME: UnitGroups doesn't exist anymore! maybe we can search path here,
 // now?
 // Do not find path here!!! It would break pathfinding for groups. Instead, we
 //  set mSearchPath to true and find path in MobileUnit::advanceMove()
 mSearchPath = true;

 return true;
}

void Unit::newPath()
{
 kdDebug() << k_funcinfo << endl;
 if(!owner()->isFogged(d->mMoveDestX / BO_TILE_SIZE, d->mMoveDestY / BO_TILE_SIZE)) {
	Cell* destCell = boCanvas()->cell(d->mMoveDestX / BO_TILE_SIZE,
			d->mMoveDestY / BO_TILE_SIZE);
	if(!destCell || (!destCell->canGo(unitProperties()))) {
		// If we can't move to destination, then we add waypoint with coordinates
		//  -1; -1 and in MobileUnit::advanceMove(), if currentWaypoint()'s
		//  coordinates are -1; -1 then we stop moving.
		clearWaypoints();
		addWaypoint(QPoint(-1, -1));
		return;
	}
 }
 // Only go until enemy is in range if we are attacking
 QValueList<QPoint> path = BosonPath::findPath(this, d->mMoveDestX, d->mMoveDestY, d->mMoveRange);
 clearWaypoints();
 for (int unsigned i = 0; i < path.count(); i++) {
	addWaypoint(path[i]);
 }
 if((currentWaypoint().x() == x() + width() / 2) && (currentWaypoint().y() == y() + height() / 2))
 {
	kdDebug() << k_funcinfo << "!!!!! First waypoint is unit's current pos! Removing" << endl;
	waypointDone();
 }
 if(waypointCount() == 0)
 {
	addWaypoint(QPoint(-1, -1));
 }
 return;
}

void Unit::clearWaypoints()
{
 d->mWaypoints.clear();
}

const QPoint& Unit::currentWaypoint() const
{
 return d->mWaypoints[0];
}

void Unit::stopMoving()
{
// kdDebug() << k_funcinfo << endl;
 clearWaypoints();

 // Call this only if we are only moving - stopMoving() is also called e.g. on
 // WorkAttack, when the unit is not yet in range.
 if (isMoving()) {
	setWork(WorkNone);
 } else if (advanceWork() != work()) {
	setAdvanceWork(work());
 }
 setXVelocity(0);
 setYVelocity(0);
}

void Unit::stopAttacking()
{
 stopMoving(); // FIXME not really intuitive... nevertheless its currently useful.
 setTarget(0);
 setWork(WorkNone);
}

bool Unit::save(QDataStream& stream)
{
 if (!UnitBase::save(stream)) {
	kdError() << "Unit not saved properly" << endl;
	return false;
 }
 stream << (float)x();
 stream << (float)y();
 stream << (float)z();
 stream << (Q_INT8)isVisible();
 stream << (Q_INT32)frame();
 return true;
}

bool Unit::load(QDataStream& stream)
{
 if (!UnitBase::load(stream)) {
	kdError() << "Unit not loaded properly" << endl;
	return false;
 }
 float x;
 float y;
 float z;
 Q_INT8 visible;
 Q_INT32 frame;
 
 stream >> x;
 stream >> y;
 stream >> z;
 stream >> visible;
 stream >> frame;

 setX(x);
 setY(y);
 setZ(z);
 setVisible(visible);
 if (isDestroyed()) {
	kdError() << k_funcinfo << "unit is already destroyed" << endl;
 } else {
	setFrame(frame);
 }
 return true;
}

bool Unit::inRange(Unit* target) const
{
 // maybe we should use an own algorithm here - can be faster than this generic
 // one
 return unitsInRange().contains(target);
}

void Unit::shootAt(Unit* target)
{
 if (reloadState() != 0) {
//	kdDebug() << "gotta reload first" << endl;
	return;
 }
 if (target->isFlying()) {
	if (!unitProperties()->canShootAtAirUnits()) {
		return;
	}
 } else {
	if (!unitProperties()->canShootAtLandUnits()) {
		return;
	}
 }
 if (target->isDestroyed()) {
	kdWarning() << k_funcinfo << target->id() << " is already destroyed" << endl;
	return;
 }
 kdDebug() << id() << " shoots at unit " << target->id() << endl;
 ((BosonCanvas*)canvas())->shootAtUnit(target, this, weaponDamage());
 if (target->isDestroyed()) {
	if (target->isFacility()) {
		owner()->statistics()->addDestroyedFacility((Facility*)target, this);
	} else {
		owner()->statistics()->addDestroyedMobileUnit((MobileUnit*)target, this);
	}
 }
 owner()->statistics()->increaseShots();
 resetReload();
}

BoItemList Unit::unitsInRange() const
{
 // TODO: we use a *rect* for the range this is extremely bad.
 // ever heard about pythagoras ;-) ?

 QPointArray cells;
 int left, right, top, bottom;
 leftTopCell(&left, &top);
 rightBottomCell(&right, &bottom);
 left = QMAX(left - (int)weaponRange(), 0);
 top = QMAX(top - (int)weaponRange(), 0);
 right = QMIN(right + (int)weaponRange(), QMAX((int)boCanvas()->mapWidth() - 1, 0));
 bottom = QMIN(bottom + (int)weaponRange(), QMAX((int)boCanvas()->mapHeight() - 1, 0));
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

 BoItemList items = boCanvas()->bosonCollisions(cells, (BosonSprite*)this, false);
 items.remove((BosonSprite*)this);
 BoItemList inRange;
 BoItemList::Iterator it = items.begin();
 for (; it != items.end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	if(((Unit*)(*it))->isDestroyed()) {
		continue;
	}
	// TODO: remove the items from inRange which are not actually in range (hint:
	// pythagoras)
	inRange.append(*it);
 }
 return inRange;
}

BoItemList Unit::enemyUnitsInRange() const
{
 BoItemList units = unitsInRange();
 BoItemList enemy;
 BoItemList::Iterator it = units.begin();
 for (; it != units.end(); ++it) {
	Unit* u = (Unit*)*it;
	if (owner()->isEnemy(u->owner())) {
		enemy.append(u);
	}
 }
 return enemy;
}

QValueList<Unit*> Unit::unitCollisions(bool exact) const
{
 QValueList<Unit*> units;
 if (isFlying()) { // flying units never collide - different altitudes
	return units;
 }
 kdDebug() << k_funcinfo << endl;
 BoItemList collisionList = boCanvas()->bosonCollisions(cells(), (BosonSprite*)this, exact);
 if (collisionList.isEmpty()) {
	return units;
 }
 
 BoItemList::Iterator it;
 for (it = collisionList.begin(); it != collisionList.end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	Unit* unit = ((Unit*)*it);
	if (unit->isDestroyed()) {
		continue;
	}
	if (unit->isFlying()) {
		continue;
	}
	units.append(unit);
 }
 return units;
}

void Unit::setAdvanceWork(WorkType w)
{
 // velicities should be 0 anyway - this is the final fallback in case it was 
 // missing by any reason
 setXVelocity(0);
 setYVelocity(0);
 if (w != advanceWork() || isDestroyed()) {
	boCanvas()->setWorkChanged(this);
 }
 UnitBase::setAdvanceWork(w);
}

bool Unit::isNextTo(Unit* target) const
{
 //const int r = BO_TILE_SIZE;
 const int r = 10;
 // in theory r = 1 should be enough... both of the above make problems under
 // certain circumstances
 if (QABS(rightEdge() - target->leftEdge()) <= r ||
		QABS(leftEdge() - target->rightEdge()) <= r ||
		rightEdge() <= target->rightEdge() && leftEdge() <= target->leftEdge()// will never happen with current pixmaps
		) { 
	if (QABS(topEdge() - target->bottomEdge() <= r) ||
			QABS(bottomEdge() - target->topEdge()) <= r||
			topEdge() <= target->topEdge() && bottomEdge() <= target->bottomEdge()// will never happen with current pixmaps
			) {
		kdDebug() << "ok - inrange" << endl;
		return true;
	}
 }
 return false;
}


/////////////////////////////////////////////////
// MobileUnit
/////////////////////////////////////////////////

class HarvesterProperties
{
public:
	KGameProperty<int> mResourcesX;
	KGameProperty<int> mResourcesY;
	KGameProperty<unsigned int> mResourcesMined;
};

class MobileUnit::MobileUnitPrivate
{
public:
	MobileUnitPrivate()
	{
		mHarvesterProperties = 0;
		mTargetCellMarked = false;
	}

	KGameProperty<float> mSpeed;
	KGameProperty<unsigned int> mMovingFailed;
	KGameProperty<unsigned int> mPathRecalculated;

	HarvesterProperties* mHarvesterProperties;
	bool mTargetCellMarked;
};

MobileUnit::MobileUnit(const UnitProperties* prop, Player* owner, BosonCanvas* canvas) : Unit(prop, owner, canvas)
{
 d = new MobileUnitPrivate;
 d->mSpeed.registerData(IdSpeed, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Speed");
 d->mMovingFailed.registerData(IdMob_MovingFailed, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "MovingFailed");
 d->mPathRecalculated.registerData(IdMob_PathRecalculated, dataHandler(),
		KGamePropertyBase::PolicyLocal, "PathRecalculated");
 d->mSpeed.setLocal(0);
 d->mMovingFailed.setLocal(0);
 d->mPathRecalculated.setLocal(0);

 d->mMovingFailed.setEmittingSignal(false);
 d->mPathRecalculated.setEmittingSignal(false);

 if (unitProperties()->canMineMinerals() || unitProperties()->canMineOil()) {
	d->mHarvesterProperties = new HarvesterProperties;
	d->mHarvesterProperties->mResourcesMined.registerData(IdMob_ResourcesMined, dataHandler(), 
			KGamePropertyBase::PolicyLocal, "ResourcesMined");
	d->mHarvesterProperties->mResourcesMined.setLocal(0);
	d->mHarvesterProperties->mResourcesX.registerData(IdMob_ResourcesX, dataHandler(), 
			KGamePropertyBase::PolicyLocal, "ResourcesX");
	d->mHarvesterProperties->mResourcesX.setLocal(0);
	d->mHarvesterProperties->mResourcesY.registerData(IdMob_ResourcesY, dataHandler(), 
			KGamePropertyBase::PolicyLocal, "ResourcesY");
	d->mHarvesterProperties->mResourcesY.setLocal(0);
 }
 setWork(WorkNone);
}

MobileUnit::~MobileUnit()
{
 delete d;
}

void MobileUnit::advanceMove()
{
 if (speed() == 0) {
	kdWarning() << "speed == 0" << endl;
	stopMoving();
	return;
 }


 if(mSearchPath) {
	newPath();
	mSearchPath = false;
	return;
 }

 if(waypointCount() == 0) {
	// Waypoints were PolicyClean previously but are now PolicyLocal so they
	//  should arrive immediately. If there are no waypoints but advanceMove is
	//  called, then probably there's an error somewhere
	kdError() << k_funcinfo << "No waypoints" << endl;
	stopMoving();
	return;
 }

 kdDebug() << k_funcinfo << endl;
 if (advanceWork() != work()) {
	if (work() == WorkAttack) {
		// no need to move to the position of the unit...
		// just check if unit is in range now.
		if (inRange(target())) {
			kdDebug() << k_funcinfo << "target is in range now" << endl;
			stopMoving();
			return;
		}
		// TODO: make sure that target() hasn't moved!
		// if it has moved also adjust waypoints
	} else if (work() == WorkRefine) {
		if (isNextTo(refinery())) {
			kdDebug() << k_funcinfo << "refinery in range now" << endl;
			stopMoving();
			return;
		}
	}
 }

 QPoint wp = currentWaypoint(); // where we go to
 // If both waypoint's coordinates are -1, then it means that path to
 //  destination can't be found and we should stop
 if((wp.x() == -1) &&(wp.y() == -1)) {
	stopMoving();
	return;
 }

 // FIXME: as path is now recalculated every time waypoint is reached, this
 //  should never be called
 if((wp.x() == -2) &&(wp.y() == -2)) {
	clearWaypoints();
	newPath();
	return;
 }

 /*
 int x = (int)(QCanvasSprite::x() + width() / 2);
 int y = (int)(QCanvasSprite::y() + height() / 2);
 */
#warning FIXME!!
 int x = (int)(BosonSprite::x() + width() / 2);
 int y = (int)(BosonSprite::y() + height() / 2);

 float xspeed = 0;
 float yspeed = 0;

 // First check if we're at waypoint
 if((x == wp.x()) && (y == wp.y())) {
	kdDebug() << k_funcinfo << "unit is at waypoint" << endl;
	waypointDone();

	if(waypointCount() == 0) {
		kdDebug() << k_funcinfo << "no more waypoints. Stopping moving" << endl;
		stopMoving();
		return;
	}

	// We now recalc path _every_ time we reach waypoint
	//  Units will then react more quickly when other units move and block their
	//  way for example
	newPath();

	wp = currentWaypoint();
 }

 // Check if we can actually go to waypoint (maybe it was fogged)
 // FIXME: currentWaypoint should have been unfogged when path was calculated
 //  because we now recalc path after every waypoint (see ~5 lines above)
 if(!boCanvas()->cell(wp.x() / BO_TILE_SIZE, wp.y() / BO_TILE_SIZE) ||
		!boCanvas()->cell(wp.x() / BO_TILE_SIZE, wp.y() / BO_TILE_SIZE)->canGo(unitProperties())) {
	kdWarning() << "cannot go to waypoint, finding new path" << endl;
	setXVelocity(0);
	setYVelocity(0);
	// We have to clear waypoints first to make sure that they aren't used next
	//  advance() call (when new waypoints haven't arrived yet)
	// FIXME: no need to clear them anymore
	clearWaypoints();
	newPath();
	return;
 }

 // Try to go to same x and y coordinates as waypoint's coordinates
 // First x coordinate
 // Slow down if there is less than speed() pixels to go
 if(QABS(wp.x() - x) < speed()) {
	xspeed = wp.x() - x;
 } else {
	xspeed = speed();
	if(wp.x() < x) {
		xspeed = -xspeed;
	}
 }
 // Same with y coordinate
 if(QABS(wp.y() - y) < speed()) {
	yspeed = wp.y() - y;
 } else {
	yspeed = speed();
	if(wp.y() < y) {
		yspeed = -yspeed;
	}
 }

 // Set velocity for actual moving
 setVelocity(xspeed, yspeed);

 // set the new direction according to new speed
 turnTo();
}

void MobileUnit::advanceMoveCheck()
{
 if (!canvas()->onCanvas(boundingRectAdvanced().topLeft())) {
	kdDebug() << k_funcinfo << "not on canvas" << endl;
	stopMoving();
	setWork(WorkNone);
	return;
 }
 if (!canvas()->onCanvas(boundingRectAdvanced().bottomRight())) {
	kdDebug() << k_funcinfo << "not on canvas" << endl;
	stopMoving();
	setWork(WorkNone);
	return;
 }
 kdDebug() << k_funcinfo << endl;
 if (boCanvas()->cellOccupied(currentWaypoint().x() / BO_TILE_SIZE,
		currentWaypoint().y() / BO_TILE_SIZE, this, false)) {
//	kdDebug() << k_funcinfo << "collisions" << endl;
//	kdWarning() << k_funcinfo << "" << id() << " -> " << l.first()->id() 
//		<< " (count=" << l.count() <<")"  << endl;
	// do not move at all. Moving is not stopped completely!
	// work() is still workMove() so we'll continue moving in the next
	// advanceMove() call

	d->mMovingFailed = d->mMovingFailed + 1;
	setXVelocity(0);
	setYVelocity(0);

	const int recalculate = 50; // recalculate when 50 advanceMove() failed
	if(d->mPathRecalculated >= 2) {
		kdDebug() << k_funcinfo << "Path recalculated 3 times and it didn't help, giving up and stopping" << endl;
		stopMoving();
		return;
	}
	if (d->mMovingFailed >= recalculate) {
		kdDebug() << "recalculating path" << endl;
		// you must not do anything that changes local variables directly here!
		// all changed of variables with PolicyClean are ok, as they are sent
		// over network and do not take immediate effect.

		newPath();
		d->mMovingFailed = 0;
		d->mPathRecalculated = d->mPathRecalculated + 1;
	}
	return;
 }
 else if (! d->mTargetCellMarked) {
	boCanvas()->cell(currentWaypoint().x() / BO_TILE_SIZE, currentWaypoint().y() / BO_TILE_SIZE)->willBeOccupiedBy(this);
	d->mTargetCellMarked = true;
 }
 d->mMovingFailed = 0;
 d->mPathRecalculated = 0;
 kdDebug() << k_funcinfo << "done" << endl;
}

void MobileUnit::setSpeed(float speed)
{
 d->mSpeed = speed;
}

float MobileUnit::speed() const
{
 return d->mSpeed;
}

void MobileUnit::turnTo(Direction direction)
{
 if (isDestroyed()) {
	kdError() << k_funcinfo << "unit is already destroyed!" << endl;
	return;
 }
 setFrame((int)direction);
}

void MobileUnit::turnTo()
{
 float xspeed = xVelocity();
 float yspeed = yVelocity();
 // Set correct frame
 if((xspeed == 0) && (yspeed < 0)) { // North
 	turnTo(North);
 } else if((xspeed > 0) && (yspeed < 0)) { // NE
	turnTo(NorthEast);
 } else if((xspeed > 0) && (yspeed == 0)) { // East
	turnTo(East);
 } else if((xspeed > 0) && (yspeed > 0)) { // SE
	turnTo(SouthEast);
 } else if((xspeed == 0) && (yspeed > 0)) { // South
	turnTo(South);
 } else if((xspeed < 0) && (yspeed > 0)) { // SW
	turnTo(SouthWest);
 } else if((xspeed < 0) && (yspeed == 0)) { // West
	turnTo(West);
 } else if((xspeed < 0) && (yspeed < 0)) { // NW
	turnTo(NorthWest);
 } else if (xspeed == 0 && yspeed == 0) {
//	kdDebug() << k_funcinfo << "xspeed == 0 and yspeed == 0" << endl;
 } else {
	kdDebug() << k_funcinfo << "error when setting frame" << endl;
 }
}

void MobileUnit::advanceMine()
{
 kdDebug() << k_funcinfo << endl;
 if (!d->mHarvesterProperties) {
	setWork(WorkNone);
	return;
 }
 if (resourcesMined() < unitProperties()->maxResources()) {
	if (canMine(boCanvas()->cellAt(this))) {
		const int step = (resourcesMined() + 10 <= unitProperties()->maxResources()) ? 10 : unitProperties()->maxResources() - resourcesMined();
		d->mHarvesterProperties->mResourcesMined = resourcesMined() + step;
		if (unitProperties()->canMineMinerals()) {
			owner()->statistics()->increaseMinedMinerals(step);
		} else if (unitProperties()->canMineOil()) {
			owner()->statistics()->increaseMinedOil(step);
		}
		kdDebug() << "resources mined: " << resourcesMined() << endl;
	} else {
		kdDebug() << k_funcinfo << "cannot mine here" << endl;
		setWork(WorkNone);
		return;
	}
 } else {
	kdDebug() << k_funcinfo << "Maximal amount of resources mined." << endl;
	setWork(WorkRefine);
 }
}

void MobileUnit::advanceRefine() 
{
 kdDebug() << k_funcinfo << endl;
 if (!d->mHarvesterProperties) {
	setWork(WorkNone);
	return;
 }
 if (resourcesMined() == 0) {
	kdDebug() << k_funcinfo << "refining done" << endl;
	if (resourcesX() != -1 && resourcesY() != -1) {
		setWork(WorkMine);
		mineAt(QPoint(resourcesX(), resourcesY()));
	} else {
		setWork(WorkNone);
	}
	return;
 }
 if (!refinery()) {
	// TODO: pick closest refinery
	QPtrList<Unit> list = owner()->allUnits();
	QPtrListIterator<Unit> it(list);
	const UnitProperties* prop = unitProperties();
	Facility* ref = 0;
	while (it.current() && !ref) {
		const UnitProperties* unitProp = it.current()->unitProperties();
		if (!it.current()->isFacility()) {
			++it;
			continue;
		}
		if (prop->canMineMinerals() && unitProp->canRefineMinerals()) {
			ref = (Facility*)it.current();
		} else if (prop->canMineOil() && unitProp->canRefineOil()) {
			ref = (Facility*)it.current();
		}
		++it;
	}
	if (!ref) {
		kdDebug() << k_funcinfo << "no suitable refinery found" << endl;
		setWork(WorkNone);
	} else {
		kdDebug() << k_funcinfo << "refinery: " << ref->id() << endl;
		refineAt(ref);
	}
	return;
 } else {
	if (isNextTo(refinery())) {
		const int step = (resourcesMined() >= 10) ? 10 : resourcesMined();
		d->mHarvesterProperties->mResourcesMined = resourcesMined() - step;
		if (unitProperties()->canMineMinerals()) {
			owner()->setMinerals(owner()->minerals() + step);
			owner()->statistics()->increaseRefinedMinerals(step);
		} else if (unitProperties()->canMineOil()) {
			owner()->setOil(owner()->oil() + step);
			owner()->statistics()->increaseRefinedOil(step);
		}
	} else {
	}
 }
}

unsigned int MobileUnit::resourcesMined() const
{
 return d->mHarvesterProperties ? d->mHarvesterProperties->mResourcesMined : 0;
}

int MobileUnit::resourcesX() const
{
 return d->mHarvesterProperties ? d->mHarvesterProperties->mResourcesX : 0;
}

int MobileUnit::resourcesY() const
{
 return d->mHarvesterProperties ? d->mHarvesterProperties->mResourcesY : 0;
}

bool MobileUnit::canMine(Cell* cell) const
{
 if (unitProperties()->canMineMinerals() &&
		cell->groundType() == Cell::GroundGrassMineral) {
	return true;
 }
 if (unitProperties()->canMineOil() && 
		cell->groundType() == Cell::GroundGrassOil) {
	return true;
 }
 return false;
}

void MobileUnit::mineAt(const QPoint& pos)
{
 //TODO: don't move if unit cannot mine more minerals/oil or no minerals/oil at all
 kdDebug() << k_funcinfo << endl;
 moveTo(pos);
 setWork(WorkMine);
 setAdvanceWork(WorkMove);
 d->mHarvesterProperties->mResourcesX = pos.x();
 d->mHarvesterProperties->mResourcesY = pos.y();
}

void MobileUnit::refineAt(Facility* refinery)
{
 if (!refinery) {
	kdError() << k_funcinfo << "NULL refinery" << endl;
	return;
 }
 if (!refinery->unitProperties()->canRefineMinerals() &&
		!refinery->unitProperties()->canRefineOil()) {
	kdError() << k_funcinfo << refinery->id() << " not a refinery" << endl;
 }
 kdDebug() << k_funcinfo << endl;
 setRefinery(refinery);
 setWork(WorkRefine);
 // move...
 kdDebug() << k_funcinfo << "move to refinery " << refinery->id() << endl;
 if (!moveTo(refinery->x(), refinery->y(), 1)) {
	kdDebug() << k_funcinfo << "Cannot find way to refinery" << endl;
	setWork(WorkNone);
 } else {
	setAdvanceWork(WorkMove);
 }
}

Facility* MobileUnit::refinery() const
{
 if (!target() || !target()->isFacility()) {
	return 0;
 }
 Facility* fac = (Facility*)target();
 const UnitProperties* prop = unitProperties();
 const UnitProperties* facProp = fac->unitProperties();
 if (prop->canMineMinerals() && facProp->canRefineMinerals()) {
	return fac;
 } else if (prop->canMineOil() && facProp->canRefineOil()) {
	return fac;
 }
 return 0;
}

void MobileUnit::setRefinery(Facility* refinery)
{
 if (!refinery) {
	return;
 }
 const UnitProperties* prop = unitProperties();
 const UnitProperties* facProp = refinery->unitProperties();
 if (prop->canMineMinerals() && facProp->canRefineMinerals()) {
	setTarget(refinery);
 } else if (prop->canMineOil() && facProp->canRefineOil()) {
	setTarget(refinery);
 }
}

QRect MobileUnit::boundingRect() const
{
// FIXME: workaround for pathfinding which does not yet support units with size
// > BO_TILE_SIZE
// we simply return a boundingrect which has size BO_TILE_SIZE
 if (width() < BO_TILE_SIZE || height() < BO_TILE_SIZE) {
	kdWarning() << "width or height  < BO_TILE_SIZE - not supported!!" << endl;
	return BosonSprite::boundingRect();
 }
 return QRect((int)x(), (int)y(), BO_TILE_SIZE, BO_TILE_SIZE);
}

void MobileUnit::clearWaypoints()
{
 Unit::clearWaypoints();
 d->mTargetCellMarked = false;
}

void MobileUnit::waypointDone()
{
 Unit::waypointDone();
 d->mTargetCellMarked = false;
}

bool MobileUnit::load(QDataStream& stream)
{
 if(!Unit::load(stream)) {
	kdError() << "Unit not loaded properly" << endl;
	return false;
 }

 return true;
}

bool MobileUnit::save(QDataStream& stream)
{
 if(!Unit::save(stream)) {
	kdError() << "Unit not loaded properly" << endl;
	return false;
 }

 return true;
}

/////////////////////////////////////////////////
// Facility
/////////////////////////////////////////////////

class Facility::FacilityPrivate
{
public:
	FacilityPrivate()
	{
		mProductionPlugin = 0;
	}

	KGameProperty<unsigned int> mConstructionState; // state of *this* unit
	ProductionPlugin* mProductionPlugin;
	RepairPlugin* mRepairPlugin;
};

Facility::Facility(const UnitProperties* prop, Player* owner, BosonCanvas* canvas) : Unit(prop, owner, canvas)
{
 d = new FacilityPrivate;
 d->mConstructionState.registerData(IdFix_ConstructionState, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Construction State");
 d->mConstructionState.setLocal(0);

 if (prop->canProduce()) {
	d->mProductionPlugin = new ProductionPlugin(this);
 }
 if (unitProperties()->weaponDamage() < 0) {
	d->mRepairPlugin = new RepairPlugin(this);
 }
 setWork(WorkConstructed);
}

Facility::~Facility()
{
 delete d;
}

unsigned int Facility::constructionSteps() const
{
 return unitProperties()->constructionSteps();
}

void Facility::advanceConstruction()
{
 if (isDestroyed()) {
	kdError() << k_funcinfo << "unit is already destroyed" << endl;
	return;
 }
 if (d->mConstructionState < constructionSteps() - 1) {
	d->mConstructionState = d->mConstructionState + 1;
	setFrame(d->mConstructionState);
	kdDebug() << k_funcinfo << "frame: " << frame() << " cout=" << frameCount()<< endl;
 } else {
	setWork(WorkNone);
	owner()->facilityCompleted(this);
 }
}

ProductionPlugin* Facility::productionPlugin() const
{
 if (!isConstructionComplete()) {
	return 0;
 }
 return d->mProductionPlugin;
}

RepairPlugin* Facility::repairPlugin() const
{
 if (!isConstructionComplete()) {
	return 0;
 }
 return d->mRepairPlugin;
}

bool Facility::isConstructionComplete() const
{
 if (work() == WorkConstructed) {
	return false;
 }
 if (d->mConstructionState < constructionSteps() - 1) {
	return false;
 }
 return true;
}

double Facility::constructionProgress() const
{
 unsigned int constructionTime = constructionSteps() - 1;
 double percentage = (double)(d->mConstructionState * 100) / (double)constructionTime;
 return percentage;
}

void Facility::setTarget(Unit* u)
{
 if (u && !isConstructionComplete()) {
	kdWarning() << "not yet constructed completely" << endl;
	return;
 }
 Unit::setTarget(u);
}

void Facility::moveTo(float x, float y, int range)
{
 if (!isConstructionComplete()) {
	kdWarning() << "not yet constructed completely" << endl;
	return;
 }
 Unit::moveTo(x, y, range);
}

void Facility::setConstructionStep(unsigned int step)
{
 if (isDestroyed()) {
	kdError() << k_funcinfo << "unit is already destroyed" << endl;
	return;
 }
 if (step >= constructionSteps()) {
	step = PIXMAP_FIX_DESTROYED - 1;
 }
 setFrame(step);
 d->mConstructionState = step;
 if (step == constructionSteps() - 1) {
	setWork(WorkNone);
	owner()->facilityCompleted(this);
 }
}

unsigned int Facility::currentConstructionStep() const
{
 return d->mConstructionState;
}

