/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "items/bosonitem.h"

#include <kgame/kgameproperty.h>

class QPoint;
class QRect;
class QDataStream;
class KGamePropertyHandler;
class QDomElement;
template<class T, class T2> class QMap;

class Player;
class PlayerIO;
class BosonCanvas;
class UnitProperties;
class PluginProperties;
class SpeciesTheme;
class BosonItemPropertyHandler;

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
class UnitBase : public BosonItem
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
		// properties in UnitBase
		IdHealth = KGamePropertyBase::IdUser + 0,
		IdArmor = KGamePropertyBase::IdUser + 1,
		IdShields = KGamePropertyBase::IdUser + 2,
		IdShieldReloadCounter = KGamePropertyBase::IdUser + 3,
		IdSightRange = KGamePropertyBase::IdUser + 5,
		IdWork = KGamePropertyBase::IdUser + 10,
		IdAdvanceWork = KGamePropertyBase::IdUser + 11,
		IdMovingStatus = KGamePropertyBase::IdUser + 12,
		IdDeletionTimer = KGamePropertyBase::IdUser + 15,

		// last entry.
		IdLast
	};

	/**
	 * What is this unit currently doing
	 *
	 * Possible value are
	 * @li WorkNone - The unit does nothing
	 * @li WorkMove A unit is currently moving
	 * @li WorkAttack - Currently attacks a unit
	 * @li WorkConstructed - Is <em>being</em> constructed
	 * @li WorkDestroyed - Is destroyed
	 * @li WorkFollow - Is following another unit
	 * @li WorkPlugin - a @ref UnitPlugin is currently used
	 * @li WorkTurn - is turning
	 **/
	enum WorkType {
		WorkNone = 0,
		WorkMove = 2,
		WorkAttack = 4,
		WorkConstructed = 5,
		WorkDestroyed = 6,
		WorkFollow = 8,
		WorkPlugin = 9,
		WorkTurn = 10
	};

	/**
	 * Describes moving status of the unit
	 * Note that if status is anything but Standing, then it means that unit is
	 * either moving or intends to do so ASAP.
	 *
	 * @li Standing Unit is standing (not moving nor intending to do so)
	 * @li Moving Unit is moving atm (or turning)
	 * @li Waiting Unit wants to move, but it's way is blocked atm. It will continue moving asap
	 * @li Engaging Unit was moving, but then spotted enemy and is engaging it.
	 * @li Removing Special status notifing that unit is being deleted
	 * @li MustSearch Special status notifing that path is not yet searched (but unit intends to move)
	 **/
	enum MovingStatus {
		Standing = 0,
		Moving = 1,
		Waiting = 2,
		Engaging = 3,
		Removing = 4,
		MustSearch = 5
	};
	
	UnitBase(const UnitProperties* prop, Player* owner, BosonCanvas* canvas);
	virtual ~UnitBase();

	/**
	 * Change what this unit is currently doing.
	 **/
	virtual void setWork(WorkType w)
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
	 * minerals field. It needs to have @ref WorkPlugin but to make moving
	 * work it needs to have @ref advanceWork == @ref WorkMove.
	 *
	 * You need to change this only very seldom.
	 **/
	virtual void setAdvanceWork(WorkType w) { mAdvanceWork = w; }

	/**
	 * @return What this unit is currently doing. See @ref WorkType on
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
	 * @return owner()->playerIO()
	 **/
	PlayerIO* ownerIO() const;

	/**
	 * @return The @ref KGamePropertyHandler for weapon properties. The
	 * handler is created if it does not yet exist.
	 **/
	KGamePropertyHandler* weaponDataHandler();

	inline unsigned long int shields() const { return mShields; }
	void setShields(unsigned long int shields);

	unsigned long int armor() const;
	void setArmor(unsigned long int armor);

	/**
	 * The type of the unit as described in the index.unit file of this
	 * unit. See also @ref UnitProperties::typeId
	 **/
	unsigned long int type() const;

	/**
	 * @return The RTTI of this unit.
	 **/
	inline virtual int rtti() const { return RTTI::UnitStart + (int)type(); }

	/**
	 * @return How far this unit can see. This is a number of cells, so you
	 * must *= BO_TILE_SIZE to use this on the canvas.
	 **/
	inline unsigned int sightRange() const { return mSightRange; }
	void setSightRange(unsigned int r) { mSightRange = r; }

	virtual bool saveAsXML(QDomElement& root);
	virtual bool loadFromXML(const QDomElement& root);

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
	 * Convenience method for unitProperties()->properties().
	 *
	 * Please note that @ref PluginProperties are somewhat different to @ref
	 * UnitPlugins. You can have @ref PluginProperties without @ref
	 * UnitPlugins and vice versa.
	 **/
	const PluginProperties* properties(int pluginType) const;

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


	virtual void setMovingStatus(MovingStatus m) { mMovingStatus = m; }

	/**
	 * @return What this unit is currently doing. See @ref WorkType on
	 * information what this can be.
	 **/
	inline MovingStatus movingStatus() const { return (MovingStatus)mMovingStatus.value(); }

//	inline QValueList<BosonWeapon>* weapons()  { return &mWeapons; };

	/**
	 * Save this unit for a scenario file. Note that this may and will
	 * change. Maybe it will save all values from the unit, maybe not -
	 * we'll see.
	 *
	 * This function adds a new node for every @ref KGamePropertyBase entry
	 * in the @ref dataHandler
	 **/
	virtual bool saveScenario(QDomElement& node);

protected:
	/**
	 * Should get called in every @ref Unit::advance call. This counts the
	 * calls and when the count exceed a certain value (currently 10) the
	 * @ref shields will be increased by 1.
	 * @param by How much the advance counter should get increased. can be
	 * used to allow faster shield reloading times.
	 **/
	void reloadShields(int by = 1);

private:
	static void initStatic();

private:
	KGameProperty<unsigned long int> mArmor;
	KGameProperty<unsigned long int> mShields;
	KGameProperty<unsigned long int> mShieldReloadCounter;
	KGameProperty<unsigned int> mDeletionTimer;
	KGameProperty<unsigned long int> mHealth;
	KGameProperty<unsigned int> mSightRange;
	KGamePropertyInt mWork;
	KGamePropertyInt mAdvanceWork;
	KGamePropertyInt mMovingStatus;

	BosonItemPropertyHandler* mWeaponProperties;

	const UnitProperties* mUnitProperties;

	friend class UpgradePropertiesBase;
};

#endif

