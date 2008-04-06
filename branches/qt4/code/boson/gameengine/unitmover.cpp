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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include "unitplugins/enterunitplugin.h"

#include <qdom.h>
//Added by qt3to4:
#include <Q3ValueList>


// If defined, units will search and fire at enemy units only when they are at
//  waypoints. This may make unit movement better for big groups, because units
//  shouldn't occupy multiple cells while firing at enemies.
//#define CHECK_ENEMIES_ONLY_AT_WAYPOINT


Q3ValueVector<BoVector2Fixed> UnitMover::mCellIntersectionTable[11][11];


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

bool UnitMover::cellOccupied(int x, int y, bool ignoremoving) const
{
 if (!unit()) {
	return false;
 }
 return unit()->cellOccupied(x, y, ignoremoving);
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
				keepgoing = (qAbs(xp - start.x()) < qAbs(to.x() - start.x())) &&
						(qAbs(yp - start.y()) < qAbs(to.y() - start.y()));

				mCellIntersectionTable[x][y].push_back(BoVector2Fixed(floor(xp), floor(yp)));
			}
			mCellIntersectionTable[x][y].pop_back();
		}
	}
 }
}

bool UnitMover::advancedRectWillBeOnCanvas() const
{
#define VERBOSE_ERROR 1
 // Check if top-left point of the unit will be on canvas after moving
 if (!canvas()->onCanvas(unit()->boundingRectAdvanced().topLeft())) {
#if VERBOSE_ERROR
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
#endif //VERBOSE_ERROR

	return false;
 }

// Check if bottom-right point of the unit will be on canvas after moving
 if (!canvas()->onCanvas(unit()->boundingRectAdvanced().bottomRight())) {
#if VERBOSE_ERROR
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
#endif // VERBOSE_ERROR

	return false;
 }

#undef VERBOSE_ERROR
 return true;
}

void UnitMover::advanceMove(unsigned int advanceCallsCount)
{
 // actually move
 advanceMoveInternal(advanceCallsCount);

 // check whether velocity/whatever of this unit causes it to go off the map or
 // collide with other units. if so, revert the changes and stop moving.
 //
 // advanceMoveInternal() should also ensure these things, so advanceMoveCheck()
 // should be a noop in theory - but since moving is a very complex task, it
 // usually is required anyway.
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

 if (qMax(qAbs(unit()->centerX() - target->centerX()), qAbs(unit()->centerY() - target->centerY())) > followorder->distance()+1) {
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
 if (!advancedRectWillBeOnCanvas()) {
	unit()->clearOrders();
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
 } else if (qAbs(xspeed) == qAbs(yspeed)) {
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

 return false;
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
 if (order && (order->isMoveOrder())) {
	unit()->currentSuborderDone(success);
 }

// boDebug() << k_funcinfo << "x=" << unit()->x() << " y=" << unit()->y() << " centerx=" << unit()->centerX() << " centery=" << unit()->centerY() << endl;
}

void UnitMover::pathPointDone()
{
 unit()->pathPointDone();
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

int UnitMoverLand::pathPointCount() const
{
 return unit()->pathPointCount();
}

const BoVector2Fixed& UnitMoverLand::currentPathPoint() const
{
 return unit()->currentPathPoint();
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
 advanceMoveInternal2(advanceCallsCount);
 // advanceMoveInternal3() is called only conditionally by advanceMoveInternal2()

 advanceMoveInternal4(advanceCallsCount);
}

// TODO: rename. maybe "calculate path", "update path", "ensure path", "ensure
//       pathpoints" or something like that.
//       -> it does not actually move, but will calculate path if necessary, and
//          will abort moving if necessary.
//
void UnitMoverLand::advanceMoveInternal2(unsigned int advanceCallsCount)
{
 BO_CHECK_NULL_RET(pathInfo());
 if (pathInfo()->waiting != 0) {
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
 if (unit()->movingStatus() == UnitBase::MustSearchPath) {
	// Path is not yet searched
	// If we're moving with attacking, first check for any enemies in the range.
	if (attackEnemyUnitsInRangeWhileMoving()) {
		return;
	}
	// If there aren't any enemies, find new path
	if (!calculateNewPath()) {
		// No path was found
		stopMoving(false);
		return;
	}
 }
 if (unit()->currentOrder()->type() == UnitOrder::MoveToUnit) {
	const BoVector2Fixed& targetpos = ((UnitMoveToUnitOrder*)unit()->currentOrder())->target()->center();
	if (((UnitMoveToUnitOrderData*)unit()->currentOrderData())->lastTargetPos.dotProduct(targetpos) > 5*5) {
		// Update path
		if (!calculateNewPath()) {
			// No path was found
			stopMoving(false);
			return;
		}
	}
 }

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

 advanceMoveInternal3(advanceCallsCount);
}


// TODO: rename. maybe "actually move", "move or turn", "accelerate decelerate
//       or turn" or something like that.
//       -> here we actually "move", as in "set velocity" (moving is done
//          _after_ the advance call in BosonCanvas).
//          also we might need to calculate another part of the path, if path
//          was calculated partially only
void UnitMoverLand::advanceMoveInternal3(unsigned int)
{
 bofixed x = unit()->centerX();
 bofixed y = unit()->centerY();


 // If we're close to destination, decelerate, otherwise accelerate
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
			if (!calculateNewPath()) {
				// Probably no path could be found
				// Unit will stop moving during the next advance call (because we want
				//  to move as much as we already have in this loop)
				if (xspeed == 0 && yspeed == 0) {
					stopMoving(false);  // TODO: is false correct here?
					return;
				}
				break;
			}
		} else {
			// TODO: rotate a bit randomly
			if (xspeed == 0 && yspeed == 0) {
				stopMoving(true);
				return;
			}
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
		if (!calculateNewPath()) {
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
		pathPointDone();
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


// TODO: rename. maybe "check for waiting" or "check fo blocked path" or so.
// -> this handles "waiting" (when path is blocked)
void UnitMoverLand::advanceMoveInternal4(unsigned int advanceCallsCount)
{
 PROFILE_METHOD;
 //boDebug(401) << k_funcinfo << endl;

 if (!unit()->currentOrder() || !unit()->currentOrder()->isMoveOrder()) {
	// We might be e.g. turning
	// TODO: before turning, we have possibly already moved, so we should still
	//  check our new pos here
	// TODO: also consider e.g. MoveToUnit order
	return;
 }

 // Take special action if path is (was) blocked and we're waiting
 if (pathInfo()->waiting != 0) {
	// We try to move every 5 advance calls
	if (pathInfo()->waiting % 5 == 0) {
		// Try to move again
		if (cellOccupied(mNextCellX, mNextCellY)) {
			// Obstacle is still there. Continue waiting
			if (pathInfo()->waiting >= 600) {
				// Enough of waiting (30 secs). Give up.
				stopMoving(false);
				return;
			} else if (pathInfo()->waiting % (20 + qMin(pathInfo()->pathrecalced * 20, 80)) == 0) {
				// First wait 20 adv. calls (1 sec) before recalculating path, then 40
				//  calls, then 60 etc, but never more than 100 calls.
				boDebug(401) << k_funcinfo << "unit " << id() << ": Recalcing path, waiting: " << pathInfo()->waiting <<
						"; pathrecalced: " << pathInfo()->pathrecalced << endl;
				calculateNewPath();
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

 advanceMoveInternal5(advanceCallsCount);
}

void UnitMoverLand::advanceMoveDoCrushing(unsigned int)
{
 // AB: FIXME: this really is no moving code. it does not belong in this class
 // at all.
#define CRUSHING 0
#if CRUSHING
 bool wait = false;
 Q3ValueList<Unit*> collisions;
 collisions = canvas()->collisionsInBox(BoVector3Fixed(unit()->x() + unit()->xVelocity(), unit()->y() + unit()->yVelocity(), unit()->z()),
		BoVector3Fixed(unit()->x() + unit()->xVelocity() + unit()->width(), unit()->y() + unit()->yVelocity() + unit()->height(), unit()->z() + unit()->depth()), unit());
 if (!collisions.isEmpty()) {
	for (Q3ValueList<Unit*>::Iterator it = collisions.begin(); it != collisions.end(); it++) {
		Unit* u = *it;
		if (unit() == u) {
			continue;
		} else if (u->isFlying() != unit()->isFlying()) {
			continue;
		} else if (!u->bosonCollidesWith(unit())) {
			continue;
		}
		// Make sure we actually can crush this unit
		if (!unit()->canCrush(u)) {
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
#endif // CRUSHING
#undef CRUSHING
}

// TODO: rename.
void UnitMoverLand::advanceMoveInternal5(unsigned int)
{
 if (unit()->isInsideUnit()) {
	// FIXME: is this correct here?
	//        I honestly have no clue if this method is needed if we move
	//        inside a unit.
	//        we definitely don't have a mNextWaypointIntersections pointer,
	//        so we somehow need to work around that.
	return;
 }

 BO_CHECK_NULL_RET(mNextWaypointIntersections);
 if (unit()->pathPointCount() == 0) {
	// This is allowed and means that unit will stop after this advance call
	return;
 }

 if (unit()->xVelocity() == 0 && unit()->yVelocity() == 0) {
	// Probably unit stopped to attack other units
	return;
 }

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
 if (cellOccupied(mNextCellX, mNextCellY)) {
	// Gotta wait
	unit()->setVelocity(0, 0, 0);
	unit()->setMovingStatus(UnitBase::Waiting);
	unit()->setSpeed(0);
	pathInfo()->waiting++;
	return;
 }


 pathInfo()->waiting = 0;
 pathInfo()->pathrecalced = 0;
}

// AB: WARNING crushing is currently disabled!
//     -> it does NOT belong into this method!
//
// AB: advanceMoveCheck() was meant to _check_ whether the move is valid, i.e.
//     check if the calculated velocity or so may cause the unit to go off the
//     map or collide with a different unit.
//     however atm it is heavily used to actually do a lot of calculations,
//     which is not intended at all.
//     this should be fixed.
void UnitMoverLand::advanceMoveCheck()
{
 PROFILE_METHOD;
 BO_CHECK_NULL_RET(unit());

 if (unit()->xVelocity() == 0 && unit()->yVelocity() == 0) {
	return;
 }

 // Make sure unit is on canvas
 UnitMover::advanceMoveCheck();

 if (!unit()->currentOrder() || !unit()->currentOrder()->isMoveOrder()) {
	// apparently the velocity was set to something != 0 although it should
	// not. revert.
	unit()->setVelocity(0, 0, 0);
	return;
 }

// TODO .. rewrite this. should check for collisions on position + velocity
//         --> if collisions occur: check if they are about to be crushed. if
//             not: don't move there. stop moving completely.
#if 0
 // Make sure the next cell is free of any obstacles
 if (cellOccupied(mNextCellX, mNextCellY)) {
	// Gotta wait
	unit()->setVelocity(0, 0, 0);
	unit()->setSpeed(0);
	stopMoving(false);
	return;
 }
#endif



// AB: crushing does NOT belong here. but it MUST be after advanceMoveCheck() -
//     i.e. at a pointer where the unit _definitely_ goes by the velocities it
//     has been set to.
#warning What the hell does crushing do in advanceMove_Check_() ??
#define CRUSHING 0
#if CRUSHING
 advanceMoveDoCrushing(advanceCallsCount);
#endif // CRUSHING
#undef CRUSHING
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
 if (unit()->isInsideUnit()) {
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
	if ((qAbs(newCellX - xpos) >= 6) || (qAbs(newCellY - ypos) >= 6)) {
		// We're too far
		break;
	}
	int xindex = newCellX - xpos + 5;
	int yindex = newCellY - ypos + 5;
	for (unsigned int i = 0; i < mCellIntersectionTable[xindex][yindex].count(); i++) {
		// Check if this cell is accessable
		// TODO: check terrain
		if (cellOccupied(xpos + mCellIntersectionTable[xindex][yindex][i].x(),
				ypos + mCellIntersectionTable[xindex][yindex][i].y(), true)) {
			cango = false;
			break;
		}
	}
	if (cango) {
		// skip previous pathpoint (go directly to this one)
		pathPointDone();
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
 BoRect2Fixed rect(unit()->leftEdge() - unit()->speed() * 40 - 1,
		unit()->topEdge() - unit()->speed() * 40 - 1,
		unit()->rightEdge() + unit()->speed() * 40 + 1,
		unit()->bottomEdge() + unit()->speed() * 40 + 1);
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
		if (qAbs(sideDistance) < unitSizesSum) {
			// We'll crash with this unit if we don't act
			// Calculate how much we'll try to avoid this unit
			bofixed factor = 1;  // Positive = turn right
			if (sideDistance > 0) {
				// The other unit is right of us. Turn left
				factor *= -1;
			}
			// The more int front of us the unit is, the bigger the factor
			factor *= unitSizesSum * 1.5 - qAbs(sideDistance);
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
 if (!pathInfo()) {
	BO_NULL_ERROR(pathInfo());
	return false;
 }

 if (!pathInfo()->movedata) {
	BO_NULL_ERROR(pathInfo()->movedata);
	return false;
 }
 int ppx = (int)unit()->currentPathPoint().x();
 int ppy = (int)unit()->currentPathPoint().y();
 if ((qAbs(ppx - xpos) >= 6) || (qAbs(ppy - ypos) >= 6)) {
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
			} else if (u->maxHealth() <= pathInfo()->movedata->crushDamage && u->owner() != unit()->owner()) {
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

bool UnitMoverLand::calculateNewPath()
{
 BosonProfiler profiler("calculateNewPath");
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
 // FIXME: AB: rivo: 1.0f is a very long distance - by using 0.5 or so we could
 //            have much more precise movements. useful for moving inside units.
 //            would that be ok?
 //        AB: even better: what about using unit()->boundingRect().contains() ?
 if (qMax(qAbs(pathInfo()->dest.x() - pathInfo()->start.x()), qAbs(pathInfo()->dest.x() - pathInfo()->start.x()))
		< 1.0f) {
	return false;
 }

 Q3ValueVector<BoVector2Fixed> pathPoints;
 if (!calculateNewPathPathPoints(&pathPoints)) {
	// Stop moving
	return false;
 }

 for (int unsigned i = 0; i < pathPoints.count(); i++) {
	unit()->addPathPoint(pathPoints[i]);
 }

 // Reset last cell
 mLastCellX = -1;
 mLastCellY = -1;
 mNextWaypointIntersections = &mCellIntersectionTable[5][5];

 // AB: FIXME: do we need a unit()->setMovingStatus() here?
 //            -> it's pretty dumb to have movingStatus() == MustSearchPath at
 //               this point

 return true;
}

bool UnitMoverLand::calculateNewPathPathPoints(Q3ValueVector<BoVector2Fixed>* pathPoints)
{
 pathPoints->clear();
 if (!pathInfo()) {
	return false;
 }

 // Find path
 canvas()->pathFinder()->findPath(pathInfo());

 if (pathInfo()->result == BosonPath::NoPath || pathInfo()->llpath.count() == 0) {
	return false;
 }

 // Copy low-level path to pathpoints' list
 *pathPoints = pathInfo()->llpath;

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
 unit()->moveCenterTo(unit()->centerX(), unit()->centerY(), unit()->unitProperties()->preferredAltitude());
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
 if (unit()->maxSpeed() == 0) {
	return;
 }
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
 if (unit()->leftEdge() < 0.5 || unit()->topEdge() < 0.5 ||
		unit()->rightEdge() > bofixed(canvas()->mapWidth()) - 0.5 || unit()->bottomEdge() > bofixed(canvas()->mapHeight()) - 0.5) {
	bofixed x = unit()->centerX();
	bofixed y = unit()->centerY();
	x = qMax(x, bofixed(1 + unit()->width()/2.0));
	y = qMax(y, bofixed(1 + unit()->height()/2.0));
	x = qMin(x, bofixed(canvas()->mapWidth()) - unit()->width()/2.0 - 1);
	y = qMin(y, bofixed(canvas()->mapHeight()) - unit()->height()/2.0 - 1);
	unit()->moveCenterTo(x, y, unit()->z());
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
	mRoll += qMin(delta, maxrollincrease);
 } else if (delta < 0) {
	mRoll -= qMin(qAbs(delta), maxrollincrease);
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

// boDebug(401) << k_funcinfo << "unit " << id() << endl;


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
#if 1
 // AB: z position of flying units are pretty much unused atm
 totarget.setZ(0);
#endif
 bofixed totargetlen = totarget.length();

// boDebug() << "flying_advanceMoveInternal "<< id() << " is at " << x << "," << y << "," << z << " z2=" << unit()->z() << " target at: " << targetpos.x() << "," << targetpos.y() << " " << targetpos.z() << " totargetlen: " << totargetlen << " range: " << pathInfo()->range << " unitspeed: " << unit()->speed() << endl;

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
	rotationdelta = qAbs(rotationdelta);
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
 /*bofixed wantedroll = qMin(difflen / turnspeed, bofixed(1)) * 45;
 if (totarget.crossProduct(velo).z() < 0) {
	// Turning left
	wantedroll = -wantedroll;
 } else {
	// Turning right
 }*/
 bofixed wantedroll = qMin(rotationdelta / maxturningspeed, bofixed(1)) * 45;
 if (!turncw) {
	// Turning left
	wantedroll = -wantedroll;
 }

 const bofixed maxrollincrease = 2;
 bofixed delta = wantedroll - mRoll;
 if (delta > 0) {
	mRoll += qMin(delta, maxrollincrease);
 } else if (delta < 0) {
	mRoll -= qMin(qAbs(delta), maxrollincrease);
 }

 unit()->setVelocity(velo.x(), velo.y(), velo.z());
 unit()->setRotation(newrotation);
 //setXRotation(Bo3dTools::rotationToPoint(sqrt(1 - velo.z() * velo.z()), velo.z()) - 90);
 unit()->setYRotation(mRoll);

 unit()->setMovingStatus(UnitBase::Moving);
}


UnitMoverInsideUnit::UnitMoverInsideUnit(Unit* u)
	: UnitMoverLand(u)
{
}

UnitMoverInsideUnit::~UnitMoverInsideUnit()
{
}

bool UnitMoverInsideUnit::calculateNewPathPathPoints(Q3ValueVector<BoVector2Fixed>* pathPoints)
{
 pathPoints->clear();
 if (!pathInfo()) {
	boError(401) << k_funcinfo  << "NULL pathInfo()" << endl;
	return false;
 }

 EnterUnitPlugin* enterUnit = (EnterUnitPlugin*)unit()->plugin(UnitPlugin::EnterUnit);
 if (!enterUnit) {
	boError(401) << k_funcinfo  << "cannot move inside a unit without a EnterUnitPlugin" << endl;
	return false;
 }

 if (unit()->currentOrder()->type() != UnitOrder::MoveInsideUnit) {
	// Move, MoveToUnit etc. are not supported
	return false;
 }

 canvas()->pathFinder()->preparePathInfo(pathInfo());

 // actually search path here
 // TODO: maybe use "enterUnit->getPathPoints()" instead?
 switch (enterUnit->movingInStatus()) {
	default:
	case EnterUnitPlugin::StatusIsOutside:
		boDebug(401) << k_funcinfo << "movingInStatus " << enterUnit->movingInStatus() << " should not be handled by this class" << endl;
		return false;
	case EnterUnitPlugin::StatusMovingToStoragePosition:
	case EnterUnitPlugin::StatusMovingToOutside:
	{
		boDebug(401) << k_funcinfo << "moving in/out: adding pathpoint" << endl;

		bofixed add_ = 0;
		if (!unit()->unitProperties()->isAircraft()) {
			if (pathInfo() && pathInfo()->movedata) {
				add_ = (((pathInfo()->movedata->size % 2) == 1) ? 0.5 : 0);
			}
		}
		BoVector2Fixed add(add_, add_);

		Q3ValueList<BoVector2Fixed> path = enterUnit->remainingInsidePath();
		pathPoints->clear();
		pathPoints->reserve(path.count());
		for (Q3ValueList<BoVector2Fixed>::iterator it = path.begin(); it != path.end(); ++it) {
			pathPoints->append(*it + add);
		}

		break;
	}
	case EnterUnitPlugin::StatusHasEntered:
		boDebug(401) << k_funcinfo << "isInside: nothing yet" << endl;
		return false;
		break;
 }


 return true;
}

void UnitMoverInsideUnit::advanceMoveInternal(unsigned int advanceCallsCount)
{
 if (!unit()->isInsideUnit()) {
	boError(401) << k_funcinfo << "not inside a unit" << endl;
	return;
 }
 BO_CHECK_NULL_RET(unit()->currentOrder());
// boDebug() << k_funcinfo << endl;

 // safety check - at this point we should always have a move order.
 if (unit()->currentOrder()->isMoveOrder()) {
	if (unit()->currentOrder()->type() != UnitOrder::MoveInsideUnit) {
		// AB: I'd like to add a LEAVE suborder now, however that would
		//     clear the current move order, since setMovingStatus() is
		//     not MustSearchPath anymore, once the LEAVE order is
		//     completed.
		//     -> so instead we MUST have a separate LEAVE order before
		//        a unit that is inside a unit can move again.
#if 1
		boDebug(401) << k_funcinfo << "not a MoveInsideUnit order. suborder done. (rather supposed to LEAVE here)" << endl;
		unit()->currentSuborderDone(false);
		return;
#else
		boDebug(401) << k_funcinfo << "adding LEAVE order" << endl;
		UnitEnterUnitOrder* o = new UnitEnterUnitOrder(unit());
		o->setIsLeaveOrder(true);
		unit()->addCurrentSuborder(o);
		return;
#endif
	}
 }

 if (!unit()->currentOrder()->isMoveOrder()) {
	boError(401) << k_funcinfo << "dont have a move order" << endl;
	return;
 }

 EnterUnitPlugin* enterUnit = (EnterUnitPlugin*)unit()->plugin(UnitPlugin::EnterUnit);
 if (!enterUnit) {
	BO_NULL_ERROR(enterUnit);
	return;
 }

 EnterUnitPlugin::LandingStatus landingStatus = enterUnit->landingStatus();
 if (!unit()->unitProperties()->isAircraft()) {
	landingStatus = EnterUnitPlugin::LandingStatusNone;
 }

 switch (landingStatus) {
	case EnterUnitPlugin::LandingStatusNone:
	{
		UnitMoverLand::advanceMoveInternal(advanceCallsCount);
		break;
	}
	case EnterUnitPlugin::LandingStatusGoingTowardsTakeOffPoint:
	{
		bool isTakingOff = false;
		advanceMoveInternalTakingOff(advanceCallsCount, isTakingOff);
		break;
	}
	case EnterUnitPlugin::LandingStatusIsTakingOff:
	{
		bool isTakingOff = true;
		advanceMoveInternalTakingOff(advanceCallsCount, isTakingOff);
		break;
	}
	case EnterUnitPlugin::LandingStatusFlyingTowardsLandingPoint:
	{
		bool isLanding = false;
		advanceMoveInternalLanding(advanceCallsCount, isLanding);
		break;
	}
	case EnterUnitPlugin::LandingStatusIsLanding:
	{
		bool isLanding = true;
		advanceMoveInternalLanding(advanceCallsCount, isLanding);
		break;
	}
	default:
		boError(401) << k_funcinfo << "unhandled landingStatus() " << enterUnit->landingStatus() << endl;
		return;
 }
}

void UnitMoverInsideUnit::advanceMoveCheck()
{
 // AB: NOT UnitMoverLand::advanceMoveCheck().
 //     -> onCanvas check only, no collision checks.
 //        colliding with other units while inside a unit is ok.
 UnitMover::advanceMoveCheck();
}

// isLanding == true: once next pathpoint is reached, we do setIsFlying(false).
// isLanding == false: once next pathpoint is reached, we start landing.
//                     -> i.e. we are moving towards landing point
void UnitMoverInsideUnit::advanceMoveInternalLanding(unsigned int advanceCallsCount, bool isLanding)
{
// boDebug(401) << k_funcinfo << endl;

 if (unit()->maxSpeed() == 0) {
#if 0
#error: TODO
// -> leave storing unit?
#else
	// If unit's max speed is 0, it cannot move
	boError(401) << k_funcinfo << "unit " << id() << ": maxspeed == 0" << endl;
	stopMoving(false);
	unit()->setMovingStatus(UnitBase::Standing);
#endif
	return;
 }

 if (unit()->movingStatus() == UnitBase::MustSearchPath) {
	if (!calculateNewPath()) {
		// No path was found
		stopMoving(false);
		boDebug(401) << k_funcinfo << "no path found" << endl;
		return;
	}
	boDebug(401) << k_funcinfo << "path found. should be :)" << endl;
 }

 if (pathPointCount() == 0) {
	stopMoving(true);
	return;
 }

 BoVector2Fixed pp = currentPathPoint();
 BoVector3Fixed targetpos = BoVector3Fixed(pp.x(), pp.y(), unit()->centerZ());
// boDebug(401) << k_funcinfo << debugStringVector(targetpos) << endl;

// targetpos.setZ(canvas()->heightAtPoint(targetpos.x(), targetpos.y()) + 3);


 bofixed x = unit()->centerX();
 bofixed y = unit()->centerY();
 bofixed z = unit()->centerZ();

 unit()->accelerate();


 BoVector3Fixed totarget(targetpos.x() - x, targetpos.y() - y, targetpos.z() - z);
#if 1
 // AB: z position of flying units are pretty much unused atm
 totarget.setZ(0);
#endif
 bofixed totargetlen = totarget.length();

 if (totargetlen <= pathInfo()->range || totargetlen <= unit()->speed()) {
#if 0
	boDebug(401) << k_funcinfo << "landing pathpoint complete - totargetlen="
			<< totargetlen
			<< " targetpos="
			<< debugStringVector(targetpos)
			<< " pos="
			<< debugStringVector(BoVector3Fixed(x, y, z))
			<< " isLanding="
			<< isLanding
			<< endl;
#endif

	EnterUnitPlugin* enterUnit = (EnterUnitPlugin*)unit()->plugin(UnitPlugin::EnterUnit);
	if (enterUnit) {
		if (isLanding) {
			enterUnit->completeLanding();
		} else {
			enterUnit->startLanding();
		}
	}
	pathPointDone();
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
	rotationdelta = qAbs(rotationdelta);
	turncw = true;
 }
 if (rotationdelta > 180) {
	rotationdelta = 180 - (rotationdelta - 180);
	turncw = !turncw;
 }

 // AB: we ignore rotationSpeed() completely at this point.
 //     -> we already are inside a unit, moving must not fail anymore!
 newrotation = wantedrotation;

 if (newrotation < 0) {
	newrotation += 360;
 } else if (newrotation >= 360) {
	newrotation -= 360;
 }

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

 bofixed wantedroll = qMin(rotationdelta / unitProperties()->rotationSpeed(), bofixed(1)) * 45;
 if (!turncw) {
	// Turning left
	wantedroll = -wantedroll;
 }

 const bofixed maxrollincrease = 2;
 float mRoll = unit()->yRotation();
 bofixed delta = wantedroll - mRoll;
 if (delta > 0) {
	mRoll += qMin(delta, maxrollincrease);
 } else if (delta < 0) {
	mRoll -= qMin(qAbs(delta), maxrollincrease);
 }

 unit()->setVelocity(velo.x(), velo.y(), velo.z());
 unit()->setRotation(newrotation);
 //setXRotation(Bo3dTools::rotationToPoint(sqrt(1 - velo.z() * velo.z()), velo.z()) - 90);
 unit()->setYRotation(mRoll);

 unit()->setMovingStatus(UnitBase::Moving);

}

void UnitMoverInsideUnit::advanceMoveInternalTakingOff(unsigned int advanceCallsCount, bool isTakingOff)
{
// boDebug(401) << k_funcinfo << endl;

 if (unit()->maxSpeed() == 0) {
	// If unit's max speed is 0, it cannot move
	boError(401) << k_funcinfo << "unit " << id() << ": maxspeed == 0" << endl;
	stopMoving(false);
	unit()->setMovingStatus(UnitBase::Standing);
	return;
 }

 if (unit()->movingStatus() == UnitBase::MustSearchPath) {
	if (!calculateNewPath()) {
		// No path was found
		stopMoving(false);
		boDebug(401) << k_funcinfo << "no path found" << endl;
		return;
	}
	boDebug(401) << k_funcinfo << "path found. should be :)" << endl;
 }

 if (pathPointCount() == 0) {
	stopMoving(true);
	return;
 }

 BoVector2Fixed pp = currentPathPoint();
 BoVector3Fixed targetpos = BoVector3Fixed(pp.x(), pp.y(), unit()->centerZ());
// boDebug(401) << k_funcinfo << debugStringVector(targetpos) << endl;

// targetpos.setZ(canvas()->heightAtPoint(targetpos.x(), targetpos.y()) + 3);


 bofixed x = unit()->centerX();
 bofixed y = unit()->centerY();
 bofixed z = unit()->centerZ();

 unit()->accelerate();


 BoVector3Fixed totarget(targetpos.x() - x, targetpos.y() - y, targetpos.z() - z);
#if 1
 // AB: z position of flying units are pretty much unused atm
 totarget.setZ(0);
#endif
 bofixed totargetlen = totarget.length();

 if (totargetlen <= pathInfo()->range || totargetlen <= unit()->speed()) {
#if 0
	boDebug(401) << k_funcinfo << "starting pathpoint complete - totargetlen="
			<< totargetlen
			<< " targetpos="
			<< debugStringVector(targetpos)
			<< " pos="
			<< debugStringVector(BoVector3Fixed(x, y, z))
			<< " isTakingOff="
			<< isTakingOff
			<< endl;
#endif

	EnterUnitPlugin* enterUnit = (EnterUnitPlugin*)unit()->plugin(UnitPlugin::EnterUnit);
	if (enterUnit) {
		if (isTakingOff) {
			enterUnit->completeTakingOff();
		} else {
			enterUnit->startTakingOff();
		}
	}
	pathPointDone();
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
	rotationdelta = qAbs(rotationdelta);
	turncw = true;
 }
 if (rotationdelta > 180) {
	rotationdelta = 180 - (rotationdelta - 180);
	turncw = !turncw;
 }

 // AB: we ignore rotationSpeed() completely at this point.
 //     -> we already are inside a unit, moving must not fail anymore!
 newrotation = wantedrotation;

 if (newrotation < 0) {
	newrotation += 360;
 } else if (newrotation >= 360) {
	newrotation -= 360;
 }

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

 bofixed wantedroll = qMin(rotationdelta / unitProperties()->rotationSpeed(), bofixed(1)) * 45;
 if (!turncw) {
	// Turning left
	wantedroll = -wantedroll;
 }

 const bofixed maxrollincrease = 2;
 float mRoll = unit()->yRotation();
 bofixed delta = wantedroll - mRoll;
 if (delta > 0) {
	mRoll += qMin(delta, maxrollincrease);
 } else if (delta < 0) {
	mRoll -= qMin(qAbs(delta), maxrollincrease);
 }

 unit()->setVelocity(velo.x(), velo.y(), velo.z());
 unit()->setRotation(newrotation);
 //setXRotation(Bo3dTools::rotationToPoint(sqrt(1 - velo.z() * velo.z()), velo.z()) - 90);
 unit()->setYRotation(mRoll);

 unit()->setMovingStatus(UnitBase::Moving);
}

bool UnitMoverInsideUnit::cellOccupied(int x, int y, bool ignoremoving) const
{
 Q_UNUSED(x);
 Q_UNUSED(y);
 Q_UNUSED(ignoremoving);

 return false;
}

void UnitMoverInsideUnit::pathPointDone()
{
 BO_CHECK_NULL_RET(unit());
 if (!unit()->isInsideUnit()) {
	return;
 }
 EnterUnitPlugin* enterUnit = (EnterUnitPlugin*)unit()->plugin(UnitPlugin::EnterUnit);
 BO_CHECK_NULL_RET(enterUnit);

 UnitMover::pathPointDone();
 enterUnit->pathPointDone();
}

bool UnitMoverInsideUnit::canGoToCurrentPathPoint(int xpos, int ypos)
{
 if (!pathInfo()) {
	BO_NULL_ERROR(pathInfo());
	return false;
 }
 if (!unit()->isInsideUnit()) {
	boError(401) << k_funcinfo << "this class should not be used when unit is not inside a unit!" << endl;
	return UnitMoverLand::canGoToCurrentPathPoint(xpos, ypos);
 }

 // AB: no validity checks are done to pathpoints.
 //     all pathpoints inside a unit are assumed to be reachable, once we
 //     received them from the storing unit.
 return true;
}

