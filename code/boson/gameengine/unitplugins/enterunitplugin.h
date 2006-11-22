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
#ifndef ENTERUNITPLUGIN_H
#define ENTERUNITPLUGIN_H

#include "unitplugin.h"

#include <kgame/kgameproperty.h>
#include <kgame/kgamepropertylist.h>
#include "../../bo3dtools.h"

template<class T> class BoVector2;
template<class T> class BoRect2;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoRect2<bofixed> BoRect2Fixed;
class UnitStoragePlugin;
class Unit;
class BosonItem;

class QDomElement;

/**
 * Every mobile unit has this plugin.
 *
 * @short This plugin allows a mobile unit to enter another unit (see @ref
 * UnitStoragePlugin).
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class EnterUnitPlugin : public UnitPlugin
{
public:
	enum MovingInStatus {
		StatusIsOutside = 0,
		StatusMovingToEnterPoint1 = 1,
		StatusWaitForEnterPermission = 2,
		StatusMovingToEnterPoint2 = 3,
		StatusMovingToStoragePosition = 4,
		StatusHasEntered = 5,

		StatusWaitForLeavePermission = 100,
		StatusMovingToOutside = 101,
		//StatusMovingBackToStoragePosition = 102, // not yet used
		StatusHasLeft = 103
	};

	/**
	 * The LandingStatus is valid for aircrafts only.
	 *
	 * This is used by the @ref UnitMoverInsideUnit, to decide how to move
	 * a flying unit that is entering a unit.
	 **/
	enum LandingStatus {
		// this unit is not currently in a "landing" relevant state,
		// i.e. it is currently neither landing nor starting.
		// -> it is either flying or it is on the ground.
		LandingStatusNone = 0,

		// unit is currently flying, but is about to start landing
		LandingStatusFlyingTowardsLandingPoint = 1,

		// unit is currently landing
		LandingStatusIsLanding = 2,

		// unit is currently on the ground, but about to start taking
		// off
		LandingStatusGoingTowardsTakeOffPoint = 3,

		// unit is currently starting, i.e. taking off
		LandingStatusIsTakingOff = 4
	};

public:
	EnterUnitPlugin(Unit* unit);
	~EnterUnitPlugin();

	virtual int pluginType() const { return UnitPlugin::EnterUnit; }

	virtual void advance(unsigned int);

	virtual bool saveAsXML(QDomElement& root) const;
	virtual bool loadFromXML(const QDomElement& root);

	virtual void unitDestroyed(Unit*);
	virtual void itemRemoved(BosonItem*);

	/**
	 * Called once the @ref UnitEnterUnitOrder becomes active. Here we
	 * move to the @p destinationUnit and start entering once we arrived
	 * there.
	 **/
	bool enter(Unit* destinationUnit);
	bool leave();

	/**
	 * Abort entering/leaving.
	 *
	 * If this unit is currently neither entering nor leaving, this is a
	 * noop.
	 *
	 * Note that in certain situations, entering/leaving cannot be aborted,
	 * e.g. when the unit has already entered the storage and is moving to
	 * its final storage position. In that case this method is a noop.
	 **/
	void abort();

	MovingInStatus movingInStatus() const;

	/**
	 * Valid for aircrafts only. See @ref LandingStatus: this is used by the
	 * @ref UnitMoverInsideUnit to decide how to move a flying unit.
	 **/
	LandingStatus landingStatus() const;

	/**
	 * Start the actual landing. This is called by @ref UnitMoverInsideUnit
	 * once the (flying) unit starts to land. This method changes the @ref
	 * landingStatus to @ref LandingStatusIsLanding.
	 **/
	void startLanding();
	void completeLanding();

	/**
	 * Similar to @ref startLanding, but inverse - i.e. this is called when
	 * taking off.
	 **/
	void startTakingOff();
	void completeTakingOff();

	/**
	 * @return The plugin that this unit is inside (or about to
	 * enter). See also @ref enter. Note that this value is set once
	 * we @em intend to enter the unit, even if we first need to move there.
	 * See @ref movingInStatus for whether we actually are inside the unit
	 * already.
	 **/
	UnitStoragePlugin* storingUnit() const;

	QValueList<BoVector2Fixed> remainingInsidePath() const;

	/**
	 * Called by @ref UnitMoverInsideUnit only. This removes the very first
	 * pathpoint from the @ref remainingInsidePath.
	 **/
	void pathPointDone();

protected:
	void changeStatus(MovingInStatus newStatus);

	bool requestEnterPath();
	bool requestLeavePath();

	bool isAtEnterPoint1() const;
	bool isAtEnterPoint2() const;

	// is inside/outside
	void advanceIsOutside(unsigned int);
	void advanceHasEntered(unsigned int);

	// entering
	void advanceMovingToEnterPoint1(unsigned int);
	void advanceWaitForEnterPermission(unsigned int);
	void advanceMovingToEnterPoint2(unsigned int);
	void advanceMovingToStoragePosition(unsigned int);

	// leaving
	void advanceWaitForLeavePermission(unsigned int);
	void advanceMovingToOutside(unsigned int);


private:
	UnitStoragePlugin* mUnitStoragePlugin;

	// a point outside of the storage unit - where we have to move to, to
	// start entering the storage.
	// point 1 is "close" to the storage, point 2 next to it.
	// -> the unit is meant to rotate to the correct direction while moving
	// from point 1 to point 2. for units that can move while standing
	// still, point 1 and 2 may be the same.
	KGameProperty<BoVector2Fixed> mEnterPoint1;
	KGameProperty<BoVector2Fixed> mEnterPoint2;

	KGameProperty<Q_INT32> mPathIndex;

	// a list of pathpoints inside the unit leading to the destination of
	// the path
	KGamePropertyList<BoVector2Fixed> mRemainingInsidePath;

	KGameProperty<Q_INT32> mMovingInStatus;
	KGameProperty<Q_INT32> mLandingStatus;

	KGameProperty<Q_INT32> mTriedMovingCounter;
};

#endif
