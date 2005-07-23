/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "../bomemory/bodummymemory.h"
#include "player.h"
#include "cell.h"
#include "unit.h"
#include "unitplugins.h"
#include "bosonmap.h"
#include "unitproperties.h"
#include "speciestheme.h"
#include "boitemlist.h"
#include "bosoneffect.h"
#include "bosoneffectproperties.h"
#include "defines.h"
#include "items/bosonshot.h"
#include "items/bosonitemrenderer.h"
#include "bosonweapon.h"
#include "bosonstatistics.h"
#include "bosonprofiling.h"
#include "bosoncanvasstatistics.h"
#include "bodebug.h"
#include "boson.h"
#include "boevent.h"
#include "boeventlistener.h"
#include "bosonpropertyxml.h"
#include "bosonpath.h"
#include "bowater.h"
#include "bowaterrenderer.h"

#include <klocale.h>
#include <kgame/kgamepropertyhandler.h>

#include <qpointarray.h>
#include <qdatastream.h>
#include <qdom.h>

#include <math.h>

#include "bosoncanvas.moc"

ItemType ItemType::typeForUnit(unsigned long int unitType)
{
 return ItemType(unitType);
}
ItemType ItemType::typeForExplosion()
{
 return ItemType(BosonShot::Explosion, 0, 0);
}
ItemType ItemType::typeForFragment()
{
 return ItemType(BosonShot::Fragment, 0, 0);
}
ItemType ItemType::typeForShot(unsigned long int shotType, unsigned long int unitType, unsigned long int weaponPropertyId)
{
 return ItemType(shotType, unitType, weaponPropertyId);
}

class BosonCanvas::BosonCanvasPrivate
{
public:
	BosonCanvasPrivate()
		: mAllItems(BoItemList(0, false))
	{
		mStatistics = 0;
		mMap = 0;

		mProperties = 0;

		mPathfinder = 0;

		mEventListener = 0;
	}
	BosonCanvasStatistics* mStatistics;

	QPtrList<Unit> mDestroyedUnits;

	BosonMap* mMap; // just a pointer - no memory allocated

	QPtrList<BosonItem> mAnimList;

	BoItemList mAllItems;
	QPtrList<BosonEffect> mEffects;

	// by default ALL items are in "work" == -1. if an item changes its work
	// (i.e. it is a unit and it called setAdvanceWork()) then it will go to
	// another list (once slotAdvance() reaches its end)
	QMap<int, QPtrList<BosonItem> > mWork2AdvanceList;
	QPtrList<BosonItem> mChangeAdvanceList; // work has been changed - request to change advance list

	KGamePropertyHandler* mProperties;

	// AB: we _really_ need ulong here. for _very_ long games (maybe more
	// than a few days playing without break and a lot of shots) we might
	// even exceed that.
	// at the moment we won't be able to have so long games and won't have
	// _that_ big battles. in the future (if we maybe add even more items)
	// we might use unsigned long long for this.
	// but for now we are safe.
	KGameProperty<unsigned long int> mNextItemId;

	BosonPath* mPathfinder;

	BoCanvasEventListener* mEventListener;
};


/**
 * Actual implementation of BosonCanvas::slotAdvance(), i.e. the items are
 * advanced here.
 *
 * This deserves a dedicated class, because it is a very (the most?) central
 * aspect of boson. Profiling of what is done here is very important, which is
 * easier with a dedicated class.
 * BoCanvasAdvance is a friend of @ref BosonCanvas, so this is not a limitation.
 **/
class BoCanvasAdvance
{
public:
	BoCanvasAdvance(BosonCanvas* c)
	{
		mCanvas = c;
	}
	void advance(const BoItemList& allItems, const QPtrList<BosonItem>& animItems, unsigned int advanceCallsCount, bool advanceFlag);

protected:
	void advanceItems(const BoItemList& allItems, const QPtrList<BosonItem>& animItems, unsigned int advanceCallsCount, bool advanceFlag);
	void itemAdvance(const BoItemList& allItems, const QPtrList<BosonItem>& animItems, unsigned int advanceCallsCount); // calls BosonItem::advance()

	// AB: note that animItems is not used here currently. we use
	// mCanvas->d->mWork2AdvanceList.
	void advanceFunctionAndMove(unsigned int advanceCallsCount, bool advanceFlag);
	void syncAdvanceFunctions(const BoItemList& allItems, bool advanceFlag); // MUST be called after advanceFunction() stuff
	void updateWork2AdvanceList();
	void maximalAdvanceCountTasks(unsigned int advanceCallsCount); // "maximalAdvanceCount" is nonsense here, but the name has historic reasons
	void updateEffects(QPtrList<BosonEffect>& effects, float elapsed);
	void notifyAboutDestroyedUnits(const QPtrList<Unit>& destroyedUnits, unsigned int first, const BoItemList& allItems);

private:
	BosonCanvas* mCanvas;
};

void BoCanvasAdvance::advance(const BoItemList& allItems, const QPtrList<BosonItem>& animItems, unsigned int advanceCallsCount, bool advanceFlag)
{
 boProfiling->push(prof_funcinfo + " - Whole method");

 unsigned int initialDestroyedUnitsCount = mCanvas->d->mDestroyedUnits.count();

 /*
  * Main part of this method. This is the most important, but unfortunately also
  * the most time consuming part.
  */
 boProfiling->push("Advance Items");
 advanceItems(allItems, animItems, advanceCallsCount, advanceFlag);
 boProfiling->pop(); // Advance Items

 boProfiling->push("Advance Effects");
 updateEffects(mCanvas->d->mEffects, 0.05);  // With default game speed, delay between advance messages is 1.0 / 20 = 0.05 sec
 boProfiling->pop(); // Advance Effects

 boProfiling->push("Advance Water");
 // FIXME: rendering advancing should be done elsewhere (probably using a
 // signal)
 boWaterRenderer->update(0.05);
 boProfiling->pop(); // Advance Water

 // TODO: check when is it best to do this
 boProfiling->push("Advance Pathfinder");
 mCanvas->pathfinder()->advance();
 boProfiling->pop(); // Advance Pathfinder

 boProfiling->push("Notify About Destroyed Units");
 notifyAboutDestroyedUnits(mCanvas->d->mDestroyedUnits, initialDestroyedUnitsCount, allItems);
 boProfiling->pop(); // Advance Maximal Advance Count

 /*
  * This contains some things that need to be done "sometimes" only - currently
  * that is deletion of destroyed units and unused shots.
  *
  * These things are often very time consuming, but that hardly matters since it
  * is rarely done.
  */
 boProfiling->push("Advance Maximal Advance Count");
 maximalAdvanceCountTasks(advanceCallsCount);
 boProfiling->pop(); // Advance Maximal Advance Count

 boProfiling->pop(); // Whole method
}

void BoCanvasAdvance::advanceItems(const BoItemList& allItems, const QPtrList<BosonItem>& animItems, unsigned int advanceCallsCount, bool advanceFlag)
{
 mCanvas->lockAdvanceFunction();
 mCanvas->d->mStatistics->resetWorkCounts();

 // first we need to call *all* BosonItem::advance() functions.
 // AB: profiling information will be inaccurate because of this... we are
 // collecting for every item advance() here, and below for some items
 // advanceFunction(). those will be listed as *different* items...
 boProfiling->push("Advance: BosonItem::advance()");
 itemAdvance(allItems, animItems, advanceCallsCount);
 boProfiling->pop();

 // now the rest - mainly call BosonItem::advanceFunction().
 // this depends on in which list an item resides (changed when Unit::work()
 // changes). normal items are usually in -1.
 boProfiling->push("Advance: BosonItem::advanceFunction() and move()");
 advanceFunctionAndMove(advanceCallsCount, advanceFlag);
 boProfiling->pop();

 // now we need to make sure that the correct advance function will be called in
 // the next advance call.
 syncAdvanceFunctions(allItems, advanceFlag);

 // we completed iterating through the advance lists. now we might have to
 // change the list for some items.
 updateWork2AdvanceList();

 mCanvas->unlockAdvanceFunction();
}

// FIXME: name is completely wrong now. it doesn't advance anything, but does
// animate() and reload() only.
void BoCanvasAdvance::itemAdvance(const BoItemList& allItems, const QPtrList<BosonItem>& animItems, unsigned int advanceCallsCount)
{
 BosonProfiler profAnimateReload("Advance: BosonItem::animate() and reload() (in itemAdvance() - whole method)");

 BosonProfiler profAnimate("Advance: BosonItem::animate() (in itemAdvance())");
 QPtrListIterator<BosonItem> animIt(animItems);
 for (; animIt.current(); ++animIt) {
	animIt.current()->animate(advanceCallsCount);
 }
 profAnimate.pop();

 BosonProfiler profReload("Advance: BosonItem::reload() (in itemAdvance())");
 const unsigned int interval = 5;
 if (advanceCallsCount % interval == 0) {
	BoItemList::ConstIterator allIt;
	BoItemList::ConstIterator allItemsEnd = allItems.end();
	for (allIt = allItems.begin(); allIt != allItemsEnd; ++allIt) {
		(*allIt)->reload(interval);
	}
 }
 profReload.pop();
}

void BoCanvasAdvance::advanceFunctionAndMove(unsigned int advanceCallsCount, bool advanceFlag)
{
 PROFILE_METHOD;
 // FIXME: the mWork2AdvanceList map should be a parameter
 QMap<int, QPtrList<BosonItem> >::Iterator it;
 for (it = mCanvas->d->mWork2AdvanceList.begin(); it != mCanvas->d->mWork2AdvanceList.end(); ++it) {
	int work = it.key();
	bool skip = false;
	switch (work) {
		// TODO: instead of a big switch we should maintain a
		// d->mWork2AdvancePeriod map
		case -1:
			// *always* execute this!
			skip = false;
			break;
		case (int)UnitBase::WorkIdle:
			// TODO: maybe use some other work for circling aircrafts?
			skip = false;
			break;
		case (int)UnitBase::WorkNone:
			skip = true;
			break;
		case (int)UnitBase::WorkMove:
			skip = false;
			break;
		case (int)UnitBase::WorkAttack:
			if (advanceCallsCount % 5 != 0) {
				skip = true;
			}
			break;
		case (int)UnitBase::WorkConstructed:
			if (advanceCallsCount % 20 != 0) {
				skip = true;
			}
			break;
		case (int)UnitBase::WorkDestroyed:
			skip = false;
			break;
		case (int)UnitBase::WorkFollow:
			if (advanceCallsCount % 5 != 0) {
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

	mCanvas->d->mStatistics->increaseWorkCountBy(work, (*it).count());

	if (skip) {
		continue;
	}
	BosonProfiler profiler(QString("advanceFunction() and moveBy() for work==%1").arg(work));
//	boDebug() << "advancing " << (*it).count() << " items with advanceWork=" << work << endl;
	QPtrListIterator<BosonItem> itemIt(*it);
	for (; itemIt.current(); ++itemIt) {
		BosonItem* s = itemIt.current();
		if (advanceFlag) { // bah - inside the loop..
			s->advanceFunction(advanceCallsCount); // once this was called this object is allowed to change its advanceFunction()
		} else {
			s->advanceFunction2(advanceCallsCount); // once this was called this object is allowed to change its advanceFunction()
		}

		if (s->xVelocity() || s->yVelocity() || s->zVelocity()) {
			s->moveBy(s->xVelocity(), s->yVelocity(), s->zVelocity());
			s->setVelocity(0, 0, 0);
		}
	}
 }
}

void BoCanvasAdvance::syncAdvanceFunctions(const BoItemList& allItems, bool advanceFlag)
{
 BoItemList::ConstIterator it;
 BoItemList::ConstIterator allItemsEnd = allItems.end();
 if (advanceFlag) {
	for (it = allItems.begin(); it != allItemsEnd; ++it) {
		BosonItem* i = *it;
		i->syncAdvanceFunction();
	}
 } else {
	for (it = allItems.begin(); it != allItemsEnd; ++it) {
		BosonItem* i = *it;
		i->syncAdvanceFunction2();
	}
 }
}

void BoCanvasAdvance::updateWork2AdvanceList()
{
 QPtrListIterator<BosonItem> changeIt(mCanvas->d->mChangeAdvanceList);
 for (; changeIt.current(); ++changeIt) {
	BosonItem* item = changeIt.current();
	mCanvas->removeFromAdvanceLists(item); // AB: this will probably take too much time :(
	if (!RTTI::isUnit(item->rtti())) {
		// oops - this should not (yet?) happen!
		// --> append to default list
		mCanvas->d->mWork2AdvanceList[-1].append(item);
		continue;
	}
	Unit* unit = (Unit*)item;
	mCanvas->d->mWork2AdvanceList[unit->advanceWork()].append(item);
 }
 mCanvas->d->mChangeAdvanceList.clear();
}

void BoCanvasAdvance::maximalAdvanceCountTasks(unsigned int advanceCallsCount)
{
 BosonProfiler profiler("Advance: special MAXIMAL_ADVANCE_COUNT tasks"); // measure _all_ advanceCallsCounts

 const unsigned int MAXIMAL_ADVANCE_COUNT = 39;
 if (advanceCallsCount % MAXIMAL_ADVANCE_COUNT != 0) {
	return;
 }
 BosonProfiler profiler2("Advance MAXIMAL_ADVANCE_COUNT: all tasks");
 boDebug(300) << "MAXIMAL_ADVANCE_COUNT" << endl;
 QPtrListIterator<Unit> deletionIt(mCanvas->d->mDestroyedUnits);
 QPtrList<BosonItem> deleteList;
 boProfiling->push("Advance MAXIMAL_ADVANCE_COUNT: construction of item deletion list");
 while (deletionIt.current()) {
	deletionIt.current()->increaseDeletionTimer();
	if (deletionIt.current()->deletionTimer() >= REMOVE_WRECKAGES_TIME) {
		deleteList.append(deletionIt.current());
	}
	++deletionIt;
 }
 boProfiling->pop();

 boProfiling->push("Advance MAXIMAL_ADVANCE_COUNT: update destroyed list");
 QPtrListIterator<BosonItem> destroyedIt(deleteList);
 while (destroyedIt.current()) {
	mCanvas->d->mDestroyedUnits.removeRef((Unit*)destroyedIt.current());
	++destroyedIt;
 }
 boProfiling->pop();


 boProfiling->push("Advance MAXIMAL_ADVANCE_COUNT: deleting items");
 mCanvas->deleteItems(deleteList);
 boProfiling->pop();

 boProfiling->push("Advance MAXIMAL_ADVANCE_COUNT: deleteUnusedShots()");
 mCanvas->deleteUnusedShots();
 boProfiling->pop();
}

// AB: elapsed is unused
void BoCanvasAdvance::updateEffects(QPtrList<BosonEffect>& effects, float elapsed)
{
 PROFILE_METHOD;
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
 QPtrList<BosonEffect> removeEffects;
 for (BosonEffect* e = effects.first(); e; e = effects.next()) {
	if (!e->hasStarted()) {
		e->update(elapsed);
	} else {
		e->markUpdate(elapsed);
	}
	if (!e->isActive()) {
		if (e->ownerId()) {
			// Remove the effect from owner
			BosonItem* owner = mCanvas->d->mAllItems.findItem(e->ownerId());
			if (owner) {
				owner->removeEffect(e);
			}
		}
		removeEffects.append(e);
	}
 }
 while (removeEffects.count() > 0) {
	BosonEffect* e = removeEffects.take(0);
	effects.removeRef(e);
 }
}

void BoCanvasAdvance::notifyAboutDestroyedUnits(const QPtrList<Unit>& destroyedUnits, unsigned int first, const BoItemList& allItems)
{
 unsigned int i = 0;
 QPtrListIterator<Unit> destIt(destroyedUnits);
 while (destIt.current() && i < first) {
	++destIt;
 }

 // notify all units about the destroyed units in this advance call (i.e. from first
 // to end of list)
 while (destIt.current()) {
	Unit* destroyedUnit = destIt.current();
	for (BoItemList::const_iterator it = allItems.begin(); it != allItems.end(); ++it) {
		if (!RTTI::isUnit((*it)->rtti())) {
			continue;
		}
		Unit* u = (Unit*)*it;
		if (u == destroyedUnit) {
			continue;
		}
		u->unitDestroyed(destroyedUnit);
	}
	++destIt;
 }
}


BosonCanvas::BosonCanvas(QObject* parent)
		: QObject(parent, "BosonCanvas")
{
 init();
}

void BosonCanvas::init()
{
 d = new BosonCanvasPrivate;
 d->mDestroyedUnits.setAutoDelete(false);
 d->mEffects.setAutoDelete(true);
 mAdvanceFunctionLocked = false;
 mCollisions = new BosonCollisions();
 d->mStatistics = new BosonCanvasStatistics(this);
 d->mProperties = new KGamePropertyHandler(this);
 d->mNextItemId.registerData(IdNextItemId, d->mProperties,
		KGamePropertyBase::PolicyLocal, "NextItemId");
 d->mNextItemId.setLocal(0);
 BosonEffectPropertiesManager::initStatic();

 if (!boGame) {
	boError() << k_funcinfo << "NULL boGame object: cannot install event listener" << endl;
 } else {
	d->mEventListener = new BoCanvasEventListener(boGame->eventManager(), this);
	if (!d->mEventListener->initScript()) {
		boError() << k_funcinfo << "cannot init eventlistener script" << endl;
		return; // TODO: return false
	}
 }
}

BosonCanvas::~BosonCanvas()
{
 boDebug()<< k_funcinfo << endl;
 quitGame();
 delete d->mStatistics;
 delete mCollisions;
 BosonEffectPropertiesManager::deleteStatic();
 delete d;
 boDebug()<< k_funcinfo <<"done"<< endl;
}

BosonCanvasStatistics* BosonCanvas::canvasStatistics() const
{
 return d->mStatistics;
}

void BosonCanvas::quitGame()
{
 // Delete pathfinder first. Otherwise lot of time would be spent recalculating
 //  regions (when units are removed), which is totally unnecessary
 delete d->mPathfinder;
 d->mPathfinder = 0;
 deleteDestroyed();
 d->mEffects.clear();
 QMap<int, QPtrList<BosonItem> >::Iterator it;
 for (it = d->mWork2AdvanceList.begin(); it != d->mWork2AdvanceList.end(); ++it) {
	(*it).clear();
 }
 deleteItems(d->mAllItems);
 if (!d->mAnimList.isEmpty()) {
	boError() << k_funcinfo << "mAnimList is not empty!" << endl;
	d->mAnimList.clear();
 }
 if (!d->mAllItems.isEmpty()) {
	boError() << k_funcinfo << "mAllItems is not empty!" << endl;
 }
 d->mChangeAdvanceList.clear();
 d->mNextItemId = 0;
}

void BosonCanvas::deleteDestroyed()
{
 QPtrList<BosonItem> items;
 QPtrListIterator<Unit> it(d->mDestroyedUnits);
 while (it.current()) {
	items.append(it.current());
	++it;
 }
 deleteItems(items);
 d->mDestroyedUnits.clear();
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

void BosonCanvas::slotAdvance(unsigned int advanceCallsCount, bool advanceFlag)
{
 boProfiling->pushStorage("Advance");
 boProfiling->push("slotAdvance()");

 BoCanvasAdvance a(this);
 a.advance(d->mAllItems, d->mAnimList, advanceCallsCount, advanceFlag);

 boProfiling->pop();
 boProfiling->popStorage();
}

bool BosonCanvas::canGo(const UnitProperties* prop, const BoRectFixed& rect, bool _default) const
{
// boDebug() << k_funcinfo << endl;
 if (rect.left() < 0 || rect.top() < 0 ||
		rect.right() > mapWidth() ||
		rect.bottom() > mapHeight()) {
	return false;
 }
 int right = (int)ceil(rect.right());
 int bottom = (int)ceil(rect.bottom());
 int y = (int)rect.y(); // what about modulu? do we care ?
 do {
	int x = (int)rect.x();
	do {
		if (!canGo(prop, x, y, _default)) {
			return false;
		}
		x++;
	} while (x < right);
	y++;
 } while (y < bottom);

 return true;
}

bool BosonCanvas::canGo(const UnitProperties* prop, int x, int y, bool _default) const
{
 // TODO: check for fog (we'll probably need Player/PlayerIO for this)
 // TODO: move this to UnitProperties. The reason it's here ATM is that we need
 //  map's width
 if (prop->isAircraft()) {
	return true;
 } else if(prop->isFacility()) {
	// TODO: add MaxSlope and WaterDepth properties to facilities and take those into account
	return !cell(x, y)->isWater();
 }

 return prop->moveData()->cellPassable[y * mapWidth() + x];
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
 } else {
	boError() << k_funcinfo << item << " has been added before! id=" << item->id() << endl;
 }
}

void BosonCanvas::removeAnimation(BosonItem* item)
{
 bool ok = d->mAnimList.removeRef(item);
 if (!ok) {
	boError() << k_funcinfo << "could not remove animation for " << item << " == " << item->id() << " which is not in the list!" << endl;
 }
}

void BosonCanvas::unitMoved(Unit* unit, bofixed oldX, bofixed oldY)
{
 updateSight(unit, oldX, oldY);

// test if any unit has this unit as target. If sou then adjust the destination.
//TODO

// used to adjust the mini map
 emit signalUnitMoved(unit, oldX, oldY);
}

void BosonCanvas::updateSight(Unit* unit, bofixed, bofixed)
{
// TODO: use the bofixed parameters - check whether the player can still see
// these coordinates and if not out fog on them again. Remember to check for -1
// (new unit placed)!

 unsigned int sight = (int)unit->sightRange();
 unsigned int x = (unsigned int)unit->centerX();
 unsigned int y = (unsigned int)unit->centerY();

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
 // Make shot's effects (e.g. smoke traces) obsolete
 if (s->effects() && s->effects()->count() > 0) {
	QPtrListIterator<BosonEffect> it(*(s->effects()));
	while (it.current()) {
		it.current()->makeObsolete();
		++it;
	}
 }
 if (s->properties()) {
	// Add hit effects
	addEffects(s->properties()->newHitEffects(BoVector3Fixed(s->x(), s->y(), s->z())));

	// Play hit sound
	s->properties()->playSound(SoundWeaponHit);
 }

 explosion(BoVector3Fixed(s->x(), s->y(), s->z()), s->damage(), s->damageRange(),
		s->fullDamageRange(), s->owner());
}

void BosonCanvas::explosion(const BoVector3Fixed& pos, long int damage, bofixed range, bofixed fullrange, Player* owner)
{
 // Decrease health of all units within damaging range of explosion
 long int d;
 bofixed dist;
 QValueList<Unit*> l = collisions()->unitCollisionsInSphere(pos, range);
 for (unsigned int i = 0; i < l.count(); i++) {
	Unit* u = l[i];
	// We substract unit's size from actual distance
	bofixed unitsize = QMIN(u->width(), u->height()) / 2.0f;
	dist = QMAX(bofixed(sqrt(u->distance(pos)) - unitsize), bofixed(0));
	if (dist <= fullrange || range == fullrange) {
		d = damage;
	} else {
		d = (long int)((1 - (dist - fullrange) / (range - fullrange)) * damage);
	}
	unitDamaged(u, d);
	if (u->isDestroyed() && owner) {
		if (u->isFacility()) {
			owner->statistics()->addDestroyedFacility(u, owner);
		} else {
			owner->statistics()->addDestroyedMobileUnit(u, owner);
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
	if (health <= (int)(unit->maxHealth() / 10.0)) {
		// If unit has only 10% or less of it's hitpoint left, armor has no effect (it's probably destroyed)
	} else if (health <= (int)(unit->maxHealth() / 2.5)) {
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
/*	bofixed factor = 2.0 - unit->health() / (unit->unitProperties()->health() / 2.0);
//	if (unit->health() <= (unit->unitProperties()->health() / 2.0)) {
	if (factor >= 1.0) {
		// If unit has less than 50% hitpoints, it's smoking
		BoVector3Fixed pos((unit->x() + unit->width() / 2),
				-((unit->y() + unit->height() / 2)),
				unit->z());
		BosonParticleSystem* s;
		if (!unit->smokeParticleSystem()) {
			s = BosonParticleManager::newSmallSmoke(pos);
			unit->setSmokeParticleSystem(s);
			d->mParticles.append(s);
		}
		s = unit->smokeParticleSystem();
		// FIXME: maybe move this to BosonParticleManager?
		s->setCreateRate(factor * 25);
//		s->setVelocity(BoVector3Fixed(0, 0, factor * 0.5));  // This is only hint for BosonParticleManager
		bofixed c = 0.8 - factor * 0.4;
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
			s->setVelocity(BoVector3Fixed(0, 0, factor * 0.5));  // This is only hint for BosonParticleManager
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
//	boDebug() << k_funcinfo << "destroy unit " << unit->id() << endl;
	Player* owner = unit->owner();
	d->mDestroyedUnits.append(unit);

	// This stops everything
	unit->stopAttacking();

	// Make all unit's effects obsolete
	if (unit->effects() && unit->effects()->count() > 0) {
		QPtrListIterator<BosonEffect> it(*(unit->effects()));
		for (; it.current(); ++it) {
//			boDebug() << k_funcinfo << "Making effect " << it.current() << " obsolete" << endl;
			it.current()->makeObsolete();
			it.current()->setOwnerId(0);
		}
		unit->clearEffects();
	}

	// the unit is added to a list - now displayed as a wreckage only.
	removeUnit(unit);
	unit->playSound(SoundReportDestroyed);
	// Pos is center of unit
	BoVector3Fixed pos(unit->x() + unit->width() / 2, unit->y() + unit->height() / 2, unit->z());
	//pos += unit->unitProperties()->hitPoint();
	// Add destroyed effects
	addEffects(unit->unitProperties()->newDestroyedEffects(pos[0], pos[1], pos[2]));
	// Make explosion if needed
	const UnitProperties* prop = unit->unitProperties();
	if (prop->explodingDamage() > 0) {
		BosonShotExplosion* e = (BosonShotExplosion*)createNewItem(RTTI::Shot, unit->owner(), ItemType(BosonShot::Explosion, 0, 0), pos);
		// Do we want ability to set fullDamageRange here?
		if (e) {
			// AB: pos parameter is redundant due to createNewItem()
			// change
			e->activate(pos, prop->explodingDamage(), prop->explodingDamageRange(), 0.0f, 10);
		}
	}
	// Add explosion fragments
	for (unsigned int i = 0; i < unit->unitProperties()->explodingFragmentCount(); i++) {
		BosonShotFragment* f = (BosonShotFragment*)createNewItem(RTTI::Shot, unit->owner(), ItemType(BosonShot::Fragment, 0, 0), pos);
		if (f) {
			// AB: pos parameter is redundant due to createNewItem()
			// change
			f->activate(pos, unit->unitProperties());
		}
	}
	// Hide unit if wreckage should be removed immediately
	if (unit->unitProperties()->removeWreckageImmediately()) {
		unit->setVisible(false);
	}

	BoEvent* unitDestroyed = new BoEvent("UnitWithTypeDestroyed", QString::number(unit->type()));
	unitDestroyed->setUnitId(unit->id());
	unitDestroyed->setPlayerId(unit->owner()->id());
	boGame->queueEvent(unitDestroyed);

	// the following events are not emitted for the neutral player
	if (owner != boGame->playerList()->at(boGame->playerCount() - 1)) {
		if (owner->mobilesCount() == 0) {
			BoEvent* event = new BoEvent("AllMobileUnitsDestroyed");
			event->setPlayerId(unit->owner()->id());
			boGame->queueEvent(event);
		}
		if (owner->facilitiesCount() == 0) {
			BoEvent* allFacilitiesDestroyed = new BoEvent("AllFacilitiesDestroyed");
			allFacilitiesDestroyed->setPlayerId(unit->owner()->id());
			boGame->queueEvent(allFacilitiesDestroyed);
		}
		if (owner->allUnits()->count() == 0) {
			BoEvent* event = new BoEvent("AllUnitsDestroyed");
			event->setPlayerId(unit->owner()->id());
			boGame->queueEvent(event);
		}
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

Cell* BosonCanvas::cellAt(bofixed x, bofixed y) const
{
 return cell((int)(x), (int)(y));
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

const float* BosonCanvas::heightMap() const
{
 return map() ? map()->heightMap() : 0;
}

void BosonCanvas::setHeightAtCorner(int x, int y, float height)
{
 BO_CHECK_NULL_RET(map());
 map()->setHeightAtCorner(x, y, height);
}

void BosonCanvas::setHeightsAtCorners(const QValueList< QPair<QPoint, float> >& heights)
{
 BO_CHECK_NULL_RET(map());
 map()->setHeightsAtCorners(heights);
}

float BosonCanvas::heightAtCorner(int x, int y) const
{
 if (!map()) {
	BO_NULL_ERROR(map());
	return 1.0f;
 }
 return map()->heightAtCorner(x, y);
}

float BosonCanvas::waterDepthAtCorner(int x, int y) const
{
 if (!map()) {
	BO_NULL_ERROR(map());
	return 0.0f;
 }
 return map()->waterDepthAtCorner(x, y);
}

float BosonCanvas::heightAtPoint(bofixed x, bofixed y) const
{
 // Coordinates of the cell (x; y) is on
 int cellX = (int)(x);
 int cellY = (int)(y);

 if ((x == cellX) && (y == cellY)) {
	return heightAtCorner(cellX, cellY) + waterDepthAtCorner(cellX, cellY);
 } else if(x == cellX) {
	bofixed y2 = (y) - cellY;
	float h1, h2;
	h1 = heightAtCorner(cellX, cellY) + waterDepthAtCorner(cellX, cellY);
	h2 = heightAtCorner(cellX, cellY + 1) + waterDepthAtCorner(cellX, cellY + 1);
	return h1 * (1 - y2) + (h2 * y2);
 } else if(y == cellY) {
	bofixed x2 = (x) - cellX;
	float h1, h2;
	h1 = heightAtCorner(cellX, cellY) + waterDepthAtCorner(cellX, cellY);
	h2 = heightAtCorner(cellX + 1, cellY) + waterDepthAtCorner(cellX + 1, cellY);
	return h1 * (1 - x2) + (h2 * x2);
 }

 // Will be used as factors for blending
 bofixed x2 = (x) - cellX;
 bofixed y2 = (y) - cellY;

 // These are heights of the corners of the cell (x; y) is on
 float h1, h2, h3, h4;

 h1 = heightAtCorner(cellX, cellY) + waterDepthAtCorner(cellX, cellY);
 h2 = heightAtCorner(cellX + 1, cellY) + waterDepthAtCorner(cellX + 1, cellY);
 h3 = heightAtCorner(cellX, cellY + 1) + waterDepthAtCorner(cellX, cellY + 1);
 h4 = heightAtCorner(cellX + 1, cellY + 1) + waterDepthAtCorner(cellX + 1, cellY + 1);

 // Blend all corners together and return the result
 // FIXME: this can probably be written _much_ more understandably and maybe faster
 return ((h1 * (1 - x2) + (h2 * x2)) * (1 - y2)) + ((h3 * (1 - x2) + (h4 * x2)) * y2);
}

float BosonCanvas::terrainHeightAtPoint(bofixed x, bofixed y) const
{
 // Coordinates of the cell (x; y) is on
 int cellX = (int)(x);
 int cellY = (int)(y);

 if ((x == cellX) && (y == cellY)) {
	return heightAtCorner(cellX, cellY);
 } else if(x == cellX) {
	bofixed y2 = (y) - cellY;
	float h1, h2;
	h1 = heightAtCorner(cellX, cellY);
	h2 = heightAtCorner(cellX, cellY + 1);
	return h1 * (1 - y2) + (h2 * y2);
 } else if(y == cellY) {
	bofixed x2 = (x) - cellX;
	float h1, h2;
	h1 = heightAtCorner(cellX, cellY);
	h2 = heightAtCorner(cellX + 1, cellY);
	return h1 * (1 - x2) + (h2 * x2);
 }

 // Will be used as factors for blending
 bofixed x2 = (x) - cellX;
 bofixed y2 = (y) - cellY;

 // These are heights of the corners of the cell (x; y) is on
 float h1, h2, h3, h4;

 h1 = heightAtCorner(cellX, cellY);
 h2 = heightAtCorner(cellX + 1, cellY);
 h3 = heightAtCorner(cellX, cellY + 1);
 h4 = heightAtCorner(cellX + 1, cellY + 1);

 // Blend all corners together and return the result
 // FIXME: this can probably be written _much_ more understandably and maybe faster
 return ((h1 * (1 - x2) + (h2 * x2)) * (1 - y2)) + ((h3 * (1 - x2) + (h4 * x2)) * y2);
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

bool BosonCanvas::canPlaceUnitAt(const UnitProperties* prop, const BoVector2Fixed& pos, ProductionPlugin* factory) const
{
 bofixed width = prop->unitWidth();
 bofixed height = prop->unitHeight();
 if (width <= 0) {
	boError() << k_funcinfo << "invalid width for " << prop->typeId() << endl;
	return false;
 }
 if (height <= 0) {
	boError() << k_funcinfo << "invalid height for " << prop->typeId() << endl;
	return false;
 }
 if (!onCanvas(pos)) {
	return false;
 }
 BoRectFixed r(pos, BoVector2Fixed(pos.x() + width, pos.y() + height));
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
	if ((r.center() - factoryUnit->center()).dotProduct() <= BUILD_RANGE * BUILD_RANGE) {
		return true;
	}
 } else {
	const RefineryProperties* refinery = (RefineryProperties*)prop->properties(PluginProperties::Refinery);
	if(refinery) {
		// Refineries can't be built close to resource mines
		QValueList<Unit*> list = collisions()->unitCollisionsInRange(r.center(), REFINERY_FORBID_RANGE);
		QValueList<Unit*>::Iterator it;
		for (it = list.begin(); it != list.end(); it++) {
			ResourceMinePlugin* resource = (ResourceMinePlugin*)(*it)->plugin(UnitPlugin::ResourceMine);
			if(resource) {
				if((refinery->canRefineMinerals() && resource->canProvideMinerals()) ||
						(refinery->canRefineOil() && resource->canProvideOil())) {
					return false;
				}
			}
		}
	}
	// must be in BUILD_RANGE of any facility of the player
	QValueList<Unit*> list = collisions()->unitCollisionsInRange(r.center(), BUILD_RANGE);
	QValueList<Unit*>::Iterator it;
	for (it = list.begin(); it != list.end(); it++) {
		if ((*it)->isFacility() && (*it)->owner() == factory->player()) {
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

 // by default it goes to "work" == -1. units will change this.
 d->mWork2AdvanceList[-1].append(item);
}

void BosonCanvas::deleteItem(BosonItem* item)
{
 // remove the item from the canvas BEFORE deleting it. we might need to do some
 // cleanups and might need rtti() for them (which doesnt exist anymore in the
 // BosonItem d'tor)
 if (RTTI::isUnit(item->rtti())) {
	Unit* u = (Unit*)item;
	// Update occupied status of cells that unit occupied
	u->setMovingStatus(UnitBase::Removing);
	// In editor mode, we need to do couple of things before deleting the unit,
	//  to prevent crashes later (e.g. when selecting units)
	if (!boGame->gameMode()) {
		u->owner()->unitDestroyed(u);
		emit signalUnitRemoved(u);
	}
 }

 removeItem(item);

 // actually delete it
 delete item;
}

void BosonCanvas::removeItem(BosonItem* item)
{
 removeAnimation(item);
 d->mAllItems.remove(item);
 for (BoItemList::Iterator it = d->mAllItems.begin(); it != d->mAllItems.end(); ++it) {
	(*it)->itemRemoved(item);
 }
 if (RTTI::isUnit(item->rtti())) {
	Unit* u = (Unit*)item;
	if (d->mDestroyedUnits.contains(u)) {
		boError() << k_funcinfo << item << " still in destroyed units list" << endl;
	}
 }

 // remove from all advance lists
 for (QMap<int, QPtrList<BosonItem> >::Iterator it = d->mWork2AdvanceList.begin(); it != d->mWork2AdvanceList.end(); ++it) {
	(*it).removeRef(item);
 }
 d->mChangeAdvanceList.removeRef(item);

 emit signalRemovedItem(item);
}

unsigned int BosonCanvas::effectsCount() const
{
 return effects()->count();
}

QPtrList<BosonEffect>* BosonCanvas::effects() const
{
 return &(d->mEffects);
}

void BosonCanvas::deleteUnusedShots()
{
 QPtrList<BosonItem> unusedShots;
 BoItemList::Iterator it;
 for (it = d->mAllItems.begin(); it != d->mAllItems.end(); ++it) {
	if (RTTI::isShot((*it)->rtti())) {
		BosonShot* shot = (BosonShot*)*it;
		if (!shot->isActive()) {
			unusedShots.append(*it);
		}
	}
 }
 while (!unusedShots.isEmpty()) {
	BosonItem* i = unusedShots.take(0);
	deleteItem(i);
 }
}

void BosonCanvas::addEffect(BosonEffect* e)
{
 d->mEffects.append(e);
}

void BosonCanvas::addEffects(const QPtrList<BosonEffect> effects)
{
 QPtrListIterator<BosonEffect> it(effects);
 for (; it.current(); ++it) {
	addEffect(it.current());
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

 QDomElement pathfinderxml = root.namedItem("Pathfinder").toElement();
 bool loadpathfinder = !pathfinderxml.isNull();
 if (loadpathfinder) {
	initPathfinder();
	pathfinder()->loadFromXML(pathfinderxml);
	pathfinder()->setDataLocked(true);
 }


 if (!loadEventListenerFromXML(root)) {
	boError(260) << k_funcinfo << "unable to load EventListener from XML" << endl;
	return false;
 }
 if (!loadItemsFromXML(root)) {
	boError(260) << k_funcinfo << "unable to load items from XML" << endl;
	return false;
 }
 if (!loadEffectsFromXML(root)) {
	boError(260) << k_funcinfo << "unable to load effects from XML" << endl;
	return false;
 }

 QDomElement handler = root.namedItem("DataHandler").toElement();
 if (handler.isNull()) {
	boError(260) << k_funcinfo << "DataHandler not found" << endl;
	return false;
 }
 BosonPropertyXML propertyXML;
 if (!propertyXML.loadFromXML(handler, d->mProperties)) {
	boError(260) << k_funcinfo << "unable to load the datahandler" << endl;
	return false;
 }

 if (loadpathfinder) {
	// TODO: make sure this is correct (it was true earlier)
	pathfinder()->setDataLocked(false);
 } else {
	initPathfinder();
 }
 boDebug(260) << k_funcinfo << "done" << endl;
 return true;
}

bool BosonCanvas::loadItemsFromXML(const QDomElement& root)
{
 QDomNodeList list = root.elementsByTagName(QString::fromLatin1("Items"));
 QValueList<QDomElement> allItemElements;
 QValueList<BosonItem*> allItems;
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement items = list.item(i).toElement();
	if (items.isNull()) {
		boError(260) << k_funcinfo << "Items tag is not an element" << endl;
		continue;
	}
	bool ok = false;

	// AB: WARNING: we store the _index_ in the playerList() in saveAsXML(),
	// but here we expect the _actual ID_ of the player!
	// -> it should get filled in on startup by BosonStarting or so
	unsigned int id = items.attribute(QString::fromLatin1("PlayerId")).toUInt(&ok);
	if (!ok) {
		boError(260) << k_funcinfo << "PlayerId of Items Tag " << i << " is not a valid number" << endl;
		continue;
	}
	Player* owner = (Player*)boGame->findPlayer(id);
	if (!owner) {
		// AB: this is totally valid. less players in game, than in the
		// file.
		continue;
	}

	QDomNodeList itemList = items.elementsByTagName(QString::fromLatin1("Item"));
	for (unsigned int j = 0; j < itemList.count(); j++) {
		QDomElement item = itemList.item(j).toElement();
		if (item.isNull()) {
			continue;
		}
		BosonItem* i = createItemFromXML(item, owner);
		if (!i) {
			boError(260) << k_funcinfo << "failed creating item " << j << endl;
			return false;
		}
		allItemElements.append(item);
		allItems.append(i);
	}
 }
 if (allItemElements.count() != allItems.count()) {
	boError(260) << k_funcinfo << "item count != element count" << endl;
	return false;
 }
 boDebug(260) << k_funcinfo << "created " << allItems.count() << " items" << endl;

 unsigned int itemCount = 0;
 for (unsigned int i = 0; i < allItems.count(); i++) {
	QDomElement e = allItemElements[i];
	BosonItem* item = allItems[i];
	if (!loadItemFromXML(e, item)) {
		boError(260) << k_funcinfo << "failed loading item" << endl;
		return false;
	}
	itemCount++;
 }
 boDebug(260) << k_funcinfo << "loaded " << itemCount << " items" << endl;

 return true;
}

bool BosonCanvas::loadEventListenerFromXML(const QDomElement& root)
{
 QDomDocument doc = root.ownerDocument();
 QDomElement eventListener = root.namedItem("EventListener").toElement();
 if (eventListener.isNull()) {
	boError(260) << k_funcinfo << "EventListener in not a valid element" << endl;
	return false;
 }
 return d->mEventListener->load(eventListener);
}

BosonItem* BosonCanvas::createItemFromXML(const QDomElement& item, Player* owner)
{
 if (item.isNull()) {
	return 0;
 }
 if (!owner) {
	return 0;
 }
 bool ok = false;
 int rtti = item.attribute(QString::fromLatin1("Rtti")).toInt(&ok);
 if (!ok) {
	boError(260) << k_funcinfo << "Rtti attribute of Item is not a valid number" << endl;
	return 0;
 }

 unsigned long int type = 0;
 unsigned long int group = 0;
 unsigned long int groupType = 0;

 if (!item.hasAttribute(QString::fromLatin1("Type"))) {
	// check for deprecated attributes
	if (!item.hasAttribute(QString::fromLatin1("UnitType"))) {
		boError(260) << k_funcinfo << "missing attribute: Type for Item tag" << endl;
		return 0;
	} else {
		type = item.attribute(QString::fromLatin1("UnitType")).toULong(&ok);
	}
 } else {
	type = item.attribute(QString::fromLatin1("Type")).toULong(&ok);
 }
 if (!ok) {
	boError(260) << k_funcinfo << "Invalid Type number for Item tag" << endl;
	return 0;
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
	return 0;
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
	return 0;
 }

 BoVector3Fixed pos;
 pos.setX(item.attribute("x").toFloat(&ok));
 if (!ok) {
	boError() << k_funcinfo << "x attribute for Item tag missing or invalid" << endl;
	return 0;
 }
 pos.setY(item.attribute("y").toFloat(&ok));
 if (!ok) {
	boError() << k_funcinfo << "y attribute for Item tag missing or invalid" << endl;
	return 0;
 }
 pos.setZ(item.attribute("z").toFloat(&ok));
 if (!ok) {
	// missing z is ok, but not recommended.
	pos.setZ(0.0f);
 }


 unsigned long int id = 0;
 if (!item.hasAttribute(QString::fromLatin1("Id"))) {
	boError(260) << k_funcinfo << "missing attribute: Id for Item tag" << endl;
	return 0;
 }
 // AB: "0" indicates that we want boson to assign an Id. the tag must be prsent.
 id = item.attribute(QString::fromLatin1("Id")).toULong(&ok);
 if (!ok) {
	boError(260) << k_funcinfo << "Invalid Id number for Item tag" << endl;
	return 0;
 }
 if (id == 0) {
	id = nextItemId();
 }

 if (RTTI::isUnit(rtti)) {
	if (!item.hasAttribute(QString::fromLatin1("DataHandlerId"))) {
		boError(260) << k_funcinfo << "missing attribute: DataHandlerId for Item tag" << endl;
		return 0;
	}
	int dataHandlerId = -1;

	if (item.hasAttribute(QString::fromLatin1("DataHandlerId"))) {
		dataHandlerId = item.attribute(QString::fromLatin1("DataHandlerId")).toInt(&ok);
		if (!ok) {
			boError(260) << k_funcinfo << "Invalid DataHandlerId number for Item tag" << endl;
			return 0;
		}
	}

	// FIXME: I think we should move addUnit() to bosoncanvas.
	//
	// AB: TODO: createNewUnit() - which includes owner->addUnit()
	// AB: maybe drop create_New_Item completely and move all it does to
	// createItem().
	// (i.e. the owner->addUnit() and the theme->loadNewUnit() call. also a
	// few additional exceptions (editor, flying unit)).
	Unit* u = (Unit*)createItem(RTTI::UnitStart + type, owner, ItemType(type), pos, id);

	if (!u) {
		boError(260) << k_funcinfo << "could not create unit type=" << type << " for owner=" << owner->id() << endl;
		return 0;
	}

	// Set additional properties
	owner->addUnit(u, dataHandlerId);

	if (u->speciesTheme()) {
		u->speciesTheme()->loadNewUnit(u);
	}


	// AB: some units may depend on properties of other units - e.g. on
	// whether a unit is constructed completely.
	// we must make sure that these properties are already loaded, even if
	// the unit that a unit depends on is loaded later.
	// I hope loading the DataHandler in advance will solve this problem
	// (it comes up for harvesters currently, as they require the
	// refineries/mines to be completely constructed, as Unit::plugin()
	// returns 0 otherwise)
	BosonCustomPropertyXML propertyXML;
	QDomElement handler = item.namedItem(QString::fromLatin1("DataHandler")).toElement();
	if (handler.isNull()) {
		boError() << k_funcinfo << "NULL DataHandler tag for item" << endl;
		delete u;
		return 0;
	}
	if (!propertyXML.loadFromXML(handler, u->dataHandler())) {
		boError(260) << k_funcinfo << "unable to load item data handler" << endl;
		return false;
	}

	return (BosonItem*)u;
 } else if (RTTI::isShot(rtti)) {
	BosonShot* s = (BosonShot*)createItem(RTTI::Shot, owner, ItemType(type, group, groupType), pos, id);
	if (!s) {
		boError() << k_funcinfo << "Invalid shot - type=" << type << " group=" << group << " groupType=" << groupType << endl;
		return 0;
	}
	return (BosonItem*)s;
 } else {
	boError(260) << k_funcinfo << "unknown Rtti " << rtti << endl;
	return 0;
 }
 return 0;
}

bool BosonCanvas::loadItemFromXML(const QDomElement& element, BosonItem* item)
{
 if (!item) {
	return false;
 }
 if (!item->loadFromXML(element)) {
	boError(260) << k_funcinfo << "Could not load item correctly" << endl;
	if (RTTI::isUnit(item->rtti())) {
		// need to remove from player again
		Player* owner = ((Unit*)item)->owner();
		if (owner) {
			owner->unitDestroyed((Unit*)item);
		}
	}
	return false;
 }
 return true;
}

bool BosonCanvas::loadEffectsFromXML(const QDomElement& root)
{
 QDomElement effects = root.namedItem("Effects").toElement();
 if (effects.isNull()) {
	boError(260) << k_funcinfo << "Effects tag not found" << endl;
	return false;
 }

 bool ret = true;
 QDomNodeList list = effects.elementsByTagName(QString::fromLatin1("Effect"));

 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement effect = list.item(i).toElement();
	bool ok = false;

	unsigned int propId = 0;
	unsigned int ownerId = 0;

	propId = effect.attribute(QString::fromLatin1("PropId")).toUInt(&ok);
	if (!propId || !ok) {
		boError() << k_funcinfo << "invalid number for PropId" << endl;
		ret = false;
		continue;
	}
	ownerId = effect.attribute(QString::fromLatin1("OwnerId")).toUInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "invalid number for OwnerId" << endl;
		ret = false;
		continue;
	}

	// AB: ownerId is an item here
	// if that item is not present, then it belongs to a player that is not
	// being loaded in this game. we ignore the effect then.
	if (!d->mAllItems.findItem(ownerId)) {
		continue;
	}

	const BosonEffectProperties* prop = boEffectPropertiesManager->effectProperties(propId);
	if (!prop) {
		boError() << k_funcinfo << "Null effect properties with id " << propId << endl;
		ret = false;
		continue;
	}

	BoVector3Fixed pos, rot;
	if (!loadVector3FromXML(&pos, effect, "Position")) {
		ret = false;
		continue;
	}
	if (!loadVector3FromXML(&rot, effect, "Rotation")) {
		ret = false;
		continue;
	}

	BosonEffect* e = prop->newEffect(pos, rot);
	if(!e) {
		boError() << k_funcinfo << "NULL effect created! id: " << propId << "; owner: " << ownerId << endl;
		ret = false;
		continue;
	}
	if(!e->loadFromXML(effect)) {
		ret = false;
		delete e;
		continue;
	}
	addEffect(e);
	if (e->ownerId()) {
		// Find effect's owner
		BosonItem* owner = d->mAllItems.findItem(e->ownerId());
		if (owner) {
			owner->addEffect(e, false);
		} else {
			boError() << k_funcinfo << "Can't find owner item with id " << e->ownerId() << " for effect!" << endl;
			e->makeObsolete();  // Maybe delete immediately?
			ret = false;
		}
	}
 }

 return ret;
}

bool BosonCanvas::saveAsXML(QDomElement& root) const
{
 boDebug() << k_funcinfo << endl;

 if (!saveEventListenerAsXML(root)) {
	boError() << k_funcinfo << "cannot save event listener as XML" << endl;
	return false;
 }
 if (!saveItemsAsXML(root)) {
	boError() << k_funcinfo << "cannot save items as xml" << endl;
	return false;
 }
 if (!saveEffectsAsXML(root)) {
	boError() << k_funcinfo << "could not save effects as xml" << endl;
	return false;
 }

 QDomDocument doc = root.ownerDocument();
 // Save pathfinder
 QDomElement pathfinderxml = doc.createElement(QString::fromLatin1("Pathfinder"));
 root.appendChild(pathfinderxml);
 if (d->mPathfinder) {
	d->mPathfinder->saveAsXML(pathfinderxml);
 }

 // Save datahandler
 BosonPropertyXML propertyXML;
 QDomElement handler = doc.createElement(QString::fromLatin1("DataHandler"));
 root.appendChild(handler);
 if (!propertyXML.saveAsXML(handler, d->mProperties)) {
	boError() << k_funcinfo << "unable to save the datahandler" << endl;
	return false;
 }
 return true;
}

bool BosonCanvas::saveEventListenerAsXML(QDomElement& root) const
{
 QDomDocument doc = root.ownerDocument();
 QDomElement eventListener = doc.createElement("EventListener");
 root.appendChild(eventListener);
 return d->mEventListener->save(eventListener);
}

bool BosonCanvas::saveItemsAsXML(QDomElement& root) const
{
 QDomDocument doc = root.ownerDocument();
 QMap<unsigned int, QDomElement> owner2Items;
 for (KPlayer* p = boGame->playerList()->first(); p; p = boGame->playerList()->next()) {
	QDomElement items = doc.createElement(QString::fromLatin1("Items"));

	// note: we need to store the index in the list here, not the p->id() !
	items.setAttribute(QString::fromLatin1("PlayerId"), boGame->playerList()->findRef(p));
	root.appendChild(items);
	owner2Items.insert(p->id(), items);
 }

 BoItemList::Iterator it;
 for (it = d->mAllItems.begin(); it != d->mAllItems.end(); ++it) {
	BosonItem* i = *it;
	QDomElement items;
	if (!i->owner()) {
		BO_NULL_ERROR(i->owner());
		return false;
	}
	unsigned int id = i->owner()->id();
	items = owner2Items[id];
	if (items.isNull()) {
		boError() << k_funcinfo << "no Items element found" << endl;
		return false;
	}
	QDomElement item = doc.createElement(QString::fromLatin1("Item"));
	if (RTTI::isShot(i->rtti())) {
		if (!((BosonShot*)i)->isActive()) {
			continue;
		}
	}
	if (!i->saveAsXML(item)) {
		boError() << k_funcinfo << "Could not save item " << i << endl;
		return false;
	}
	items.appendChild(item);
 }
 return true;
}

bool BosonCanvas::saveEffectsAsXML(QDomElement& root) const
{
 QDomDocument doc = root.ownerDocument();

 // Save effects
 QDomElement effects = doc.createElement(QString::fromLatin1("Effects"));
 root.appendChild(effects);
 QPtrListIterator<BosonEffect> effectIt(d->mEffects);
 while (effectIt.current()) {
	QDomElement e = doc.createElement(QString::fromLatin1("Effect"));
	effectIt.current()->saveAsXML(e);
	effects.appendChild(e);
	++effectIt;
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

bool BosonCanvas::onCanvas(const BoVector2Fixed& pos) const
{
 return onCanvas(pos.x(), pos.y());
}

bool BosonCanvas::onCanvas(const BoVector3Fixed& pos) const
{
 return onCanvas(pos.x(), pos.y());
}

void BosonCanvas::deleteItems(const QValueList<unsigned long int>& _ids)
{
 if (!boGame || boGame->gameMode()) {
	boError() << k_funcinfo << "not in editor mode" << endl;
	return;
 }
 QValueList<unsigned long int> ids = _ids;
 BoItemList::Iterator it;
 while (!ids.isEmpty()) {
	unsigned long int id = ids.first();
	ids.pop_front();
	BosonItem* item = 0;
	for (it = d->mAllItems.begin(); !item && it != d->mAllItems.end(); ++it) {
		if (id == (*it)->id()) {
			item = (*it);
		}
	}
	deleteItem(item);
 }
}

void BosonCanvas::deleteItems(BoItemList& items)
{
 QPtrList<BosonItem> list;
 BoItemList::Iterator it;
 for (it = items.begin(); it != items.end(); ++it) {
	list.append(*it);
 }
 deleteItems(list);
 if (list.count() != 0) {
	boError() << k_funcinfo << "error on deleting items!" << endl;
 }
 items.clear();
}

void BosonCanvas::deleteItems(QPtrList<BosonItem>& items)
{
 while (items.count() > 0) {
	BosonItem* i = items.first();
	items.removeRef(i);
	deleteItem(i);
 }
}

BosonItem* BosonCanvas::createNewItem(int rtti, Player* owner, const ItemType& type, const BoVector3Fixed& pos)
{
 BosonItem* item = createItem(rtti, owner, type, pos, nextItemId());
 if (!item) {
	return 0;
 }
 if (RTTI::isUnit(item->rtti())) {
	Unit* unit = (Unit*)item;
	if (unit->owner() != owner) {
		boError() << k_funcinfo << "unexpected owner for new unit" << endl;
		return item;
	}
	owner->addUnit(unit);
	SpeciesTheme* theme = owner->speciesTheme();
	if (!theme) {
		boError() << k_funcinfo << "NULL speciesTheme" << endl;
		return item;
	}
	theme->loadNewUnit(unit);
	if (unit->itemRenderer()) {
		unit->itemRenderer()->setAnimationMode(UnitAnimationIdle);
	}
	if (unit->isFlying()) {
		// AB: we have currently not decided how to treat flying units,
		// so we just place them at a height of 2.0 on construction.
		// note that on loading units this may break the positions and
		// when the height of the ground is at 2.0, we don't recognize
		// that either.
		unit->move(unit->x(), unit->y(), 2.0f);
	}
 }


 return item;
}

BosonItem* BosonCanvas::createItem(int rtti, Player* owner, const ItemType& type, const BoVector3Fixed& pos, unsigned long int id)
{
 BosonItem* item = 0;
 if (!onCanvas(pos)) {
	boError() << k_funcinfo << "(" << pos[0] << "," << pos[1] << "," << pos[2] << ") is not on the canvas" << endl;
	return 0;
 }
 if (id == 0) {
	boError() << k_funcinfo << "id==0 is invalid." << endl;
	return 0;
 }
 if (RTTI::isUnit(rtti)) {
	item = (BosonItem*)createUnit(owner, type.mType);
 } else if (RTTI::isShot(rtti)) {
	item = (BosonItem*)createShot(owner, type.mType, type.mGroup, type.mGroupType);
 }
 if (item) {
	addItem(item);
	item->setId(id);
	item->move(pos.x(), pos.y(), pos.z());
	addAnimation(item);
	if (!item->initItemRenderer()) {
		boError() << k_funcinfo << "initModel() failed. cannot create item." << endl;
		deleteItem(item);
		item = 0;
	}
	if (!boGame->gameMode()) {
		item->setRendererToEditorMode();
	}
	if (item && !item->init()) {
		boError() << k_funcinfo << "item initialization failed. cannot create item." << endl;
		deleteItem(item);
		item = 0;
	}
 }
 if (item) {
	if (RTTI::isUnit(rtti)) {
		// We also need to recalc occupied status for cells that unit is on.
		// FIXME: this is hackish
		unitMovingStatusChanges((Unit*)item, UnitBase::Moving, UnitBase::Standing);
	}
	emit signalItemAdded(item);
 }
 return item;
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
 theme->loadNewUnit(unit);
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
	case BosonShot::Rocket:
	{
		SpeciesTheme* t = owner->speciesTheme();
		BO_CHECK_NULL_RET0(t);
		const UnitProperties* unitProperties = t->unitProperties(unitType);
		BO_CHECK_NULL_RET0(unitProperties);
		const BosonWeaponProperties* prop = unitProperties->weaponProperties(weaponPropertyId);
		BO_CHECK_NULL_RET0(prop);
		s = (BosonShot*)new BosonShotRocket(owner, this, prop);
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
		s = (BosonShot*)new BosonShotFragment(owner, this);
		break;
	}
	default:
		boError() << k_funcinfo << "Invalid type: " << shotType << endl;
		s = 0;
		break;
 }
 return s;
}

unsigned long int BosonCanvas::nextItemId()
{
 // note that per definition 0 is an invalid item ID!
 d->mNextItemId = d->mNextItemId + 1;
 return d->mNextItemId;
}

void BosonCanvas::initPathfinder()
{
 boDebug() << k_funcinfo << endl;

 if (d->mPathfinder) {
	boError() << k_funcinfo << "Pathfinder already created!" << endl;
	return;
 }

 boDebug() << k_funcinfo << "Constructing..." << endl;
 d->mPathfinder = new BosonPath(map());
 boDebug() << k_funcinfo << "Constructing done!" << endl;

 boDebug() << k_funcinfo << "Initing..." << endl;
 d->mPathfinder->init();
 boDebug() << k_funcinfo << "Initing done!" << endl;

 boDebug() << k_funcinfo << "DONE" << endl;
}

BosonPath* BosonCanvas::pathfinder() const
{
 return d->mPathfinder;
}

void BosonCanvas::unitMovingStatusChanges(Unit* u, int oldstatus, int newstatus)
{
 if (pathfinder()) {
	pathfinder()->unitMovingStatusChanges(u, oldstatus, newstatus);
 }
}

BoEventListener* BosonCanvas::eventListener() const
{
 return d->mEventListener;
}

bool BosonCanvas::saveConditions(QDomElement& root) const
{
 if (!eventListener()) {
	BO_NULL_ERROR(eventListener());
	return false;
 }
 return eventListener()->saveConditions(root);
}

bool BosonCanvas::loadConditions(const QDomElement& root)
{
 if (!eventListener()) {
	BO_NULL_ERROR(eventListener());
	return false;
 }
 return eventListener()->loadConditions(root);
}

BosonItem* BosonCanvas::findItem(unsigned long int id) const
{
 return d->mAllItems.findItem(id);
}

