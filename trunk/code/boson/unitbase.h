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

class UnitBasePrivate;
class UnitBase
{
public:
	enum PropertyIds {
		IdHealth = KGamePropertyBase::IdUser + 0,
		IdArmor = KGamePropertyBase::IdUser + 1,
		IdShields = KGamePropertyBase::IdUser + 2,
		IdId = KGamePropertyBase::IdUser + 3, // useful? perhaps use dataHandler()->id() instead
		IdCost = KGamePropertyBase::IdUser + 4,
		IdType = KGamePropertyBase::IdUser + 5,
		IdWork = KGamePropertyBase::IdUser + 6,
		IdSpeed = KGamePropertyBase::IdUser + 7,
		IdDamage = KGamePropertyBase::IdUser + 8,
		IdRange = KGamePropertyBase::IdUser + 9,
		IdReload = KGamePropertyBase::IdUser + 10,
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
	
	UnitBase(int type);
	virtual ~UnitBase();

	void setWork(WorkType w);

	WorkType work() const;

	/**
	 * @return Guess what?
	 **/
	const QString& name() const; // FIXME: NOT HERE! is in UnitProperties!!! // woule take too much memory (100 * unitType_1 would be 100 * 1 name...)

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

	virtual void setHealth(unsigned long int h);

	bool isDestroyed() const
	{
		return (health() == 0);
	}

	/**
	 * @return The owner (player) of the unit
	 **/
	Player* owner() const { return mOwner; }
	void setOwner(Player* owner);

	KGamePropertyHandler* dataHandler() const;

	/**
	 * The ID of the unit. This ID is unique for this game.
	 * @return The uniuque ID of the unit
	 **/
	unsigned long int id() const;

	void setId(unsigned long int id);

	unsigned long int shields() const;
	void setShields(unsigned long int shields);

	unsigned long int armor() const;
	void setArmor(unsigned long int armor);

	/**
	 * @return The price of the unit. It can be changed using @ref setCost
	 * but this shouldn't be necessary very often (beside the initial
	 * creation).
	 **/
	virtual unsigned long int cost() const; // we don't need this. in UnitProperties it's enough!

	/**
	 * Change the price of the unit. This usually doesn't change often
	 * within a game but it is at least possible...
	 **/
	void setCost(unsigned long int price);

	/**
	 * The type of the unit as described in the index.desktop file of this
	 * unit. See also @ref UnitProperties::typeId
	 **/
	virtual int type() const;

	virtual int rtti() const { return RTTI::UnitStart + (int)type(); }

	/*
	 * For mobile units this means where the unit is currently moving to.
	 * For a facility this means where produced units shall be placed.
	 *
	 * Must be reimplemented in derived classes.
	 **/
//	virtual const QPoint& destination() const = 0; // TODO

	long int damage() const; // not unsigned - can also repair :-)
	void setDamage(long int d); // not unsigned - can also repair :-)
	unsigned long int range() const;
	void setRange(unsigned long int r);
	unsigned int reload() const; // number of advance() calls until reloaded
	void setReload(unsigned int r);

	// TODO: ONLY mobile units!
	// I dont want to have classes UnitBase, MobileUnit, FixUnit, Unit,
	// VisualMobileUnit and VisualFixUnit.
	// Should the speed stay in UnitBase - memory overhead for facilities
	// or should it go to VisualMobileUnit - kind of unclean?
	// Or will we use the speed for something else as well (facilities)?
	// E.g. produce speed?
	double speed() const;
	void setSpeed(double s);


	virtual bool save(QDataStream& stream);
	virtual bool load(QDataStream& stream);

	/**
	 * These are <em>not</em> the @ref KGameProperties! See @ref dataHandler
	 * for these.
	 *
	 * The @ref UnitProperties describes a unit type generally. This
	 * includes the name of tha unit as well as all initial values.
	 *
	 * Note that the @ref UnitProperties class is a property of @ref
	 * SpeciesTheme and therefore of the @ref owner of this unit. If this
	 * unit does not yet have an owner this always returns 0!!
	 **/
	const UnitProperties* unitProperties() const;

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
	
private:
	UnitBasePrivate* d;
	Player* mOwner;
};

#endif
