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
#ifndef UNITBASE_H
#define UNITBASE_H

#include "rtti.h"

#include <kgame/kgameproperty.h>

class KGamePropertyHandler;

class QPoint;
class QRect;
class QDataStream;

class Player;
class UnitProperties;
class SpeciesTheme;

/**
 * Ok, ok - this class is useless. All it does is providing a lot of properties
 * and the basic stuff of all units. But there is no point for an own class.
 * Maybe we'll put all of this directly to @ref Unit. But when I created the
 * basic design I still thought of some inheritance but never made this true. 
 *
 * So here you can find most of the stuff about the unit, like health, weapon
 * damage, sight range and so on. But all the useful stuff is in @ref Unit.
 * @short The base class of all Units.
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class UnitBase
{
public:
	/**
	 * Property IDs for the @ref KGameProperty's. Note that you can change
	 * the numbers without any problem as long as all numbers appear only
	 * once and are greater than (or equal) @ref KGamePropretyBase::IdUser.
	 * All Ids Below are inernal KGame IDs (although we don't have to
	 * care as that matters only for @ref KGame and @ref KPlayer - but let's
	 * go the save way).
	 **/
	enum PropertyIds {
		IdHealth = KGamePropertyBase::IdUser + 0,
		IdArmor = KGamePropertyBase::IdUser + 1,
		IdShields = KGamePropertyBase::IdUser + 2,
		IdId = KGamePropertyBase::IdUser + 3,
		IdWork = KGamePropertyBase::IdUser + 6,
		IdAdvanceWork = KGamePropertyBase::IdUser + 7,
		IdSpeed = KGamePropertyBase::IdUser + 8,
		IdWeaponDamage = KGamePropertyBase::IdUser + 9,
		IdWeaponRange = KGamePropertyBase::IdUser + 10,
		IdSightRange = KGamePropertyBase::IdUser + 12,
		IdDeletionTimer = KGamePropertyBase::IdUser + 13,
		IdReloadState = KGamePropertyBase::IdUser + 14,
		//...
		IdLast
	};

	/**
	 * What is this unit currently doing
	 *
	 * Possible value are
	 * @li WorkNone - The unit does nothing
	 * @li WorkProduce - a facility is producing something
	 * @li WorkMove A unit is currently moving
	 * @li WorkMine - a mining unit is working...
	 * @li WorkAttack - Currently attacks a unit
	 * @li WorkConstructed - Is <em>being</em> constructed
	 * @li WorkMoveInGroup - Unit is moving in group (following leader)
	 **/
	enum WorkType {
		WorkNone = 0,
		WorkProduce = 1,
		WorkMove = 2,
		WorkMine = 3,
		WorkAttack = 4,
		WorkConstructed = 5,
		WorkMoveInGroup = 6,
		WorkDestroyed = 7,
		WorkRefine = 8
	};
	
	UnitBase(const UnitProperties* prop);
	virtual ~UnitBase();

	/**
	 * Change what this unit is currently doing.
	 **/
	void setWork(WorkType w) 
	{ 
		mWork = w;
		setAdvanceWork(w); 
	}

	/**
	 * Change what this unit is currently doing.
	 *
	 * The difference to @ref setWork is important in @ref
	 * BosonCanvas::slotAdvance. This method decides what the unit is meant
	 * to do (concerning the Unit::advanceXYZ() methods) depending on @ref
	 * advanceWork. This is usually the same as @ref work, but think e.g. of
	 * a unit that should mine minerals but first needs to move to the
	 * minerals field. It needs to have @ref WorkMine but to make moving
	 * work it needs to have @ref advanceWork == @ref WorkMove.
	 *
	 * You need to change this only very seldom.
	 **/
	virtual void setAdvanceWork(WorkType w) { mAdvanceWork = w; }

	/**
	 * @return What this unit is currently doing. See @ref WorkTyp on
	 * information what this can be.
	 **/
	inline WorkType work() const { return (WorkType)mWork.value(); }

	/**
	 * See also @ref setAdvanceWork, where usage is explained. You should
	 * not use this, use @ref work instead!
	 * @return Usually the same as @ref work, but sometime the advanceWork
	 * differs from the actual @ref work.
	 **/
	inline WorkType advanceWork() const { return (WorkType)mAdvanceWork.value(); }

	/**
	 * @return Guess what? See @ref UnitProperties::name
	 **/
	const QString& name() const;

	/**
	 * Health aka hitpoints
	 *
	 * 0 means destroyed.
	 *
	 * This could be replaced in derived classes for mobile units which can
	 * change into a facility. Then they could have more health? Or just
	 * armor?
	 * @return The health of the unit.
	 **/
	inline virtual unsigned long int health() const { return mHealth; };

	/**
	 * Change the health/power of this unit.
	 **/
	inline virtual void setHealth(unsigned long int h) { mHealth = h; }

	bool isDestroyed() const
	{
		return (health() == 0);
	}

	/**
	 * @return The owner (player) of the unit
	 **/
	inline Player* owner() const { return mOwner; }

	/**
	 * Set the owner of this unit. Note that this should be done on
	 * construction only! We do not yet support changing the owner!
	 **/
	void setOwner(Player* owner);

	KGamePropertyHandler* dataHandler() const;

	/**
	 * The ID of the unit. This ID is unique for this game.
	 * @return The uniuque ID of the unit
	 **/
	unsigned long int id() const;

	/**
	 * Set the ID of this unit. A ID must be unique for the owner, so it
	 * must be ensured that a ID exists only once per player. Should be done
	 * on construction only.
	 **/
	void setId(unsigned long int id);

	unsigned long int shields() const;
	void setShields(unsigned long int shields);

	unsigned long int armor() const;
	void setArmor(unsigned long int armor);

	/**
	 * The type of the unit as described in the index.desktop file of this
	 * unit. See also @ref UnitProperties::typeId
	 **/
	int type() const;

	/**
	 * @return The RTTI of this unit. You can use @ref RTTI::isUnit to find
	 * out if a @ref QCanvasSprite is a unit.
	 **/
	inline virtual int rtti() const { return RTTI::UnitStart + (int)type(); }

	/**
	 * @return How much damage the weapon can make to other units. Note that
	 * a negative value means that this unit can repair!
	 **/
	inline long int weaponDamage() const { return mWeaponDamage; }

	/**
	 * Change the damage this unit can do to other units
	 **/
	void setWeaponDamage(long int d) { mWeaponDamage = d; }
	
	/**
	 * @return The weapon range of this unit. This is a number of cells, so you
	 * must *= BO_TILE_SIZE to use this on the canvas.
	 **/
	inline unsigned long int weaponRange() const { return mWeaponRange; }

	/**
	 * Change the weapong range of this unit. This is number of cells.
	 **/
	void setWeaponRange(unsigned long int r) { mWeaponRange = r; }
	
	/**
	 * @return How far this unit can see. This is a number of cells, so you
	 * must *= BO_TILE_SIZE to use this on the canvas.
	 **/
	inline unsigned int sightRange() const { return mSightRange; }
	void setSightRange(unsigned int r) { mSightRange = r; }

	/**
	 * @return The speed of the unit. Must be replaced in derived classes to
	 * be of use as this just return 0.
	 **/
	inline virtual float speed() const { return 0.0; }
	virtual void setSpeed(float ) { }

	/**
	 * Save the unit to a stream. You can use @ref load to load the same
	 * unit again. Note that if derived classes add properties which are no
	 * @ref KGameProperty they must replace this function.
	 **/
	virtual bool save(QDataStream& stream);

	/**
	 * Load a unit from a stream. Note that just like @ref save derived
	 * classes must replace this if they add non-KGameProperty properties.
	 **/
	virtual bool load(QDataStream& stream);

	/**
	 * These are <em>not</em> the @ref KGameProperties! See @ref dataHandler
	 * for these.
	 *
	 * The @ref UnitProperties describes a unit type generally. This
	 * includes the name of tha unit as well as all initial values.
	 *
	 * The @ref UnitProperties which you get is the same that you provided
	 * in the constructor. Note that if you get NULL here you don't have to
	 * care about crashes as the game will crash anyway.
	 **/
	const UnitProperties* unitProperties() const { return mUnitProperties; }

	/**
	 * Convenience method for owner()->speciesTheme().
	 * See @ref Player::speciesTheme
	 * @return The @ref SpeciesTheme of the owner of this unit.
	 **/
	SpeciesTheme* speciesTheme() const;

	/**
	 * Cenvenience method for unitProperties()->isFacility().
	 * See @ref UnitProperties::isFacility
	 **/
	bool isFacility() const;
	/**
	 * Cenvenience method for unitProperties()->isMobile().
	 * See @ref UnitProperties::isMobile
	 **/
	bool isMobile() const;

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

	void increaseDeletionTimer();
	unsigned int deletionTimer() const;

	/**
	 * @return true if unit is moving (work() == WorkMove || work() == WorkMoveInGroup)
	 **/
	inline bool isMoving() 
	{ 
		return (work() == WorkMove || work() == WorkMoveInGroup); 
	}

	/**
	 * @return THe reload state of the weapon. If this is 0 the unit can
	 * shoot, otherwise not.
	 **/
	unsigned int reloadState() const;

	/**
	 * Reduces @ref reloadState by 1 until it reaches 0.
	 **/
	void reloadWeapon();

	/**
	 * Set @ref reloadState to @ref UnitProperties::reload
	 **/
	void resetReload();

private:
	class UnitBasePrivate;
	UnitBasePrivate* d;
	
	Player* mOwner;

	KGameProperty<unsigned long int> mHealth;
	KGameProperty<unsigned long int> mWeaponRange;
	KGameProperty<unsigned int> mSightRange;
	KGameProperty<long int> mWeaponDamage; // can also be repair (negative value)
	KGamePropertyInt mWork;
	KGamePropertyInt mAdvanceWork;

	const UnitProperties* mUnitProperties;
};

#endif
