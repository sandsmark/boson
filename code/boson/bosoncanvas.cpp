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

#include "bosoncanvas.h"
#include "player.h"
#include "cell.h"
#include "unit.h"
#include "unitplugins.h"
#include "bosonmap.h"
#include "unitproperties.h"
#include "boshot.h"
#include "bosonmusic.h"
#include "unitgroup.h"
#include "bosontiles.h"
#include "bodisplaymanager.h"
#include "speciestheme.h"

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <qptrdict.h>
#include <qbitmap.h>
#include <qdatetime.h>
#include <qtimer.h>

#include <unistd.h>

#include "defines.h"

#include "bosoncanvas.moc"


/**
 * Pixmap loader for the tileset. We use a different thread to provide
 * non-blocking UI. UPDATE: not correct anymore!
 *
 * It is important that you <em>DON'T</em> access the canvas (especially
 * update()) before this is completed! The canvas does <em>not</em> have tiles
 * before that!
 *
 * The first function accessing the canvas (in terms of update()) is @ref
 * initFogOfWar which therefore checks if the thread is finished.
 **/
class TileLoader
{
public:
	TileLoader(BosonCanvas* c)
	{
		mCanvas = c;
		mTile = 0;
		mTiles = new BosonTiles;
	}
	virtual ~TileLoader()
	{
		delete mTiles;
	}

	void setDir(const QString& d)
	{
		mDir = d;
	}

	void start()
	{
		run();
	}

protected:
	virtual void run()
	{
		kdDebug() << k_funcinfo << endl;
		QTime time;
		time.start();
		mTiles->loadTiles(mDir);
		mTile = new QPixmap(mTiles->pixmap());
		kdDebug() << k_funcinfo << "loading took: " << time.elapsed() << endl;
		if (mTile->isNull()) {
			kdError() << k_funcinfo << "NULL pixmap" << endl;
			return;
		}
		mCanvas->setTileSet(mTile);
		delete mTile;
		mTile = 0;
		kdDebug() << k_funcinfo << "done" << endl;
	}
private:
	QString mDir;
	QPixmap* mTile;
	BosonCanvas* mCanvas;
	BosonTiles* mTiles;
	friend class BosonCanvas;  // Then BosonCanvas can access mTiles
};

class FogOfWar : public QCanvasSprite
{
public:
	FogOfWar(QCanvasPixmapArray* a, QCanvas* c) : QCanvasSprite(a, c) {}
	virtual int rtti() const { return RTTI::FogOfWar; }

	virtual bool collidesWith(const QCanvasItem*) const
	{
		return false;
	}
};

class BosonCanvas::BosonCanvasPrivate
{
public:
	BosonCanvasPrivate()
	{
		mMap = 0;
		mFogPixmap = 0;

		mLoader = 0;

		mDisplayManager = 0;
	}
	
	QPixmap mPix;
	QPtrList<Unit> mDestroyedUnits;
	QPtrList<BoShot> mDeleteShot;
	QPtrDict<QCanvasSprite> mFogOfWar;
	QCanvasPixmapArray* mFogPixmap;
	BoDisplayManager* mDisplayManager;

	BosonMap* mMap; // just a pointer - no memory allocated

	QPtrList<Unit> mWorkChanged; // Unit::setAdvanceWork() has been called on these units.

	QPtrList<QCanvasItem> mAnimList; // see BosonCanvas::slotAdvance()
	QPtrList<Unit> mWorkNone;
	QPtrList<Unit> mWorkProduce;
	QPtrList<Unit> mWorkMove;
	QPtrList<Unit> mWorkMine;
	QPtrList<Unit> mWorkRefine;
	QPtrList<Unit> mWorkAttack;
	QPtrList<Unit> mWorkConstructed;

	QValueList<UnitGroup> mGroups;

	TileLoader* mLoader;
};

BosonCanvas::BosonCanvas(QObject* parent)
		: QCanvas(parent, "BosonCanvas")
{
 init();
}

BosonCanvas::BosonCanvas(QPixmap p, unsigned int w, unsigned int h) 
		: QCanvas(p, w, h, BO_TILE_SIZE, BO_TILE_SIZE)
{
 init();

 d->mPix = p;
}

void BosonCanvas::init()
{
 d = new BosonCanvasPrivate;
 d->mFogOfWar.setAutoDelete(true);
 d->mDestroyedUnits.setAutoDelete(false);
 d->mDeleteShot.setAutoDelete(true);

 d->mLoader = new TileLoader(this);
 connect(d->mLoader->mTiles, SIGNAL(signalTilesLoading(int)), this, SIGNAL(signalTilesLoading(int)));
}

BosonCanvas::~BosonCanvas()
{
kdDebug()<< k_funcinfo << endl;
 quitGame();
 delete d->mLoader;
 delete d->mFogPixmap;
 delete d;
kdDebug()<< k_funcinfo <<"done"<< endl;
}

void BosonCanvas::quitGame()
{
 deleteDestroyed(); // already called before
 d->mAnimList.clear();
 d->mFogOfWar.clear();
 d->mDeleteShot.clear();

 d->mWorkNone.clear();
 d->mWorkProduce.clear();
 d->mWorkMove.clear();
 d->mWorkMine.clear();
 d->mWorkRefine.clear();
 d->mWorkAttack.clear();
 d->mWorkConstructed.clear();
 d->mGroups.clear();
}

void BosonCanvas::deleteDestroyed()
{
 d->mDestroyedUnits.setAutoDelete(true);
 d->mDestroyedUnits.clear();
 d->mDestroyedUnits.setAutoDelete(false);
}

void BosonCanvas::setTileSet(QPixmap* p)
{
 // called from TileLoader.
 kdDebug() << k_funcinfo << endl;
 if (p->isNull()) {
	kdError() << k_funcinfo << "NULL pixmap" << endl;
	return;
 }
 if (width() == 0 || height() == 0) {
	kdError() << "Cannot load tiles" << endl;
	return;
 }
 if (!d->mMap) {
	kdError() << k_funcinfo << ": NULL map" << endl;
	return;
 }
 setTiles(*p, d->mMap->width(), d->mMap->height(), BO_TILE_SIZE, BO_TILE_SIZE); 
 
 for (unsigned int i = 0; i < d->mMap->width(); i++) {
	for (unsigned int j = 0; j < d->mMap->height(); j++) {
		Cell* c = d->mMap->cell(i, j);
		if (!c) {
			kdError() << k_funcinfo << ": NULL cell" << endl;
			continue;
		}
		slotAddCell(i, j, c->groundType(), c->version());
	}
 }
 update();
 emit signalTilesLoaded();
}

Cell* BosonCanvas::cell(int x, int y) const
{
 if (!d->mMap) {
	kdError() << k_funcinfo << ": NULL map" << endl;
	return 0;
 }
 return d->mMap->cell(x, y);
}

void BosonCanvas::slotAddUnit(Unit* unit, int x, int y)
{
 if (!unit) {
	kdError() << k_funcinfo << ": NULL unit!" << endl;
	return;
 }
 
 double z = 0;
 if ((unit->unitProperties()->isFacility())) {
	z = Z_FACILITY;
 } else if (unit->unitProperties()->isAircraft()) {
	z = Z_MOBILE + 10;
 } else { // ship or land unit
	z = Z_MOBILE;
 }
 unit->setZ(z);
 unit->move( BO_TILE_SIZE * x, BO_TILE_SIZE * y );
 unit->show();

 update();
}

Unit* BosonCanvas::findUnitAt(const QPoint& pos)
{
 QCanvasItemList list = collisions(pos);
 QCanvasItemList::Iterator it;

 for (it = list.begin(); it != list.end(); ++it) {
	if (RTTI::isUnit((*it)->rtti())) {
		Unit* u = (Unit*)*it;
		if (!u->isDestroyed()) {
			return u;
		}
	}
 }
 return 0;
}

void BosonCanvas::advance()
{
// we use slotAdvance() now.
 kdError() << k_funcinfo << "is obsolete!!" << endl;
}

void BosonCanvas::slotAdvance(unsigned int advanceCount)
{
// we cannot use QCanvas::advance() as it advances the animated items in an
// internal "animDict". this is sorted by pointer addresses and therefore the
// members differ on every client. So we implement our own advance() as well as
// addAnimation()/removeAnimation(). We create our own animDict which is sorted
// by id :-)
// why does QCanvas use a QPtrDict? we use a QPtrList...

// update (02/03/14): for performance reasons we use several lists now. Every
// UnitBase::WorkType has an own list which is iterated and the units are
// advanced according to advanceCount. E.g. we don't need to call
// advanceProduce() *every* advance call but we *need* to call advanceMove()
// every advance call (if the unit is moving). Their lists must not be changed
// while they are iterated. So we test if the advanceWork() was changed and if so we add
// the unit to the workChanged list and after all of the advance calls we change
// the lists.
	
 QPtrListIterator<QCanvasItem> animIt(d->mAnimList);
 while (animIt.current()) {
	// the only thing done here is to increase the reload counter. perhaps
	// we should add a separate list containing all units which are
	// realoading instead? would save a lot of function calls...
	animIt.current()->advance(0);
	++animIt;
 }

 if (d->mWorkNone.count() > 0 && (advanceCount % 10) == 0) {
	QPtrListIterator<Unit> it(d->mWorkNone);
	while (it.current()) {
		it.current()->advanceNone();
		++it;
	}
 }
 if (d->mWorkProduce.count() > 0 && (advanceCount % 1) == 0) {// always true. should be be bigger, like % 10 or so. we need to change something in the production logic for this.
	QPtrListIterator<Unit> it(d->mWorkProduce);
	while (it.current()) {
		if (!it.current()->productionPlugin()) {
			kdWarning() << k_lineinfo << "unit cannot produce" << endl;
			it.current()->setWork(Unit::WorkNone);
		} else {
			it.current()->productionPlugin()->advance();
		}
		++it;
	}
 }
 if ((d->mWorkMove.count() > 0 || d->mGroups.count() > 0) && (advanceCount % 1) == 0) { // always true
	QPtrListIterator<Unit> it(d->mWorkMove);
	while (it.current()) {
		if (!it.current()->isDestroyed()) {
			it.current()->advanceMove(); // move
			it.current()->advanceMoveCheck(); // safety check for advanceMove(). See comments in Unit::moveBy()
		}
		++it;
	}
	QValueList<UnitGroup>::Iterator groupIt;
	for (groupIt = d->mGroups.begin(); groupIt != d->mGroups.end(); ++groupIt) {
		(*groupIt).advanceGroupMove();
	}
 }
 if (d->mWorkMine.count() > 0 && (advanceCount % 40) == 0) {
	QPtrListIterator<Unit> it(d->mWorkMine);
	while (it.current()) {
		if (!it.current()->isDestroyed()) {
			it.current()->advanceMine();
		}
		++it;
	}
 }
 if (d->mWorkRefine.count() > 0 && (advanceCount % 40) == 0) {
	QPtrListIterator<Unit> it(d->mWorkRefine);
	while (it.current()) {
		if (!it.current()->isDestroyed()) {
			it.current()->advanceRefine();
		}
		++it;
	}
 }
 if (d->mWorkAttack.count() > 0 && (advanceCount % 5) == 0) {
	QPtrListIterator<Unit> it(d->mWorkAttack);
	while (it.current()) {
		if (!it.current()->isDestroyed()) {
			it.current()->advanceAttack();
		}
		++it;
	}
 }

 if (d->mWorkConstructed.count() > 0 && (advanceCount % 30) == 0) {
	QPtrListIterator<Unit> it(d->mWorkConstructed);
	while (it.current()) {
		if (!it.current()->isDestroyed()) {
			it.current()->advanceConstruction();
		}
		++it;
	}
 }
 animIt.toFirst();
 while (animIt.current()) {
	animIt.current()->advance(1);
	++animIt;
 }

 changeWork();

 if (advanceCount == MAXIMAL_ADVANCE_COUNT) {
//TODO: ensure that these units are in *no* list!
	kdDebug() << "MAXIMAL_ADVANCE_COUNT" << endl;
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
		}
		++deletionIt;
	}
 
	while (deleteList.count() > 0) {
		Unit* u = deleteList.first();
		deleteList.removeRef(u);
		d->mDestroyedUnits.removeRef(u);
		delete u;
	}
	d->mDeleteShot.clear();
 }

 // note we do *not* call update() here anymore! it is done in in Boson, after
 // signalAdvance() was emitted. This way we are able to call slotAdvance()
 // twice in a loop without the need to update before all calls are done
}

bool BosonCanvas::canGo(const UnitProperties* prop, const QRect& rect) const
{
// kdDebug() << k_funcinfo << endl;
 int y = rect.y() / BO_TILE_SIZE; // what about modulu? do we care ?
 do {
	int x = rect.x() / BO_TILE_SIZE;
	do {
		Cell* newCell = cell(x, y);
		if (!newCell) {
			kdError() << k_funcinfo << "NULL cell" << endl;
			return false;
		}
		if (!newCell->canGo(prop)) {
			kdDebug() << "can  not go on " << x << "," << y << endl;
			return false;
		} else {
//			kdDebug() << "can go on " << x << "," << y << endl;
		}
		x++;
	} while (x * BO_TILE_SIZE <= rect.right());
	y++;
 } while(y * BO_TILE_SIZE <= rect.bottom());

 return true;
}

void BosonCanvas::setMap(BosonMap* map)
{
 d->mMap = map;
}

void BosonCanvas::loadTiles(const QString& tiles)
{
 QString dir = KGlobal::dirs()->findResourceDir("data", QString("boson/themes/grounds/%1/index.desktop").arg(tiles)) + QString("boson/themes/grounds/%1").arg(tiles);
 if (dir == QString::null) {
	kdError() << k_funcinfo << "Cannot find tileset " << tiles << endl;
	return;
 }
 d->mLoader->setDir(dir);
 QTimer::singleShot(0, this, SLOT(slotLoadTiles()));
}

void BosonCanvas::slotLoadTiles()
{
 kdDebug() << k_funcinfo << endl;
 if (!d->mMap) {
	kdError() << k_funcinfo << ": NULL map" << endl;
	return;
 }
 resize(d->mMap->width() * BO_TILE_SIZE, d->mMap->height() * BO_TILE_SIZE);
 d->mLoader->start();
}

void BosonCanvas::slotAddCell(int x, int y, int groundType, unsigned char version)
{
 int tile = Cell::tile(groundType, version);
 if (tile < 0 || tile >= (int)d->mMap->width() * (int)d->mMap->height()) {
	kdWarning() << "Invalid tile " << tile << endl;
	return;
 }
 setTile(x, y, tile);
}

void BosonCanvas::addAnimation(QCanvasItem* item)
{
 d->mAnimList.append(item);
}

void BosonCanvas::removeAnimation(QCanvasItem* item)
{
 d->mAnimList.removeRef(item);
}

void BosonCanvas::unitMoved(Unit* unit, double oldX, double oldY)
{
 updateSight(unit, oldX, oldY);
	
// test if any unit has this unit as target. If sou then adjust the destination. 
//TODO 

// used to adjust the mini map
 emit signalUnitMoved(unit, oldX, oldY);
}

void BosonCanvas::leaderMoved(Unit* unit, double oldX, double oldY)
{
 QValueListIterator<UnitGroup> it;
 for(it = d->mGroups.begin(); it != d->mGroups.end(); ++it) {
	if((*it).isLeader(unit)) {
		(*it).leaderMoved(unit->x() - oldX, unit->y() - oldY);
		break;
	}
 }
 unitMoved(unit, oldX, oldY);
}

void BosonCanvas::leaderDestroyed(Unit* unit)
{
 QValueListIterator<UnitGroup> it;
 for(it = d->mGroups.begin(); it != d->mGroups.end(); ++it) {
	if((*it).isLeader(unit)) {
		(*it).leaderDestroyed();
		return;
	}
 }
}

void BosonCanvas::leaderStopped(Unit* unit)
{
 QValueListIterator<UnitGroup> it;
 for(it = d->mGroups.begin(); it != d->mGroups.end(); ++it) {
	if((*it).isLeader(unit)) {
		(*it).leaderStopped();
		(*it).setDeleteGroup(true);
		return;
	}
 }
}

void BosonCanvas::slotNewGroup(Unit* leader, QPtrList<Unit> members)
{
 UnitGroup group(true);
 group.setLeader(leader);
 Unit *unit;
 leader->setWork(UnitBase::WorkMoveInGroup);
 for(unit = members.first(); unit; unit = members.next()) {
	unit->setWork(UnitBase::WorkMoveInGroup);
	group.addMember(unit);
 }
 d->mGroups.append(group);
}

void BosonCanvas::updateSight(Unit* unit, double, double)
{
// TODO: use the double parameters - check whether the player can still see
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
// kdDebug() << k_funcinfo << endl;
// kdDebug() << "left=" << left << ",right=" << right << endl;
// kdDebug() << "top=" << top << ",bottom=" << bottom << endl;

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

void BosonCanvas::shootAtUnit(Unit* target, Unit* attackedBy, long int damage)
{
 if (!target) {
	kdError() << k_funcinfo << ": NULL target" << endl;
	return;
 }
 if (!attackedBy) {
	kdError() << k_funcinfo << ": NULL attacker" << endl;
	return;
 }

 if (damage < 0) {
	target->setHealth(target->health() + ((unsigned long)-damage));
 } else {
	target->setHealth((target->health() > (unsigned long)damage) ? target->health() - damage : 0);
 }
 
 if (target->isDestroyed()) {
	destroyUnit(target); // display the explosion ; not the shoot
 } else {
	(void) new BoShot(target, attackedBy, this);
 }
 boMusic->playSound(attackedBy, Unit::SoundShoot);
}

void BosonCanvas::destroyUnit(Unit* unit)
{
 // please note: you MUST NOT delete the unit here!!
 // we call it from advance() and items must not be deleted from there!
 if (!unit) {
	return;
 }
 if (!d->mDestroyedUnits.contains(unit)) {
	kdDebug() << "destroy unit " << unit->id() << endl;
	Player* owner = unit->owner();
	d->mDestroyedUnits.append(unit);

	// the unit is added to a list - now displayed as a wreckage only.
	unit->setAnimated(false);
	unit->setHealth(0); // in case of an accidental change before
	unit->setWork(UnitBase::WorkDestroyed);
	owner->unitDestroyed(unit); // remove from player without deleting
	boMusic->playSound(unit, Unit::SoundReportDestroyed);
	(void) new BoShot(unit, 0, this, true);
	emit signalUnitDestroyed(unit);
	if (owner->checkOutOfGame()) {
		killPlayer(owner);
	}
 }
}

Cell* BosonCanvas::cellAt(Unit* unit) const
{
 if (!unit) {
	return 0;
 }
 return cellAt(unit->x() + unit->width() / 2, unit->y() + unit->width() / 2);
}

Cell* BosonCanvas::cellAt(double x, double y) const
{
 return cell(x / BO_TILE_SIZE, y / BO_TILE_SIZE);
}

BosonMap* BosonCanvas::map() const
{
 return d->mMap;
}

void BosonCanvas::fogLocal(int x, int y)
{
 // AB: I would rather like to place this into BosonBigDisplay as the fog of war
 // logically belongs to there. But I'd need to add a BosonMap pointer there and
 // make some small hacks - so you can find this here now. Note that this is
 // only the *local* fow, i.e. the fog of the local player.
 // Same is valid for unfogLocal.

 if (!d->mFogPixmap) {
	return;
 }
 Cell* c = cell(x, y);
 if (!c) {
	kdError() << k_funcinfo << "NULL cell" << endl;
	return;
 }
 if (d->mFogOfWar[c]) {
	kdError() << "tried adding fog of war twice!!!!" << endl;
	return;
 }
 QCanvasSprite* fog = new FogOfWar(d->mFogPixmap, this);
 fog->move(x * BO_TILE_SIZE, y * BO_TILE_SIZE);
 fog->setZ(Z_FOG_OF_WAR);
 fog->show();
 d->mFogOfWar.insert(c, fog);
}

void BosonCanvas::unfogLocal(int x, int y)
{
// it seems like this doesn't work sometimes... dunno why...
 Cell* c = cell(x, y);
 if (!c) {
	kdError() << k_funcinfo << "NULL cell" << endl;
	return;
 }

 QCanvasSprite* s = d->mFogOfWar.take(c);
 if (s) {
	s->hide();
	delete s;
 } else {
	kdDebug() << "cannot remove fog on " << x << "," << y << endl;
 }
}

void BosonCanvas::initFogOfWar(Player* p)
{
 if (!d->mFogPixmap) {
	QString fogPath = locate("data", "boson/themes/fow.xpm");
	d->mFogPixmap = new QCanvasPixmapArray(fogPath);
	if (!d->mFogPixmap->image(0) || 
			d->mFogPixmap->image(0)->width() != (BO_TILE_SIZE * 2) ||
			d->mFogPixmap->image(0)->height() != (BO_TILE_SIZE * 2)) {
		kdError() << k_funcinfo << "Cannot load fow.xpm" << endl;
		delete d->mFogPixmap;
		d->mFogPixmap = 0;
		return;
	}
	QBitmap mask(fogPath);
	if (mask.width() != (BO_TILE_SIZE * 2) || mask.height() != (BO_TILE_SIZE * 2)) {
		kdError() << k_funcinfo << "Can't create fow mask" << endl;
		delete d->mFogPixmap;
		d->mFogPixmap = 0;
		return;
	}
	d->mFogPixmap->image(0)->setMask(mask);
	d->mFogPixmap->image(0)->setOffset(BO_TILE_SIZE / 2, BO_TILE_SIZE / 2);
 }

// now put the fow wherever necessary to meet the players sight
 for (unsigned int i = 0; i < d->mMap->width(); i++) {
	for (unsigned int j = 0; j < d->mMap->height(); j++) {
		if (p->isFogged(i, j)) {
			fogLocal(i, j);
		} else {
			unfogLocal(i, j);
		}
	}
 }
}

QValueList<Unit*> BosonCanvas::unitCollisionsInRange(const QPoint& pos, int radius) const
{
// qt bug (confirmed). will be fixed in 3.1
#if QT_VERSION >= 310
 QCanvasItemList l = collisions(QRect(
		(pos.x() - radius > 0) ? pos.x() - radius : 0,
		(pos.y() - radius > 0) ? pos.y() - radius : 0,
		pos.x() + radius,
		pos.y() + radius));
#else
 QCanvasItemList l = collisions(QRect(
		(pos.x() - radius > 0) ? pos.x() - radius : 0,
		(pos.y() - radius > 0) ? pos.y() - radius : 0,
		pos.x() + radius - 1,
		pos.y() + radius - 1));
#endif
		
			
 QValueList<Unit*> list;
 for (unsigned int i = 0; i < l.count(); i++) {
	if (!RTTI::isUnit(l[i]->rtti())) {
		// this item is not important for us here
		continue;
	}
	Unit* u = (Unit*)l[i];
	if (u->isDestroyed()) {
		// this item is not important for us here
		continue;
	}
//	kdDebug() << "unit at x=" << u->x() << ",y=" << u->y() << ",pos=" << pos.x() << "," << pos.y() << endl;
	int w = pos.x() - (int)u->x();
	int h = pos.y() - (int)u->y();
//	kdDebug() << "w*w=" << w*w << ",h*h=" << h*h << " <= r*r=" << radius*radius<< endl;

	if (w * w + h * h <= radius * radius) {
//		kdDebug() << "adding " << u->id() << endl;
		list.append(u);
	}
 }
 return list;
}

QValueList<Unit*> BosonCanvas::unitsAtCell(int x, int y) const
{
 if (!cell(x, y)) {
	return QValueList<Unit*>();
 }
 return cell(x, y)->items()->units(false);
}

bool BosonCanvas::cellOccupied(int x, int y) const
{
 if (!cell(x, y)) {
	return true;
 }
 return cell(x, y)->isOccupied();
}

bool BosonCanvas::cellOccupied(int x, int y, Unit* unit, bool excludeMoving) const
{
 if (unit->isFlying()) {
	return false; // even if there are other flying units - different altitudes!
 }
 if (!cell(x, y)) {
	kdError() << k_funcinfo << "NULL cell" << endl;
	return true;
 }
 bool includeMoving = !excludeMoving; // FIXME: replace exclude by include in parameter
 return cell(x, y)->isOccupied(unit, includeMoving);
}

bool BosonCanvas::cellsOccupied(const QRect& rect) const
{
 const int left = rect.left() / BO_TILE_SIZE;
 const int top = rect.top() / BO_TILE_SIZE;
 const int right = rect.right() / BO_TILE_SIZE + ((rect.right() % BO_TILE_SIZE == 0) ? 0 : 1);
 const int bottom = rect.bottom() / BO_TILE_SIZE + ((rect.bottom() % BO_TILE_SIZE == 0) ? 0 : 1);

 for (int x = left; x < right; x++) {
	for (int y = top; y < bottom; y++) {
		if (cellOccupied(x, y)) {
			return true;
		}
	}
 }
 return false;
}

void BosonCanvas::setWorkChanged(Unit* u)
{
 if (d->mWorkChanged.contains(u)) {
	d->mWorkChanged.remove(u);
 }
 d->mWorkChanged.append(u);
}

void BosonCanvas::changeWork()
{
 if (d->mWorkChanged.count() == 0) {
	return;
 }
 QPtrListIterator<Unit> it(d->mWorkChanged);
 for (; it.current(); ++it) {
	Unit* u = it.current();

	// remove from all lists.
	d->mWorkNone.removeRef(u);
	d->mWorkProduce.removeRef(u);
	d->mWorkMove.removeRef(u);
	d->mWorkMine.removeRef(u);
	d->mWorkRefine.removeRef(u);
	d->mWorkAttack.removeRef(u);
	d->mWorkConstructed.removeRef(u);

	if (d->mDestroyedUnits.contains(u)) {
		continue;
	}
	if (u->isDestroyed() || u->advanceWork() == UnitBase::WorkDestroyed) {
		kdWarning() << k_funcinfo << "is already destroyed" << endl;
		continue;
	}
	switch (u->advanceWork()) {
		case UnitBase::WorkNone:
			d->mWorkNone.append(u);
			break;
		case UnitBase::WorkProduce:
			d->mWorkProduce.append(u);
			break;
		case UnitBase::WorkMove:
			d->mWorkMove.append(u);
			break;
		case UnitBase::WorkMine:
			d->mWorkMine.append(u);
			break;
		case UnitBase::WorkRefine:
			d->mWorkRefine.append(u);
			break;
		case UnitBase::WorkAttack:
			d->mWorkAttack.append(u);
			break;
		case UnitBase::WorkConstructed:
			d->mWorkConstructed.append(u);
			break;
		case UnitBase::WorkMoveInGroup:
			// already in d->mGroups
			break;
		case UnitBase::WorkDestroyed:
			// nothing to do here. is in d->mDestroyedUnits
			break;
	}
 }
 d->mWorkChanged.clear();


 unsigned int i = 0;
 while (i < d->mGroups.count()) {
	if (d->mGroups[i].deleteGroup()) {
		d->mGroups.remove(d->mGroups.at(i));
	} else {
		i++;
	}
 }
}

void BosonCanvas::deleteShot(BoShot* s)
{
 d->mDeleteShot.append(s);
}

void BosonCanvas::update()
{
 QCanvas::update();
 d->mDisplayManager->paintResources();
 d->mDisplayManager->paintChatMessages();
}

void BosonCanvas::killPlayer(Player* player)
{
 while (player->allUnits().count() > 0) {
	destroyUnit(player->allUnits().first());
 }
 player->setMinerals(0);
 player->setOil(0);
 kdDebug() << "player " << player->id() << " is out of game" << endl;
 emit signalOutOfGame(player);
}

void BosonCanvas::removeFromCells(Unit* u)
{
 int x = u->leftEdge() / BO_TILE_SIZE;
 int y = u->topEdge() / BO_TILE_SIZE;
 int w = u->width() / BO_TILE_SIZE;
 int h = u->height() / BO_TILE_SIZE;
 for (int i = x; i < x + w; i++) {
	for (int j = y; j < y + h; j++) {
		Cell* c = cell(i, j);
		if (!c) {
			kdError() << k_funcinfo << "NULL cell - x=" << i << ",y=" << j << ",id=" << u->id() << endl;
			continue;
		}
		c->removeUnit(u);
	}
 }
}

void BosonCanvas::addToCells(Unit* u)
{
 int x = u->leftEdge() / BO_TILE_SIZE;
 int y = u->topEdge() / BO_TILE_SIZE;
 int w = u->width() / BO_TILE_SIZE;
 int h = u->height() / BO_TILE_SIZE;
 for (int i = x; i < x + w; i++) {
	for (int j = y; j < y + h; j++) {
		Cell* c = cell(i, j);
		if (!c) {
			kdError() << k_funcinfo << "NULL cell - x=" << i << ",y=" << j << ",id=" << u->id() << endl;
			continue;
		}
		c->addUnit(u);
	}
 }
}

void BosonCanvas::setDisplayManager(BoDisplayManager* m)
{
 d->mDisplayManager = m;
}

bool BosonCanvas::canPlaceUnitAt(const UnitProperties* prop, const QPoint& pos, Facility* factory) const
{
 QCanvasPixmapArray* a = prop->theme()->pixmapArray(prop->typeId());
 if (!a) {
	kdError() << k_funcinfo << "Cannot fine pixmap array for " << prop->typeId() << endl;
	return false;
 }
 if (!a->image(0)) {
	kdError() << k_funcinfo << "Cannot find first pixmap for " << prop->typeId() << endl;
	return false;
 }
 int width = a->image(0)->width();
 int height = a->image(0)->height();
 QRect r(pos.x(), pos.y(), width, height);
 if (!canGo(prop, r)) {
	return false;
 }
 if (cellsOccupied(r)) {
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
	int dx = QABS(r.center().x() - factory->boundingRect().center().x());
	int dy = QABS(r.center().y() - factory->boundingRect().center().y());
	if (dx * dx + dy * dy <= BUILD_RANGE * BUILD_RANGE) {
		return true;
	}
 } else {
	// must be in BUILD_RANGE of any facility of the player
	QValueList<Unit*> list = unitCollisionsInRange(r.center(), BUILD_RANGE);
	for (unsigned int i = 0; i < list.count(); i++) {
		if (list[i]->isFacility() && list[i]->owner() == factory->owner()) {
			return true;
		}
	}
 }
 return false;
}

