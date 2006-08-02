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
#ifndef UNITMOVER_H
#define UNITMOVER_H


#include "unit.h"

#include <qvaluevector.h>

class QDomElement;
class UpgradeProperties;
class BosonCanvas;
class UnitProperties;
class BosonPathInfo;



/**
 * @short Class responsible for moving units.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class UnitMover
{
public:
	UnitMover(Unit* u);
	virtual ~UnitMover();

	static void initStatic();

	virtual bool init();

	virtual bool saveAsXML(QDomElement& root) const;
	virtual bool loadFromXML(const QDomElement& root);

	virtual void addUpgrade(const UpgradeProperties* upgrade);
	virtual void removeUpgrade(const UpgradeProperties* upgrade);

	virtual void advanceIdle()  {}
	virtual void advanceFollow(unsigned int);

	virtual bool attackEnemyUnitsInRangeWhileMoving();


	/**
	 * @return Maximum speed of this mobile unit. WARNING: there is also a
	 * @ref BosonItem::maxSpeed. Both values should be synchronized.
	 **/
	bofixed maxSpeed() const;

	/**
	 * @return How fast this mobile unit accelerates.
	 **/
	bofixed maxAccelerationSpeed() const;

	/**
	 * @return How fast this mobile unit decelerates.
	 **/
	bofixed maxDecelerationSpeed() const;

	inline Unit* unit() const
	{
		return mUnit;
	}
	inline const BosonCanvas* canvas() const
	{
		return unit()->canvas();
	}
	inline unsigned long int id() const
	{
		return unit()->id();
	}
	inline UnitBase::WorkType advanceWork() const
	{
		return unit()->advanceWork();
	}
	inline const UnitProperties* unitProperties() const
	{
		return unit()->unitProperties();
	}
	inline BosonPathInfo* pathInfo() const
	{
		return unit()->pathInfo();
	}

	/**
	 * Call turnTo according to the current speed (you want to use this!)
	 **/
	bool turnTo();

	void advanceMove(unsigned int advanceCallsCount);

	/**
	 * Stops moving and completes current move order
	 **/
	virtual void stopMoving(bool success);




protected:
	static void initCellIntersectionTable();
	bool advancedRectWillBeOnCanvas() const;
	void changeUpgrades(const UpgradeProperties* upgrade);

	virtual void advanceMoveInternal(unsigned int) = 0; // move one step futher to path

	/**
	 * Note: this is not actually an advance*() method, like @ref
	 * advanceWork and the like. advanceMoveCheck() must get called
	 * (manually) after any advance*() method that moves a unit.
	 *
	 * This is most notably @ref advanceMove
	**/
	virtual void advanceMoveCheck();

protected:
	static QValueVector<BoVector2Fixed> mCellIntersectionTable[11][11];

private:

	Unit* mUnit;

	BoUpgradeableProperty<bofixed> mMaxSpeed;
	BoUpgradeableProperty<bofixed> mMaxAccelerationSpeed;
	BoUpgradeableProperty<bofixed> mMaxDecelerationSpeed;
};



class UnitMoverLand : public UnitMover
{
public:
	UnitMoverLand(Unit* u);
	virtual ~UnitMoverLand();

	virtual bool init();


protected:
	virtual void advanceMoveInternal(unsigned int advanceCallsCount);
	virtual void advanceMoveCheck();


	/**
	 * Finds new path to destination.
	 * Destination must have been set before in @ref pathInfo
	 * @return TRUE on success. @ref pathInfo contains the new path
	 * and @ref unit::pathPointList has been updated. FALSE on failure, @ref
	 * Unit::pathPointList is empty then. The unit should stop moving in
	 * this case afterwards.
	 **/
	bool calculateNewPath();

	/**
	 * Move towards p, going at most maxdist (in canvas coords).
	 * How much unit should move, will be added to xspeed and yspeed.
	 * (x; y) marks unit's current position
	 *
	 * @return How much is moved (will be <= maxdist)
	 **/
	bofixed moveTowardsPoint(const BoVector2Fixed& p, bofixed x, bofixed y, bofixed maxdist, bofixed &xspeed, bofixed &yspeed);

	int selectNextPathPoint(int xpos, int ypos);
	void avoidance();
	bool canGoToCurrentPathPoint(int xpos, int ypos);
	void currentPathPointChanged(int unitx, int unity);

private:
	// Should these be made KGameProperty?
	int mLastCellX;
	int mLastCellY;

	int mNextCellX;
	int mNextCellY;
	QValueVector<BoVector2Fixed>* mNextWaypointIntersections;
	int mNextWaypointIntersectionsXOffset;
	int mNextWaypointIntersectionsYOffset;
};



class UnitMoverFlying : public UnitMover
{
public:
	UnitMoverFlying(Unit* u);
	virtual ~UnitMoverFlying();

	virtual bool init();

	virtual void advanceIdle();

	virtual bool saveAsXML(QDomElement& root) const;
	virtual bool loadFromXML(const QDomElement& root);

protected:
	virtual void advanceMoveInternal(unsigned int advanceCallsCount);

	void flyInCircle();

private:
	bofixed mRoll;
};

#endif

