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
#ifndef UNIT_H
#define UNIT_H

#include "unitbase.h"
#include "global.h"

#include <qcanvas.h>

class Player;
class BosonCanvas;
class UnitProperties;
class Cell;
class Facility;
class ProductionPlugin;
class RepairPlugin;

/**
 * Implementation of the visual parts of a unit. As far as possible all stuff
 * should go to UnitBase directly - except the visual stuff. (UPDATE 01/12/27:
 * most stuff is in Unit, UnitBase is kind of obsolete, might be removed
 * someday)
 *
 * Note that Unit does <em>not</em> inherit @ref QObject! Signals/Slots are
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
		IdMoveRange = UnitBase::IdLast + 5,
		IdMob_MovingFailed = UnitBase::IdLast + 20,
		IdMob_ResourcesMined = UnitBase::IdLast + 21,
		IdMob_ResourcesX= UnitBase::IdLast + 22,
		IdMob_ResourcesY= UnitBase::IdLast + 23,
		IdMob_PathRecalculated = UnitBase::IdLast + 24,
		IdFix_ConstructionState = UnitBase::IdLast + 30,

		IdUnitPropertyLast
	};
	enum PluginPropertyIds {
		IdPlugin_Productions = IdUnitPropertyLast + 1,
		IdPlugin_ProductionState = IdUnitPropertyLast + 2,
		IdPlugin_RepairList = IdUnitPropertyLast + 10,
//		IdPlugin_RepairAdvanceCounter = IdUnitPropertyLast + 11,

		IdPluginProperyLast
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

	virtual void setHealth(unsigned long int h);

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
	virtual void advanceMine() { }

	virtual void advanceRefine() { }

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
	 * @return the @ref ProductionPlugin if this unit is able to produce
	 * units (this means: the @ref UnitProperties allow productions and the
	 * unit is e.g. fully constructed). Note that a @ref MobileUnit can not
	 * yet have such a plugin, since a log of @ref isFacility tests are in
	 * the code :-(
	 **/
	virtual ProductionPlugin* productionPlugin() const { return 0; }
	virtual RepairPlugin* repairPlugin() const { return 0; }

	Unit* target() const;
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
	 * @param range Number of tiles around the destination the unit is
	 * allowed to go to. You must use range > 0 e.g. if destination is
	 * occupied and you should use e.g. @ref weaponRange if the unit should
	 * attack. -1 keeps the previously set range.
	 * @return true if unit can go to destination, false otherwise
	 **/
	bool moveTo(double x, double y, int range = 0);

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

	/**
	 * @return All units except this that are in @ref weaponRange of this
	 * unit.
	 **/
	QCanvasItemList unitsInRange() const;

	/**
	 * @return Just like @ref unitsInRange but only enemy units.
	 **/
	QCanvasItemList enemyUnitsInRange() const;

	/**
	 * Calls @ref BosonCanvas setWorkChanged
	 **/
	virtual void setAdvanceWork(WorkType w);

	virtual bool collidesWith(const QCanvasItem* item) const;

	int destinationX() const;
	int destinationY() const;
	int moveRange() const;

	/**
	 * @return TRUE if this unit is next to unit (i.e. less than one cell
	 * distance) otherwise FALSE
	 **/
	bool isNextTo(Unit* unit) const;

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
	 **/
	void newPath();

protected:
	bool mSearchPath;

private:
	class UnitPrivate;
	UnitPrivate* d;
};


// if you add class members - ONLY KGameProperties!! otherwise Player::load and
// Player::save() won't work correctly! - if you add non KGameProperties adjust
// UnitBase::save() and unit::load()
/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class MobileUnit : public Unit
{
public:
	MobileUnit(const UnitProperties* prop, Player* owner, QCanvas* canvas);
	virtual ~MobileUnit();

	virtual void setSpeed(double s);
	virtual double speed() const;

	/**
	 * Turn to direction. This sets a new frame according to the new
	 * direction.
	 **/
	void turnTo(Direction direction);

	/**
	 * Call turnTo according to the current speed (you want to use this!)
	 **/
	void turnTo();

	void leaderMoved(double x, double y);

	virtual void advanceMine();
	virtual void advanceRefine();
	virtual void advanceMove(); // move one step futher to path
	/**
	 * Move according to the velocity of leader
	 **/
	virtual void advanceGroupMove(Unit* leader);
	virtual void advanceMoveCheck();


	/**
	 * @return The number of resources that have been mined by this unit.
	 * Always 0 if this unit can't mine oil/minerals
	 **/
	unsigned int resourcesMined() const;
	
	/**
	 * @return The x-coordinate of the resource field or 0 if none has been
	 * set or this unit cannot mine.
	 **/
	int resourcesX() const;
	/**
	 * @return The y-coordinate of the resource field or 0 if none has been
	 * set or this unit cannot mine.
	 **/
	int resourcesY() const;

	/**
	 * @return TRUE if this unit can mine the minerals at cell (if any), 
	 * otherwise FALSE.
	 **/
	bool canMine(Cell* cell) const;

	/**
	 * Order the unit to mine minerals/oil at pos
	 **/
	void mineAt(const QPoint& pos);
	void refineAt(Facility* refinery);

	/**
	 * Move this unit to the repairYard and repair it there.
	 **/
	void repairAt(Facility* repairYard);

	void setRefinery(Facility* refinery);

	/**
	 * We use @ref target for the refinery. If @ref target is not a refinery
	 * we return NULL
	 * @return NULL if no valid refinery is set, otherwise the refinery.
	 **/
	Facility* refinery() const;
	virtual QRect boundingRect() const;

	virtual void clearWaypoints(bool send = false);
	virtual void waypointDone();

private:
	// a d pointer is probably not very good here - far too much memory consumption
	// same apllies to Unit and UnitBase. But it speeds up compiling as we don't
	// have to change the headers every time...
	class MobileUnitPrivate; 
	MobileUnitPrivate* d;
};

// if you add class members - ONLY KGameProperties!! otherwise Player::load and
// Player::save() won't work correctly!
/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
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
	bool isConstructionComplete() const;

	double constructionProgress() const;

	/**
	 * Used by to initialize the scenario. A scenario file can specify that
	 * a facility is already completed - to do so we can
	 * setContructionStep(constructionSteps() - 1).
	 *
	 * This should <em>not</em> be called once the game is started
	 **/
	void setConstructionStep(unsigned int step);

	unsigned int currentConstructionStep() const;

	/**
	 * Reimplemented. Does nothing if @ref isConstructionComplete is false -
	 * otherwise the same as @ref Unit::setTarget
	 **/
	virtual void setTarget(Unit*);

	/**
	 * Does nothing if @ref isConstructionComplete is false - otherwise the
	 * same as @ref Unit::moveTo
	 **/
	virtual void moveTo(double x, double y, int range = 0);

	/**
	 * Advance the construction animation. This is usually called when
	 * placing the unit until the construction is completed. See @ref
	 * isConstructionComplete
	 **/
	virtual void advanceConstruction();

	virtual ProductionPlugin* productionPlugin() const;
	virtual RepairPlugin* repairPlugin() const;

protected:

private:
	class FacilityPrivate;
	FacilityPrivate* d;
};

#endif
