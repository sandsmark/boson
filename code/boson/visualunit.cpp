
#include "visualunit.h"
#include "player.h"
#include "bosoncanvas.h"
#include "selectpart.h"
#include "cell.h"
#include "speciestheme.h"
#include "unitproperties.h"

#include <kgame/kgamepropertylist.h>

#include "defines.h"


#define PM_DELTA_H      (+4)   // mobiles selection boxes are DELTA pixels more inside rect()
#define PM_DELTA_V      (+10)   // mobiles selection boxes are DELTA pixels more inside rect()



class VisualUnitPrivate
{
public:
	VisualUnitPrivate()
	{
		mSelectBoxUp = 0;
		mSelectBoxDown = 0;
	}
	KGamePropertyInt mDirection;
	KGamePropertyInt mReloadState;

	KGamePropertyList<QPoint> mWaypoints;

	SelectPart* mSelectBoxUp;
	SelectPart* mSelectBoxDown;

	VisualUnit* mTarget;
};

VisualUnit::VisualUnit(int type, Player* owner, QCanvas* canvas) 
		: Unit(type), QCanvasSprite(owner->pixmapArray(type), canvas)
{
 d = new VisualUnitPrivate;
 setOwner(owner);

//hmm.. would probably make more sense in Unit instead of VisualUnit
 d->mDirection.registerData(IdDirection, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Direction");
 d->mWaypoints.registerData(IdWaypoints, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Waypoints");
 d->mReloadState.registerData(IdReloadState, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "ReloadState");

 d->mDirection.setLocal(0); // not yet used
 d->mReloadState.setLocal(0);
}

VisualUnit::~VisualUnit()
{
//kdDebug() << "~VisualUnit()" << endl;
 d->mWaypoints.setEmittingSignal(false); // just to prevent warning in Player::slotUnitPropertyChanged()
 d->mWaypoints.clear();
 unselect();
 delete d;
//kdDebug() << "~VisualUnit() done" << endl;
}

void VisualUnit::select()
{
 if (isDestroyed()) {
	return;
 }
 if (d->mSelectBoxUp || d->mSelectBoxDown) {
	// the box was already created
	return;
 }
// put the selection box on the same canvas as the unit and around the unit
// QRect r = rect();//AB: FIXME
 QRect r = boundingRect();
 d->mSelectBoxUp = new SelectPart(health(), z(), SelectPart::PartUp, canvas());
 d->mSelectBoxUp->move(r.right() - PM_DELTA_H, r.top() + PM_DELTA_V);
 d->mSelectBoxDown = new SelectPart(health(), z(), SelectPart::PartDown, canvas());
 d->mSelectBoxDown->move(r.left() - PM_DELTA_H, r.bottom() + PM_DELTA_V);
}

void VisualUnit::unselect()
{
 if (d->mSelectBoxUp) {
	delete d->mSelectBoxUp;
	d->mSelectBoxUp = 0;
 }
 if (d->mSelectBoxDown) {
	delete d->mSelectBoxDown;
	d->mSelectBoxDown = 0;
 }
}

VisualUnit* VisualUnit::target() const
{
 return d->mTarget;
}

void VisualUnit::setTarget(VisualUnit* target)
{
 d->mTarget = target;
 if (target != 0) {
	setWork(WorkAttack);
 }
}

void VisualUnit::setHealth(unsigned long int h)
{
 unsigned long int maxHealth = unitProperties()->health();
 if (h > maxHealth) {
	h = maxHealth;
 }
 if (maxHealth == 0) {
	kdError() << "Ooop - maxHealth == 0" << endl;
	return;
 }
 Unit::setHealth(h);
 if (d->mSelectBoxUp) {
	double div = (double)h / maxHealth;
	int frame = (int)((double)(SelectPart::frames() - 1) * div);
	d->mSelectBoxUp->setFrame(frame);
 }
}

void VisualUnit::moveBy(double moveX, double moveY)
{
// time critical function
 double oldX = x();
 double oldY = x();
 QCanvasSprite::moveBy(moveX, moveY);
 if (d->mSelectBoxUp) {
	d->mSelectBoxUp->moveBy(moveX, moveY);
 }
 if (d->mSelectBoxDown) {
	d->mSelectBoxDown->moveBy(moveX, moveY);
 }
 ((BosonCanvas*)canvas())->unitMoved(this, oldX, oldY);
}

void VisualUnit::advance(int phase)
{ // time critical function !!!
// kdDebug() << "VisualUnit::advance() id=" << id() << endl;
 if (phase == 0) {
	// collision detection should be done here as far as i understand
	// do not move the item/unit here!

	// perhaps test if there is a enemy unit in weapon range (x() - range()
	// -> x() + width() + range() and so on)
	// but we would need a setAnimated(true) for all units then :-(
 } else {
	if (work() == WorkMove) {
		advanceMove(); // move one step
	} else if (work() == WorkAttack) {
		attackUnit(target());
	} else if (work() == WorkProduce) {
		// TODO
	} else if (work() == WorkMine) {
		// TODO
	} else if (work() == WorkConstructed) {
		beConstructed();
	} else if (work() == WorkNone) {
		kdDebug() << "VisualUnit::advance(): work==WorkNone" << endl;
	} else {
		kdError() << "work: " << work() << endl;
	}
	if (d->mReloadState > 0) {
		d->mReloadState = d->mReloadState - 1;
	}
 }
 QCanvasSprite::advance(phase);
}

void VisualUnit::turnTo(int direction)
{
 if (direction < 0 || direction >= PIXMAP_PER_MOBILE - 1) {
	kdError() << "direction " << direction << " not supported" << endl;
	return;
 }
 setFrame(direction);
}

void VisualUnit::addWaypoint(const QPoint& pos)
{
 d->mWaypoints.append(pos);
// kdDebug() << "added " <<pos.x() << " " << pos.y() << endl;
}

void VisualUnit::waypointDone()
{
 d->mWaypoints.remove(d->mWaypoints.at(0));
}

unsigned int VisualUnit::waypointCount() const
{
 return d->mWaypoints.count();
}

void VisualUnit::moveTo(const QPoint& pos)
{
 d->mTarget = 0;
 clearWaypoints();
 addWaypoint(pos);
 setWork(WorkMove);
 setAnimated(true);
}

void VisualUnit::clearWaypoints()
{
 d->mWaypoints.clear();
}

const QPoint& VisualUnit::currentWaypoint() const
{
 return d->mWaypoints[0];
}

void VisualUnit::stopMoving(bool send)
{
 clearWaypoints();
 setWork(WorkNone);
 setAnimated(false); // do not call advance() anymore - is more efficient

 // in theory all units move on all clients the same - i.e. a playerInput() is
 // transmitted to all clients and all variables should always have the same
 // value.
 // but at least as of today (01/11/03) this is not completely working by any
 // reason. It can happen that a unit moves on client A but on client B there is
 // a collision with another unit - so it doesn't move. There are several
 // solutions possible
 // -> we should send a "IdStopMoving" and stop moving on all
 // clients at once - but the time lag is not nice. so we currently do the
 // following:
 // When one client has a collision for a unit and calls "stopMoving()" it sends
 // out the current coordinates of the unit. These coordinates are applied on
 // the other clients (they now also stop the unit). So all clients are synced
 // again.
 // the problem: if client A has a collision or arrival on destination, as well
 // as client B (this is the usual case!!) the *both* send out IdStopMoving with
 // the same coordinates... not nice...

 // I think I have found the reason for the not-synced units:
 // the problem seems to appear if several units are moved at once. Probably
 // they are added to the QCanvas in a different order and therefore are
 // advanced in a different order. This might result in an already moved (and
 // therefore no more a case for collisions) unit on client A but not yet moved
 // unit on client B (and therefore collisions return a different list, ...)
 // a solution will be: reorder the canvaslists!

 // so now i hace fixed the above. The problem seems to be solved - the
 // sendStopMoving below still resides here as of testing and debugging. Remove
 // it as soon as it's sure that we don't need it anymore. Will save a lot of
 // network traffik!
 if (send) {
	owner()->sendStopMoving(this);
 }
}

void VisualUnit::stopAttacking()
{
 stopMoving(); // FIXME not really intuitive... nevertheless its currenlty useful.
 setTarget(0);
}

bool VisualUnit::save(QDataStream& stream)
{
 if (!Unit::save(stream)) {
	kdError() << "Unit not saved properly" << endl;
	return false;
 }
 stream << (double)x();
 stream << (double)y();
 stream << (double)z();
 stream << (Q_INT8)isVisible();
 stream << (Q_INT32)frame();
 return true;
}

bool VisualUnit::load(QDataStream& stream)
{
 if (!Unit::load(stream)) {
	kdError() << "Unit not loaded properly" << endl;
	return false;
 }
 double x;
 double y;
 double z;
 Q_INT8 visible;
 Q_INT32 frame;
 
 stream >> x;
 stream >> y;
 stream >> z;
 stream >> visible;
 stream >> frame;

 setX(x);
 setY(y);
 setZ(z);
 setVisible(visible);
 setFrame(frame);
 return true;
}

bool VisualUnit::inRange(VisualUnit* target) const
{
 QRect r = boundingRect();
 r.setTop((r.top() > (int)range()) ? r.top() - range() : 0);
 r.setBottom(r.bottom() + range());
 r.setRight(r.right() + range());
 r.setLeft((r.left() > (int)range()) ? r.left() - range() : 0);
 return canvas()->collisions(r).contains(target);
}

void VisualUnit::attackUnit(VisualUnit* target)
{
 if (!target) {
	kdError() << "VisualUnit::attackUnit(): cannot attack NULL target" << endl;
	return;
 }
 if (!inRange(target)) {
	if (!canvas()->allItems().contains(target)) {
		kdDebug() << "Unit seems to be destroyed!" << endl;
		stopAttacking();
		return;
	}
	// FIXME: no waypoint yet...
	kdDebug() << "unit not in range - moving..." << endl;
	advanceMove();
	return;
 }
 if (d->mReloadState != 0) {
//	kdDebug() << "gotta reload first" << endl;
	return;
 }
 kdDebug() << "shoot at unit " << target->id() << endl;
 setXVelocity(0);
 setYVelocity(0);
 ((BosonCanvas*)canvas())->shootAtUnit(target, this, damage());
 d->mReloadState = reload();
 if (target->isDestroyed()) {
	stopAttacking();
 }
}





/////////////////////////////////////////////////
// VisualMobileUnit
/////////////////////////////////////////////////

class VisualMobileUnitPrivate
{
public:
	VisualMobileUnitPrivate()
	{
	}
};

VisualMobileUnit::VisualMobileUnit(int type, Player* owner, QCanvas* canvas) : VisualUnit(type, owner, canvas)
{
 d = new VisualMobileUnitPrivate;
}

VisualMobileUnit::~VisualMobileUnit()
{
 delete d;
}

void VisualMobileUnit::advanceMove()
{
// kdDebug() << "advanceMove()" << endl;

// TODO turnTo()
// FIXME this is still the initial implementation and all but good... should be
// improved.

 if (waypointCount() == 0) {
	// shouldn't happen - work() should be WorkNone here
	kdWarning() << "VisualUnit::advanceMove(): no waypoints?!" << endl;
	stopMoving(); // should have been called before already
	return;
 }

 if (speed() == 0) {
	stopMoving();
	kdDebug() << "speed = 0" << endl;
	return;
 }

 QRect position = boundingRect(); // where we currently are.
 QPoint waypoint = currentWaypoint(); // where we go to
/* if (position.contains(waypoint)) { // we arrived at our destination
	kdDebug() << "arrived at point" << endl;
	if (waypointCount() == 1) { // this is the only point left in path
		stopMoving();
	} else {
		waypointDone(); // remove the waypoint
	}
	return;
 }*/
 // we haven't arrived - let's move now
// kdDebug() << "w: " << waypoint.x() << " " << waypoint.y() << endl;
 setXVelocity(0); // so that boundinRectAdvanced() is working
 setYVelocity(0); // so that boundinRectAdvanced() is working
 double x = position.center().x();
 double y = position.center().y();
 if (waypoint.x() != x) {
	if (waypoint.x() > x) {
		setXVelocity(speed());
		if (waypoint.x() <= boundingRectAdvanced().center().x()) {
			setXVelocity(0);
		}
	} else {
		setXVelocity(speed() * -1);
		if (waypoint.x() >= boundingRectAdvanced().center().x()) {
			setXVelocity(0);
		}
	}
	if (!boCanvas()->canGo(this, boundingRectAdvanced())) {
		// we cannot go there :-(
		setXVelocity(0);
	}
 } else {
	setXVelocity(0);
 }
 if (waypoint.y() != y) {
	if (waypoint.y() > y) {
		setYVelocity(speed());
		if (waypoint.y() <= boundingRectAdvanced().center().y()) {
			setYVelocity(0);
		}
	} else {
		setYVelocity(speed() * -1);
		if (waypoint.y() >= boundingRectAdvanced().center().y()) {
			setYVelocity(0);
		}
	}
	if (!boCanvas()->canGo(this, boundingRectAdvanced())) {
		// we cannot go there :-(
		setYVelocity(0);
	}
 } else {
	setYVelocity(0);
 }
 if (xVelocity() == 0 && yVelocity() == 0) {
	kdDebug() << "could not complete move ?!" << endl;
	stopMoving();
 }
}



class VisualFacilityPrivate
{
public:
	VisualFacilityPrivate()
	{
	}

	KGamePropertyInt mConstructionState;
	KGamePropertyInt mConstructionDelay;
};

VisualFacility::VisualFacility(int type, Player* owner, QCanvas* canvas) : VisualUnit(type, owner, canvas)
{
 d = new VisualFacilityPrivate;
 d->mConstructionState.registerData(IdFix_ConstructionState, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Construction State");
 d->mConstructionDelay.registerData(IdFix_ConstructionDelay, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Construction Delay");
 d->mConstructionState.setLocal(0);

 setWork(WorkConstructed);
 setAnimated(true); // construcion animation
 setConstructionDelay(50); // default
}

VisualFacility::~VisualFacility()
{
 delete d;
}

int VisualFacility::constructionSteps()
{
 return FACILITY_CONSTRUCTION_STEPS;
}

void VisualFacility::setConstructionDelay(int delay)
{
 d->mConstructionDelay = delay;
}

int VisualFacility::constructionDelay() const
{
 if (d->mConstructionDelay > 0) {
	return d->mConstructionDelay;
 }
 return 1;
}

void VisualFacility::beConstructed()
{
 if (d->mConstructionState < (constructionSteps() - 1) * constructionDelay()) {
	d->mConstructionState = d->mConstructionState + 1;
	if (d->mConstructionState % constructionDelay() == 0) {
		setFrame(d->mConstructionState / constructionDelay());
	}
 } else {
	setAnimated(false);
 }
}

