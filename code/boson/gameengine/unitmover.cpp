/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2006 Rivo Laks (rivolaks@hot.ee)

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

#include "unitmover.h"

#include "bosoncanvas.h"
#include "unitproperties.h"
#include "bosonpath.h"
#include "bodebug.h"
#include "bosonprofiling.h"
#include "boitemlist.h"
#include "cell.h"
#include "unitorder.h"

#include <qdom.h>


// If defined, units will search and fire at enemy units only when they are at
//  waypoints. This may make unit movement better for big groups, because units
//  shouldn't occupy multiple cells while firing at enemies.
//#define CHECK_ENEMIES_ONLY_AT_WAYPOINT


QValueVector<BoVector2Fixed> UnitMover::mCellIntersectionTable[11][11];


UnitMover::UnitMover(Unit* u)
	: mMaxSpeed(u->unitProperties(), "Speed", "MaxValue"),
	mMaxAccelerationSpeed(u->unitProperties(), "AccelerationSpeed", "MaxValue"),
	mMaxDecelerationSpeed(u->unitProperties(), "DecelerationSpeed", "MaxValue")
{
 mUnit = u;

 unit()->setMaxSpeed(maxSpeed());
 unit()->setAccelerationSpeed(maxAccelerationSpeed());
 unit()->setDecelerationSpeed(maxDecelerationSpeed());
}

UnitMover::~UnitMover()
{
}

bool UnitMover::init()
{
 unit()->setAdvanceWork(UnitBase::WorkIdle);
 return true;
}

bofixed UnitMover::maxSpeed() const
{
 return mMaxSpeed.value(unit()->upgradesCollection());
}

bofixed UnitMover::maxAccelerationSpeed() const
{
 return mMaxAccelerationSpeed.value(unit()->upgradesCollection());
}

bofixed UnitMover::maxDecelerationSpeed() const
{
 return mMaxDecelerationSpeed.value(unit()->upgradesCollection());
}

bool UnitMover::saveAsXML(QDomElement& root) const
{
 root.setAttribute("Speed", unit()->speed());
 return true;
}

bool UnitMover::loadFromXML(const QDomElement& root)
{
 bool ok = false;

 bofixed speed = 0.0f;
 if (root.hasAttribute("Speed")) {
	speed = root.attribute("Speed").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "Invalid value for Speed attribute" << endl;
		return false;
	}
	unit()->setSpeed(speed);
 }

 return true;
}

void UnitMover::initStatic()
{
 initCellIntersectionTable();
}

void UnitMover::initCellIntersectionTable()
{
 PROFILE_METHOD
 // Go over all the cells in 11x11 area and store all the cells that the unit
 //  would be on if it would move from (0; 0) to a certain point in straight
 //  line
 // This code is entirely taken from TA Spring project
 for(int y = 0; y < 11; y++) {
	for(int x = 0; x < 11; x++) {
		BoVector2Fixed start(0.5, 0.5);
		BoVector2Fixed to(x - 5 + 0.5, y - 5 + 0.5);

		bofixed dx = to.x() - start.x();
		bofixed dy = to.y() - start.y();
		bofixed xp = start.x();
		bofixed yp = start.y();
		bofixed xn, yn;

		if (floor(start.x()) == floor(to.x())) {
			// x coordinates are same
			if (dy > 0) {
				for (int a = 1; a <= floor(to.y()); a++) {
					mCellIntersectionTable[x][y].push_back(BoVector2Fixed(0, a));
				}
			} else {
				for (int a = -1; a >= floor(to.y()); a--) {
					mCellIntersectionTable[x][y].push_back(BoVector2Fixed(0, a));
				}
			}
		} else if (floor(start.y()) == floor(to.y())) {
			// y coordinates are same
			if (dx > 0) {
				for (int a = 1; a <= floor(to.x()); a++) {
					mCellIntersectionTable[x][y].push_back(BoVector2Fixed(a, 0));
				}
			} else {
				for (int a = -1; a >= floor(to.x()); a--) {
					mCellIntersectionTable[x][y].push_back(BoVector2Fixed(a, 0));
				}
			}
		} else {
			// Tough case: both coordinates differ
			bool keepgoing = true;
			while (keepgoing) {
				if (dx > 0) {
					xn = (floor(xp) + 1 - xp) / dx;
				} else {
					xn = (floor(xp) - xp) / dx;
				}
				if (dy > 0) {
					yn = (floor(yp) + 1 - yp) / dy;
				} else {
					yn = (floor(yp) - yp) / dy;
				}

				if (xn < yn) {
					xp += (xn + 0.0001) * dx;
					yp += (xn + 0.0001) * dy;
				} else {
					xp += (yn + 0.0001) * dx;
					yp += (yn + 0.0001) * dy;
				}
				keepgoing = (QABS(xp - start.x()) < QABS(to.x() - start.x())) &&
						(QABS(yp - start.y()) < QABS(to.y() - start.y()));

				mCellIntersectionTable[x][y].push_back(BoVector2Fixed(floor(xp), floor(yp)));
			}
			mCellIntersectionTable[x][y].pop_back();
		}
	}
 }
}

void UnitMover::advanceMove(unsigned int advanceCallsCount)
{
 advanceMoveInternal(advanceCallsCount);
 advanceMoveCheck();
}

void UnitMover::advanceFollow(unsigned int advanceCallsCount)
{
 if (advanceCallsCount % 5 != 0) {
	return;
 }
 BosonProfiler profiler("advanceFollow");

 UnitFollowOrder* followorder = (UnitFollowOrder*)unit()->currentOrder();
 Unit* target = followorder->target();
 if (!target) {
	boWarning() << k_funcinfo << "cannot follow NULL unit" << endl;
	unit()->currentSuborderDone(false);
	return;
 }
 if (target->isDestroyed()) {
	boDebug(401) << k_funcinfo << "Unit is destroyed!" << endl;
	unit()->currentSuborderDone(true);
	return;
 }

 if (QMAX(QABS(unit()->x() - target->x()), QABS(unit()->y() - target->y())) > followorder->distance()+1) {
	// We're too far from the followed unit
	// AB: warning - this does a lookup on all items and therefore is slow!
	// --> but we need it as a simple test on the pointer causes trouble if
	// that pointer is already deleted. any nice solutions?
	if (!canvas()->allItems()->contains(target)) {
		boDebug(401) << k_funcinfo << "Unit seems to be destroyed!" << endl;
		unit()->currentSuborderDone(true);
		return;
	}
	boDebug(401) << k_funcinfo << "unit (" << target->id() << ") not in range - moving..." << endl;
	if (!unit()->addCurrentSuborder(new UnitMoveToUnitOrder(target, followorder->distance(), false))) {
		unit()->currentSuborderDone(false);
	}
	return;
 }
 // Do nothing (unit is in range)
}


void UnitMover::advanceMoveCheck()
{
 PROFILE_METHOD;
 // Check if top-left point of the unit will be on canvas after moving
 if (!canvas()->onCanvas(unit()->boundingRectAdvanced().topLeft())) {
	BoVector2Fixed point = unit()->boundingRectAdvanced().topLeft();
	QString problem;
	if (!canvas()->onCanvas(BoVector2Fixed(0, point.y()))) {
		problem = QString("top==%1").arg(point.y());
	} else if (!canvas()->onCanvas(BoVector2Fixed(point.x(), 0))) {
		problem = QString("left==%1").arg(point.x());
	} else {
		boError(401) << k_funcinfo
				<< "internal error: (0," << point.y() <<
				") and (" << point.x()
				<< ",0) are on canvas, but (" << point.x() <<
				"," << point.y() << ") isn't !"
				<< endl;
		problem = "internal";
	}
	boError(401) << k_funcinfo << "unit " << id()
			<< " not on canvas (topLeftAdvanced): (" << point.x()
			<< ";" << point.y() << ")" << " problem was: "
			<< problem << endl;
	boError(401) << k_funcinfo << "leaving unit a current topleft pos: ("
			<< unit()->boundingRect().topLeft().x() << ";"
			<< unit()->boundingRect().topLeft().y() << ")" << endl;
	// TODO: is this the right thing to do?
	unit()->clearOrders();
	return;
 }
 // Check if bottom-right point of the unit will be on canvas after moving
 if (!canvas()->onCanvas(unit()->boundingRectAdvanced().bottomRight())) {
	BoVector2Fixed point = unit()->boundingRectAdvanced().bottomRight();
	QString problem;
	if (!canvas()->onCanvas(BoVector2Fixed(0, point.y()))) {
		problem = QString("bottom==%1").arg(point.y());
	} else if (!canvas()->onCanvas(BoVector2Fixed(point.x(), 0))) {
		problem = QString("right==%1").arg(point.x());
	} else {
		boError(401) << k_funcinfo
				<< "internal error: (0," << point.y() <<
				") and (" << point.x()
				<< ",0) are on canvas, but (" << point.x() <<
				"," << point.y() << ") isn't !"
				<< endl;
	}
	boError(401) << k_funcinfo << "unit " << id()
			<< " not on canvas (bottomRightAdvanced): ("
			<< point.x() << ";" << point.y() << ")"
			<< "  current rightEdge: " << unit()->rightEdge()
			<< " ; current bottomEdge:" << unit()->bottomEdge()
			<< " ; xVelocity: " << unit()->xVelocity()
			<< " ; (int)xVelocity: " << (int)unit()->xVelocity()
			<< " ; yVelocity: " << unit()->yVelocity()
			<< " ; (int)yVelocity: " << (int)unit()->yVelocity()
			<< " problem was: "
			<< problem << endl;
	boError(401) << k_funcinfo << "leaving unit a current bottomright pos: ("
			<< unit()->boundingRect().bottomRight().x() << ";"
			<< unit()->boundingRect().bottomRight().y() << ")" << endl;
	// TODO: is this the right thing to do?
	unit()->clearOrders();
	return;
 }
}

bool UnitMover::turnTo()
{
 bofixed xspeed = unit()->xVelocity();
 bofixed yspeed = unit()->yVelocity();
 // Set correct rotation
 // Try to find rotation fast first
 if ((xspeed == 0) && (yspeed < 0)) { // North
	return unit()->turnTo(0);
 } else if ((xspeed > 0) && (yspeed == 0)) { // East
	return unit()->turnTo(90);
 } else if ((xspeed == 0) && (yspeed > 0)) { // South
	return unit()->turnTo(180);
 } else if ((xspeed < 0) && (yspeed == 0)) { // West
	return unit()->turnTo(270);
 } else if (QABS(xspeed) == QABS(yspeed)) {
	if ((xspeed > 0) && (yspeed < 0)) { // NE
		return unit()->turnTo(45);
	} else if ((xspeed > 0) && (yspeed > 0)) { // SE
		return unit()->turnTo(135);
	} else if ((xspeed < 0) && (yspeed > 0)) { // SW
		return unit()->turnTo(225);
	} else if ((xspeed < 0) && (yspeed < 0)) { // NW
		return unit()->turnTo(315);
	} else if(xspeed == 0 && yspeed == 0) {
//		boDebug() << k_funcinfo << "xspeed == 0 and yspeed == 0" << endl;
	}
 } else {
	// Slow way - calculate direction
	return unit()->turnTo(Bo3dTools::rotationToPoint(xspeed, yspeed));
 }
}

bool UnitMover::attackEnemyUnitsInRangeWhileMoving()
{
 PROFILE_METHOD
 if (!pathInfo()->moveAttacking) {
	return false;
 }

 UnitMoveOrderData* orderdata = (UnitMoveOrderData*)unit()->currentOrderData();
 orderdata->target = unit()->attackEnemyUnitsInRange(orderdata->target);
 if (orderdata->target) {
	boDebug(401) << k_funcinfo << "unit " << id() << ": Enemy units found in range, attacking" << endl;
	if (unit()->isFlying()) {
		// Don't stop moving
		return true;
	}
	unit()->setVelocity(0.0, 0.0, 0.0);  // To prevent moving
	unit()->setSpeed(0);
	unit()->setMovingStatus(UnitBase::Engaging);
	return true;
 }
 return false;
}

void UnitMover::stopMoving(bool success)
{
// boDebug() << k_funcinfo << endl;
 unit()->clearPathPoints();

 // Release highlevel path here once we cache them

 if (!unit()->isFlying()) {
	unit()->setMovingStatus(UnitBase::Standing);
	unit()->setVelocity(0.0, 0.0, 0.0);

	if (pathInfo() && pathInfo()->slowDownAtDest) {
		unit()->setSpeed(0);
	}
 }
 UnitOrder* order = unit()->currentOrder();
 if (order && (order->type() == UnitOrder::Move || order->type() == UnitOrder::MoveToUnit)) {
	unit()->currentSuborderDone(success);
 }
}



void UnitMover::addUpgrade(const UpgradeProperties* upgrade)
{
 changeUpgrades(upgrade);
}

void UnitMover::removeUpgrade(const UpgradeProperties* upgrade)
{
 changeUpgrades(upgrade);
}

void UnitMover::changeUpgrades(const UpgradeProperties* upgrade)
{
 // AB: these are special cases: they are stored and handled in BosonItem and
 // therefore we can not use some kind of "speedFactor" as we do with e.g.
 // health

 if (unit()->maxSpeed() != maxSpeed()) {
	unit()->setMaxSpeed(maxSpeed());
 }

 // AB: accelerationSpeed/decelerationSpeed always use maximum values
 if (unit()->accelerationSpeed() != maxAccelerationSpeed()) {
	unit()->setAccelerationSpeed(maxAccelerationSpeed());
 }
 if (unit()->decelerationSpeed() != maxDecelerationSpeed()) {
	unit()->setDecelerationSpeed(maxDecelerationSpeed());
 }
}



/*****  UnitMoverLand  *****/

UnitMoverLand::UnitMoverLand(Unit* u) : UnitMover(u)
{
 mNextWaypointIntersections = 0;
 mLastCellX = -1;
 mLastCellY = -1;
 mNextCellX = -1;
 mNextCellY = -1;
 mNextWaypointIntersectionsXOffset = 0;
 mNextWaypointIntersectionsYOffset = 0;
}

UnitMoverLand::~UnitMoverLand()
{
}

bool UnitMoverLand::init()
{
 if (unit()->unitProperties()->isAircraft()) {
	boError() << k_funcinfo << "Unit " << unit()->id() << " is an aircraft!" << endl;
	return false;
 }

 if (!UnitMover::init()) {
	return false;
 }

 return true;
}

void UnitMoverLand::advanceMoveInternal(unsigned int advanceCallsCount)
{
 BosonProfiler profiler("advanceMoveInternal");
 //boDebug(401) << k_funcinfo << endl;

 if (pathInfo()->waiting) {
	// If path is blocked and we're waiting, then there's no point in
	//  recalculating velocity and other stuff every advance call
	// TODO: check for enemies
	return;
 }

 // FIXME: is this really necessary? Such units shouldn't have their work set
 //  to WorkMove in the first place...
 if (unit()->maxSpeed() == 0) {
	// If unit's max speed is 0, it cannot move
	boError(401) << k_funcinfo << "unit " << id() << ": maxspeed == 0" << endl;
	stopMoving(false);
	return;
 }

 // Check if path is already found or not
 if (unit()->movingStatus() == UnitBase::MustSearch) {
	// Path is not yet searched
	// If we're moving with attacking, first check for any enemies in the range.
	if (attackEnemyUnitsInRangeWhileMoving()) {
		return;
	}
	// If there aren't any enemies, find new path
	if (!newPath()) {
		// No path was found
		stopMoving(false);
		return;
	}
 }
 if (unit()->currentOrder()->type() == UnitOrder::MoveToUnit) {
	const BoVector2Fixed& targetpos = ((UnitMoveToUnitOrder*)unit()->currentOrder())->target()->center();
	if (((UnitMoveToUnitOrderData*)unit()->currentOrderData())->lastTargetPos.dotProduct(targetpos) > 5*5) {
		// Update path
		if (!newPath()) {
			// No path was found
			stopMoving(false);
			return;
		}
	}
 }

 // Make sure we have pathpoints
 if (unit()->pathPointCount() == 0) {
	if (pathInfo()->result == BosonPath::OutOfRange) {
		// New pathfinding query has to made, because last returned path was
		//  only partial.
		if (!newPath()) {
			// Probably no path could be found or we're already at destination point
			stopMoving(false);  // TODO: is false correct here?
			return;
		}
	} else {
		// TODO: rotate a bit randomly
		stopMoving(true);
		return;
	}
 }

 boDebug(401) << k_funcinfo << "unit " << id() << endl;
#warning TODO!!!
#if 0
 if (advanceWork() != work()) {
	if (work() == UnitBase::WorkAttack) {
		// Unit is attacking. ATM it's moving to target.
		// no need to move to the position of the unit...
		// just check if unit is in range now.
		if (!unit()->target()) {
			boError() << k_funcinfo << "unit " << id() << " is in WorkAttack, but has NULL target!" << endl;
			unit()->stopAttacking();
			unit()->setMovingStatus(UnitBase::Standing);
			return;
		}
		int range;
		if (unit()->target()->isFlying()) {
			range = unit()->maxAirWeaponRange();
		} else {
			range = unit()->maxLandWeaponRange();
		}
		if (unit()->inRange(range, unit()->target())) {
			boDebug(401) << k_funcinfo << "unit " << id() << ": target is in range now" << endl;
			unit()->setMovingStatus(UnitBase::Standing);
			unit()->stopMoving();
			return;
		}
		// TODO: make sure that target() hasn't moved!
		// if it has moved also adjust pathpoints
	}
 } else
#endif
 if (pathInfo()->moveAttacking) {
	// Attack any enemy units in range
	// Don't check for enemies every time (if we don't have a target) because it
	//  slows things down
	if (((UnitMoveOrderData*)unit()->currentOrderData())->target || (advanceCallsCount % 10 == 0)) {
		if (attackEnemyUnitsInRangeWhileMoving()) {
			return;
		}
	}
 }

 // x and y are center of the unit here
 bofixed x = unit()->centerX();
 bofixed y = unit()->centerY();


 //boDebug(401) << k_funcinfo << "unit " << id() << ": pos: (" << x << "; "<< y << ")" << endl;
 // If we're close to destination, decelerate, otherwise  accelerate
 // TODO: we should also slow down when turning at pathpoint.
 // TODO: support range != 0
 bofixed oldspeed = unit()->speed();
 unit()->accelerate();

 // Go speed() distance towards the next pathpoint. If distance to it is less
 //  than speed(), we go towards the one after this as well
 bofixed xspeed = 0;
 bofixed yspeed = 0;
 bofixed dist = unit()->speed();
 BoVector2Fixed pp;

 // We move through the pathpoints, until we've passed dist distance
 while (dist > 0) {
	// Check if we have any pathpoints left
	if (unit()->pathPointCount() == 0) {
		if (pathInfo()->result == BosonPath::OutOfRange) {
			// New pathfinding query has to made, because last returned path was
			//  only partial.
			if (!newPath()) {
				// Probably no path could be found
				// Unit will stop moving during the next advance call (because we want
				//  to move as much as we already have in this loop)
				break;
			}
		} else {
			// TODO: rotate a bit randomly
			break;
		}
	}

	// Take next pathpoint
	pp = unit()->currentPathPoint();

	// Pathpoint skipping
	int currentCellX = (int)(x + xspeed);
	int currentCellY = (int)(y + yspeed);
	// We will check if we can skip some pathpoint if we're not on the same cell
	//  as the current pathpoint. Also we remember where we were when we skipped
	//  pathpoints last time in order not to do it every frame
	if ((currentCellX != mLastCellX) || (currentCellY != mLastCellY)) {
		if (mLastCellX == -1) {
			// New path has been searched.
			currentPathPointChanged((int)(x + xspeed), (int)(y + yspeed));
			// Modify lastCellX to not execute this again until next cell is reached
			//  or new path is searched.
			mLastCellX = -2;
		}
		if (((int)pp.x() != currentCellX) || ((int)pp.y() != currentCellY)) {
			selectNextPathPoint(currentCellX, currentCellY);
			mLastCellX = currentCellX;
			mLastCellY = currentCellY;
			pp = unit()->currentPathPoint();
		}
	}

	//boDebug(401) << k_funcinfo << "unit " << id() << ": Current pp: (" << pp.x() << "; "<< pp.y() << ")" << endl;

	// Make sure it's possible to go to the pathpoint
	if (!canGoToCurrentPathPoint(x + xspeed, y + yspeed)) {
		// Gotta find another path
		if (!newPath()) {
			// Probably no path could be found
			// Unit will stop moving during the next advance call (because we want
			//  to move as much as we already have in this loop)
			break;
		}
		currentPathPointChanged((int)(x + xspeed), (int)(y + yspeed));
		selectNextPathPoint((int)(x + xspeed), (int)(y + yspeed));
		mLastCellX = (int)(x + xspeed);
		mLastCellY = (int)(y + yspeed);
	}

	// Move towards it
	dist -= moveTowardsPoint(pp, x + xspeed, y + yspeed, dist, xspeed, yspeed);

	// Check if we reached this pathpoint
	if ((x + xspeed == pp.x()) && (y + yspeed == pp.y())) {
		// Unit has reached pathpoint
		boDebug(401) << k_funcinfo << "unit " << id() << ": unit is at pathpoint" << endl;
		unit()->pathPointDone();
		currentPathPointChanged((int)(x + xspeed), (int)(y + yspeed));
		// Check for enemies
		if (attackEnemyUnitsInRangeWhileMoving()) {
			break;
		}
	}
 }

 // If the unit didn't move, we can just return now (this is valid)
 if ((xspeed == 0) && (yspeed == 0)) {
	return;
 }

 //boDebug(401) << k_funcinfo << "unit " << id() << ": Setting velo to: (" << xspeed << "; "<< yspeed << ")" << endl;
 unit()->setVelocity(xspeed, yspeed, 0.0);
 unit()->setMovingStatus(UnitBase::Moving);

 //avoidance();

 bool turning = turnTo();

 // If we just started moving and now want to turn, then set velocity back to 0
 if (turning && (oldspeed == 0)) {
	unit()->setVelocity(0, 0, 0);
 }
}

// AB: WARNING crushing is currently disabled!
//
// see #warning below
void UnitMoverLand::advanceMoveCheck()
{
 PROFILE_METHOD;
 //boDebug(401) << k_funcinfo << endl;

 if (!unit()->currentOrder() || (unit()->currentOrder()->type() != UnitOrder::Move && unit()->currentOrder()->type() != UnitOrder::MoveToUnit)) {
	// We might be e.g. turning
	// TODO: before turning, we have possibly already moved, so we should still
	//  check our new pos here
	// TODO: also consider e.g. MoveToUnit order
	return;
 }

 // Take special action if path is (was) blocked and we're waiting
 if (pathInfo()->waiting) {
	// We try to move every 5 advance calls
	if (pathInfo()->waiting % 5 == 0) {
		// Try to move again
		if (unit()->cellOccupied(mNextCellX, mNextCellY)) {
			// Obstacle is still there. Continue waiting
			if (pathInfo()->waiting >= 600) {
				// Enough of waiting (30 secs). Give up.
				stopMoving(false);
				return;
			} else if (pathInfo()->waiting % (20 + QMIN(pathInfo()->pathrecalced * 20, 80)) == 0) {
				// First wait 20 adv. calls (1 sec) before recalculating path, then 40
				//  calls, then 60 etc, but never more than 100 calls.
				boDebug(401) << k_funcinfo << "unit " << id() << ": Recalcing path, waiting: " << pathInfo()->waiting <<
						"; pathrecalced: " << pathInfo()->pathrecalced << endl;
				newPath();
				pathInfo()->pathrecalced++;
				pathInfo()->waiting = 0;
				// New path will be used next advance call
				return;
			}
			pathInfo()->waiting++;
			return;
		} else {
			// Our path is free again. We can continue moving next advance call, but
			//  not immediately, because advanceMoveInternal() didn't calculate
			//  velocities
			pathInfo()->waiting = 0;
			unit()->setMovingStatus(UnitBase::Waiting);  // TODO: is this ok here? maybe set to Moving instead?
			return;
		}
	} else {
		// Don't check if we can move every advance call. Just return for now
		pathInfo()->waiting++;
		return;
	}
 }

 // Make sure unit is one canvas
 UnitMover::advanceMoveCheck();

 if (unit()->pathPointCount() == 0) {
	// This is allowed and means that unit will stop after this this advance call
	return;
 }

 if (unit()->xVelocity() == 0 && unit()->yVelocity() == 0) {
	// Probably unit stopped to attack other units
	return;
 }

 //boDebug(401) << k_funcinfo << "unit " << id() << endl;

 // Crushing & waiting
#warning What the hell does crushing do in advanceMove_Check_() ??
#define CRUSHING 0
#if CRUSHING
 bool wait = false;
 QValueList<Unit*> collisions;
 collisions = canvas()->collisionsInBox(BoVector3Fixed(unit()->x() + unit()->xVelocity(), unit()->y() + unit()->yVelocity(), unit()->z()),
		BoVector3Fixed(unit()->x() + unit()->xVelocity() + unit()->width(), unit()->y() + unit()->yVelocity() + unit()->height(), unit()->z() + unit()->depth()), unit());
 if (!collisions.isEmpty()) {
	for (QValueList<Unit*>::Iterator it = collisions.begin(); it != collisions.end(); it++) {
		Unit* u = *it;
		if (unit() == u) {
			continue;
		} else if (u->isFlying() != unit()->isFlying()) {
			continue;
		} else if (!u->bosonCollidesWith(unit())) {
			continue;
		}
		// Make sure we actually can crush this unit
		if (u->maxHealth() > unitProperties()->crushDamage()) {
			boDebug(401) << k_funcinfo << id() << ": Colliding with uncrushable unit " << u->id() << endl;
			// Whoops. Now what?
			wait = true;
			continue;
		}
		// Crush it!
		boDebug(401) << k_funcinfo << id() << ": Crushing " << u->id() << endl;
		u->setHealth(0);
		canvas()->destroyUnit(u);
	}
 }
#endif
 /*if (wait) {
	setVelocity(0, 0, 0);
	setMovingStatus(UnitBase::Waiting);
	setSpeed(0);
 }*/

 // Check if we need to wait
 // Find the next cell we'll be on
 if (mNextWaypointIntersections->count() == 0) {
	boWarning() << k_funcinfo << unit()->id() << ": mNextWaypointIntersections is empty" << endl;
	stopMoving(false);
	return;
 }

 int currentx = (int)(unit()->center().x() + unit()->xVelocity());
 int currenty = (int)(unit()->center().y() + unit()->yVelocity());
 mNextCellX = -1;
 BO_CHECK_NULL_RET(mNextWaypointIntersections);
 for (unsigned int i = 0; i < mNextWaypointIntersections->count(); i++) {
	if (currentx == mNextWaypointIntersections->at(i).x() && currenty == mNextWaypointIntersections->at(i).y()) {
		if (i+1 >= mNextWaypointIntersections->count()) {
			// This is the last cell, i.e. we're at pathpoint cell
			mNextCellX = mNextWaypointIntersections->last().x();
			mNextCellY = mNextWaypointIntersections->last().y();
		} else {
			// We're in the middle of the way to the next pathpoint. Use the next cell
			mNextCellX = mNextWaypointIntersections->at(i+1).x();
			mNextCellY = mNextWaypointIntersections->at(i+1).y();
		}
		break;
	}
 }
 if (mNextCellX == -1) {
	// We're at the beginning of the way to the next pathpoint
	mNextCellX = mNextWaypointIntersections->first().x();
	mNextCellY = mNextWaypointIntersections->first().y();
 }
 mNextCellX += mNextWaypointIntersectionsXOffset;
 mNextCellY += mNextWaypointIntersectionsYOffset;

 // Make sure the next cell is free of any obstacles
 if (unit()->cellOccupied(mNextCellX, mNextCellY)) {
	// Gotta wait
	unit()->setVelocity(0, 0, 0);
	unit()->setMovingStatus(UnitBase::Waiting);
	unit()->setSpeed(0);
	pathInfo()->waiting++;
	return;
 }


 pathInfo()->waiting = 0;
 pathInfo()->pathrecalced = 0;

 //boDebug(401) << k_funcinfo << "unit " << id() << ": done" << endl;
}

void UnitMoverLand::currentPathPointChanged(int unitx, int unity)
{
 if (unitProperties()->isAircraft()) {
	return;
 }
 int xindex = (int)unit()->currentPathPoint().x() - unitx + 5;
 int yindex = (int)unit()->currentPathPoint().y() - unity + 5;
 mNextWaypointIntersectionsXOffset = unitx;
 mNextWaypointIntersectionsYOffset = unity;
 mNextWaypointIntersections = &mCellIntersectionTable[xindex][yindex];
}

int UnitMoverLand::selectNextPathPoint(int xpos, int ypos)
{
 if (unitProperties()->isAircraft()) {
	return 0;
 }
 //return 0;
 // TODO: better name?
 // Find next pathpoint where we can go in straight line
 int skipped = 0;
 bool cango = true;
 while (cango && (skipped < 6) && (unit()->pathPointCount() > 1)) {
	int newCellX = (int)unit()->pathPointList()[1].x();
	int newCellY = (int)unit()->pathPointList()[1].y();
	if ((QABS(newCellX - xpos) >= 6) || (QABS(newCellY - ypos) >= 6)) {
		// We're too far
		break;
	}
	int xindex = newCellX - xpos + 5;
	int yindex = newCellY - ypos + 5;
	for (unsigned int i = 0; i < mCellIntersectionTable[xindex][yindex].count(); i++) {
		// Check if this cell is accessable
		// TODO: check terrain
		if (unit()->cellOccupied(xpos + mCellIntersectionTable[xindex][yindex][i].x(),
				ypos + mCellIntersectionTable[xindex][yindex][i].y(), true)) {
			cango = false;
			break;
		}
	}
	if (cango) {
		// skip previous pathpoint (go directly to this one)
		unit()->pathPointDone();
		currentPathPointChanged(xpos, ypos);
		skipped++;
	}
 }

 return skipped;
}

void UnitMoverLand::avoidance()
{
 BoVector2Fixed velocity(unit()->xVelocity(), unit()->yVelocity());
 velocity.normalize();
 BoVector3Fixed toRight3 = BoVector3Fixed(velocity.x(), -velocity.y(), 0).crossProduct(BoVector3Fixed(0, 0, 1));
 BoVector2Fixed toRight(toRight3.x(), -toRight3.y());
 bofixed avoidstrength = 0;
 // Find all units which are near us
 BoRect2Fixed rect(unit()->x() - unit()->speed() * 40 - 1,
		unit()->y() - unit()->speed() * 40 - 1,
		unit()->x() + unit()->width() + unit()->speed() * 40 + 1,
		unit()->y() + unit()->height() + unit()->speed() * 40 + 1);
 BoItemList* items = canvas()->collisions()->collisionsAtCells(rect, unit(), false);
 // Go through the units
 for (BoItemList::ConstIterator it = items->begin(); it != items->end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		// TODO: check for e.g. mines
		continue;
	}
	Unit* u = (Unit*)*it;
	if (u == unit()) {
		continue;
	} else if (u->isFlying()) {
		// We don't care about flying units
		continue;
	} else if (u->maxHealth() <= unitProperties()->crushDamage()) {
		// We can just crush this one
		continue;
	}
	if (velocity.dotProduct(u->center() - unit()->center()) <= 0) {
		// We only care about units in front of us
		continue;
	}

//	BoVector2Fixed toUnit((u->center() + BoVector2Fixed(u->xVelocity(), u->yVelocity()) * 20) - center());
	bofixed unitSizesSum = (unit()->width() + u->width()) / 2;
	// Vector and distance to the other unit ATM
	BoVector2Fixed toUnit(u->center() - unit()->center());
	bofixed distToUnit = toUnit.length();
	// Vector and distance to the other unit in one second
	BoVector2Fixed toUnitSoon((u->center() + BoVector2Fixed(u->xVelocity(), u->yVelocity()) * 20) -
			(unit()->center() + BoVector2Fixed(unit()->xVelocity(), unit()->yVelocity()) * 20));
	bofixed distToUnitSoon = toUnitSoon.length();
	if ((distToUnitSoon < distToUnit) && (distToUnitSoon < unitSizesSum + 1.5)) {
		// We're getting too close to the other unit
		// Distance between the units on the axis perpendicular to this unit's rotation
		// TODO: maybe use toUnitSoon instead?
		bofixed sideDistance = toUnit.dotProduct(toRight);
		if (QABS(sideDistance) < unitSizesSum) {
			// We'll crash with this unit if we don't act
			// Calculate how much we'll try to avoid this unit
			bofixed factor = 1;  // Positive = turn right
			if (sideDistance > 0) {
				// The other unit is right of us. Turn left
				factor *= -1;
			}
			// The more int front of us the unit is, the bigger the factor
			factor *= unitSizesSum * 1.5 - QABS(sideDistance);
			// The farther from us the unit is, the smaller the factor
			factor *= 2 - (distToUnitSoon - unitSizesSum);
			//factor /= distToUnit;

			avoidstrength += factor;
		}
	}
 }
 if (avoidstrength != 0) {
	// Calculate new velocity
	BoVector2Fixed avoidance(toRight * avoidstrength);
	boDebug(401) << k_funcinfo << id() << ": Avoidstrength: " << avoidstrength <<
			"; avoidance vector: (" << avoidance.x() << "; " << avoidance.y() << ")" << endl;
	velocity += avoidance;
	velocity.normalize();
	boDebug(401) << k_funcinfo << id() << ": Changing velo from (" << unit()->xVelocity() << "; " << unit()->yVelocity() <<
			") to (" << velocity.x() * unit()->speed() << "; " << velocity.y() * unit()->speed() << ")" << endl;
	unit()->setVelocity(velocity.x() * unit()->speed(), velocity.y() * unit()->speed(), unit()->zVelocity());
 }
}

bool UnitMoverLand::canGoToCurrentPathPoint(int xpos, int ypos)
{
 if (unitProperties()->isAircraft()) {
	return true;
 }
 int ppx = (int)unit()->currentPathPoint().x();
 int ppy = (int)unit()->currentPathPoint().y();
 if ((QABS(ppx - xpos) >= 6) || (QABS(ppy - ypos) >= 6)) {
	// We're too far
	return false;
 }
 int xindex = ppx - xpos + 5;
 int yindex = ppy - ypos + 5;
 for (unsigned int i = 0; i < UnitMover::mCellIntersectionTable[xindex][yindex].count(); i++) {
	// Check if this cell is accessable
	// TODO: check terrain
	const BoItemList* items = canvas()->cell(xpos + UnitMover::mCellIntersectionTable[xindex][yindex][i].x(),
			ypos + UnitMover::mCellIntersectionTable[xindex][yindex][i].y())->items();
	for (BoItemList::ConstIterator it = items->begin(); it != items->end(); ++it) {
		if (RTTI::isUnit((*it)->rtti())) {
			Unit* u = (Unit*)*it;
			if (unit() == u) {
				continue;
			} else if (u->isFlying()) {
				continue;
			}
			if (u->movingStatus() == UnitBase::Moving) {
				// Hopefully it will move away
				continue;
			} else if(u->maxHealth() <= pathInfo()->movedata->crushDamage && u->owner() != unit()->owner()) {
				// We can just crush it
				continue;
			}
			return false;
		}
	}
 }
 return true;
}

bofixed UnitMoverLand::moveTowardsPoint(const BoVector2Fixed& p, bofixed x, bofixed y, bofixed maxdist, bofixed &xspeed, bofixed &yspeed)
{
 PROFILE_METHOD;
 // Passed distance
 bofixed dist = 0.0f;
 // Calculate difference between point and our current position
 bofixed xdiff, ydiff;
 xdiff = p.x() - x;
 ydiff = p.y() - y;
 bofixed difflen = sqrt(xdiff*xdiff + ydiff*ydiff);

 if (difflen <= maxdist) {
	xspeed += xdiff;
	yspeed += ydiff;
	return difflen;
 } else {
	bofixed factor = maxdist / difflen;
	xspeed += xdiff * factor;
	yspeed += ydiff * factor;
	return maxdist;
 }
}

bool UnitMoverLand::newPath()
{
 BosonProfiler profiler("newPath");
 boDebug(401) << k_funcinfo << "unit " << id() << endl;

 // Update our start position
 pathInfo()->start.set(unit()->centerX(), unit()->centerY());

 if (unit()->currentOrder()->type() == UnitOrder::MoveToUnit) {
	// Update destination
	const BoVector2Fixed& targetpos = ((UnitMoveToUnitOrder*)unit()->currentOrder())->target()->center();
	pathInfo()->dest = targetpos;
	((UnitMoveToUnitOrderData*)unit()->currentOrderData())->lastTargetPos = targetpos;
 }

 // Clear previous pathpoints
 unit()->clearPathPoints();

 // Don't try to find the path if we're already at the destination point
 if(QMAX(QABS(pathInfo()->dest.x() - pathInfo()->start.x()), QABS(pathInfo()->dest.x() - pathInfo()->start.x()))
		< 1.0f) {
	return false;
 }

 // Find path
 canvas()->pathFinder()->findPath(pathInfo());

 if (pathInfo()->result == BosonPath::NoPath || pathInfo()->llpath.count() == 0) {
	// Stop moving
	return false;
 }
 // Copy low-level path to pathpoints' list
 for (int unsigned i = 0; i < pathInfo()->llpath.count(); i++) {
	unit()->addPathPoint(pathInfo()->llpath[i]);
 }

 // Reset last cell
 mLastCellX = -1;
 mLastCellY = -1;
 mNextWaypointIntersections = &mCellIntersectionTable[5][5];

 return true;
}



/*****  UnitMoverFlying  *****/

UnitMoverFlying::UnitMoverFlying(Unit* u) : UnitMover(u)
{
 mRoll = 0;
}

UnitMoverFlying::~UnitMoverFlying()
{
}

bool UnitMoverFlying::init()
{
 if (!unit()->unitProperties()->isAircraft()) {
	boError() << k_funcinfo << "Unit " << unit()->id() << " is not an aircraft!" << endl;
	return false;
 }

 if (!UnitMover::init()) {
	return false;
 }

 unit()->setSpeed(maxSpeed() * 0.75);
 unit()->move(unit()->x(), unit()->y(), unit()->unitProperties()->preferredAltitude());
 return true;
}

bool UnitMoverFlying::saveAsXML(QDomElement& root) const
{
 if (!UnitMover::saveAsXML(root)) {
	return false;
 }

 root.setAttribute("Roll", mRoll);

 return true;
}

bool UnitMoverFlying::loadFromXML(const QDomElement& root)
{
 if (!UnitMover::loadFromXML(root)) {
	return false;
 }

 bool ok = false;
 if (root.hasAttribute("Roll")) {
	mRoll = root.attribute("Roll").toFloat(&ok);
	if (!ok) {
		boWarning() << k_funcinfo << "Invalid value for Roll attribute" << endl;
	} else {
		unit()->setYRotation(mRoll);
	}
 }

#warning TODO: mNextWaypointIntersections
 return true;
}

void UnitMoverFlying::advanceIdle()
{
 flyInCircle();
}

void UnitMoverFlying::flyInCircle()
{
 bofixed speedfactor = unit()->speed() / unit()->maxSpeed();
 if (speedfactor < 0.7) {
	unit()->accelerate();
 } else if (speedfactor > 0.8) {
	unit()->decelerate();
 }

 // Flying units need to keep flying
 // TODO: choose which way to turn
 bofixed newrot = unit()->rotation() + unitProperties()->rotationSpeed();
 if (newrot > 360) {
	newrot -= 360;
 }
 BoVector3Fixed velo(0, 0, 0);
 velo.setX(cos(Bo3dTools::deg2rad(newrot - 90)) * unit()->speed());
 velo.setY(sin(Bo3dTools::deg2rad(newrot - 90)) * unit()->speed());

 // Don't go off the map
 if (unit()->x() < 0.5 || unit()->y() < 0.5 ||
		unit()->x() > bofixed(canvas()->mapWidth()) - unit()->width() - 0.5 || unit()->y() > bofixed(canvas()->mapHeight()) - unit()->height() - 0.5) {
	bofixed x = unit()->x();
	bofixed y = unit()->y();
	x = QMAX(x, bofixed(1));
	y = QMAX(y, bofixed(1));
	x = QMIN(x, bofixed(canvas()->mapWidth()) - unit()->width() - 1);
	y = QMIN(y, bofixed(canvas()->mapHeight()) - unit()->height() - 1);
	unit()->move(x, y, unit()->z());
 }

 bofixed groundz = canvas()->heightAtPoint(unit()->centerX() + velo.x(), unit()->centerY() + velo.y());
 if (unit()->z() + velo.z() < groundz + unitProperties()->preferredAltitude() - 1) {
	velo.setZ(groundz - unit()->z() + unitProperties()->preferredAltitude() - 1);
 } else if (unit()->z() + velo.z() > groundz + unitProperties()->preferredAltitude() + 1) {
	velo.setZ(groundz - unit()->z() + unitProperties()->preferredAltitude() + 1);
 }

 unit()->setRotation(newrot);
 unit()->setVelocity(velo.x(), velo.y(), velo.z());
 //d->currentVelocity = velo;
 //d->currentVelocity.normalize();

 // Calculate roll
 bofixed wantedroll = 25;
 const bofixed maxrollincrease = 2;
 bofixed delta = wantedroll - mRoll;
 if (delta > 0) {
	mRoll += QMIN(delta, maxrollincrease);
 } else if (delta < 0) {
	mRoll -= QMIN(QABS(delta), maxrollincrease);
 }

 unit()->setYRotation(mRoll);
}

void UnitMoverFlying::advanceMoveInternal(unsigned int advanceCallsCount)
{
 BosonProfiler profiler("advanceMoveFlying");
 //boDebug(401) << k_funcinfo << endl;

 if (unit()->maxSpeed() == 0) {
	// If unit's max speed is 0, it cannot move
	boError(401) << k_funcinfo << "unit " << id() << ": maxspeed == 0" << endl;
	stopMoving(false);
	unit()->setMovingStatus(UnitBase::Standing);
	return;
 }

 boDebug(401) << k_funcinfo << "unit " << id() << endl;


 // Calculate velocity
 BoVector3Fixed targetpos(unit()->pathInfo()->dest.x(), unit()->pathInfo()->dest.y(), 0);
 if (unit()->currentOrder()->type() == UnitOrder::MoveToUnit) {
	const BoVector2Fixed& targetunitpos = ((UnitMoveToUnitOrder*)unit()->currentOrder())->target()->center();
	targetpos.set(targetunitpos.x(), targetunitpos.y(), 0);
 }
 targetpos.setZ(canvas()->heightAtPoint(targetpos.x(), targetpos.y()) + 3);
#warning TODO!!!
#if 0
 if (advanceWork() != work()) {
	if (work() == UnitBase::WorkAttack) {
		// Unit is attacking. ATM it's moving to target.
		if (!unit()->target()) {
			boError() << k_funcinfo << "unit " << id() << " is in WorkAttack, but has NULL target!" << endl;
			unit()->stopAttacking();
			advanceIdle();
			return;
		}
		int range;
		if (unit()->target()->isFlying()) {
			range = unit()->maxAirWeaponRange();
		} else {
			range = unit()->maxLandWeaponRange();
		}
		if (unit()->inRange(range, unit()->target())) {
			boDebug(401) << k_funcinfo << "unit " << id() << ": target is in range now" << endl;
			unit()->stopMoving();
			advanceIdle();
			return;
		}

		targetpos = BoVector3Fixed(unit()->target()->centerX(), unit()->target()->centerY(), unit()->target()->centerZ());
		if (!unit()->target()->isFlying()) {
			targetpos.setZ(targetpos.z() + 3);
		}
	}
 } else
#endif
if (pathInfo()->moveAttacking) {
	// Attack any enemy units in range
	// Don't check for enemies every time (if we don't have a target) because it
	//  slows things down
	if (((UnitMoveOrderData*)unit()->currentOrderData())->target || (advanceCallsCount % 10 == 0)) {
		attackEnemyUnitsInRangeWhileMoving();
	}
 }


 // x and y are center of the unit here
 bofixed x = unit()->centerX();
 bofixed y = unit()->centerY();
 bofixed z = unit()->centerZ();

 unit()->accelerate();


 // Calculate totarget vector
 BoVector3Fixed totarget(targetpos.x() - x, targetpos.y() - y, targetpos.z() - z);
 bofixed totargetlen = totarget.length();
 // We need check this here to avoid division by 0 later
 if (totargetlen <= pathInfo()->range || totargetlen <= unit()->speed()) {
	stopMoving(true);
	return;
 }
 // Normalize totarget. totarget vector now shows direction to target
 totarget.scale(1.0f / totargetlen);

 // Check if the target is to our left or right
 bofixed wantedrotation = Bo3dTools::rotationToPoint(totarget.x(), totarget.y());
 bofixed rotationdelta = unit()->rotation() - wantedrotation;  // How many degrees to turn
 bofixed newrotation = unit()->rotation();
 bool turncw = false;  // Direction of turning, CW or CCW

 // Find out direction of turning and huw much is left to turn
 if (rotationdelta < 0) {
	rotationdelta = QABS(rotationdelta);
	turncw = true;
 }
 if (rotationdelta > 180) {
	rotationdelta = 180 - (rotationdelta - 180);
	turncw = !turncw;
 }

 const bofixed maxturningspeed = unitProperties()->rotationSpeed();
 if (rotationdelta <= maxturningspeed) {
	newrotation = wantedrotation;
 } else {
	if (turncw) {
		newrotation += maxturningspeed;
	} else {
		newrotation -= maxturningspeed;
	}
 }
 // Check for overflows
 if (newrotation < 0) {
	newrotation += 360;
 } else if (newrotation >= 360) {
	newrotation -= 360;
 }

 /*bofixed crossproduct = d->currentVelocity.x() * totarget.y() - d->currentVelocity.y() * totarget.x();
 bool turnright = true;
 if (crossproduct < 0) {
	// Turn left
	turnright = false;
 } else {
	// Turn right
 }*/

 //bofixed rotationdelta = rotation

 // Difference between current direction and direction to target
 /*BoVector3Fixed diff = totarget - d->currentVelocity;
 bofixed difflen = diff.length();
 const bofixed turnspeed = 0.08;
 if (difflen != 0) {
	// We're not flying towards the target atm
	// Calculate new velocity vector
	if (turnspeed < difflen) {
		diff.scale(turnspeed / difflen);
	}
	// Alter velocity direction so that it's more towards the target
	d->currentVelocity += diff;
	d->currentVelocity.normalize();
 }

 // This is final velocity
 BoVector3Fixed velo(d->currentVelocity * speed());*/

 // Calculate velocity
 BoVector3Fixed velo;
 velo.setX(cos(Bo3dTools::deg2rad(newrotation - 90)) * unit()->speed());
 velo.setY(sin(Bo3dTools::deg2rad(newrotation - 90)) * unit()->speed());

 bofixed groundz = canvas()->heightAtPoint(x + velo.x(), y + velo.y());
 if (z + velo.z() < groundz + unitProperties()->preferredAltitude() - 1) {
	velo.setZ(groundz - z + unitProperties()->preferredAltitude() - 1);
 } else if (z + velo.z() > groundz + unitProperties()->preferredAltitude() + 1) {
	velo.setZ(groundz - z + unitProperties()->preferredAltitude() + 1);
 }

 // Calculate roll
 /*bofixed wantedroll = QMIN(difflen / turnspeed, bofixed(1)) * 45;
 if (totarget.crossProduct(velo).z() < 0) {
	// Turning left
	wantedroll = -wantedroll;
 } else {
	// Turning right
 }*/
 bofixed wantedroll = QMIN(rotationdelta / maxturningspeed, bofixed(1)) * 45;
 if (!turncw) {
	// Turning left
	wantedroll = -wantedroll;
 }

 const bofixed maxrollincrease = 2;
 bofixed delta = wantedroll - mRoll;
 if (delta > 0) {
	mRoll += QMIN(delta, maxrollincrease);
 } else if (delta < 0) {
	mRoll -= QMIN(QABS(delta), maxrollincrease);
 }

 unit()->setVelocity(velo.x(), velo.y(), velo.z());
 unit()->setRotation(newrotation);
 //setXRotation(Bo3dTools::rotationToPoint(sqrt(1 - velo.z() * velo.z()), velo.z()) - 90);
 unit()->setYRotation(mRoll);

 unit()->setMovingStatus(UnitBase::Moving);
}
