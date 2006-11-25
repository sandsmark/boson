/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

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

#include "enterunitplugin.h"

#include "../../bomemory/bodummymemory.h"
#include "unitstorageplugin.h"
#include "unit.h"
#include "unitorder.h"
#include "unitproperties.h"
#include "pluginproperties.h"
#include "bo3dtools.h"
#include "boson.h"
#include "bodebug.h"

#include <qdom.h>

EnterUnitPlugin::EnterUnitPlugin(Unit* owner)
	: UnitPlugin(owner)
{
 mUnitStoragePlugin = 0;
 owner->registerData(&mEnterPoint1, Unit::IdEnterPointOutside1);
 owner->registerData(&mEnterPoint2, Unit::IdEnterPointOutside2);
 owner->registerData(&mRemainingInsidePath, Unit::IdEnterUnitRemainingInsidePath);
 owner->registerData(&mPathIndex, Unit::IdEnterUnitPathIndex);
 owner->registerData(&mMovingInStatus, Unit::IdMovingInStatus);
 owner->registerData(&mLandingStatus, Unit::IdLandingStatus);
 owner->registerData(&mTriedMovingCounter, Unit::IdEnterUnitTriedMovingCounter);

 mEnterPoint1 = BoVector2Fixed(0.0f, 0.0f);
 mEnterPoint2 = BoVector2Fixed(0.0f, 0.0f);
 mMovingInStatus = StatusIsOutside;
 mLandingStatus = LandingStatusNone;
 mTriedMovingCounter = 0;
}

EnterUnitPlugin::~EnterUnitPlugin()
{
}

EnterUnitPlugin::MovingInStatus EnterUnitPlugin::movingInStatus() const
{
 return (MovingInStatus)mMovingInStatus.value();
}

EnterUnitPlugin::LandingStatus EnterUnitPlugin::landingStatus() const
{
 return (LandingStatus)mLandingStatus.value();
}

void EnterUnitPlugin::startLanding()
{
 mLandingStatus = LandingStatusIsLanding;
}

void EnterUnitPlugin::completeLanding()
{
 mLandingStatus = LandingStatusNone;
 unit()->setIsFlying(false);
}

void EnterUnitPlugin::startTakingOff()
{
 mLandingStatus = LandingStatusIsTakingOff;
}

void EnterUnitPlugin::completeTakingOff()
{
 mLandingStatus = LandingStatusNone;
 unit()->setIsFlying(true);
}

UnitStoragePlugin* EnterUnitPlugin::storingUnit() const
{
 return mUnitStoragePlugin;
}

bool EnterUnitPlugin::saveAsXML(QDomElement& root) const
{
 unsigned int destStorageId = 0;
 if (mUnitStoragePlugin) {
	destStorageId = mUnitStoragePlugin->unit()->id();
 }
 root.setAttribute(QString::fromLatin1("DestinationUnitStorage"), destStorageId);

 return true;
}

bool EnterUnitPlugin::loadFromXML(const QDomElement& root)
{
 bool ok = false;
 unsigned int destStorageId = 0;

 destStorageId = root.attribute(QString::fromLatin1("DestinationUnitStorage")).toUInt(&ok);
 if (!ok) {
	boError() << k_funcinfo << "Invalid number for DestinationUnitStorage attribute" << endl;
	return false;
 }
 if (destStorageId != 0) {
	Unit* u = game()->findUnit(destStorageId, 0);
	if (!u) {
		boError() << k_funcinfo << "cannot find unit storage " << destStorageId << endl;
		return false;
	}
	mUnitStoragePlugin = (UnitStoragePlugin*)u->plugin(UnitPlugin::UnitStorage);
	if (!mUnitStoragePlugin) {
		boError() << k_funcinfo << "unit " << destStorageId << " is not a UnitStorage" << endl;
		mUnitStoragePlugin = 0;
		return false;
	}
 }

 return true;
}

void EnterUnitPlugin::unitDestroyed(Unit* unit)
{
 if (mUnitStoragePlugin && unit == mUnitStoragePlugin->unit()) {
	mUnitStoragePlugin = 0;
	changeStatus(StatusIsOutside);
 }
}

void EnterUnitPlugin::itemRemoved(BosonItem* item)
{
 if (!RTTI::isUnit(item->rtti())) {
	return;
 }
 Unit* unit = (Unit*)item;
 if (mUnitStoragePlugin && unit == mUnitStoragePlugin->unit()) {
	mUnitStoragePlugin = 0;
	changeStatus(StatusIsOutside);
 }
}



bool EnterUnitPlugin::enter(Unit* enterUnit)
{
 if (!enterUnit) {
	return false;
 }
 boDebug() << k_funcinfo << unit()->id() << " trying to enter " << enterUnit->id() << endl;
 if (movingInStatus() != StatusIsOutside) {
	boDebug() << k_funcinfo << "not StatusIsOutside" << endl;
	return false;
 }
 UnitStoragePlugin* storage = (UnitStoragePlugin*)enterUnit->plugin(UnitPlugin::UnitStorage);
 if (!storage) {
	return false;
 }
 if (!storage->canStore(unit())) {
	boDebug() << k_funcinfo << "cannot store " << unit()->id() << endl;
	return false;
 }

 mUnitStoragePlugin = storage;

 changeStatus(StatusMovingToEnterPoint1);

 return true;
}

bool EnterUnitPlugin::leave()
{
 if (!unit()->isInsideUnit()) {
	boDebug() << k_funcinfo << unit()->id() << " is not inside a unit" << endl;
	return false;
 }
 boDebug() << k_funcinfo << endl;
 if (movingInStatus() != StatusHasEntered) {
	return false;
 }
 if (!mUnitStoragePlugin) {
	BO_NULL_ERROR(mUnitStoragePlugin);
	changeStatus(StatusHasLeft);
	return false;
 }

 if (!storingUnit()) {
	// unit will probably be unusable now: is inside a unit, but has no
	// storing unit. completely invalid.
	BO_NULL_ERROR(storingUnit());
	return false;
 }

 changeStatus(StatusWaitForLeavePermission);

 return true;
}

void EnterUnitPlugin::abort()
{
 boDebug() << k_funcinfo << "status: " << movingInStatus() << endl;
 switch (movingInStatus()) {
	case StatusHasLeft:
	case StatusHasEntered:
	case StatusIsOutside:
		// nothing to abort
		break;
	case StatusMovingToEnterPoint1:
	case StatusWaitForEnterPermission:
	case StatusMovingToEnterPoint2:
	{
		changeStatus(StatusIsOutside);
		break;
	}
	case StatusMovingToStoragePosition:
	{
		// can't be aborted!
		boWarning() << k_funcinfo << "StatusMovingToStoragePosition can't be aborted!" << endl;
		return;
	}
	case StatusWaitForLeavePermission:
	{
		changeStatus(StatusHasEntered);
		break;
	}
	case StatusMovingToOutside:
	{
		// can't be aborted (yet?)
		// this definitely can't be aborted, if unit is an airplane and
		// already took off.
		// but otherwise we might be able to abort leaving, by moving
		// back to storage position.
		boWarning() << k_funcinfo << "StatusMovingToOutside can't (yet?) be aborted!" << endl;
		break;
	}

	default:
	{
		boError() << k_funcinfo << "unexpected status " << (int)movingInStatus() << endl;
		break;
	}
 }
}

void EnterUnitPlugin::changeStatus(MovingInStatus status)
{
 boDebug() << k_funcinfo << (int)status << endl;
 mMovingInStatus = status;
 mTriedMovingCounter = 0;

 switch (movingInStatus()) {
	case StatusIsOutside:
	{
		unit()->setIsInsideUnit(false);
		mLandingStatus = LandingStatusNone;

		if (mUnitStoragePlugin) {
			mUnitStoragePlugin->revokeEnterRequest(unit());
		}
		break;
	}
	case StatusMovingToEnterPoint1:
	{
		bool ok = true;
		if (!mUnitStoragePlugin) {
			ok = false;
		}
		if (ok) {
			ok = requestEnterPath();
		}

		if (!ok) {
			boDebug() << k_funcinfo << "could not get enter path. aborting." << endl;
			changeStatus(StatusIsOutside);
		}
		break;
	}
	case StatusWaitForEnterPermission:
	{
		if (!mUnitStoragePlugin) {
			changeStatus(StatusIsOutside);
		} else {
			mUnitStoragePlugin->requestEnterPermission(unit());
		}
		break;
	}
	case StatusMovingToEnterPoint2:
	{
		bool ok = true;
		if (!mUnitStoragePlugin) {
			ok = false;
		}

		if (!ok) {
			boDebug() << k_funcinfo << "abort entering while trying to move to enter point 2" << endl;
			changeStatus(StatusIsOutside);
		}
		break;
	}
	case StatusMovingToStoragePosition:
	{
		bool enterPermission = true;
		if (!mUnitStoragePlugin) {
			enterPermission = false;
		}
		if (enterPermission) {
			bool wait = false;
			mUnitStoragePlugin->getEnterPermissionResult(unit(), &wait, &enterPermission);
			if (wait) {
				enterPermission = false;
			}
		}
		if (enterPermission) {
			enterPermission = mUnitStoragePlugin->takeMe(unit(), mPathIndex);
		}

		if (!enterPermission) {
			changeStatus(StatusIsOutside);
		} else {
			unit()->setIsInsideUnit(true);
			mLandingStatus = LandingStatusNone;
			if (unit()->isFlying()) {
				mLandingStatus = LandingStatusFlyingTowardsLandingPoint;
			}
		}
		break;
	}
	case StatusHasEntered:
	{
		mLandingStatus = LandingStatusNone;

		UnitOrder* order = unit()->currentOrder();
		if (order && order->type() == UnitOrder::EnterUnit) {
			UnitEnterUnitOrder* o = (UnitEnterUnitOrder*)unit()->currentOrder();
			if (o->isLeaveOrder()) {
				boDebug() << k_funcinfo << "Leave order aborted" << endl;
				unit()->currentSuborderDone(false);
			} else {
				boDebug() << k_funcinfo << "Enter order completed" << endl;
				unit()->currentSuborderDone(true);
			}
		}

		// note: we can reach this state not only through an ENTER
		// order, but also if a request to LEAVE has been denied (or
		// aborted)!

		if (!mUnitStoragePlugin) {
			BO_NULL_ERROR(mUnitStoragePlugin);
			changeStatus(StatusIsOutside);
		} else {
			mUnitStoragePlugin->arrivedAtStoragePosition(unit());
		}

		break;
	}
	case StatusWaitForLeavePermission:
	{
		if (!mUnitStoragePlugin) {
			boError() << k_funcinfo << "status: " << movingInStatus() << endl;
			BO_NULL_ERROR(mUnitStoragePlugin);
			changeStatus(StatusHasLeft);
		} else {
			mUnitStoragePlugin->requestLeavePermission(unit());
		}
		break;
	}
	case StatusMovingToOutside:
	{
		if (!mUnitStoragePlugin) {
			boError() << k_funcinfo << "status: " << movingInStatus() << endl;
			BO_NULL_ERROR(mUnitStoragePlugin);
			changeStatus(StatusHasLeft);
		} else {
			mLandingStatus = LandingStatusNone;
			if (unit()->unitProperties()->isAircraft()) {
				mLandingStatus = LandingStatusGoingTowardsTakeOffPoint;
			}
			bool ok = requestLeavePath();
			if (!ok) {
				boDebug() << k_funcinfo << "cannot get leave path. (leave permission revoked?" << endl;
				changeStatus(StatusHasEntered);
				break;
			}
		}
		break;
	}
	case StatusHasLeft:
	{
		// dummy state: final cleanups
		// we'll switch to StatusIsOutside before leaving this method

		unit()->setIsInsideUnit(false);
		mLandingStatus = LandingStatusNone;
		UnitOrder* order = unit()->currentOrder();
		if (order && order->type() == UnitOrder::EnterUnit) {
			UnitEnterUnitOrder* o = (UnitEnterUnitOrder*)unit()->currentOrder();
			if (o->isLeaveOrder()) {
				boDebug() << k_funcinfo << "Leave order aborted" << endl;
				unit()->currentSuborderDone(false);
			} else {
				boDebug() << k_funcinfo << "Enter order completed" << endl;
				unit()->currentSuborderDone(true);
			}
		}

		if (mUnitStoragePlugin) {
			mUnitStoragePlugin->releaseMe(unit());
		}
		mUnitStoragePlugin = 0;

		changeStatus(StatusIsOutside);
		break;
	}

	default:
		boError() << k_funcinfo << "invalid status " << (int)movingInStatus() << endl;
		changeStatus(StatusIsOutside);
		break;
 }
}

void EnterUnitPlugin::advance(unsigned int advanceCallsCount)
{
// boDebug() << k_funcinfo << "id: " << unit()->id() << endl;
 if (!unit()->isMobile()) {
	boError() << k_funcinfo << "only mobile units can enter other units" << endl;
	changeStatus(StatusIsOutside);
	unit()->currentSuborderDone(false);
	return;
 }
 if (!mUnitStoragePlugin) {
	// may happen if destination was destroyed while entering
	boDebug() << k_funcinfo << "have no storage plugin" << endl;
	changeStatus(StatusIsOutside);
	unit()->currentSuborderDone(false);
	return;
 }

 switch (movingInStatus()) {
	case StatusIsOutside:
	{
		advanceIsOutside(advanceCallsCount);
		break;
	}
	case StatusMovingToEnterPoint1:
	{
		advanceMovingToEnterPoint1(advanceCallsCount);
		break;
	}
	case StatusWaitForEnterPermission:
	{
		advanceWaitForEnterPermission(advanceCallsCount);
		break;
	}
	case StatusMovingToEnterPoint2:
	{
		advanceMovingToEnterPoint2(advanceCallsCount);
		break;
	}
	case StatusMovingToStoragePosition:
	{
		advanceMovingToStoragePosition(advanceCallsCount);
		break;
	}
	case StatusHasEntered:
	{
		advanceHasEntered(advanceCallsCount);
		break;
	}
	case StatusWaitForLeavePermission:
	{
		advanceWaitForLeavePermission(advanceCallsCount);
		break;
	}
	case StatusMovingToOutside:
	{
		advanceMovingToOutside(advanceCallsCount);
		break;
	}
	case StatusHasLeft:
	{
		boError() << k_funcinfo << "StatusHasLeft must not be reached (is a dummy state)" << endl;
		changeStatus(StatusIsOutside);
		break;
	}
	default:
	{
		boError() << k_funcinfo << "invalid status " << (int)movingInStatus() << endl;
		changeStatus(StatusIsOutside);
		break;
	}

 }
}


void EnterUnitPlugin::advanceIsOutside(unsigned int)
{
 boDebug() << k_funcinfo << "id: " << unit()->id() << endl;
 if (!unit()->isMobile()) {
	boError() << k_funcinfo << "only mobile units can enter other units" << endl;
	unit()->currentSuborderDone(false);
	return;
 }
 if (!mUnitStoragePlugin) {
	BO_NULL_ERROR(mUnitStoragePlugin);
	unit()->currentSuborderDone(false);
	return;
 }

 if (!unit()->currentOrder()) {
	boWarning() << k_funcinfo << "no order active" << endl;
	return;
 }
 if (unit()->currentOrder()->type() != UnitOrder::EnterUnit) {
	boWarning() << k_funcinfo << "unexpected current order type: " << unit()->currentOrder()->type() << endl;
	return;
 }
 UnitEnterUnitOrder* o = (UnitEnterUnitOrder*)unit()->currentOrder();
 if (o->isLeaveOrder()) {
	// should never be reached.
	// -> changeStatus() should have completed the order.
	unit()->currentSuborderDone(true);
 } else {
	// should never be reached.
	// -> enter() should have changed the status.
	boWarning() << k_funcinfo << "Enter order: still in status IsOutside. aborting Enter order." << endl;
	unit()->currentSuborderDone(false);
 }
}

// moves to enterpoint 1
// -> rotation is ignored here
void EnterUnitPlugin::advanceMovingToEnterPoint1(unsigned int)
{
 if (!mUnitStoragePlugin) {
	BO_NULL_ERROR(mUnitStoragePlugin);
	unit()->currentSuborderDone(false);
	changeStatus(StatusIsOutside);
	return;
 }

 boDebug() << k_funcinfo << endl;
 if (isAtEnterPoint1()) {
	boDebug() << k_funcinfo << "arrived at enter point 1" << endl;
	changeStatus(StatusWaitForEnterPermission);
	return;
 }

 if (mTriedMovingCounter > 10) {
	boDebug() << k_funcinfo << "could not reach enter point 1" << endl;
	changeStatus(StatusIsOutside);
	unit()->currentSuborderDone(false);
	return;
 }

 mTriedMovingCounter = mTriedMovingCounter + 1;
 if (!unit()->addCurrentSuborder(new UnitMoveInsideUnitOrder(mEnterPoint1))) {
	boDebug() << k_funcinfo << "could not move to enter point 1. abort entering." << endl;
	changeStatus(StatusIsOutside);
	unit()->currentSuborderDone(false);
	return;
 }
}

void EnterUnitPlugin::advanceWaitForEnterPermission(unsigned int)
{
 if (!mUnitStoragePlugin) {
	BO_NULL_ERROR(mUnitStoragePlugin);
	unit()->currentSuborderDone(false);
	changeStatus(StatusIsOutside);
	return;
 }
 bool wait = false;
 bool requestApproved = false;
 mUnitStoragePlugin->getEnterPermissionResult(unit(), &wait, &requestApproved);
 boDebug() << k_funcinfo << "enter permission result: wait=" << wait << " approved=" << requestApproved << endl;
 if (wait) {
	bool canStopMoving = true;
	if (unitProperties()->isAircraft() && !unitProperties()->isHelicopter()) {
		canStopMoving = false;
	}

	if (!canStopMoving) {
		boDebug() << k_funcinfo << "unit is required to wait for enter permission, but is not allowed to stop moving! abort entering" << endl;
		unit()->currentSuborderDone(false);
		return;
	}
	return;
 } else {
	if (requestApproved) {
		changeStatus(StatusMovingToEnterPoint2);
	} else {
		changeStatus(StatusIsOutside);
		unit()->currentSuborderDone(false);
	}
 }
}

void EnterUnitPlugin::advanceMovingToEnterPoint2(unsigned int)
{
 if (!mUnitStoragePlugin) {
	BO_NULL_ERROR(mUnitStoragePlugin);
	unit()->currentSuborderDone(false);
	changeStatus(StatusIsOutside);
	return;
 }

 boDebug() << k_funcinfo << endl;
 if (isAtEnterPoint2()) {
	boDebug() << "arrived at enter point 2" << endl;
	changeStatus(StatusMovingToStoragePosition);
	return;
 }

 if (mTriedMovingCounter > 10) {
	boDebug() << k_funcinfo << "could not reach enter point 2" << endl;
	changeStatus(StatusIsOutside);
	unit()->currentSuborderDone(false);
	return;
 }

 mTriedMovingCounter = mTriedMovingCounter + 1;
 if (!unit()->addCurrentSuborder(new UnitMoveInsideUnitOrder(mEnterPoint2))) {
	boDebug() << k_funcinfo << "could not move to enter point 2. abort entering." << endl;
	changeStatus(StatusIsOutside);
	unit()->currentSuborderDone(false);
	return;
 }
}

void EnterUnitPlugin::advanceMovingToStoragePosition(unsigned int)
{
 if (!mUnitStoragePlugin) {
	BO_NULL_ERROR(mUnitStoragePlugin);
	return;
 }
 boDebug() << k_funcinfo << endl;

 if (mRemainingInsidePath.isEmpty()) {
	boDebug() << k_funcinfo << "at inside destination" << endl;
	changeStatus(StatusHasEntered);
	return;
 }

 boDebug() << k_funcinfo
	<< "not yet at inside destination: unit is at "
	<< debugStringVector(BoVector2Fixed(unit()->centerX(), unit()->centerY()))
	<< " remaining path length "
	<< mRemainingInsidePath.count()
	<< endl;

 // TODO: make sure that moving inside a unit is _always_ successful, i.e. we
 // can even move through obstacles if necessary
 // -> this is required to have the game remain in a valid state at all times.
 //    otherwise we can NEVER fix the problem that occured here: we are already
 //    inside the unit, so "normal" pathfinding will probably fail, as this unit
 //    is colliding with the storing unit
 bool ok = unit()->addCurrentSuborder(new UnitMoveInsideUnitOrder(mRemainingInsidePath[mRemainingInsidePath.count() - 1]));
 if (!ok) {
	// umm ok, so what now?
	// -> we can't stop moving in (--> unit unusable and an obstacle)
	// -> we can't move out either, as we need to move inside first
	//    for now we just do nothing and will try again
	boWarning() << k_funcinfo << "could not move inside?!" << endl;
 } else {
	boDebug() << k_funcinfo
		<< unit()->id() << " started moving inside... unit is at "
		<< debugStringVector(BoVector2Fixed(unit()->centerX(), unit()->centerY()))
		<< " remaining path length "
		<< mRemainingInsidePath.count()
		<< endl;
 }
}

void EnterUnitPlugin::advanceHasEntered(unsigned int)
{
 if (!mUnitStoragePlugin) {
	BO_NULL_ERROR(mUnitStoragePlugin);
	return;
 }
 // should never be reached: changeStatus() should have finalized the order
 boError() << k_funcinfo << "(should not be reached!)" << endl;

 changeStatus(StatusHasEntered);
}

void EnterUnitPlugin::advanceWaitForLeavePermission(unsigned int)
{
 if (!mUnitStoragePlugin) {
	BO_NULL_ERROR(mUnitStoragePlugin);
	unit()->currentSuborderDone(false);
	changeStatus(StatusHasLeft);
	return;
 }

 bool wait = false;
 bool requestApproved = false;
 mUnitStoragePlugin->getLeavePermissionResult(unit(), &wait, &requestApproved);
 if (wait) {
	// AB: in contrast to advanceWaitForEnterPermission(), unit does not
	//     have to move here, even if unit is an airplane.
	//     so we can safely just return here.
	return;
 } else {
	if (requestApproved) {
		changeStatus(StatusMovingToOutside);
	} else {
		changeStatus(StatusHasEntered);
		unit()->currentSuborderDone(false);
	}
 }
}

void EnterUnitPlugin::advanceMovingToOutside(unsigned int)
{
 if (!mUnitStoragePlugin) {
	BO_NULL_ERROR(mUnitStoragePlugin);
	unit()->currentSuborderDone(false);
	changeStatus(StatusHasLeft);
	return;
 }

 if (mRemainingInsidePath.isEmpty()) {
	boDebug() << k_funcinfo << "at outside destination" << endl;
	changeStatus(StatusHasLeft);
	return;
 }

 boDebug() << k_funcinfo
	<< "not yet at outside destination: unit is at "
	<< debugStringVector(BoVector2Fixed(unit()->centerX(), unit()->centerY()))
	<< " remaining path length "
	<< mRemainingInsidePath.count()
	<< endl;


 bool ok = unit()->addCurrentSuborder(new UnitMoveInsideUnitOrder(mRemainingInsidePath[mRemainingInsidePath.count() - 1]));
 if (!ok) {
	// umm ok, so what now?
	// -> we can't stop moving in (--> unit unusable and an obstacle)
	// -> we can't move back in either, as we need to move outside first
	//    for now we just do nothing and will try again
	boWarning() << k_funcinfo << "could not move to outside?!" << endl;
 } else {
	boDebug() << k_funcinfo
		<< unit()->id() << " started moving to outside... unit is at "
		<< debugStringVector(BoVector2Fixed(unit()->centerX(), unit()->centerY()))
		<< " remaining path length "
		<< mRemainingInsidePath.count()
		<< endl;
 }
}

bool EnterUnitPlugin::requestEnterPath()
{
 if (unit()->isInsideUnit()) {
	return false;
 }
 if (movingInStatus() != StatusMovingToEnterPoint1) {
	return false;
 }
 // AB: mRemainingInsidePath must be empty at this point. if it is not,
 //     it doesn't harm to clear it, since the unit is still outside!
 mRemainingInsidePath.clear();

 BoVector2Fixed point1;
 BoVector2Fixed point2;
 QValueList<BoVector2Fixed> path;
 unsigned int pathIndex = 0;
 bool ok = mUnitStoragePlugin->getEnterPathFor(unit(), &point1, &point2, &path, &pathIndex);
 if (!ok) {
	boDebug() << k_funcinfo << "no enter position received" << endl;
	return false;
 }
 mPathIndex = pathIndex;
 mEnterPoint1 = point1;
 mEnterPoint2 = point2;
 for (QValueList<BoVector2Fixed>::iterator it = path.begin(); it != path.end(); ++it) {
	mRemainingInsidePath.append(*it);
 }

 boDebug() << k_funcinfo << "enter point 1: " << debugStringVector(point1, 4) << endl;
 boDebug() << k_funcinfo << "enter point 2: " << debugStringVector(point2, 4) << endl;
 boDebug() << k_funcinfo << "inside path length: " << mRemainingInsidePath.count() << endl;

 return true;
}


bool EnterUnitPlugin::requestLeavePath()
{
 if (!unit()->isInsideUnit()) {
	return false;
 }
 if (movingInStatus() != StatusMovingToOutside) {
	return false;
 }
 // AB: mRemainingInsidePath must be empty at this point. if it is not,
 //     it doesn't harm to clear it, since the unit is still _standing_ inside!
 mRemainingInsidePath.clear();

 QValueList<BoVector2Fixed> path;
 bool ok = mUnitStoragePlugin->getLeavePathFor(unit(), &path);
 if (!ok) {
	boDebug() << k_funcinfo << "no leave path received" << endl;
	return false;
 }
 for (QValueList<BoVector2Fixed>::iterator it = path.begin(); it != path.end(); ++it) {
	mRemainingInsidePath.append(*it);
 }

 return true;
}

bool EnterUnitPlugin::isAtEnterPoint1() const
{
 BoRect2Fixed r = unit()->boundingRect();
 // check for "nearby" only, i.e. make the rect a little more fuzzy
 const bofixed diffW = 0.5;
 const bofixed diffH = 0.5;
 r.set(r.left() - diffW, r.top() - diffH ,
		r.right() + diffW, r.bottom() + diffH);
 return r.contains(mEnterPoint1.value());
}

bool EnterUnitPlugin::isAtEnterPoint2() const
{
 BoRect2Fixed r = unit()->boundingRect();
 // check for "nearby" only, i.e. make the rect a little more fuzzy
 const bofixed diffW = 0.5;
 const bofixed diffH = 0.5;
 r.set(r.left() - diffW, r.top() - diffH ,
		r.right() + diffW, r.bottom() + diffH);
 return r.contains(mEnterPoint2.value());
}

QValueList<BoVector2Fixed> EnterUnitPlugin::remainingInsidePath() const
{
 return mRemainingInsidePath;
}

void EnterUnitPlugin::pathPointDone()
{
 boDebug() << k_funcinfo << endl;
 if (mRemainingInsidePath.isEmpty()) {
	boError() << k_funcinfo << "no pathpoint" << endl;
	return;
 }
 mRemainingInsidePath.pop_front();
}

// TODO:
// 1. what about rotated unitstorages?
// 2. leaving: move units blocking the exit (kinda done: normal pathfinder
//    handles the problem by "going trough" blocking units
//    -> actually this TODO is still valid, but MUCH less important now)
// 2.1 wait with leaving, if exit is blocked (kind done: see above)
//    -> actually this TODO is still valid, but MUCH less important now)



/*
 * Moving inside a unit must NEVER fail! If by some reason it does fail, the
 * unit must assume that it worked correctly. Therefore we must never check for
 * whether we reached the desired location - once the path is empty, we must 
 * assume we are at the destination.
 * Adding proper error handling to "moving inside unit", would be horribly
 * complicated (we would need essentially a complete pathfinding architecture)
 * and thus this simplification is required for us.
 * -> as a consequence stopMoving() must NEVER be called in certain states!
 *    -> in particular StatusMovingToStoragePosition and StatusMovingToOutside
 */

