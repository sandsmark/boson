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
#include "bosontiles.h"
#include "bodisplaymanager.h"
#include "speciestheme.h"
#include "boitemlist.h"

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
	BosonTiles* tileSet() const
	{
		return mTiles;
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
};

class FogOfWar 
#ifndef NO_OPENGL
#warning FOW is not yet implemented for OpenGL
#else
: public QCanvasSprite
#endif
{
public:
	FogOfWar(QCanvasPixmapArray* a, BosonCanvas* c) 
#ifndef NO_OPENGL
		// TODO
#else
		: QCanvasSprite(a, (QCanvas*)c) 
#endif
	{
	}
	virtual int rtti() const { return RTTI::FogOfWar; }

	virtual bool collidesWith(const QCanvasItem*) const
	{
		return false;
	}

#ifndef NO_OPENGL
	void setVisible(bool) {}
	void move(float , float ) { }
	void setZ(float) { }
#endif
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
	QPtrDict<FogOfWar> mFogOfWar;
	QCanvasPixmapArray* mFogPixmap;
	BoDisplayManager* mDisplayManager;

	BosonMap* mMap; // just a pointer - no memory allocated

	QPtrList<Unit> mWorkChanged; // Unit::setAdvanceWork() has been called on these units.

	QPtrList<BosonSprite> mAnimList; // see BosonCanvas::slotAdvance()
	QPtrList<Unit> mWorkNone;
	QPtrList<Unit> mWorkProduce;
	QPtrList<Unit> mWorkMove;
	QPtrList<Unit> mWorkMine;
	QPtrList<Unit> mWorkRefine;
	QPtrList<Unit> mWorkAttack;
	QPtrList<Unit> mWorkConstructed;

	BoItemList mAllItems;

	TileLoader* mLoader;
};

BosonCanvas::BosonCanvas(QObject* parent)
#ifndef NO_OPENGL
		: CanvasHack(parent, "BosonCanvas")
#else
		: CanvasHack(parent, "BosonCanvas")

#endif
{
 init();
}

void BosonCanvas::init()
{
 d = new BosonCanvasPrivate;
 d->mFogOfWar.setAutoDelete(true);
 d->mDestroyedUnits.setAutoDelete(false);
 d->mDeleteShot.setAutoDelete(true);

 d->mLoader = new TileLoader(this);
 connect(d->mLoader->tileSet(), SIGNAL(signalTilesLoading(int)), this, SIGNAL(signalTilesLoading(int)));
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
	kdError() << k_funcinfo << "NULL map" << endl;
	return;
 }
#ifdef NO_OPENGL
 setTiles(*p, d->mMap->width(), d->mMap->height(), BO_TILE_SIZE, BO_TILE_SIZE); 
#endif
 
 for (unsigned int i = 0; i < d->mMap->width(); i++) {
	for (unsigned int j = 0; j < d->mMap->height(); j++) {
		Cell* c = d->mMap->cell(i, j);
		if (!c) {
			kdError() << k_funcinfo << "NULL cell" << endl;
			continue;
		}
		slotAddCell(i, j, c->groundType(), c->version());
	}
 }
 emit signalTilesLoaded();
}

Cell* BosonCanvas::cell(int x, int y) const
{
 if (!d->mMap) {
	kdError() << k_funcinfo << "NULL map" << endl;
	return 0;
 }
 return d->mMap->cell(x, y);
}

void BosonCanvas::slotAddUnit(Unit* unit, int x, int y)
{
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit!" << endl;
	return;
 }
 
 float z = 0.0;
 if ((unit->unitProperties()->isFacility())) {
	z = Z_FACILITY;
 } else if (unit->unitProperties()->isAircraft()) {
	z = Z_MOBILE + 10;
 } else { // ship or land unit
	z = Z_MOBILE;
 }
 unit->setZ(z);
 unit->move(x, y);
 unit->show();
}

Unit* BosonCanvas::findUnitAt(const QPoint& pos)
{
 BoItemList list = bosonCollisions(pos);
 BoItemList::Iterator it;

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
	
 QPtrListIterator<BosonSprite> animIt(d->mAnimList);
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
 if ((d->mWorkMove.count() > 0) && (advanceCount % 1) == 0) { // always true
	QPtrListIterator<Unit> it(d->mWorkMove);
	while (it.current()) {
		if (!it.current()->isDestroyed()) {
			it.current()->advanceMove(); // move
			it.current()->advanceMoveCheck(); // safety check for advanceMove(). See comments in Unit::moveBy()
		}
		++it;
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

void BosonCanvas::loadTiles(const QString& tiles, bool withtimer)
{
 QString dir = KGlobal::dirs()->findResourceDir("data", QString("boson/themes/grounds/%1/index.desktop").arg(tiles)) + QString("boson/themes/grounds/%1").arg(tiles);
 if (dir == QString::null) {
	kdError() << k_funcinfo << "Cannot find tileset " << tiles << endl;
	return;
 }
 d->mLoader->setDir(dir);
 if(withtimer) {
	QTimer::singleShot(0, this, SLOT(slotLoadTiles()));
 }
 else {
	slotLoadTiles();
 }
}

void BosonCanvas::slotLoadTiles()
{
 kdDebug() << k_funcinfo << endl;
 if (!d->mMap) {
	kdError() << k_funcinfo << "NULL map" << endl;
	return;
 }
 resize(d->mMap->width() * BO_TILE_SIZE, d->mMap->height() * BO_TILE_SIZE);
 d->mLoader->start();
 d->mMap->setTileSet(d->mLoader->tileSet());
}

void BosonCanvas::slotAddCell(int x, int y, int groundType, unsigned char version)
{
#ifdef NO_OPENGL
 int tile = Cell::tile(groundType, version);
 if (tile < 0 || tile >= (int)d->mMap->width() * (int)d->mMap->height()) {
	kdWarning() << "Invalid tile " << tile << endl;
	return;
 }
 setTile(x, y, tile);
#endif
}

void BosonCanvas::addAnimation(BosonSprite* item)
{
 d->mAnimList.append(item);
}

void BosonCanvas::removeAnimation(BosonSprite* item)
{
 d->mAnimList.removeRef(item);
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
	kdError() << k_funcinfo << "NULL target" << endl;
	return;
 }
 if (!attackedBy) {
	kdError() << k_funcinfo << "NULL attacker" << endl;
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
 FogOfWar* fog = new FogOfWar(d->mFogPixmap, this);
 fog->move(x * BO_TILE_SIZE, y * BO_TILE_SIZE);
 fog->setZ(Z_FOG_OF_WAR);
 fog->setVisible(true);
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

 FogOfWar* f = d->mFogOfWar.take(c);
 if (f) {
	f->setVisible(false);
	delete f;
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
kdDebug() << k_funcinfo << endl;
 BoItemList l = bosonCollisions(QRect(
		(pos.x() - radius > 0) ? pos.x() - radius : 0,
		(pos.y() - radius > 0) ? pos.y() - radius : 0,
		pos.x() + radius,
		pos.y() + radius));
			
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
 if (!unit) {
	return cellOccupied(x, y);
 }
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

bool BosonCanvas::cellsOccupied(const QRect& rect, Unit* unit, bool excludeMoving) const
{
 const int left = rect.left() / BO_TILE_SIZE;
 const int top = rect.top() / BO_TILE_SIZE;
 const int right = rect.right() / BO_TILE_SIZE + ((rect.right() % BO_TILE_SIZE == 0) ? 0 : 1);
 const int bottom = rect.bottom() / BO_TILE_SIZE + ((rect.bottom() % BO_TILE_SIZE == 0) ? 0 : 1);

 for (int x = left; x < right; x++) {
	for (int y = top; y < bottom; y++) {
		if (cellOccupied(x, y, unit, excludeMoving)) {
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
		case UnitBase::WorkDestroyed:
			// nothing to do here. is in d->mDestroyedUnits
			break;
	}
 }
 d->mWorkChanged.clear();

}

void BosonCanvas::deleteShot(BoShot* s)
{
 d->mDeleteShot.append(s);
}

void BosonCanvas::update()
{
#ifdef NO_OPENGL
 QCanvas::update();
 d->mDisplayManager->paintResources();
 d->mDisplayManager->paintChatMessages();
#endif

 // TODO: only if canvas is changed
 // maybe only if relevant parts are changed
 // AB: we don't do this anymore here. we use an update timer which updates the
 // screen in a certain interval instead.
// d->mDisplayManager->slotUpdateCanvas();
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

void BosonCanvas::removeFromCells(BosonSprite* item)
{
 QPointArray cells = item->cells();
 for (unsigned int i = 0; i < cells.count(); i++) {
	Cell* c = cell(cells[i].x(), cells[i].y());
	if (!c) {
		kdError() << k_funcinfo << "NULL cell - x=" << cells[i].x() << ",y=" << cells[i].y() << endl;
		continue;
	}
	c->removeItem(item);
 }
}

void BosonCanvas::addToCells(BosonSprite* item)
{
 QPointArray cells = item->cells();
 for (unsigned int i = 0; i < cells.count(); i++) {
	Cell* c = cell(cells[i].x(), cells[i].y());
	if (!c) {
		kdError() << k_funcinfo << "NULL cell - x=" << cells[i].x() << ",y=" << cells[i].y() << endl;
		continue;
	}
	c->addItem(item);
 }
}

void BosonCanvas::setDisplayManager(BoDisplayManager* m)
{
 d->mDisplayManager = m;
}

bool BosonCanvas::canPlaceUnitAt(const UnitProperties* prop, const QPoint& pos, Facility* factory) const
{
 int width = prop->theme()->unitWidth(prop->typeId());
 int height= prop->theme()->unitHeight(prop->typeId());
 if (!width) {
	kdError() << k_funcinfo << "null width for " << prop->typeId() << endl;
	return false;
 }
 if (!height) {
	kdError() << k_funcinfo << "null height for " << prop->typeId() << endl;
	return false;
 }
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

BosonTiles* BosonCanvas::tileSet() const
{
 return d->mLoader->tileSet();
}

BoItemList BosonCanvas::allBosonItems() const
{
 return d->mAllItems;
}

void BosonCanvas::addItem(BosonSprite* item)
{
 d->mAllItems.append(item);
}

void BosonCanvas::removeItem(BosonSprite* item)
{
 d->mAllItems.remove(item);
}

BoItemList BosonCanvas::bosonCollisions(const QPointArray& cells, const BosonSprite* item, bool exact) const
{
 // FIXME: if exact is true we assume that cells == item->cells() !!
// AB: item can be NULL, too!
 BoItemList collisions;
 BoItemList seen;
 const BoItemList* cellItems;
 Cell* c = 0;
 for (unsigned int i = 0; i < cells.count(); i++) {
	c = cell(cells[i].x(), cells[i].y());
	if (!c) {
		kdWarning() << k_funcinfo << "NULL cell " << cells[i].x() << " " << cells[i].y() << endl;
		continue;
	}
	cellItems = c->items();
	BoItemList::ConstIterator it;
	for (it = cellItems->begin(); it != cellItems->end(); ++it) {
		BosonSprite* s = *it;
		if (s != item) {
			if (seen.findIndex(s) < 0 && (!item || !exact || item->bosonCollidesWith(s))) { 
				seen.append(s);
				collisions.append(s);
			}
		}
	}
 }
 return collisions;
}

BoItemList BosonCanvas::bosonCollisions(const QRect& r) const
{
 // r is canvas coordinates!
 QPointArray cells;
 int left, right, top, bottom;
 left = QMAX(r.left() / BO_TILE_SIZE, 0);
 right = QMIN(r.right() / BO_TILE_SIZE, QMAX((int)mapWidth() - 1, 0));
 top = QMAX(r.top() / BO_TILE_SIZE, 0);
 bottom = QMIN(r.bottom() / BO_TILE_SIZE, QMAX((int)mapHeight() - 1, 0));
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
 return bosonCollisions(cells, 0, true);// FIXME: exact = true has no effect
}

BoItemList BosonCanvas::bosonCollisions(const QPoint& pos) const
{
 // pos is canvas coordinates!
 QPointArray cells(1);
 cells[0] = pos / BO_TILE_SIZE;
 kdDebug() << k_funcinfo << cells[0].x() << " " << cells[0].y() << endl;
 return bosonCollisions(cells, 0, true); // FIXME: ecact = true has no effect
}

#ifndef NO_OPENGL
void BosonCanvas::resize(int w, int h)
{
 if (width() == w && height() == h) {
	return;
 }
 mWidth = w;
 mHeight = h;
}
#endif

