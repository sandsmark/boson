#ifndef __VISUALUNIT_H__
#define __VISUALUNIT_H__

#include "unit.h"

#include <qcanvas.h>

class Player;
class BosonCanvas;
class UnitProperties;

class VisualUnitPrivate;

/**
 * Implementation of the visual parts of a unit. As far as possible all stuff
 * should go to Unit directly - except the visual stuff.
 *
 * Probably most things here can be moved to Unit. is a FIXME
 *
 * Not that VisualUnit does <em>not</em> inherit @ref QObject! Signals/Slots are
 * therefore not possible!
 **/
class VisualUnit : public Unit, public QCanvasSprite
{ // could become an abstract class
public:
	enum PropertyIds {
		IdDirection = Unit::IdLast + 1,
		IdWaypoints = Unit::IdLast + 2,
		IdFix_ConstructionState = Unit::IdLast + 3,
		IdFix_ConstructionDelay = Unit::IdLast + 4,
		IdReloadState = Unit::IdLast + 5

	};
	VisualUnit(int type, Player* owner, QCanvas* canvas);
	virtual ~VisualUnit();

	virtual int rtti() const { return Unit::rtti(); }

	void turnTo(int direction);

	virtual void setHealth(unsigned long int h);

	BosonCanvas* boCanvas() const { return (BosonCanvas*)canvas(); }

	void select();
	void unselect();

	virtual void moveBy(double x, double y);

	virtual void advance(int phase);

	void attackUnit(VisualUnit* target);
	
	/**
	 * Move the unit. By default this does nothing. Reimplemented in @ref
	 * VisualMobileUnit
	 **/
	virtual void advanceMove() { }

	/**
	 * Move the construction animation one step forward. Does nothing by
	 * default - reimplemented in VisualFacility
	 **/
	virtual void beConstructed() { }

	VisualUnit* target() const;
	void setTarget(VisualUnit* target);
	bool inRange(VisualUnit* unit) const;

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

protected:

private:
	VisualUnitPrivate* d;

};





// a d pointer is probably not very good here - far too much memory consumption
// same apllies to VisualUnit and Unit. But it speeds up compiling as we don't
// have to change the headers every time...
class VisualMobileUnitPrivate; 
// if you add class members - ONLY KGameProperties!! otherwise Player::load and
// Player::save() won't work correctly! - if you add non KGameProperties adjust
// Unit::save() and unit::load()
class VisualMobileUnit : public VisualUnit
{
public:
	VisualMobileUnit(int type, Player* owner, QCanvas* canvas);
	virtual ~VisualMobileUnit();

	virtual void advanceMove(); // move one step futher to path
	
private:
	VisualMobileUnitPrivate* d;
};

class VisualFacilityPrivate;
// if you add class members - ONLY KGameProperties!! otherwise Player::load and
// Player::save() won't work correctly!
class VisualFacility : public VisualUnit
{
public:
	VisualFacility(int type, Player* owner, QCanvas* canvas);
	virtual ~VisualFacility();

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

private:
	VisualFacilityPrivate* d;
};





#endif
