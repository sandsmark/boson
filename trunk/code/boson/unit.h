/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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
 * should go to UnitBase directly - except the visual stuff.
 *
 * Probably most things here can be moved to UnitBase. is a FIXME
 *
 * Not that Unit does <em>not</em> inherit @ref QObject! Signals/Slots are
 * therefore not possible!
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class Unit : public UnitBase, public QCanvasSprite
{
public:
	enum PropertyIds {
		IdDirection = UnitBase::IdLast + 1,
		IdWaypoints = UnitBase::IdLast + 2,
		IdFix_ConstructionState = UnitBase::IdLast + 3,
		IdFix_ConstructionDelay = UnitBase::IdLast + 4,
		IdReloadState = UnitBase::IdLast + 5,
		IdFix_Constructions = UnitBase::IdLast + 6

	};
	Unit(int type, Player* owner, QCanvas* canvas);
	virtual ~Unit();

	virtual int rtti() const { return UnitBase::rtti(); }

	void turnTo(int direction);

	virtual void setHealth(unsigned long int h);

	BosonCanvas* boCanvas() const { return (BosonCanvas*)canvas(); }

	void select();
	void unselect();

	virtual void moveBy(double x, double y);

	virtual void advance(int phase);

	void attackUnit(Unit* target);
	
	/**
	 * Move the unit. By default this does nothing. Reimplemented in @ref
	 * MobileUnit
	 **/
	virtual void advanceMove() { }

	/**
	 * Move the construction animation one step forward. Does nothing by
	 * default - reimplemented in Facility
	 **/
	virtual void beConstructed() { }

	Unit* target() const;
	void setTarget(Unit* target);
	bool inRange(Unit* unit) const;

// waypoint stuff: // also in facility - produced units receive this initial waypoint
	void addWaypoint(const QPoint& pos);
	const QPoint& currentWaypoint() const;
	unsigned int waypointCount() const;
	void clearWaypoints();
	void waypointDone();

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

	/**
	 * @return The absolute filename to the shooting sound of this unit.
	 **/
	QString soundShoot() const;

protected:
	/**
	 * @return a list of interesting collisions, i.e. no non-units, no
	 * destryed units, ...
	 **/
	QValueList<Unit*> unitCollisions(bool exact = false) const;

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
	MobileUnit(int type, Player* owner, QCanvas* canvas);
	virtual ~MobileUnit();

	virtual void advanceMove(); // move one step futher to path
	
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
	Facility(int type, Player* owner, QCanvas* canvas);
	virtual ~Facility();

	/**
	 * @return The number of available construction steps for a facility.
	 **/
	static int constructionSteps();

	/**
	 * @return The number of @ref advance calls to achieve another
	 * construction step. See @ref constructionSteps
	 **/
	int constructionDelay() const;

	/**
	 * Change the number of @ref advance calls needed to achieve another
	 * construction step. See @ref constructionDelay
	 **/
	void setConstructionDelay(int delay);
	
	virtual void beConstructed();

	/**
	 * @return Whether there are any constructions pending for this unit
	 **/
	bool hasConstruction() const;

	int completedConstruction() const;

	void removeConstruction(); // removes first item
	void addConstruction(int unitType);

private:
	class FacilityPrivate;
	FacilityPrivate* d;
};

#endif
