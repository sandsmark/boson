/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef __UNITBASE_H__
#define __UNITBASE_H__

#include "rtti.h"

#include <qstring.h>
#include <qdatastream.h>

#include <kgame/kgameproperty.h>

class KGamePropertyHandler;

class QPoint;
class QRect;

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
		IdId = KGamePropertyBase::IdUser + 3, // useful? perhaps use dataHandler()->id() instead
		IdCost = KGamePropertyBase::IdUser + 4,
//		IdType = KGamePropertyBase::IdUser + 5, // obsolete
		IdWork = KGamePropertyBase::IdUser + 6,
		IdSpeed = KGamePropertyBase::IdUser + 7,
		IdDamage = KGamePropertyBase::IdUser + 8,
		IdRange = KGamePropertyBase::IdUser + 9,
		IdReload = KGamePropertyBase::IdUser + 10,
		IdSightRange = KGamePropertyBase::IdUser + 11,
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
	 **/
	enum WorkType {
		WorkNone = 0,
		WorkProduce = 1,
		WorkMove = 2,
		WorkMine = 3,
		WorkAttack = 4,
		WorkConstructed = 5 
	};
	
	UnitBase(const UnitProperties* prop);
	virtual ~UnitBase();

	/**
	 * Change what this unit is currently doing.
	 **/
	void setWork(WorkType w) { mWork= w; }

	/**
	 * @return What this unit is currently doing. See @ref WorkTyp on
	 * information what this can be.
	 **/
	WorkType work() const { return (WorkType)mWork.value(); }

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
	virtual unsigned long int health() const { return mHealth; };

	/**
	 * Change the health/power of this unit.
	 **/
	virtual void setHealth(unsigned long int h) { mHealth = h; }

	bool isDestroyed() const
	{
		return (health() == 0);
	}

	/**
	 * @return The owner (player) of the unit
	 **/
	Player* owner() const { return mOwner; }

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
	inline int type() const;

	/**
	 * @return The RTTI of this unit. You can use @ref RTTI::isUnit to find
	 * out if a @ref QCanvasSprite is a unit.
	 **/
	virtual int rtti() const { return RTTI::UnitStart + (int)type(); }

	/**
	 * @return How much damage the weapon can make to other units. Note that
	 * a negative value means that this unit can repair!
	 **/
	long int damage() const { return mDamage; }

	/**
	 * Change the damage this unit can do to other units
	 **/
	void setDamage(long int d) { mDamage = d; }
	
	/**
	 * @return The weapon range of this unit
	 **/
	unsigned long int range() const { return mRange; }

	/**
	 * Change the weapong range of this unit
	 **/
	void setRange(unsigned long int r) { mRange = r; }
	
	/**
	 * @return The number of @ref Unit::advance calls unit the unit has
	 * reloaded its weapon.
	 **/
	unsigned int reload() const { return mReload; }
	void setReload(unsigned int r) { mReload = r; }

	/**
	 * @return How far this unit can see. This is a number of cells, so you
	 * must *= BO_TILE_SIZE to use this on the canvas.
	 **/
	unsigned int sightRange() const { return mSightRange; }
	void setSightRange(unsigned int r) { mSightRange = r; }

	/**
	 * @return The speed of the unit. Must be replaced in derived classes to
	 * be of use as this just return 0.
	 **/
	virtual double speed() const { return 0.0; }
	virtual void setSpeed(double ) { }

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
	inline SpeciesTheme* speciesTheme() const;

	/**
	 * Cenvenience method for unitProperties()->isFacility().
	 * See @ref UnitProperties::isFacility
	 **/
	inline bool isFacility() const;
	/**
	 * Cenvenience method for unitProperties()->isMobile().
	 * See @ref UnitProperties::isMobile
	 **/
	inline bool isMobile() const;

	/**
	 * One day we might have units which can go on air <em>and</em> on land
	 * or which just can land. Examples might be helicopters or planes at an
	 * airport. For these units we need to know whether they are flying or
	 * not.
	 * @return Whether this unit is currently flying. Always false if @ref
	 * unitProperties()->isAircraft is false. Currently this is juste the 
	 * same as @ref unitProperties()->isAircraft.
	 **/
	inline bool isFlying() const;
	
private:
	class UnitBasePrivate;
	UnitBasePrivate* d;
	
	Player* mOwner;

	KGameProperty<unsigned long int> mHealth;
	KGameProperty<unsigned long int> mRange;
	KGameProperty<unsigned int> mSightRange;
	KGameProperty<long int> mDamage; // can also be repair (negative value)
	KGameProperty<unsigned int> mReload;
	KGamePropertyInt mWork;

	const UnitProperties* mUnitProperties;
};

#endif
