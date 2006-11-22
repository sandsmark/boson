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
#ifndef UNITSTORAGEPLUGIN_H
#define UNITSTORAGEPLUGIN_H

#include "unitplugin.h"

template<class T> class BoVector2;
template<class T> class BoRect2;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoVector2<float> BoVector2Float;
typedef BoRect2<bofixed> BoRect2Fixed;
class EnterUnitPlugin;

class QDomElement;

class UnitStoragePluginPrivate;
/**
 * @short Plugin that allows other units to enter this unit
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class UnitStoragePlugin : public UnitPlugin
{
public:
	enum StorageStatus {
		StatusDoorsClosed = 0,
		StatusDoorsOpening = 1,
		StatusDoorsClosing = 2,
		StatusUnitCanEnterOrLeave = 3,
		StatusUnitEnteringOrLeaving = 4
	};

public:
	UnitStoragePlugin(Unit* unit);
	~UnitStoragePlugin();

	virtual int pluginType() const { return UnitPlugin::UnitStorage; }

	virtual void advance(unsigned int advanceCallsCount);

	virtual bool saveAsXML(QDomElement& root) const;
	virtual bool loadFromXML(const QDomElement& root);

	virtual void unitDestroyed(Unit*);
	virtual void itemRemoved(BosonItem*);

	StorageStatus storageStatus() const;

	/**
	 * @return TRUE if this unit has some kind of "doors", i.e. something
	 * that must be "opened" or "switched on" before a unit can enter. FALSE
	 * if no such thing exists here.
	 **/
	bool haveDoors() const;

	/**
	 * @return The total number of enter/leave requests. Both, pending and
	 * approved requests are counted.
	 **/
	unsigned int requestsCount() const;
	unsigned int approvedRequestsCount() const;

	/**
	 * @return The @ref UnitStorageProperties::PathType for the @p unit
	 **/
	int pathTypeForUnit(const Unit* unit) const;

	/**
	 * @return TRUE if @p unit can be stored by this plugin. Otherwise
	 * FALSE. This means this plugin has a path that supports @p unit, see
	 * @ref pathTypeForUnit and @ref UnitStorageProperties::pathUnitType.
	 *
	 * Note that a storage that CAN store @p unit, still may deny an enter
	 * request, e.g. because it has no capacity left.
	 **/
	bool canStore(const Unit* unit) const;

	/**
	 * @param pathIndex The index of the returned path (see @ref
	 * UnitStorageProperties) is returned here. Undefined if FALSE is
	 * returned.
	 * @return TRUE if a way to enter this unit could be found for @p
	 * enteringUnit. Otherwise FALSE.
	 **/
	bool getEnterPathFor(const Unit* enteringUnit, BoVector2Fixed* enterPosOutside1, BoVector2Fixed* enterPosOutside2, QValueList<BoVector2Fixed>* insidePath, unsigned int* pathIndex);
	bool getLeavePathFor(const Unit* leavingUnit, QValueList<BoVector2Fixed>* insidePath);

	/**
	 * @p enteringUnit requests to enter this unit. This method stores the
	 * request and will eventually either approve or deny it.
	 *
	 * The request will be approved once the "doors are open", if this unit
	 * has some kind of doors. See also @ref getEnterPermissionResult.
	 **/
	void requestEnterPermission(const Unit* enteringUnit);
	void revokeEnterRequest(const Unit* enteringUnit);

	void requestLeavePermission(const Unit* enteringUnit);
	void revokeLeaveRequest(const Unit* enteringUnit);

	/**
	 * Returns the result of a previoud @ref requestEnterPermission request
	 * in the two parameters. A call to this method without requesting a
	 * permission first is valid and is equal to a denied request.
	 *
	 * @param wait TRUE if the entering unit has to wait for permission
	 * (e.g. the doors are still being opened). The result of the request is
	 * not yet known - @p permission is undefined. Otherwise FALSE: the
	 * result of the request is in the @p permission parameter.
	 * @param permission If @p wait is FALSE, this contains the result of
	 * the request: TRUE if the entering permission has been granted,
	 * otherwise FALSE. The value is undefined, if @p wait is TRUE.
	 **/
	void getEnterPermissionResult(const Unit* enteringUnit, bool* wait, bool* permission);
	void getLeavePermissionResult(const Unit* enteringUnit, bool* wait, bool* permission);

	/**
	 * Called by @p enteringUnit as soon as it actually enters this unit,
	 * i.e. right before (or right after - not strictly defined!) @ref
	 * Unit::setIsInsideUnit(true) is called.
	 *
	 * This method is meant to
	 * @li Do a final check if the @p enteringUnit can actually be taken
	 * right now
	 * @li Add the @p enteringUnit to the "is stored in this unit" list
	 *
	 * @return FALSE if @p enteringUnit can not currently be taken by this
	 * unit. The unit was NOT added to the "is taken" list. TRUE if @p
	 * enteringUnit has been taken and added to the "is taken" list.
	 **/
	bool takeMe(Unit* enteringUnit, unsigned int path);

	/**
	 * This is the counterpart of @ref takeMe. This method removes the @p
	 * leavingUnit from the "is taken" list.
	 *
	 * The method always removes the @p leavingUnit from the "is taken"
	 * list, it does not check whether the unit is in the correct (i.e.
	 * expected) state. Thus this method does not return anything - it is
	 * always successful.
	 **/
	void releaseMe(Unit* leavingUnit);

	void arrivedAtStoragePosition(Unit* enteringUnit);

protected:
	/**
	 * @return The map coordinates of pos @p factorX, @p factorY inside the
	 * unit.
	 **/
	BoVector2Fixed getPathPointPos(float factorX, float factorY) const;

	/**
	 * @return A position outside this unit. The returned point is close to
	 * @p pos (which is inside this unit) in direction @p direction. This is
	 * meant to return the outside "enter pos" when the inside "enter pos"
	 * is already known.
	 **/
	BoVector2Fixed getOutsidePos1(const BoVector2Fixed& pos, const BoVector2Float& direction) const;

	/**
	 * Like @ref getOutsidePos1, but returns a point right next to @p pos
	 * (which is inside this unit). I.e. the actual entering of this unit
	 * should start from the returned point of this method.
	 *
	 * Usually @ref getOutsidePos1 and @ref getOutsidePos2 return the same
	 * point. However for units that can't rotate while standing still
	 * (airplanes) the points may differ - the unit is meant to rotate to
	 * the correct direction while moving from @ref getOutsidePos1 to @ref
	 * getOutsidePos2.
	 **/
	BoVector2Fixed getOutsidePos2(const BoVector2Fixed& pos, const BoVector2Float& direction) const;

	QValueList<BoVector2Fixed> getAbsoluteFromRelativePath(const QValueList<BoVector2Float>& relative) const;

	/**
	 * @return TRUE if a unit is on path @p i, otherwise FALSE. At most one
	 * unit can be on a path at any time.
	 **/
	bool pathIsTaken(unsigned int i) const;

	/**
	 * @return The number of paths that are not taken. See @ref pathIsTaken
	 **/
	unsigned int freePathCount() const;

	/**
	 * This method tries to approve one request, if possible.
	 * Leave requests are always preferred.
	 *
	 * A request can only be approved if several things are true:
	 * @li There is at least one request available (either to leave or to 
	 * enter)
	 * @li There is no other request approved atm (only one request can be
	 * approved at any time!)
	 * @li No other unit is currently entering/leaving
	 * @li The doors are open (if this storage has something like doors)
	 **/
	void approveOneRequest();
	void removeFromAllLists(const Unit* unit);

	void changeStatus(StorageStatus status);

	void advanceDoorsClosed(unsigned int advanceCallsCount);
	void advanceDoorsOpening(unsigned int advanceCallsCount);
	void advanceDoorsClosing(unsigned int advanceCallsCount);
	void advanceUnitCanEnterOrLeave(unsigned int advanceCallsCount);
	void advanceUnitEnteringOrLeaving(unsigned int advanceCallsCount);

private:
	UnitStoragePluginPrivate* d;
};

#endif

