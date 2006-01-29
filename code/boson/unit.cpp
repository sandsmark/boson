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

#include "unit.h"

#include "../bomemory/bodummymemory.h"
#include "player.h"
#include "playerio.h"
#include "bosoncanvas.h"
#include "cell.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "bosonpath.h"
#include "bosonstatistics.h"
#include "unitplugins.h"
#include "boitemlist.h"
#include "pluginproperties.h"
#include "boson.h"
#include "bosonweapon.h"
#include "bopointeriterator.h"
#include "bodebug.h"
#include "bo3dtools.h"
#include "bosonprofiling.h"
#include "items/bosonitemrenderer.h"

#include <kgame/kgamepropertylist.h>
#include <kgame/kgame.h>
#include <krandomsequence.h>

#include <qptrlist.h>
#include <qdom.h>

#include <math.h>

#include "defines.h"


#define MAX_PATH_AGE 5

// If defined, new (not cell based) collision detection method is used
// WARNING: new collision detection is buggy and doesn't work really well (with
//  current pathfinder(s))
//#define USE_NEW_COLLISION_DETECTION

// If defined, units will search and fire at enemy units only when they are at
//  waypoints. This may make unit movement better for big groups, because units
//  shouldn't occupy multiple cells while firing at enemies.
//#define CHECK_ENEMIES_ONLY_AT_WAYPOINT



bool Unit::mInitialized = false;

class UnitPrivate
{
public:
	UnitPrivate()
	{
		mTarget = 0;

		mWeapons = 0;

		mMoveData = 0;
	}
	KGamePropertyList<BoVector2Fixed> mWaypoints;
	KGamePropertyList<BoVector2Fixed> mPathPoints;
	KGameProperty<int> mWantedRotation;

	// be *very* careful with those - NewGameDialog uses Unit::save() which
	// saves all KGameProperty objects. If non-KGameProperty properties are
	// changed before all players entered the game we'll have a broken
	// network game.
	Unit* mTarget;

	// these must NOT be touched (items added or removed) after the c'tor.
	// loading code will depend in this list to be at the c'tor state!
	QPtrList<UnitPlugin> mPlugins;
	BosonWeapon** mWeapons;

	BosonMoveData* mMoveData;
	BosonPathInfo mPathInfo;

	unsigned long int mMaxWeaponRange;
	unsigned long int mMaxLandWeaponRange;
	unsigned long int mMaxAirWeaponRange;
};

Unit::Unit(const UnitProperties* prop, Player* owner, BosonCanvas* canvas)
		: UnitBase(prop, owner, canvas)
{
 if (!mInitialized) {
	initStatic();
 }
 d = new UnitPrivate;
 d->mMaxWeaponRange = 0;
 d->mMaxAirWeaponRange = 0;
 d->mMaxLandWeaponRange = 0;
 mCurrentPlugin = 0;
 mAdvanceFunction = &Unit::advanceIdle;
 mAdvanceFunction2 = &Unit::advanceIdle;
 d->mPlugins.setAutoDelete(true);
 d->mPlugins.clear();

 // note: these width and height can be used for e.g. pathfinding. It does not
 // depend in any way on the .3ds file or another OpenGL thing.
 setSize(prop->unitWidth(), prop->unitHeight(), prop->unitDepth());

 registerData(&d->mWaypoints, IdWaypoints);
 registerData(&d->mPathPoints, IdPathPoints);
 registerData(&d->mWantedRotation, IdWantedRotation);
 d->mWantedRotation.setLocal(0);
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
 if (canvas()->pathFinder()) {
	// Release highlevel path here once we cache them
 }
 delete d;
}

bool Unit::init()
{
 bool ret = BosonItem::init();
 if (!ret) {
	return ret;
 }
 if (!speciesTheme()) {
	BO_NULL_ERROR(speciesTheme());
	return false;
 }
 const UnitProperties* prop = unitProperties();
 if (!prop) {
	BO_NULL_ERROR(prop);
	return false;
 }
 if (!owner()) {
	BO_NULL_ERROR(owner());
	return false;
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
	d->mPlugins.append(new RefineryPlugin(this));
 }
 if (prop->properties(PluginProperties::ResourceMine)) {
	d->mPlugins.append(new ResourceMinePlugin(this));
 }
 if (prop->properties(PluginProperties::AmmunitionStorage)) {
	d->mPlugins.append(new AmmunitionStoragePlugin(this));
 }
 if (prop->properties(PluginProperties::Radar)) {
	d->mPlugins.append(new RadarPlugin(this));
 }

 loadWeapons();

 speciesTheme()->loadNewUnit(this);

 return true;
}

void Unit::initStatic()
{
 // we initialize the properties for Unit, MobileUnit, Facility and the plugins
 // here
 // Unit
 addPropertyId(IdWaypoints, QString::fromLatin1("Waypoints"));
 addPropertyId(IdPathPoints, QString::fromLatin1("PathPoints"));
 addPropertyId(IdWantedRotation, QString::fromLatin1("WantedRotation"));

 // MobileUnit

 // Facility
 addPropertyId(IdConstructionStep, QString::fromLatin1("ConstructionStep"));

 // UnitPlugin and derived classes
 addPropertyId(IdProductionState, QString::fromLatin1("ProductionState"));
 addPropertyId(IdResourcesMined, QString::fromLatin1("ResourcesMined"));
 addPropertyId(IdResourcesX, QString::fromLatin1("ResourcesX"));
 addPropertyId(IdResourcesY, QString::fromLatin1("ResourcesY"));
 addPropertyId(IdHarvestingType, QString::fromLatin1("HarvestingType"));
 addPropertyId(IdBombingTargetX, QString::fromLatin1("IdBombingTargetX"));
 addPropertyId(IdBombingTargetY, QString::fromLatin1("IdBombingTargetY"));
 addPropertyId(IdMinePlacingCounter, QString::fromLatin1("MinePlacingCounter"));
 addPropertyId(IdResourceMineMinerals, QString::fromLatin1("ResourceMineMinerals"));
 addPropertyId(IdResourceMineOil, QString::fromLatin1("ResourceMineOil"));
 addPropertyId(IdBombingDropDist, QString::fromLatin1("IdBombingDropDist"));
 addPropertyId(IdBombingLastDistFromDropPoint, QString::fromLatin1("IdBombingLastDistFromDropPoint"));
 addPropertyId(IdMineralsPaid, QString::fromLatin1("IdMineralsPaid"));
 addPropertyId(IdOilPaid, QString::fromLatin1("IdOilPaid"));

 mInitialized = true;
}

void Unit::setMoveData(BosonMoveData* data)
{
 d->mMoveData = data;
}

BosonMoveData* Unit::moveData() const
{
 return d->mMoveData;
}

void Unit::select(bool markAsLeader)
{
 if (isDestroyed()) {
	boDebug() << k_funcinfo << id() << " is destroyed" << endl;
	return; // shall we really return?
 }
 BosonItem::select(markAsLeader);
}

bofixed Unit::destinationX() const
{
 return pathInfo()->dest.x();
}

bofixed Unit::destinationY() const
{
 return pathInfo()->dest.y();
}

int Unit::moveRange() const
{
 return pathInfo()->range;
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
 if (h > maxHealth()) {
	h = maxHealth();
 }
 if (maxHealth() == 0) {
	boError() << "Ooop - maxHealth == 0" << endl;
	return;
 }
 UnitBase::setHealth(h);
 if (isDestroyed()) {
	unselect();
	updateAnimationMode();
 }
}

void Unit::setSightRange(unsigned long int r)
{
 UnitBase::setSightRange(r);
 if (isDestroyed()) {
	return;
 }
 if (canvas()) {
	canvas()->updateSight(this, x(), y());
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


void Unit::moveBy(bofixed moveX, bofixed moveY, bofixed moveZ)
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

 bofixed oldX = x();
 bofixed oldY = y();

 if (!isFlying()) {
	bofixed rotateX = 0.0f;
	bofixed rotateY = 0.0f;
	updateZ(moveX, moveY, &moveZ, &rotateX, &rotateY);
	setXRotation(rotateX);
	setYRotation(rotateY);
 }

 BosonItem::moveBy(moveX, moveY, moveZ);
 canvas()->unitMoved(this, oldX, oldY);
}

void Unit::updateZ(bofixed moveByX, bofixed moveByY, bofixed* moveByZ, bofixed* rotateX, bofixed* rotateY)
{
 // Center point of the unit.
 bofixed centerx = x() + moveByX + width() / 2;
 bofixed centery = y() + moveByY + height() / 2;

 // Calculate unit's rotation (depends on ground height).
 // These are offsets (from center point) to front and right side of the unit.
 bofixed frontx = 0.0f, fronty = 0.0f, sidex = 0.0f, sidey = 0.0f;
 // Calculate front offset
 Bo3dTools::pointByRotation(&frontx, &fronty, rotation(), height() / 2);
 // Calculate right side offset
 bofixed myrot = rotation() + 90;
 if (myrot > 360) {
	myrot -= 360;
 }
 Bo3dTools::pointByRotation(&sidex, &sidey, myrot, width() / 2);

 // Find necessary height values.
 bofixed rearz, frontz, rightz, leftz, newZ;
 // For flying units and ships, we take water level into surface; for land
 //  units, we don't.
 if (unitProperties()->isAircraft() || unitProperties()->canGoOnWater()) {
	newZ = canvas()->heightAtPoint(centerx, centery);
	frontz = canvas()->heightAtPoint(centerx + frontx, centery + fronty);
	rearz = canvas()->heightAtPoint(centerx - frontx, centery - fronty);
	rightz = canvas()->heightAtPoint(centerx + sidex, centery + sidey);
	leftz = canvas()->heightAtPoint(centerx - sidex, centery - sidey);
	if (!unitProperties()->isAircraft()) {
		// We want ships to be a bit inside the water.
		newZ -= 0.05;
		// rearz, frontz, rightz and leftz are only used for rotation calculation,
		//  so there's no need to change them.
	}
 } else {
   // Land unit
	newZ = canvas()->terrainHeightAtPoint(centerx, centery);
	frontz = canvas()->terrainHeightAtPoint(centerx + frontx, centery + fronty);
	rearz = canvas()->terrainHeightAtPoint(centerx - frontx, centery - fronty);
	rightz = canvas()->terrainHeightAtPoint(centerx + sidex, centery + sidey);
	leftz = canvas()->terrainHeightAtPoint(centerx - sidex, centery - sidey);
 }

 // Calculate rotations
 // Calculate angle from frontz to rearz
 bofixed xrot = Bo3dTools::rad2deg(atan(QABS(frontz - rearz) / height()));
 *rotateX = (frontz >= rearz) ? xrot : -xrot;

 // Calculate y rotation
 // Calculate angle from leftz to rightz
 bofixed yrot = Bo3dTools::rad2deg(atan(QABS(rightz - leftz) / width()));
 *rotateY = (leftz >= rightz) ? yrot : -yrot;


 if (isFlying()) {
	newZ += 2.0f;  // Flying units are always 2 units above the ground
 }
 *moveByZ = newZ - z();
}

void Unit::reload(unsigned int count)
{
 if (isDestroyed()) {
	return;
 }
 chargePowerForReload(owner()->powerChargeForCurrentAdvanceCall());
 bool unitConsumesPower = false;
 if (powerConsumedByUnit() > 0) {
	unitConsumesPower = true;
 }
 bool isCharged = isChargedForReload();
 if (d->mWeapons[0]) {
	BosonWeapon** w = &d->mWeapons[0];
	for (; *w; w++) {
		// TODO
#if 0
		bool weaponRequiresPower = true;
		if ((*w)->powerConsumedByWeapon == 0) {
			weaponRequiresPower = false;
		}
#else
		bool weaponRequiresPower = unitConsumesPower;
#endif
		if (isCharged || !weaponRequiresPower) {
			(*w)->reload(this, count);
		}
	}
 }

 // shield reloading always requires power (unless the unit doesn't consume
 // power at all)
 bool shieldsRequirePower = true;
 if (!unitConsumesPower) {
	shieldsRequirePower = false;
 }
 if (shields() < maxShields()) {
	if (isCharged || !shieldsRequirePower) {
		reloadShields(count);
	}
 }

 unchargePowerForReload();
}

unsigned long int Unit::requestAmmunition(const QString& type, unsigned long int requested)
{
 if (!owner()) {
	BO_NULL_ERROR(owner);
	return 0;
 }

 // AB: there are 3 places where ammunition could be takes from
 // 1. the global "pool" of the player. this ammunition "just exists" and is
 //    bound to a player only (not to e.g. a facility)
 // 2. the globally accessible ammunition that is stored in some storage unit
 //    (usually a facility that produces the ammo). it is destroyed when that
 //    storage unit is destroyed, but can be accessed globally, without the need
 //    to pick it up.
 // 3. ammunition that needs to be picked up at a unit.

 // this searches for ammo of type 1 and 2
 unsigned long int ammo = owner()->requestAmmunition(type, requested);
 if (ammo > requested) {
	boError(610) << k_funcinfo << "received more ammo than requested" << endl;
	ammo = requested;
 }
 if (ammo == requested) {
	return ammo;
 }

 boDebug(610) << k_funcinfo << "searching for unit to pick ammo up from" << endl;
 for (QPtrListIterator<Unit> it(*owner()->allUnits()); it.current(); ++it) {
	if (it.current()->isDestroyed()) {
		continue;
	}
	Unit* unit = it.current();
	AmmunitionStoragePlugin* storage = (AmmunitionStoragePlugin*)unit->plugin(UnitPlugin::AmmunitionStorage);
	if (!storage) {
		continue;
	}
	if (storage->ammunitionStored(type) == 0) {
		continue;
	}

	bool denied = false;
	ammo += storage->pickupAmmunition(this, type, requested - ammo, &denied);
	if (ammo > requested) {
		boError(610) << k_funcinfo << "received more ammo than requested" << endl;
		ammo = requested;
	}
	if (ammo == requested) {
		return ammo;
	}
	if (denied) {
		boDebug(610) << k_funcinfo << id() << ": picking up from unit " << storage->unit()->id() << " denied" << endl;
	}
 }


 return ammo;
}

void Unit::advanceNone(unsigned int)
{
 // do NOT do anything here!
 // usually it won't be called anyway.
}

void Unit::advanceIdle(unsigned int advanceCallsCount)
{
// this is called when the unit has nothing specific to do. Usually we just want
// to fire at every enemy in range.

 if (!target()) {
	if (advanceCallsCount % 40 != 10) {
		return;
	}
 } else if (advanceCallsCount % 10 != 0) {
	return;
 }
 BosonProfiler profiler("advanceIdle");

 if ((!unitProperties()->canShoot() ||!d->mWeapons[0]) && (!unitProperties()->isAircraft())) {
	// this unit does not have any weapons, so it will never shoot anyway.
	// no need to call advanceIdle() again
	setAdvanceWork(WorkNone);
	return;
 }

 attackEnemyUnitsInRange();
}

bool Unit::attackEnemyUnitsInRange()
{
 PROFILE_METHOD
 if (!unitProperties()->canShoot()) {
	return false;
 }
 if (!d->mWeapons[0]) {
	return false;
 }

 // TODO: Note that this is not completely realistic nor good: it may be good to
 //  e.g. not waste some weapon with very big damage and reload values for very
 //  weak unit. So there room left for improving :-)
 bool targetfound = false;
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
			!inRange(maxWeaponRange(), target())) {
		d->mTarget = bestEnemyUnitInRange();
		if (!target()) {
			return false;
		}
	}
	targetfound = true;

	// If unit is mobile, rotate to face the target if it isn't facing it yet
	if (isMobile() && !isFlying()) {
		bofixed rot = Bo3dTools::rotationToPoint(target()->x() - x(), target()->y() - y());
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
	if (w->canShootAt(target()) && inRange(w->range(), target())) {
		shootAt(w, target());
		if (target()->isDestroyed()) {
			d->mTarget = 0;
		}
	}
 }

 // It might be that none of the unit's weapons is reloaded, but target has been
 //  found. So we check for this now.
 targetfound = targetfound || (target());

 return targetfound;
}

Unit* Unit::bestEnemyUnitInRange()
{
 PROFILE_METHOD
 // Return if unit can't shoot
 if (!unitProperties()->canShoot()) {
	return 0;
 }
 // Return if no enemies in range
 BoItemList* list = enemyUnitsInRange(maxWeaponRange());
 if (!list->count() > 0) {
	return 0;
 }

 // Initialize some variables
 Unit* best = 0;
 BoItemList::Iterator it = list->begin();
 Unit* u = 0;
 bofixed dist = 0;
 // Candidates to best unit, see below
 Unit* c1 = 0;
 Unit* c2 = 0;
 Unit* c3 = 0;

 // Iterate through the list of enemies and pick the best ones
 for (; it != list->end(); ++it) {
	u = ((Unit*)*it);
	dist = QMAX(QABS((int)(u->x() - x())), QABS((int)(u->y() - y())));
	// Quick check if we can shoot at u
	if (u->isFlying()) {
		if (!unitProperties()->canShootAtAirUnits()) {
			continue;
		}
		if (dist > maxAirWeaponRange()) {
			continue;
		}
	} else {
		if (!unitProperties()->canShootAtLandUnits()) {
			continue;
		}
		if (dist > maxLandWeaponRange()) {
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

void Unit::advanceAttack(unsigned int advanceCallsCount)
{
 if (advanceCallsCount % 5 != 0) {
	if (isFlying()) {
		flyInCircle();
	}
	return;
 }
 BosonProfiler profiler("advanceAttack");

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
	range = maxAirWeaponRange();
 } else {
	range = maxLandWeaponRange();
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
	if (range >= 1) {
		range--;
	}
	if (!moveTo(target()->x(), target()->y(), range)) {
		setWork(WorkIdle);
	} else {
		addWaypoint(BoVector2Fixed(target()->x(), target()->y()));
		setAdvanceWork(WorkMove);
	}
	return;
 }

 if (isMobile() && !isFlying()) {
	bofixed rot = Bo3dTools::rotationToPoint(target()->x() - x(), target()->y() - y());
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
	if (w->properties()->autoUse() && w->reloaded() && w->canShootAt(target()) && inRange(w->range(), target())) {
		shootAt(w, target());
		if (target()->isDestroyed()) {
			boDebug(300) << "    " << k_funcinfo << "target destroyed, returning" << endl;
			stopAttacking();
			flyInCircle();
			return;
		}
	}
 }
 boDebug(300) << "    " << k_funcinfo << "done shooting" << endl;
 // TODO: fly on straight, pass the target, fly a bit more, then turn and fly back toward the target
 flyInCircle();
}

void Unit::advanceDestroyed(unsigned int advanceCallsCount)
{
 // note: the unit/wreckage will get deleted pretty soon
 if (advanceCallsCount % 10 != 0) {
	return;
 }
 BosonProfiler profiler("advanceDestroyed");
 if (isVisible()) {
	// Make unit slowly sink into ground
#define MAXIMAL_ADVANCE_COUNT 19
	setVelocity(0, 0, -(depth() / (REMOVE_WRECKAGES_TIME * MAXIMAL_ADVANCE_COUNT)) * 1.2);
#undef MAXIMAL_ADVANCE_COUNT
 }
}

void Unit::advancePlugin(unsigned int advanceCallsCount)
{
 BosonProfiler profiler("advancePlugin");
 if (!currentPlugin()) {
	boWarning() << k_funcinfo << "NULL plugin!" << endl;
	setWork(WorkIdle);
 } else {
	currentPlugin()->advance(advanceCallsCount);
 }
}

void Unit::advanceTurn(unsigned int)
{
 BosonProfiler profiler("advanceTurn");
 // Unit is still while turning
 setVelocity(0, 0, 0);

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

 setRotation(bofixed(dir));
 updateRotation();

 if (d->mWantedRotation == dir) {
	/**
	 * AFAICS there are 3 possibilities:
	 * 1. The user explicitly wanted the unit to turn.
	 *    -> we are done now, set work to WorkIdle
	 *       (work also includes advanceWork!)
	 * 2. The user wanted to do something else (move, shoot, ...) and for
	 *    this the unit had to turn.
	 *    -> continue with original work, by setting advanceWork to work()
	 * 3. The user wanted to do something else (just like 2.) and for that
	 *    the unit had to _move_ which required the unit to turn.
	 *    -> continue moving, i.e. set advanceWork to WorkMove
	 **/
	if (work() == WorkTurn) {
		setWork(WorkIdle);
	} else if (advanceWork() != work()) {
		if (pathPointCount() != 0 || waypointCount() != 0) {
			// this is probably case 3 (but case 2 is possible, too)
			setAdvanceWork(WorkMove);
		} else {
			setAdvanceWork(work());
		}
	}
 }
}

void Unit::addWaypoint(const BoVector2Fixed& pos)
{
 d->mWaypoints.append(pos);
}

void Unit::waypointDone()
{
 if (d->mWaypoints.count() == 0) {
	boError() << k_funcinfo << id() << ": no waypoints" << endl;
	return;
 }
 d->mWaypoints.remove(d->mWaypoints.at(0));
}

const QValueList<BoVector2Fixed>& Unit::waypointList() const
{
 return d->mWaypoints;
}

unsigned int Unit::waypointCount() const
{
 return d->mWaypoints.count();
}

void Unit::resetPathInfo()
{
 // Release highlevel path here once we cache them
 pathInfo()->reset();
 pathInfo()->unit = this;
 pathInfo()->flying = unitProperties()->isAircraft();
}


void Unit::moveTo(const BoVector2Fixed& pos, bool attack)
{
 PROFILE_METHOD
 d->mTarget = 0;

 // We want land unit's center point to be in the middle of the cell after
 //  moving.
 bofixed add = 0;
 if (!unitProperties()->isAircraft()) {
	BO_CHECK_NULL_RET(moveData());
	add = (((moveData()->size % 2) == 1) ? 0.5 : 0);
 }
 bofixed x = (int)pos.x() + add;
 bofixed y = (int)pos.y() + add;

 if (moveTo(x, y, -1)) {
	boDebug() << k_funcinfo << "unit " << id() << ": Will move to (" << x << "; " << y << ")" << endl;
	pathInfo()->moveAttacking = attack;
	pathInfo()->slowDownAtDest = true;
	setWork(WorkMove);
 } else {
	boDebug() << k_funcinfo << "unit " << id() << ": CANNOT move to (" << x << "; " << y << ")" << endl;
	setWork(WorkIdle);
	stopMoving();
 }
}

bool Unit::moveTo(BosonItem* target, int range)
{
 // TODO: try to merge this with the moveTo() method below
 if (maxSpeed() == 0) {
	// If unit's max speed is 0, it cannot move
	return false;
 }

 if (!target) {
	boError() << k_funcinfo << "NULL target" << endl;
	return false;
 }

 bofixed x = target->centerX();
 bofixed y = target->centerY();
 if (unitProperties()->isAircraft()) {
	// Aircrafts cannot go near the border of the map to make sure they have
	//  enough room for turning around
	x = QMIN(QMAX(x, bofixed(6)), (bofixed)canvas()->mapWidth() - 6);
	y = QMIN(QMAX(y, bofixed(6)), (bofixed)canvas()->mapHeight() - 6);
 }


 // Update path info
 resetPathInfo();
 pathInfo()->target = target;
 pathInfo()->range = range;
 boDebug() << k_funcinfo << "unit " << id() << ": target: (" << target->id() << "); range: " << range << endl;

 // Remove old way/pathpoints
 clearWaypoints();
 clearPathPoints();


 // Path is not searched here (it would break pathfinding for groups). Instead,
 //  moving status is set to MustSearch and in MobileUnit::advanceMove(), path
 //  is searched for.
 setMovingStatus(MustSearch);

 addWaypoint(BoVector2Fixed(x, y));
 pathInfo()->slowDownAtDest = true;

 return true;
}

bool Unit::moveTo(bofixed x, bofixed y, int range)
{
 if (maxSpeed() == 0) {
	// If unit's max speed is 0, it cannot move
	return false;
 }

 if (unitProperties()->isAircraft()) {
	// Aircrafts cannot go near the border of the map to make sure they have
	//  enough room for turning around
	x = QMIN(QMAX(x, bofixed(6)), (bofixed)canvas()->mapWidth() - 6);
	y = QMIN(QMAX(y, bofixed(6)), (bofixed)canvas()->mapHeight() - 6);
 }

 // TODO: move the destination point a bit in case the unit partially goes off
 // the map otherwise
 // Find destination cell
 int cellX = (int)x;
 int cellY = (int)y;
 Cell* cell = canvas()->cell(cellX, cellY);
 if (!cell) {
	boError() << k_funcinfo << "unit " << id() << ": Cell (" << cellX << "; " << cellY << ") at (" <<
			x << "; " << y << ") is not valid!" << endl;
	return false;
 }

 // Update path info
 resetPathInfo();
 pathInfo()->dest.setX(x);
 pathInfo()->dest.setY(y);
 pathInfo()->range = range;
 boDebug() << k_funcinfo << "unit " << id() << ": dest: (" << x << "; " << y << "); range: " << range << endl;

 // Remove old way/pathpoints
 // TODO: maybe call stopMoving() instead and remove setMovingStatus(Standing)
 //  from there...
 clearWaypoints();
 clearPathPoints();

 addWaypoint(BoVector2Fixed(x, y));

 // Path is not searched here (it would break pathfinding for groups). Instead,
 //  moving status is set to MustSearch and in MobileUnit::advanceMove(), path
 //  is searched for.
 setMovingStatus(MustSearch);

 return true;
}

void Unit::clearWaypoints()
{
 d->mWaypoints.clear();
}

const BoVector2Fixed& Unit::currentWaypoint() const
{
 return d->mWaypoints[0];
}

void Unit::addPathPoint(const BoVector2Fixed& pos)
{
 d->mPathPoints.append(pos);
}

unsigned int Unit::pathPointCount() const
{
 return d->mPathPoints.count();
}

const BoVector2Fixed& Unit::currentPathPoint()
{
 return d->mPathPoints.first();
}

void Unit::clearPathPoints()
{
 d->mPathPoints.clear();
}

void Unit::pathPointDone()
{
 if (d->mPathPoints.count() == 0) {
	boError() << k_funcinfo << "no pathpoints" << endl;
	return;
 }
 d->mPathPoints.pop_front();
}

const QValueList<BoVector2Fixed>& Unit::pathPointList() const
{
 return d->mPathPoints;
}

void Unit::stopMoving()
{
// boDebug() << k_funcinfo << endl;
 clearWaypoints();
 clearPathPoints();

 // Release highlevel path here once we cache them

 // Call this only if we are only moving - stopMoving() is also called e.g. on
 // WorkAttack, when the unit is not yet in range.
 if (work() == WorkMove) {
	setWork(WorkIdle);
 } else if (advanceWork() != work()) {
	setAdvanceWork(work());
 }
 if (!isFlying()) {
	setMovingStatus(Standing);
	setVelocity(0.0, 0.0, 0.0);
 }
}

void Unit::stopAttacking()
{
 stopMoving(); // FIXME not really intuitive... nevertheless its currently useful.
 setMovingStatus(Standing);
 setTarget(0);
 setWork(WorkIdle);
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

 QDomDocument doc = root.ownerDocument();

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

 // Save pathinfo
 QDomElement pathinfoxml = doc.createElement(QString::fromLatin1("PathInfo"));
 root.appendChild(pathinfoxml);
 // Save start/dest points and range
 saveVector2AsXML(pathInfo()->start, pathinfoxml, "start");
 saveVector2AsXML(pathInfo()->dest, pathinfoxml, "dest");
 pathinfoxml.setAttribute(QString::fromLatin1("target"), pathInfo()->target ? pathInfo()->target->id() : 0);
 pathinfoxml.setAttribute("range", pathInfo()->range);
 // Save last pf query result
 pathinfoxml.setAttribute("result", pathInfo()->result);
 // Save llpath
 pathinfoxml.setAttribute("llpathlength", pathInfo()->llpath.count());
 for (unsigned int i = 0; i < pathInfo()->llpath.count(); i++) {
	saveVector2AsXML(pathInfo()->llpath[i], pathinfoxml, QString("llpath-%1").arg(i));
 }
 // hlpath doesn't have to be saved atm
 // Save misc stuff
 pathinfoxml.setAttribute("moveAttacking", pathInfo()->moveAttacking ? 1 : 0);
 pathinfoxml.setAttribute("slowDownAtDest", pathInfo()->slowDownAtDest ? 1 : 0);
 pathinfoxml.setAttribute("waiting", pathInfo()->waiting);
 pathinfoxml.setAttribute("pathrecalced", pathInfo()->pathrecalced);


 return true;
}

bool Unit::loadFromXML(const QDomElement& root)
{
 if (!UnitBase::loadFromXML(root)) {
	boError(260) << k_funcinfo << "Unit not loaded properly" << endl;
	return false;
 }
 if (health() > maxHealth()) {
	boError(260) << k_funcinfo << "Unit with Id " << id() << " (Type=" << type() << ") wants health=" << health() << " but only " << maxHealth() << " is possible for that type according to index.unit file. decreasing health to maximum." << endl;
	setHealth(maxHealth());
 }
 bool ok = false;
 bofixed rotation = root.attribute(QString::fromLatin1("Rotation")).toFloat(&ok);
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


 // Load pathinfo
 pathInfo()->reset();
 QDomElement pathinfoxml = root.namedItem("PathInfo").toElement();
 if (!pathinfoxml.isNull()) {
	loadVector2FromXML(&pathInfo()->start, pathinfoxml, "start");
	loadVector2FromXML(&pathInfo()->dest, pathinfoxml, "dest");
	pathInfo()->range = pathinfoxml.attribute("range").toInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "Error loading range attribute ('" << pathinfoxml.attribute("range") << "')" << endl;
		return false;
	}
	if (root.hasAttribute(QString::fromLatin1("result"))) {
		pathInfo()->result = (BosonPath::Result)pathinfoxml.attribute("result").toInt(&ok);
		if (!ok) {
			boError() << k_funcinfo << "Error loading result attribute ('" << pathinfoxml.attribute("result") << "')" << endl;
			return false;
		}
	}
	// target
	unsigned int pathinfoTargetId = 0;
	if (root.hasAttribute(QString::fromLatin1("target"))) {
		pathinfoTargetId = root.attribute(QString::fromLatin1("target")).toUInt(&ok);
		if (!ok) {
			boError() << k_funcinfo << "Error loading pathinfo target attribute ('" <<
					pathinfoxml.attribute("target") << "')" << endl;
			pathinfoTargetId = 0;
		}
	}
	if (pathinfoTargetId == 0) {
		pathInfo()->target = 0;
	} else {
		pathInfo()->target = canvas()->findItem(pathinfoTargetId);
		if (!pathInfo()->target) {
			boWarning(260) << k_funcinfo << "Could not find target with id=" << pathinfoTargetId << endl;
		}
	}

	// llpath
	unsigned int llpathlength = pathinfoxml.attribute("llpathlength").toInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "Error loading llpathlength attribute ('" << pathinfoxml.attribute("llpathlength") << "')" << endl;
		return false;
	}
	pathInfo()->llpath.reserve(llpathlength);
	for(unsigned int i = 0; i < llpathlength; i++) {
		loadVector2FromXML(&pathInfo()->llpath[i], pathinfoxml, QString("llpath-%1").arg(i));
	}

	// misc
	pathInfo()->moveAttacking = (pathinfoxml.attribute("moveAttacking").toInt(&ok));
	if (!ok) {
		boError() << k_funcinfo << "Error loading moveAttacking attribute ('" << pathinfoxml.attribute("moveAttacking") << "')" << endl;
		return false;
	}
	pathInfo()->slowDownAtDest = (pathinfoxml.attribute("slowDownAtDest").toInt(&ok));
	if (!ok) {
		boError() << k_funcinfo << "Error loading slowDownAtDest attribute ('" << pathinfoxml.attribute("slowDownAtDest") << "')" << endl;
		return false;
	}
	pathInfo()->waiting = pathinfoxml.attribute("waiting").toInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "Error loading waiting attribute ('" << pathinfoxml.attribute("waiting") << "')" << endl;
		return false;
	}
	pathInfo()->pathrecalced = pathinfoxml.attribute("pathrecalced").toInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "Error loading pathrecalced attribute ('" << pathinfoxml.attribute("pathrecalced") << "')" << endl;
		return false;
	}
	// These aren't saved
	pathInfo()->unit = this;
	pathInfo()->flying = unitProperties()->isAircraft();
 }
 // Null pathinfoxml is valid too: in this case, we're loading from playfield
 //  file, not from savegame


 recalculateMaxWeaponRange();

 updateAnimationMode();

 return true;
}

bool Unit::inRange(unsigned long int r, Unit* target) const
{
 return (QMAX(QABS((target->x() - x())), QABS((target->y() - y()))) <= bofixed(r));
}

void Unit::shootAt(BosonWeapon* w, Unit* target)
{
 if (!w->reloaded()) {
//	boDebug() << k_funcinfo << "gotta reload first" << endl;
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
 PROFILE_METHOD
 // TODO: we use a *rect* for the range this is extremely bad.
 // ever heard about pythagoras ;-) ?

 // AB: note that we don't need to do error checking like left < 0, since
 // collisions() does this anyway.
 BoRectFixed rect(leftEdge() - range, topEdge() - range, rightEdge() + range, bottomEdge() + range);

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
 PROFILE_METHOD
 BoItemList* units = unitsInRange(range);
 BoItemList* enemy = new BoItemList();
 Unit* u;
 BoItemList::Iterator it = units->begin();
 for (; it != units->end(); ++it) {
	u = (Unit*)*it;
	if (ownerIO()->isEnemy(u)) {
		enemy->append(u);
	}
 }
 return enemy;
}

QValueList<Unit*> Unit::unitCollisions(bool exact)
{
 PROFILE_METHOD
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
 // AB: this could break loading games! so we do NOT do it anymore. remove this,
 // if it doesn't cause problems.
// setVelocity(0.0, 0.0, 0.0);

 if (advanceWork() != w) {
	// we change the list only if work() actually changed (in favor of
	// performance). but do not return here!
	canvas()->changeAdvanceList((BosonItem*)this);
 }
 UnitBase::setAdvanceWork(w);

 // we even do this if nothing changed - just in case...
 switch (w) {
	case WorkIdle:
		setAdvanceFunction(&Unit::advanceIdle, owner()->advanceFlag());
		break;
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
		boDebug() << k_funcinfo << "ok - inrange" << endl;
		return true;
	}
 }
 return false;
}

void Unit::turnTo(int deg)
{
// boDebug() << k_funcinfo << id() << ": turning to " << deg << endl;
 d->mWantedRotation = deg;
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
 recalculateMaxWeaponRange();
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

bool Unit::moveAttacking() const
{
 return pathInfo()->moveAttacking;
}

bool Unit::slowDownAtDestination() const
{
 return pathInfo()->slowDownAtDest;
}

bofixed Unit::distance(const Unit* u) const
{
 // !!! This method returns square of actual distance. You may want to use sqrt() !!!
 bofixed dx = centerX() - u->centerX();
 bofixed dy = centerY() - u->centerY();
 bofixed dz = centerZ() - u->centerZ();
 return dx*dx + dy*dy + dz*dz;
}

bofixed Unit::distance(const BoVector3Fixed& pos) const
{
 // !!! This method returns square of actual distance. You may want to use sqrt() !!!
 bofixed dx = pos.x() - centerX();
 bofixed dy = pos.y() - centerY();
 bofixed dz = pos.z() - centerZ();
 return dx*dx + dy*dy + dz*dz;
}

const QColor* Unit::teamColor() const
{
 return &ownerIO()->teamColor();
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
 bofixed rotateX = 0.0f;
 bofixed rotateY = 0.0f;
 bofixed moveZ = 0.0f;
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
 canvas()->unitMovingStatusChanges(this, old, m);
}

BosonPathInfo* Unit::pathInfo() const
{
 return &d->mPathInfo;
}

void Unit::unitDestroyed(Unit* unit)
{
 if (unit == this) {
	return;
 }
 if (target() == unit) {
	setTarget(0);
	if (work() == WorkAttack) {
		stopAttacking();
		return;
	}
 }
 QPtrListIterator<UnitPlugin> it(d->mPlugins);
 while (it.current()) {
	it.current()->unitDestroyed(unit);
	++it;
 }
}

void Unit::itemRemoved(BosonItem* item)
{
 UnitBase::itemRemoved(item);
 if (item == (BosonItem*)this) {
	return;
 }
 if ((BosonItem*)target() == item) {
	setTarget(0);
	if (work() == WorkAttack) {
		stopAttacking();
		return;
	}
 }
 QPtrListIterator<UnitPlugin> it(d->mPlugins);
 while (it.current()) {
	it.current()->itemRemoved(item);
	++it;
 }
}

void Unit::addUpgrade(const UpgradeProperties* upgrade)
{
 UnitBase::addUpgrade(upgrade);
 recalculateMaxWeaponRange();
}

void Unit::removeUpgrade(const UpgradeProperties* upgrade)
{
 UnitBase::removeUpgrade(upgrade);
 recalculateMaxWeaponRange();
}

unsigned long int Unit::maxWeaponRange() const
{
 return d->mMaxWeaponRange;
}

unsigned long int Unit::maxAirWeaponRange() const
{
 return d->mMaxAirWeaponRange;
}

unsigned long int Unit::maxLandWeaponRange() const
{
 return d->mMaxLandWeaponRange;
}

void Unit::recalculateMaxWeaponRange()
{
 d->mMaxAirWeaponRange = 0;
 d->mMaxLandWeaponRange = 0;
 for (BoPointerIterator<BosonWeapon> it = d->mWeapons; *it; ++it) {
	const BosonWeapon* w = *it;
	if (!w->properties()) {
		boError() << k_funcinfo << "NULL properties for weapon " << w << endl;
		continue;
	}
	if (w->properties()->canShootAtAirUnits()) {
		if (w->range() > d->mMaxAirWeaponRange) {
			d->mMaxAirWeaponRange = w->range();
		}
	}
	if (w->properties()->canShootAtLandUnits()) {
		if (w->range() > d->mMaxLandWeaponRange) {
			d->mMaxLandWeaponRange = w->range();
		}
	}
 }
 d->mMaxWeaponRange = QMAX(d->mMaxAirWeaponRange, d->mMaxLandWeaponRange);
}

bool Unit::cellOccupied(int x, int y, bool ignoremoving) const
{
 // TODO: move this method away from here and merge it with the one in the pathfinder
 if (unitProperties()->isAircraft()) {
	return false;
 }
 if (!moveData()) {
	BO_NULL_ERROR(moveData());
	return false;
 }

 if (x < moveData()->edgedist1 || y < moveData()->edgedist1 ||
	x > (int)canvas()->mapWidth() - 1 - moveData()->edgedist2 ||
	y > (int)canvas()->mapHeight() - 1 - moveData()->edgedist2) {
	return true;
 }

 for (int x2 = x - moveData()->edgedist1; x2 <= x + moveData()->edgedist2; x2++) {
	for (int y2 = y - moveData()->edgedist1; y2 <= y + moveData()->edgedist2; y2++) {
		if (!moveData()->cellPassable[y2 * canvas()->mapWidth() + x2]) {
			return true;
		}

		const BoItemList* items = canvas()->cell(x2, y2)->items();
		for (BoItemList::ConstIterator it = items->begin(); it != items->end(); ++it) {
			if (RTTI::isUnit((*it)->rtti())) {
				Unit* u = (Unit*)*it;
				if(u->isFlying()) {
					// We don't care about air units
					continue;
				} else if(u == this) {
					continue;
				}

				// Maybe we can just crush the obstacle
				if (u->maxHealth() <= moveData()->crushDamage) {
					// Check player's relationship with the u's owner
					if (ownerIO()->isEnemy(u)) {
						// Crush the damn enemy :-)
						continue;
					} else if (ownerIO()->isNeutral(u)) {
						// Also crush it
						// TODO: maybe have additional cost for crushing neutral stuff???
						continue;
					}
				}

				if (ignoremoving && u->movingStatus() == UnitBase::Moving) {
					continue;
				}

				// This one's occupying the cell
				return true;
			}
			// TODO: check for e.g. mines
		}

	}
 }

 return false;
}

int Unit::getAnimationMode() const
{
 if (isDestroyed()) {
	return UnitAnimationWreckage;
 }
 return BosonItem::getAnimationMode();
}



/////////////////////////////////////////////////
// MobileUnit
/////////////////////////////////////////////////


class UnitMover
{
public:
	UnitMover(MobileUnit* u);
	~UnitMover();

	static void initCellIntersectionTable();

	bool saveAsXML(QDomElement& root) const;
	bool loadFromXML(const QDomElement& root);

	void advanceIdle(unsigned int advanceCallsCount);
	void advanceFollow(unsigned int);
	void advanceMoveInternal(unsigned int);
	void advanceMoveCheck();

	bool attackEnemyUnitsInRangeWhileMoving();

	/**
	 * Move towards p, going at most maxdist (in canvas coords).
	 * How much unit should move, will be added to xspeed and yspeed.
	 * (x; y) marks unit's current position
	 *
	 * @return How much is moved (will be <= maxdist)
	 **/
	bofixed moveTowardsPoint(const BoVector2Fixed& p, bofixed x, bofixed y, bofixed maxdist, bofixed &xspeed, bofixed &yspeed);

	void flyInCircle();

	MobileUnit* unit() const
	{
		return mUnit;
	}
	const BosonCanvas* canvas() const
	{
		return unit()->canvas();
	}
	unsigned long int id() const
	{
		return unit()->id();
	}
	UnitBase::WorkType work() const
	{
		return unit()->work();
	}
	UnitBase::WorkType advanceWork() const
	{
		return unit()->advanceWork();
	}
	const UnitProperties* unitProperties() const
	{
		return unit()->unitProperties();
	}
	BosonPathInfo* pathInfo() const
	{
		return unit()->pathInfo();
	}

	/**
	 * Call turnTo according to the current speed (you want to use this!)
	 **/
	void turnTo();

protected:
	void advanceMoveLeader(unsigned int advanceCallsCount);
	void advanceMoveFollowing(unsigned int advanceCallsCount);
	void advanceMoveFlying(unsigned int advanceCallsCount);


	/**
	 * Finds new path to destination.
	 * Destination must have been set before in @ref pathInfo
	 *
	 * This is in Unit instead of @ref MobileUnit so that we can apply a
	 * path to newly constructed units of factories.
	 **/
	bool newPath();

	int selectNextPathPoint(int xpos, int ypos);
	void avoidance();
	bool canGoToCurrentPathPoint(int xpos, int ypos);
	void currentPathPointChanged(int unitx, int unity);

public:
	static QValueVector<BoVector2Fixed> mCellIntersectionTable[11][11];

private:
	MobileUnit* mUnit;

	// Should these be made KGameProperty?
	int mLastCellX;
	int mLastCellY;

	int mNextCellX;
	int mNextCellY;
	QValueVector<BoVector2Fixed>* mNextWaypointIntersections;
	int mNextWaypointIntersectionsXOffset;
	int mNextWaypointIntersectionsYOffset;

	bofixed mRoll;

};

QValueVector<BoVector2Fixed> UnitMover::mCellIntersectionTable[11][11];


UnitMover::UnitMover(MobileUnit* u)
{
 mUnit = u;
 mNextWaypointIntersections = 0;
 mLastCellX = -1;
 mLastCellY = -1;
 mNextCellX = -1;
 mNextCellY = -1;
 mNextWaypointIntersectionsXOffset = 0;
 mNextWaypointIntersectionsYOffset = 0;

 mRoll = 0;
}

UnitMover::~UnitMover()
{
}

bool UnitMover::saveAsXML(QDomElement& root) const
{
 if (unitProperties()->isAircraft()) {
	root.setAttribute("Roll", mRoll);
 }
 return true;
}

bool UnitMover::loadFromXML(const QDomElement& root)
{
 if (unitProperties()->isAircraft()) {
	if (root.hasAttribute("Roll")) {
		bool ok;
		mRoll = root.attribute("Roll").toFloat(&ok);
		if (!ok) {
			boWarning() << k_funcinfo << "Invalid value for Roll attribute" << endl;
		} else {
			unit()->setYRotation(mRoll);
		}
	}
 }

#warning TODO: mNextWaypointIntersections
 return true;
}

void UnitMover::initCellIntersectionTable()
{
 PROFILE_METHOD
 // Go over all the cells in 11x11 area and store all the cells that the unit
 //  would be on if it would move from (0; 0) to a certain point in straight
 //  line
 // This code is entirely taken from TA Spring project
 for(int y = 0; y < 11; y++) {
	for(int x = 0; x < 11; x++) {
		BoVector2Fixed start(0.5, 0.5);
		BoVector2Fixed to(x - 5 + 0.5, y - 5 + 0.5);

		bofixed dx = to.x() - start.x();
		bofixed dy = to.y() - start.y();
		bofixed xp = start.x();
		bofixed yp = start.y();
		bofixed xn, yn;

		if (floor(start.x()) == floor(to.x())) {
			// x coordinates are same
			if (dy > 0) {
				for (int a = 1; a <= floor(to.y()); a++) {
					mCellIntersectionTable[x][y].push_back(BoVector2Fixed(0, a));
				}
			} else {
				for (int a = -1; a >= floor(to.y()); a--) {
					mCellIntersectionTable[x][y].push_back(BoVector2Fixed(0, a));
				}
			}
		} else if (floor(start.y()) == floor(to.y())) {
			// y coordinates are same
			if (dx > 0) {
				for (int a = 1; a <= floor(to.x()); a++) {
					mCellIntersectionTable[x][y].push_back(BoVector2Fixed(a, 0));
				}
			} else {
				for (int a = -1; a >= floor(to.x()); a--) {
					mCellIntersectionTable[x][y].push_back(BoVector2Fixed(a, 0));
				}
			}
		} else {
			// Tough case: both coordinates differ
			bool keepgoing = true;
			while (keepgoing) {
				if (dx > 0) {
					xn = (floor(xp) + 1 - xp) / dx;
				} else {
					xn = (floor(xp) - xp) / dx;
				}
				if (dy > 0) {
					yn = (floor(yp) + 1 - yp) / dy;
				} else {
					yn = (floor(yp) - yp) / dy;
				}

				if (xn < yn) {
					xp += (xn + 0.0001) * dx;
					yp += (xn + 0.0001) * dy;
				} else {
					xp += (yn + 0.0001) * dx;
					yp += (yn + 0.0001) * dy;
				}
				keepgoing = (QABS(xp - start.x()) < QABS(to.x() - start.x())) &&
						(QABS(yp - start.y()) < QABS(to.y() - start.y()));

				mCellIntersectionTable[x][y].push_back(BoVector2Fixed(floor(xp), floor(yp)));
			}
			mCellIntersectionTable[x][y].pop_back();
		}
	}
 }
}


void UnitMover::advanceIdle(unsigned int advanceCallsCount)
{
 if (unit()->isFlying()) {
	unit()->flyInCircle();
 }
}

void UnitMover::advanceFollow(unsigned int advanceCallsCount)
{
 if (advanceCallsCount % 5 != 0) {
	return;
 }
 BosonProfiler profiler("advanceFollow");
 if (!unit()->target()) {
	boWarning() << k_funcinfo << "cannot follow NULL unit" << endl;
	unit()->stopAttacking();  // stopAttacking should maybe be renamed to stopEverything
			//  or just stop because it's used in several places to stop unit from
			//  doing whatever it does.
	return;
 }
 if (unit()->target()->isDestroyed()) {
	boDebug(401) << k_funcinfo << "Unit is destroyed!" << endl;
	unit()->stopAttacking();
	return;
 }
// if (!isNextTo(target())) {  // This doesn't work for some reason :-(  Dunno why.
 if (QMAX(QABS(unit()->x() - unit()->target()->x()), QABS(unit()->y() - unit()->target()->y())) > 1.0f) {
	// We're not next to unit
	// AB: warning - this does a lookup on all items and therefore is slow!
	// --> but we need it as a simple test on the pointer causes trouble if
	// that pointer is already deleted. any nice solutions?
	if (!canvas()->allItems()->contains(unit()->target())) {
		boDebug(401) << k_funcinfo << "Unit seems to be destroyed!" << endl;
		unit()->stopAttacking();
		return;
	}
	boDebug(401) << k_funcinfo << "unit (" << unit()->target()->id() << ") not in range - moving..." << endl;
	if (!unit()->moveTo(unit()->target()->x(), unit()->target()->y(), 1)) {
		unit()->setWork(UnitBase::WorkIdle);
	} else {
		unit()->setAdvanceWork(UnitBase::WorkMove);
	}
 }
 // Do nothing (unit is in range)
}

void UnitMover::advanceMoveInternal(unsigned int advanceCallsCount) // this actually needs to be called for every advanceCallsCount.
{
 if (mUnit->isFlying()) {
	advanceMoveFlying(advanceCallsCount);
 } else {
	advanceMoveLeader(advanceCallsCount);
 }
}

void UnitMover::advanceMoveFlying(unsigned int advanceCallsCount)
{
 BosonProfiler profiler("advanceMoveFlying");
 //boDebug(401) << k_funcinfo << endl;

 if (unit()->maxSpeed() == 0) {
	// If unit's max speed is 0, it cannot move
	boError(401) << k_funcinfo << "unit " << id() << ": maxspeed == 0" << endl;
	unit()->stopMoving();
	unit()->setMovingStatus(UnitBase::Standing);
	return;
 }

 boDebug(401) << k_funcinfo << "unit " << id() << endl;


 // Calculate velocity
 // Missile always moves towards it's target
 BoVector3Fixed targetpos(unit()->currentWaypoint().x(), unit()->currentWaypoint().y(), 0);
 targetpos.setZ(canvas()->heightAtPoint(targetpos.x(), targetpos.y()) + 3);
 if (advanceWork() != work()) {
	if (work() == UnitBase::WorkAttack) {
		// Unit is attacking. ATM it's moving to target.
		if (!unit()->target()) {
			boError() << k_funcinfo << "unit " << id() << " is in WorkAttack, but has NULL target!" << endl;
			unit()->stopAttacking();
			unit()->flyInCircle();
			return;
		}
		int range;
		if (unit()->target()->isFlying()) {
			range = unit()->maxAirWeaponRange();
		} else {
			range = unit()->maxLandWeaponRange();
		}
		if (unit()->inRange(range, unit()->target())) {
			boDebug(401) << k_funcinfo << "unit " << id() << ": target is in range now" << endl;
			unit()->stopMoving();
			unit()->flyInCircle();
			return;
		}

		targetpos = BoVector3Fixed(unit()->target()->centerX(), unit()->target()->centerY(), unit()->target()->centerZ());
		if (!unit()->target()->isFlying()) {
			targetpos.setZ(targetpos.z() + 3);
		}
	}
 } else if (pathInfo()->moveAttacking) {
	// Attack any enemy units in range
	// Don't check for enemies every time (if we don't have a target) because it
	//  slows things down
	if (unit()->target() || (advanceCallsCount % 10 == 0)) {
		attackEnemyUnitsInRangeWhileMoving();
	}
 }


 // x and y are center of the unit here
 bofixed x = unit()->centerX();
 bofixed y = unit()->centerY();
 bofixed z = unit()->centerZ();

 unit()->accelerate();


 // Calculate totarget vector
 BoVector3Fixed totarget(targetpos.x() - x, targetpos.y() - y, targetpos.z() - z);
 bofixed totargetlen = totarget.length();
 // We need check this here to avoid division by 0 later
 if (totargetlen <= unit()->speed()) {
	unit()->stopMoving();
	return;
 }
 // Normalize totarget. totarget vector now shows direction to target
 totarget.scale(1.0f / totargetlen);

 // Check if the target is to our left or right
 bofixed wantedrotation = Bo3dTools::rotationToPoint(totarget.x(), totarget.y());
 bofixed rotationdelta = unit()->rotation() - wantedrotation;  // How many degrees to turn
 bofixed newrotation = unit()->rotation();
 bool turncw = false;  // Direction of turning, CW or CCW

 // Find out direction of turning and huw much is left to turn
 if (rotationdelta < 0) {
	rotationdelta = QABS(rotationdelta);
	turncw = true;
 }
 if (rotationdelta > 180) {
	rotationdelta = 180 - (rotationdelta - 180);
	turncw = !turncw;
 }

 const bofixed maxturningspeed = 2;
 if (rotationdelta <= maxturningspeed) {
	newrotation = wantedrotation;
 } else {
	if (turncw) {
		newrotation += maxturningspeed;
	} else {
		newrotation -= maxturningspeed;
	}
 }
 // Check for overflows
 if (newrotation < 0) {
	newrotation += 360;
 } else if (newrotation >= 360) {
	newrotation -= 360;
 }

 /*bofixed crossproduct = d->currentVelocity.x() * totarget.y() - d->currentVelocity.y() * totarget.x();
 bool turnright = true;
 if (crossproduct < 0) {
	// Turn left
	turnright = false;
 } else {
	// Turn right
 }*/

 //bofixed rotationdelta = rotation

 // Difference between current direction and direction to target
 /*BoVector3Fixed diff = totarget - d->currentVelocity;
 bofixed difflen = diff.length();
 const bofixed turnspeed = 0.08;
 if (difflen != 0) {
	// We're not flying towards the target atm
	// Calculate new velocity vector
	if (turnspeed < difflen) {
		diff.scale(turnspeed / difflen);
	}
	// Alter velocity direction so that it's more towards the target
	d->currentVelocity += diff;
	d->currentVelocity.normalize();
 }

 // This is final velocity
 BoVector3Fixed velo(d->currentVelocity * speed());*/

 // Calculate velocity
 BoVector3Fixed velo;
 velo.setX(cos(Bo3dTools::deg2rad(newrotation - 90)) * unit()->speed());
 velo.setY(sin(Bo3dTools::deg2rad(newrotation - 90)) * unit()->speed());

 bofixed groundz = canvas()->heightAtPoint(x + velo.x(), y + velo.y());
 if (z + velo.z() < groundz + unitProperties()->preferredAltitude() - 1) {
	velo.setZ(groundz - z + unitProperties()->preferredAltitude() - 1);
 } else if (z + velo.z() > groundz + unitProperties()->preferredAltitude() + 1) {
	velo.setZ(groundz - z + unitProperties()->preferredAltitude() + 1);
 }

 // Calculate roll
 /*bofixed wantedroll = QMIN(difflen / turnspeed, bofixed(1)) * 45;
 if (totarget.crossProduct(velo).z() < 0) {
	// Turning left
	wantedroll = -wantedroll;
 } else {
	// Turning right
 }*/
 bofixed wantedroll = QMIN(rotationdelta / maxturningspeed, bofixed(1)) * 45;
 if (!turncw) {
	// Turning left
	wantedroll = -wantedroll;
 }

 const bofixed maxrollincrease = 2;
 bofixed delta = wantedroll - mRoll;
 if (delta > 0) {
	mRoll += QMIN(delta, maxrollincrease);
 } else if (delta < 0) {
	mRoll -= QMIN(QABS(delta), maxrollincrease);
 }

 unit()->setVelocity(velo.x(), velo.y(), velo.z());
 unit()->setRotation(newrotation);
 //setXRotation(Bo3dTools::rotationToPoint(sqrt(1 - velo.z() * velo.z()), velo.z()) - 90);
 unit()->setYRotation(mRoll);

 unit()->setMovingStatus(UnitBase::Moving);
}

void UnitMover::advanceMoveLeader(unsigned int advanceCallsCount)
{
 BosonProfiler profiler("advanceMoveInternal");
 //boDebug(401) << k_funcinfo << endl;

 if (pathInfo()->waiting) {
	// If path is blocked and we're waiting, then there's no point in
	//  recalculating velocity and other stuff every advance call
	// TODO: check for enemies
	return;
 }

 if (unit()->maxSpeed() == 0) {
	// If unit's max speed is 0, it cannot move
	boError(401) << k_funcinfo << "unit " << id() << ": maxspeed == 0" << endl;
	unit()->stopMoving();
	unit()->setMovingStatus(UnitBase::Standing);
	return;
 }

 // Check if path is already found or not
 if (unit()->movingStatus() == UnitBase::MustSearch) {
	// Path is not yet searched
	// If we're moving with attacking, first check for any enemies in the range.
	if (attackEnemyUnitsInRangeWhileMoving()) {
		return;
	}
	// If there aren't any enemies, find new path
	if (!newPath()) {
		// No path was found
		return;
	}
 }

 // Make sure we have pathpoints
 if (unit()->pathPointCount() == 0) {
	if (pathInfo()->result == BosonPath::OutOfRange) {
		// New pathfinding query has to made, because last returned path was
		//  only partial.
		if (!newPath()) {
			// Probably no path could be found (and stopMoving() was called)
			return;
		}
	} else {
		// TODO: rotate a bit randomly
		// TODO: support multiple waypoints
		unit()->stopMoving();
		return;
	}
 }

 boDebug(401) << k_funcinfo << "unit " << id() << endl;
 if (advanceWork() != work()) {
	if (work() == UnitBase::WorkAttack) {
		// Unit is attacking. ATM it's moving to target.
		// no need to move to the position of the unit...
		// just check if unit is in range now.
		if (!unit()->target()) {
			boError() << k_funcinfo << "unit " << id() << " is in WorkAttack, but has NULL target!" << endl;
			unit()->stopAttacking();
			unit()->setMovingStatus(UnitBase::Standing);
			return;
		}
		int range;
		if (unit()->target()->isFlying()) {
			range = unit()->maxAirWeaponRange();
		} else {
			range = unit()->maxLandWeaponRange();
		}
		if (unit()->inRange(range, unit()->target())) {
			boDebug(401) << k_funcinfo << "unit " << id() << ": target is in range now" << endl;
			unit()->setMovingStatus(UnitBase::Standing);
			unit()->stopMoving();
			return;
		}
		// TODO: make sure that target() hasn't moved!
		// if it has moved also adjust waypoints
	}
 } else if (pathInfo()->moveAttacking) {
	// Attack any enemy units in range
	// Don't check for enemies every time (if we don't have a target) because it
	//  slows things down
#ifndef CHECK_ENEMIES_ONLY_AT_WAYPOINT
	if (unit()->target() || (advanceCallsCount % 10 == 0)) {
		if (attackEnemyUnitsInRangeWhileMoving()) {
			return;
		}
	}
#else
	// Attack only if we already have target
	if (unit()->target()) {
		if (attackEnemyUnitsInRangeWhileMoving()) {
			return;
		}
	}
#endif
 }

 // x and y are center of the unit here
 bofixed x = unit()->centerX();
 bofixed y = unit()->centerY();


 //boDebug(401) << k_funcinfo << "unit " << id() << ": pos: (" << x << "; "<< y << ")" << endl;
 // If we're close to destination, decelerate, otherwise  accelerate
 // TODO: we should also slow down when turning at pathpoint.
 // TODO: support range != 0
 bofixed oldspeed = unit()->speed();
 unit()->accelerate();

 // Go speed() distance towards the next pathpoint. If distance to it is less
 //  than speed(), we go towards the one after this as well
 bofixed xspeed = 0;
 bofixed yspeed = 0;
 bofixed dist = unit()->speed();
 BoVector2Fixed pp;

 // We move through the pathpoints, until we've passed dist distance
 while (dist > 0) {
	// Check if we have any pathpoints left
	if (unit()->pathPointCount() == 0) {
		if (pathInfo()->result == BosonPath::OutOfRange) {
			// New pathfinding query has to made, because last returned path was
			//  only partial.
			if (!newPath()) {
				// Probably no path could be found (and stopMoving() was called)
				break;
			}
		} else {
			// TODO: rotate a bit randomly
			// TODO: support multiple waypoints
			break;
		}
	}

	// Take next pathpoint
	pp = unit()->currentPathPoint();

	// Pathpoint skipping
	int currentCellX = (int)(x + xspeed);
	int currentCellY = (int)(y + yspeed);
	// We will check if we can skip some pathpoint if we're not on the same cell
	//  as the current pathpoint. Also we remember where we were when we skipped
	//  pathpoints last time in order not to do it every frame
	if ((currentCellX != mLastCellX) || (currentCellY != mLastCellY)) {
		if (mLastCellX == -1) {
			// New path has been searched.
			currentPathPointChanged((int)(x + xspeed), (int)(y + yspeed));
			// Modify lastCellX to not execute this again until next cell is reached
			//  or new path is searched.
			mLastCellX = -2;
		}
		if (((int)pp.x() != currentCellX) || ((int)pp.y() != currentCellY)) {
			selectNextPathPoint(currentCellX, currentCellY);
			mLastCellX = currentCellX;
			mLastCellY = currentCellY;
			pp = unit()->currentPathPoint();
		}
	}

	//boDebug(401) << k_funcinfo << "unit " << id() << ": Current pp: (" << pp.x() << "; "<< pp.y() << ")" << endl;

	// Make sure it's possible to go to the pathpoint
	if (!canGoToCurrentPathPoint(x + xspeed, y + yspeed)) {
		// Gotta find another path
		if (!newPath()) {
			// Probably no path could be found (and stopMoving() was called)
			break;
		}
		currentPathPointChanged((int)(x + xspeed), (int)(y + yspeed));
		selectNextPathPoint((int)(x + xspeed), (int)(y + yspeed));
		mLastCellX = (int)(x + xspeed);
		mLastCellY = (int)(y + yspeed);
	}

	// Move towards it
	dist -= moveTowardsPoint(pp, x + xspeed, y + yspeed, dist, xspeed, yspeed);

	// Check if we reached this pathpoint
	if ((x + xspeed == pp.x()) && (y + yspeed == pp.y())) {
		// Unit has reached pathpoint
		boDebug(401) << k_funcinfo << "unit " << id() << ": unit is at pathpoint" << endl;
		unit()->pathPointDone();
		currentPathPointChanged((int)(x + xspeed), (int)(y + yspeed));
		// Check for enemies
		if (attackEnemyUnitsInRangeWhileMoving()) {
			break;
		}
	}
 }

 // If the unit didn't move, we can just return now (this is valid)
 if ((xspeed == 0) && (yspeed == 0)) {
	return;
 }

 //boDebug(401) << k_funcinfo << "unit " << id() << ": Setting velo to: (" << xspeed << "; "<< yspeed << ")" << endl;
 unit()->setVelocity(xspeed, yspeed, 0.0);
 unit()->setMovingStatus(UnitBase::Moving);

 //avoidance();

 turnTo();

 // If we just started moving and now want to turn, then set velocity back to 0
 if ((advanceWork() == UnitBase::WorkTurn) && (oldspeed == 0)) {
	unit()->setVelocity(0, 0, 0);
 }
}

// AB: WARNING crushing is currently disabled!
//
// see #warning below
void UnitMover::advanceMoveCheck()
{
 BosonProfiler profiler("advanceMoveCheck");
 //boDebug(401) << k_funcinfo << endl;

 // Take special action if path is (was) blocked and we're waiting
 if (pathInfo()->waiting) {
	// We try to move every 5 advance calls
	if (pathInfo()->waiting % 5 == 0) {
		// Try to move again
		if (unit()->cellOccupied(mNextCellX, mNextCellY)) {
			// Obstacle is still there. Continue waiting
			if (pathInfo()->waiting >= 600) {
				// Enough of waiting (30 secs). Give up.
				unit()->stopMoving();
				unit()->setWork(UnitBase::WorkIdle);
				return;
			} else if (pathInfo()->waiting % (20 + QMIN(pathInfo()->pathrecalced * 20, 80)) == 0) {
				// First wait 20 adv. calls (1 sec) before recalculating path, then 40
				//  calls, then 60 etc, but never more than 100 calls.
				boDebug(401) << k_funcinfo << "unit " << id() << ": Recalcing path, waiting: " << pathInfo()->waiting <<
						"; pathrecalced: " << pathInfo()->pathrecalced << endl;
				newPath();
				pathInfo()->pathrecalced++;
				pathInfo()->waiting = 0;
				// New path will be used next advance call
				return;
			}
			pathInfo()->waiting++;
			return;
		} else {
			// Our path is free again. We can continue moving next advance call, but
			//  not immediately, because advanceMoveInternal() didn't calculate
			//  velocities
			pathInfo()->waiting = 0;
			unit()->setMovingStatus(UnitBase::Waiting);  // TODO: is this ok here? maybe set to Moving instead?
			return;
		}
	} else {
		// Don't check if we can move every advance call. Just return for now
		pathInfo()->waiting++;
		return;
	}
 }

 // Check if top-left point of the unit will be on canvas after moving
 if (!canvas()->onCanvas(unit()->boundingRectAdvanced().topLeft())) {
	BoVector2Fixed point = unit()->boundingRectAdvanced().topLeft();
	QString problem;
	if (!canvas()->onCanvas(BoVector2Fixed(0, point.y()))) {
		problem = QString("top==%1").arg(point.y());
	} else if (!canvas()->onCanvas(BoVector2Fixed(point.x(), 0))) {
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
			<< unit()->boundingRect().topLeft().x() << ";"
			<< unit()->boundingRect().topLeft().y() << ")" << endl;
	unit()->stopMoving();
	return;
 }
 // Check if bottom-right point of the unit will be on canvas after moving
 if (!canvas()->onCanvas(unit()->boundingRectAdvanced().bottomRight())) {
	BoVector2Fixed point = unit()->boundingRectAdvanced().bottomRight();
	QString problem;
	if (!canvas()->onCanvas(BoVector2Fixed(0, point.y()))) {
		problem = QString("bottom==%1").arg(point.y());
	} else if (!canvas()->onCanvas(BoVector2Fixed(point.x(), 0))) {
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
			<< "  current rightEdge: " << unit()->rightEdge()
			<< " ; current bottomEdge:" << unit()->bottomEdge()
			<< " ; xVelocity: " << unit()->xVelocity()
			<< " ; (int)xVelocity: " << (int)unit()->xVelocity()
			<< " ; yVelocity: " << unit()->yVelocity()
			<< " ; (int)yVelocity: " << (int)unit()->yVelocity()
			<< " problem was: "
			<< problem << endl;
	boError(401) << k_funcinfo << "leaving unit a current bottomright pos: ("
			<< unit()->boundingRect().bottomRight().x() << ";"
			<< unit()->boundingRect().bottomRight().y() << ")" << endl;
	unit()->stopMoving();
	return;
 }

 if (unit()->pathPointCount() == 0) {
	// This is allowed and means that unit will stop after this this advance call
	return;
 }

 if (unitProperties()->isAircraft()) {
	return;
 }

 if (unit()->xVelocity() == 0 && unit()->yVelocity() == 0) {
	// Probably unit stopped to attack other units
	return;
 }

 //boDebug(401) << k_funcinfo << "unit " << id() << endl;

 // Crushing & waiting
#warning What the hell does crushing do in advanceMove_Check_() ??
#define CRUSHING 0
#if CRUSHING
 bool wait = false;
 QValueList<Unit*> collisions;
 collisions = canvas()->collisionsInBox(BoVector3Fixed(unit()->x() + unit()->xVelocity(), unit()->y() + unit()->yVelocity(), unit()->z()),
		BoVector3Fixed(unit()->x() + unit()->xVelocity() + unit()->width(), unit()->y() + unit()->yVelocity() + unit()->height(), unit()->z() + unit()->depth()), unit());
 if (!collisions.isEmpty()) {
	for (QValueList<Unit*>::Iterator it = collisions.begin(); it != collisions.end(); it++) {
		Unit* u = *it;
		if (unit() == u) {
			continue;
		} else if (u->isFlying() != unit()->isFlying()) {
			continue;
		} else if (!u->bosonCollidesWith(unit())) {
			continue;
		}
		// Make sure we actually can crush this unit
		if (u->maxHealth() > unitProperties()->crushDamage()) {
			boDebug(401) << k_funcinfo << id() << ": Colliding with uncrushable unit " << u->id() << endl;
			// Whoops. Now what?
			wait = true;
			continue;
		}
		// Crush it!
		boDebug(401) << k_funcinfo << id() << ": Crushing " << u->id() << endl;
		u->setHealth(0);
		canvas()->destroyUnit(u);
	}
 }
#endif
 /*if (wait) {
	setVelocity(0, 0, 0);
	setMovingStatus(UnitBase::Waiting);
	setSpeed(0);
 }*/

 // Check if we need to wait
 // Find the next cell we'll be on
 int currentx = (int)(unit()->center().x() + unit()->xVelocity());
 int currenty = (int)(unit()->center().y() + unit()->yVelocity());
 mNextCellX = -1;
 BO_CHECK_NULL_RET(mNextWaypointIntersections);
 for (unsigned int i = 0; i < mNextWaypointIntersections->count(); i++) {
	if (currentx == mNextWaypointIntersections->at(i).x() && currenty == mNextWaypointIntersections->at(i).y()) {
		if (i+1 >= mNextWaypointIntersections->count()) {
			// This is the last cell, i.e. we're at pathpoint cell
			mNextCellX = mNextWaypointIntersections->last().x();
			mNextCellY = mNextWaypointIntersections->last().y();
		} else {
			// We're in the middle of the way to the next pathpoint. Use the next cell
			mNextCellX = mNextWaypointIntersections->at(i+1).x();
			mNextCellY = mNextWaypointIntersections->at(i+1).y();
		}
		break;
	}
 }
 if (mNextCellX == -1) {
	// We're at the beginning of the way to the next pathpoint
	mNextCellX = mNextWaypointIntersections->first().x();
	mNextCellY = mNextWaypointIntersections->first().y();
 }
 mNextCellX += mNextWaypointIntersectionsXOffset;
 mNextCellY += mNextWaypointIntersectionsYOffset;

 // Make sure the next cell is free of any obstacles
 if (unit()->cellOccupied(mNextCellX, mNextCellY)) {
	// Gotta wait
	unit()->setVelocity(0, 0, 0);
	unit()->setMovingStatus(UnitBase::Waiting);
	unit()->setSpeed(0);
	pathInfo()->waiting++;
	return;
 }


 pathInfo()->waiting = 0;
 pathInfo()->pathrecalced = 0;

 //boDebug(401) << k_funcinfo << "unit " << id() << ": done" << endl;
}

void UnitMover::advanceMoveFollowing(unsigned int advanceCallsCount)
{
}

void UnitMover::currentPathPointChanged(int unitx, int unity)
{
 if (unitProperties()->isAircraft()) {
	return;
 }
 int xindex = (int)unit()->currentPathPoint().x() - unitx + 5;
 int yindex = (int)unit()->currentPathPoint().y() - unity + 5;
 mNextWaypointIntersectionsXOffset = unitx;
 mNextWaypointIntersectionsYOffset = unity;
 mNextWaypointIntersections = &mCellIntersectionTable[xindex][yindex];
}

int UnitMover::selectNextPathPoint(int xpos, int ypos)
{
 if (unitProperties()->isAircraft()) {
	return 0;
 }
 //return 0;
 // TODO: better name?
 // Find next pathpoint where we can go in straight line
 int skipped = 0;
 bool cango = true;
 while (cango && (skipped < 6) && (unit()->pathPointCount() > 1)) {
	int newCellX = (int)unit()->pathPointList()[1].x();
	int newCellY = (int)unit()->pathPointList()[1].y();
	if ((QABS(newCellX - xpos) >= 6) || (QABS(newCellY - ypos) >= 6)) {
		// We're too far
		break;
	}
	int xindex = newCellX - xpos + 5;
	int yindex = newCellY - ypos + 5;
	for (unsigned int i = 0; i < mCellIntersectionTable[xindex][yindex].count(); i++) {
		// Check if this cell is accessable
		// TODO: check terrain
		if (unit()->cellOccupied(xpos + mCellIntersectionTable[xindex][yindex][i].x(),
				ypos + mCellIntersectionTable[xindex][yindex][i].y(), true)) {
			cango = false;
			break;
		}
	}
	if (cango) {
		// skip previous waypoint (go directly to this one)
		unit()->pathPointDone();
		currentPathPointChanged(xpos, ypos);
		skipped++;
	}
 }

 return skipped;
}

void UnitMover::avoidance()
{
 BoVector2Fixed velocity(unit()->xVelocity(), unit()->yVelocity());
 velocity.normalize();
 BoVector3Fixed toRight3 = BoVector3Fixed(velocity.x(), -velocity.y(), 0).crossProduct(BoVector3Fixed(0, 0, 1));
 BoVector2Fixed toRight(toRight3.x(), -toRight3.y());
 bofixed avoidstrength = 0;
 // Find all units which are near us
 BoRectFixed rect(unit()->centerX() - unit()->width() - unit()->speed() * 20 - 3, unit()->centerY() - unit()->width() - unit()->speed() * 20 - 3,
		unit()->centerX() + unit()->width() + unit()->speed() * 20 + 3, unit()->centerY() + unit()->width() + unit()->speed() * 20 + 3);
 BoItemList* items = canvas()->collisions()->collisionsAtCells(rect, unit(), false);
 // Go through the units
 for (BoItemList::ConstIterator it = items->begin(); it != items->end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		// TODO: check for e.g. mines
		continue;
	}
	Unit* u = (Unit*)*it;
	if (u == unit()) {
		continue;
	} else if (u->isFlying()) {
		// We don't care about flying units
		continue;
	} else if (u->maxHealth() <= unitProperties()->crushDamage()) {
		// We can just crush this one
		continue;
	}
	if (velocity.dotProduct(u->center() - unit()->center()) <= 0) {
		// We only care about units in front of us
		continue;
	}

//	BoVector2Fixed toUnit((u->center() + BoVector2Fixed(u->xVelocity(), u->yVelocity()) * 20) - center());
	bofixed unitSizesSum = (unit()->width() + u->width()) / 2;
	// Vector and distance to the other unit ATM
	BoVector2Fixed toUnit(u->center() - unit()->center());
	bofixed distToUnit = toUnit.length();
	// Vector and distance to the other unit in one second
	BoVector2Fixed toUnitSoon((u->center() + BoVector2Fixed(u->xVelocity(), u->yVelocity()) * 20) -
			(unit()->center() + BoVector2Fixed(unit()->xVelocity(), unit()->yVelocity()) * 20));
	bofixed distToUnitSoon = toUnitSoon.length();
	if ((distToUnitSoon < distToUnit) && (distToUnitSoon < unitSizesSum + 1.5)) {
		// We're getting too close to the other unit
		// Distance between the units on the axis perpendicular to this unit's rotation
		// TODO: maybe use toUnitSoon instead?
		bofixed sideDistance = toUnit.dotProduct(toRight);
		if (QABS(sideDistance) < unitSizesSum) {
			// We'll crash with this unit if we don't act
			// Calculate how much we'll try to avoid this unit
			bofixed factor = 1;  // Positive = turn right
			if (sideDistance > 0) {
				// The other unit is right of us. Turn left
				factor *= -1;
			}
			// The more int front of us the unit is, the bigger the factor
			factor *= unitSizesSum * 1.5 - QABS(sideDistance);
			// The farther from us the unit is, the smaller the factor
			factor *= 2 - (distToUnitSoon - unitSizesSum);
			//factor /= distToUnit;

			avoidstrength += factor;
		}
	}
 }
 if (avoidstrength != 0) {
	// Calculate new velocity
	BoVector2Fixed avoidance(toRight * avoidstrength);
	boDebug(401) << k_funcinfo << id() << ": Avoidstrength: " << avoidstrength <<
			"; avoidance vector: (" << avoidance.x() << "; " << avoidance.y() << ")" << endl;
	velocity += avoidance;
	velocity.normalize();
	boDebug(401) << k_funcinfo << id() << ": Changing velo from (" << unit()->xVelocity() << "; " << unit()->yVelocity() <<
			") to (" << velocity.x() * unit()->speed() << "; " << velocity.y() * unit()->speed() << ")" << endl;
	unit()->setVelocity(velocity.x() * unit()->speed(), velocity.y() * unit()->speed(), unit()->zVelocity());
 }
}

bool UnitMover::canGoToCurrentPathPoint(int xpos, int ypos)
{
 if (unitProperties()->isAircraft()) {
	return true;
 }
 int ppx = (int)unit()->currentPathPoint().x();
 int ppy = (int)unit()->currentPathPoint().y();
 if ((QABS(ppx - xpos) >= 6) || (QABS(ppy - ypos) >= 6)) {
	// We're too far
	return false;
 }
 int xindex = ppx - xpos + 5;
 int yindex = ppy - ypos + 5;
 for (unsigned int i = 0; i < UnitMover::mCellIntersectionTable[xindex][yindex].count(); i++) {
	// Check if this cell is accessable
	// TODO: check terrain
	const BoItemList* items = canvas()->cell(xpos + UnitMover::mCellIntersectionTable[xindex][yindex][i].x(),
			ypos + UnitMover::mCellIntersectionTable[xindex][yindex][i].y())->items();
	for (BoItemList::ConstIterator it = items->begin(); it != items->end(); ++it) {
		if (RTTI::isUnit((*it)->rtti())) {
			Unit* u = (Unit*)*it;
			if (unit() == u) {
				continue;
			} else if (u->isFlying()) {
				continue;
			}
			if (u->movingStatus() != UnitBase::Moving) {
				return false;
			}
		}
	}
 }
 return true;
}

void UnitMover::turnTo()
{
 bofixed xspeed = unit()->xVelocity();
 bofixed yspeed = unit()->yVelocity();
 // Set correct rotation
 // Try to find rotation fast first
 if ((xspeed == 0) && (yspeed < 0)) { // North
	unit()->turnTo(0);
 } else if ((xspeed > 0) && (yspeed == 0)) { // East
	unit()->turnTo(90);
 } else if ((xspeed == 0) && (yspeed > 0)) { // South
	unit()->turnTo(180);
 } else if ((xspeed < 0) && (yspeed == 0)) { // West
	unit()->turnTo(270);
 } else if (QABS(xspeed) == QABS(yspeed)) {
	if ((xspeed > 0) && (yspeed < 0)) { // NE
		unit()->turnTo(45);
	} else if ((xspeed > 0) && (yspeed > 0)) { // SE
		unit()->turnTo(135);
	} else if ((xspeed < 0) && (yspeed > 0)) { // SW
		unit()->turnTo(225);
	} else if ((xspeed < 0) && (yspeed < 0)) { // NW
		unit()->turnTo(315);
	} else if(xspeed == 0 && yspeed == 0) {
//		boDebug() << k_funcinfo << "xspeed == 0 and yspeed == 0" << endl;
	}
 } else {
	// Slow way - calculate direction
	unit()->turnTo((int)Bo3dTools::rotationToPoint(xspeed, yspeed));
 }
}

bool UnitMover::attackEnemyUnitsInRangeWhileMoving()
{
 PROFILE_METHOD
 // Don't attack other units unless work is WorkMove
 if (work() != UnitBase::WorkMove) {
	return false;
 }
 if (pathInfo()->moveAttacking && unit()->attackEnemyUnitsInRange()) {
	boDebug(401) << k_funcinfo << "unit " << id() << ": Enemy units found in range, attacking" << endl;
	if (unit()->isFlying()) {
		// Don't stop moving
		return true;
	}
	unit()->setVelocity(0.0, 0.0, 0.0);  // To prevent moving
	unit()->setSpeed(0);
	unit()->setMovingStatus(UnitBase::Engaging);
	return true;
 }
 return false;
}

void UnitMover::flyInCircle()
{
 if (!unit()->isFlying()) {
	return;
 }

 bofixed speedfactor = unit()->speed() / unit()->maxSpeed();
 if (speedfactor < 0.7) {
	unit()->accelerate();
 } else if (speedfactor > 0.8) {
	unit()->decelerate();
 }

 // Flying units need to keep flying
 // TODO: choose which way to turn
 bofixed newrot = unit()->rotation() + unit()->maxSpeed() * 20;
 if (newrot > 360) {
	newrot -= 360;
 }
 BoVector3Fixed velo(0, 0, 0);
 velo.setX(cos(Bo3dTools::deg2rad(newrot - 90)) * unit()->speed());
 velo.setY(sin(Bo3dTools::deg2rad(newrot - 90)) * unit()->speed());

 // Don't go off the map
 if (unit()->x() < 0.5 || unit()->y() < 0.5 ||
		unit()->x() > bofixed(canvas()->mapWidth()) - unit()->width() - 0.5 || unit()->y() > bofixed(canvas()->mapHeight()) - unit()->height() - 0.5) {
	unit()->move(QMIN(QMAX(unit()->x(), bofixed(1)), bofixed(canvas()->mapWidth()) - unit()->width() - 1),
			QMIN(QMAX(unit()->y(), bofixed(1)), bofixed(canvas()->mapHeight()) - unit()->height() - 1), unit()->z());
 }

 bofixed groundz = canvas()->heightAtPoint(unit()->centerX() + velo.x(), unit()->centerY() + velo.y());
 if (unit()->z() + velo.z() < groundz + unitProperties()->preferredAltitude() - 1) {
	velo.setZ(groundz - unit()->z() + unitProperties()->preferredAltitude() - 1);
 } else if (unit()->z() + velo.z() > groundz + unitProperties()->preferredAltitude() + 1) {
	velo.setZ(groundz - unit()->z() + unitProperties()->preferredAltitude() + 1);
 }

 unit()->setRotation(newrot);
 unit()->setVelocity(velo.x(), velo.y(), velo.z());
 //d->currentVelocity = velo;
 //d->currentVelocity.normalize();

 // Calculate roll
 bofixed wantedroll = 25;
 const bofixed maxrollincrease = 2;
 bofixed delta = wantedroll - mRoll;
 if (delta > 0) {
	mRoll += QMIN(delta, maxrollincrease);
 } else if (delta < 0) {
	mRoll -= QMIN(QABS(delta), maxrollincrease);
 }

 unit()->setYRotation(mRoll);
}

bofixed UnitMover::moveTowardsPoint(const BoVector2Fixed& p, bofixed x, bofixed y, bofixed maxdist, bofixed &xspeed, bofixed &yspeed)
{
 PROFILE_METHOD;
 // Passed distance
 bofixed dist = 0.0f;
 // Calculate difference between point and our current position
 bofixed xdiff, ydiff;
 xdiff = p.x() - x;
 ydiff = p.y() - y;
 bofixed difflen = sqrt(xdiff*xdiff + ydiff*ydiff);

 if (difflen <= maxdist) {
	xspeed += xdiff;
	yspeed += ydiff;
	return difflen;
 } else {
	bofixed factor = maxdist / difflen;
	xspeed += xdiff * factor;
	yspeed += ydiff * factor;
	return maxdist;
 }
}

bool UnitMover::newPath()
{
 BosonProfiler profiler("newPath");
 boDebug(401) << k_funcinfo << "unit " << id() << endl;

 // Update our start position
 pathInfo()->start.set(unit()->centerX(), unit()->centerY());

 // Find path
 canvas()->pathFinder()->findPath(pathInfo());

 if (pathInfo()->result == BosonPath::NoPath || pathInfo()->llpath.count() == 0) {
	// Stop moving
	unit()->stopMoving();
	return false;
 }
 // Copy low-level path to pathpoints' list
 unit()->clearPathPoints();
 for (int unsigned i = 0; i < pathInfo()->llpath.count(); i++) {
	unit()->addPathPoint(pathInfo()->llpath[i]);
 }

 // Reset last cell
 mLastCellX = -1;
 mLastCellY = -1;
 mNextWaypointIntersections = &mCellIntersectionTable[5][5];

 return true;
}


MobileUnit::MobileUnit(const UnitProperties* prop, Player* owner, BosonCanvas* canvas)
	: Unit(prop, owner, canvas),
	mMaxSpeed(prop, "Speed", "MaxValue"),
	mMaxAccelerationSpeed(prop, "AccelerationSpeed", "MaxValue"),
	mMaxDecelerationSpeed(prop, "DecelerationSpeed", "MaxValue")
{
 mUnitMover = new UnitMover(this);

 setMaxSpeed(mMaxSpeed.value(upgradesCollection())); // AB: WARNING: maxSpeed() must NOT be used here (we _set_ it here)
 setAccelerationSpeed(maxAccelerationSpeed());
 setDecelerationSpeed(maxDecelerationSpeed());
}

MobileUnit::~MobileUnit()
{
 delete mUnitMover;
}

bool MobileUnit::init()
{
  bool ret = Unit::init();
  if (!ret) {
	return ret;
  }

 if (isFlying()) {
	setSpeed(maxSpeed() * 0.75);
	move(x(), y(), unitProperties()->preferredAltitude());
 }

 setWork(WorkIdle);
 return true;
}

void MobileUnit::initStatic()
{
 UnitMover::initCellIntersectionTable();
}

bofixed MobileUnit::maxAccelerationSpeed() const
{
 return mMaxAccelerationSpeed.value(upgradesCollection());
}

bofixed MobileUnit::maxDecelerationSpeed() const
{
 return mMaxDecelerationSpeed.value(upgradesCollection());
}

void MobileUnit::addUpgrade(const UpgradeProperties* upgrade)
{
 changeUpgrades(upgrade, true);
}

void MobileUnit::removeUpgrade(const UpgradeProperties* upgrade)
{
 changeUpgrades(upgrade, false);
}

void MobileUnit::changeUpgrades(const UpgradeProperties* upgrade, bool add)
{
 // AB: these are special cases: they are stored and handled in BosonItem and
 // therefore we can not use some kind of "speedFactor" as we do with e.g.
 // health
 bofixed origMaxSpeed = mMaxSpeed.value(upgradesCollection());
 bofixed origMaxAccelerationSpeed = maxAccelerationSpeed();
 bofixed origMaxDecelerationSpeed = maxDecelerationSpeed();

 if (add) {
	Unit::addUpgrade(upgrade);
 } else {
	Unit::removeUpgrade(upgrade);
 }

 if (origMaxSpeed != mMaxSpeed.value(upgradesCollection())) {
	setMaxSpeed(mMaxSpeed.value(upgradesCollection()));
 }
 // AB: accelerationSpeed/decelerationSpeed always use maximum values
 if (origMaxAccelerationSpeed != maxAccelerationSpeed()) {
	setAccelerationSpeed(maxAccelerationSpeed());
 }
 if (origMaxDecelerationSpeed != maxDecelerationSpeed()) {
	setDecelerationSpeed(maxDecelerationSpeed());
 }
}

void MobileUnit::advanceIdle(unsigned int advanceCallsCount)
{
 Unit::advanceIdle(advanceCallsCount);

 mUnitMover->advanceIdle(advanceCallsCount);
}

void MobileUnit::advanceMoveInternal(unsigned int advanceCallsCount) // this actually needs to be called for every advanceCallsCount.
{
 mUnitMover->advanceMoveInternal(advanceCallsCount);
}

void MobileUnit::advanceMoveCheck()
{
 mUnitMover->advanceMoveCheck();
}

void MobileUnit::advanceFollow(unsigned int advanceCallsCount)
{
 mUnitMover->advanceFollow(advanceCallsCount);
}

void MobileUnit::turnTo(int dir)
{
 if (isDestroyed()) {
	boError() << k_funcinfo << "unit is already destroyed!" << endl;
	return;
 }
 if ((int)rotation() != dir) {
	// Find out how much we have to turn
	bofixed delta = rotation() - dir;
	if (delta < 0) {
		delta = QABS(delta);
	}
	if (delta > 180) {
		delta = 360 - delta;
	}

	if (delta < unitProperties()->rotationSpeed()) {
		// Turn immediately (and hope this method won't be called more than once per
		//  advance call)
		setRotation(dir);
		return;
	}
	boDebug() << k_funcinfo << id() << ": will slowly rotate from " << rotation() << " to " << dir << endl;
	// If we're moving, we want to take one more step with current velocity, but
	//  setAdvanceWork() resets it to 0, so we have this workaround here
	bofixed _xVelocity = 0, _yVelocity = 0;
	if (advanceWork() == WorkMove) {
		_xVelocity = xVelocity();
		_yVelocity = yVelocity();
	}
	Unit::turnTo(dir);
	setAdvanceWork(WorkTurn);
	setVelocity(_xVelocity, _yVelocity, 0);
 }
}

bool MobileUnit::loadFromXML(const QDomElement& root)
{
 if (!Unit::loadFromXML(root)) {
	boError() << k_funcinfo << "Unit not loaded properly" << endl;
	return false;
 }

 bool ok = false;
 bofixed speed = 0.0f;
 if (root.hasAttribute("Speed")) {
	speed = root.attribute("Speed").toFloat(&ok);
	if (!ok) {
		boError() << k_funcinfo << "Invalid value for Speed attribute" << endl;
		return false;
	}
	setSpeed(speed);
 }

 return mUnitMover->loadFromXML(root);
}

bool MobileUnit::saveAsXML(QDomElement& root)
{
 if (!Unit::saveAsXML(root)) {
	boError() << k_funcinfo << "Unit not saved properly" << endl;
	return false;
 }

 root.setAttribute("Speed", speed());

 return mUnitMover->saveAsXML(root);
}

void MobileUnit::stopMoving()
{
 Unit::stopMoving();
 if (!isFlying()) {
	if (pathInfo()->slowDownAtDest) {
		setSpeed(0);
	}
 }
}

void MobileUnit::flyInCircle()
{
 mUnitMover->flyInCircle();
}


/////////////////////////////////////////////////
// Facility
/////////////////////////////////////////////////

UnitConstruction::UnitConstruction(Facility* f)
{
 mFacility = f;

 unit()->registerData(&mConstructionStep, Unit::IdConstructionStep);
 mConstructionStep.setLocal(0);
}

UnitConstruction::~UnitConstruction()
{
}

void UnitConstruction::advanceConstruction(unsigned int advanceCallsCount)
{
 if (advanceCallsCount % 20 != 0) {
	return;
 }
 BosonProfiler profiler("advanceConstruction");
 if (unit()->isDestroyed()) {
	boError() << k_funcinfo << "unit is already destroyed" << endl;
	return;
 }
 setConstructionStep(currentConstructionStep() + 1);
}

bool UnitConstruction::isConstructionComplete() const
{
 if (unit()->work() == UnitBase::WorkConstructed) {
	return false;
 }
 if (currentConstructionStep() < constructionSteps()) {
	return false;
 }
 return true;
}

double UnitConstruction::constructionProgress() const
{
 unsigned int constructionTime = constructionSteps();
 double percentage = (double)(currentConstructionStep() * 100) / (double)constructionTime;
 return percentage;
}

unsigned int UnitConstruction::constructionSteps() const
{
 return unit()->unitProperties()->constructionSteps();
}

void UnitConstruction::setConstructionStep(unsigned int step)
{
 if (step > constructionSteps()) {
	step = constructionSteps();
 }

 mConstructionStep = step;
 if (step == constructionSteps()) {
	unit()->setWork(UnitBase::WorkIdle);
	unit()->owner()->facilityCompleted(unit());
	unit()->updateAnimationMode();
 }
}

unsigned int UnitConstruction::currentConstructionStep() const
{
 return mConstructionStep;
}

bool UnitConstruction::loadFromXML(const QDomElement& root)
{
 if (mConstructionStep > constructionSteps()) {
	mConstructionStep = constructionSteps();
 }

 unit()->updateAnimationMode();

 return true;
}

bool UnitConstruction::saveAsXML(QDomElement&)
{
 return true;
}


Facility::Facility(const UnitProperties* prop, Player* owner, BosonCanvas* canvas) : Unit(prop, owner, canvas)
{
 mUnitConstruction = new UnitConstruction(this);
}

Facility::~Facility()
{
 delete mUnitConstruction;
}

bool Facility::init()
{
 bool ret = Unit::init();
 if (!ret) {
	return ret;
 }
 setWork(WorkConstructed);
 updateAnimationMode();
 return true;
}

void Facility::advanceConstruction(unsigned int advanceCallsCount)
{
 mUnitConstruction->advanceConstruction(advanceCallsCount);
}

UnitPlugin* Facility::plugin(int pluginType) const
{
 if (!mUnitConstruction->isConstructionComplete()) {
	return 0;
 }
 return Unit::plugin(pluginType);
}

void Facility::setTarget(Unit* u)
{
 if (u && !mUnitConstruction->isConstructionComplete()) {
	boWarning() << k_funcinfo << "not yet constructed completely" << endl;
	return;
 }
 Unit::setTarget(u);
}

void Facility::moveTo(bofixed x, bofixed y, int range)
{
 if (!mUnitConstruction->isConstructionComplete()) {
	boWarning() << k_funcinfo << "not yet constructed completely" << endl;
	return;
 }
 Unit::moveTo(x, y, range);
}

bool Facility::loadFromXML(const QDomElement& root)
{
 if (!Unit::loadFromXML(root)) {
	boError() << k_funcinfo << "Unit not loaded properly" << endl;
	return false;
 }

 return mUnitConstruction->loadFromXML(root);
}

bool Facility::saveAsXML(QDomElement& root)
{
 return Unit::saveAsXML(root);
}

int Facility::getAnimationMode() const
{
 if (isDestroyed()) {
	return Unit::getAnimationMode();
 }
 if (!mUnitConstruction->isConstructionComplete()) {
	return UnitAnimationConstruction;
 }
 return Unit::getAnimationMode();
}

