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

#include <kgame/kgamepropertylist.h>
#include <kgame/kgame.h>

#include "defines.h"

class Unit::UnitPrivate
{
public:
	UnitPrivate()
	{
		mSelectBox = 0;
		mLeader = false;
	}
	KGamePropertyInt mDirection;
	KGameProperty<unsigned int> mReloadState;

	KGamePropertyList<QPoint> mWaypoints;
	KGameProperty<int> mMoveDestX;
	KGameProperty<int> mMoveDestY;

	SelectBox* mSelectBox;

	bool mLeader;
	Unit* mTarget;
};

Unit::Unit(const UnitProperties* prop, Player* owner, QCanvas* canvas) 
		: UnitBase(prop), QCanvasSprite(owner->pixmapArray(prop->typeId()), canvas)
{
 d = new UnitPrivate;
 setOwner(owner);

 d->mDirection.registerData(IdDirection, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Direction");
 d->mWaypoints.registerData(IdWaypoints, dataHandler(), 
		KGamePropertyBase::PolicyClean, "Waypoints");
 d->mMoveDestX.registerData(IdMoveDestX, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "MoveDestX");
 d->mMoveDestY.registerData(IdMoveDestY, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "MoveDestY");

 d->mReloadState.registerData(IdReloadState, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "ReloadState");

 d->mDirection.setLocal(0); // not yet used
 d->mReloadState.setLocal(0);
 setAnimated(true);
 searchpath = false;
}

Unit::~Unit()
{
 d->mWaypoints.setEmittingSignal(false); // just to prevent warning in Player::slotUnitPropertyChanged()
 d->mWaypoints.clear();
 unselect();
 delete d;
}

void Unit::select()
{
 if (isDestroyed()) {
	return; // shall we really return?
 }
 if (d->mSelectBox) {
	// the box was already created
	return;
 }
// put the selection box on the same canvas as the unit and around the unit
 d->mSelectBox = new SelectBox(x(), y(), width(), height(), z(), canvas(), d->mLeader);
 updateSelectBox();
}

void Unit::unselect()
{
 if (d->mSelectBox) {
	delete d->mSelectBox;
	d->mSelectBox = 0;
 }
}

Unit* Unit::target() const
{
 return d->mTarget;
}

void Unit::setTarget(Unit* target)
{
 d->mTarget = target;
 if (!target) {
	return;
 }
 if (damage() == 0) {
	kdWarning() << "Cannot attack: damage() == 0" << endl;
	return;
 }
 if (!target->isDestroyed()) {
	setWork(WorkAttack);
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
	if(d->mLeader)
	{
		boCanvas()->leaderDestroyed(this);
		d->mLeader = false;
	}
 }
}

void Unit::updateSelectBox()
{
 if (d->mSelectBox) {
	unsigned long int maxHealth = unitProperties()->health();
	double div = (double)health() / maxHealth;
	d->mSelectBox->update(div);
	d->mSelectBox->show();
 }
}

void Unit::moveBy(double moveX, double moveY)
{
// time critical function

 if (!moveX && !moveY) {
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
 double oldX = x();
 double oldY = y();
 QCanvasSprite::moveBy(moveX, moveY);
 if (d->mSelectBox) {
	d->mSelectBox->moveBy(moveX, moveY);
 }
 boCanvas()->unitMoved(this, oldX, oldY);
}

void Unit::advance(int phase)
{ // time critical function !!!
// kdDebug() << k_funcinfo << " id=" << id() << endl;
 if (isDestroyed()) {
	return;
 }
 if (phase == 0) {
	if (d->mReloadState > 0) {
		d->mReloadState = d->mReloadState - 1;
	}
 } else { // phase == 1
	// QCanvasSprite::advance() just moves for phase == 1 ; let's do it
	// here, too. Collision detection is done is phase == 0 - in all of the
	// other advance*() functions.
	moveBy(xVelocity(), yVelocity());
 }
}


void Unit::advanceMine()
{
 if (boCanvas()->cellAt(this)->groundType() == Cell::GroundGrassMineral) {
	if (!unitProperties()->canMineMinerals()) {
		kdWarning() << k_funcinfo << "cannot mine minerals" << endl;
		return;
	}
	kdDebug() << "mining... minerals" << endl;
 } else if (boCanvas()->cellAt(this)->groundType() == Cell::GroundGrassOil) {
	if (!unitProperties()->canMineOil()) {
		kdWarning() << k_funcinfo << "cannot mine oil" << endl;
		return;
	}
	kdDebug() << "mining... oil" << endl;
 } else {
	setWork(WorkNone);
 }
}

void Unit::advanceNone()
{
// this is called when the unit has nothing specific to do. Usually we just want
// to fire at every enemy in range.
// kdDebug() << k_funcinfo << ": work==WorkNone" << endl;
 QCanvasItemList list = enemyUnitsInRange();
 if (!list.isEmpty()) {
	shootAt((Unit*)list[0]);
 }
}

void Unit::advanceAttack()
{
 attackUnit(target());
}

void Unit::addWaypoint(const QPoint& pos)
{
 d->mWaypoints.append(pos);
}

void Unit::waypointDone()
{
// waypoints are added with PolicyClean, but removed with PolicyLocal. That is
// kind of ugly but this way we can ensure that only one client has to calculate
// the path but every client uses the path.
// Try to avoid this concept! Do NOT copy it!
 d->mWaypoints.setPolicy(KGamePropertyBase::PolicyLocal);
 d->mWaypoints.remove(d->mWaypoints.at(0));
 d->mWaypoints.setPolicy(KGamePropertyBase::PolicyClean);
 if (waypointCount() == 0) {
	setWork(WorkNone);
 }
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
 if(moveTo(pos.x(), pos.y())) {
	setWork(WorkMove);
 }
 else {
	setWork(WorkNone);
 }
}

bool Unit::moveTo(int x, int y)
{
 stopMoving();

 if(!owner()->isFogged(x / BO_TILE_SIZE, y / BO_TILE_SIZE)) {
	// No pathfinding if goal not reachable or occupied and we can see it
	if(!boCanvas()->cell(x / BO_TILE_SIZE, y / BO_TILE_SIZE)->canGo(unitProperties())) {
		return false;
	}
	if(boCanvas()->cellOccupied(x / BO_TILE_SIZE, y / BO_TILE_SIZE) && work() != WorkAttack) {
		return false;
	}
 }

 d->mMoveDestX = x;
 d->mMoveDestY = y;

 // Do not find new path here!!! Instead, set work to WorkMove and searchpath
 //  to true and then find path later in advanceMove()
 //newPath();
 searchpath = true;
 return true;
}

bool Unit::newPath()
{
 if (owner()->isVirtual()) {
	// only the owner of the unit calculates the path and then transmits it
	// over network. a "virtual" player is one which is duplicated on
	// another client - but the actual player is on another client.
	return false;
 }
 if(!owner()->isFogged(d->mMoveDestX / BO_TILE_SIZE, d->mMoveDestY / BO_TILE_SIZE)) {
	if(! boCanvas()->cell(d->mMoveDestX / BO_TILE_SIZE, d->mMoveDestY / BO_TILE_SIZE)->
			canGo(unitProperties())) {
		return false;
	}
	if(boCanvas()->cellOccupied(d->mMoveDestX / BO_TILE_SIZE, d->mMoveDestY / BO_TILE_SIZE,
			this) && work() != WorkAttack) {
		return false;
	}
 }
 QValueList<QPoint> path = BosonPath::findPath(this, d->mMoveDestX, d->mMoveDestY);
 if(path.count() < 1) {// Is it actually needed?
	return false;
 }
 clearWaypoints(true); // send it over network. the list is cleared just before the addWaypoints() below take effect
 for (int unsigned i = 0; i < path.count(); i++) {
	addWaypoint(path[i]);
 }
 return true;
}

void Unit::clearWaypoints(bool send)
{
// waypoints are added with PolicyClean, but removed with PolicyLocal. That is
// kind of ugly but this way we can ensure that only one client has to calculate
// the path but every client uses the path.
// Try to avoid this concept! Do NOT copy it!
 if (!send) {
	d->mWaypoints.setPolicy(KGamePropertyBase::PolicyLocal);
 }
 d->mWaypoints.clear();
 d->mWaypoints.setPolicy(KGamePropertyBase::PolicyClean);
}

const QPoint& Unit::currentWaypoint() const
{
 return d->mWaypoints[0];
}

void Unit::stopMoving()
{
 kdDebug() << "stopMoving" << endl;
 clearWaypoints();

 // Call this only if we are only moving - stopMoving() is also called e.g. on
 // WorkAttack, when the unit is not yet in range.
 if (isMoving()) {
	setWork(WorkNone);
 }
 setXVelocity(0);
 setYVelocity(0);
 if(d->mLeader) {
	boCanvas()->leaderStopped(this);
 }
}

void Unit::stopAttacking()
{
 stopMoving(); // FIXME not really intuitive... nevertheless its currenlty useful.
 setTarget(0);
 setWork(WorkNone);
}

bool Unit::save(QDataStream& stream)
{
 if (!UnitBase::save(stream)) {
	kdError() << "Unit not saved properly" << endl;
	return false;
 }
 stream << (double)x();
 stream << (double)y();
 stream << (double)z();
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
 double x;
 double y;
 double z;
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
 setFrame(frame);
 return true;
}

bool Unit::inRange(Unit* target) const
{
 // maybe we should use an own algorithm here - can be faster than this generic
 // one
 return unitsInRange().contains(target);
}

void Unit::attackUnit(Unit* target)
{
 if (!target) {
	kdError() << k_funcinfo << ": cannot attack NULL target" << endl;
	return;
 }
 if (target->isDestroyed()) {
	kdDebug() << "Target is destroyed!" << endl;
	stopAttacking();
	return;
 }
 if (!inRange(target)) {
	if (!canvas()->allItems().contains(target)) {
		kdDebug() << "Target seems to be destroyed!" << endl;
		stopAttacking();
		return;
	}
	// TODO: make sure that the attakced unit has not moved!!
	// if it has moved the waypoints should be regenerated (perhaps if the
	// unit moved across one or more cells?)
	if (waypointCount() == 0) { //FIXME perhaps it could happen that the new waypoints have not yet arrived but are underway! moveTo() would clear them...
		moveTo(target->x(), target->y());
	}
	kdDebug() << "unit not in range - moving..." << endl;
	advanceMove();
	return;
 }
 if (waypointCount() > 0) {
	clearWaypoints();
 }
 setXVelocity(0);
 setYVelocity(0);
 shootAt(target);
 if (target->isDestroyed()) {
	stopAttacking();
 }
}

void Unit::shootAt(Unit* target)
{
 if (d->mReloadState != 0) {
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
 kdDebug() << "shoot at unit " << target->id() << endl;
 ((BosonCanvas*)canvas())->shootAtUnit(target, this, damage());
 d->mReloadState = unitProperties()->reload();
}

QCanvasItemList Unit::unitsInRange() const
{
 // TODO: we use a *rect* for the range this is extremely bad.
 // ever heard about pythagoras ;-) ?
 
 QRect r = boundingRect();
 r.setTop((r.top() > (int)range()) ? r.top() - range() : 0);
// qt bug (confirmed). will be fixed in 3.1
#if QT_VERSION >= 310
 r.setBottom(r.bottom() + range());
 r.setRight(r.right() + range());
#else
 r.setBottom(r.bottom() + range() - 1);
 r.setRight(r.right() + range() - 1);
#endif
 r.setLeft((r.left() > (int)range()) ? r.left() - range() : 0);

 QCanvasItemList items = canvas()->collisions(r);
 items.remove((QCanvasItem*)this);
 QCanvasItemList inRange;
 QCanvasItemList::Iterator it = items.begin();
 for (; it != items.end(); ++it) {
	if (! RTTI::isUnit((*it)->rtti()))
		continue;
	if(((Unit*)(*it))->isDestroyed())
		continue;
	// TODO: remove the items from inRange which are not actually in range (hint:
	// pythagoras)
	inRange.append(*it);
 }
 return inRange;
}

QCanvasItemList Unit::enemyUnitsInRange() const
{
 QCanvasItemList units = unitsInRange();
 QCanvasItemList enemy;
 QCanvasItemList::Iterator it = units.begin();
 for (; it != units.end(); ++it) {
	if (((Unit*)(*it))->owner() != owner()) {
		enemy.append(*it);
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
 QCanvasItemList collisionList = collisions(exact);
 if (collisionList.isEmpty()) {
	return units;
 }
 
 QCanvasItemList::Iterator it;
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

unsigned int Unit::reloadState() const
{
 return d->mReloadState;
}

void Unit::setWork(WorkType w)
{
 if (w != work()) {
	boCanvas()->setWorkChanged(this, work());
 }
 UnitBase::setWork(w);
}

void Unit::moveInGroup()
{
 setWork(WorkMoveInGroup);
}

void Unit::setGroupLeader(bool leader)
{
 d->mLeader = leader;
 if (d->mSelectBox) {
	unselect();
	select();
 }
}


/////////////////////////////////////////////////
// MobileUnit
/////////////////////////////////////////////////

class MobileUnit::MobileUnitPrivate
{
public:
	MobileUnitPrivate()
	{
	}

	KGameProperty<double> mSpeed;

	KGameProperty<unsigned int> mMovingFailed;

};

MobileUnit::MobileUnit(const UnitProperties* prop, Player* owner, QCanvas* canvas) : Unit(prop, owner, canvas)
{
 d = new MobileUnitPrivate;
 d->mSpeed.registerData(IdSpeed, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Speed");
 d->mMovingFailed.registerData(IdMovingFailed, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "MovingFailed");
 d->mSpeed.setLocal(0);
 d->mMovingFailed.setLocal(0);
 d->mMovingFailed.setEmittingSignal(false);
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

 if(searchpath)
 {
	if(! newPath())
		stopMoving();
	searchpath = false;
 }

 if(waypointCount() == 0) {
	// waypoints are PolicyClean - so they might need some advanceMove()
	// calls until they are actually here
	kdDebug() << "waypoints have not yet arrived" << endl;
	setXVelocity(0);
	setYVelocity(0);
	return;
 }

 QPoint wp = currentWaypoint(); // where we go to
 // Check if we can actually go to waypoint (maybe it was fogged)
 if((boCanvas()->cellOccupied(wp.x() / BO_TILE_SIZE, wp.y() / BO_TILE_SIZE, this, true) &&
		work() != WorkAttack) || 
		!boCanvas()->cell(wp.x() / BO_TILE_SIZE, wp.y() / BO_TILE_SIZE)->canGo(unitProperties())) {
	kdDebug() << "cannot go to waypoint, finding new path" << endl;
	kdDebug() << "waypoint is at (" << wp.x() << ", " << wp.y() << "), my pos: (" << x() << ", " << y() << ")" << endl;
	setXVelocity(0);
	setYVelocity(0);
	// We have to clear waypoints first to make sure that they aren't used next
	//  advance() call (when new waypoints haven't arrived yet)
	clearWaypoints();
	if(! newPath())
		stopMoving();
	return;
 }

 QRect position = boundingRect(); // where we currently are.
 int x = position.center().x();
 int y = position.center().y();
 double xspeed = 0;
 double yspeed = 0;

 // First check if we're at waypoint
 if((x == wp.x()) && (y == wp.y())) {
	QPoint wp = currentWaypoint(); // where we go to

	kdDebug() << k_funcinfo << ": unit is at waypoint" << endl;
 	waypointDone();
	
	if(waypointCount() == 0) {
		kdDebug() << k_funcinfo << ": no more waypoints. Stopping moving" << endl;
		// What to do?
		stopMoving();
		return;
	}
	
	wp = currentWaypoint();
	// Check if we can actually go to waypoint
	if((boCanvas()->cellOccupied(wp.x() / BO_TILE_SIZE, wp.y() / BO_TILE_SIZE, this, true) &&
			work() != WorkAttack) ||
			!boCanvas()->cell(wp.x() / BO_TILE_SIZE, wp.y() / BO_TILE_SIZE)->canGo(unitProperties())) {
		setXVelocity(0);
		setYVelocity(0);
		kdDebug() << "cannot go to new waypoint, finding new path" << endl;
		// We have to clear waypoints first to make sure that they aren't used next
		//  advance() call (when new waypoints haven't arrived yet)
		clearWaypoints();
		if(! newPath())
			stopMoving();
		return;
	}
	
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
 if(QABS(wp.y() - y) < (double)speed()) {
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

// Dont't test for unit collisions here, because advanceMoveCheck() will be called anyway
/* QCanvasItemList collisionList = collisions(exact);
 if(! collisionList.isEmpty())
 {
	QCanvasItemList::Iterator it;
	for( it = collisionList.begin(); it != collisionList.end(); ++it)
	{
		if(RTTI::isUnit((*it)->rtti()))
		{
			if(! ((Unit)*it)->isDestroyed())
			{
				// Unit on way: find new path
				clearWaypoints();
				// Find our position
				QRect pos = boundingRect();
				int x = pos.center().x();
				int y = pos.center().y();
				// Find path to target
				BosonPath path(this, x / BO_TILE_SIZE, y / BO_TILE_SIZE,
						pos.x() / BO_TILE_SIZE, pos.y() / BO_TILE_SIZE);
				path.findPath();
				for(vector<WayPoint>::iterator i = path.path.begin();
						path != path.path.end(); ++i)
				{
					addWaypoint((*i));
				}
				// Call move method again
				advanceMove();
			}
		}
	}
 }*/
}

void MobileUnit::advanceGroupMove(Unit* leader)
{
 setXVelocity(leader->xVelocity());
 setYVelocity(leader->yVelocity());
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
 QValueList<Unit*> l = unitCollisions(true);
 if (!l.isEmpty()) {
	kdDebug() << k_funcinfo << "collisions" << endl;
//	kdWarning() << k_funcinfo << ": " << id() << " -> " << l.first()->id() 
//		<< " (count=" << l.count() <<")"  << endl;
	// do not move at all. Moving is not stopped completely!
	// work() is still workMove() so we'll continue moving in the next
	// advanceMove() call

	d->mMovingFailed = d->mMovingFailed + 1; // perhaps use PolicyDirty for mMovingFailed. then add a if(isVirtual()) to this
	setXVelocity(0);
	setYVelocity(0);

	const int recalculate = 50; // recalculate when 50 advanceMove() failed
	if (d->mMovingFailed >= recalculate) {
		kdDebug() << "recalculating path" << endl;
		// you must not do anything that changes local variables directly here!
		// all changed of variables with PolicyClean are ok, as they are sent
		// over network and do not take immediate effect.

		if(! newPath())
			stopMoving();
		d->mMovingFailed = 0;
	}
	return;
 }
 d->mMovingFailed = 0;
}

void MobileUnit::setSpeed(double speed)
{
 d->mSpeed = speed;
}

double MobileUnit::speed() const
{
 return d->mSpeed;
}

void MobileUnit::turnTo(Direction direction)
{
/* if (direction < 0 || direction >= PIXMAP_PER_MOBILE - 1) {
	kdError() << "direction " << direction << " not supported" << endl;
	return;
 }*/
 setFrame((int)direction);
}

void MobileUnit::turnTo()
{
 double xspeed = xVelocity();
 double yspeed = yVelocity();
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
 } else {
	kdDebug() << "xspeed == 0 and yspeed == 0 or error when setting frame" << endl;
 }
}

void MobileUnit::leaderMoved(double x, double y)
{
 if(work() == WorkMoveInGroup) {
	setVelocity(x, y);
	turnTo();
	setVelocity(0, 0);
	moveBy(x, y);
 } else {
	kdDebug() << "MobileUnit::leaderMoved() called, but work() != WorkMoveInGroup" << endl;
 }
}


/////////////////////////////////////////////////
// Facility
/////////////////////////////////////////////////

class Facility::FacilityPrivate
{
public:
	FacilityPrivate()
	{
	}

	KGamePropertyInt mConstructionState; // state of *this* unit

	KGamePropertyList<int> mProductions; // what this unit produces currently

	KGameProperty<unsigned int> mProductionState; // state of the unit we are producing
};

Facility::Facility(const UnitProperties* prop, Player* owner, QCanvas* canvas) : Unit(prop, owner, canvas)
{
 d = new FacilityPrivate;
 d->mConstructionState.registerData(IdFix_ConstructionState, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Construction State");
 d->mProductions.registerData(IdFix_Productions, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Productions");
 d->mProductionState.registerData(IdFix_ProductionState, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "ProductionState");
 d->mConstructionState.setLocal(0);
 d->mProductionState.setLocal(0);

 setWork(WorkConstructed);

 d->mProductions.setEmittingSignal(false); // just to prevent warning in Player::slotUnitPropertyChanged()
 d->mProductionState.setEmittingSignal(false); // called quite often - not emitting will increase speed a little bit
}

Facility::~Facility()
{
 delete d;
}

int Facility::constructionSteps()
{
 return FACILITY_CONSTRUCTION_STEPS;
}

void Facility::advanceConstruction()
{
 if (d->mConstructionState < (constructionSteps() - 1) * constructionDelay()) {
	d->mConstructionState = d->mConstructionState + 1;
	if (d->mConstructionState % constructionDelay() == 0) {
		setFrame(d->mConstructionState / constructionDelay());
	}
 } else {
	setWork(WorkNone);
 }
}

bool Facility::hasProduction() const
{
 return (unitProperties()->canProduce() ? !d->mProductions.isEmpty() : false);
}

bool Facility::canPlaceProductionAt(const QPoint& pos) const
{
 if (!hasProduction() || completedProduction() < 0) {
	kdDebug() << k_lineinfo << "no completed construction" << endl;
	return false;
 }
 QValueList<Unit*> list = boCanvas()->unitCollisionsInRange(pos, BUILD_RANGE);

 const UnitProperties* prop = owner()->unitProperties(currentProduction());
 if (!prop) {
	kdError() << k_lineinfo << "NULL unit properties - EVIL BUG!" << endl;
	return false;
 }
 if (prop->isFacility()) {
	// a facility can be placed within BUILD_RANGE of *any* friendly
	// facility on map
	for (unsigned int i = 0; i < list.count(); i++) {
		if (list[i]->isFacility() && list[i]->owner() == owner()) {
			kdDebug() << "Facility in BUILD_RANGE" << endl;
			// TODO: also check whether a unit is already at that position!!
			return true;
		}
	}
 } else {
	// a mobile unit can be placed within BUILD_RANGE of its factory *only*
	for (unsigned int i = 0; i < list.count(); i++) {
		if (list[i] == (Unit*)this) {
			// TODO: also check whether a unit is already at that position!!
			return true;
		}
	}
 }


 return false;
}

int Facility::completedProduction() const
{
 if (!hasProduction()) {
	return -1;
 }
 int type = currentProduction();
 if (type < 0) {
	return -1;
 }
 if (d->mProductionState < owner()->unitProperties(type)->productionTime()) {
	kdDebug() << "not yet completed: " << type << endl;
	return -1;
 }
 return type;
}

int Facility::currentProduction() const
{
 if (d->mProductions.isEmpty()) {
	return -1;
 }
 return d->mProductions.first();
}

void Facility::addProduction(int unitType)
{
 if (!unitProperties()->produceList().contains(unitType)) {
	kdError() << id() << " cannot produce " << unitType << endl;
	return;
 }
 if (!completedConstruction()) {
	kdWarning() << "not yet constructed completely" << endl;
	return;
 }
 bool start = false;
 if (!hasProduction()) {
	start = true;
 }
 d->mProductions.append(unitType);
 if (start) {
	setWork(WorkProduce);
 }
}

void Facility::removeProduction()
{
 d->mProductions.pop_front();
 d->mProductionState = 0; // start next production (if any)
}

void Facility::removeProduction(int unitType)
{
 for (unsigned int i = 0; i < d->mProductions.count(); i++) {
	if (d->mProductions[i] == unitType) {
		kdDebug() << k_funcinfo << "remove " << unitType << endl;
		d->mProductions.remove(d->mProductions.at(i));
		return;
	}
 }
}

QValueList<int> Facility::productionList() const
{
 return d->mProductions;
}

void Facility::advanceProduction()
{
 if (!completedConstruction()) {
	kdWarning() << "not yet constructed completely" << endl;
	return;
 }
 int type = currentProduction();
 if (type < 0) { // no production
	setWork(WorkNone);
	d->mProductionState = 0;
	return;
 }



 // FIXME: this code is broken!
 // it gets executed on *every* client but sendInput() should be used on *one*
 // client only!
 // a unit is completed as soon as d->mProductionState == owner()->unitProperties(type)->productionTime()
 unsigned int productionTime = owner()->unitProperties(type)->productionTime();
 if (d->mProductionState <= productionTime) {
	if (d->mProductionState == productionTime) {
		kdDebug() << "unit " << type << " completed :-)" << endl;
		d->mProductionState = d->mProductionState + 1;
		// Auto-place unit
		// Unit positioning scheme: all tiles starting with tile that is below
		// facility's lower-left tile, are tested counter-clockwise. Unit is placed
		// to first free tile.
		// No auto-placing for facilities
		if(!owner()->unitProperties(type)) {
			kdError() << k_lineinfo << "Unknown type " << type << endl;
			return;
		}
		if(owner()->unitProperties(type)->isFacility()) {
			return;
		}
		int tilex, tiley; // Position of lower-left corner of facility in tiles
		int theight, twidth; // height and width of facility in tiles
		int currentx, currenty; // Position of tile currently tested
		theight = height() / BO_TILE_SIZE;
		twidth = width() / BO_TILE_SIZE;
		tilex = (int)(x() / BO_TILE_SIZE);
		tiley = (int)(y() / BO_TILE_SIZE + theight);
		int tries; // Tiles to try for free space
		int ctry; // Current try
		currentx = tilex - 1;
		currenty = tiley - 1;
		for(int i=1; i <= 3; i++) {
			tries = 2 * i * twidth + 2 * i * theight + 4;
			currenty++;
			for(ctry = 1; ctry <= tries; ctry++) {
				kdDebug() << "    Try " << ctry << " of " << tries << endl;
				if(ctry <= twidth + i) {
					currentx++;
				} else if(ctry <= twidth + i + theight + 2 * i - 1) {
					currenty--;
				} else if(ctry <= twidth + i + 2 * (theight + 2 * i - 1)) {
					currentx--;
				} else if(ctry <= twidth + i + 3 * (theight + 2 * i - 1)) {
					currenty++;
				} else {
					currentx++;
				}

				if(! boCanvas()->cellOccupied(currentx, currenty)) {
					// Free cell - place unit at it
					d->mProductionState = d->mProductionState + 1;
					((Boson*)owner()->game())->buildProducedUnit(this, type, currentx, currenty);
					return;
				}
			}
		}
		kdDebug() << "Cannot find free cell around facility :-(" << endl;
	} else {
		d->mProductionState = d->mProductionState + 1;
	}
 }
}

double Facility::productionProgress() const
{
 unsigned int productionTime = owner()->unitProperties(currentProduction())->productionTime();
 double percentage = (double)(d->mProductionState * 100) / (double)productionTime;
 return percentage;
}

bool Facility::completedConstruction() const
{
 if (work() == WorkConstructed) {
	return false;
 }
 if (d->mConstructionState < (constructionSteps() - 1) * constructionDelay()) {
	return false;
 }
 return true;
}

double Facility::constructionProgress() const
{
 unsigned int constructionTime = (constructionSteps() - 1) * constructionDelay();
 double percentage = (double)(d->mConstructionState * 100) / (double)constructionTime;
 return percentage;
}

void Facility::setTarget(Unit* u)
{
 if (!completedConstruction()) {
	kdWarning() << "not yet constructed completely" << endl;
	return;
 }
 Unit::setTarget(u);
}

void Facility::moveTo(int x, int y)
{
 if (!completedConstruction()) {
	kdWarning() << "not yet constructed completely" << endl;
	return;
 }
 Unit::moveTo(x, y);
}

int Facility::constructionDelay()
{
 return 5;
}
