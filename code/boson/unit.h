#ifndef __UNIT_H__
#define __UNIT_H__

#include "unitbase.h"

#include <qcanvas.h>

class Player;
class BosonCanvas;
class UnitProperties;

class UnitPrivate;

/**
 * Implementation of the visual parts of a unit. As far as possible all stuff
 * should go to UnitBase directly - except the visual stuff.
 *
 * Probably most things here can be moved to UnitBase. is a FIXME
 *
 * Not that Unit does <em>not</em> inherit @ref QObject! Signals/Slots are
 * therefore not possible!
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
	void moveTo(const QPoint& pos);
	void stopMoving(bool send = true);
	void stopAttacking();

	virtual bool save(QDataStream& stream);
	virtual bool load(QDataStream& stream);

	void updateSelectBox();


	QCanvasItemList unitsInRange() const;
	QCanvasItemList enemyUnitsInRange() const;

protected:
	/**
	 * @return a list of interesting collisions, i.e. no non-units, no
	 * destryed units, ...
	 **/
	QValueList<Unit*> unitCollisions(bool exact = false) const;

private:
	UnitPrivate* d;

};


// a d pointer is probably not very good here - far too much memory consumption
// same apllies to Unit and UnitBase. But it speeds up compiling as we don't
// have to change the headers every time...
class MobileUnitPrivate; 
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
	MobileUnitPrivate* d;
};

class FacilityPrivate;
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
	FacilityPrivate* d;
};

#endif
