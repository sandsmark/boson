/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

class Player;
class BosonCanvas;
class BoItemList;
class UnitProperties;
class Cell;
class Facility;
class ProductionPlugin;
class UnitPlugin;
class RepairPlugin;
class BosonWeapon;
class BosonPathInfo;
template<class T> class BoVector2;
template<class T> class BoVector3;
template<class T> class BoRect;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoVector3<bofixed> BoVector3Fixed;
typedef BoRect<bofixed> BoRectFixed;
template<class T> class QValueList;
template<class T> class QPtrList;
class QDomElement;

class KGameUnitDebug;

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
class Unit : public UnitBase
{
public:
	enum PropertyIds {
		// properties in Unit
		IdWaypoints = UnitBase::IdLast + 2,
		IdWantedRotation = UnitBase::IdLast + 6,
		IdPathPoints = UnitBase::IdLast + 10,

		// properties in MobileUnit

		// properties in Facility
		IdConstructionStep = UnitBase::IdLast + 100,

		// properties in UnitPlugins and derived classes
		IdProductionState = UnitBase::IdLast + 300,
		IdResourcesMined = UnitBase::IdLast + 301,
		IdResourcesX = UnitBase::IdLast + 302,
		IdResourcesY = UnitBase::IdLast + 303,
		IdHarvestingType = UnitBase::IdLast + 304,
		IdBombingPosX = UnitBase::IdLast + 305,
		IdBombingPosY = UnitBase::IdLast + 306,
		IdMinePlacingCounter = UnitBase::IdLast + 307,
		IdResourceMineMinerals = UnitBase::IdLast + 308,
		IdResourceMineOil = UnitBase::IdLast + 309
	};

	Unit(const UnitProperties* prop, Player* owner, BosonCanvas* canvas);
	virtual ~Unit();

	virtual bool init();

	static void initStatic();

	inline virtual int rtti() const { return UnitBase::rtti(); }

	virtual void setHealth(unsigned long int h);

	/**
	 * @return The current plugin, used in e.g. @ref advance or NULL if
	 * none. Always NULL if @ref work != WorkPlugin
	 **/
	inline UnitPlugin* currentPlugin() const { return mCurrentPlugin; }

	/**
	 * @return The @refUnitPlugin::pluginType of the @ref currentPlugin or
	 * 0 if there is no plugin.
	 **/
	int currentPluginType() const;

	/**
	 * @param pluginType See @ref UnitPlugin::UnitPlugins
	 **/
	virtual UnitPlugin* plugin(int pluginType) const;

	virtual void setWork(WorkType w);
	void setPluginWork(int pluginType);

	/**
	 * We are about to remove @p item from the game. Make sure that this
	 * unit does not use that item in any way (e.g. as a target)
	 *
	 * -> after this method was called @p item will become an invalid
	 *  pointer, so all references to that pointer must be set to NULL
	 **/
	virtual void itemRemoved(BosonItem* item);


	virtual void select(bool markAsLeader = false);

	/**
	 * Moves unit by specified amount and also calculates new z-coordinate and
	 * rotation.
	 * This should only be called by BosonCanvas' advance methods.
	 **/
	virtual void moveBy(bofixed x, bofixed y, bofixed z);

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
	 *
	 * Please note that the @ref UnitPlugin::advance methods get called
	 * here, too!
	 **/
	virtual void advance(unsigned int advanceCallsCount);

	/**
	 * Call the advance*() function that is currently used. The advance
	 * function is changed by @ref Unit::setWork usually. It can be changed in
	 * special situations by @ref setAdvanceWork directly.
	 * Both functions use @ref setAdvanceFunction
	 *
	 * Note that we store a pointer to the advance function internally. This
	 * is done for performance reasons - advance gets called pretty often
	 * for a lot of units and with this we avoid any switch on @ref
	 * advanceWork
	 **/
	inline virtual void advanceFunction(unsigned int advanceCallsCount)
	{
		(this->*mAdvanceFunction)(advanceCallsCount);
	}

	/**
	 * See @ref advanceFunction.
	 *
	 * We store 2 advance pointers because the units are iterated in loops.
	 * The first unit might change the advance function of the second unit -
	 * but we must call the previous advance function of the second unit.
	 *
	 * See also @ref Boson::advanceFlag and @ref Boson::toggleAdvanceFlag
	 **/
	inline virtual void advanceFunction2(unsigned int advanceCallsCount)
	{
		(this->*mAdvanceFunction2)(advanceCallsCount);
	}


	/**
	 * Synchronize the advance function variable. This needs to be called
	 * after @ref advanceFunction has been called for <em>all</em> units.
	 *
	 * Use @ref syncAdvanceFunction2 when @ref advanceFunction2 was called
	 * on all units.
	 *
	 * DO NOT CALL THIS unless you REALLY know what youre doing! Call it
	 * from @ref BosonCanvas::slotAdvance ONLY!
	 **/
	inline virtual void syncAdvanceFunction()
	{
		mAdvanceFunction = mAdvanceFunction2;
	}
	/**
	 * See @ref syncAdvanceFunction
	 **/
	inline virtual void syncAdvanceFunction2()
	{
		mAdvanceFunction2 = mAdvanceFunction;
	}


	/**
	 * Convenience method for moving the unit - @ref advanceMoveCheck must
	 * be called after @ref advanceMoveInternal, so you should never call @ref
	 * advanceMoveInternal directly, but rather use this.
	 **/
	void advanceMove(unsigned int advanceCallsCount)
	{
		advanceMoveInternal(advanceCallsCount);
		advanceMoveCheck();
	}


	/**
	 * Attack a unit. The target was set before using @ref setTarget
	 **/
	virtual void advanceAttack(unsigned int advanceCallsCount);

	/**
	 * Follow another unit.
	 * Note that this method is somewhat similar to @ref advanceAttack
	 **/
	virtual void advanceFollow(unsigned int) { }

	/**
	 * This is called when there is nothing else to do for this unit.
	 * Usually the unit will check for enemy units in range and fire at
	 * them.
	 **/
	virtual void advanceNone(unsigned int advanceCallsCount);

	/**
	 * Move the construction animation one step forward. Does nothing by
	 * default - reimplemented in @ref Facility
	 **/
	virtual void advanceConstruction(unsigned int) { }

	/**
	 * Called when the unit has been destroyed. Maybe compute a destruction
	 * animation here, or animate the wreckage.
	 *
	 * Should do - just like all advance*() methods - as little as possible.
	 **/
	virtual void advanceDestroyed(unsigned int advanceCallsCount);

	/**
	 * Call the @ref UnitPlugin::advance method of the @ref currentPlugin
	 **/
	virtual void advancePlugin(unsigned int advanceCallsCount);

	/**
	 * Called when unit has to turn to some direction. It smootly turns unit
	 * until it has wanted direction
	 **/
	virtual void advanceTurn(unsigned int);

	/**
	 * @return Target of the unit.
	 * Note that target is not neccessarily the unit that this unit attacks - it
	 * is also used for following. In this case, the followed unit will be target
	 * for the followers.
	 **/
	Unit* target() const;
	virtual void setTarget(Unit* target);

	bool inRange(unsigned long int, Unit* unit) const;

// waypoint stuff: // also in facility - produced units receive this initial waypoint
	/**
	 * Add pos to the waypoint list.
	 **/
	void addWaypoint(const BoVector2Fixed& pos);

	const BoVector2Fixed& currentWaypoint() const;
	unsigned int waypointCount() const;
	/**
	 * Removes all waypoints from the list.
	 **/
	void clearWaypoints();

	/**
	 * Remove the first waypoint from the list.
	 **/
	void waypointDone();

	/**
	 * @return A list of all waypoints for debugging
	 **/
	const QValueList<BoVector2Fixed>& waypointList() const;

	/**
	 * @return List of path-points
	 **/
	const QValueList<BoVector2Fixed>& pathPointList() const;

	/**
	 * Move this unit to a specified point. Also make sure that previous
	 * @ref work is cleared. After the path is found the unit starts to get
	 * animated and to move to the destination.
	 * @param pos The point on the canvas to move to.
	 * @param attack If this is true, unit will stop and attack any enemy units in range while moving
	 **/
	virtual void moveTo(const BoVector2Fixed& pos, bool attack = false);


	// TODO: maybe make this protected?
	/**
	 * Internal moving method.
	 * This is the most central method for having unit starting moving to specific
	 * point. You shouldn't use this directly, but instead use 'client methods',
	 * which will then call this one.
	 *
	 * Moves unit's _center_ exactly to given position. If range is not 0, unit
	 * will move until it's less that range _cells_ away from destination.
	 * Destination is in canvas coords.
	 **/
	bool moveTo(bofixed x, bofixed y, int range = 0);

	/**
	 * Turns unit smoothly to given degrees
	 **/
	virtual void turnTo(int degrees);

	/**
	 * Updates unit's x- and y-rotation, so that it will be rotated accordingly to
	 * to the slope of the terrain it's on
	 **/
	void updateRotation();

	/**
	 * Just stop moving. Don't call this if you don't want to stop attacking
	 * as well! This sets @ref work to @ref WorkNone
	 **/
	virtual void stopMoving();
	void stopAttacking();

	virtual bool saveAsXML(QDomElement& root);
	virtual bool loadFromXML(const QDomElement& root);

	void updateSelectBox();

	/**
	 * @return All units except this that are @ref range or less cells away from this unit
	 * Note that it doesn't include units on cells that are not visible
	 * (e.g. fogged) for local player.
	 **/
	BoItemList* unitsInRange(unsigned long int range) const;

	/**
	 * @return Just like @ref unitsInRange but only enemy units.
	 * Note that it doesn't include units on fogged cells (see @ref unitsInRange)
	 **/
	BoItemList* enemyUnitsInRange(unsigned long int range) const;

	/**
	 * Calls @ref BosonCanvas setWorkChanged
	 **/
	virtual void setAdvanceWork(WorkType w);


	/**
	 * @return X-coordinate of unit's current destination (where it's moving).
	 * If unit isn't moving, returned value is undefined.
	 * Returned value is in canvas coordinates.
	 **/
	bofixed destinationX() const;
	/**
	 * @return Y-coordinate of unit's current destination (where it's moving).
	 * If unit isn't moving, returned value is undefined.
	 * Returned value is in canvas coordinates.
	 **/
	bofixed destinationY() const;
	/**
	 * @return Current moving range of the unit (how close to destination point it
	 *  must move).
	 * If unit isn't moving, returned value is undefined.
	 * Returned value is in cell coordinates.
		**/
	int moveRange() const;

	/**
	 * @return Whether unit should attack any enemy units in range while moving.
	 **/
	bool moveAttacking() const;

	/**
	 * @return Whether unit should slow down (instead of immediately stopping)
	 *  before moving destination is reached.
	 **/
	bool slowDownAtDestination() const;

	/**
	 * @return Current pathinfo structure for this unit.
	 * Note that this is quite internal and shouldn't be used by anything else
	 * than Unit and it's inheritants if possible.
	 * It may be made protected later.
	 **/
	BosonPathInfo* pathInfo() const;


	/**
	 * @return TRUE if this unit is next to unit (i.e. less than one cell
	 * distance) otherwise FALSE
	 **/
	bool isNextTo(Unit* unit) const;

	void playSound(UnitSoundEvent event);

	void loadWeapons();

	bool canShootAt(Unit* u);

	/**
	 * Attack enemy units in range
	 * @return whether there was any enemy in range
	 **/
	bool attackEnemyUnitsInRange();

	/**
	 * @return Best enemy unit in range to attack
	 **/
	Unit* bestEnemyUnitInRange();

	/**
	 * @return Square of distance between center points of this unit and u
	 **/
	float distance(const Unit* u) const;
	/**
	 * @return Square of distance between center point of this unit and pos
	 **/
	float distance(const BoVector3Fixed& pos) const;

	/**
	 * @return Weapon with given id for this unit. 0 if it doesn't exist.
	 **/
	BosonWeapon* weapon(unsigned long int id) const;


	virtual void setMovingStatus(MovingStatus m);


protected:
	void shootAt(BosonWeapon* w, Unit* target);

	/**
	 * @return a list of interesting collisions, i.e. no non-units, no
	 * destryed units, ...
	 **/
	QValueList<Unit*> unitCollisions(bool exact = false);

	/**
	 * Finds new path to destination.
	 * Destination must have been set before in variables movedestx and
	 * movedesty - usually using @ref moveTo
	 *
	 * This is in Unit instead of @ref MobileUnit so that we can apply a
	 * path to newly constructed units of factories.
	 **/
	virtual void newPath();

	/**
	 * Move the unit. By default this does nothing. Reimplemented in @ref
	 * MobileUnit
	 **/
	virtual void advanceMoveInternal(unsigned int) { }

	/**
	 * Also reimplemented in @ref MobileUnit. Used to check whether the path
	 * calculated by @ref advanceMove was actually valid.
	 **/
	virtual void advanceMoveCheck() { }


	/**
	 * Resets internal pathfinder's info structure
	 **/
	void resetPathInfo();


	/**
	 * Adds so-called path point for this unit.
	 * Path points are like waypoints, except that they are internal.
	 **/
	void addPathPoint(const BoVector2Fixed& pos);
	/**
	 * @return Number of path-points
	 **/
	unsigned int pathPointCount() const;
	/**
	 * @return Current path-point (where unit should go next)
	 **/
	const BoVector2Fixed& currentPathPoint();
	/**
	 * Clears list of path-points
	 **/
	void clearPathPoints();
	/**
	 * Removes first path-point from the list
	 **/
	void pathPointDone();


	/**
	 * Called when the unit wants to move to another position. It may be
	 * possible that the heights of the cells at the new position are
	 * different and therefore the unit needs to move in z direction and
	 * maybe rotate around x and y axis.
	 *
	 * Currently rotateX and rotateY are set to 0.0, since it is not yet
	 * implemented. moveZ is simply set to the highest z value of the
	 * corners of all cells it occupies.
	 **/
	void updateZ(bofixed moveByX, bofixed moveByY, bofixed* moveZ, bofixed* rotateX, bofixed* rotateY);

	virtual const QColor* teamColor() const;


private:
	typedef void (Unit::*MemberFunction)(unsigned int advanceCallsCount);
	void setAdvanceFunction(MemberFunction, bool advanceFlag);

private:
	class UnitPrivate;
	UnitPrivate* d;
	UnitPlugin* mCurrentPlugin;

	static bool mInitialized; // whether initStatic() was called or not

	// now we come to the interesting slightly hackish performance part :-)
	// we store a pointer to the advance*() method that is currently used.
	// we do NULL checking on this pointer - but it *must not* happen, that
	// this pointer is null (advanceNone instead).
	// we store 2 pointers - see advanceFunction2() on this.
	MemberFunction mAdvanceFunction;
	MemberFunction mAdvanceFunction2;

	// will save us some trouble :)
	friend class KGameUnitDebug;
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
	MobileUnit(const UnitProperties* prop, Player* owner, BosonCanvas* canvas);
	virtual ~MobileUnit();

	virtual bool init();

	/**
	 * Turn to direction. This sets a new frame according to the new
	 * direction.
	 **/
	virtual void turnTo(int degrees);

	/**
	 * Call turnTo according to the current speed (you want to use this!)
	 **/
	void turnTo();


	virtual void advanceFollow(unsigned int advanceCallsCount);


	/**
	 * Move this unit to the repairYard and repair it there.
	 * TODO: move to plugin
	 **/
	void repairAt(Facility* repairYard);

	virtual BoRectFixed boundingRect() const;

	virtual bool saveAsXML(QDomElement& root);
	virtual bool loadFromXML(const QDomElement& root);

	virtual void stopMoving();

	bool attackEnemyUnitsInRangeWhileMoving();

	/**
	 * Move towards p, going at most maxdist (in canvas coords).
	 * How much unit should move, will be added to xspeed and yspeed.
	 * (x; y) marks unit's current position
	 *
	 * @return How much is moved (will be <= maxdist)
	 **/
	bofixed moveTowardsPoint(const BoVector2Fixed& p, bofixed x, bofixed y, bofixed maxdist, bofixed &xspeed, bofixed &yspeed);

	virtual void newPath();

	/**
	 * Check if pathpoint p marks end of the path. If yes, then it stops unit,
	 * turns to random direction and true, otherwise returns false.
	 **/
	bool checkPathPoint(const BoVector2Fixed& p);

protected:
	virtual void advanceMoveInternal(unsigned int advanceCallsCount); // move one step futher to path

	/**
	 * Note: this is not actually an advance*() method, like @ref
	 * advanceWork and the like. advanceMoveCheck() must get called
	 * (manually) after any advance*() method that moves a unit.
	 *
	 * This is most notably @ref advanceMove
	 **/
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
/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class Facility : public Unit
{
public:
	Facility(const UnitProperties* prop, Player* owner, BosonCanvas* canvas);
	virtual ~Facility();

	virtual bool init();

	/**
	 * The construction steps are the number of frames until the complete
	 * pixmap of the facility is shown.
	 * @return The number of available construction steps for a facility.
	 **/
	unsigned int constructionSteps() const;

	/**
	 * Please note that the construction state of a unit specifies if a unit
	 * <em>has been built</em> completely - it has nothing to do with the
	 * productions of a facility!
	 * @return If this unit has been built (constructed) completely
	 **/
	bool isConstructionComplete() const;

	/**
	 * @return A percentage that describes how far the construction progress
	 * of this facility is.
	 **/
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
	virtual void moveTo(bofixed x, bofixed y, int range = 0);

	/**
	 * Advance the construction animation. This is usually called when
	 * placing the unit until the construction is completed. See @ref
	 * isConstructionComplete
	 **/
	virtual void advanceConstruction(unsigned int advanceCallsCount);

	/**
	 * @return NULL if the facility has not yet been fully constructed,
	 * otherwise @ref Unit::plugin
	 **/
	virtual UnitPlugin* plugin(int pluginType) const;

	virtual bool saveAsXML(QDomElement& root);
	virtual bool loadFromXML(const QDomElement& root);

private:
	class FacilityPrivate;
	FacilityPrivate* d;
};

#endif
