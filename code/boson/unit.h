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
#ifndef __UNIT_H__
#define __UNIT_H__

#include "unitbase.h"

#include <qcanvas.h>

class Player;
class BosonCanvas;
class UnitProperties;

/**
 * Implementation of the visual parts of a unit. As far as possible all stuff
 * should go to UnitBase directly - except the visual stuff. (UPDATE 01/12/27:
 * most stuff is in Unit, UnitBase is kind of obsolete, might be removed
 * someday)
 *
 * Not that Unit does <em>not</em> inherit @ref QObject! Signals/Slots are
 * therefore not possible! This is done to save as much memory as possible.
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class Unit : public UnitBase, public QCanvasSprite
{
public:
	enum PropertyIds {
		IdDirection = UnitBase::IdLast + 1,
		IdWaypoints = UnitBase::IdLast + 2,
		IdFix_ConstructionState = UnitBase::IdLast + 3,
		IdReloadState = UnitBase::IdLast + 4,
		IdFix_Productions = UnitBase::IdLast + 5,
		IdFix_ProductionState = UnitBase::IdLast + 6

	};

	enum Direction {
		North = 0,
		NorthEast = 1,
		East = 2, 
		SouthEast = 3,
		South = 4,
		SouthWest = 5,
		West = 6,
		NorthWest = 7
	};

	enum UnitSound {
		SoundShoot = 0,
		SoundOrderMove = 1,
		SoundOrderAttack = 2,
		SoundOrderSelect = 3,
		SoundReportProduced = 4,
		SoundReportDestroyed = 5,
		SoundReportUnderAttack = 6
	};

	Unit(const UnitProperties* prop, Player* owner, QCanvas* canvas);
	virtual ~Unit();

	inline virtual int rtti() const { return UnitBase::rtti(); }

	inline virtual void setHealth(unsigned long int h);

	inline BosonCanvas* boCanvas() const { return (BosonCanvas*)canvas(); }

	void select();
	void unselect();

	virtual void moveBy(double x, double y);

	virtual void advance(int phase);

	void attackUnit(Unit* target);
	
	inline Unit* target() const;
	void setTarget(Unit* target);
	bool inRange(Unit* unit) const;

// waypoint stuff: // also in facility - produced units receive this initial waypoint
	void addWaypoint(const QPoint& pos);
	const QPoint& currentWaypoint() const;
	unsigned int waypointCount() const;
	void clearWaypoints();
	void waypointDone();

	/**
	 * @return A list of all waypoints for debugging
	 **/
	QValueList<QPoint> waypointList() const;

	/**
	 * Move this unit to a specified point. Also make sure that previous
	 * @ref work is cleared. After the path is found the unit starts to get
	 * animated and to move to the destination.
	 * @param pos The point on the canvas to move to.
	 **/
	void moveTo(const QPoint& pos);
	
	/**
	 * Nearly similar to the above version (actually this is called by the
	 * above) but any previous @ref work is <em>not</em> cleared. This way
	 * you can call this while e.g. a unit is being attacked and actually
	 * start to attack as soon as the unit is in range. We also assume that
	 * the @ref setAnimated was already called.
	 * @param x The destination x-coordinate on the canvas
	 * @param y The destination y-coordinate on the canvas
	 **/
	void moveTo(int x, int y);

	/**
	 * Just stop moving. Don't call this if you don't want to stop attacking
	 * as well! This sets @ref work to @ref WorkNone
	 **/
	void stopMoving();
	void stopAttacking();

	virtual bool save(QDataStream& stream);
	virtual bool load(QDataStream& stream);

	void updateSelectBox();


	QCanvasItemList unitsInRange() const;
	QCanvasItemList enemyUnitsInRange() const;


	inline unsigned int reloadState() const;

protected:
	void shootAt(Unit* target);

	/**
	 * @return a list of interesting collisions, i.e. no non-units, no
	 * destryed units, ...
	 **/
	QValueList<Unit*> unitCollisions(bool exact = false) const;

	/**
	 * Move the unit. By default this does nothing. Reimplemented in @ref
	 * MobileUnit
	 **/
	virtual void advanceMove() { }
	virtual void advanceMoveCheck() { }

	/**
	 * Move the construction animation one step forward. Does nothing by
	 * default - reimplemented in Facility
	 **/
	virtual void advanceConstruction() { }

	virtual void advanceProduction() { }

	int mPathrecalc;
	int mMoveDestX, mMoveDestY;
	bool mAttacking;

private:
	class UnitPrivate;
	UnitPrivate* d;
};


// if you add class members - ONLY KGameProperties!! otherwise Player::load and
// Player::save() won't work correctly! - if you add non KGameProperties adjust
// UnitBase::save() and unit::load()
class MobileUnit : public Unit
{
public:
	MobileUnit(const UnitProperties* prop, Player* owner, QCanvas* canvas);
	virtual ~MobileUnit();

	virtual void setSpeed(double s);
	inline virtual double speed() const;

	/**
	 * Turn to direction. This sets a new frame according to the new
	 * direction.
	 **/
	inline void turnTo(Direction direction);

	/**
	 * Call turnTo according to the current speed (you want to use this!)
	 **/
	inline void turnTo();

protected:
	virtual void advanceMove(); // move one step futher to path
	virtual void advanceMoveCheck();
	/** Finds new path to destination
	  * Destination must have been set before in variables movedestx and movedesty
	  * @return true if path was found, false otherwise
	  */
	bool newPath();
	/** Same as above, but sets movedestx to destx and movedesty to desty first */
	bool newPath(int destx, int desty);

private:
	// a d pointer is probably not very good here - far too much memory consumption
	// same apllies to Unit and UnitBase. But it speeds up compiling as we don't
	// have to change the headers every time...
	class MobileUnitPrivate; 
	MobileUnitPrivate* d;
};

// if you add class members - ONLY KGameProperties!! otherwise Player::load and
// Player::save() won't work correctly!
class Facility : public Unit
{
public:
	Facility(const UnitProperties* prop, Player* owner, QCanvas* canvas);
	virtual ~Facility();

	/**
	 * The construction steps are the number of frames until the complete
	 * pixmap of the facility is shown.
	 * @return The number of available construction steps for a facility.
	 **/
	static int constructionSteps();

	/**
	 * @return Whether there are any productions pending for this unit.
	 * Always FALSE if unitProperties()->canProduce() is FALSE.
	 **/
	inline bool hasProduction() const;

	/**
	 * @return Whether the current construction can be placed at pos. FALSE
	 * if @ref hasConstruction is FALSE.
	 **/
	bool canPlaceProductionAt(const QPoint& pos) const;

	/**
	 * @return The unit type ID (see @ref UnitProperties::typeId) of the
	 * completed production (if any).
	 **/
	inline int completedProduction() const;

	/**
	 * @return The unit type ID of the current production. -1 if there is no
	 * production.
	 **/
	inline int currentProduction() const;
	 

	/**
	 * Remove the first item from the production list.
	 **/
	void removeProduction(); // removes first item

	/**
	 * Remove first occurance of unitType in the production list. Does not
	 * remove anything if unitType is not in the list.
	 **/
	void removeProduction(int unitType);

	/**
	 * Add unitType (see @ref UnitProprties::typeId) to the construction
	 * list.
	 **/
	void addProduction(int unitType);

	QValueList<int> productionList() const;

protected:
	virtual void advanceProduction();

	/**
	 * Advance the construction animation. This is usually called when
	 * placing the unit until thje construction is completed.
	 **/
	virtual void advanceConstruction();

private:
	class FacilityPrivate;
	FacilityPrivate* d;
};

#endif
