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

#include "bosoncanvas.h"
#include "player.h"
#include "cell.h"
#include "unit.h"
#include "unitplugins.h"
#include "bosonmap.h"
#include "unitproperties.h"
#include "speciestheme.h"
#include "boitemlist.h"
#include "bosonparticlesystem.h"
#include "bosonparticlesystemproperties.h"
#include "defines.h"
#include "items/bosonshot.h"
#include "bosonweapon.h"
#include "bosonstatistics.h"
#include "bosonprofiling.h"
#include "bosoncanvasstatistics.h"
#include "bodebug.h"
#include "boson.h"

#include <klocale.h>

#include <qpointarray.h>
#include <qdatastream.h>
#include <qdom.h>

#include <math.h>

#include "bosoncanvas.moc"


class BosonCanvas::BosonCanvasPrivate
{
public:
	BosonCanvasPrivate()
		: mAllItems(BoItemList(0, false))
	{
		mStatistics = 0;
		mMap = 0;
	}
	BosonCanvasStatistics* mStatistics;

	QPtrList<Unit> mDestroyedUnits;

	BosonMap* mMap; // just a pointer - no memory allocated

	QPtrList<BosonItem> mAnimList; // see BosonCanvas::slotAdvance()

	BoItemList mAllItems;
	QPtrList<BosonParticleSystem> mParticles;

	// by default ALL items are in "work" == -1. if an item changes its work
	// (i.e. it is a unit and it called setAdvanceWork()) then it will go to
	// another list (once slotAdvance() reaches its end)
	QMap<int, QPtrList<BosonItem> > mWork2AdvanceList;
	QPtrList<BosonItem> mChangeAdvanceList; // work has been changed - request to change advance list
};

BosonCanvas::BosonCanvas(QObject* parent)
		: QObject(parent, "BosonCanvas")
{
 init();
}

void BosonCanvas::init()
{
 d = new BosonCanvasPrivate;
 d->mDestroyedUnits.setAutoDelete(false);
 d->mParticles.setAutoDelete(true);
 mAdvanceFunctionLocked = false;
 mCollisions = new BosonCollisions();
 d->mStatistics = new BosonCanvasStatistics(this);
}

BosonCanvas::~BosonCanvas()
{
boDebug()<< k_funcinfo << endl;
 quitGame();
 delete d->mStatistics;
 delete mCollisions;
 delete d;
boDebug()<< k_funcinfo <<"done"<< endl;
}

BosonCanvasStatistics* BosonCanvas::canvasStatistics() const
{
 return d->mStatistics;
}

void BosonCanvas::quitGame()
{
 deleteDestroyed(); // already called before
 d->mAnimList.clear();
 d->mParticles.clear();
 QMap<int, QPtrList<BosonItem> >::Iterator it;
 for (it = d->mWork2AdvanceList.begin(); it != d->mWork2AdvanceList.end(); ++it) {
	(*it).clear();
 }
 d->mChangeAdvanceList.clear();
}

void BosonCanvas::deleteDestroyed()
{
 d->mDestroyedUnits.setAutoDelete(true);
 d->mDestroyedUnits.clear();
 d->mDestroyedUnits.setAutoDelete(false);
}

Cell* BosonCanvas::cell(int x, int y) const
{
 if (!d->mMap) {
	boError() << k_funcinfo << "NULL map" << endl;
	return 0;
 }
 return d->mMap->cell(x, y);
}

Cell* BosonCanvas::cells() const
{
 BO_CHECK_NULL_RET0(d->mMap);
 return d->mMap->cells();
}

void BosonCanvas::slotAddUnit(Unit* unit, int x, int y)
{
 if (!unit) {
	boError() << k_funcinfo << "NULL unit!" << endl;
	return;
 }

 unit->move(x, y, unit->z());
// unit->show();
}

void BosonCanvas::slotAdvance(unsigned int advanceCount, bool advanceFlag)
{
#define USE_ADVANCE_LISTS 1
#define DO_ITEM_PROFILING 0
 boProfiling->advance(true, advanceCount);
 QPtrListIterator<BosonItem> animIt(d->mAnimList);
 lockAdvanceFunction();
 boProfiling->advanceFunction(true);
 d->mStatistics->resetWorkCounts();

#if USE_ADVANCE_LISTS
 // first we need to call *all* BosonItem::advance() functions.
 // AB: profiling information will be inaccurate because of this... we are
 // collecting for every item advance() here, and below for some items
 // advanceFunction(). those will be listed as *different* items...
 for (; animIt.current(); ++animIt) {
	unsigned int id;
	int work;
	BosonItem* s = animIt.current();
	if (RTTI::isUnit(s->rtti())) {
		id = ((Unit*)s)->id();
		work = (int)((Unit*)s)->advanceWork();
	} else {
		id = 0;
		work = -1;
	}
	d->mStatistics->increaseWorkCount(work);
#if DO_ITEM_PROFILING
	boProfiling->advanceItemStart(s->rtti(), id, work);
	boProfiling->advanceItem(true);
#endif
	s->advance(advanceCount);
#if DO_ITEM_PROFILING
	boProfiling->advanceItem(false);
	boProfiling->advanceItemStop();
#endif
 }

 // now the rest - mainly call BosonItem::advanceFunction().
 // this depends on in which list an item resides (changed when Unit::work()
 // changes). normal items are usually in -1.
 QMap<int, QPtrList<BosonItem> >::Iterator it;
 for (it = d->mWork2AdvanceList.begin(); it != d->mWork2AdvanceList.end(); ++it) {
	int work = it.key();
	bool skip = false;
	switch (work) {
		// TODO: instead of a big switch we should maintain a
		// d->mWork2AdvancePeriod map
		case -1:
			// *always* execute this!
			skip = false;
			break;
		case (int)UnitBase::WorkNone:
			if (advanceCount % 10 != 0) {
				skip = true;
			}
			break;
		case (int)UnitBase::WorkMove:
			skip = false;
			break;
		case (int)UnitBase::WorkAttack:
			if (advanceCount % 5 != 0) {
				skip = true;
			}
			break;
		case (int)UnitBase::WorkConstructed:
			if (advanceCount % 20 != 0) {
				skip = true;
			}
			break;
		case (int)UnitBase::WorkDestroyed:
			skip = false;
			break;
		case (int)UnitBase::WorkFollow:
			if (advanceCount % 5 != 0) {
				skip = true;
			}
			break;
		case (int)UnitBase::WorkPlugin:
			skip = false;
			break;
		case (int)UnitBase::WorkTurn:
			skip = false;
			break;
		default:
			// shouldn't happen! (TODO: add a warning! havent done
			// so in favor of testing)
			skip = false;
			break;
	}
	if (skip) {
		continue;
	}
	QPtrListIterator<BosonItem> itemIt(*it);
	for (; itemIt.current(); ++itemIt) {
		unsigned int id;
		int work;
		BosonItem* s = itemIt.current();
		if (RTTI::isUnit(s->rtti())) {
			id = ((Unit*)s)->id();
			work = (int)((Unit*)s)->advanceWork();
		} else {
			id = 0;
			work = -1;
		}
#if DO_ITEM_PROFILING
		boProfiling->advanceItemStart(s->rtti(), id, work);
		boProfiling->advanceItemFunction(true);
#endif
		if (advanceFlag) { // bah - inside the loop..
			s->advanceFunction(advanceCount); // once this was called this object is allowed to change its advanceFunction()
		} else {
			s->advanceFunction2(advanceCount); // once this was called this object is allowed to change its advanceFunction()
		}
#if DO_ITEM_PROFILING
		boProfiling->advanceItemFunction(false);
#endif

		// AB: moveBy() is *NOT* called if advanceFunction() isn't
		// called for an item!!
		// --> i.e. if it isn't in one of the lists that are executed
		// here
#if DO_ITEM_PROFILING
		boProfiling->advanceItemMove(true);
#endif
		if (s->xVelocity() || s->yVelocity() || s->zVelocity()) {
			s->moveBy(s->xVelocity(), s->yVelocity(), s->zVelocity());
		}
#if DO_ITEM_PROFILING
		boProfiling->advanceItemMove(false);
		boProfiling->advanceItemStop();
#endif
	}
 }

 // now we need to make sure that the correct advance function will be called in
 // the next advance call.
 animIt.toFirst();
 if (advanceFlag) {
	for (; animIt.current(); ++animIt) {
		BosonItem* i = animIt.current();
		i->syncAdvanceFunction();
	}
 } else {
	for (; animIt.current(); ++animIt) {
		BosonItem* i = animIt.current();
		i->syncAdvanceFunction2();
	}
 }

 // we completed iterating through the advance lists. now we might have to
 // change the list for some items.
 QPtrListIterator<BosonItem> changeIt(d->mChangeAdvanceList);
 for (; changeIt.current(); ++changeIt) {
	BosonItem* item = changeIt.current();
	removeFromAdvanceLists(item); // AB: this will probably take too much time :(
	if (!RTTI::isUnit(item->rtti())) {
		// oops - this should not (yet?) happen!
		// --> append to default list
		d->mWork2AdvanceList[-1].append(item);
		continue;
	}
	Unit* unit = (Unit*)item;
	d->mWork2AdvanceList[unit->advanceWork()].append(item);
 }
 d->mChangeAdvanceList.clear();

#else // USE_ADVANCE_LISTS
 if (advanceFlag) {
	// note: the advance methods must not change the advanceFunction()s
	// here!
	// AB: do NOT add something here - if you add something for units then
	// check for isDestroyed() !!
	while (animIt.current()) {
		unsigned int id;
		int work;
		BosonItem* s = animIt.current();
		if (RTTI::isUnit(s->rtti())) {
			id = ((Unit*)s)->id();
			work = (int)((Unit*)s)->advanceWork();
		} else {
			id = 0;
			work = -1;
		}
#if DO_ITEM_PROFILING
		boProfiling->advanceItemStart(s->rtti(), id, work);
		boProfiling->advanceItem(true);
#endif

		// TODO: group some stuff.
		// e.g. we reload weapons and shields here whenever we call
		// this. instead we could reaload every 5th or 10th call only,
		// but then by 5 or 10 instead of 1. the units would reload with
		// the same speed (except for up to 4 or 9 advance calls - but
		// that doesnt matter anyway) and this function would execute
		// faster.
		s->advance(advanceCount);
#if DO_ITEM_PROFILING
		boProfiling->advanceItem(false);
		boProfiling->advanceItemFunction(true);
#endif
		s->advanceFunction(advanceCount); // once this was called this object is allowed to change its advanceFunction()
#if DO_ITEM_PROFILING
		boProfiling->advanceItemFunction(false);
#endif

		// AB: warning: this might cause trouble at this point! see Unit::moveBy()
#if DO_ITEM_PROFILING
		boProfiling->advanceItemMove(true);
#endif
		if (s->xVelocity() || s->yVelocity() || s->zVelocity()) {
			s->moveBy(s->xVelocity(), s->yVelocity(), s->zVelocity());
		}
#if DO_ITEM_PROFILING
		boProfiling->advanceItemMove(false);
		boProfiling->advanceItemStop();
#endif
		++animIt;
	}
 } else {
	// note: the advance methods must not change the advanceFunction2()s
	// here!
	while (animIt.current()) {
		unsigned int id;
		int work;
		BosonItem* s = animIt.current();
		if (RTTI::isUnit(s->rtti())) {
			id = ((Unit*)s)->id();
			work = (int)((Unit*)s)->work();
		} else {
			id = 0;
			work = 0;
		}
#if DO_ITEM_PROFILING
		boProfiling->advanceItemStart(s->rtti(), id, work);
		boProfiling->advanceItem(true);
#endif
		s->advance(advanceCount);
#if DO_ITEM_PROFILING
		boProfiling->advanceItem(false);
		boProfiling->advanceItemFunction(true);
#endif
		s->advanceFunction2(advanceCount); // once this was called this object is allowed to change its advanceFunction2()
#if DO_ITEM_PROFILING
		boProfiling->advanceItemFunction(false);
#endif

		// AB: warning: this might cause trouble at this point! see Unit::moveBy()
#if DO_ITEM_PROFILING
		boProfiling->advanceItemMove(true);
#endif
		if (s->xVelocity() || s->yVelocity() || s->zVelocity()) {
			s->moveBy(s->xVelocity(), s->yVelocity(), s->zVelocity());
		}
#if DO_ITEM_PROFILING
		boProfiling->advanceItemMove(false);
		boProfiling->advanceItemStop();
#endif
		++animIt;
	}
 }
#endif
 boProfiling->advanceFunction(false);
 unlockAdvanceFunction();

 boProfiling->advanceParticles(true);
 updateParticleSystems(0.05);  // With default game speed, delay between advance messages is 1.0 / 20 = 0.05 sec
 boProfiling->advanceParticles(false);

 boProfiling->advanceMaximalAdvanceCount(true);
 if (advanceCount == MAXIMAL_ADVANCE_COUNT) {
	boDebug(300) << "MAXIMAL_ADVANCE_COUNT" << endl;
	// there are 2 different timers for deletion of canvas items.
	// The first is done in BosonCanvas - we only delete anything when
	// advanceCount == MAXIMAL_ADVANCE_COUNT.
	// The second is unit based. every MAXIMAL_ADVANCE_COUNT advance calls
	// we increase the deletion timer of the unit and delete it when
	// REMOVE_WRECKAGES_TIME is reached. This way we don't see all wreckages
	// diappear at once...
	QPtrListIterator<Unit> deletionIt(d->mDestroyedUnits);
	QPtrList<Unit> deleteList;
	while (deletionIt.current()) {
		deletionIt.current()->increaseDeletionTimer();
		if (deletionIt.current()->deletionTimer() >= REMOVE_WRECKAGES_TIME) {
			deleteList.append(deletionIt.current());
			d->mDestroyedUnits.removeRef(deletionIt.current());
		}
		++deletionIt;
	}

	deleteUnits(&deleteList);

	boProfiling->advanceDeleteUnusedShots(true);
	deleteUnusedShots();
	boProfiling->advanceDeleteUnusedShots(false);
 }
 boProfiling->advanceMaximalAdvanceCount(false);
 boProfiling->advance(false, advanceCount);
}

bool BosonCanvas::canGo(const UnitProperties* prop, const QRect& rect) const
{
// boDebug() << k_funcinfo << endl;
 if (rect.x() < 0 || rect.y() < 0 ||
		rect.x() + rect.width() >= (int)mapWidth() * BO_TILE_SIZE ||
		rect.y() + rect.height() >= (int)mapHeight() * BO_TILE_SIZE) {
	return false;
 }
 int y = rect.y() / BO_TILE_SIZE; // what about modulu? do we care ?
 do {
	int x = rect.x() / BO_TILE_SIZE;
	do {
		Cell* newCell = cell(x, y);
		if (!newCell) {
			boError() << k_funcinfo << "NULL cell" << endl;
			return false;
		}
		if (!newCell->canGo(prop)) {
			boDebug() << k_funcinfo << "can not go on " << x << "," << y << endl;
			return false;
		} else {
//			boDebug() << k_funcinfo << "can go on " << x << "," << y << endl;
		}
		x++;
	} while (x * BO_TILE_SIZE < rect.right());
	y++;
 } while (y * BO_TILE_SIZE < rect.bottom());

 return true;
}

void BosonCanvas::setMap(BosonMap* map)
{
 d->mMap = map;
 collisions()->setMap(map);
}

void BosonCanvas::addAnimation(BosonItem* item)
{
 if (!d->mAnimList.contains(item)) {
	d->mAnimList.append(item);

	// by default it goes to "work" == -1. units will change this.
	d->mWork2AdvanceList[-1].append(item);
 }
}

void BosonCanvas::removeAnimation(BosonItem* item)
{
 d->mAnimList.removeRef(item);

 // remove from all advance lists
 QMap<int, QPtrList<BosonItem> >::Iterator it;
 for (it = d->mWork2AdvanceList.begin(); it != d->mWork2AdvanceList.end(); ++it) {
	(*it).removeRef(item);
 }
 d->mChangeAdvanceList.removeRef(item);
}

void BosonCanvas::unitMoved(Unit* unit, float oldX, float oldY)
{
 updateSight(unit, oldX, oldY);
 
// test if any unit has this unit as target. If sou then adjust the destination.
//TODO

// used to adjust the mini map
 emit signalUnitMoved(unit, oldX, oldY);
}

void BosonCanvas::updateSight(Unit* unit, float , float)
{
// TODO: use the float parameters - check whether the player can still see
// these coordinates and if not out fog on them again. Remember to check for -1
// (new unit placed)!

 unsigned int sight = unit->sightRange(); // *cell* number! not pixel number!
 unsigned int x = unit->boundingRect().center().x() / BO_TILE_SIZE;
 unsigned int y = unit->boundingRect().center().y() / BO_TILE_SIZE;

 int left = ((x > sight) ? (x - sight) : 0) - x;
 int top = ((y > sight) ? (y - sight) : 0) - y;
 int right = ((x + sight > d->mMap->width()) ?  d->mMap->width() :
		x + sight) - x;
 int bottom = ((y + sight > d->mMap->height()) ?  d->mMap->height() :
		y + sight) - y;
 
 sight *= sight;
// boDebug() << k_funcinfo << endl;
// boDebug() << "left=" << left << ",right=" << right << endl;
// boDebug() << "top=" << top << ",bottom=" << bottom << endl;

 for (int i = left; i < right; i++) {
	for (int j = top; j < bottom; j++) {
		if (i*i + j*j < (int)sight) {
			if (unit->owner()->isFogged(x + i, y + j)) {
				unit->owner()->unfog(x + i, y + j);
			}
		} else {
			//TODO
			// cell(i, j) is not in sight anymore. Check if any
			// other unit can see it!
			// if (we_cannot_see_this) {
			//	unit->owner()->fog(x + i, y + j);
			// }
		}
	}
 }
}

void BosonCanvas::newShot(BosonShot*)
{
 boDebug(350) << k_funcinfo << endl;
}

void BosonCanvas::shotHit(BosonShot* s)
{
 if (!s) {
	boError() << k_funcinfo << "NULL shot" << endl;
	return;
 }
 // Set age of flying particle systems (e.g. smoke traces) to 0 so they won't create any new particles
 if (s->particleSystems() && s->particleSystems()->count() > 0) {
	QPtrListIterator<BosonParticleSystem> it(*(s->particleSystems()));
	while (it.current()) {
		it.current()->setAge(0);
		++it;
	}
 }
 if (s->properties()) {
	// Add hit particle systems
	addParticleSystems(s->properties()->newHitParticleSystems(BoVector3(s->x(), s->y(), s->z())));

	// Play hit sound
	s->properties()->playSound(SoundWeaponHit);
 }

 explosion(BoVector3(s->x(), s->y(), s->z()), s->damage(), s->damageRange(),
		s->fullDamageRange(), s->owner());
}

void BosonCanvas::explosion(const BoVector3& pos, long int damage, float range, float fullrange, Player* owner)
{
 // Decrease health of all units within damaging range of explosion
 float r = QMAX(0, range * BO_TILE_SIZE - 1);  // - 1 is needed to prevent units on next cells from also being damaged
 float fr = QMAX(0, fullrange * BO_TILE_SIZE - 1);
 long int d;
 float dist;
 QValueList<Unit*> l = collisions()->unitCollisionsInSphere(pos, (int)r);
 for (unsigned int i = 0; i < l.count(); i++) {
	dist = l[i]->distance(pos);
	if (dist <= fr * fr || r == fr) {
		d = damage;
	} else {
		d = (long int)((1 - (sqrt(dist) - fr) / (r - fr)) * damage);
	}
	unitDamaged(l[i], d);
	if (l[i]->isDestroyed() && owner) {
		if (l[i]->isFacility()) {
			owner->statistics()->addDestroyedFacility(l[i], owner);
		} else {
			owner->statistics()->addDestroyedMobileUnit(l[i], owner);
		}
	}
 }
}

void BosonCanvas::unitDamaged(Unit* unit, long int damage)
{
 // Shield
 if (unit->shields() > 0) {
	if (unit->shields() >= (unsigned long int)damage) {
		// Unit will not be damaged (it has enough shields)
		unit->setShields(unit->shields() - damage);
		// TODO: show some shield animation
		return;
	} else {
		damage -= unit->shields();
		unit->setShields(0);
		// Also show shield animation?
	}
 }

 if (damage < 0) {
	unit->setHealth(unit->health() + ((unsigned long)-damage));
 } else {
	// Usually, unit's armor is substracted from attacker's weaponDamage, but
	//  if target has only little health left, then armor doesn't have full effect
	int health = (int)unit->health();
	if (health <= (int)(unit->unitProperties()->health() / 10.0)) {
		// If unit has only 10% or less of it's hitpoint left, armor has no effect (it's probably destroyed)
	} else if (health <= (int)(unit->unitProperties()->health() / 2.5)) {
		// Unit has 40% or less of hitpoints left. Only half of armor is "working"
		damage -= (int)(unit->armor() / 2.0);
	} else {
		damage -= unit->armor();
	}
	if (damage < 0) {
		damage = 0;
	}
	health -= damage;
	unit->setHealth((health >= 0) ? health : 0);
 }
 
 if (unit->isDestroyed()) {
	destroyUnit(unit); // display the explosion ; not the shoot
 } else {
/*	float factor = 2.0 - unit->health() / (unit->unitProperties()->health() / 2.0);
//	if (unit->health() <= (unit->unitProperties()->health() / 2.0)) {
	if (factor >= 1.0) {
		// If unit has less than 50% hitpoints, it's smoking
		BoVector3 pos((unit->x() + unit->width() / 2) * BO_GL_CELL_SIZE / (float)BO_TILE_SIZE,
				-((unit->y() + unit->height() / 2) * BO_GL_CELL_SIZE / (float)BO_TILE_SIZE),
				unit->z() * BO_GL_CELL_SIZE / (float)BO_TILE_SIZE);
		BosonParticleSystem* s;
		if (!unit->smokeParticleSystem()) {
			s = BosonParticleManager::newSmallSmoke(pos);
			unit->setSmokeParticleSystem(s);
			d->mParticles.append(s);
		}
		s = unit->smokeParticleSystem();
		// FIXME: maybe move this to BosonParticleManager?
		s->setCreateRate(factor * 25);
//		s->setVelocity(BoVector3(0, 0, factor * 0.5));  // This is only hint for BosonParticleManager
		float c = 0.8 - factor * 0.4;
		s->setColor(BoVector4(c, c, c, 0.25));

		// Facilities are burning too
		if (unit->isFacility()) {
			if (!((Facility*)unit)->flamesParticleSystem()) {
				s = BosonParticleManager::newFire(pos);
				((Facility*)unit)->setFlamesParticleSystem(s);
				d->mParticles.append(s);
			}
			s = ((Facility*)unit)->flamesParticleSystem();
			// FIXME: maybe move this to BosonParticleManager?
			s->setCreateRate(factor * 30);
			s->setVelocity(BoVector3(0, 0, factor * 0.5));  // This is only hint for BosonParticleManager
		}
	} else {
		// If it has more hitpoints, it's not burning ;-)
		if (unit->isFacility()) {
			if (((Facility*)unit)->flamesParticleSystem()) {
				((Facility*)unit)->flamesParticleSystem()->setAge(0);
			}
		}
		if (unit->smokeParticleSystem()) {
			unit->smokeParticleSystem()->setAge(0);
		}
	}*/
 }
}

void BosonCanvas::destroyUnit(Unit* unit)
{
 // please note: you MUST NOT delete the unit here!!
 // we call it from advance() and items must not be deleted from there!
 if (!unit) {
	return;
 }
 if (!d->mDestroyedUnits.contains(unit)) {
	boDebug() << k_funcinfo << "destroy unit " << unit->id() << endl;
	Player* owner = unit->owner();
	d->mDestroyedUnits.append(unit);

	// This stops everything
	unit->stopAttacking();

	// Stop particle emitting for all systems this unit has
	if (unit->particleSystems() && unit->particleSystems()->count() > 0) {
		QPtrListIterator<BosonParticleSystem> it(*(unit->particleSystems()));
		for (; it.current(); ++it) {
			boDebug() << k_funcinfo << "Setting age to 0 for particle system" << it.current() << endl;
			it.current()->setAge(0);
		}
		unit->particleSystems()->clear();
	}

	// the unit is added to a list - now displayed as a wreckage only.
	removeUnit(unit);
	unit->playSound(SoundReportDestroyed);
	// Pos is center of unit
	BoVector3 pos(unit->x() + unit->width() / 2, unit->y() + unit->height() / 2, unit->z());
	//pos += unit->unitProperties()->hitPoint();
	// Add destroyed particle systems
	addParticleSystems(unit->unitProperties()->newDestroyedParticleSystems(pos[0], pos[1], pos[2]));
	// Make explosion if needed
	const UnitProperties* prop = unit->unitProperties();
	if (prop->explodingDamage() > 0) {
		BosonShotExplosion* e = (BosonShotExplosion*)createItem(RTTI::Shot, unit->owner(), BosonShot::Explosion, 0, 0);
		// Do we want ability to set fullDamageRange here?
		if (e) {
			e->activate(pos, prop->explodingDamage(), prop->explodingDamageRange(), 0.0f, 10);
		}
	}
	// Add explosion fragments
	for (unsigned int i = 0; i < unit->unitProperties()->explodingFragmentCount(); i++) {
		BosonShotFragment* f = (BosonShotFragment*)createItem(RTTI::Shot, unit->owner(), BosonShot::Fragment, 0, 0);
		if (f) {
			f->activate(pos, unit->unitProperties());
		}
	}
	// Hide unit if wreckage should be removed immediately
	if (unit->unitProperties()->removeWreckageImmediately()) {
		unit->setVisible(false);
	}
	// Check if owner is out of game
	if (owner->checkOutOfGame()) {
		boGame->killPlayer(owner);
	}
 }
}

void BosonCanvas::removeUnit(Unit* unit)
{
 // please note: you MUST NOT delete the unit here!!
 // we call it from advance() and items must not be deleted from there!
 if (!unit) {
	return;
 }
 Player* owner = unit->owner();
 //unit->setAnimated(false);
 unit->setHealth(0); // in case of an accidental change before
 unit->setWork(UnitBase::WorkDestroyed);
 owner->unitDestroyed(unit); // remove from player without deleting
 emit signalUnitRemoved(unit);

 // note: we don't add unit to any list and we don't delete it here.
 // editor will now delete it, while game mustn't delete it (displays wreckage)
}

Cell* BosonCanvas::cellAt(Unit* unit) const
{
 if (!unit) {
	return 0;
 }
 return cellAt(unit->x() + unit->width() / 2, unit->y() + unit->width() / 2);
}

Cell* BosonCanvas::cellAt(float x, float y) const
{
 return cell((int)(x / BO_TILE_SIZE), (int)(y / BO_TILE_SIZE));
}

BosonMap* BosonCanvas::map() const
{
 return d->mMap;
}

unsigned int BosonCanvas::mapWidth() const
{
 return map() ? map()->width() : 0;
}

unsigned int BosonCanvas::mapHeight() const
{
 return map() ? map()->height() : 0;
}

float* BosonCanvas::heightMap() const
{
 return map() ? map()->heightMap() : 0;
}

void BosonCanvas::setHeightAtCorner(int x, int y, float height)
{
 BO_CHECK_NULL_RET(map());
 map()->setHeightAtCorner(x, y, height);
}

float BosonCanvas::heightAtCorner(int x, int y) const
{
 if (!map()) {
	BO_NULL_ERROR(map());
	return 1.0f;
 }
 return map()->heightAtCorner(x, y);
}

float BosonCanvas::heightAtPoint(float x, float y) const
{
 // Coordinates of the cell (x; y) is on
 int cellX = (int)(x / BO_TILE_SIZE);
 int cellY = (int)(y / BO_TILE_SIZE);

 // Will be used as factors for blending
 float x2 = (x / BO_TILE_SIZE) - cellX;
 float y2 = (y / BO_TILE_SIZE) - cellY;

 // These are heights of the corners of the cell (x; y) is on
 float h1, h2, h3, h4;

 h1 = heightAtCorner(cellX, cellY);
 h2 = heightAtCorner(cellX + 1, cellY);
 h3 = heightAtCorner(cellX, cellY + 1);
 h4 = heightAtCorner(cellX + 1, cellY + 1);

 // Blend all corners together and return the result
 // FIXME: this can probably be written _much_ more understandably and maybe faster
 return ((h1 * (1 - x2)) + (h2 * x2) * (1 - y2)) + ((h3 * (1 - x2)) + (h4 * x2) * y2);
}

void BosonCanvas::removeFromCells(BosonItem* item)
{
 const QPtrVector<Cell>* cells = item->cells();
 for (unsigned int i = 0; i < cells->count(); i++) {
	Cell* c = cells->at(i);
	if (!c) {
		boError() << k_funcinfo << "NULL cell at " << i << endl;
		continue;
	}
	c->removeItem(item);
 }
}

void BosonCanvas::addToCells(BosonItem* item)
{
 const QPtrVector<Cell>* cells = item->cells();
 for (unsigned int i = 0; i < cells->count(); i++) {
	Cell* c = cells->at(i);
	if (!c) {
		boError() << k_funcinfo << "NULL cell at " << i << endl;
		continue;
	}
	c->addItem(item);
 }
}

bool BosonCanvas::canPlaceUnitAtCell(const UnitProperties* prop, const QPoint& pos, ProductionPlugin* factory) const
{
 int width = prop->unitWidth();
 int height = prop->unitHeight();
 if (width == 0) {
	boError() << k_funcinfo << "null width for " << prop->typeId() << endl;
	return false;
 }
 if (height == 0) {
	boError() << k_funcinfo << "null height for " << prop->typeId() << endl;
	return false;
 }
 if (!onCanvas(pos * BO_TILE_SIZE)) {
	return false;
 }
 QRect r(pos.x() * BO_TILE_SIZE, pos.y() * BO_TILE_SIZE, width, height);
 if (!canGo(prop, r)) {
	return false;
 }
 if (collisions()->cellsOccupied(r)) {
	return false;
 }
 if (!factory) {
	return true;
 }
 if (prop->isMobile()) {
	// must be in BUILD_RANGE of factory
	// not perfect - there is alays a distance from center() to the edge of
	// both units which should also added to this. but this is not a maths
	// contest, so its ok this way
	Unit* factoryUnit = factory->unit();
	if (!factoryUnit) {
		boError() << k_funcinfo << "production plugin has NULL owner" << endl;
		return false;
	}
	int dx = QABS(r.center().x() - factoryUnit->boundingRect().center().x());
	int dy = QABS(r.center().y() - factoryUnit->boundingRect().center().y());
	if (dx * dx + dy * dy <= BUILD_RANGE * BUILD_RANGE) {
		return true;
	}
 } else {
	// must be in BUILD_RANGE of any facility of the player
	QValueList<Unit*> list = collisions()->unitCollisionsInRange(r.center(), BUILD_RANGE);
	for (unsigned int i = 0; i < list.count(); i++) {
		if (list[i]->isFacility() && list[i]->owner() == factory->player()) {
			return true;
		}
	}
 }
 return false;
}

BoItemList* BosonCanvas::allItems() const
{
 return &d->mAllItems;
}

unsigned int BosonCanvas::allItemsCount() const
{
 return d->mAllItems.count();
}

void BosonCanvas::addItem(BosonItem* item)
{
 d->mAllItems.append(item);
}

void BosonCanvas::removeItem(BosonItem* item)
{
 d->mAllItems.remove(item);
 emit signalRemovedItem(item);
}

unsigned int BosonCanvas::particleSystemsCount() const
{
 return particleSystems()->count();
}

QPtrList<BosonParticleSystem>* BosonCanvas::particleSystems() const
{
 return &(d->mParticles);
}

void BosonCanvas::updateParticleSystems(float elapsed)
{
/* int count = d->mParticles.count();
 if (count <= 0) {
	return;
 }
	BosonParticleSystem* s;
	for (int i = 0; i < count; i++) {
	s = d->mParticles.at(i);
	s->update(elapsed);
	if (!s->isActive()) {
		boDebug() << k_funcinfo << "**********  REMOVING inactive particle system (particle count: " << s->particleCount() << ")!  *****" << endl;
		d->mParticles.remove();
		i--;
		count--;
	}
 }*/
 for (BosonParticleSystem* s = d->mParticles.first(); s; s = d->mParticles.next()) {
	s->update(elapsed);
	if (!s->isActive()) {
		d->mParticles.removeRef(s);
	}
 }
}

void BosonCanvas::deleteUnusedShots()
{
 for (BosonItem* i = d->mAnimList.first(); i; i = d->mAnimList.next()) {
	if (RTTI::isShot(i->rtti())) {
		BosonShot* shot = (BosonShot*)i;
		if (!shot->isActive()) {
			delete i;
		}
	}
 }
}

void BosonCanvas::addParticleSystem(BosonParticleSystem* s)
{
 d->mParticles.append(s);
}

void BosonCanvas::addParticleSystems(const QPtrList<BosonParticleSystem> systems)
{
 QPtrListIterator<BosonParticleSystem> it(systems);
 for (; it.current(); ++it) {
	addParticleSystem(it.current());
 }
}

unsigned int BosonCanvas::animationsCount() const
{
 return d->mAnimList.count();
}

bool BosonCanvas::loadFromXML(const QDomElement& root)
{
 if (root.isNull()) {
	boError(260) << k_funcinfo << "NULL root node" << endl;
	return false;
 }


 QDomNodeList list = root.elementsByTagName(QString::fromLatin1("Items"));
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement items = list.item(i).toElement();
	if (items.isNull()) {
		boError(260) << k_funcinfo << "Items tag is not an element" << endl;
		continue;
	}
	bool ok = false;
	unsigned int ownerId = items.attribute(QString::fromLatin1("OwnerId")).toUInt(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "OwnerId of Items Tag " << i << " is not a valid number" << endl;
		continue;
	}
	Player* owner = (Player*)boGame->findPlayer(ownerId);
	if (!owner) {
		boError(260) << k_funcinfo << "player with Id=" << ownerId << " can not be found" << endl;
		continue;
	}

	// FIXME: the unit target must be loaded after _all_ units are loaded
	QDomNodeList itemList = items.elementsByTagName(QString::fromLatin1("Item"));
	for (unsigned int j = 0; j < itemList.count(); j++) {
		QDomElement item = itemList.item(j).toElement();
		if (item.isNull()) {
			continue;
		}
		if (!loadItemFromXML(item, owner)) {
			boError(260) << k_funcinfo << "Item " << j << " was not loaded correctly" << endl;
			continue;
		}
	}
 }
 return true;
}

bool BosonCanvas::loadItemFromXML(const QDomElement& item, Player* owner)
{
 if (item.isNull()) {
	return false;
 }
 if (!owner) {
	return false;
 }
 bool ok = false;
 int rtti = item.attribute(QString::fromLatin1("Rtti")).toInt(&ok);
 if (!ok) {
	boError(260) << k_funcinfo << "Rtti attribute of Item is not a valid number" << endl;
	return false;
 }

 unsigned long int type = 0;
 unsigned long int group = 0;
 unsigned long int groupType = 0;

 if (!item.hasAttribute(QString::fromLatin1("Type"))) {
	// check for deprecated attributes
	if (!item.hasAttribute(QString::fromLatin1("UnitType"))) {
		boError(260) << k_funcinfo << "missing attribute: Type for Item tag" << endl;
		return false;
	} else {
		type = item.attribute(QString::fromLatin1("UnitType")).toULong(&ok);
	}
 } else {
	type = item.attribute(QString::fromLatin1("Type")).toULong(&ok);
 }
 if (!ok) {
	boError(260) << k_funcinfo << "Invalid Type number for Item tag" << endl;
	return false;
 }

 if (item.hasAttribute(QString::fromLatin1("Group"))) {
	group = item.attribute(QString::fromLatin1("Group")).toULong(&ok);
 } else {
	// check for deprecated attributes.
	if (item.hasAttribute(QString::fromLatin1("UnitType")) && item.hasAttribute(QString::fromLatin1("Type"))) {
		// old Shot tags used "Type" for the type and "UnitType" for the
		// "Group" attribute.
		group = item.attribute(QString::fromLatin1("UnitType")).toULong(&ok);
	} else {
		// "Group" attribute is not present.
		// this is totally valid! items don't have to provide a Group
		// attribute.
		ok = true;
	}
 }
 if (!ok) {
	boError(260) << k_funcinfo << "Invalid Group number for Item tag" << endl;
	return false;
 }

 if (item.hasAttribute(QString::fromLatin1("GroupType"))) {
	groupType = item.attribute(QString::fromLatin1("GroupType")).toULong(&ok);
 } else {
	// check for deprecated attributes.
	if (item.hasAttribute(QString::fromLatin1("WeaponType"))) {
		groupType = item.attribute(QString::fromLatin1("WeaponType")).toULong(&ok);
	}
 }
 if (!ok) {
	boError(260) << k_funcinfo << "Invalid GroupType number for Item tag" << endl;
	return false;
 }


 if (RTTI::isUnit(rtti)) {
	if (!item.hasAttribute(QString::fromLatin1("Id"))) {
		boError(260) << k_funcinfo << "missing attribute: Id for Item tag" << endl;
		return false;
	}
	if (!item.hasAttribute(QString::fromLatin1("DataHandlerId"))) {
		boError(260) << k_funcinfo << "missing attribute: DataHandlerId for Item tag" << endl;
		return false;
	}
	unsigned long int id = 0;
	int dataHandlerId = 0;

	id = item.attribute(QString::fromLatin1("Id")).toULong(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "Invalid Id number for Item tag" << endl;
		return false;
	}
	dataHandlerId = item.attribute(QString::fromLatin1("DataHandlerId")).toInt(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "Invalid DataHandlerId number for Item tag" << endl;
		return false;
	}

	// Create unit with Boson
	Unit* u = boGame->loadUnit(type, owner);

	// Set additional properties
	owner->addUnit(u, dataHandlerId);
	u->setId(id);

	// Call unit's loading methods
	if (!u->loadFromXML(item)) {
		boWarning(260) << k_funcinfo << "Could not load unit " << id << " correctly" << endl;
		// no need to return false
		// also it is dangerous now, as we already called addUnit()!
	}

	return true;
 } else if (RTTI::isShot(rtti)) {
	BosonShot* s = (BosonShot*)createItem(RTTI::Shot, owner, type, group, groupType);
	if (!s) {
		boError() << k_funcinfo << "Invalid shot - type=" << type << " group=" << group << " groupType=" << groupType << endl;
		return false;
	}
	if (!s->loadFromXML(item)) {
		boWarning(260) << k_funcinfo << "Could not load shot correctly" << endl;
		delete s;
		return false;
	}
	ok = true;
	return true;
 } else {
	boError(260) << k_funcinfo << "unknown Rtti " << rtti << endl;
	return false;
 }
 return false;
}

bool BosonCanvas::saveAsXML(QDomElement& root)
{
 boDebug() << k_funcinfo << endl;

 QDomDocument doc = root.ownerDocument();
 QMap<unsigned int, QDomElement> owner2Items;
 for (KPlayer* p = boGame->playerList()->first(); p; p = boGame->playerList()->next()) {
	QDomElement items = doc.createElement(QString::fromLatin1("Items"));
	items.setAttribute(QString::fromLatin1("OwnerId"), p->id());
	root.appendChild(items);
	owner2Items.insert(p->id(), items);
 }

 for (BosonItem* i = d->mAnimList.first(); i; i = d->mAnimList.next()) {
	QDomElement items;
	if (RTTI::isShot(i->rtti())) {
		BosonShot* s = (BosonShot*)i;
		if (!s->owner()) {
			BO_NULL_ERROR(s->owner());
			continue;
		}
		unsigned int id = s->owner()->id();
		items = owner2Items[id];
	} else if (RTTI::isUnit(i->rtti())) {
		Unit* u = (Unit*)i;
		if (!u->owner()) {
			BO_NULL_ERROR(u->owner());
			continue;
		}
		unsigned int id = u->owner()->id();
		items = owner2Items[id];
	}
	if (items.isNull()) {
		boError() << k_funcinfo << "no Items element found" << endl;
		continue;
	}
	QDomElement item = doc.createElement(QString::fromLatin1("Item"));
	if (RTTI::isShot(i->rtti())) {
		if (!((BosonShot*)i)->isActive()) {
			continue;
		}
	}
	if (!i->saveAsXML(item)) {
		boError() << k_funcinfo << "Could not save item " << i << endl;
		continue;
	}
	items.appendChild(item);
 }
 return true;
}

void BosonCanvas::changeAdvanceList(BosonItem* item)
{
 if (!d->mChangeAdvanceList.contains(item)) { // AB: this requires a complete search (I guess at least)! might be slow
	d->mChangeAdvanceList.append(item);
 }
}

void BosonCanvas::removeFromAdvanceLists(BosonItem* item)
{
 // this is slow :(
 // we need to iterator through all lists and all lists need to search for the
 // item. since all (except one) will fail finding the item they need to search
 // very long (remember: a search for a not-existing item usually takes long).
 // we cannot use an "oldWork" variable or so, as we can never depend 100% on
 // it. there may be several situations where it is unreliable (loading games,
 // changing advancWork() very often in one advance call, ...)
 //
 // possible solutions may be to add a QPtrDict which maps item->list. so
 // whenever we add an item to a list, we also add this item to that dict (and
 // of course remove when it when it gets removed from the list). then we could
 // get the correct list in constant time.
 QMap<int, QPtrList<BosonItem> >::Iterator it;
 for (it = d->mWork2AdvanceList.begin(); it != d->mWork2AdvanceList.end(); ++it) {
	(*it).removeRef(item);
 }
}

bool BosonCanvas::onCanvas(const BoVector3& pos) const
{
 return onCanvas((int)pos.x(), (int)pos.y());
}

void BosonCanvas::deleteUnits(QPtrList<Unit>* units)
{
 // this is the only place where a unit may be deleted. NEVER delete a unit
 // outside this method!

 // ensure that NO unit has one of the deleted units as target.
 QPtrListIterator<Unit> deleteIt(*units);
 BoItemList::Iterator it;
 for (it = d->mAllItems.begin(); it != d->mAllItems.end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	Unit* u = (Unit*)*it;
	if (!u->target()) {
		continue;
	}
	for (deleteIt.toFirst(); deleteIt.current(); ++deleteIt) {
		if (u->target() == deleteIt.current()) {
			u->setTarget(0);
		}
	}
 }
 while (units->count() > 0) {
	Unit* u = units->first();
	units->removeRef(u);
	delete u;
 }
}

BosonItem* BosonCanvas::createItem(int rtti, Player* owner, unsigned long int type, unsigned long int group, unsigned long int groupType)
{
 if (RTTI::isUnit(rtti)) {
	Q_UNUSED(group);
	Q_UNUSED(groupType);
	return (BosonItem*)createUnit(owner, type);
 } else if (RTTI::isShot(rtti)) {
	return (BosonItem*)createShot(owner, type, group, groupType);
 }
 return 0;
}

Unit* BosonCanvas::createUnit(Player* owner, unsigned long int unitType)
{
 BO_CHECK_NULL_RET0(owner);
 SpeciesTheme* theme = owner->speciesTheme();
 BO_CHECK_NULL_RET0(theme); // BAAAAD - will crash

 const UnitProperties* prop = theme->unitProperties(unitType);
 if (!prop) {
	boError() << k_funcinfo << "Unknown unitType " << unitType << endl;
	return 0;
 }

 Unit* unit = 0;
 if (prop->isMobile()) {
	unit = new MobileUnit(prop, owner, this);
 } else if (prop->isFacility()) {
	unit = new Facility(prop, owner, this);
 } else { // should be impossible
	boError() << k_funcinfo << "invalid unit type " << unitType << endl;
	return 0;
 }
 owner->addUnit(unit); // can also be in Unit c'tor - is this clean?
 theme->loadNewUnit(unit);
 unit->setAnimationMode(UnitAnimationIdle);
 if (unit->isFlying()) {
//	unit->moveBy(0.0f, 0.0f, 2.0 * BO_TILE_SIZE / BO_GL_CELL_SIZE);
	unit->moveBy(0.0f, 0.0f, 2.0);
 }

 return unit;
}

BosonShot* BosonCanvas::createShot(Player* owner, unsigned long int shotType, unsigned long int unitType, unsigned long int weaponPropertyId)
{
 BO_CHECK_NULL_RET0(owner);
 BosonShot* s = 0;
 switch (shotType) {
	case BosonShot::Bullet:
	{
		SpeciesTheme* t = owner->speciesTheme();
		BO_CHECK_NULL_RET0(t);
		const UnitProperties* unitProperties = t->unitProperties(unitType);
		BO_CHECK_NULL_RET0(unitProperties);
		const BosonWeaponProperties* prop = unitProperties->weaponProperties(weaponPropertyId);
		BO_CHECK_NULL_RET0(prop);
		s = (BosonShot*)new BosonShotBullet(owner, this, prop);
		break;
	}
	case BosonShot::Missile:
	{
		SpeciesTheme* t = owner->speciesTheme();
		BO_CHECK_NULL_RET0(t);
		const UnitProperties* unitProperties = t->unitProperties(unitType);
		BO_CHECK_NULL_RET0(unitProperties);
		const BosonWeaponProperties* prop = unitProperties->weaponProperties(weaponPropertyId);
		BO_CHECK_NULL_RET0(prop);
		s = (BosonShot*)new BosonShotMissile(owner, this, prop);
		break;
	}
	case BosonShot::Explosion:
		s = (BosonShot*)new BosonShotExplosion(owner, this);
		break;
	case BosonShot::Mine:
	{
		SpeciesTheme* t = owner->speciesTheme();
		BO_CHECK_NULL_RET0(t);
		const UnitProperties* unitProperties = t->unitProperties(unitType);
		BO_CHECK_NULL_RET0(unitProperties);
		const BosonWeaponProperties* prop = unitProperties->weaponProperties(weaponPropertyId);
		BO_CHECK_NULL_RET0(prop);
		s = (BosonShot*)new BosonShotMine(owner, this, prop);
		break;
	}
	case BosonShot::Bomb:
	{
		SpeciesTheme* t = owner->speciesTheme();
		BO_CHECK_NULL_RET0(t);
		const UnitProperties* unitProperties = t->unitProperties(unitType);
		BO_CHECK_NULL_RET0(unitProperties);
		const BosonWeaponProperties* prop = unitProperties->weaponProperties(weaponPropertyId);
		BO_CHECK_NULL_RET0(prop);
		s = (BosonShot*)new BosonShotBomb(owner, this, prop);
		break;
	}
	case BosonShot::Fragment:
	{
		SpeciesTheme* t = owner->speciesTheme();
		BO_CHECK_NULL_RET0(t);
		BosonModel* model = t->objectModel("fragment");
		BO_CHECK_NULL_RET0(model);
		s = (BosonShot*)new BosonShotFragment(owner, this, model);
		break;
	}
	default:
		boError() << k_funcinfo << "Invalid type: " << shotType << endl;
		s = 0;
		break;
 }
 return s;
}


