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

#include "unit.h"
#include "player.h"
#include "bosoncanvas.h"
#include "cell.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "bosonpath.h"
#include "selectbox.h"
#include "bosonmessage.h"
#include "bosonstatistics.h"
#include "unitplugins.h"
#include "boitemlist.h"
#include "bosonmodel.h"
#include "pluginproperties.h"
//#include "kspritetooltip.h" // FIXME

#include <kgame/kgamepropertylist.h>
#include <kgame/kgame.h>

#include <qpointarray.h>

#include "defines.h"

class Unit::UnitPrivate
{
public:
	UnitPrivate()
	{
		mTarget = 0;
		shieldReloadCounter = 0;
	}
	KGamePropertyInt mDirection;

	KGamePropertyList<QPoint> mWaypoints;
	KGameProperty<int> mMoveDestX;
	KGameProperty<int> mMoveDestY;
	KGameProperty<int> mMoveRange;

	// be *very* careful with those - NewGameDialog uses Unit::save() which
	// saves all KGameProperty objects. If non-KGameProperty properties are
	// changed before all players entered the game we'll have a broken
	// network game.
	Unit* mTarget;

	int shieldReloadCounter; // FIXME: should be a KGameProperty! And in UnitBase...

	QPtrList<UnitPlugin> mPlugins; // you don't need to save/load this - gets constructed in the c'tor anyway.
};

Unit::Unit(const UnitProperties* prop, Player* owner, BosonCanvas* canvas)
		: UnitBase(prop),
		BosonSprite(owner->speciesTheme() ? owner->speciesTheme()->unitModel(prop->typeId()) : 0, canvas)
{
 d = new UnitPrivate;
 mCurrentPlugin = 0;
 mAdvanceFunction = &Unit::advanceNone;
 mAdvanceFunction2 = &Unit:: advanceNone;
 setOwner(owner);

 // note: these width and height can be used for e.g. pathfinding. It does not
 // depend in any way on the .3ds file or another OpenGL thing.
 setWidth(prop->unitWidth());
 setHeight(prop->unitWidth());

 d->mDirection.registerData(IdDirection, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Direction");
 d->mWaypoints.registerData(IdWaypoints, dataHandler(),
		KGamePropertyBase::PolicyLocal, "Waypoints");
 d->mMoveDestX.registerData(IdMoveDestX, dataHandler(),
		KGamePropertyBase::PolicyLocal, "MoveDestX");
 d->mMoveDestY.registerData(IdMoveDestY, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "MoveDestY");
 d->mMoveRange.registerData(IdMoveRange, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "MoveRange");

 d->mDirection.setLocal(0); // not yet used
 setAnimated(true);
 d->mMoveDestX.setLocal(0);
 d->mMoveDestY.setLocal(0);
 d->mMoveRange.setLocal(0);

 // TODO: the tooltips do not yet work with OpenGL!!
// KSpriteToolTip::add(rtti(), unitProperties()->name());
 if (!model()) {
	kdError() << k_funcinfo << "NULL model - this will most probably crash!" << endl;
	return;
 }
 setFrame(0);

 if (isFlying()) {
	// FIXME i guess we can store z-position as opengl coordinates
	// instead...
	setZ(2.0 * BO_TILE_SIZE / BO_GL_CELL_SIZE); // FIXME - hardcoded
 }

// create the plugins
// note: we use fixed KGame-property IDs, so we can't add any plugin twice. if
// we ever want to support this, we need to use dynamically assigned (see
// KGameProperty docs) - i use fixed ids to make debugging easier.
 if (prop->properties(PluginProperties::Production)) {
	d->mPlugins.append(new ProductionPlugin(this));
 }
 if (prop->properties(PluginProperties::Repair)) {
	d->mPlugins.append(new RepairPlugin(this));
 }
 if (prop->properties(PluginProperties::Harvester)) {
	d->mPlugins.append(new HarvesterPlugin(this));
 }
 if (prop->properties(PluginProperties::Refine)) {
//	d->mPlugins.append(new RefinePlugin(this));
 }

}

Unit::~Unit()
{
// KSpriteToolTip::remove(this);
 d->mWaypoints.setEmittingSignal(false); // just to prevent warning in Player::slotUnitPropertyChanged()
 d->mWaypoints.clear();
 unselect();
 delete d;
}

void Unit::select(bool markAsLeader)
{
 if (isDestroyed()) {
	return; // shall we really return?
 }
 BosonSprite::select(markAsLeader);
 updateSelectBox();
}

int Unit::destinationX() const
{
 return d->mMoveDestX;
}

int Unit::destinationY() const
{
 return d->mMoveDestY;
}

int Unit::moveRange() const
{
 return d->mMoveRange;
}

Unit* Unit::target() const
{
 return d->mTarget;
}

void Unit::setTarget(Unit* target)
{
 d->mTarget = target;
 if (!d->mTarget) {
	return;
 }
 if (d->mTarget->isDestroyed()) {
	 d->mTarget = 0;
 }
}

void Unit::setHealth(unsigned long int h)
{
 unsigned long int maxHealth = unitProperties()->health();
 if (h > maxHealth) {
	h = maxHealth;
 }
 if (maxHealth == 0) {
	kdError() << "Ooop - maxHealth == 0" << endl;
	return;
 }
 UnitBase::setHealth(h);
 updateSelectBox();
 if (isDestroyed()) {
	unselect();
	setFrame(frameCount() - 1);
	setAnimated(false);
 }
}

void Unit::setWork(WorkType w)
{
 if (currentPlugin() && w != WorkPlugin) {
	mCurrentPlugin = 0;
 }
 UnitBase::setWork(w);
}

void Unit::setPluginWork(int pluginType)
{
 UnitPlugin* p = plugin(pluginType);
 if (!p) {
	kdError() << k_funcinfo << id() << " does not have plugin " << pluginType << endl;
	return;
 }
 setWork(WorkPlugin);
 mCurrentPlugin = p;
}

UnitPlugin* Unit::plugin(int pluginType) const
{
 QPtrListIterator<UnitPlugin> it(d->mPlugins);
 for (; it.current(); ++it) {
	if (it.current()->pluginType() == pluginType) {
		return it.current();
	}
 }
 return 0;
}

int Unit::currentPluginType() const
{
 if (!currentPlugin()) {
	return 0;
 }
 return currentPlugin()->pluginType();
}


void Unit::updateSelectBox()
{
 if (selectBox()) {
	unsigned long int maxHealth = unitProperties()->health();
	double div = (double)health() / maxHealth;
	selectBox()->update(div);
 }
}

void Unit::moveBy(float moveX, float moveY, float moveZ)
{
// time critical function

 if (!moveX && !moveY && !moveZ || isDestroyed()) {
	return;
 }

 // QCanvasItem::moveBy() is called from QCanvasItem::advance(1). I finally
 // found out why it is a bad idea (tm) to do collision detection here.
 // QCanvas::collisions() (and all other collisions()) use imageAdvanced() for
 // collision detection.
 // collision detection of item A may found none - so it is moved. but item B
 // may find item A as collision candidate. but it now tests for the *next*
 // advance() pahse, as A has already been moved. so it may happen that item B
 // is ok, too, as item A won't be in the way in the next phase.
 // This means that be will be moved, too, but it mustn't be moved - we have a
 // collision.
 // UDPATE: we don't use QCanvas anymore - but this is still valid! We don't
 // support the advance phases anymore - we actually move directly in
 // BosonCanvas::slotAdvance()
 float oldX = x();
 float oldY = y();
 canvas()->removeFromCells(this);
 BosonSprite::moveBy(moveX, moveY, moveZ);
 canvas()->addToCells(this);
 canvas()->unitMoved(this, oldX, oldY);
}

void Unit::advance(unsigned int advanceCount)
{ // time critical function !!!
 if (isDestroyed()) {
	return;
 }
 reloadWeapon();
// Reload shields
 if(d->shieldReloadCounter >= 10) {
	if(shields() < unitProperties()->shields()) {
		setShields(shields() + 1);  // Maybe increase by more than 1. Or make configurable (ShieldsReload=xxx)
	}
	d->shieldReloadCounter = 0;
 } else {
	d->shieldReloadCounter++;
 }
}

void Unit::advanceNone(unsigned int advanceCount)
{
// this is called when the unit has nothing specific to do. Usually we just want
// to fire at every enemy in range.

 if (advanceCount % 10 != 0) {
	return;
 }

 if (weaponDamage() > 0) {
	BoItemList list = enemyUnitsInRange();
	if (list.count() > 0) {
		BoItemList::Iterator it = list.begin();
		for (; it != list.end(); ++it) {
			if (((Unit*)*it)->unitProperties()->canShoot()) {
				shootAt((Unit*)*it);
				break;
			}
		}
		if (it != list.end()) {
			// no military unit (i.e. unit that can shoot) found
			shootAt((Unit*)*it);
		}
	}
 } else if (weaponDamage() < 0) {
	if (!repairPlugin()) {
		kdWarning() << k_funcinfo << "weaponDamage < 0 but no repair plugin??" << endl;
		return;
	}
	repairPlugin()->repairInRange();
 } else {
	// weaponDamage() == 0 - what can be done here?
 }
}

void Unit::advanceAttack(unsigned int advanceCount)
{
 if (advanceCount % 5 != 0) {
	return;
 }
 kdDebug() << k_funcinfo << endl;
 if (!target()) {
	kdWarning() << k_funcinfo << "cannot attack NULL target" << endl;
	stopAttacking();
	return;
 }
 if (target()->isDestroyed()) {
	kdDebug() << "Target is destroyed!" << endl;
	stopAttacking();
	return;
 }
 if (!inRange(target())) {
	if (!canvas()->allBosonItems().contains(target())) {
		kdDebug() << "Target seems to be destroyed!" << endl;
		stopAttacking();
		return;
	}
	kdDebug() << "unit (" << target()->id() << ") not in range - moving..." << endl;
	if (!moveTo(target()->x(), target()->y(), weaponRange())) {
		setWork(WorkNone);
	} else {
		setAdvanceWork(WorkMove);
	}
	return;
 }
 shootAt(target());
 if (target()->isDestroyed()) {
	stopAttacking();
 }

}

void Unit::advanceDestroyed(unsigned int advanceCount)
{
 // note: the unit/wreckage will get deleted pretty soon
 if (advanceCount % 50 != 0) {
	return;
 }
}

void Unit::advancePlugin(unsigned int advanceCount)
{
 if (!currentPlugin()) {
	kdWarning() << k_funcinfo << "NULL plugin!" << endl;
	setWork(WorkNone);
 } else {
	currentPlugin()->advance(advanceCount);
 }
}

void Unit::addWaypoint(const QPoint& pos)
{
 d->mWaypoints.append(pos);
}

void Unit::waypointDone()
{
 d->mWaypoints.remove(d->mWaypoints.at(0));
}

QValueList<QPoint> Unit::waypointList() const
{
 return d->mWaypoints;
}

unsigned int Unit::waypointCount() const
{
 return d->mWaypoints.count();
}

void Unit::moveTo(const QPoint& pos)
{
 d->mTarget = 0;
 if(moveTo(pos.x(), pos.y(), 0)) {
	setWork(WorkMove);
 } else {
	setWork(WorkNone);
 }
}

bool Unit::moveTo(float x, float y, int range)
{
 stopMoving();

 if (range == -1) {
	range = d->mMoveRange;
 }
 if(!owner()->isFogged((int)(x / BO_TILE_SIZE), (int)(y / BO_TILE_SIZE))) {
	Cell* c = canvas()->cell((int)(x / BO_TILE_SIZE), (int)(y / BO_TILE_SIZE));
	if (!c) {
		kdError() << k_funcinfo << "NULL cell at " << x << "," << y << endl;
		return false;
	}
	// No pathfinding if goal not reachable or occupied and we can see it
	if(!c->canGo(unitProperties())) {
		return false;
	}
 }

 d->mMoveDestX = (int)x;
 d->mMoveDestY = (int)y;
 d->mMoveRange = range;

 // AB: FIXME: UnitGroups doesn't exist anymore! maybe we can search path here,
 // now?
 // Do not find path here!!! It would break pathfinding for groups. Instead, we
 //  set mSearchPath to true and find path in MobileUnit::advanceMove()
 // AB: UnitGroup is obsolete - can we search path here now?
 mSearchPath = true;

 return true;
}

void Unit::newPath()
{
 kdDebug() << k_funcinfo << endl;
 if(!owner()->isFogged(d->mMoveDestX / BO_TILE_SIZE, d->mMoveDestY / BO_TILE_SIZE)) {
	Cell* destCell = canvas()->cell(d->mMoveDestX / BO_TILE_SIZE,
			d->mMoveDestY / BO_TILE_SIZE);
	if(!destCell || (!destCell->canGo(unitProperties()))) {
		// If we can't move to destination, then we add waypoint with coordinates
		//  -1; -1 and in MobileUnit::advanceMove(), if currentWaypoint()'s
		//  coordinates are -1; -1 then we stop moving.
		clearWaypoints();
		addWaypoint(QPoint(-1, -1));
		return;
	}
 }
 // Only go until enemy is in range if we are attacking
 QValueList<QPoint> path = BosonPath::findPath(this, d->mMoveDestX, d->mMoveDestY, d->mMoveRange);
 clearWaypoints();
 for (int unsigned i = 0; i < path.count(); i++) {
	addWaypoint(path[i]);
 }
 if((currentWaypoint().x() == x() + width() / 2) && (currentWaypoint().y() == y() + height() / 2))
 {
	kdDebug() << k_funcinfo << "!!!!! First waypoint is unit's current pos! Removing" << endl;
	waypointDone();
 }
 if(waypointCount() == 0)
 {
	addWaypoint(QPoint(-1, -1));
 }
 return;
}

void Unit::clearWaypoints()
{
 d->mWaypoints.clear();
}

const QPoint& Unit::currentWaypoint() const
{
 return d->mWaypoints[0];
}

void Unit::stopMoving()
{
// kdDebug() << k_funcinfo << endl;
 clearWaypoints();

 // Call this only if we are only moving - stopMoving() is also called e.g. on
 // WorkAttack, when the unit is not yet in range.
 if (isMoving()) {
	setWork(WorkNone);
 } else if (advanceWork() != work()) {
	setAdvanceWork(work());
 }
 setXVelocity(0);
 setYVelocity(0);
}

void Unit::stopAttacking()
{
 stopMoving(); // FIXME not really intuitive... nevertheless its currently useful.
 setTarget(0);
 setWork(WorkNone);
}

bool Unit::save(QDataStream& stream)
{
 //TODO: d->mTarget
 //we should probably add pure virtual methods save() and load() to the plugins,
 //in order to store non-KGameProperty data there, too
 if (!UnitBase::save(stream)) {
	kdError() << "Unit not saved properly" << endl;
	return false;
 }
 stream << (float)x();
 stream << (float)y();
 stream << (float)z();
 stream << (Q_INT8)true; // FIXME: replacement for isVisible() which is obsolete
 stream << (Q_INT32)frame();
 return true;
}

bool Unit::load(QDataStream& stream)
{
 if (!UnitBase::load(stream)) {
	kdError() << "Unit not loaded properly" << endl;
	return false;
 }
 float x;
 float y;
 float z;
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
// setVisible(visible);//obsolete
 if (isDestroyed()) {
	kdError() << k_funcinfo << "unit is already destroyed" << endl;
 } else {
	setFrame(frame);
 }
 return true;
}

bool Unit::inRange(Unit* target) const
{
 // maybe we should use an own algorithm here - can be faster than this generic
 // one
 return unitsInRange().contains(target);
}

void Unit::shootAt(Unit* target)
{
 if (reloadState() != 0) {
//	kdDebug() << "gotta reload first" << endl;
	return;
 }
 if (target->isFlying()) {
	if (!unitProperties()->canShootAtAirUnits()) {
		return;
	}
 } else {
	if (!unitProperties()->canShootAtLandUnits()) {
		return;
	}
 }
 if (target->isDestroyed()) {
	kdWarning() << k_funcinfo << target->id() << " is already destroyed" << endl;
	return;
 }
 kdDebug() << id() << " shoots at unit " << target->id() << endl;
 ((BosonCanvas*)canvas())->shootAtUnit(target, this, weaponDamage());
 if (target->isDestroyed()) {
	if (target->isFacility()) {
		owner()->statistics()->addDestroyedFacility((Facility*)target, this);
	} else {
		owner()->statistics()->addDestroyedMobileUnit((MobileUnit*)target, this);
	}
 }
 owner()->statistics()->increaseShots();
 resetReload();
}

BoItemList Unit::unitsInRange() const
{
 // TODO: we use a *rect* for the range this is extremely bad.
 // ever heard about pythagoras ;-) ?

 QPointArray cells;
 int left, right, top, bottom;
 leftTopCell(&left, &top);
 rightBottomCell(&right, &bottom);
 left = QMAX(left - (int)weaponRange(), 0);
 top = QMAX(top - (int)weaponRange(), 0);
 right = QMIN(right + (int)weaponRange(), QMAX((int)canvas()->mapWidth() - 1, 0));
 bottom = QMIN(bottom + (int)weaponRange(), QMAX((int)canvas()->mapHeight() - 1, 0));
 int size = (right - left + 1) * (bottom - top + 1);
 if (size <= 0) {
	return BoItemList();
 }
 cells.resize(size);
 int n = 0;
 for (int i = left; i <= right; i++) { 
	for (int j = top; j <= bottom; j++) {
		cells[n++] = QPoint(i, j);
	}
 }

 BoItemList items = canvas()->bosonCollisions(cells, (BosonSprite*)this, false);
 items.remove((BosonSprite*)this);
 BoItemList inRange;
 BoItemList::Iterator it = items.begin();
 for (; it != items.end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	if(((Unit*)(*it))->isDestroyed()) {
		continue;
	}
	// TODO: remove the items from inRange which are not actually in range (hint:
	// pythagoras)
	inRange.append(*it);
 }
 return inRange;
}

BoItemList Unit::enemyUnitsInRange() const
{
 BoItemList units = unitsInRange();
 BoItemList enemy;
 BoItemList::Iterator it = units.begin();
 for (; it != units.end(); ++it) {
	Unit* u = (Unit*)*it;
	if (owner()->isEnemy(u->owner())) {
		enemy.append(u);
	}
 }
 return enemy;
}

QValueList<Unit*> Unit::unitCollisions(bool exact) const
{
 QValueList<Unit*> units;
 if (isFlying()) { // flying units never collide - different altitudes
	return units;
 }
 kdDebug() << k_funcinfo << endl;
 BoItemList collisionList = canvas()->bosonCollisions(cells(), (BosonSprite*)this, exact);
 if (collisionList.isEmpty()) {
	return units;
 }
 
 BoItemList::Iterator it;
 for (it = collisionList.begin(); it != collisionList.end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	Unit* unit = ((Unit*)*it);
	if (unit->isDestroyed()) {
		continue;
	}
	if (unit->isFlying()) {
		continue;
	}
	units.append(unit);
 }
 return units;
}

void Unit::setAdvanceWork(WorkType w)
{
 // velicities should be 0 anyway - this is the final fallback in case it was 
 // missing by any reason
 setXVelocity(0);
 setYVelocity(0);

 UnitBase::setAdvanceWork(w);

 // we even do this if nothing changed - just in case...
 switch (w) {
	case WorkNone:
		setAdvanceFunction(&Unit::advanceNone, owner()->advanceFlag());
		break;
	case WorkMove:
		setAdvanceFunction(&Unit::advanceMove, owner()->advanceFlag());
		break;
	case WorkAttack:
		setAdvanceFunction(&Unit::advanceAttack, owner()->advanceFlag());
		break;
	case WorkConstructed:
		setAdvanceFunction(&Unit::advanceConstruction, owner()->advanceFlag());
		break;
	case WorkDestroyed:
		setAdvanceFunction(&Unit::advanceDestroyed, owner()->advanceFlag());
		break;
	case WorkFollow:
		setAdvanceFunction(&Unit::advanceFollow, owner()->advanceFlag());
		break;
	case WorkPlugin:
		setAdvanceFunction(&Unit::advancePlugin, owner()->advanceFlag());
		break;
 }
}

void Unit::setAdvanceFunction(MemberFunction func, bool advanceFlag)
{
 if (canvas()->advanceFunctionLocked()) {
	if (advanceFlag) {
		mAdvanceFunction = func;
	} else {
		mAdvanceFunction2 = func;
	}
 } else {
	mAdvanceFunction = func;
	mAdvanceFunction2 = func;
 }
}

bool Unit::isNextTo(Unit* target) const
{
 //const int r = BO_TILE_SIZE;
 const int r = 10;
 // in theory r = 1 should be enough... both of the above make problems under
 // certain circumstances
 if (QABS(rightEdge() - target->leftEdge()) <= r ||
		QABS(leftEdge() - target->rightEdge()) <= r ||
		rightEdge() <= target->rightEdge() && leftEdge() <= target->leftEdge()// will never happen with current pixmaps
		) { 
	if (QABS(topEdge() - target->bottomEdge() <= r) ||
			QABS(bottomEdge() - target->topEdge()) <= r||
			topEdge() <= target->topEdge() && bottomEdge() <= target->bottomEdge()// will never happen with current pixmaps
			) {
		kdDebug() << "ok - inrange" << endl;
		return true;
	}
 }
 return false;
}

void Unit::playSound(UnitSoundEvent event)
{
 if (!speciesTheme()) {
	return;
 }
 speciesTheme()->playSound(this, event);
}


/////////////////////////////////////////////////
// MobileUnit
/////////////////////////////////////////////////

class MobileUnit::MobileUnitPrivate
{
public:
	MobileUnitPrivate()
	{
		mTargetCellMarked = false;
	}

	KGameProperty<float> mSpeed;
	KGameProperty<unsigned int> mMovingFailed;
	KGameProperty<unsigned int> mPathRecalculated;

	bool mTargetCellMarked;
};

MobileUnit::MobileUnit(const UnitProperties* prop, Player* owner, BosonCanvas* canvas) : Unit(prop, owner, canvas)
{
 d = new MobileUnitPrivate;
 d->mSpeed.registerData(IdSpeed, dataHandler(),
		KGamePropertyBase::PolicyLocal, "Speed");
 d->mMovingFailed.registerData(IdMob_MovingFailed, dataHandler(),
		KGamePropertyBase::PolicyLocal, "MovingFailed");
 d->mPathRecalculated.registerData(IdMob_PathRecalculated, dataHandler(),
		KGamePropertyBase::PolicyLocal, "PathRecalculated");
 d->mSpeed.setLocal(0);
 d->mMovingFailed.setLocal(0);
 d->mPathRecalculated.setLocal(0);

 d->mMovingFailed.setEmittingSignal(false);
 d->mPathRecalculated.setEmittingSignal(false);

 setWork(WorkNone);
}

MobileUnit::~MobileUnit()
{
 delete d;
}

void MobileUnit::advanceMoveInternal(unsigned int) // this actually needs to be called for every advanceCount.
{
 if (speed() == 0) {
	kdWarning() << k_funcinfo << "speed == 0" << endl;
	stopMoving();
	return;
 }

 if(mSearchPath) {
	newPath();
	mSearchPath = false;
	return;
 }

 if(waypointCount() == 0) {
	// Waypoints were PolicyClean previously but are now PolicyLocal so they
	//  should arrive immediately. If there are no waypoints but advanceMove is
	//  called, then probably there's an error somewhere
	kdError() << k_funcinfo << "No waypoints" << endl;
	stopMoving();
	return;
 }

 kdDebug() << k_funcinfo << endl;
 if (advanceWork() != work()) {
	if (work() == WorkAttack) {
		// no need to move to the position of the unit...
		// just check if unit is in range now.
		if (inRange(target())) {
			kdDebug() << k_funcinfo << "target is in range now" << endl;
			stopMoving();
			return;
		}
		// TODO: make sure that target() hasn't moved!
		// if it has moved also adjust waypoints
	} else if (currentPluginType() == UnitPlugin::Harvester) {
		HarvesterPlugin* h = (HarvesterPlugin*)currentPlugin();
		if (isNextTo(h->refinery())) {
			kdDebug() << k_funcinfo << "refinery in range now" << endl;
			stopMoving();
			return;
		}
	}
 }

 QPoint wp = currentWaypoint(); // where we go to
 // If both waypoint's coordinates are -1, then it means that path to
 //  destination can't be found and we should stop
 if ((wp.x() == -1) && (wp.y() == -1)) {
	stopMoving();
	return;
 }

 // FIXME: as path is now recalculated every time waypoint is reached, this
 //  should never be called
 if((wp.x() == -2) &&(wp.y() == -2)) {
	clearWaypoints();
	newPath();
	return;
 }

 int x = (int)(BosonSprite::x() + width() / 2);
 int y = (int)(BosonSprite::y() + height() / 2);

 float xspeed = 0;
 float yspeed = 0;

 // First check if we're at waypoint
 if((x == wp.x()) && (y == wp.y())) {
	kdDebug() << k_funcinfo << "unit is at waypoint" << endl;
	waypointDone();

	if(waypointCount() == 0) {
		kdDebug() << k_funcinfo << "no more waypoints. Stopping moving" << endl;
		stopMoving();
		return;
	}

	// We now recalc path _every_ time we reach waypoint
	//  Units will then react more quickly when other units move and block their
	//  way for example
	newPath();

	wp = currentWaypoint();
 }

 // Check if we can actually go to waypoint (maybe it was fogged)
 // FIXME: currentWaypoint should have been unfogged when path was calculated
 //  because we now recalc path after every waypoint (see ~5 lines above)
 if(!canvas()->cell(wp.x() / BO_TILE_SIZE, wp.y() / BO_TILE_SIZE) ||
		!canvas()->cell(wp.x() / BO_TILE_SIZE, wp.y() / BO_TILE_SIZE)->canGo(unitProperties())) {
	kdWarning() << k_funcinfo << "cannot go to waypoint, finding new path" << endl;
	setXVelocity(0);
	setYVelocity(0);
	// We have to clear waypoints first to make sure that they aren't used next
	//  advance() call (when new waypoints haven't arrived yet)
	// FIXME: no need to clear them anymore
	clearWaypoints();
	newPath();
	return;
 }

 // Try to go to same x and y coordinates as waypoint's coordinates
 // First x coordinate
 // Slow down if there is less than speed() pixels to go
 if(QABS(wp.x() - x) < speed()) {
	xspeed = wp.x() - x;
 } else {
	xspeed = speed();
	if(wp.x() < x) {
		xspeed = -xspeed;
	}
 }
 // Same with y coordinate
 if(QABS(wp.y() - y) < speed()) {
	yspeed = wp.y() - y;
 } else {
	yspeed = speed();
	if(wp.y() < y) {
		yspeed = -yspeed;
	}
 }

 // Set velocity for actual moving
 setVelocity(xspeed, yspeed);

 // set the new direction according to new speed
 turnTo();
}

void MobileUnit::advanceMoveCheck()
{
 if (!canvas()->onCanvas(boundingRectAdvanced().topLeft())) {
	kdDebug() << k_funcinfo << "not on canvas" << endl;
	stopMoving();
	setWork(WorkNone);
	return;
 }
 if (!canvas()->onCanvas(boundingRectAdvanced().bottomRight())) {
	kdDebug() << k_funcinfo << "not on canvas" << endl;
	stopMoving();
	setWork(WorkNone);
	return;
 }
 kdDebug() << k_funcinfo << endl;
 if (canvas()->cellOccupied(currentWaypoint().x() / BO_TILE_SIZE,
		currentWaypoint().y() / BO_TILE_SIZE, this, false)) {
//	kdDebug() << k_funcinfo << "collisions" << endl;
//	kdWarning() << k_funcinfo << "" << id() << " -> " << l.first()->id() 
//		<< " (count=" << l.count() <<")"  << endl;
	// do not move at all. Moving is not stopped completely!
	// work() is still workMove() so we'll continue moving in the next
	// advanceMove() call

	d->mMovingFailed = d->mMovingFailed + 1;
	setXVelocity(0);
	setYVelocity(0);

	const int recalculate = 50; // recalculate when 50 advanceMove() failed
	if(d->mPathRecalculated >= 2) {
		kdDebug() << k_funcinfo << "Path recalculated 3 times and it didn't help, giving up and stopping" << endl;
		stopMoving();
		return;
	}
	if (d->mMovingFailed >= recalculate) {
		kdDebug() << "recalculating path" << endl;
		// you must not do anything that changes local variables directly here!
		// all changed of variables with PolicyClean are ok, as they are sent
		// over network and do not take immediate effect.

		newPath();
		d->mMovingFailed = 0;
		d->mPathRecalculated = d->mPathRecalculated + 1;
	}
	return;
 }
 else if (! d->mTargetCellMarked) {
	canvas()->cell(currentWaypoint().x() / BO_TILE_SIZE, currentWaypoint().y() / BO_TILE_SIZE)->willBeOccupiedBy(this);
	d->mTargetCellMarked = true;
 }
 d->mMovingFailed = 0;
 d->mPathRecalculated = 0;
 kdDebug() << k_funcinfo << "done" << endl;
}

void MobileUnit::setSpeed(float speed)
{
 d->mSpeed = speed;
}

float MobileUnit::speed() const
{
 return d->mSpeed;
}

void MobileUnit::turnTo(Direction direction)
{
 if (isDestroyed()) {
	kdError() << k_funcinfo << "unit is already destroyed!" << endl;
	return;
 }
 // At the moment, all units are facing south by default, but currect would be
 //  north so change direction hack as soon as it's fixed
 setRotation((float)((((int)direction + 4) % 8) * -45));
}

void MobileUnit::turnTo()
{
 float xspeed = xVelocity();
 float yspeed = yVelocity();
 // Set correct frame
 if((xspeed == 0) && (yspeed < 0)) { // North
 	turnTo(North);
 } else if((xspeed > 0) && (yspeed < 0)) { // NE
	turnTo(NorthEast);
 } else if((xspeed > 0) && (yspeed == 0)) { // East
	turnTo(East);
 } else if((xspeed > 0) && (yspeed > 0)) { // SE
	turnTo(SouthEast);
 } else if((xspeed == 0) && (yspeed > 0)) { // South
	turnTo(South);
 } else if((xspeed < 0) && (yspeed > 0)) { // SW
	turnTo(SouthWest);
 } else if((xspeed < 0) && (yspeed == 0)) { // West
	turnTo(West);
 } else if((xspeed < 0) && (yspeed < 0)) { // NW
	turnTo(NorthWest);
 } else if (xspeed == 0 && yspeed == 0) {
//	kdDebug() << k_funcinfo << "xspeed == 0 and yspeed == 0" << endl;
 } else {
	kdDebug() << k_funcinfo << "error when setting frame" << endl;
 }
}

void MobileUnit::advanceFollow(unsigned int advanceCount)
{
 if (advanceCount % 5 != 0) {
	return;
 }
 if (!target()) {
	kdWarning() << k_funcinfo << "cannot follow NULL unit" << endl;
	stopAttacking();  // stopAttacking should maybe be renamed to stopEverything
			//  or just stop because it's used in several places to stop unit from
			//  doing whatever it does.
	return;
 }
 if (target()->isDestroyed()) {
	kdDebug() << k_funcinfo << "Unit is destroyed!" << endl;
	stopAttacking();
	return;
 }
// if (!isNextTo(target())) {  // This doesn't work for some reason :-(  Dunno why.
 if (QMAX(QABS(x() - target()->x()), QABS(y() - target()->y())) > BO_TILE_SIZE) {
	// We're not next to unit
	if (!canvas()->allBosonItems().contains(target())) {
		kdDebug() << k_funcinfo << "Unit seems to be destroyed!" << endl;
		stopAttacking();
		return;
	}
	kdDebug() << k_funcinfo << "unit (" << target()->id() << ") not in range - moving..." << endl;
	if (!moveTo(target()->x(), target()->y(), 1)) {
		setWork(WorkNone);
	} else {
		setAdvanceWork(WorkMove);
	}
 }
 // Do nothing (unit is in range)
}

QRect MobileUnit::boundingRect() const
{
// FIXME: workaround for pathfinding which does not yet support units with size
// > BO_TILE_SIZE
// we simply return a boundingrect which has size BO_TILE_SIZE
 if (width() < BO_TILE_SIZE || height() < BO_TILE_SIZE) {
	kdWarning() << k_funcinfo << "width or height  < BO_TILE_SIZE - not supported!!" << endl;
	return BosonSprite::boundingRect();
 }
 return QRect((int)x(), (int)y(), BO_TILE_SIZE, BO_TILE_SIZE);
}

void MobileUnit::clearWaypoints()
{
 Unit::clearWaypoints();
 d->mTargetCellMarked = false;
}

void MobileUnit::waypointDone()
{
 Unit::waypointDone();
 d->mTargetCellMarked = false;
}

bool MobileUnit::load(QDataStream& stream)
{
 if(!Unit::load(stream)) {
	kdError() << "Unit not loaded properly" << endl;
	return false;
 }

 return true;
}

bool MobileUnit::save(QDataStream& stream)
{
 if(!Unit::save(stream)) {
	kdError() << "Unit not loaded properly" << endl;
	return false;
 }

 return true;
}

/////////////////////////////////////////////////
// Facility
/////////////////////////////////////////////////

class Facility::FacilityPrivate
{
public:
	FacilityPrivate()
	{
		mRepairPlugin = 0;
	}

	KGameProperty<unsigned int> mConstructionState; // state of *this* unit
	RepairPlugin* mRepairPlugin;
};

Facility::Facility(const UnitProperties* prop, Player* owner, BosonCanvas* canvas) : Unit(prop, owner, canvas)
{
 d = new FacilityPrivate;
 d->mConstructionState.registerData(IdFix_ConstructionState, dataHandler(), 
		KGamePropertyBase::PolicyLocal, "Construction State");
 d->mConstructionState.setLocal(0);

 if (unitProperties()->weaponDamage() < 0) { // TODO use a property plugin
	d->mRepairPlugin = new RepairPlugin(this);
 }
 setWork(WorkConstructed);
}

Facility::~Facility()
{
 // TODO: write a plugin framework and manage plugins in a list.
 delete d->mRepairPlugin;
 delete d;
}

unsigned int Facility::constructionSteps() const
{
 return unitProperties()->constructionSteps();
}

void Facility::advanceConstruction(unsigned int advanceCount)
{
 if (advanceCount % 20 != 0) {
	return;
 }
 if (isDestroyed()) {
	kdError() << k_funcinfo << "unit is already destroyed" << endl;
	return;
 }
 setConstructionStep(d->mConstructionState + 1);
}

UnitPlugin* Facility::plugin(int pluginType) const
{
 if (!isConstructionComplete()) {
	return 0;
 }
 return Unit::plugin(pluginType);
}

RepairPlugin* Facility::repairPlugin() const
{
 if (!isConstructionComplete()) {
	return 0;
 }
 return d->mRepairPlugin;
}

bool Facility::isConstructionComplete() const
{
 if (work() == WorkConstructed) {
	return false;
 }
 if (d->mConstructionState < constructionSteps()) {
	return false;
 }
 return true;
}

double Facility::constructionProgress() const
{
 unsigned int constructionTime = constructionSteps();
 double percentage = (double)(d->mConstructionState * 100) / (double)constructionTime;
 return percentage;
}

void Facility::setTarget(Unit* u)
{
 if (u && !isConstructionComplete()) {
	kdWarning() << k_funcinfo << "not yet constructed completely" << endl;
	return;
 }
 Unit::setTarget(u);
}

void Facility::moveTo(float x, float y, int range)
{
 if (!isConstructionComplete()) {
	kdWarning() << k_funcinfo << "not yet constructed completely" << endl;
	return;
 }
 Unit::moveTo(x, y, range);
}

void Facility::setConstructionStep(unsigned int step)
{
 if (isDestroyed()) {
	kdError() << k_funcinfo << "unit is already destroyed" << endl;
	return;
 }
 if (step > constructionSteps()) {
	step = constructionSteps();
 }
 // warning: constructionSteps() and BosonModel::constructionSteps() are
 // *totally* different values!!
 unsigned int modelStep = 0;
 if (step == constructionSteps()) {
	modelStep = model()->constructionSteps(); // completed construction
 } else {
	modelStep = model()->constructionSteps() * step / constructionSteps();
//	kdDebug() << k_funcinfo << "step="<<step<<",modelstep="<<modelStep<<endl;
 }
 setGLConstructionStep(modelStep); // the displayed construction step. note that currentConstructionStep() is a *different* value!
 d->mConstructionState = step;
 if (step == constructionSteps()) {
	setWork(WorkNone);
	owner()->facilityCompleted(this);
	setFrame(0);
 }
}

unsigned int Facility::currentConstructionStep() const
{
 return d->mConstructionState;
}

