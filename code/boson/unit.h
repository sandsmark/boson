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
#include "global.h"

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
		IdMoveDestX = UnitBase::IdLast + 3,
		IdMoveDestY = UnitBase::IdLast + 4,
		IdMovingFailed = UnitBase::IdLast + 5,
		IdFix_ConstructionState = UnitBase::IdLast + 20,
		IdFix_Productions = UnitBase::IdLast + 21,
		IdFix_ProductionState = UnitBase::IdLast + 22

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

	/**
	 * There is not much to do here. Keep the stuff in this function as
	 * simple as possible as it's called for <em>every</em> unit on
	 * <em>every</em> advance call. An example of what could be done here is
	 * to increase the realod state.
	 * 
	 * The really interesting things get done in the advanceXYZ() functions
	 * below. They are called from @ref BosonCanvas::slotAdvance(). For
	 * every @ref UnitBase::WorkType there is at least one advance function which
	 * implements its behaviour.
	 * @param phase 0 is most game logic, 1 is nothing but moving. Collision
	 * detection, path finding and all like this should be done in phase 0
	 **/
	virtual void advance(int phase);

	/**
	 * Move the unit. By default this does nothing. Reimplemented in @ref
	 * MobileUnit
	 **/
	virtual void advanceMove() { }

	/**
	 * Move according to the velocity of the leader
	 **/
	virtual void advanceGroupMove(Unit* ) { }
	
	/**
	 * Also reimplemented in @ref MobileUnit. Used to check whether the path
	 * calculated by @ref advanceMove was actually valid.
	 **/
	virtual void advanceMoveCheck() { }

	/**
	 * Mine, mine, mine ...
	 **/
	virtual void advanceMine();

	/**
	 * Attack a unit. The target was set before using @ref setTarget
	 **/
	virtual void advanceAttack();

	/**
	 * This is called when there is nothing else to do for this unit.
	 * Usually the unit will check for enemy units in range and fire at
	 * them.
	 **/
	virtual void advanceNone();

	/**
	 * Move the construction animation one step forward. Does nothing by
	 * default - reimplemented in @ref Facility
	 **/
	virtual void advanceConstruction() { }

	/**
	 * Produce a unit. Reimplemented in @ref Facility.
	 **/
	virtual void advanceProduction() { }

	void attackUnit(Unit* target);
	
	inline Unit* target() const;
	virtual void setTarget(Unit* target);
	bool inRange(Unit* unit) const;

// waypoint stuff: // also in facility - produced units receive this initial waypoint
	/**
	 * Add pos to the waypoint list. Please note that waypoints are actually
	 * added later. addWaypoint() sends the point over network and only as
	 * soon as it is received from there you can work with it. That means,
	 * that after calling addWaypoint() @ref waypointCount has not yet been
	 * increased!
	 **/
	void addWaypoint(const QPoint& pos);
	
	const QPoint& currentWaypoint() const;
	unsigned int waypointCount() const;

	/**
	 * Removes all waypoints from the list. Note that this is done
	 * <em>immediately</em> - in contrary to @ref addWaypoint this is not
	 * sent over network as all function calling clearWaypoints() are
	 * already called on all clients!
	 * @param send If FALSE clear the waypoints immediately (like
	 * PolicyLocal) otherwise send the command over network (like
	 * PolicyClean)
	 **/
	void clearWaypoints(bool send = false);

	/**
	 * Remove the first waypoint from the list. Note that this is done
	 * <em>immediately</em>. See @ref clearWaypoints for more infos.
	 **/
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
	virtual void moveTo(const QPoint& pos);
	
	/**
	 * Nearly similar to the above version (actually this is called by the
	 * above) but any previous @ref work is <em>not</em> cleared. This way
	 * you can call this while e.g. a unit is being attacked and actually
	 * start to attack as soon as the unit is in range. We also assume that
	 * the @ref setAnimated was already called.
	 * @param x The destination x-coordinate on the canvas
	 * @param y The destination y-coordinate on the canvas
	 * @return true if unit can go to destination, false otherwise
	 **/
	bool moveTo(int x, int y);

	/**
	 * Just stop moving. Don't call this if you don't want to stop attacking
	 * as well! This sets @ref work to @ref WorkNone
	 **/
	void stopMoving();
	void stopAttacking();

	void moveInGroup();
	void leaderMoved(double , double ) {};
	void setGroupLeader(bool leader);

	virtual bool save(QDataStream& stream);
	virtual bool load(QDataStream& stream);

	void updateSelectBox();

	QCanvasItemList unitsInRange() const;
	QCanvasItemList enemyUnitsInRange() const;

	/**
	 * Calls @ref BosonCanvas setWorkChanged
	 **/
	virtual void setWork(WorkType w);

	virtual bool collidesWith(const QCanvasItem* item) const;

protected:
	void shootAt(Unit* target);

	/**
	 * @return a list of interesting collisions, i.e. no non-units, no
	 * destryed units, ...
	 **/
	QValueList<Unit*> unitCollisions(bool exact = false) const;

	/** 
	 * Finds new path to destination.
	 * Destination must have been set before in variables movedestx and 
	 * movedesty - usually using @ref moveTo
	 *
	 * This is in Unit instead of @ref MobileUnit so that we can apply a
	 * path to newly constructed units of factories.
	 * @return true if path was found, false otherwise
	 **/
	bool newPath();

	bool searchpath;

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

	void leaderMoved(double x, double y);

protected:
	virtual void advanceMove(); // move one step futher to path
	/**
	 * Move according to the velocity of leader
	 **/
	virtual void advanceGroupMove(Unit* leader);
	virtual void advanceMoveCheck();

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
	 * @return How many advance calls are needed to increase @ref
	 * constructionSteps
	 **/
	static int constructionDelay();

	/**
	 * Please note that the construction state of a unit specifies if a unit
	 * <em>has been built</em> completely - it has nothing to do with the
	 * productions of a facility!
	 * @return If this unit has been built (constructed) completely
	 **/
	inline bool isConstructionComplete() const;

	inline double constructionProgress() const;

	/**
	 * Used by to initialize the scenario. A scenario file can specify that
	 * a facility is already completed - to do so we can
	 * setContructionStep(constructionSteps() - 1).
	 *
	 * This should <em>not</em> be called once the game is started
	 **/
	void setConstructionStep(unsigned int step);

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

	/**
	 * @return The percentage of the production progress. 0 means the
	 * production just started, 100 means the production is completed.
	 **/
	inline double productionProgress() const;

	/**
	 * Reimplemented. Does nothing if @ref isConstructionComplete is false -
	 * otherwise the same as @ref Unit::setTarget
	 **/
	virtual void setTarget(Unit*);

	/**
	 * Does nothing if @ref isConstructionComplete is false - otherwise the
	 * same as @ref Unit::moveTo
	 **/
	virtual void moveTo(int x, int y);

	virtual void advanceProduction();

	/**
	 * Advance the construction animation. This is usually called when
	 * placing the unit until the construction is completed. See @ref
	 * isConstructionComplete
	 **/
	virtual void advanceConstruction();

protected:

private:
	class FacilityPrivate;
	FacilityPrivate* d;
};

#endif
