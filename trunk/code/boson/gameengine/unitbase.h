/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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
#include "bosonitem.h"
#include "boupgradeableproperty.h"

#include <kgame/kgameproperty.h>

class KGamePropertyHandler;
class QDomElement;
template<class T, class T2> class QMap;
template<class T> class QValueList;

class Player;
class PlayerIO;
class BosonCanvas;
class UnitProperties;
class PluginProperties;
class SpeciesTheme;
class BosonItemPropertyHandler;
class UpgradeProperties;


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
		// UnitBase uses IDs from 512 to 1023.
		//IdHealth = 512 + 0,
		//IdArmor = 512 + 1,
		//IdShields = 512 + 2,
		IdShieldReloadCounter = 512 + 3,
		//IdSightRange = 512 + 5,
		//IdWork = 512 + 10,
		IdAdvanceWork = 512 + 11,
		IdMovingStatus = 512 + 12,
		IdDeletionTimer = 512 + 15,
		IdHealthFactor = 512 + 20,
		IdArmorFactor = 512 + 21,
		IdShieldsFactor = 512 + 22,
		IdSightRangeFactor = 512 + 23,
		IdPowerChargeForAdvance = 512 + 24,
		IdPowerChargeForReload = 512 + 25
	};

	/**
	 * What is this unit currently doing
	 *
	 * Possible value are
	 * @li WorkIdle - The unit has no specific orders
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
		// AB: only _append_ new numbers, do not insert something (when
		// numbers get removed they still may be referenced by
		// savegames)
		WorkIdle = 0,
		WorkMove = 2,
		WorkAttack = 4,
		WorkConstructed = 5,
		WorkDestroyed = 6,
		WorkFollow = 8,
		WorkPlugin = 9,
		WorkTurn = 10,
		WorkNone = 11
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

	virtual QString getModelIdForItem() const;

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

	unsigned long int maxHealth() const;

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
	virtual unsigned long int health() const;

	/**
	 * Change the health of this unit.
	 **/
	virtual void setHealth(unsigned long int h);

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

	unsigned long int shields() const;
	void setShields(unsigned long int shields);
	unsigned long int maxShields() const;

	unsigned long int armor() const;
	void setArmor(unsigned long int armor);
	unsigned long int maxArmor() const;

	unsigned long int powerConsumedByUnit() const;
	unsigned long int powerGeneratedByUnit() const;

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
	 * @return How far this unit can see. This is a number of cells
	 **/
	unsigned long int sightRange() const;
	virtual void setSightRange(unsigned long int r);
	unsigned long int maxSightRange() const;

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

	/**
	 * Charge the unit by a given @p factor. The factor must be a number
	 * between 0 and 1, where 0 is a noop and 1 means to completely charge
	 * the unit for this advance call.
	 *
	 * The factor is calculated by @ref Player, based on the amount of power
	 * this player currently has and the amount of power this player
	 * currently consumes.
	 *
	 * This method must not be called more than once per advance call.
	 *
	 * See also @ref unchargePowerForAdvance and @ref isChargedForAdvance
	 **/
	void chargePowerForAdvance(bofixed factor);
	void chargePowerForReload(bofixed factor);

	/**
	 * This method is called once per advance call @em after (!!) the tasks
	 * of this unit (i.e @ref advanceFunction) have been performed. If the
	 * unit was charged in this advance call, it is uncharged (the power was
	 * used), otherwise this method is a noop (unit is still charging).
	 **/
	void unchargePowerForAdvance();
	void unchargePowerForReload();

	/**
	 * Most probably you want to use @ref requestPowerChargeForAdvance
	 * instead!
	 *
	 * Note that this method is valid while an advance call is being
	 * processed only. It's value is undefined if called after or before an
	 * advance call.
	 *
	 * @return TRUE if this unit is fully charged, i.e. it can perform its
	 * tasks in this advance call. Otherwise FALSE - the unit is supposed to
	 * do NOTHING in the advance function that would require power. Note
	 * that @ref reload is independent from this - see @ref
	 * isChargedForReload
	 **/
	bool isChargedForAdvance() const
	{
		return mPowerChargeForAdvance >= 1;
	}
	bool isChargedForReload() const
	{
		return mPowerChargeForReload >= 1;
	}

	/**
	 * Request the unit to be charged for this advance call. This may be
	 * called once per advance call (at most) for a unit.
	 *
	 * @return TRUE if the unit is (after charging) fully charged, so it can
	 * perform it's advance action. Otherwise FALSE - the unit is not fully
	 * charged and must not perform any advance action that requires power.
	 **/
	bool requestPowerChargeForAdvance();

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

	const QValueList<const UpgradeProperties*>* upgrades() const;

	void clearUpgrades();
	virtual void addUpgrade(const UpgradeProperties* upgrade);
	virtual void removeUpgrade(const UpgradeProperties* upgrade);
	const BoUpgradesCollection& upgradesCollection() const
	{
		return mUpgradesCollection;
	}

	/**
	 * @return how many percentage of maximum health this unit has
	 * Note that the returned value is in range 0-1, not 0-100!
	 **/
	inline bofixed healthFactor() const { return mHealthFactor; }

	inline void setScheduledForSightUpdate(bool set)
	{
		mScheduledForSightUpdate = set;
	}
	inline bool isScheduledForSightUpdate() const
	{
		return mScheduledForSightUpdate;
	}

protected:
	/**
	 * Should get called in every @ref Unit::advance call. This counts the
	 * calls and when the count exceed a certain value (currently 10) the
	 * @ref shields will be increased by 1.
	 * @param by How much the advance counter should get increased. can be
	 * used to allow faster shield reloading times.
	 **/
	void reloadShields(int by = 1);

	inline void setHealthFactor(bofixed f) { mHealthFactor = f; }

	inline bofixed armorFactor() const { return mArmorFactor; }
	inline void setArmorFactor(bofixed f) { mArmorFactor = f; }

	inline bofixed shieldsFactor() const { return mShieldsFactor; }
	inline void setShieldsFactor(bofixed f) { mShieldsFactor = f; }

	inline bofixed sightRangeFactor() const { return mSightRangeFactor; }
	inline void setSightRangeFactor(bofixed f) { mSightRangeFactor = f; }

private:
	static void initStatic();

private:
	const UnitProperties* mUnitProperties;
	BoUpgradesCollection mUpgradesCollection;

	BosonItemPropertyHandler* mWeaponProperties;

	KGameProperty<unsigned long int> mShieldReloadCounter;
	KGameProperty<unsigned int> mDeletionTimer;
	KGamePropertyInt mAdvanceWork;
	KGamePropertyInt mMovingStatus;

	KGameProperty<bofixed> mHealthFactor;
	KGameProperty<bofixed> mArmorFactor;
	KGameProperty<bofixed> mShieldsFactor;
	KGameProperty<bofixed> mSightRangeFactor;

	KGameProperty<bofixed> mPowerChargeForAdvance;
	KGameProperty<bofixed> mPowerChargeForReload;


	BoUpgradeableProperty<unsigned long int> mMaxHealth;
	BoUpgradeableProperty<unsigned long int> mMaxArmor;
	BoUpgradeableProperty<unsigned long int> mMaxShields;
	BoUpgradeableProperty<unsigned long int> mMaxSightRange;
	BoUpgradeableProperty<unsigned long int> mPowerGenerated;
	BoUpgradeableProperty<unsigned long int> mPowerConsumed;


	bool mAdvanceWasChargedThisAdvanceCall; // updated every advance call, no need to save

	bool mScheduledForSightUpdate;
};

#endif

