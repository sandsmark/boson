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

#include "unit.h"
#include "player.h"
#include "playerio.h"
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
#include "boson.h"
#include "bosonparticlesystem.h"
#include "bosonweapon.h"
#include "bopointeriterator.h"
#include "bodebug.h"
#include "bo3dtools.h"

#include <kgame/kgamepropertylist.h>
#include <kgame/kgame.h>
#include <krandomsequence.h>

#include <qpointarray.h>
#include <qptrlist.h>
#include <qdom.h>

#include <math.h>

#include "defines.h"

#define MAX_PATH_AGE 5

#define USE_SMOOTH_HEIGHT


bool Unit::mInitialized = false;

class Unit::UnitPrivate
{
public:
	UnitPrivate()
	{
		mTarget = 0;

		mWeapons = 0;
	}
	KGamePropertyList<QPoint> mWaypoints;
	KGamePropertyList<QPoint> mPathPoints;
	KGameProperty<int> mMoveDestX;
	KGameProperty<int> mMoveDestY;
	KGameProperty<int> mMoveRange;
	KGameProperty<int> mWantedRotation;
	KGameProperty<int> mMoveAttacking;
	KGameProperty<int> mSearchPath;
	KGameProperty<int> mSlowDownAtDestination;

	// be *very* careful with those - NewGameDialog uses Unit::save() which
	// saves all KGameProperty objects. If non-KGameProperty properties are
	// changed before all players entered the game we'll have a broken
	// network game.
	Unit* mTarget;

	// these must NOT be touched (items added or removed) after the c'tor.
	// loading code will depend in this list to be at the c'tor state!
	QPtrList<UnitPlugin> mPlugins;
	BosonWeapon** mWeapons;

	// OpenGL only:
	QPtrList<BosonParticleSystem> mParticleSystems;  // No autodelete!!!

	BosonPathInfo mPathInfo;
};

Unit::Unit(const UnitProperties* prop, Player* owner, BosonCanvas* canvas)
		: UnitBase(prop, owner, canvas)
{
 if (!mInitialized) {
	initStatic();
 }
 d = new UnitPrivate;
 mCurrentPlugin = 0;
 mAdvanceFunction = &Unit::advanceNone;
 mAdvanceFunction2 = &Unit:: advanceNone;
 d->mPlugins.setAutoDelete(true);
 d->mPlugins.clear();

 // note: these width and height can be used for e.g. pathfinding. It does not
 // depend in any way on the .3ds file or another OpenGL thing.
 setSize(prop->unitWidth(), prop->unitHeight(), prop->unitDepth());

 registerData(&d->mWaypoints, IdWaypoints);
 registerData(&d->mPathPoints, IdPathPoints);
 registerData(&d->mMoveDestX, IdMoveDestX);
 registerData(&d->mMoveDestY, IdMoveDestY);
 registerData(&d->mMoveRange, IdMoveRange);
 registerData(&d->mWantedRotation, IdWantedRotation);
 registerData(&d->mMoveAttacking, IdMoveAttacking);
 registerData(&d->mSearchPath, IdSearchPath);
 registerData(&d->mSlowDownAtDestination, IdSlowDownAtDestination);
 d->mMoveDestX.setLocal(0);
 d->mMoveDestY.setLocal(0);
 d->mMoveRange.setLocal(0);
 d->mWantedRotation.setLocal(0);
 d->mMoveAttacking.setLocal(0);
 d->mSearchPath.setLocal(0);
 d->mSlowDownAtDestination.setLocal(1);

 setAnimated(true);

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
 if (prop->properties(PluginProperties::Refinery)) {
	boDebug() << k_funcinfo << "Unit has refinery properties!!!" << endl;
	boWarning() << k_funcinfo << "TODO: refinery plugin" << endl;
//	d->mPlugins.append(new RefinePlugin(this));
 }
 if (prop->properties(PluginProperties::ResourceMine)) {
	d->mPlugins.append(new ResourceMinePlugin(this));
 }

 loadWeapons();
}

Unit::~Unit()
{
 d->mWaypoints.setEmittingSignal(false); // just to prevent warning in Player::slotUnitPropertyChanged()
 d->mWaypoints.clear();
 unselect();
 d->mPlugins.clear();
 BoPointerIterator<BosonWeapon> it = d->mWeapons;
 for (; *it; ++it) {
	delete *it;
 }
 delete[] d->mWeapons;
 delete d;
}

void Unit::initStatic()
{
 // we initialize the properties for Unit, MobileUnit, Facility and the plugins
 // here
 // Unit
 addPropertyId(IdWaypoints, QString::fromLatin1("Waypoints"));
 addPropertyId(IdPathPoints, QString::fromLatin1("PathPoints"));
 addPropertyId(IdMoveDestX, QString::fromLatin1("MoveDestX"));
 addPropertyId(IdMoveDestY, QString::fromLatin1("MoveDestY"));
 addPropertyId(IdMoveRange, QString::fromLatin1("MoveRange"));
 addPropertyId(IdWantedRotation, QString::fromLatin1("WantedRotation"));
 addPropertyId(IdMoveAttacking, QString::fromLatin1("MoveAttacking"));
 addPropertyId(IdSearchPath, QString::fromLatin1("SearchPath"));
 addPropertyId(IdSlowDownAtDestination, QString::fromLatin1("SlowDownAtDestination"));

 // MobileUnit
 addPropertyId(IdMovingFailed, QString::fromLatin1("MovingFailed"));
 addPropertyId(IdPathRecalculated, QString::fromLatin1("PathRecalculated"));
 addPropertyId(IdPathAge, QString::fromLatin1("PathAge"));

 // Facility
 addPropertyId(IdConstructionStep, QString::fromLatin1("ConstructionStep"));

 // UnitPlugin and derived classes
 addPropertyId(IdProductionState, QString::fromLatin1("ProductionState"));
 addPropertyId(IdResourcesMined, QString::fromLatin1("ResourcesMined"));
 addPropertyId(IdResourcesX, QString::fromLatin1("ResourcesX"));
 addPropertyId(IdResourcesY, QString::fromLatin1("ResourcesY"));
 addPropertyId(IdHarvestingType, QString::fromLatin1("HarvestingType"));
 addPropertyId(IdBombingPosX, QString::fromLatin1("BombingPosX"));
 addPropertyId(IdBombingPosY, QString::fromLatin1("BombingPosY"));
 addPropertyId(IdMinePlacingCounter, QString::fromLatin1("MinePlacingCounter"));
 addPropertyId(IdResourceMineMinerals, QString::fromLatin1("ResourceMineMinerals"));
 addPropertyId(IdResourceMineOil, QString::fromLatin1("ResourceMineOil"));

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
	selectBox()->update((float)health() / unitProperties()->health());
 }
}

void Unit::moveBy(float moveX, float moveY, float moveZ)
{
// time critical function
 if (!moveX && !moveY && !moveZ) {
	return;
 }

 if (isDestroyed()) {
	// Just move the unit and return. No need to update z or minimap
	BosonItem::moveBy(moveX, moveY, moveZ);
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
}

void Unit::updateZ(float moveByX, float moveByY, float* moveByZ, float* rotateX, float* rotateY)
{
 // rotation is not yet implemented here
 *rotateX = 0.0f;
 *rotateY = 0.0f;

#ifndef USE_SMOOTH_HEIGHT
 const float* heightMap = canvas()->heightMap();
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
#else
 float centerx = x() + moveByX + width() / 2;
 float centery = y() + moveByY + height() / 2;

 float newZ = canvas()->heightAtPoint(centerx, centery);

 // Calculate rotation stuff
 float realx = 0.0f, realy = 0.0f;
 // Calculate x rotation
 Bo3dTools::pointByRotation(&realx, &realy, rotation(), height() / 2);
 float rearz, frontz;
 frontz = canvas()->heightAtPoint(centerx + realx, centery + realy);
 rearz = canvas()->heightAtPoint(centerx - realx, centery - realy);
 // Calculate angle from frontz to rearz
 float xrot = Bo3dTools::rad2deg(atan(QABS(frontz - rearz) * BO_TILE_SIZE / height()));
 *rotateX = (frontz >= rearz) ? xrot : -xrot;

 // Calculate y rotation
 float myrot = rotation() + 90;
 if (myrot > 360) {
	myrot -= 360;
 }
 Bo3dTools::pointByRotation(&realx, &realy, myrot, width() / 2);
 float rightz, leftz;
 rightz = canvas()->heightAtPoint(centerx + realx, centery + realy);
 leftz = canvas()->heightAtPoint(centerx - realx, centery - realy);
 // Calculate angle from leftz to rightz
 float yrot = Bo3dTools::rad2deg(atan(QABS(rightz - leftz) * BO_TILE_SIZE / width()));
 *rotateY = (leftz >= rightz) ? yrot : -yrot;

#endif

 if (isFlying()) {
	newZ += 2.0f;  // Flying units are always 2 units above the ground
 }
 *moveByZ = newZ - z();
}

void Unit::advance(unsigned int advanceCount)
{ // time critical function !!!
// Mostly animation:
 BosonItem::advance(advanceCount);

 if (isDestroyed()) {
	return;
 }
 // Reload weapons
 if (d->mWeapons[0]) {
	BosonWeapon** w = &d->mWeapons[0];
	for (; *w; w++) {
		(*w)->advance(advanceCount);
	}
 }

 // Reload shields
 if (shields() < unitProperties()->shields()) {
	reloadShields(); // AB: maybe we make that method inline one day.
 }
}

void Unit::advanceNone(unsigned int advanceCount)
{
// this is called when the unit has nothing specific to do. Usually we just want
// to fire at every enemy in range.

 if (!target()) {
	if (advanceCount != 10) {
		return;
	}
 } else if (advanceCount % 10 != 0) {
	return;
 }

 attackEnemyUnitsInRange();
}

bool Unit::attackEnemyUnitsInRange()
{
 if (!unitProperties()->canShoot()) {
	return false;
 }
 if (!d->mWeapons[0]) {
	return false;
 }

 // TODO: Note that this is not completely realistic nor good: it may be good to
 //  e.g. not waste some weapon with very big damage and reload values for very
 //  weak unit. So there room left for improving :-)
 BoPointerIterator<BosonWeapon> wit(d->mWeapons);
 for (; *wit; ++wit) {
	BosonWeapon* w = *wit;

	if (!w->properties()->autoUse()) {
		continue;
	}

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
		float rot = Bo3dTools::rotationToPoint(target()->x() - x(), target()->y() - y());
		if (rot < rotation() - 5 || rot > rotation() + 5) {
			// Rotate to face target
			if (QABS(rotation() - rot) > unitProperties()->rotationSpeed()) {
				turnTo((int)rot);
				setAdvanceWork(WorkTurn);
				return true;
			} else {
				// If we can get wanted rotation with only little turning, then we don't call turnTo()
				setRotation(rot);
				updateRotation();
			}
		}
	}

	// And finally... let it have everything we've got
	if (w->canShootAt(target()) && inRange(w->properties()->range(), target())) {
		shootAt(w, target());
		if (target()->isDestroyed()) {
			d->mTarget = 0;
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
 BoItemList* list = enemyUnitsInRange(unitProperties()->maxWeaponRange());
 if (!list->count() > 0) {
	return 0;
 }

 // Initialize some variables
 Unit* best = 0l;
 BoItemList::Iterator it = list->begin();
 Unit* u = 0l;
 float dist = 0;
 // Candidates to best unit, see below
 Unit* c1 = 0l;
 Unit* c2 = 0l;
 Unit* c3 = 0l;

 // Iterate through the list of enemies and pick the best ones
 for (; it != list->end(); ++it) {
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
	boWarning() << k_funcinfo << id() << " cannot attack NULL target" << endl;
	stopAttacking();
	return;
 }
 if (target()->isDestroyed()) {
	boDebug(300) << "Target (" << target()->id() << ") is destroyed!" << endl;
	stopAttacking();
	return;
 }

 boDebug(300) << "    " << k_funcinfo << "checking if unit " << target()->id() << ") is in range" << endl;
 int range;
 if (target()->isFlying()) {
	range = unitProperties()->maxAirWeaponRange();
 } else {
	range = unitProperties()->maxLandWeaponRange();
 }

 if (!inRange(range, target())) {
	// AB: warning - this does a lookup on all items and therefore is slow!
	// --> but we need it as a simple test on the pointer causes trouble if
	// that pointer is already deleted. any nice solutions?
	if (!canvas()->allItems()->contains(target())) {
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

 if (isMobile()) {
	float rot = Bo3dTools::rotationToPoint(target()->x() - x(), target()->y() - y());
	if(rot < rotation() - 5 || rot > rotation() + 5) {
		if(QABS(rotation() - rot) > unitProperties()->rotationSpeed()) {
			turnTo((int)rot);
			setAdvanceWork(WorkTurn);
			return;
		} else {
			setRotation(rot);
			updateRotation();
		}
	}
 }

 // Shoot at target with as many weapons as possible
 boDebug(300) << "    " << k_funcinfo << "shooting at target" << endl;
 BoPointerIterator<BosonWeapon> wit(d->mWeapons);
 BosonWeapon* w;
 for (; *wit; ++wit) {
	w = *wit;
	if (w->properties()->autoUse() && w->reloaded() && w->canShootAt(target()) && inRange(w->properties()->range(), target())) {
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
 if (advanceCount % 10 != 0) {
	return;
 }
 if (isVisible()) {
	// Make unit slowly sink into ground
	setVelocity(0, 0, -(depth() / (REMOVE_WRECKAGES_TIME * MAXIMAL_ADVANCE_COUNT)) * 1.2);
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
 int dir = (int)rotation();
 int a = dir - d->mWantedRotation;  // How many degrees to turn
 bool turncw = false;  // Direction of turning, CW or CCW

 // First find out direction of turning and huw much is left to turn
 if (a < 0) {
	a = QABS(a);
	turncw = true;
 }
 if (a > 180) {
	a = 180 - (a - 180);
	turncw = !turncw;
 }

 if (a <= unitProperties()->rotationSpeed()) {
	dir = d->mWantedRotation;
 } else {
	if (turncw) {
		dir += unitProperties()->rotationSpeed();
	} else {
		dir -= unitProperties()->rotationSpeed();
	}
 }
 // Check for overflows
 if (dir < 0) {
	dir += 360;
 } else if (dir > 360) {
	dir -= 360;
 }

 setRotation((float)dir);
 updateRotation();

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

const QValueList<QPoint>& Unit::waypointList() const
{
 return d->mWaypoints;
}

unsigned int Unit::waypointCount() const
{
 return d->mWaypoints.count();
}

void Unit::resetPathInfo()
{
 if(pathInfo()->hlpath) {
	canvas()->pathfinder()->releaseHighLevelPath(pathInfo()->hlpath);
	pathInfo()->hlpath = 0;
 }
 d->mPathInfo.reset();
 d->mPathInfo.unit = this;
 d->mPathInfo.canMoveOnLand = unitProperties()->canGoOnLand();
 d->mPathInfo.canMoveOnWater = unitProperties()->canGoOnWater();
}


void Unit::moveTo(const QPoint& pos, bool attack)
{
 d->mTarget = 0;

#ifdef PATHFINDER_TNG
 // We want unit's center point to be in the middle of the cell after moving.
 float x = pos.x() / BO_TILE_SIZE * BO_TILE_SIZE + (BO_TILE_SIZE / 2);
 float y = pos.y() / BO_TILE_SIZE * BO_TILE_SIZE + (BO_TILE_SIZE / 2);


 if (moveTo(x, y, 0)) {
	addWaypoint(QPoint((int)x, (int)y));
	d->mPathInfo.moveAttacking = attack;
	d->mPathInfo.slowDownAtDest = true;
	setWork(WorkMove);
 } else {
	setWork(WorkNone);
	stopMoving();
 }
#else
 if (moveTo(pos.x(), pos.y(), 0, attack)) {
	setWork(WorkMove);
 } else {
	setWork(WorkNone);
 }
#endif // PATHFINDER_TNG
}

#ifndef PATHFINDER_TNG

bool Unit::moveTo(float x, float y, int range, bool attack, bool slowDownAtDest)
{
 // If we're moving already, we don't want to set our speed to 0
 // Set slowDownAtDestination temporarily to false to achieve that.
 // Maybe also when advanceWork() == WorkMove ?
 if (!ownerIO()) {
	BO_NULL_ERROR(ownerIO());
	return false;
 }
 if (work() == WorkMove) {
	d->mSlowDownAtDestination = 0;
 }
 stopMoving();

 if (range == -1) {
	range = d->mMoveRange;
 }
 int cellX = (int)(x / BO_TILE_SIZE);
 int cellY = (int)(y / BO_TILE_SIZE);
 Cell* cell = canvas()->cell(cellX, cellY);
 if (!cell) {
	boError() << k_funcinfo << x << "," << y << " is no valid cell!" << endl;
	setMovingStatus(Standing);
	return false;
 }
 if (ownerIO()->canSee(cell)) {
	if (!ownerIO()->canGo(this, cell)) {
		// No pathfinding if goal not reachable or occupied and we can see it
		boDebug() << k_funcinfo << "unit " << id() << ": Can't go to " << x << "," << y << endl;
		setMovingStatus(Standing);
		return false;
	}
 }

 // Center of the destination cell
 d->mMoveDestX = (int)(cellX * BO_TILE_SIZE + width() / 2);
 d->mMoveDestY = (int)(cellY * BO_TILE_SIZE + height() / 2);
 d->mMoveRange = range;

 // Do not find path here!!! It would break pathfinding for groups. Instead, we
 //  set mSearchPath to true and find path in MobileUnit::advanceMove()
 setSearchPath(1);
 setMoving(true);
 setMovingStatus(Moving);

 if (attack) {
	d->mMoveAttacking = 1;
 } else {
	d->mMoveAttacking = 0;
 }
 if (slowDownAtDest) {
	d->mSlowDownAtDestination = 1;
 } else {
	d->mSlowDownAtDestination = 0;
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
 if ((currentWaypoint().x() == x() + width() / 2) && (currentWaypoint().y() == y() + height() / 2)) {
	boDebug() << k_funcinfo << "!!!!! First waypoint is unit's current pos! Removing" << endl;
	waypointDone();
 }
 if (waypointCount() == 0) {
	// Pathfinder now adds -1; -1 itself, so this shouldn't be reached
	boError() << k_funcinfo << "!!!!! No valid points in path !!!!! Fix pathfinder !!!!!" << endl;
	addWaypoint(QPoint(-1, -1));
 }
 return;
}

#else // ! PATHFINDER_TNG

bool Unit::moveTo(float x, float y, int range, bool /*attack*/, bool /*slowDownAtDest*/)
{
 // Range -1 means to use previously set range
 if (range == -1) {
	range = d->mPathInfo.range;
 }

 // Find destination cell
 int cellX = (int)(x / BO_TILE_SIZE);
 int cellY = (int)(y / BO_TILE_SIZE);
 Cell* cell = canvas()->cell(cellX, cellY);
 if (!cell) {
	boError() << k_funcinfo << "unit " << id() << ": Cell (" << cellX << "; " << cellY << ") at (" <<
			x << "; " << y << ") is not valid!" << endl;
	return false;
 }

 // Check if unit can go to destination cell.
 // FIXME: this doesn't support units which occupy multiple cells!
 //  We could iterate through all the cells unit would occupy, but that would be
 //  too slow...
 if (range == 0) {
	if (ownerIO()->canSee(cell)) {
		if (!ownerIO()->canGo(this, cell)) {
			// No pathfinding if goal not reachable or occupied and we can see it
			boDebug() << k_funcinfo << "unit " << id() << ": Can't go to " << x << "," << y << endl;
			return false;
		}
	}
 }

 // Update path info
 resetPathInfo();
 d->mPathInfo.dest.setX((int)x);
 d->mPathInfo.dest.setY((int)y);
 d->mPathInfo.range = range;

 // Remove old way/pathpoints
 // TODO: maybe call stopMoving() instead and remove setMovingStatus(Standing)
 //  from there...
 clearWaypoints();
 clearPathPoints();


 // Path is not searched here (it would break pathfinding for groups). Instead,
 //  moving status is set to MustSearch and in MobileUnit::advanceMove(), path
 //  is searched for.
 setMovingStatus(MustSearch);

 return true;
}

void Unit::newPath()
{
 // Update our start position
 d->mPathInfo.start.setX((int)(BosonItem::x() + width() / 2));
 d->mPathInfo.start.setY((int)(BosonItem::y() + height() / 2));
 // Check if we can still go to cell (it may be that cell was fogged previously,
 //  so we couldn't check, but now it's unfogged)
 // FIXME: don't check if cell is valid/invalid if range > 0
 //  then unit would behave correctly when e.g. commanding land unit to attack
 //  a ship
 int cellX = d->mPathInfo.dest.x() / BO_TILE_SIZE;
 int cellY = d->mPathInfo.dest.y() / BO_TILE_SIZE;
 if (!owner()->isFogged(cellX, cellY)) {
	Cell* destCell = canvas()->cell(cellX, cellY);
	if (!destCell || (!destCell->canGo(unitProperties()))) {
		// If we can't move to destination, then we add special pathpoint with
		//  coordinates (PF_TNG_CANNOT_GO; PF_TNG_CANNOT_GO) and in
		//  MobileUnit::advanceMove(), we check for these special codes
		boDebug() << k_funcinfo << "unit " << id() << ": Null cell or can't go to (" <<
				d->mPathInfo.dest.x() << "; " << d->mPathInfo.dest.y() << ") (cell (" << cellX << "; " << cellY << "))" << endl;
		clearPathPoints();
		addPathPoint(QPoint(PF_TNG_CANNOT_GO, PF_TNG_CANNOT_GO));
		return;
	}
 }

 // Find path
 canvas()->pathfinder()->findPath(&d->mPathInfo);

 // Copy low-level path to pathpoints' list
 clearPathPoints();
 for (int unsigned i = 0; i < d->mPathInfo.llpath.count(); i++) {
	addPathPoint(d->mPathInfo.llpath[i]);
 }

 if (pathPointCount() == 0) {
	// Pathfinder now adds -1; -1 itself, so this shouldn't be reached
	boError() << k_funcinfo << "!!!!! No valid points in path !!!!! Fix pathfinder !!!!!" << endl;
	addPathPoint(QPoint(PF_TNG_CANNOT_GO, PF_TNG_CANNOT_GO));
 }
 return;
}

#endif // ! PATHFINDER_TNG


void Unit::clearWaypoints()
{
 d->mWaypoints.clear();
}

const QPoint& Unit::currentWaypoint() const
{
 return d->mWaypoints[0];
}

void Unit::addPathPoint(const QPoint& pos)
{
 d->mPathPoints.append(pos);
}

unsigned int Unit::pathPointCount() const
{
 return d->mPathPoints.count();
}

const QPoint& Unit::currentPathPoint()
{
 return d->mPathPoints.first();
}

void Unit::clearPathPoints()
{
 d->mPathPoints.clear();
}

void Unit::pathPointDone()
{
 d->mPathPoints.pop_front();
}

const QValueList<QPoint>& Unit::pathPointList() const
{
 return d->mPathPoints;
}

#ifdef PATHFINDER_TNG
void Unit::stopMoving()
{
// boDebug() << k_funcinfo << endl;
 clearWaypoints();
 clearPathPoints();

 // Release high-level path. We check if it exists here, because it's ok if it
 //  doesn't. That's the case when e.g. path wasn't found
 if(pathInfo()->hlpath) {
	canvas()->pathfinder()->releaseHighLevelPath(pathInfo()->hlpath);
	pathInfo()->hlpath = 0;
	pathInfo()->hlstep = 0;
 }

 // Call this only if we are only moving - stopMoving() is also called e.g. on
 // WorkAttack, when the unit is not yet in range.
 if (work() == WorkMove) {
	setWork(WorkNone);
 } else if (advanceWork() != work()) {
	setAdvanceWork(work());
 }
 setMovingStatus(Standing);
 setVelocity(0.0, 0.0, 0.0);
}
#else
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
 setMoving(false);
 setVelocity(0.0, 0.0, 0.0);
}
#endif // PATHFINDER_TNG

void Unit::stopAttacking()
{
 stopMoving(); // FIXME not really intuitive... nevertheless its currently useful.
#ifdef PATHFINDER_TNG
 setMovingStatus(Standing);
#endif
 setTarget(0);
 setWork(WorkNone);
}

bool Unit::saveAsXML(QDomElement& root)
{
 // we should probably add pure virtual methods save() and load() to the plugins,
 // in order to store non-KGameProperty data there, too
 // note that UnitBase::save() also saves KGameProperty data of plugins and
 // weapons
 if (!UnitBase::saveAsXML(root)) {
	boError() << "Unit not saved properly" << endl;
	return false;
 }

 // AB: we use too many attributes ... maybe we should add a few more elements :)
 root.setAttribute(QString::fromLatin1("x"), x());
 root.setAttribute(QString::fromLatin1("y"), y());
 root.setAttribute(QString::fromLatin1("z"), z());
 root.setAttribute(QString::fromLatin1("Rotation"), rotation());
 // No need to store x and y rotations

 // store the current plugin:
 if (currentPlugin()) {
	int pluginIndex = d->mPlugins.findRef(currentPlugin());
	root.setAttribute(QString::fromLatin1("CurrentPlugin"), pluginIndex);
 } else {
	// the unit won't have this attribute.
 }

 // also store the target:
 unsigned long int targetId = 0;
 if (target()) {
	targetId = target()->id();
 }
 root.setAttribute(QString::fromLatin1("Target"), (unsigned int)targetId);

 if (d->mPlugins.count() != 0) {
	QDomDocument doc = root.ownerDocument();
	QDomElement pluginElement = doc.createElement(QString::fromLatin1("UnitPlugin"));
	QPtrListIterator<UnitPlugin> it(d->mPlugins);
	for (; it.current(); ++it) {
		it.current()->saveAsXML(pluginElement);

		// we won't add the element at all, if it wasn't used.
		if (pluginElement.hasAttributes() || pluginElement.hasChildNodes()) {
			pluginElement.setAttribute(QString::fromLatin1("Type"), it.current()->pluginType());

			// AB: atm we support a single instance only anyway.
			pluginElement.setAttribute(QString::fromLatin1("Instance"), 0);
			root.appendChild(pluginElement);

			// create an element for the next plugin
			pluginElement = doc.createElement(QString::fromLatin1("UnitPlugin"));
		}
	}
 }


 // TODO: somehow save active particle systems

 return true;
}

bool Unit::loadFromXML(const QDomElement& root)
{
 if (!UnitBase::loadFromXML(root)) {
	boError(260) << k_funcinfo << "Unit not loaded properly" << endl;
	return false;
 }
 bool ok = false;
 float rotation = root.attribute(QString::fromLatin1("Rotation")).toFloat(&ok);
 if (!ok) {
	boError(260) << k_funcinfo << "Invalid value for Rotation tag" << endl;
	rotation = 0.0f;
 }

 int pluginIndex = 0;
 if (root.hasAttribute(QString::fromLatin1("CurrentPlugin"))) {
	pluginIndex = root.attribute(QString::fromLatin1("CurrentPlugin")).toInt(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "Invalid value for CurrentPlugin tag" << endl;
		pluginIndex = 0;
	}
	if ((unsigned int)pluginIndex >= d->mPlugins.count()) {
		boWarning(260) << k_funcinfo << "Invalid current plugin index: " << pluginIndex << endl;
		pluginIndex = 0;
	}
	mCurrentPlugin = d->mPlugins.at(pluginIndex);
 } else {
	mCurrentPlugin = 0;
 }

 // note: don't use setTarget() here, as it does some additional calculations
 unsigned int targetId = 0;
 if (root.hasAttribute(QString::fromLatin1("Target"))) {
	targetId = root.attribute(QString::fromLatin1("Target")).toUInt(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "Invalid value for Target tag" << endl;
		targetId = 0;
	}
 }
 if (targetId == 0) {
	d->mTarget = 0;
 } else {
	d->mTarget = boGame->findUnit(targetId, 0);
	if (!d->mTarget) {
		boWarning(260) << k_funcinfo << "Could not find target with unitId=" << targetId << endl;
	}
 }

 if (d->mPlugins.count() != 0) {
	QDomNodeList list = root.elementsByTagName(QString::fromLatin1("UnitPlugin"));
	for (unsigned int i = 0; i < list.count(); i++) {
		QDomElement e = list.item(i).toElement();
		if (e.isNull()) {
			continue;
		}
		unsigned int type;
		unsigned int instance;
		bool ok = false;
		type = e.attribute(QString::fromLatin1("Type")).toUInt(&ok);
		if (!ok) {
			boError() << k_funcinfo << "Type of UnitPlugin " << i << " is not a valid number" << endl;
			return false;
		}
		instance = e.attribute(QString::fromLatin1("Instance")).toUInt(&ok);
		if (!ok) {
			boError() << k_funcinfo << "Instance of UnitPlugin " << i << " is not a valid number" << endl;
			return false;
		}
		if (instance != 0) {
			boError() << k_funcinfo << "instance != 0 for UnitPlugin " << i << " is not yet supported." << endl;
			return false;
		}
		UnitPlugin* p = plugin(type);
		Q_UNUSED(instance);
		if (!p) {
			boWarning() << k_funcinfo << "UnitPlugin " << type << " not found for unit " << id() << endl;
			continue;
		}
		p->loadFromXML(e);
	}
 }
 setRotation(rotation);
 updateRotation();
 setAdvanceWork(advanceWork());
 return true;
}

bool Unit::inRange(unsigned long int r, Unit* target) const
{
 return (QMAX(QABS((target->x() - x()) / BO_TILE_SIZE), QABS((target->y() - y()) / BO_TILE_SIZE)) <= (float)r);
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
 ownerIO()->statistics()->increaseShots();
}

BoItemList* Unit::unitsInRange(unsigned long int range) const
{
 // TODO: we use a *rect* for the range this is extremely bad.
 // ever heard about pythagoras ;-) ?

 int left, right, top, bottom;
 leftTopCell(&left, &top);
 rightBottomCell(&right, &bottom);
 // AB: note that we don't need to do error checking like left < 0, since
 // collisions() does this anyway.
 QRect rect;
 rect.setCoords(left - range, top - range, right + range, bottom + range);

 // TODO: we should do this using PlayerIO. It should return items that are
 // actually visible to us only!
 BoItemList* items = collisions()->collisionsAtCells(rect, (BosonItem*)this, false);
 items->remove((BosonItem*)this);

 BoItemList* units = new BoItemList();
 BoItemList::Iterator it = items->begin();
 Unit* u;
 for (; it != items->end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	u = (Unit*)(*it);
	if (u->isDestroyed()) {
		continue;
	}
	if (!ownerIO()->canSee(u)) {
		continue;
	}
	if (!inRange(range, u)) {
		continue;
	}
	// TODO: remove the items from inRange which are not actually in range (hint:
	// pythagoras)
	units->append(*it);
 }
 return units;
}

BoItemList* Unit::enemyUnitsInRange(unsigned long int range) const
{
 BoItemList* units = unitsInRange(range);
 BoItemList* enemy = new BoItemList();
 Unit* u;
 BoItemList::Iterator it = units->begin();
 for (; it != units->end(); ++it) {
	u = (Unit*)*it;
	if (ownerIO()->isEnemyUnit(u)) {
		enemy->append(u);
	}
 }
 return enemy;
}

QValueList<Unit*> Unit::unitCollisions(bool exact)
{
 QValueList<Unit*> units;
 boDebug(310) << k_funcinfo << endl;
 BoItemList* collisionList = collisions()->collisionsAtCells(cells(), (BosonItem*)this, exact);
 if (collisionList->isEmpty()) {
	return units;
 }

 BoItemList::Iterator it;
 Unit* unit;
 for (it = collisionList->begin(); it != collisionList->end(); ++it) {
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

 if (advanceWork() != w) {
	// we change the list only if work() actually changed (in favor of
	// performance). but do not return here!
	canvas()->changeAdvanceList((BosonItem*)this);
 }
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
// boDebug() << k_funcinfo << id() << ": turning to " << deg << endl;
 d->mWantedRotation = deg;
}

const QPtrList<BosonParticleSystem>* Unit::particleSystems() const
{
 return &(d->mParticleSystems);
}

void Unit::clearParticleSystems()
{
 d->mParticleSystems.clear();
}

void Unit::setParticleSystems(const QPtrList<BosonParticleSystem>& list)
{
 d->mParticleSystems = list;
 canvas()->addParticleSystems(d->mParticleSystems);
}

void Unit::loadWeapons()
{
 if (d->mWeapons) {
	boWarning() << k_funcinfo << "Weapons already loaded! doing nothing." << endl;
	return;
 }

 // since we use an array and not a list we need to count the weapons first.
 int count = 0;
 bool hasbomb = false;
 bool hasmine = false;
 QPtrListIterator<PluginProperties> it(*(unitProperties()->plugins()));
 for (; it.current(); ++it) {
	if (it.current()->pluginType() == PluginProperties::Weapon) {
		count++;
		// Check for bombing weapons and create bombing plugin if necessary
		if (!hasbomb && ((BosonWeaponProperties*)it.current())->shotType() == BosonShot::Bomb) {
#warning FIXME: this does _not_ belog to plugins, but to weapons!
			d->mPlugins.append(new BombingPlugin(this));
			hasbomb = true;
		}
		// Check for mine weapons and create mining plugin if necessary
		if (!hasmine && ((BosonWeaponProperties*)it.current())->shotType() == BosonShot::Mine) {
#warning FIXME: this does _not_ belog to plugins, but to weapons!
			d->mPlugins.append(new MiningPlugin(this));
			hasmine = true;
		}
	}
 }
 if (count > MAX_WEAPONS_PER_UNIT) {
	boError() << k_funcinfo << "Too many weapons in this unit! type=" << type() << endl;
	count = MAX_WEAPONS_PER_UNIT;
 }
 d->mWeapons = new BosonWeapon*[count + 1]; // last element MUST be 0
 for (int i = 0; i < count + 1; i++) {
	d->mWeapons[i] = 0;
 }
 int weaponPos = 0;
 it.toFirst();
 for (; it.current() && weaponPos < count; ++it) {
	if (it.current()->pluginType() == PluginProperties::Weapon) {
		BosonWeaponProperties* prop = (BosonWeaponProperties*)it.current();
		d->mWeapons[weaponPos] = new BosonWeapon(weaponPos, prop, this);
		weaponPos++;
	}
 }
}

bool Unit::canShootAt(Unit *u)
{
 BoPointerIterator<BosonWeapon> it(d->mWeapons);
 for (; *it; ++it) {
	if ((*it)->canShootAt(u)) {
		return true;
	}
 }
 return false;
}

int Unit::moveAttacking() const
{
 return d->mMoveAttacking;
}

int Unit::slowDownAtDestination() const
{
 return d->mSlowDownAtDestination;
}

int Unit::distance(const Unit* u) const
{
 // !!! This method returns square of actual distance. You may want to use sqrt() !!!
 int dx = (int)((x() + width() / 2) - (u->x() + u->width() / 2));
 int dy = (int)((y() + height() / 2) - (u->y() + u->height() / 2));
 int dz = (int)(z() - u->z());
 return dx*dx + dy*dy + dz*dz;
}

int Unit::distance(const BoVector3& pos) const
{
 // !!! This method returns square of actual distance. You may want to use sqrt() !!!
 int dx = (int)(pos.x() - (x() + width() / 2));
 int dy = (int)(pos.y() - (y() + height() / 2));
 int dz = (int)(pos.z() - z());
 return dx*dx + dy*dy + dz*dz;
}

const QColor* Unit::teamColor() const
{
 return &ownerIO()->teamColor();
}

int Unit::searchPath() const
{
 return d->mSearchPath;
}

void Unit::setSearchPath(int search)
{
 d->mSearchPath = search;
}

BosonWeapon* Unit::weapon(unsigned long int id) const
{
 BoPointerIterator<BosonWeapon> it(d->mWeapons);
 for (; *it; ++it) {
	if ((*it)->properties()->id() == id) {
		return *it;
	}
 }
 return 0;
}

void Unit::updateRotation()
{
 float rotateX = 0.0f;
 float rotateY = 0.0f;
 float moveZ = 0.0f;
 updateZ(0.0f, 0.0f, &moveZ, &rotateX, &rotateY);
 setXRotation(rotateX);
 setYRotation(rotateY);
}

void Unit::setMovingStatus(MovingStatus m)
{
 if(movingStatus() == m) {
	return;
 }
 MovingStatus old = movingStatus();
 UnitBase::setMovingStatus(m);
#ifdef PATHFINDER_TNG
 canvas()->unitMovingStatusChanges(this, old, m);
#endif
}

BosonPathInfo* Unit::pathInfo() const
{
 return &d->mPathInfo;
}

void Unit::itemRemoved(BosonItem* item)
{
 UnitBase::itemRemoved(item);
 if (item == (BosonItem*)this) {
	return;
 }
 if ((BosonItem*)target() == item) {
	setTarget(0);
 }
 QPtrListIterator<UnitPlugin> it(d->mPlugins);
 while (it.current()) {
	it.current()->itemRemoved(item);
	++it;
 }
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

	KGameProperty<unsigned int> mMovingFailed;
	KGameProperty<unsigned int> mPathRecalculated;
	// Maybe it would be better to move this to Unit? Currently we have to reset
	//  it every time we search a new path. Then we could do it directly in
	//  newPath()
	KGameProperty<unsigned int> mPathAge;

	// Should this be made KGameProperty?
	bool mWayPointReached;  // FIXME: bad hack
	float lastxvelo;
	float lastyvelo;
};

MobileUnit::MobileUnit(const UnitProperties* prop, Player* owner, BosonCanvas* canvas) : Unit(prop, owner, canvas)
{
 d = new MobileUnitPrivate;

 registerData(&d->mMovingFailed, IdMovingFailed);
 registerData(&d->mPathRecalculated, IdPathRecalculated);
 registerData(&d->mPathAge, IdPathAge);

 d->mMovingFailed.setLocal(0);
 d->mPathRecalculated.setLocal(0);
 d->mPathAge.setLocal(0);
 d->mWayPointReached = false;

 d->mMovingFailed.setEmittingSignal(false);
 d->mPathRecalculated.setEmittingSignal(false);
 d->mPathAge.setEmittingSignal(false);

 setWork(WorkNone);

 ((Boson*)owner->game())->slotUpdateProductionOptions();

 setParticleSystems(unitProperties()->newConstructedParticleSystems(x() + width() / 2, y() + height() / 2, z()));

 setRotation((float)(owner->game()->random()->getLong(359)));
 updateRotation();
 setMaxSpeed(prop->speed());
 setAccelerationSpeed(prop->accelerationSpeed());
 setDecelerationSpeed(prop->decelerationSpeed());
 d->lastxvelo = 0.0f;
 d->lastyvelo = 0.0f;
}

MobileUnit::~MobileUnit()
{
 delete d;
}

#ifdef PATHFINDER_TNG
void MobileUnit::advanceMoveInternal(unsigned int advanceCount) // this actually needs to be called for every advanceCount.
{
 boDebug(401) << k_funcinfo << endl;

 if (pathInfo()->waiting) {
	// If path is blocker and we're waiting, then there's no point in
	//  recalculating velocity and other stuff every advance call
	return;
 }

 if (maxSpeed() == 0) {
	// If unit's max speed is 0, it cannot move
	boError(401) << k_funcinfo << "unit " << id() << ": maxspeed == 0" << endl;
	stopMoving();
	setMovingStatus(Standing);
	return;
 }

 // Check if path is already found or not
 if (movingStatus() == MustSearch) {
	// Path is not yet searched
	// If we're moving with attacking, first check for any enemies in the range.
	if (attackEnemyUnitsInRangeWhileMoving()) {
		return;
	}
	// If there aren't any enemies, find new path
	newPath();
 }

 // Make sure we have pathpoints
 if (pathPointCount() == 0) {
	// No pathpoints. We should have at least one point in any case, so most
	//  probably something is broken
	boError(401) << k_funcinfo << "unit " << id() << ": No pathpoints" << endl;
	// Try to find new path
	newPath();
	if (pathPointCount() == 0) {
		// Still no points - serious problem somewhere
		boError(401) << k_funcinfo << "unit " << id() << ": Still no pathpoints. Aborting" << endl;
		stopMoving();
		setMovingStatus(Standing);
		return;
	}
 }

 boDebug(401) << k_funcinfo << "unit " << id() << endl;
 if (advanceWork() != work()) {
	if (work() == WorkAttack) {
		// Unit is attacking. ATM it's moving to target.
		// no need to move to the position of the unit...
		// just check if unit is in range now.
		if (!target()) {
			boError() << k_funcinfo << id() << " is in WorkAttack, but has NULL target!" << endl;
			stopAttacking();
			setMovingStatus(Standing);
			return;
		}
		int range;
		if (target()->isFlying()) {
			range = unitProperties()->maxAirWeaponRange();
		} else {
			range = unitProperties()->maxLandWeaponRange();
		}
		if (inRange(range, target())) {
			boDebug(401) << k_funcinfo << "unit " << id() << ": target is in range now" << endl;
			setMovingStatus(Standing);
			stopMoving();
			return;
		}
		// TODO: make sure that target() hasn't moved!
		// if it has moved also adjust waypoints
	}
 } else if (pathInfo()->moveAttacking) {
	// Attack any enemy units in range
	// Don't check for enemies every time (if we don't have a target) because it
	//  slows things down
	if (target() || (advanceCount % 10 == 0)) {
		if (attackEnemyUnitsInRangeWhileMoving()) {
			return;
		}
	}
 }

 // x and y are center of the unit here
 int x = (int)(BosonItem::x() + width() / 2);
 int y = (int)(BosonItem::y() + height() / 2);
 boDebug(401) << k_funcinfo << "Unit pos: (" << x << "; "<< y << ")" << endl;
 // If we're close to destination, decelerate, otherwise  accelerate
 // TODO: we should also slow down when turning at pathpoint.
 if (pathInfo()->slowDownAtDest && QMAX(QABS(x - pathInfo()->dest.x()), QABS(y - pathInfo()->dest.y())) <= decelerationDistance()) {
	decelerate();
 } else {
	accelerate();
 }

 // Go speed() distance towards the next pathpoint. If distance to it is less
 //  than speed(), we go towards the one after this as well
 float xspeed = 0;
 float yspeed = 0;
 float dist = speed();
 QPoint pp;

 // We move through the waypoints, until we've passed dist distance
 while (dist > 0) {
	// Make sure we have more pathpoints left
	if (pathPointCount() == 0) {
		// Pathpoints' count can _not_ be 0, there is always special point at the end
		boError(401) << k_funcinfo << "unit " << id() << "No pathpoints!!!" << endl;
		return;
	}

	// Take next pathpoint
	if (checkPathPoint(currentPathPoint())) {
		break;
	}
	pp = currentPathPoint();
	boDebug(401) << k_funcinfo << "Current pp: (" << pp.x() << "; "<< pp.y() << ")" << endl;

	// Move towards it
	dist -= moveTowardsPoint(pp, x + xspeed, y + yspeed, dist, xspeed, yspeed);

	// Check if we reached that pathpoint
	if ((x + xspeed == pp.x()) && (y + yspeed == pp.y())) {
		// Unit has reached pathpoint
		boDebug(401) << k_funcinfo << "unit " << id() << ": unit is at pathpoint" << endl;
		pathPointDone();
		// Check for enemies
		if (attackEnemyUnitsInRangeWhileMoving()) {
			break;
		}
	}
 }

 if ((xspeed == 0.0f) && (yspeed == 0.0f)) {
	boWarning() << k_funcinfo << "Speed is 0.0" << endl;
	return;
 }

 boDebug(401) << k_funcinfo << "Setting velo to: (" << xspeed << "; "<< yspeed << ")" << endl;
 setVelocity(xspeed, yspeed, 0.0);
 d->lastxvelo = xspeed;
 d->lastyvelo = yspeed;
 setMovingStatus(Moving);

 // FIXME: turnTo() supports only 45*x degree angles
 turnTo();
}
#endif // PATHFINDER_TNG

float MobileUnit::moveTowardsPoint(const QPoint& p, float x, float y, float maxdist, float &xspeed, float &yspeed)
{
 boDebug(401) << k_funcinfo << "p: (" << p.x() << "; "<< p.y() << "); pos: (" << x << "; "<< y <<
		");  maxdist: " << maxdist << "; xspeed: " << xspeed << "; yspeed: " << yspeed << endl;
 // Passed distance
 float dist = 0.0f;
#ifdef PATHFINDER_TNG
 // Calculate difference between point and our current position
 float xdiff, ydiff;
 xdiff = p.x() - x;
 ydiff = p.y() - y;

 // Calculate how much to move on x-axis
 if (QABS(xdiff) < maxdist) {
	xspeed += xdiff;
	dist = QMAX(dist, xdiff);
 } else {
	xspeed += (xdiff > 0) ? maxdist : -maxdist;
	dist = QMAX(dist, (xdiff > 0) ? maxdist : -maxdist);
 }
 // Calculate how much to move on y-axis
 if (QABS(ydiff) < maxdist) {
	yspeed += ydiff;
	dist = QMAX(dist, ydiff);
 } else {
	yspeed += (ydiff > 0) ? maxdist : -maxdist;
	dist = QMAX(dist, (ydiff > 0) ? maxdist : -maxdist);
 }

 // Calculate passed distance
 // TODO: use pythagoras
#endif // PATHFINDER_TNG
 return dist;
}

#ifdef PATHFINDER_TNG
#define USE_NEW_COLLISION_DETECTION

void MobileUnit::advanceMoveCheck()
{
 boDebug(401) << k_funcinfo << endl;

 // Take special action if path is (was) blocked and we're waiting
 if (pathInfo()->waiting) {
	// We try to move every 5 advance calls
	if (pathInfo()->waiting % 5 == 0) {
		// Try to move again
#ifdef USE_NEW_COLLISION_DETECTION
		if(canvas()->collisionsInBox(BoVector3(x() + d->lastxvelo, y() + d->lastyvelo, z()),
				BoVector3(x() + d->lastxvelo + width(), y() + d->lastyvelo + height(), z() + depth()), this) &&
				canvas()->collisionsInBox(BoVector3(x() + d->lastxvelo * 5, y() + d->lastyvelo * 5, z()),
				BoVector3(x() + d->lastxvelo * 5 + width(), y() + d->lastyvelo * 5 + height(), z() + depth()), this)) {
#else
		if (canvas()->cellOccupied(currentPathPoint().x() / BO_TILE_SIZE,
				currentPathPoint().y() / BO_TILE_SIZE, this, false)) {
#endif
			// Obstacle is still there. Continue waiting
			if (pathInfo()->waiting >= 400) {
				// Enough of waiting (20 secs). Give up.
				setMovingStatus(Standing);
				stopMoving();
				setWork(WorkNone);
				return;
			} else if ((pathInfo()->waiting >= 20) && (pathInfo()->waiting % 20 == 0)) {
				// We will recalculate path every 20 advance calls (every 1 sec)
				newPath();
			}
			pathInfo()->waiting++;
			return;
		} else {
			// Our path is free again. We can continue moving next advance call, but
			//  not immediately, because advanceMoveInternal() didn't calculate
			//  velocities
			pathInfo()->waiting = 0;
			setMovingStatus(Waiting);  // TODO: is this ok here? maybe set to Moving instead?
			return;
		}
	} else {
		// Don't check if we can move every advance call. Just return for now
		pathInfo()->waiting++;
		return;
	}
 }

 // Check if top-left point of the unit will be on canvas after moving
 if (!canvas()->onCanvas(boundingRectAdvanced().topLeft())) {
	QPoint point = boundingRectAdvanced().topLeft();
	QString problem;
	if (!canvas()->onCanvas(QPoint(0, point.y()))) {
		problem = QString("top==%1").arg(point.y());
	} else if (!canvas()->onCanvas(QPoint(point.x(), 0))) {
		problem = QString("left==%1").arg(point.x());
	} else {
		boError(401) << k_funcinfo
				<< "internal error: (0," << point.y() <<
				") and (" << point.x()
				<< ",0) are on canvas, but (" << point.x() <<
				"," << point.y() << ") isn't !"
				<< endl;
		problem = "internal";
	}
	boError(401) << k_funcinfo << "unit " << id()
			<< " not on canvas (topLeftAdvanced): (" << point.x()
			<< ";" << point.y() << ")" << " problem was: "
			<< problem << endl;
	boError(401) << k_funcinfo << "leaving unit a current topleft pos: ("
			<< boundingRect().topLeft().x() << ";"
			<< boundingRect().topLeft().y() << ")" << endl;
	setMovingStatus(Standing);
	stopMoving();
	setWork(WorkNone);
	return;
 }
 // Check if bottom-right point of the unit will be on canvas after moving
 if (!canvas()->onCanvas(boundingRectAdvanced().bottomRight())) {
	QPoint point = boundingRectAdvanced().bottomRight();
	QString problem;
	if (!canvas()->onCanvas(QPoint(0, point.y()))) {
		problem = QString("bottom==%1").arg(point.y());
	} else if (!canvas()->onCanvas(QPoint(point.x(), 0))) {
		problem = QString("right==%1").arg(point.x());
	} else {
		boError(401) << k_funcinfo
				<< "internal error: (0," << point.y() <<
				") and (" << point.x()
				<< ",0) are on canvas, but (" << point.x() <<
				"," << point.y() << ") isn't !"
				<< endl;
	}
	boError(401) << k_funcinfo << "unit " << id()
			<< " not on canvas (bottomRightAdvanced): ("
			<< point.x() << ";" << point.y() << ")"
			<< "  current rightEdge: " << rightEdge()
			<< " ; current bottomEdge:" << bottomEdge()
			<< " ; xVelo: " << xVelocity()
			<< " ; (int)xVelo: " << (int)xVelocity()
			<< " ; yVelo: " << yVelocity()
			<< " ; (int)yVelo: " << (int)yVelocity()
			<< " problem was: "
			<< problem << endl;
	boError(401) << k_funcinfo << "leaving unit a current bottomright pos: ("
			<< boundingRect().bottomRight().x() << ";"
			<< boundingRect().bottomRight().y() << ")" << endl;
	setMovingStatus(Standing);
	stopMoving();
	setWork(WorkNone);
	return;
 }

 //boDebug(401) << k_funcinfo << "unit " << id() << endl;
 // Make sure our next pathpoint is not occupied
 // TODO: check for advanced bounding rect instead
#ifdef USE_NEW_COLLISION_DETECTION
 if (canvas()->collisionsInBox(BoVector3(x() + d->lastxvelo, y() + d->lastyvelo, z()),
		BoVector3(x() + d->lastxvelo + width(), y() + d->lastyvelo + height(), z() + depth()), this)) {
#else
 if (canvas()->cellOccupied(currentPathPoint().x() / BO_TILE_SIZE,
		currentPathPoint().y() / BO_TILE_SIZE, this, false)) {
#endif
	// Unit will collide with something next advance call
//	boDebug(401) << k_funcinfo << "collisions" << endl;
//	boWarning(401) << k_funcinfo << "" << id() << " -> " << l.first()->id()
//		<< " (count=" << l.count() <<")"  << endl;
	// do not move at all. Moving is not stopped completely!
	// work() is still workMove() so we'll continue moving in the next
	// advanceMove() call

	// Stop unit (note that it doesn't stop completely, work will still be moving)
	setVelocity(0.0, 0.0, 0.0);
	setSpeed(0);
	setMovingStatus(Waiting);

	// Increase waiting counter (how long unit has been waiting)
	pathInfo()->waiting++;
	return;
 }
#ifdef USE_NEW_COLLISION_DETECTION
 else if (canvas()->collisionsInBox(BoVector3(x() + d->lastxvelo * 5, y() + d->lastyvelo * 5, z()),
		BoVector3(x() + d->lastxvelo * 5 + width(), y() + d->lastyvelo * 5 + height(), z() + depth()), this)) {
	// Unit will collide with smth in 5 adv. calls
	setVelocity(0.0, 0.0, 0.0);
	setSpeed(0);
	setMovingStatus(Waiting);

	// Increase waiting counter (how long unit has been waiting)
	pathInfo()->waiting++;
	return;
 } else if (canvas()->collisionsInBox(BoVector3(x() + d->lastxvelo * 10, y() + d->lastyvelo * 10, z()),
		BoVector3(x() + d->lastxvelo * 10 + width(), y() + d->lastyvelo * 10 + height(), z() + depth()), this)) {
	// Unit will collide with smth in 10 adv. calls. Decelerate
	decelerate();
	decelerate();  // Yes, we decelerate _two_ times
	float s = speed();
	if (s > 0.0f) {
		d->lastxvelo = QMIN(d->lastxvelo, s);
		d->lastyvelo = QMIN(d->lastyvelo, s);
		setVelocity(d->lastxvelo, d->lastyvelo, 0.0);
	} else {
		// Stop and set moving status to Waiting
		setVelocity(0.0, 0.0, 0.0);
		setSpeed(0);
		setMovingStatus(Waiting);
		// Increase waiting counter (how long unit has been waiting)
		pathInfo()->waiting++;
		return;
	}
 }
#endif
 pathInfo()->waiting = 0;

 //boDebug(401) << k_funcinfo << "unit " << id() << ": done" << endl;
}

#else // PATHFINDER_TNG

void MobileUnit::advanceMoveInternal(unsigned int advanceCount) // this actually needs to be called for every advanceCount.
{
 if (maxSpeed() == 0) {
	// If unit's max speed is 0, it cannot move
	boError(401) << k_funcinfo << "unit " << id() << ": maxspeed == 0" << endl;
	stopMoving();
	setMovingStatus(Standing);
	return;
 }

 if (searchPath()) {
	// If we're moving with attacking, first check for any enemies in the range
	//  This prevents units from moving on when you order them to move somewhere
	//  but there are still enemies in the range.
	if (attackEnemyUnitsInRangeWhileMoving()) {
		return;
	}
	// If there aren't any enemies, find new path
	newPath();
 }

 if (waypointCount() == 0) {
	// Waypoints were PolicyClean previously but are now PolicyLocal so they
	//  should arrive immediately. If there are no waypoints but advanceMove is
	//  called, then probably there's an error somewhere
	// Also, there must now be point -1; -1 at the end of the path, so something
	//  is wrong here
	boError(401) << k_funcinfo << "unit " << id() << ": No waypoints" << endl;
	newPath();
	if (waypointCount() == 0) {
		// Serious problem somewhere
		boError(401) << k_funcinfo << "unit " << id() << ": Still no waypoints. Aborting" << endl;
		stopMoving();
		setMovingStatus(Standing);
		return;
	}
 }

 boDebug(401) << k_funcinfo << "unit " << id() << endl;
 if (advanceWork() != work()) {
	if (work() == WorkAttack && target()) {
		// no need to move to the position of the unit...
		// just check if unit is in range now.
		int range;
		if (target()->isFlying()) {
			range = unitProperties()->maxAirWeaponRange();
		} else {
			range = unitProperties()->maxLandWeaponRange();
		}
		if (inRange(range, target())) {
			boDebug(401) << k_funcinfo << "unit " << id() << ": target is in range now" << endl;
			setMovingStatus(Standing);
			stopMoving();
			return;
		}
		// TODO: make sure that target() hasn't moved!
		// if it has moved also adjust waypoints
	} else if (work() == WorkAttack && !target()) {
		boError() << k_funcinfo << id() << " is in WorkAttack, but has NULL target!" << endl;
		stopAttacking();
		return;
	}
 } else if (moveAttacking()) {
	// Attack any enemy units in range
	// Don't check for enemies every time (if we don't have a target) because it
	//  slows things down
	if (target()) {
		if (attackEnemyUnitsInRangeWhileMoving()) {
			return;
		}
	}
 }

 // Get current waypoint and check if it's valid
 QPoint wp = currentWaypoint();
 if (checkWaypoint(wp)) {
	return;
 }

 // x and y are center of the unit here
 int x = (int)(BosonItem::x() + width() / 2);
 int y = (int)(BosonItem::y() + height() / 2);
 /*boDebug(401) << k_funcinfo << "pos: (" << x << "; " << y << ")" <<
    "; wp: (" << wp.x() << "; " << wp.y() << "); wpc: " << waypointCount() << endl;*/

 // First check if we're at waypoint
 if ((x == wp.x()) && (y == wp.y())) {
	boDebug(401) << k_funcinfo << "unit " << id() << ": unit is at waypoint" << endl;
	waypointDone();
	d->mWayPointReached = true;
 }

 // This is a bad hack to fix #55926
 if (d->mWayPointReached) {
	// Check for enemy units in range every time waypoint is reached
	if (attackEnemyUnitsInRangeWhileMoving()) {
		return;
	}
	// Path is recalculated every MAX_PATH_AGE waypoints
	d->mPathAge = d->mPathAge + 1;
	if (d->mPathAge >= MAX_PATH_AGE || waypointCount() == 0) {
		boDebug(401) << k_funcinfo << "Searching new path (update)" << endl;
		newPath();
	}

	wp = currentWaypoint();
	if (checkWaypoint(wp)) {
		return;
	}
	d->mWayPointReached = false;
 }

 // If we're close to destination, decelerate, otherwise accelerate
 // TODO: we should also slow down when turning at waypoint.
 if (slowDownAtDestination() && QMAX(QABS(x - destinationX()), QABS(y - destinationY())) <= decelerationDistance()) {
/*	boDebug(401) << "MOVING: " << "decelerating; pos: (" << x << ", " << y << "); dest: (" <<
			destinationX() << ", " << destinationY() << "); dist: " <<
			QMAX(QABS(x - destinationX()), QABS(y - destinationY())) << "; decel. dist: " <<
			decelerationDistance() << ";  speed: " << speed() << ";  acc/dec/max speed: " << accelerationSpeed() <<
			"/" << decelerationSpeed() << "/" << maxSpeed() << endl;*/
	decelerate();
 } else {
	accelerate();
 }
 /*boDebug() << k_funcinfo << "Speed is now " << speed() << "; exact pos: (" <<
    BosonItem::x() << "; " << BosonItem::y() << ")" << endl;*/

 // Try to go to same x and y coordinates as waypoint's coordinates
 // First x coordinate
 // Slow down if there is less than speed() pixels to go
 // FIXME: don't slow down if direction doesn't change and moving continues
 float xspeed = 0;
 float yspeed = 0;
 float dist = (int)speed();
 if (dist < 1) {
	// speed() was 0.something. Make dist 1
	dist = 1;
 }
 float xdiff, ydiff;

 xdiff = wp.x() - x;
 ydiff = wp.y() - y;

 if (QABS(xdiff) < dist) {
	xspeed += xdiff;
 } else {
	xspeed += (xdiff > 0) ? dist : -dist;
 }
 // Same with y coordinate
 if (QABS(ydiff) < dist) {
	yspeed += ydiff;
 } else {
	yspeed += (ydiff > 0) ? dist : -dist;
 }


 //boDebug() << k_funcinfo << "Setting velocity to (" << xspeed << "; " << yspeed << ")" << endl;
 // Set velocity for actual moving
 setVelocity(xspeed, yspeed, 0.0);
 setMoving(true);

 // set the new direction according to new speed
 turnTo();
}

void MobileUnit::advanceMoveCheck()
{
 if (!canvas()->onCanvas(boundingRectAdvanced().topLeft())) {
	QPoint point = boundingRectAdvanced().topLeft();
	QString problem;
	if (!canvas()->onCanvas(QPoint(0, point.y()))) {
		problem = QString("top==%1").arg(point.y());
	} else if (!canvas()->onCanvas(QPoint(point.x(), 0))) {
		problem = QString("left==%1").arg(point.x());
	} else {
		boError(401) << k_funcinfo
				<< "internal error: (0," << point.y() <<
				") and (" << point.x()
				<< ",0) are on canvas, but (" << point.x() <<
				"," << point.y() << ") isn't !"
				<< endl;
		problem = "internal";
	}
	boError(401) << k_funcinfo << "unit " << id()
			<< " not on canvas (topLeftAdvanced): (" << point.x()
			<< ";" << point.y() << ")" << " problem was: "
			<< problem << endl;
	boError(401) << k_funcinfo << "leaving unit a current topleft pos: ("
			<< boundingRect().topLeft().x() << ";"
			<< boundingRect().topLeft().y() << ")" << endl;
	setMovingStatus(Standing);
	stopMoving();
	setWork(WorkNone);
	return;
 }
 if (!canvas()->onCanvas(boundingRectAdvanced().bottomRight())) {
	QPoint point = boundingRectAdvanced().bottomRight();
	QString problem;
	if (!canvas()->onCanvas(QPoint(0, point.y()))) {
		problem = QString("bottom==%1").arg(point.y());
	} else if (!canvas()->onCanvas(QPoint(point.x(), 0))) {
		problem = QString("right==%1").arg(point.x());
	} else {
		boError(401) << k_funcinfo
				<< "internal error: (0," << point.y() <<
				") and (" << point.x()
				<< ",0) are on canvas, but (" << point.x() <<
				"," << point.y() << ") isn't !"
				<< endl;
	}
	boError(401) << k_funcinfo << "unit " << id()
			<< " not on canvas (bottomRightAdvanced): ("
			<< point.x() << ";" << point.y() << ")"
			<< "  current rightEdge: " << rightEdge()
			<< " ; current bottomEdge:" << bottomEdge()
			<< " ; xVelo: " << xVelocity()
			<< " ; (int)xVelo: " << (int)xVelocity()
			<< " ; yVelo: " << yVelocity()
			<< " ; (int)yVelo: " << (int)yVelocity()
			<< " problem was: "
			<< problem << endl;
	boError(401) << k_funcinfo << "leaving unit a current bottomright pos: ("
			<< boundingRect().bottomRight().x() << ";"
			<< boundingRect().bottomRight().y() << ")" << endl;
	setMovingStatus(Standing);
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
	setSpeed(0);
	// If we haven't yet recalculated path, consider unit to be moving
	setMoving(d->mPathRecalculated == 0);

	const int recalculate = 50; // recalculate when 50 advanceMove() failed

	if (d->mPathRecalculated >= 2) {
		boDebug(401) << k_funcinfo << "unit: " << id() << ": Path recalculated 3 times and it didn't help, giving up and stopping" << endl;
		// TODO: wait some time (~10 secs) and then try again.
		setMovingStatus(Standing);
		stopMoving();
		return;
	}
	if (d->mMovingFailed >= recalculate) {
		boDebug(401) << "unit " << id() << ": recalculating path" << endl;
		// you must not do anything that changes local variables directly here!
		// all changed of variables with PolicyClean are ok, as they are sent
		// over network and do not take immediate effect.

		newPath();
		d->mPathAge = 0;
		d->mMovingFailed = 0;
		d->mPathRecalculated = d->mPathRecalculated + 1;
	}
	return;
 }
 d->mMovingFailed = 0;
 d->mPathRecalculated = 0;
 //boDebug(401) << k_funcinfo << "unit " << id() << ": done" << endl;
}
#endif // PATHFINDER_TNG

void MobileUnit::turnTo(Direction direction)
{
 if (isDestroyed()) {
	boError() << k_funcinfo << "unit is already destroyed!" << endl;
	return;
 }
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
	// AB: warning - this does a lookup on all items and therefore is slow!
	// --> but we need it as a simple test on the pointer causes trouble if
	// that pointer is already deleted. any nice solutions?
	if (!canvas()->allItems()->contains(target())) {
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

bool MobileUnit::loadFromXML(const QDomElement& root)
{
 if (!Unit::loadFromXML(root)) {
	boError() << k_funcinfo << "Unit not loaded properly" << endl;
	return false;
 }

 bool ok = false;
 float speed = 0.0f;
 if (root.hasAttribute("Speed")) {
	speed = root.attribute("Speed").toFloat(&ok);
	if (!ok) {
		boWarning() << k_funcinfo << "Invalid value for Speed attribute" << endl;
	} else {
		setSpeed(speed);
	}
 }

 return true;
}

bool MobileUnit::saveAsXML(QDomElement& root)
{
 if (!Unit::saveAsXML(root)) {
	boError() << k_funcinfo << "Unit not saved properly" << endl;
	return false;
 }

 root.setAttribute("Speed", speed());

 return true;
}

void MobileUnit::stopMoving()
{
 Unit::stopMoving();
#ifdef PATHFINDER_TNG
 if (pathInfo()->slowDownAtDest) {
	setSpeed(0);
 }
 d->lastxvelo = 0.0f;
 d->lastyvelo = 0.0f;
#else
 // Reset moveCheck variables
 d->mMovingFailed = 0;
 d->mPathRecalculated = 0;
 d->mWayPointReached = false;
 if (slowDownAtDestination()) {
	setSpeed(0);
 }
#endif
}

#ifdef PATHFINDER_TNG
bool MobileUnit::attackEnemyUnitsInRangeWhileMoving()
{
 if (pathInfo()->moveAttacking && attackEnemyUnitsInRange()) {
	boDebug(401) << k_funcinfo << "unit " << id() << ": Enemy units found in range, attacking" << endl;
	setVelocity(0.0, 0.0, 0.0);  // To prevent moving
	d->lastxvelo = 0.0f;
	d->lastyvelo = 0.0f;
	setSpeed(0);
	setMovingStatus(Engaging);
	return true;
 }
 return false;
}
#else
bool MobileUnit::attackEnemyUnitsInRangeWhileMoving()
{
 if (moveAttacking() && attackEnemyUnitsInRange()) {
	boDebug(401) << k_funcinfo << "unit " << id() << ": Enemy units found in range, attacking" << endl;
	setVelocity(0.0, 0.0, 0.0);  // To prevent moving
	setSpeed(0);
	setMoving(false);
	return true;
 }
 return false;
}
#endif


void MobileUnit::newPath()
{
#ifdef PATHFINDER_TNG
 // FIXME: remove this, Unit::newPath() seems to be enough
 Unit::newPath();
#else
 Unit::newPath();
 d->mPathAge = 0;
 setSearchPath(0);
#endif
}

bool MobileUnit::checkPathPoint(const QPoint& p)
{
#ifdef PATHFINDER_TNG
 boDebug(401) << k_funcinfo << "p: (" << p.x() << "; " << p.y() << ")" << endl;
 // Check for special codes in path point
 if (p.x() != p.y()) {
	// All special points have x == y, so this is not one of them
	return false;
 }

 // Check for code. We only check x, because y == x anyway
 if (p.x() == PF_TNG_END_CODE) {
	boDebug() << k_funcinfo << id() << ": found PF_TNG_END_CODE code" << endl;
	// Remove this 'coded' waypoint
	pathPointDone();
	// Path has ended. Check if we have more waypoints
	waypointDone();
	if (waypointCount() == 0) {
		// No more waypoints. Unit should stop moving
		setMovingStatus(Standing);
		stopMoving();
		if (work() == WorkNone) {
			// If we were just moving, turn to random direction
			boDebug() << k_funcinfo << id() << ": stopping moving. Turning" << endl;
			// Turn a bit
			int turn = (int)rotation() + (owner()->game()->random()->getLong(60) - 30);
			// Check for overflows
			if (turn < 0) {
				turn += 360;
			} else if (turn >= 360) {
				turn -= 360;
			}
			Unit::turnTo(turn);
			setWork(WorkTurn);
			return true;
		}
	} else {
		// There are more waypoints. Take the next one
		//QPoint wp = currentWaypoint();
		boWarning() << k_funcinfo << "Waypoints aren't supported yet!" << endl;
		// Stop for now (until we have working implementation)
		setMovingStatus(Standing);
		stopMoving();
		return true;
	}
 } else if (p.x() == PF_TNG_NEXT_REGION) {
	boDebug() << k_funcinfo << id() << ": found PF_TNG_NEXT_REGION code" << endl;
	// Remove this 'coded' waypoint
	pathPointDone();
	// Path has reached next region
	// Increase hlstep (high-level path step), i.e. go to next region
	pathInfo()->hlstep++;
	// We have to find new low-level path
	newPath();
	// Now we need to check for special points again (in case path couldn't be
	//  found now, e.g. it destination could have been fogged before, but is
	//  unfogged now). I know, recursive isn't good, but unless something is
	//  broken, it should recurse only once
	return checkPathPoint(currentPathPoint());
 } else if (p.x() == PF_TNG_CANNOT_GO) {
	boDebug() << k_funcinfo << id() << ": found PF_TNG_CANNOT_GO code" << endl;
	// Remove this 'coded' waypoint
	pathPointDone();
	// Unit can't go to destination. Stop moving.
	// FIXME: is this a good place to do this? Maybe it should be done in
	//  newPath() instead?
	setMovingStatus(Standing);
	stopMoving();
	return true;
 }

 // No special code was found (this is ok, e.g. if point was (5; 5) or so)
#endif // PATHFINDER_TNG
 return false;
}

bool MobileUnit::checkWaypoint(const QPoint& wp)
{
#ifndef PATHFINDER_TNG
 // If both waypoint's coordinates are -1, then it means that either path to
 //  destination can't be found or that we're already at the goal point. Either
 //  way, we stop moving
 if ((wp.x() == -1) && (wp.y() == -1)) {
	setMovingStatus(Standing);
	stopMoving();
	if (work() == WorkNone) {
		boDebug() << k_funcinfo << id() << ": stopping moving. Turning" << endl;
		// Turn a bit
		int turn = (int)rotation() + (owner()->game()->random()->getLong(90) - 45);
		// Check for overflows
		if (turn < 0) {
			turn += 360;
		} else if (turn > 360) {
			turn -= 360;
		}
		Unit::turnTo(turn);
		setMovingStatus(Standing);
		setWork(WorkTurn);
	}
	return true;
 }
#endif // PATHFINDER_TNG
 return false;
}


/////////////////////////////////////////////////
// Facility
/////////////////////////////////////////////////

class Facility::FacilityPrivate
{
public:
	KGameProperty<unsigned int> mConstructionStep;
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
	setParticleSystems(unitProperties()->newConstructedParticleSystems(x() + width() / 2, y() + height() / 2, z()));
 }
}

unsigned int Facility::currentConstructionStep() const
{
 return d->mConstructionStep;
}

bool Facility::loadFromXML(const QDomElement& root)
{
 if (!Unit::loadFromXML(root)) {
	boError() << k_funcinfo << "Unit not loaded properly" << endl;
	return false;
 }

 if (d->mConstructionStep > constructionSteps()) {
	d->mConstructionStep = constructionSteps();
 }
 unsigned int modelStep = 0;
 if (d->mConstructionStep == constructionSteps()) {
	modelStep = model()->constructionSteps(); // completed construction
 } else {
	modelStep = model()->constructionSteps() * d->mConstructionStep / constructionSteps();
 }
 setGLConstructionStep(modelStep);

 // FIXME: remove. this is from Facility::setConstructionStep. we _need_ to load
 // particles in loadFromXML() - then these lines are obsolete.
 if (d->mConstructionStep == constructionSteps()) {
	setParticleSystems(unitProperties()->newConstructedParticleSystems(x() + width() / 2, y() + height() / 2, z()));
 }

 return true;
}

bool Facility::saveAsXML(QDomElement& root)
{
 if (!Unit::saveAsXML(root)) {
	boError() << k_funcinfo << "Unit not loaded properly" << endl;
	return false;
 }
 return true;
}

