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
#include "boson.h"
#include "bosonparticlesystem.h"
#include "bosonweapon.h"
#include "bodebug.h"

#include <kgame/kgamepropertylist.h>
#include <kgame/kgame.h>
#include <krandomsequence.h>

#include <qpointarray.h>
#include <qptrlist.h>

#include "defines.h"

#define TURN_STEP 5

bool Unit::mInitialized = false;

class Unit::UnitPrivate
{
public:
	UnitPrivate()
	{
		mTarget = 0;
		mSmokeParticleSystem = 0;
	}
	KGamePropertyList<QPoint> mWaypoints;
	KGameProperty<int> mMoveDestX;
	KGameProperty<int> mMoveDestY;
	KGameProperty<int> mMoveRange;
	KGameProperty<int> mWantedRotation;
	KGameProperty<int> mMoveAttacking;

	// be *very* careful with those - NewGameDialog uses Unit::save() which
	// saves all KGameProperty objects. If non-KGameProperty properties are
	// changed before all players entered the game we'll have a broken
	// network game.
	Unit* mTarget;

	// these must NOT be touched (items added or removed) after the c'tor.
	// loading code will depend in this list to be at the c'tor state!
	QPtrList<UnitPlugin> mPlugins;
	QPtrList<BosonWeapon> mWeapons;

	// OpenGL only:
	BosonParticleSystem* mSmokeParticleSystem;  // This will be removed soon
	QPtrList<BosonParticleSystem> mActiveParticleSystems;  // No autodelete!!!
};

Unit::Unit(const UnitProperties* prop, Player* owner, BosonCanvas* canvas)
		: UnitBase(prop),
		BosonItem(owner->speciesTheme() ? owner->speciesTheme()->unitModel(prop->typeId()) : 0, canvas)
{
 if (!mInitialized) {
	initStatic();
 }
 d = new UnitPrivate;
 mCurrentPlugin = 0;
 mAdvanceFunction = &Unit::advanceNone;
 mAdvanceFunction2 = &Unit:: advanceNone;
 setOwner(owner);
 d->mPlugins.setAutoDelete(true);
 d->mWeapons.setAutoDelete(true);
 d->mPlugins.clear();
 d->mWeapons.clear();

 // note: these width and height can be used for e.g. pathfinding. It does not
 // depend in any way on the .3ds file or another OpenGL thing.
 setSize(prop->unitWidth(), prop->unitHeight());

 registerData(&d->mWaypoints, IdWaypoints);
 registerData(&d->mMoveDestX, IdMoveDestX);
 registerData(&d->mMoveDestY, IdMoveDestY);
 registerData(&d->mMoveRange, IdMoveRange);
 registerData(&d->mWantedRotation, IdWantedRotation);
 registerData(&d->mMoveAttacking, IdMoveAttacking);

 setAnimated(true);
 d->mMoveDestX.setLocal(0);
 d->mMoveDestY.setLocal(0);
 d->mMoveRange.setLocal(0);
 d->mWantedRotation.setLocal(0);
 d->mMoveAttacking.setLocal(0);

 // TODO: the tooltips do not yet work with OpenGL!!
// KSpriteToolTip::add(rtti(), unitProperties()->name());
 if (!model()) {
	boError() << k_funcinfo << "NULL model - this will most probably crash!" << endl;
	return;
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

 loadWeapons();
}

Unit::~Unit()
{
// KSpriteToolTip::remove(this);
 d->mWaypoints.setEmittingSignal(false); // just to prevent warning in Player::slotUnitPropertyChanged()
 d->mWaypoints.clear();
 unselect();
 d->mPlugins.clear();
 d->mWeapons.clear();
 delete d;
}

void Unit::initStatic()
{
 // we initialize the properties for Unit, MobileUnit, Facility and the plugins
 // here
 // Unit
 addPropertyId(IdWaypoints, QString::fromLatin1("Waypoints"));
 addPropertyId(IdMoveDestX, QString::fromLatin1("MoveDestX"));
 addPropertyId(IdMoveDestY, QString::fromLatin1("MoveDestY"));
 addPropertyId(IdMoveRange, QString::fromLatin1("MoveRange"));
 addPropertyId(IdWantedRotation, QString::fromLatin1("WantedRotation"));
 addPropertyId(IdMoveAttacking, QString::fromLatin1("MoveAttacking"));

 // MobileUnit
 addPropertyId(IdSpeed, QString::fromLatin1("Speed"));
 addPropertyId(IdMovingFailed, QString::fromLatin1("MovingFailed"));
 addPropertyId(IdPathRecalculated, QString::fromLatin1("PathRecalculated"));

 // Facility
 addPropertyId(IdConstructionStep, QString::fromLatin1("ConstructionStep"));

 // UnitPlugin and derived classes
 addPropertyId(IdProductionState, QString::fromLatin1("ProductionState"));
 addPropertyId(IdResourcesMined, QString::fromLatin1("ResourcesMined"));
 addPropertyId(IdResourcesX, QString::fromLatin1("ResourcesX"));
 addPropertyId(IdResourcesY, QString::fromLatin1("ResourcesY"));
 addPropertyId(IdHarvestingType, QString::fromLatin1("HarvestingType"));

 mInitialized = true;
}

void Unit::select(bool markAsLeader)
{
 if (isDestroyed()) {
	return; // shall we really return?
 }
 BosonItem::select(markAsLeader);
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
 boDebug() << k_funcinfo << endl;
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
	boError() << "Ooop - maxHealth == 0" << endl;
	return;
 }
 UnitBase::setHealth(h);
 updateSelectBox();
 if (isDestroyed()) {
	unselect();
	setAnimationMode(UnitAnimationWreckage);
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
	boError() << k_funcinfo << id() << " does not have plugin " << pluginType << endl;
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

 // QCanvas::advance() uses a different approach than we do. They call moveBy()
 // from phase 1 and do interesting stuff like collision detection in phase 0.
 // we do collision detection of 1st unit and then move 1st unit and *then* do
 // collision detection of 2nd unit and then move 2nd unit, i.e. we do both
 // parts in a single phase.
 //
 // this will most probably cause trouble in the future, but it is necessary for
 // things like addToCells().

 float oldX = x();
 float oldY = y();

 float rotateX = 0.0f;
 float rotateY = 0.0f;
 updateZ(moveX, moveY, &moveZ, &rotateX, &rotateY);
 setXRotation(rotateX);
 setYRotation(rotateY);

 BosonItem::moveBy(moveX, moveY, moveZ);
 canvas()->unitMoved(this, oldX, oldY);
 /*if (smokeParticleSystem()) {
	 smokeParticleSystem()->moveParticles(BoVector3(moveX / BO_TILE_SIZE, -moveY / BO_TILE_SIZE, moveZ / BO_TILE_SIZE));
 }*/
}
void Unit::updateZ(float moveByX, float moveByY, float* moveByZ, float* rotateX, float* rotateY)
{
 // rotation is not yet implemented here
 *rotateX = 0.0f;
 *rotateY = 0.0f;

 float* heightMap = canvas()->heightMap();
 BO_CHECK_NULL_RET(heightMap);
 int heightMapWidth = canvas()->mapWidth() + 1;


 // FIXME: currently this is a 2-minute implementation. we simply ensure
 // that he unit is always on top of all cells it occupies.
 // This should be taken into account when calculating the moving
 // distance. Also it should be rotated. e.g. if 3 of 4 corners are at
 // 0.0 and the forth corner is at 1.0 then it currently has z=1.0, but
 // it should be rotated as real units would be.
 // facilities are a special case - we have not yet fully decided what
 // will happen if they are not on a "flat" surface. probably they won't
 // get rotated, but will have a "box" below them which make the surface
 // flat (i.e. the current implementation will remain for facilities).
 // but thats not fully sure.

 // we emulate cells() here. this is a) faster and we can b) prevent some
 // redundant checls (even faster).
 int left, top, right, bottom;
 leftTopCell(&left, &top, leftEdge() + moveByX, topEdge() + moveByY);
 rightBottomCell(&right, &bottom, rightEdge() + moveByX, bottomEdge() + moveByY);
 right = QMIN(right, QMAX((int)canvas()->mapWidth() - 1, 0));

 // ensure that the values are valid
 bottom = QMIN(bottom, QMAX((int)canvas()->mapHeight() - 1, 0));
 left = QMAX(left, 0);
 top = QMAX(top, 0);
 right = QMAX(left, right);
 bottom = QMAX(top, bottom);

 // we need left/right and upper/lower corner. so:
 right++;
 bottom++;

 // a final sanity check:
 if (right >= heightMapWidth) {
	boError() << k_funcinfo << "invalid right corner: " << right << endl;
	return;
 }
 if (bottom >= (int)canvas()->mapHeight() + 1) {
	boError() << k_funcinfo << "invalid bottom corner: " << bottom << endl;
	return;
 }

 float newZ = heightMap[top * heightMapWidth + left];
 for (int x = left; x <= right; x++) {
	for (int y = top; y <= bottom; y++) {
		if (heightMap[y * heightMapWidth + x] > newZ) {
			newZ = heightMap[y * heightMapWidth + x];
		}
	}
 }

 if (isFlying()) {
	if (newZ < z() + *moveByZ) {
		// don't touch moveByZ.
		return;
	}
 }
 *moveByZ = newZ - z();
}

void Unit::advance(unsigned int advanceCount)
{ // time critical function !!!
 if (isDestroyed()) {
	// FIXME: won't this make problems with wreckage animations? We should call
	//  BosonItem::advance() in any way (even if we're destroyed)
	return;
 }
 // Reload weapons
 QPtrListIterator<BosonWeapon> it(d->mWeapons);
 while (it.current()) {
	it.current()->advance(advanceCount);
	++it;
 }
// Reload shields
 reloadShields(); // AB: maybe we make that method inline one day.

// Mostly animation:
 BosonItem::advance(advanceCount);
}

void Unit::advanceNone(unsigned int advanceCount)
{
// this is called when the unit has nothing specific to do. Usually we just want
// to fire at every enemy in range.

 if (advanceCount % 10 != 0) {
	return;
 }

 if (!target() && (advanceCount % 20 != 0)) {
	return;
 }
 attackEnemyUnitsInRange();
}

bool Unit::attackEnemyUnitsInRange()
{
 if (!unitProperties()->canShoot()) {
	return false;
 }

 // TODO: Note that this is not completely realistic nor good: it may be good to
 //  e.g. not waste some weapon with very big damage and reload values for very
 //  weak unit. So there room left for improving :-)
 QPtrListIterator<BosonWeapon> wit(d->mWeapons);
 BosonWeapon* w;
 while (( w = wit.current()) != 0) {
	++wit;

	if (!w->reloaded()) {
		continue;
	}

	// We use target to store best enemy in range so we don't have to look for it every time.
	// If there's no target or target isn't in range anymore, find new best enemy unit in range
	// FIXME: check for max(Land|Air)WeaponRange
	if (!target() || target()->isDestroyed() ||
			!inRange(unitProperties()->maxWeaponRange(), target())) {
		d->mTarget = bestEnemyUnitInRange();
		if (!target()) {
			return false;
		}
	}

	// If unit is mobile, rotate to face the target if it isn't facing it yet
	if (isMobile()) {
		float rot = rotationToPoint(target()->x() - x(), target()->y() - y());
		if (rot < rotation() - 5 || rot > rotation() + 5) {
			// Rotate to face target
			if (QABS(rotation() - rot) > (2 * speed())) {
				turnTo((int)rot);
				setAdvanceWork(WorkTurn);
				return true;
			} else {
				// If we can get wanted rotation with only little turning, then we don't call turnTo()
				setRotation(rot);
			}
		}
	}

	// And finally... let it have everything we've got
	if (w->canShootAt(target()) && inRange(w->properties()->range(), target())) {
		shootAt(w, target());
		if (target()->isDestroyed()) {
			d->mTarget = 0l;
		}
	}
 }

 return true;

}

Unit* Unit::bestEnemyUnitInRange()
{
 // Return if unit can't shoot
 if (!unitProperties()->canShoot()) {
	return 0l;
 }
 // Return if no enemies in range
 BoItemList list = enemyUnitsInRange(unitProperties()->maxWeaponRange());
 if (!list.count() > 0) {
	return 0l;
 }

 // Initialize some variables
 Unit* best = 0l;
 BoItemList::Iterator it = list.begin();
 Unit* u = 0l;
 float dist = 0;
 // Candidates to best unit, see below
 Unit* c1 = 0l;
 Unit* c2 = 0l;
 Unit* c3 = 0l;

 // Iterate through the list of enemies and pick the best ones
 for (; it != list.end(); ++it) {
	u = ((Unit*)*it);
	dist = QMAX(QABS((int)(u->x() - x()) / BO_TILE_SIZE), QABS((int)(u->y() - y()) / BO_TILE_SIZE));
	// Quick check if we can shoot at u
	if (u->isFlying()) {
		if (!unitProperties()->canShootAtAirUnits()) {
			continue;
		}
		if (dist > unitProperties()->maxAirWeaponRange()) {
			continue;
		}
	} else {
		if (!unitProperties()->canShootAtLandUnits()) {
			continue;
		}
		if (dist > unitProperties()->maxLandWeaponRange()) {
			continue;
		}
	}

	// Check if it's the best unit so far.
	// This is presedence of enemies:
	//  1. enemies that can shoot at us
	//  2. enemies that can shoot, but not at us
	//  3. others
	// Shoot at units that can shoot first, then at units that cannot shoot
	if (u->unitProperties()->canShoot()) {
		if ((isFlying() && u->unitProperties()->canShootAtAirUnits()) ||
				(!isFlying() && u->unitProperties()->canShootAtLandUnits())) {
			// u is type 1 - it can shoot at us
			// TODO: check also for health here - first kill weaker units
			c1 = u;
		} else {
			// u is type 2 - it can shoot but not at us
			c2 = u;
		}
	} else {
		// u is type 3 - it can't shoot
		c3 = u;
	}
 }

 // Pick the best unit from the candidates
 if (c1) {
	best = c1;
 } else if (c2) {
	best = c2;
 } else if (c3) {
	best = c3;
 }
 return best;
}

void Unit::advanceAttack(unsigned int advanceCount)
{
 if (advanceCount % 5 != 0) {
	return;
 }

 boDebug(300) << k_funcinfo << endl;
 if (!target()) {
	boWarning() << k_funcinfo << "cannot attack NULL target" << endl;
	stopAttacking();
	return;
 }
 if (target()->isDestroyed()) {
	boDebug(300) << "Target is destroyed!" << endl;
	stopAttacking();
	return;
 }

 boDebug(300) << "    " << k_funcinfo << "checking if unit's in range" << endl;
 int range;
 if (target()->isFlying()) {
	range = unitProperties()->maxAirWeaponRange();
 } else {
	range = unitProperties()->maxLandWeaponRange();
 }

 if (!inRange(range, target())) {
	if (!canvas()->allItems().contains(target())) {
		boDebug(300) << "Target seems to be destroyed!" << endl;
		stopAttacking();
		return;
	}
	boDebug(300) << "unit (" << target()->id() << ") not in range - moving..." << endl;
	if (!moveTo(target()->x(), target()->y(), range)) {
		setWork(WorkNone);
	} else {
		setAdvanceWork(WorkMove);
	}
	return;
 }

 if(isMobile()) {
	float rot = rotationToPoint(target()->x() - x(), target()->y() - y());
	if(rot < rotation() - 5 || rot > rotation() + 5) {
		if(QABS(rotation() - rot) > (2 * speed())) {
			turnTo((int)rot);
			setAdvanceWork(WorkTurn);
			return;
		} else {
			setRotation(rot);
		}
	}
 }

 // Shoot at target with as many weapons as possible
 boDebug(300) << "    " << k_funcinfo << "shooting at target" << endl;
 QPtrListIterator<BosonWeapon> wit(d->mWeapons);
 BosonWeapon* w;
 while (( w = wit.current()) != 0) {
	++wit;
	if (w->reloaded() && w->canShootAt(target()) && inRange(w->properties()->range(), target())) {
		shootAt(w, target());
		if (target()->isDestroyed()) {
			boDebug(300) << "    " << k_funcinfo << "target destroyed, returning" << endl;
			stopAttacking();
			return;
		}
	}
 }
 boDebug(300) << "    " << k_funcinfo << "done shooting" << endl;
}

void Unit::advanceDestroyed(unsigned int advanceCount)
{
 // note: the unit/wreckage will get deleted pretty soon
 if (advanceCount % 20 != 0) {
	return;
 }
}

void Unit::advancePlugin(unsigned int advanceCount)
{
 if (!currentPlugin()) {
	boWarning() << k_funcinfo << "NULL plugin!" << endl;
	setWork(WorkNone);
 } else {
	currentPlugin()->advance(advanceCount);
 }
}

void Unit::advanceTurn(unsigned int)
{
 int dir = (int)rotation(); // wanted heading
 bool turnright; // direction of turning
 // Find out direction of turning
 // TODO: maybe we can cache it somewhere so we'd only have to calculate it
 //  when we want to turn. OTOH, it shouldn't be very expensive calculation
 if (dir >= 180) {
	if ((d->mWantedRotation < dir) && (d->mWantedRotation >= dir - 180)) {
		turnright = false;
	} else {
		turnright = true;
	}
 } else {
	if ((d->mWantedRotation > dir) && (d->mWantedRotation <= dir + 180)) {
		turnright = true;
	} else {
		turnright = false;
	}
 }

 // FIXME: This algorithm _sucks_. Replace it with something better
 for (int i = 0; i < (int)(2 * speed())/*TURN_STEP*/; i++) {
	if (dir == d->mWantedRotation) {
		break;
	}
	if (turnright) {
		dir += 1;
	} else {
		dir -= 1;
	}
	// Check for overflows
	if (dir < 0) {
		dir += 360;
	} else if (dir > 360) {
		dir -= 360;
	}
 }

 setRotation((float)dir);

 if (d->mWantedRotation == dir) {
	if (work() == WorkTurn) {
		setWork(WorkNone);
	} else if (advanceWork() != work()) {
		setAdvanceWork(work());
	}
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

void Unit::moveTo(const QPoint& pos, bool attack)
{
 d->mTarget = 0;
 if (moveTo(pos.x(), pos.y(), 0, attack)) {
	setWork(WorkMove);
 } else {
	setWork(WorkNone);
 }
}

bool Unit::moveTo(float x, float y, int range, bool attack)
{

 stopMoving();

 if (range == -1) {
	range = d->mMoveRange;
 }
 int cellX = (int)(x / BO_TILE_SIZE);
 int cellY = (int)(x / BO_TILE_SIZE);
 if (!canvas()->cell(cellX, cellY)) {
	boError() << k_funcinfo << x << "," << y << " is no valid cell!" << endl;
	return false;
 }
 if (!owner()->isFogged(cellX, cellY)) {
	Cell* c = canvas()->cell(cellX, cellY);
	if (!c) {
		boError() << k_funcinfo << "unit " << id() << ": NULL cell at " << x << "," << y << endl;
		return false;
	}
	// No pathfinding if goal not reachable or occupied and we can see it
	if (!c->canGo(unitProperties())) {
		boDebug() << k_funcinfo << "unit " << id() << ": Can't go to " << x << "," << y << endl;
		return false;
	}
 }

 d->mMoveDestX = (int)x;
 d->mMoveDestY = (int)y;
 d->mMoveRange = range;

 // Do not find path here!!! It would break pathfinding for groups. Instead, we
 //  set mSearchPath to true and find path in MobileUnit::advanceMove()
 mSearchPath = true;
 setMoving(true);

 if (attack) {
	d->mMoveAttacking = 1;
 } else {
	d->mMoveAttacking = 0;
 }

 return true;
}

void Unit::newPath()
{
 // FIXME: don't check if cell is valid/invalid if range > 0
 //  then unit would behave correctly when e.g. commanding land unit to attack
 //  a ship
 if (!canvas()->cell(d->mMoveDestX / BO_TILE_SIZE, d->mMoveDestY / BO_TILE_SIZE)) {
	boError() << k_funcinfo << "invalid cell " << d->mMoveDestX / BO_TILE_SIZE << "," << d->mMoveDestY / BO_TILE_SIZE << endl;
	return;
 }
 if (!owner()->isFogged(d->mMoveDestX / BO_TILE_SIZE, d->mMoveDestY / BO_TILE_SIZE)) {
	Cell* destCell = canvas()->cell(d->mMoveDestX / BO_TILE_SIZE,
			d->mMoveDestY / BO_TILE_SIZE);
	if (!destCell || (!destCell->canGo(unitProperties()))) {
		// If we can't move to destination, then we add waypoint with coordinates
		//  -1; -1 and in MobileUnit::advanceMove(), if currentWaypoint()'s
		//  coordinates are -1; -1 then we stop moving.
		boDebug() << k_funcinfo << "unit " << id() << ": Null cell or can't go to " << d->mMoveDestX << "," << d->mMoveDestY << endl;
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
 if ((currentWaypoint().x() == x() + width() / 2) && (currentWaypoint().y() == y() + height() / 2))
 {
	boDebug() << k_funcinfo << "!!!!! First waypoint is unit's current pos! Removing" << endl;
	waypointDone();
 }
 if (waypointCount() == 0)
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
// boDebug() << k_funcinfo << endl;
 clearWaypoints();

 // Call this only if we are only moving - stopMoving() is also called e.g. on
 // WorkAttack, when the unit is not yet in range.
 if (work() == WorkMove) {
	setWork(WorkNone);
 } else if (advanceWork() != work()) {
	setAdvanceWork(work());
 }
 setMoving(true);
 setVelocity(0.0, 0.0, 0.0);
}

void Unit::stopAttacking()
{
 stopMoving(); // FIXME not really intuitive... nevertheless its currently useful.
 setTarget(0);
 setWork(WorkNone);
}

bool Unit::save(QDataStream& stream)
{
 //we should probably add pure virtual methods save() and load() to the plugins,
 //in order to store non-KGameProperty data there, too
 // note that UnitBase::save() also saves KGameProperty data of plugins and
 // weapons
 if (!UnitBase::save(stream)) {
	boError() << "Unit not saved properly" << endl;
	return false;
 }
 stream << (float)x();
 stream << (float)y();
 stream << (float)z();

 // store the current plugin:
 int pluginIndex = 0;
 if (currentPlugin()) {
	pluginIndex = d->mPlugins.findRef(currentPlugin());
 }
 stream << (Q_INT32)pluginIndex;

 // also store the target:
 unsigned long int targetId = 0;
 if (target()) {
	targetId = target()->id();
 }
 stream << (Q_UINT32)targetId;

 return true;
}

bool Unit::load(QDataStream& stream)
{
 if (!UnitBase::load(stream)) {
	boError() << "Unit not loaded properly" << endl;
	return false;
 }
 //AB: the frame number was loaded/saved as well. i removed this because we
 //shouldn't have this in loading code. frame is dependant on the current
 //actions/animations/... but not relevant to actual game code. it is just the
 //visible appearance
 float x;
 float y;
 float z;
 Q_INT32 pluginIndex;
 Q_UINT32 targetId;

 stream >> x;
 stream >> y;
 stream >> z;
 stream >> pluginIndex;
 stream >> targetId;

 if (pluginIndex <= 0) {
	mCurrentPlugin = 0;
 } else {
	if ((unsigned int)pluginIndex >= d->mPlugins.count()) {
		boWarning() << k_funcinfo << "Invalid current plugin index: " << pluginIndex << endl;
		mCurrentPlugin = 0;
	} else {
		mCurrentPlugin = d->mPlugins.at(pluginIndex);
	}
 }

 // note: don't use setTarget() here, as it does some additional calculations
 if (targetId == 0) {
	d->mTarget = 0;
 } else {
	d->mTarget = boGame->findUnit(targetId, 0);
	if (!d->mTarget) {
		boWarning() << k_funcinfo << "Could not find target with unitId=" << targetId << endl;
	}
 }

 move(x, y, z);
 setAdvanceWork(advanceWork());
 return true;
}

bool Unit::inRange(unsigned long int r, Unit* target) const
{
 return (QMAX(QABS((int)(target->x() - x()) / BO_TILE_SIZE), QABS((int)(target->y() - y()) / BO_TILE_SIZE)) <= (int)r);
}

void Unit::shootAt(BosonWeapon* w, Unit* target)
{
 if (!w->reloaded()) {
//	boDebug() << "gotta reload first" << endl;
	return;
 }
 if (!w->canShootAt(target)) {
	boDebug() << k_funcinfo << "can't shoot at target!" << endl;
	return;
 }
 if (target->isDestroyed()) {
	boWarning() << k_funcinfo << target->id() << " is already destroyed" << endl;
	return;
 }
// boDebug() << id() << " shoots at unit " << target->id() << endl;
 w->shoot(target);
 owner()->statistics()->increaseShots();
}

BoItemList Unit::unitsInRange(unsigned long int r) const
{
 // TODO: we use a *rect* for the range this is extremely bad.
 // ever heard about pythagoras ;-) ?

 long int range = r; // To get rid of some warnings
 QPointArray cells;
 int left, right, top, bottom;
 leftTopCell(&left, &top);
 rightBottomCell(&right, &bottom);
 left = QMAX(left - range, 0);
 top = QMAX(top - range, 0);
 right = QMIN(right + range, QMAX((int)canvas()->mapWidth() - 1, 0));
 bottom = QMIN(bottom + range, QMAX((int)canvas()->mapHeight() - 1, 0));
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

 BoItemList items = canvas()->collisionsAtCells(cells, (BosonItem*)this, false);
 items.remove((BosonItem*)this);
 BoItemList inRange;
 BoItemList::Iterator it = items.begin();
 Unit* u;
 for (; it != items.end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	u = (Unit*)(*it);
	if (u->isDestroyed()) {
		continue;
	}
	if (owner()->isFogged(u->x() / BO_TILE_SIZE, u->y() / BO_TILE_SIZE)) {
		continue;
	}
	// TODO: remove the items from inRange which are not actually in range (hint:
	// pythagoras)
	inRange.append(*it);
 }
 return inRange;
}

BoItemList Unit::enemyUnitsInRange(unsigned long int range) const
{
 BoItemList units = unitsInRange(range);
 BoItemList enemy;
 Unit* u;
 BoItemList::Iterator it = units.begin();
 for (; it != units.end(); ++it) {
	u = (Unit*)*it;
	if (owner()->isEnemy(u->owner())) {
		enemy.append(u);
	}
 }
 return enemy;
}

QValueList<Unit*> Unit::unitCollisions(bool exact)
{
 QValueList<Unit*> units;
 boDebug(310) << k_funcinfo << endl;
 BoItemList collisionList = canvas()->collisionsAtCells(cells(), (BosonItem*)this, exact);
 if (collisionList.isEmpty()) {
	return units;
 }

 BoItemList::Iterator it;
 Unit* unit;
 for (it = collisionList.begin(); it != collisionList.end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	unit = ((Unit*)*it);
	if (unit->isDestroyed()) {
		continue;
	}
	if (unit->isFlying() != isFlying()) {
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
 setVelocity(0.0, 0.0, 0.0);

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
	case WorkTurn:
		setAdvanceFunction(&Unit::advanceTurn, owner()->advanceFlag());
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
		boDebug() << "ok - inrange" << endl;
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

void Unit::turnTo(int deg)
{
// boDebug() << k_funcinfo << deg << endl;
 d->mWantedRotation = deg;
}

BosonParticleSystem* Unit::smokeParticleSystem() const
{
 return d->mSmokeParticleSystem;
}

void Unit::setSmokeParticleSystem(BosonParticleSystem* s)
{
 d->mSmokeParticleSystem = s;
}

QPtrList<BosonParticleSystem>* Unit::activeParticleSystems() const
{
 return &(d->mActiveParticleSystems);
}

void Unit::setActiveParticleSystems(QPtrList<BosonParticleSystem> list)
{
 d->mActiveParticleSystems = list;
}

void Unit::loadWeapons()
{
 QPtrListIterator<PluginProperties> it(*(unitProperties()->plugins()));
 while (it.current()) {
	if (it.current()->pluginType() == PluginProperties::Weapon) {
		if (d->mWeapons.count() < MAX_WEAPONS_PER_UNIT) {
			d->mWeapons.append(new BosonWeapon(d->mWeapons.count(), (BosonWeaponProperties*)(it.current()), this));
		} else {
			boError() << k_funcinfo << "Too many weapons in this unit! type=" << type() << endl;
			return;
		}
	}
	++it;
 }
}

bool Unit::canShootAt(Unit *u)
{
 QPtrListIterator<BosonWeapon> it(d->mWeapons);
 while (it.current()) {
	if (it.current()->canShootAt(u)) {
		return true;
	}
	++it;
 }
 return false;
}

int Unit::moveAttacking() const
{
 return d->mMoveAttacking;
}


/////////////////////////////////////////////////
// MobileUnit
/////////////////////////////////////////////////

class MobileUnit::MobileUnitPrivate
{
public:
	MobileUnitPrivate()
	{
	}

	KGameProperty<float> mSpeed;
	KGameProperty<unsigned int> mMovingFailed;
	KGameProperty<unsigned int> mPathRecalculated;
};

MobileUnit::MobileUnit(const UnitProperties* prop, Player* owner, BosonCanvas* canvas) : Unit(prop, owner, canvas)
{
 d = new MobileUnitPrivate;

 registerData(&d->mSpeed, IdSpeed);
 registerData(&d->mMovingFailed, IdMovingFailed);
 registerData(&d->mPathRecalculated, IdPathRecalculated);

 d->mSpeed.setLocal(0);
 d->mMovingFailed.setLocal(0);
 d->mPathRecalculated.setLocal(0);

 d->mMovingFailed.setEmittingSignal(false);
 d->mPathRecalculated.setEmittingSignal(false);

 setWork(WorkNone);

 setRotation((float)(owner->game()->random()->getLong(359)));
 ((Boson*)owner->game())->slotUpdateProductionOptions();

 setActiveParticleSystems(unitProperties()->newConstructedParticleSystems(x() + width() / 2, y() + height() / 2, z()));
 canvas->addParticleSystems(*activeParticleSystems());
}

MobileUnit::~MobileUnit()
{
 delete d;
}

void MobileUnit::advanceMoveInternal(unsigned int advanceCount) // this actually needs to be called for every advanceCount.
{
 if (speed() == 0) {
	boWarning(401) << k_funcinfo << "unit " << id() << ": speed == 0" << endl;
	stopMoving();
	return;
 }

 if (mSearchPath) {
	newPath();
	mSearchPath = false;
	// Do we have to return here?
	return;
 }

 if (waypointCount() == 0) {
	// Waypoints were PolicyClean previously but are now PolicyLocal so they
	//  should arrive immediately. If there are no waypoints but advanceMove is
	//  called, then probably there's an error somewhere
	boError(401) << k_funcinfo << "unit " << id() << ": No waypoints" << endl;
	stopMoving();
	return;
 }

 boDebug(401) << k_funcinfo << "unit " << id() << endl;
 if (advanceWork() != work()) {
	if (work() == WorkAttack) {
		// no need to move to the position of the unit...
		// just check if unit is in range now.
		// TODO: maybe cache range somewhere. OTOH, I don't think it would make things much faster
		int range;
		if (target()->isFlying()) {
			range = unitProperties()->maxAirWeaponRange();
		} else {
			range = unitProperties()->maxLandWeaponRange();
		}
		if (inRange(range, target())) {
			boDebug(401) << k_funcinfo << "unit " << id() << ": target is in range now" << endl;
			stopMoving();
			return;
		}
		// TODO: make sure that target() hasn't moved!
		// if it has moved also adjust waypoints
		// RL: I'm not sure if it's needed because path will be recalced every time
		//  new cell is reached anyway
	}
 } else if (moveAttacking()) {
	// Attack any enemy units in range
	// Don't check for enemies every time (if we don't have a target) because it
	//  slows things down
	if (target() || (advanceCount % 20 == 0)) {
		if (attackEnemyUnitsInRange()) {
			boDebug(401) << k_funcinfo << "unit " << id() << ": Enemy units found in range, attacking" << endl;
			setVelocity(0.0, 0.0, 0.0);  // To prevent moving
			setMoving(false);
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

 int x = (int)(BosonItem::x() + width() / 2);
 int y = (int)(BosonItem::y() + height() / 2);

 float xspeed = 0;
 float yspeed = 0;

 // First check if we're at waypoint
 if ((x == wp.x()) && (y == wp.y())) {
	//boDebug(401) << k_funcinfo << "unit " << id() << ": unit is at waypoint" << endl;
	waypointDone();

	if (waypointCount() == 0) {
		//boDebug(401) << k_funcinfo << "unit " << id() << ": no more waypoints. Stopping moving" << endl;
		stopMoving();
		if (work() == WorkNone) {
			// Turn a bit
			int turn = (int)rotation() + (owner()->game()->random()->getLong(90) - 45);
			// Check for overflows
			if (turn < 0) {
				turn += 360;
			} else if (turn > 360) {
				turn -= 360;
			}
			Unit::turnTo(int(turn));
			setWork(WorkTurn);
		}
		return;
	}

	// We now recalc path _every_ time we reach waypoint
	//  Units will then react more quickly when other units move and block their
	//  way for example
	newPath();

	wp = currentWaypoint();
 }

 // Try to go to same x and y coordinates as waypoint's coordinates
 // First x coordinate
 // Slow down if there is less than speed() pixels to go
 if (QABS(wp.x() - x) < speed()) {
	xspeed = wp.x() - x;
 } else {
	xspeed = speed();
	if (wp.x() < x) {
		xspeed = -xspeed;
	}
 }
 // Same with y coordinate
 if (QABS(wp.y() - y) < speed()) {
	yspeed = wp.y() - y;
 } else {
	yspeed = speed();
	if (wp.y() < y) {
		yspeed = -yspeed;
	}
 }

 // Set velocity for actual moving
 setVelocity(xspeed, yspeed, 0.0);
 setMoving(true);

 // set the new direction according to new speed
 turnTo();
}

void MobileUnit::advanceMoveCheck()
{
 if (!canvas()->onCanvas(boundingRectAdvanced().topLeft())) {
	boDebug(401) << k_funcinfo << "unit " << id() << ": not on canvas (topLeft): (" <<
			(int)(leftEdge() + xVelocity()) << "; " << (int)(topEdge() + yVelocity()) << ")" << endl;
	stopMoving();
	setWork(WorkNone);
	return;
 }
 if (!canvas()->onCanvas(boundingRectAdvanced().bottomRight())) {
	boDebug(401) << k_funcinfo << "unit " << id() << ": not on canvas (bottomRight): (" <<
			(int)(leftEdge() + width() + xVelocity()) << "; " << (int)(topEdge() + height() + yVelocity()) << ")" << endl;
	stopMoving();
	setWork(WorkNone);
	return;
 }
 //boDebug(401) << k_funcinfo << "unit " << id() << endl;
 if (canvas()->cellOccupied(currentWaypoint().x() / BO_TILE_SIZE,
		currentWaypoint().y() / BO_TILE_SIZE, this, false)) {
//	boDebug(401) << k_funcinfo << "collisions" << endl;
//	boWarning(401) << k_funcinfo << "" << id() << " -> " << l.first()->id() 
//		<< " (count=" << l.count() <<")"  << endl;
	// do not move at all. Moving is not stopped completely!
	// work() is still workMove() so we'll continue moving in the next
	// advanceMove() call

	d->mMovingFailed = d->mMovingFailed + 1;
	setVelocity(0.0, 0.0, 0.0);
	// If we haven't yet recalculated path, consider unit to be moving
	setMoving(d->mPathRecalculated == 0);

	const int recalculate = 50; // recalculate when 50 advanceMove() failed
	
	if (d->mPathRecalculated >= 2) {
		boDebug(401) << k_funcinfo << "unit: " << id() << ": Path recalculated 3 times and it didn't help, giving up and stopping" << endl;
		stopMoving();
		return;
	}
	if (d->mMovingFailed >= recalculate) {
		boDebug(401) << "unit " << id() << ": recalculating path" << endl;
		// you must not do anything that changes local variables directly here!
		// all changed of variables with PolicyClean are ok, as they are sent
		// over network and do not take immediate effect.

		newPath();
		d->mMovingFailed = 0;
		d->mPathRecalculated = d->mPathRecalculated + 1;
	}
	return;
 }
 d->mMovingFailed = 0;
 d->mPathRecalculated = 0;
 //boDebug(401) << k_funcinfo << "unit " << id() << ": done" << endl;
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
	boError() << k_funcinfo << "unit is already destroyed!" << endl;
	return;
 }
 // At the moment, all units are facing south by default, but currect would be
 //  north so change direction hack as soon as it's fixed
 float dir = (int)direction * 45;
 if (rotation() != dir) {
	Unit::turnTo((int)dir);
	setAdvanceWork(WorkTurn);
 }
}

void MobileUnit::turnTo()
{
 float xspeed = xVelocity();
 float yspeed = yVelocity();
 // Set correct frame
 if ((xspeed == 0) && (yspeed < 0)) { // North
 	turnTo(North);
 } else if ((xspeed > 0) && (yspeed < 0)) { // NE
	turnTo(NorthEast);
 } else if ((xspeed > 0) && (yspeed == 0)) { // East
	turnTo(East);
 } else if ((xspeed > 0) && (yspeed > 0)) { // SE
	turnTo(SouthEast);
 } else if ((xspeed == 0) && (yspeed > 0)) { // South
	turnTo(South);
 } else if ((xspeed < 0) && (yspeed > 0)) { // SW
	turnTo(SouthWest);
 } else if ((xspeed < 0) && (yspeed == 0)) { // West
	turnTo(West);
 } else if ((xspeed < 0) && (yspeed < 0)) { // NW
	turnTo(NorthWest);
 } else if (xspeed == 0 && yspeed == 0) {
//	boDebug() << k_funcinfo << "xspeed == 0 and yspeed == 0" << endl;
 } else {
	boDebug() << k_funcinfo << "error when setting frame" << endl;
 }
}

void MobileUnit::advanceFollow(unsigned int advanceCount)
{
 if (advanceCount % 5 != 0) {
	return;
 }
 if (!target()) {
	boWarning() << k_funcinfo << "cannot follow NULL unit" << endl;
	stopAttacking();  // stopAttacking should maybe be renamed to stopEverything
			//  or just stop because it's used in several places to stop unit from
			//  doing whatever it does.
	return;
 }
 if (target()->isDestroyed()) {
	boDebug(401) << k_funcinfo << "Unit is destroyed!" << endl;
	stopAttacking();
	return;
 }
// if (!isNextTo(target())) {  // This doesn't work for some reason :-(  Dunno why.
 if (QMAX(QABS(x() - target()->x()), QABS(y() - target()->y())) > BO_TILE_SIZE) {
	// We're not next to unit
	if (!canvas()->allItems().contains(target())) {
		boDebug(401) << k_funcinfo << "Unit seems to be destroyed!" << endl;
		stopAttacking();
		return;
	}
	boDebug(401) << k_funcinfo << "unit (" << target()->id() << ") not in range - moving..." << endl;
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
	boWarning() << k_funcinfo << "width or height  < BO_TILE_SIZE - not supported!!" << endl;
	return BosonItem::boundingRect();
 }
 return QRect((int)x(), (int)y(), BO_TILE_SIZE, BO_TILE_SIZE);
}

bool MobileUnit::load(QDataStream& stream)
{
 if (!Unit::load(stream)) {
	boError() << "Unit not loaded properly" << endl;
	return false;
 }

 return true;
}

bool MobileUnit::save(QDataStream& stream)
{
 if (!Unit::save(stream)) {
	boError() << "Unit not loaded properly" << endl;
	return false;
 }

 return true;
}

void MobileUnit::stopMoving()
{
 Unit::stopMoving();
 // Reset moveCheck variables
 d->mMovingFailed = 0;
 d->mPathRecalculated = 0;
}


/////////////////////////////////////////////////
// Facility
/////////////////////////////////////////////////

class Facility::FacilityPrivate
{
public:
	FacilityPrivate()
	{
		mFlamesParticleSystem = 0;
	}

	KGameProperty<unsigned int> mConstructionStep;

	BosonParticleSystem* mFlamesParticleSystem;
};

Facility::Facility(const UnitProperties* prop, Player* owner, BosonCanvas* canvas) : Unit(prop, owner, canvas)
{
 d = new FacilityPrivate;

 registerData(&d->mConstructionStep, IdConstructionStep);

 d->mConstructionStep.setLocal(0);

 setWork(WorkConstructed);
}

Facility::~Facility()
{
 // TODO: write a plugin framework and manage plugins in a list.
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
	boError() << k_funcinfo << "unit is already destroyed" << endl;
	return;
 }
 setConstructionStep(currentConstructionStep() + 1);
}

UnitPlugin* Facility::plugin(int pluginType) const
{
 if (!isConstructionComplete()) {
	return 0;
 }
 return Unit::plugin(pluginType);
}

bool Facility::isConstructionComplete() const
{
 if (work() == WorkConstructed) {
	return false;
 }
 if (currentConstructionStep() < constructionSteps()) {
	return false;
 }
 return true;
}

double Facility::constructionProgress() const
{
 unsigned int constructionTime = constructionSteps();
 double percentage = (double)(currentConstructionStep() * 100) / (double)constructionTime;
 return percentage;
}

void Facility::setTarget(Unit* u)
{
 if (u && !isConstructionComplete()) {
	boWarning() << k_funcinfo << "not yet constructed completely" << endl;
	return;
 }
 Unit::setTarget(u);
}

void Facility::moveTo(float x, float y, int range)
{
 if (!isConstructionComplete()) {
	boWarning() << k_funcinfo << "not yet constructed completely" << endl;
	return;
 }
 Unit::moveTo(x, y, range);
}

void Facility::setConstructionStep(unsigned int step)
{
 if (isDestroyed()) {
	boError() << k_funcinfo << "unit is already destroyed" << endl;
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
//	boDebug() << k_funcinfo << "step="<<step<<",modelstep="<<modelStep<<endl;
 }
 setGLConstructionStep(modelStep); // the displayed construction step. note that currentConstructionStep() is a *different* value!
 d->mConstructionStep = step;
 if (step == constructionSteps()) {
	setWork(WorkNone);
	owner()->facilityCompleted(this);
	((Boson*)owner()->game())->slotUpdateProductionOptions();
	setAnimationMode(UnitAnimationIdle);
	setActiveParticleSystems(unitProperties()->newConstructedParticleSystems(x() + width() / 2, y() + height() / 2, z()));
	canvas()->addParticleSystems(*activeParticleSystems());
 }
}

unsigned int Facility::currentConstructionStep() const
{
 return d->mConstructionStep;
}

BosonParticleSystem* Facility::flamesParticleSystem() const
{
 return d->mFlamesParticleSystem;
}

void Facility::setFlamesParticleSystem(BosonParticleSystem* s)
{
 d->mFlamesParticleSystem = s;
}

bool Facility::load(QDataStream& stream)
{
 if (!Unit::load(stream)) {
	boError() << "Unit not loaded properly" << endl;
	return false;
 }

 setConstructionStep(d->mConstructionStep);

 return true;
}

bool Facility::save(QDataStream& stream)
{
 if (!Unit::save(stream)) {
	boError() << "Unit not loaded properly" << endl;
	return false;
 }

 return true;
}
