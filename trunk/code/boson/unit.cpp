/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "cell.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "bosonpath.h"
#include "selectbox.h"

#include <kgame/kgamepropertylist.h>

#include "defines.h"

class Unit::UnitPrivate
{
public:
	UnitPrivate()
	{
		mSelectBox = 0;
	}
	KGamePropertyInt mDirection;
	KGameProperty<unsigned int> mReloadState;

	KGamePropertyList<QPoint> mWaypoints;

	SelectBox* mSelectBox;

	Unit* mTarget;
};

Unit::Unit(const UnitProperties* prop, Player* owner, QCanvas* canvas) 
		: UnitBase(prop), QCanvasSprite(owner->pixmapArray(prop->typeId()), canvas)
{
 d = new UnitPrivate;
 setOwner(owner);

//hmm.. would probably make more sense in Unit instead of Unit
 d->mDirection.registerData(IdDirection, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Direction");
 d->mWaypoints.registerData(IdWaypoints, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Waypoints");

 d->mReloadState.registerData(IdReloadState, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "ReloadState");

 d->mDirection.setLocal(0); // not yet used
 d->mReloadState.setLocal(0);
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
 d->mSelectBox = new SelectBox(x(), y(), width(), height(), z(), canvas());
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
	setAnimated(true);
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
 double oldX = x();
 double oldY = y();
 QCanvasSprite::moveBy(moveX, moveY);
/*
// FIXME 
 QCanvasItemList list = collisions(true);
 if (!list.isEmpty()) {
	QCanvasItemList::Iterator it;
	for (it = list.begin(); it != list.end(); ++it) {
		if (RTTI::isUnit((*it)->rtti())) {
			if (!((Unit*)*it)->isDestroyed()) {
				kdWarning() << "collided with " << list.count() 
						<< " units" << endl;
				kdWarning() << "moving back and stop moving" << endl;
				QCanvasSprite::moveBy(-moveX, -moveY);
				stopMoving();
				return; // No need to move select boxes by zero
			}
		}
	}
 }*/


 if (d->mSelectBox) {
	d->mSelectBox->moveBy(moveX, moveY);
 }
 ((BosonCanvas*)canvas())->unitMoved(this, oldX, oldY);
}

void Unit::advance(int phase)
{ // time critical function !!!
// kdDebug() << k_funcinfo << " id=" << id() << endl;
 if (isDestroyed()) {
	return;
 }
 if (phase == 0) {
	// collision detection should be done here as far as i understand
	// do not move the item/unit here!

	// perhaps test if there is a enemy unit in weapon range (x() - range()
	// -> x() + width() + range() and so on)
	// but we would need a setAnimated(true) for all units then :-(
	if (work() == WorkMove) {
		advanceMove(); // move one step
	} else if (work() == WorkAttack) {
		attackUnit(target());
	} else if (work() == WorkProduce) {
		advanceProduction();
	} else if (work() == WorkMine) {
		// TODO
	} else if (work() == WorkConstructed) {
		advanceConstruction();
	} else if (work() == WorkNone) {
//		kdDebug() << k_funcinfo << ": work==WorkNone" << endl;
		QCanvasItemList list = enemyUnitsInRange();
		if (!list.isEmpty()) {
			shootAt((Unit*)list[0]);
		}
	} else {
		kdError() << "work: " << work() << endl;
	}
	if (d->mReloadState > 0) {
		d->mReloadState = d->mReloadState - 1;
	}
	QCanvasSprite::advance(phase); // does nothing for phase == 0
 } else { // phase == 1
	QCanvasSprite::advance(phase); // actually move
 }
}

void Unit::turnTo(int direction)
{
 if (direction < 0 || direction >= PIXMAP_PER_MOBILE - 1) {
	kdError() << "direction " << direction << " not supported" << endl;
	return;
 }
 setFrame(direction);
}

void Unit::addWaypoint(const QPoint& pos)
{
 d->mWaypoints.append(pos);
// kdDebug() << "added " <<pos.x() << " " << pos.y() << endl;
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
 moveTo(pos.x(), pos.y());
 if (waypointCount() > 0) {
	setWork(WorkMove);
	setAnimated(true);
 }
}

void Unit::moveTo(int x, int y)
{
 stopMoving();
 
 if(! boCanvas()->cell(x / BO_TILE_SIZE,
			y / BO_TILE_SIZE)->canGo(unitProperties())) {
	// No pathfinding if goal not reachable
	return;
 }

 // Find path to target
 QValueList<QPoint> path = BosonPath::findPath(this, x, y);
 for (int unsigned i = 0; i < path.count(); i++) {
	 addWaypoint(path[i]);
 }
 setWork(WorkMove);
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
 kdDebug() << "stopMoving" << endl;
 clearWaypoints();
 setWork(WorkNone);
 setXVelocity(0);
 setYVelocity(0);
// setAnimated(false); // do not call advance() anymore - is more efficient

 // in theory all units move on all clients the same - i.e. a playerInput() is
 // transmitted to all clients and all variables should always have the same
 // value.
 // but at least as of today (01/11/03) this is not completely working by any
 // reason. It can happen that a unit moves on client A but on client B there is
 // a collision with another unit - so it doesn't move. There are several
 // solutions possible
 // -> we should send a "IdStopMoving" and stop moving on all
 // clients at once - but the time lag is not nice. so we currently do the
 // following:
 // When one client has a collision for a unit and calls "stopMoving()" it sends
 // out the current coordinates of the unit. These coordinates are applied on
 // the other clients (they now also stop the unit). So all clients are synced
 // again.
 // the problem: if client A has a collision or arrival on destination, as well
 // as client B (this is the usual case!!) the *both* send out IdStopMoving with
 // the same coordinates... not nice...

 // I think I have found the reason for the not-synced units:
 // the problem seems to appear if several units are moved at once. Probably
 // they are added to the QCanvas in a different order and therefore are
 // advanced in a different order. This might result in an already moved (and
 // therefore no more a case for collisions) unit on client A but not yet moved
 // unit on client B (and therefore collisions return a different list, ...)
 // a solution will be: reorder the canvaslists!

 // so now i hace fixed the above. The problem seems to be solved - the
 // sendStopMoving below still resides here as of testing and debugging. Remove
 // it as soon as it's sure that we don't need it anymore. Will save a lot of
 // network traffik!
 
 // UPDATE (01/12/22): This is now obsolete. we don't need it anymore.
}

void Unit::stopAttacking()
{
 stopMoving(); // FIXME not really intuitive... nevertheless its currenlty useful.
 setTarget(0);
}

bool Unit::save(QDataStream& stream)
{
 if (!Unit::save(stream)) {
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
	if (waypointCount() == 0) {
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
 r.setBottom(r.bottom() + range());
 r.setRight(r.right() + range());
 r.setLeft((r.left() > (int)range()) ? r.left() - range() : 0);

 QCanvasItemList items = canvas()->collisions(r);
 items.remove((QCanvasItem*)this);
 QCanvasItemList inRange;
 QCanvasItemList::Iterator it = items.begin();
 for (; it != items.end(); ++it) {
	if (RTTI::isUnit((*it)->rtti())) {
		// TODO: remove the items from inRange which are not actually in range (hint:
		// pythagoras)
		inRange.append(*it);
	}
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

QString Unit::soundShoot() const
{
 return speciesTheme()->themePath() + "sounds/shoot.wav";
}

QValueList<Unit*> Unit::unitCollisions(bool exact) const
{
 QValueList<Unit*> units;
 QCanvasItemList collisionList = collisions(exact);
 if (collisionList.isEmpty()) {
	return units;
 }
 
 bool flying = isFlying();
 QCanvasItemList::Iterator it;
 for (it = collisionList.begin(); it != collisionList.end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	Unit* unit = ((Unit*)*it);
	if (unit->isDestroyed()) {
		continue;
	}
	if (flying == unit->isFlying()) {
		units.append(unit);
	}
 }
 return units;
}

unsigned int Unit::reloadState() const
{
 return d->mReloadState;
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
};

MobileUnit::MobileUnit(const UnitProperties* prop, Player* owner, QCanvas* canvas) : Unit(prop, owner, canvas)
{
 d = new MobileUnitPrivate;
 d->mSpeed.registerData(IdSpeed, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Speed");
}

MobileUnit::~MobileUnit()
{
 delete d;
}

void MobileUnit::advanceMove()
{
 if(waypointCount() == 0) {
	// shouldn't happen - work() should be WorkNone here
	kdWarning() << k_funcinfo << ": no waypoints?!" << endl;
	stopMoving(); // should have been called before already
	return;
 }

 if (speed() == 0) {
	stopMoving();
	kdDebug() << "speed == 0" << endl;
	return;
 }


 QPoint wp = currentWaypoint(); // where we go to
 QRect position = boundingRect(); // where we currently are.
 int x = position.center().x();
 int y = position.center().y();
 double xspeed = 0;
 double yspeed = 0;

 // First check if we're at waypoint
 if((x == wp.x()) && (y == wp.y()))
 {
	kdDebug() << k_funcinfo << ": unit is at waypoint" << endl;
 	waypointDone();
	if(waypointCount() == 0)
	{
		kdDebug() << k_funcinfo << ": no more waypoints. Stopping moving" << endl;
		// What to do?
		stopMoving();
		return;
	}
	wp = currentWaypoint();
 }
 
 // Try to go to same x and y coordinates as waypoint's coordinates
 // First x coordinate
 // Slow down if there is less than speed() pixels to go
 if(abs(wp.x() - x) < speed()) {
	xspeed = wp.x() - x;
 } else {
	xspeed = speed();
	if(wp.x() < x) {
		xspeed = -xspeed;
	}
 }
 // Same with y coordinate
 if(abs(wp.y() - y) < (double)speed()) {
	yspeed = wp.y() - y;
 } else {
	yspeed = speed();
	if(wp.y() < y) {
		yspeed = -yspeed;
	}
 }

 // Set correct frame
 if((xspeed == 0) && (yspeed < 0)) { // North
 	turnTo(0);
 } else if((xspeed > 0) && (yspeed < 0)) { // NE
	turnTo(1);
 } else if((xspeed > 0) && (yspeed == 0)) { // East
	turnTo(2);
 } else if((xspeed > 0) && (yspeed > 0)) { // SE
	turnTo(3);
 } else if((xspeed == 0) && (yspeed > 0)) { // South
	turnTo(4);
 } else if((xspeed < 0) && (yspeed > 0)) { // SW
	turnTo(5);
 } else if((xspeed < 0) && (yspeed == 0)) { // West
	turnTo(6);
 } else if((xspeed < 0) && (yspeed < 0)) { // NW
	turnTo(7);
 } else {
	kdDebug() << "xspeed == 0 and yspeed == 0 or error when setting frame" << endl;
 }

 // Set velocity for actual moving
// kdDebug() << k_funcinfo << "setting velocity: x=" << xspeed << ", y=" << yspeed << endl;
 setVelocity(xspeed, yspeed);

 // Check for units on way
 QValueList<Unit*> collisionList = unitCollisions();
 for (int unsigned i = 0; i < collisionList.count(); i++) {
	if (collidesWith(collisionList[i])) {
		kdWarning() << id() << " colliding with unit" << endl;
		// just stop. Do not (yet) search new path
		stopMoving();
	}
 }
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

void MobileUnit::setSpeed(double speed)
{
 d->mSpeed = speed;
}

double MobileUnit::speed() const
{
 return d->mSpeed;
}

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
 setAnimated(true); // construcion animation

 d->mProductions.setEmittingSignal(false); // just to prevent warning in Player::slotUnitPropertyChanged()
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
 if (d->mConstructionState < (constructionSteps() - 1) * unitProperties()->constructionDelay()) {
	d->mConstructionState = d->mConstructionState + 1;
	if (d->mConstructionState % unitProperties()->constructionDelay() == 0) {
		setFrame(d->mConstructionState / unitProperties()->constructionDelay());
	}
 } else {
	setAnimated(false);
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
 if (pos.x() > x() + width() + BUILD_RANGE ||
		pos.y() > y() + height() + BUILD_RANGE) {
	 kdDebug() << "pos.x()=" << pos.x() << ",x+width+BUILD_RANGE=" << x() + width() + BUILD_RANGE << endl;
	 kdDebug() << "pos.y()=" << pos.y() << ",y+height+BUILD_RANGE=" << y() + height() + BUILD_RANGE << endl;
	 return false;
 }
 if (pos.x() < (x() > BUILD_RANGE ? x() - BUILD_RANGE : 0) || 
		pos.y() < (y() > BUILD_RANGE ? y() - BUILD_RANGE : 0)) {
	 kdDebug() << "pos.x()=" << pos.x() << ",x-BUILD_RANGE=" << x() - BUILD_RANGE << endl;
	 kdDebug() << "pos.y()=" << pos.y() << ",y-BUILD_RANGE=" << y() - BUILD_RANGE << endl;
	 return false;
 }
 return true;
}

int Facility::completedProduction() const
{
 if (!hasProduction()) {
	return -1;
 }
 int type = currentProduction();
 if (d->mProductionState < owner()->unitProperties(type)->productionTime()) {
	kdDebug() << "not yet completed: " << type << endl;
	return -1;
 }
 return type;
}

int Facility::currentProduction() const
{
 return d->mProductions.first();
}

void Facility::addProduction(int unitType)
{
 if (!unitProperties()->produceList().contains(unitType)) {
	kdError() << id() << " cannot produce " << unitType << endl;
	return;
 }
 d->mProductions.append(unitType);
 setWork(WorkProduce);
 setAnimated(true);
}

void Facility::removeProduction()
{
 d->mProductions.pop_front();
 d->mProductionState = 0; // start next production (if any)
}

QValueList<int> Facility::productionList() const
{
 return d->mProductions;
}

void Facility::advanceProduction()
{
 int type = currentProduction();
 if (type < 0) { // no production
	setAnimated(false);
	d->mProductionState = 0;
	return;
 }
 // a unit is completed as soon as d->mProductionState == owner()->unitProperties(type)->productionTime()
 unsigned int productionTime = owner()->unitProperties(type)->productionTime();
 if (d->mProductionState <= productionTime) {
	if (d->mProductionState == productionTime) {
		kdDebug() << "unit " << type << " completed :-)" << endl;
		d->mProductionState = d->mProductionState + 1;
	} else {
		d->mProductionState = d->mProductionState + 1;
		double percentage = (double)(d->mProductionState * 100) / (double)productionTime;
		owner()->productionAdvanced(this, percentage);
	}
 }
}

