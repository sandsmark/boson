/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2006 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2006 Rivo Laks (rivolaks@hot.ee)

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
#include "../global.h"

#include <qvaluevector.h>

class Player;
class BosonCanvas;
class BoItemList;
class UnitProperties;
class Cell;
class ProductionPlugin;
class UnitPlugin;
class RepairPlugin;
class BosonWeapon;
class BosonMoveData;
class BosonPathInfo;
class UnitMover;
class UnitMoverFlying;
class UnitMoverLand;
class UnitMoverInsideUnit;
class UnitConstruction;
class UnitOrder;
class UnitOrderData;
template<class T> class BoVector2;
template<class T> class BoVector3;
template<class T> class BoRect2;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoVector3<bofixed> BoVector3Fixed;
typedef BoRect2<bofixed> BoRect2Fixed;
template<class T> class QValueList;
template<class T> class QPtrList;
class QDomElement;

class KGameUnitDebug;


/**
 * A small helper class that establishes an interfaace between @ref
 * UnitOrderQueue and @ref Unit.
 *
 * This interface handles the direction UnitOrderQueue->Unit only, i.e. it
 * contains methods that may be called by UnitOrderQueue in order to make Unit
 * do something. The other direction is not necessary, as @ref Unit can call
 * methods of @ref UnitOrderQueue directly.
 *
 * This interface is required so that the methods can be protected or private
 * in @ref Unit, yet @ref UnitOrderQueue can still call them.
 * @ref Unit is meant to derive (using protected or private inheritance!)
 * from this class.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class UnitOrdersInterface
{
public:
	virtual ~UnitOrdersInterface()
	{
	}

	virtual bool currentOrderChanged() = 0;
	virtual bool currentOrderAdded() = 0;
	virtual void currentSuborderRemoved() = 0;

	/**
	 * @return TRUE if another order can be added (usually). Otherwise
	 * FALSE, e.g. if the construction of the facility is not yet completed.
	 **/
	virtual bool canAddOrder() const = 0;
};


class UnitPrivate;
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
class Unit : public UnitBase, private UnitOrdersInterface
{
public:
	enum PropertyIds {
		// properties in Unit. IDs from 1024 to 1279 may be used here
		// (1024+0 .. 1024+255)
		IdWantedRotation = 1024 + 6,
		IdPathPoints = 1024 + 10,
		IdIsInsideUnit = 1024 + 11,
		IdIsFlying = 1024 + 12,

		// properties in MobileUnit or Facility.
		// IDs from 1280 to 1535 may be used here
		// (1280+0 .. 1280+255)
		IdConstructionStep = 1280 + 100,

		// properties in UnitPlugins and derived classes
		// IDs from 1536 to 4095 may be used here
		// (1536+0 .. 1536+3071)
		IdProductionState = 1536 + 0,
		IdResourcesMined = 1536 + 1,
		IdResourcesX = 1536 + 2,
		IdResourcesY = 1536 + 3,
		IdHarvestingType = 1536 + 4,
		IdBombingTargetX = 1536 + 5,
		IdBombingTargetY = 1536 + 6,
		IdMinePlacingCounter = 1536 + 7,
		IdResourceMineMinerals = 1536 + 8,
		IdResourceMineOil = 1536 + 9,
		IdBombingDropDist = 1536 + 10,
		IdBombingLastDistFromDropPoint = 1536 + 11,
		IdMineralsPaid = 1536 + 12,
		IdOilPaid = 1536 + 13,
		IdEnterPointOutside1 = 1536 + 14,
		IdEnterPointOutside2 = 1536 + 15,
		IdEnterUnitRemainingInsidePath = 1536 + 16,
		IdEnterUnitPathIndex = 1536 + 17,
		IdMovingInStatus = 1536 + 18,
		IdLandingStatus = 1536 + 19,
		IdEnterUnitTriedMovingCounter = 1536 + 20,
		IdStoringStatus = 1536 + 21,
		IdPendingEnterRequests = 1536 + 22,
		IdApprovedEnterRequests = 1536 + 23,
		IdPendingLeaveRequests = 1536 + 24,
		IdApprovedLeaveRequests = 1536 + 25,
		IdEnteringUnitOnPath = 1536 + 26
	};

	Unit(const UnitProperties* prop, Player* owner, BosonCanvas* canvas);
	virtual ~Unit();

	virtual bool init();

	static void initStatic();

	inline virtual int rtti() const { return UnitBase::rtti(); }

	void setMoveData(BosonMoveData* moveData);
	BosonMoveData* moveData() const;

	virtual void setHealth(unsigned long int h);

	virtual void setSightRange(unsigned long int r);

	/**
	 * @return The current plugin, used in e.g. @ref advance or NULL if
	 * none. Always NULL if @ref work != WorkPlugin
	 **/
	inline UnitPlugin* currentPlugin() const { return mCurrentPlugin; }

	/**
	 * @return The @ref UnitPlugin::pluginType of the @ref currentPlugin or
	 * 0 if there is no plugin.
	 **/
	int currentPluginType() const;

	/**
	 * @param pluginType See @ref UnitPlugin::UnitPlugins
	 **/
	virtual UnitPlugin* plugin(int pluginType) const;

	void setPluginWork(int pluginType);

	UnitConstruction* construction() const
	{
		return mUnitConstruction;
	}

	/**
	 * Called when @p unit is destroyed. If the unit is a target of this
	 * unit, it can now decide to reset the target.
	 *
	 * Note that @p unit usually will remain on the map as a wreckage for a
	 * certain time. @ref itemRemoved is called when it is finally removed
	 * completely.
	 **/
	virtual void unitDestroyed(Unit* unit);

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
	 * Reload shields and weapons. @p count specifies how much reloading
	 * should be done (note that this is <em>not</em> the advanceCallsCount
	 **/
	virtual void reload(unsigned int count);

	/**
	 * Called by @ref BosonWeapon to actually reload the
	 * ammunition of the weapon.
	 *
	 * @return A value between 0 and @p requested, representing the amount
	 * by which the weapon gets refilled.
	 **/
	unsigned long int requestAmmunition(const QString& type, unsigned long int requested);

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
	 * Moves the unit.
	 **/
	virtual void advanceMove(unsigned int advanceCallsCount);


	/**
	 * Attack a unit. The target was set before using @ref setTarget
	 **/
	virtual void advanceAttack(unsigned int advanceCallsCount);

	/**
	 * Follow another unit.
	 * Note that this method is somewhat similar to @ref advanceAttack
	 **/
	virtual void advanceFollow(unsigned int);

	/**
	 * This is called when the unit is supposed to do nothing.
	 *
	 * Note that the game may decide _not_ to call this advance function at
	 * all, as it is a noop anyway. Do NOT implement anything here.
	 **/
	virtual void advanceNone(unsigned int advanceCallsCount);

	/**
	 * This is called when there is nothing else to do for this unit.
	 * Usually the unit will check for enemy units in range and fire at
	 * them.
	 **/
	virtual void advanceIdle(unsigned int advanceCallsCount);

	/**
	 * Move the construction animation one step forward. Does nothing by
	 * default - reimplemented in @ref Facility
	 **/
	virtual void advanceConstruction(unsigned int);

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

	bool inRange(unsigned long int, Unit* unit) const;

	/**
	 * @return List of path-points
	 **/
	const QValueList<BoVector2Fixed>& pathPointList() const;


	/**
	 * Turns unit smoothly to given degrees
	 * @return true if unit was turned immediately, false when suborder was
	 *  constructed to rotate the unit during next advance calls
	 **/
	virtual bool turnTo(bofixed degrees);

	/**
	 * Same as above, but turns to given unit
	 **/
	virtual bool turnToUnit(Unit* target);

	/**
	 * Updates unit's x- and y-rotation, so that it will be rotated accordingly to
	 * to the slope of the terrain it's on
	 **/
	void updateRotation();

	virtual bool saveAsXML(QDomElement& root);
	virtual bool loadFromXML(const QDomElement& root);

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

	void loadWeapons();

	bool canShootAt(Unit* u) const;

	/**
	 * @return TRUE if this unit can "crush" @p u, i.e. walk over it and
	 * destroy it by doing so. Otherwise FALSE. This method assumes that
	 * this unit actually collides with @p u. It does not check for it's
	 * position itself.
	 **/
	bool canCrush(Unit* u) const;

	/**
	 * Attack enemy units in range
	 * @return whether there was any enemy in range
	 **/
	Unit* attackEnemyUnitsInRange(Unit* target = 0);

	/**
	 * @return Best enemy unit in range to attack
	 **/
	Unit* bestEnemyUnitInRange();

	/**
	 * @return Square of distance between center points of this unit and u
	 **/
	bofixed distanceSquared(const Unit* u) const;
	/**
	 * @return Square of distance between center point of this unit and pos
	 **/
	bofixed distanceSquared(const BoVector3Fixed& pos) const;

	/**
	 * @return Weapon with given id for this unit. 0 if it doesn't exist.
	 **/
	BosonWeapon* weapon(unsigned long int id) const;

	/**
	 * @return maximum range of weapons of this unit e.g. range of weapon with the longest range
	 **/
	unsigned long int maxWeaponRange() const;
	unsigned long int maxAirWeaponRange() const;
	unsigned long int maxLandWeaponRange() const;


	virtual void setMovingStatus(MovingStatus m);

	virtual void addUpgrade(const UpgradeProperties* upgrade);
	virtual void removeUpgrade(const UpgradeProperties* upgrade);

	/**
	 * You can call this when unit is idle and could move
	 * It makes flying units fly around in circles
	 **/
	void moveIdle();

	/**
	 * Called when the unit wants to move to another position. It may be
	 * possible that the heights of the cells at the new position are
	 * different and therefore the unit needs to move in z direction and
	 * maybe rotate around x and y axis.
	 **/
	void updateZ(bofixed moveByX, bofixed moveByY, bofixed* moveZ, bofixed* rotateX, bofixed* rotateY);

	/**
	 * @return UnitOrder object specifying currently executed order or 0 if unit
	 *  doesn't have any orders atm.
	 **/
	UnitOrder* currentOrder() const;
	UnitOrderData* currentOrderData() const;
	/**
	 * @return Current toplevel order for this unit. As orders can have
	 *  suborders, this can be different from @ref currentOrder;
	 **/
	UnitOrder* toplevelOrder() const;
	UnitOrderData* toplevelOrderData() const;

	/**
	 * Clears all orders that units has (including scheduled ones)
	 **/
	void clearOrders();

	/**
	 * Adds new toplevel order for this unit. If the unit already has
	 *  @ref toplevelOrder, then the new order becomes scheduled, i.e. it will be
	 *  carried out once all previously scheduled orders have been completed
	 **/
	bool addToplevelOrder(UnitOrder* order);

	/**
	 * Replaces all current order with the given one.
	 * This is mostly equal to calling  clearOrders(); addToplevelOrder(order);
	 **/
	bool replaceToplevelOrders(UnitOrder* order);

	/**
	 * Adds new suborder to @ref currentOrder of this unit. Given suborder will
	 *  become new @ref currentOrder . This is mostly meant for internal use.
	 * @return Whether it is possible to fulfill given order. E.g. if you order
	 *  unit to move to unpassable terrain, it return false
	 **/
	bool addCurrentSuborder(UnitOrder* order);

	/**
	 * Notify the unit that the suborder that is currently being executed
	 * has been completed. @p success indicates whether the order was
	 * completed successfully or not (e.g. unit could not reach the desired
	 * destination)
	 **/
	void currentSuborderDone(bool success);

	/**
	 * @return TRUE if the last executed order was completed successfully,
	 * FALSE if it failed.
	 **/
	bool lastOrderStatus() const;

	/**
	 * Set whether this unit is inside a different unit.
	 *
	 * See @ref isInsideUnit and @ref EnterUnitPlugin.
	 **/
	void setIsInsideUnit(bool isInside);

	/**
	 * Some (mobile) units can move inside units that have a @ref
	 * UnitStoragePlugin. Units with such a plugin can be e.g. a
	 * transportation unit, a repairyard or an airport.
	 *
	 * If this unit is considered to be "inside" such a unit, this method
	 * returns true, otherwise false. This method considers a unit "inside"
	 * a unit, if the @ref UnitStoragePlugin has taken control of this unit
	 * and in particular of movements of this units, this means that even if
	 * this unit is still physically outside the destination unit, it is
	 * considered "inside" already, if it has started to move in.
	 *
	 * @return TRUE if this unit is "inside" a different unit, otherwise
	 * FALSE.
	 **/
	bool isInsideUnit() const;

	/**
	 * One day we might have units which can go on air <em>and</em> on land
	 * or which just can land. Examples might be helicopters or planes at an
	 * airport. For these units we need to know whether they are flying or
	 * not.
	 * @return Whether this unit is currently flying. Always false if @ref
	 * unitProperties()->isAircraft is false. Currently this is juste the
	 * same as @ref unitProperties()->isAircraft.
	 **/
	bool isFlying() const;

	void setIsFlying(bool f);


protected:
	void shootAt(BosonWeapon* w, Unit* target);

	/**
	 * @return a list of interesting collisions, i.e. no non-units, no
	 * destryed units, ...
	 **/
	QValueList<Unit*> unitCollisions(bool exact = false);

	/**
	 * Helper method to @ref advanceIdle.
	 *
	 * This does basic tasks common to all units
	 **/
	void advanceIdleBasic(unsigned int advanceCallsCount);

	/**
	 * If this unit has a unit storage, it does some additional "idle" tasks.
	 **/
	void advanceIdleUnitStorage(unsigned int advanceCallsCount);


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


	virtual const QColor* teamColor() const;

	void recalculateMaxWeaponRange();

	// TODO: move to BosonCanvas or to somewhere else
	bool cellOccupied(int x, int y, bool ignoremoving = false) const;

	virtual int getAnimationMode() const;

	/**
	 * Internal moving method.
	 * This is the most central method for having unit starting moving to specific
	 * point. You shouldn't use this directly, but instead use 'client methods',
	 * which will then call this one.
	 *
	 * Moves unit's _center_ exactly to given position. If range is not -1, unit
	 * will move until it's at most that range _cells_ away from destination.
	 * Destination is in canvas coords.
	 *
	 * See also @ref addToplevelOrder and @ref addCurrentSuborder as well as
	 * @ref UnitMoveOrder
	 **/
	bool moveTo(bofixed x, bofixed y, int range = -1);

	UnitMover* unitMover() const;

private:
	/**
	 * See @ref UnitOrdersInterface
	 **/
	virtual bool currentOrderChanged();
	/**
	 * See @ref UnitOrdersInterface
	 **/
	virtual bool currentOrderAdded();
	/**
	 * See @ref UnitOrdersInterface
	 **/
	virtual void currentSuborderRemoved();
	/**
	 * See @ref UnitOrdersInterface
	 **/
	virtual bool canAddOrder() const;


private:
	typedef void (Unit::*MemberFunction)(unsigned int advanceCallsCount);
	void setAdvanceFunction(MemberFunction, bool advanceFlag);

	friend class UnitMover;
	friend class UnitMoverFlying;
	friend class UnitMoverLand;

private:
	UnitPrivate* d;
	UnitPlugin* mCurrentPlugin;
	UnitConstruction* mUnitConstruction;

	static bool mInitialized; // whether initStatic() was called or not

	// now we come to the interesting slightly hackish performance part :-)
	// we store a pointer to the advance*() method that is currently used.
	// we do NULL checking on this pointer - but it *must not* happen, that
	// this pointer is null (advanceIdle instead).
	// we store 2 pointers - see advanceFunction2() on this.
	MemberFunction mAdvanceFunction;
	MemberFunction mAdvanceFunction2;

	// will save us some trouble :)
	friend class KGameUnitDebug;
};


/**
 * Construction of a facility
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class UnitConstruction
{
public:
	UnitConstruction(Unit* f);
	~UnitConstruction();

	/**
	 * Advance the construction animation. This is usually called when
	 * placing the unit until the construction is completed. See @ref
	 * isConstructionComplete.
	 *
	 * Do NOT call this manually, but let the internal unit advance
	 * mechanism do that!
	 **/
	void advanceConstruction(unsigned int advanceCallsCount);

	/**
	 * Used by to initialize the scenario. A scenario file can specify that
	 * a facility is already completed - to do so we can
	 * setContructionStep(constructionSteps() - 1).
	 *
	 * This should <em>not</em> be called once the game is started
	 **/
	void setConstructionStep(unsigned int step);

	/**
	 * Please note that the construction state of a unit specifies if a unit
	 * <em>has been built</em> completely - it has nothing to do with the
	 * productions of a facility!
	 * @return If this unit has been built (constructed) completely
	 **/
	bool isConstructionComplete() const;

	/**
	 * The construction steps are the number of frames until the complete
	 * pixmap of the facility is shown.
	 * @return The number of available construction steps for a facility.
	 **/
	unsigned int constructionSteps() const;

	unsigned int currentConstructionStep() const;

	/**
	 * @return A percentage that describes how far the construction progress
	 * of this facility is.
	 **/
	double constructionProgress() const;

	bool loadFromXML(const QDomElement& root);
	bool saveAsXML(QDomElement&);

	Unit* unit() const
	{
		return mFacility;
	}

private:
	Unit* mFacility;
	KGameProperty<Q_UINT32> mConstructionStep;
};


#endif
