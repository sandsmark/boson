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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "unitstorageplugin.h"

#include "../../bomemory/bodummymemory.h"
#include "enterunitplugin.h"
#include "unit.h"
#include "unitproperties.h"
#include "pluginproperties.h"
#include "bo3dtools.h"
#include "bodebug.h"

#include <kgame/kgamepropertylist.h>
#include <kgame/kgamepropertyarray.h>

#include <qdom.h>

class UnitStoragePluginPrivate
{
public:
	UnitStoragePluginPrivate()
	{
	}

	KGameProperty<Q_INT32> mStoringStatus;

	KGamePropertyList<Q_UINT32> mPendingEnterRequests;
	KGamePropertyList<Q_UINT32> mApprovedEnterRequests;
	KGamePropertyList<Q_UINT32> mPendingLeaveRequests;
	KGamePropertyList<Q_UINT32> mApprovedLeaveRequests;

	KGamePropertyArray<Q_UINT32> mEnteringUnitOnPath;
};

UnitStoragePlugin::UnitStoragePlugin(Unit* owner)
	: UnitPlugin(owner)
{
 d = new UnitStoragePluginPrivate;
 owner->registerData(&d->mStoringStatus, Unit::IdStoringStatus);
 owner->registerData(&d->mPendingEnterRequests, Unit::IdPendingEnterRequests);
 owner->registerData(&d->mApprovedEnterRequests, Unit::IdApprovedEnterRequests);
 owner->registerData(&d->mPendingLeaveRequests, Unit::IdPendingLeaveRequests);
 owner->registerData(&d->mApprovedLeaveRequests, Unit::IdApprovedLeaveRequests);
 owner->registerData(&d->mEnteringUnitOnPath, Unit::IdEnteringUnitOnPath);

 d->mStoringStatus = StatusDoorsClosed;

 UnitStorageProperties* prop = (UnitStorageProperties*)properties(PluginProperties::UnitStorage);
 d->mEnteringUnitOnPath.resize(prop->enterPathCount());
 for (unsigned int i = 0; i < prop->enterPathCount(); i++) {
	d->mEnteringUnitOnPath[i] = 0;
 }
}

UnitStoragePlugin::~UnitStoragePlugin()
{
 delete d;
}

bool UnitStoragePlugin::saveAsXML(QDomElement& root) const
{
 Q_UNUSED(root);
 return true;
}

bool UnitStoragePlugin::loadFromXML(const QDomElement& root)
{
 Q_UNUSED(root);
 return true;
}

void UnitStoragePlugin::unitDestroyed(Unit* unit)
{
 removeFromAllLists(unit);
}

void UnitStoragePlugin::itemRemoved(BosonItem* item)
{
 Q_UNUSED(item);
 if (RTTI::isUnit(item->rtti())) {
	removeFromAllLists((Unit*)item);
 }
}

UnitStoragePlugin::StorageStatus UnitStoragePlugin::storageStatus() const
{
 return (StorageStatus)d->mStoringStatus.value();
}

void UnitStoragePlugin::advance(unsigned int advanceCallsCount)
{
 switch (storageStatus()) {
	case StatusDoorsClosed:
		advanceDoorsClosed(advanceCallsCount);
		break;
	case StatusDoorsOpening:
		advanceDoorsOpening(advanceCallsCount);
		break;
	case StatusDoorsClosing:
		advanceDoorsClosing(advanceCallsCount);
		break;
	case StatusUnitCanEnterOrLeave:
		advanceUnitCanEnterOrLeave(advanceCallsCount);
		break;
	case StatusUnitEnteringOrLeaving:
		advanceUnitEnteringOrLeaving(advanceCallsCount);
		break;
	default:
		boError() << k_funcinfo << "unexpected state " << (int)storageStatus() << endl;
		changeStatus(StatusDoorsClosing);
		break;
 }
}

bool UnitStoragePlugin::canStore(const Unit* u) const
{
 if (!u) {
	return false;
 }
 const UnitStorageProperties * prop = (UnitStorageProperties*)unit()->properties(PluginProperties::UnitStorage);
 if (!prop) {
	BO_NULL_ERROR(prop);
	return false;
 }

 int pathType = pathTypeForUnit(u);

 for (unsigned int i = 0; i < prop->enterPathCount(); i++) {
	if (prop->enterPathUnitType(i) == pathType) {
		return true;
	}
 }
 return false;
}

bool UnitStoragePlugin::haveDoors() const
{
 // TODO
 return false;
}

BoVector2Fixed UnitStoragePlugin::getPathPointPos(float factorX, float factorY) const
{
 BoVector2Fixed center = BoVector2Fixed(unit()->centerX(), unit()->centerY());
 if (factorX < 0.0f || factorX > 1.0f) {
	return center;
 }
 if (factorY < 0.0f || factorY > 1.0f) {
	return center;
 }

 // AB: we need BoVector2 only, but BoMatrix requires BoVector3
 float x = -unit()->width() / 2 + factorX * unit()->width();
 float y = -unit()->height() / 2 + factorY * unit()->height();
 BoVector3Float untransformedPos(x, y, 0.0f);
 BoVector3Float pos;

 BoMatrix rot;
 rot.rotate(-unit()->rotation(), 0.0f, 0.0f, 1.0f);
 rot.transform(&pos, &untransformedPos);

 BoVector2Fixed p = center + BoVector2Fixed(pos.x(), pos.y());

/*
 boDebug() << k_funcinfo
		<< "center: " << debugStringVector(center)
		<< " pos: " << debugStringVector(pos)
		<< " p: " << debugStringVector(p)
		<< " factorX: " << factorX << " factorY: " << factorY
		<< " x: " << x << " y: " << y
		<< " rotation: " << unit()->rotation()
		<< endl;
*/
 return p;
}

BoVector2Fixed UnitStoragePlugin::getOutsidePos1(const BoVector2Fixed& inside, const BoVector2Float& _direction) const
{
 BoVector2Float direction = _direction;
 direction.normalize();

 // TODO
 // -> this should return a point on a cell as close to "inside" as possible
 //    (yet far enough away, so that an entering unit could actually move there)

 BoVector2Float add;
 add = direction * -1.0f;

 boDebug() << k_funcinfo << direction.x() << " " << direction.y() << endl;

 return inside + add.toFixed();
}

BoVector2Fixed UnitStoragePlugin::getOutsidePos2(const BoVector2Fixed& inside, const BoVector2Float& _direction) const
{
 return getOutsidePos1(inside, _direction);
}

QValueList<BoVector2Fixed> UnitStoragePlugin::getAbsoluteFromRelativePath(const QValueList<BoVector2Float>& relative) const
{
 QValueList<BoVector2Fixed> path;
 const UnitStorageProperties * prop = (UnitStorageProperties*)unit()->properties(PluginProperties::UnitStorage);
 if (!prop) {
	BO_NULL_ERROR(prop);
	return path;
 }

 for (QValueList<BoVector2Float>::const_iterator it = relative.begin(); it != relative.end(); ++it) {
	path.append(getPathPointPos((*it).x(), (*it).y()));
 }

 return path;
}

int UnitStoragePlugin::pathTypeForUnit(const Unit* unit) const
{
 if (!unit || !unit->isMobile()) {
	boError() << k_funcinfo << "invalid unit parameter" << endl;
	return UnitStorageProperties::PathTypeLand;
 }
 if (unit->unitProperties()->isAircraft()) {
	if (unit->unitProperties()->isHelicopter()) {
		return UnitStorageProperties::PathTypeHelicopter;
	} else {
		return UnitStorageProperties::PathTypePlane;
	}
 }
 if (unit->unitProperties()->isShip()) {
	return UnitStorageProperties::PathTypeShip;
 }
 if (unit->unitProperties()->isLand()) {
	return UnitStorageProperties::PathTypeLand;
 }
 boError() << k_funcinfo << "cannot figure out proper type of unit " << unit->id() << endl;
 return UnitStorageProperties::PathTypeLand;
}

bool UnitStoragePlugin::getEnterPathFor(const Unit* enteringUnit, BoVector2Fixed* enterPosOutside1, BoVector2Fixed* enterPosOutside2, QValueList<BoVector2Fixed>* enterPathInside, unsigned int* pathIndex)
{
 const UnitStorageProperties * prop = (UnitStorageProperties*)unit()->properties(PluginProperties::UnitStorage);
 if (!prop) {
	BO_NULL_ERROR(prop);
	return false;
 }

 if (!enteringUnit->isMobile()) {
	boDebug() << k_funcinfo << "only mobile units can enter units" << endl;
	return false;
 }
 UnitStorageProperties::PathUnitType type = (UnitStorageProperties::PathUnitType)pathTypeForUnit(enteringUnit);

 *pathIndex = 0;

 for (unsigned int i = 0; i < prop->enterPathCount(); i++) {
	if (pathIsTaken(i)) {
		continue;
	}
	if (prop->enterPathUnitType(i) != type) {
		continue;
	}
	QValueList<BoVector2Float> relativePath = prop->enterPath(i);
	if (relativePath.count() == 0) {
		boError() << k_funcinfo << "path " << i << " is invalid!" << endl;
		continue;
	}
	BoVector2Float direction = prop->enterDirection(i);

	*pathIndex = i;
	*enterPathInside = getAbsoluteFromRelativePath(relativePath);
	*enterPosOutside1 = getOutsidePos1((*enterPathInside)[0], direction);
	*enterPosOutside2 = getOutsidePos2((*enterPathInside)[0], direction);

	return true;
 }
 return false;
}

bool UnitStoragePlugin::getLeavePathFor(const Unit* leavingUnit, QValueList<BoVector2Fixed>* leavePath)
{
 Q_UNUSED(leavingUnit);
 Q_UNUSED(leavePath);

 const UnitStorageProperties * prop = (UnitStorageProperties*)unit()->properties(PluginProperties::UnitStorage);
 if (!prop) {
	BO_NULL_ERROR(prop);
	return false;
 }

 if (!leavingUnit->isMobile()) {
	boDebug() << k_funcinfo << "only mobile units can enter units" << endl;
	return false;
 }
 UnitStorageProperties::PathUnitType type = (UnitStorageProperties::PathUnitType)pathTypeForUnit(leavingUnit);

 unsigned int path = d->mEnteringUnitOnPath.size();
 for (unsigned int i = 0; i < d->mEnteringUnitOnPath.size(); i++) {
	if (d->mEnteringUnitOnPath[i] == leavingUnit->id()) {
		path = i;
	}
 }
 if (path >= d->mEnteringUnitOnPath.size()) {
	boDebug() << k_funcinfo << "unit " << leavingUnit->id() << " not stored in this storage?!" << endl;
	return false;
 }

 if (prop->enterPathUnitType(path) != type) {
	boDebug() << k_funcinfo << "unit " << leavingUnit->id() << " cannot use path " << path << endl;
	return false;
 }
 QValueList<BoVector2Float> relativePath = prop->leavePathForEnterPath(path);
 if (relativePath.count() == 0) {
	boError() << k_funcinfo << "leave path " << path << " is invalid!" << endl;
	return false;
 }
 BoVector2Float direction = prop->enterDirection(path);
// direction = direction * -1.0;

 *leavePath = getAbsoluteFromRelativePath(relativePath);
 BoVector2Fixed outside = getOutsidePos2((*leavePath)[leavePath->count() - 1], direction);

 leavePath->append(outside);

 return true;
}

bool UnitStoragePlugin::pathIsTaken(unsigned int i) const
{
 if (d->mEnteringUnitOnPath[i] != 0) {
	return true;
 }
 return false;
}

unsigned int UnitStoragePlugin::freePathCount() const
{
 unsigned int count = 0;
 for (unsigned int i = 0; i < d->mEnteringUnitOnPath.size(); i++) {
	if (d->mEnteringUnitOnPath[i] == 0) {
		count++;
	}
 }
 return count;
}


bool UnitStoragePlugin::takeMe(Unit* enteringUnit, unsigned int path)
{
 boDebug() << k_funcinfo << this->unit()->id() << " is asked to take " << enteringUnit->id() << endl;
 if (!canStore(enteringUnit)) {
	boDebug() << k_funcinfo << "can't store " << enteringUnit->id() << endl;
	return false;
 }

 if (enteringUnit->isInsideUnit()) {
	boDebug() << k_funcinfo << "unit "  << enteringUnit->id() << " already is inside a unit" << endl;
	return false;
 }
 if (enteringUnit->advanceWork() != Unit::WorkPlugin) {
	boError() << k_funcinfo << "unit must be in WorkPlugin to be taken! work=" << enteringUnit->advanceWork() << endl;
	return false;
 }
 EnterUnitPlugin* enterPlugin = (EnterUnitPlugin*)enteringUnit->plugin(UnitPlugin::EnterUnit);
 if (!enterPlugin) {
	return false;
 }
 if (enterPlugin->storingUnit() != this) {
	boError() << k_funcinfo << "storing unit is != this" << endl;
	return false;
 }
 if (enterPlugin->movingInStatus() != EnterUnitPlugin::StatusMovingToStoragePosition) {
	boError() << k_funcinfo << "entering unit must be in StatusMovingToStoragePosition. movingInStatus(): " << enterPlugin->movingInStatus() << endl;
	return false;
 }

 bool waitForPermission = false;
 bool permission = false;
 getEnterPermissionResult(enteringUnit, &waitForPermission, &permission);
 if (waitForPermission || !permission) {
	return false;
 }

 if (path >= d->mEnteringUnitOnPath.count()) {
	return false;
 }
 if (d->mEnteringUnitOnPath[path] != 0) {
	boDebug() << k_funcinfo << "already a unit on path " << path << endl;
	return false;
 }

 boDebug() << k_funcinfo << this->unit()->id() << " is taking " << enteringUnit->id() << endl;

 d->mEnteringUnitOnPath[path] = enteringUnit->id();

 changeStatus(StatusUnitEnteringOrLeaving);

 return true;
}

void UnitStoragePlugin::releaseMe(Unit* leavingUnit)
{
 boDebug() << k_funcinfo << leavingUnit->id() << endl;

 // AB: note: we _always_ remove the leavingUnit from the "is taken" list, even
 //     if an error occurred (e.g. if leavingUnit is in the wrong state or so).
 //
 //     this is important, as the "is taken" list must (in any case) not contain
 //     units that are not stored here anymore.

 removeFromAllLists(leavingUnit);
}

void UnitStoragePlugin::arrivedAtStoragePosition(Unit* enteringUnit)
{
 // AB: this method can either be called when the unit completed entering,
 //     or if leaving was aborted/denied.

 boDebug() << k_funcinfo << "unit: " << enteringUnit->id() << endl;
 d->mPendingEnterRequests.remove(enteringUnit->id());
 d->mApprovedEnterRequests.remove(enteringUnit->id());
 d->mPendingLeaveRequests.remove(enteringUnit->id());
 d->mApprovedLeaveRequests.remove(enteringUnit->id());

// boDebug() << k_funcinfo << requestsCount() << " status=" << (int)storageStatus() << endl;
// boDebug() << k_funcinfo << unit()->advanceWork() << endl;
}

void UnitStoragePlugin::requestEnterPermission(const Unit* enteringUnit)
{
 if (!enteringUnit) {
	return;
 }
 boDebug() << k_funcinfo << "requests permission: " << enteringUnit->id() << endl;

 if (!canStore(enteringUnit)) {
	boDebug() << k_funcinfo << "cannot store " << enteringUnit->id() << endl;
	return;
 }

 removeFromAllLists(enteringUnit);

 d->mPendingEnterRequests.append(enteringUnit->id());

 // aggressive implementation: try to approve as fast as possible
 // -> flying units can't easily wait in the air
 if (StorageStatus() == StatusDoorsClosed) {
	changeStatus(StatusDoorsOpening);
 }
}

void UnitStoragePlugin::revokeEnterRequest(const Unit* enteringUnit)
{
 if (!enteringUnit) {
	return;
 }
 d->mPendingEnterRequests.remove(enteringUnit->id());
 d->mApprovedEnterRequests.remove(enteringUnit->id());
 boDebug() << k_funcinfo << enteringUnit->id() << " revoked enter permission. remaining requests: " << d->mPendingEnterRequests.count() + d->mApprovedEnterRequests.count() << endl;
}

void UnitStoragePlugin::getEnterPermissionResult(const Unit* enteringUnit, bool* wait, bool* permission)
{
 *wait = false;
 *permission = false;
 if (!enteringUnit) {
	return;
 }

 if (d->mPendingEnterRequests.contains(enteringUnit->id())) {
	*wait = true;
 } else if (d->mApprovedEnterRequests.contains(enteringUnit->id())) {
	*wait = false;
	*permission = true;
 }

 if (*permission) {
	if (storageStatus() != StatusUnitCanEnterOrLeave) {
		*wait = true;
	}
 }
}

void UnitStoragePlugin::requestLeavePermission(const Unit* leavingUnit)
{
 if (!leavingUnit) {
	return;
 }

 d->mApprovedLeaveRequests.remove(leavingUnit->id());
 d->mPendingLeaveRequests.remove(leavingUnit->id());

 d->mPendingLeaveRequests.append(leavingUnit->id());
}

void UnitStoragePlugin::revokeLeaveRequest(const Unit* leavingUnit)
{
 if (!leavingUnit) {
	return;
 }
 d->mPendingLeaveRequests.remove(leavingUnit->id());
 d->mApprovedLeaveRequests.remove(leavingUnit->id());
}

void UnitStoragePlugin::getLeavePermissionResult(const Unit* leavingUnit, bool* wait, bool* permission)
{
 *wait = false;
 *permission = false;
 if (!leavingUnit) {
	return;
 }

 if (d->mPendingLeaveRequests.contains(leavingUnit->id())) {
	*wait = true;
	return;
 }
 if (d->mApprovedLeaveRequests.contains(leavingUnit->id())) {
	*wait = false;
	*permission = true;
	return;
 }
}

void UnitStoragePlugin::removeFromAllLists(const Unit* unit)
{
 if (!unit) {
	return;
 }

 d->mPendingEnterRequests.remove(unit->id());
 d->mApprovedEnterRequests.remove(unit->id());
 d->mPendingLeaveRequests.remove(unit->id());
 d->mApprovedLeaveRequests.remove(unit->id());

 for (unsigned int i = 0; i < d->mEnteringUnitOnPath.size(); i++) {
	if (d->mEnteringUnitOnPath[i] == unit->id()) {
		d->mEnteringUnitOnPath[i] = 0;
	}
 }
}

void UnitStoragePlugin::approveOneRequest()
{
 if (storageStatus() != StatusUnitCanEnterOrLeave) {
	// only in this state we can approve a request
	return;
 }

 // AB: at most 1 request can be approved at any time
 if (approvedRequestsCount() > 0) {
	return;
 }

 if (d->mPendingLeaveRequests.count() > 0) {
	unsigned long int unitId = d->mPendingLeaveRequests.first();
	d->mPendingLeaveRequests.remove(unitId);
	d->mApprovedLeaveRequests.append(unitId);

	boDebug() << k_funcinfo << "approving leave request for unit " << unitId << endl;

 } else if (d->mPendingEnterRequests.count() > 0) {
	bool capacityFull = false;
	if (freePathCount() == 0) {
		capacityFull = true;
	}

	if (capacityFull) {
		// deny ALL enter requests
		d->mPendingEnterRequests.clear();
		boDebug() << k_funcinfo << "capacity full. deny all enter requests" << endl;
	} else {
		unsigned long int unitId = d->mPendingEnterRequests.first();
		d->mPendingEnterRequests.remove(unitId);
		d->mApprovedEnterRequests.append(unitId);
		boDebug() << k_funcinfo << "approve enter request for unit " << unitId << endl;
	}
 }
}

void UnitStoragePlugin::changeStatus(StorageStatus status)
{
 d->mStoringStatus = (int)status;
 boDebug() << k_funcinfo << storageStatus() << endl;

 switch (storageStatus()) {
	case StatusDoorsClosed:
	{
		d->mApprovedEnterRequests.clear();
		d->mApprovedLeaveRequests.clear();
		break;
	}
	case StatusDoorsOpening:
	{
		d->mApprovedEnterRequests.clear();
		d->mApprovedLeaveRequests.clear();

		if (!haveDoors()) {
			// aggressive implementation: try to open doors as fast
			// as possible
			// -> flying units cannot wait in the air
			//
			// we do this only if this unit has no doors anyway, so
			// we can skip this whole phase.
			changeStatus(StatusUnitCanEnterOrLeave);
		}
		break;
	}
	case StatusDoorsClosing:
	{
		d->mApprovedEnterRequests.clear();
		d->mApprovedLeaveRequests.clear();
		break;
	}
	case StatusUnitCanEnterOrLeave:
	{
		approveOneRequest();
		break;
	}
	case StatusUnitEnteringOrLeaving:
	{
		break;
	}
	default:
	{
		boError() << k_funcinfo << "unexpected state " << (int) status << endl;
		changeStatus(StatusDoorsClosing);
		return;
	}
 }
}

unsigned int UnitStoragePlugin::approvedRequestsCount() const
{
 unsigned int approved = d->mApprovedEnterRequests.count() + d->mApprovedLeaveRequests.count();
 return approved;
}

unsigned int UnitStoragePlugin::requestsCount() const
{
 unsigned int pending = d->mPendingEnterRequests.count() + d->mPendingLeaveRequests.count();
 unsigned int approved = approvedRequestsCount();
 return pending + approved;
}

void UnitStoragePlugin::advanceDoorsClosed(unsigned int)
{
 if (requestsCount() > 0) {
	boDebug() << k_funcinfo << "have requests" << endl;
	changeStatus(StatusDoorsOpening);
	return;
 }
}

void UnitStoragePlugin::advanceDoorsOpening(unsigned int)
{
 bool doorsAreOpen = true;

 // TODO: actually open doors (if necessary)
 //       -> animation!

 if (doorsAreOpen) {
	boDebug() << k_funcinfo << "doors are open" << endl;
	changeStatus(StatusUnitCanEnterOrLeave);
	return;
 }

 if (requestsCount() == 0) {
	changeStatus(StatusDoorsClosing);
	return;
 }
}

void UnitStoragePlugin::advanceDoorsClosing(unsigned int)
{
 bool doorsAreClosed = true;

 // TODO: actually close doors (if necessary)
 //       -> animation!

 if (doorsAreClosed) {
	boDebug() << k_funcinfo << "doors are closed" << endl;
	changeStatus(StatusDoorsClosed);
	return;
 }
}

void UnitStoragePlugin::advanceUnitCanEnterOrLeave(unsigned int)
{
 if (requestsCount() == 0) {
	boDebug() << k_funcinfo << "no requests left. closing doors." << endl;
	changeStatus(StatusDoorsClosing);
 } else {
	approveOneRequest();

	// TODO:
	// timeout for approved request - deny it after some time (so that some
	// other unit can be allowed to enter)
 }
}

void UnitStoragePlugin::advanceUnitEnteringOrLeaving(unsigned int)
{
 if (requestsCount() == 0) {
	changeStatus(StatusDoorsClosing);
	return;
 }

 if (approvedRequestsCount() == 0) {
	changeStatus(StatusUnitCanEnterOrLeave);
	return;
 }
}

// TODO:
// 1. doors support (mainly animations and a certain delay in the gameengine)
// 2. timeout for approved requests
// 3. write a bounit interface for writing EnterPaths (UnitStorageProperties)

