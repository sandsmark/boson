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
#include "boson.h"
#include "player.h"
#include "cell.h"
#include "unit.h"
#include "bosonmap.h"
#include "unitproperties.h"
#include "boshot.h"

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <qintdict.h>
#include <qptrdict.h>
#include <qbitmap.h>

#include "defines.h"

#include "bosoncanvas.moc"

class FogOfWar : public QCanvasSprite
{
public:
	FogOfWar(QCanvasPixmapArray* a, QCanvas* c) : QCanvasSprite(a, c) {}
	virtual int rtti() const { return RTTI::FogOfWar; }
};

class BosonCanvas::BosonCanvasPrivate
{
public:
	BosonCanvasPrivate()
	{
		mMap = 0;
		mFogPixmap = 0;
	}
	
	QPixmap mPix;
	QPtrList<QCanvasItem> mAnimList; // see BosonCanvas::advance()
	QPtrList<Unit> mDestroyUnits;
	QPtrList<Unit> mDestroyedUnits;
	QPtrDict<QCanvasSprite> mFogOfWar;
	QCanvasPixmapArray* mFogPixmap;

	BosonMap* mMap; // just a pointer - no memory allocated
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
 d->mDestroyedUnits.setAutoDelete(true);
 d->mFogOfWar.setAutoDelete(true);
}

BosonCanvas::~BosonCanvas()
{
// if (d->mFogPixmap) {
//	delete d->mFogPixmap;
// }

 deleteDestroyed(); // already called before
 d->mAnimList.clear();
 d->mFogOfWar.clear();
 delete d;
}

void BosonCanvas::deleteDestroyed()
{
 d->mDestroyUnits.clear();
 d->mDestroyedUnits.clear();
}

void BosonCanvas::loadTiles(const QString& name)
{
 QString themePath = locate("data", QString("boson/themes/grounds/%1").arg(name));
 QPixmap p(themePath);
 if (p.isNull()) {
	kdError() << k_funcinfo << ": Could not load " << name << endl;
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
 setTiles(p, d->mMap->width(), d->mMap->height(), BO_TILE_SIZE, BO_TILE_SIZE); 
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
		return (Unit*)*it;
	}
 }
 return 0;
}

void BosonCanvas::advance()
{
// we cannot use QCanvas::advance() as it advances the animated items in an
// internal "animDict". this is sorted by pointer addresses and therefore the
// members differ on every client. So we implement our own advance() as well as
// addAnimation()/removeAnimation(). We create our own animDict which is sorted
// by id :-)
// why does QCanvas use a QPtrDict? we use a QPtrList...
 QPtrListIterator<QCanvasItem> it(d->mAnimList);
 while (it.current()) {
	it.current()->advance(0);
	++it;
 }
 it.toFirst();
 while (it.current()) {
	it.current()->advance(1);
	++it;
 }

 // a unit must not be deleted in QCanvasItem::advance (see QT docs). Therefore
 // we add them to d->mDestroyUnits and delete them all here now.
 if (!d->mDestroyUnits.isEmpty()) {
	QPtrListIterator<Unit> destroyedIt(d->mDestroyUnits);
	while (destroyedIt.current()) {
		Unit* unit = destroyedIt.current();
		kdDebug() << "destroy unit " << unit->id() << endl;
		emit signalUnitDestroyed(unit);
		unit->owner()->unitDestroyed(unit); // remove from player without deleting
		d->mDestroyedUnits.append(unit); // delete it in destructor - maybe remove the wreckage after a timerevent?
		++destroyedIt;
	}
	d->mDestroyUnits.clear();
 }
 update();
}

bool BosonCanvas::canGo(const UnitProperties* prop, const QRect& rect) const
{
// kdDebug() << k_funcinfo << endl;
 int y = rect.y() / BO_TILE_SIZE; // what about modulu? do we care ?
 do {
	int x = rect.x() / BO_TILE_SIZE;
	do {
		Cell* newCell = cell(x, y);
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

void BosonCanvas::initMap(const QString& tileFile)
{
 if (!d->mMap) {
	kdError() << k_funcinfo << ": NULL map" << endl;
	return;
 }
 resize(d->mMap->width() * BO_TILE_SIZE, d->mMap->height() * BO_TILE_SIZE);
 loadTiles(tileFile);
 for (unsigned int i = 0; i < d->mMap->width(); i++) {
	for (unsigned int j = 0; j < d->mMap->height(); j++) {
		Cell* c = d->mMap->cell(i, j);
		if (!c) {
			kdError() << k_funcinfo << ": NULL cell" << endl;
			continue;
		}
		slotAddCell(i, j, c->groundType(), c->version());
		fogLocal(i, j); // the fog of war of the local player
	}
 }
 update();
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
	// we cannot delete it here!
	// called from Unit::advance() so we must delay
	if (!d->mDestroyUnits.contains(target)) {
		// dunno if this is correct - should it be delete immediately?
		// what about e.g. a wreckage?
		d->mDestroyUnits.append(target);
		(void) new BoShot(target, attackedBy, this, true);
	}
 } else {
	(void) new BoShot(target, attackedBy, this);
 }
 play(attackedBy->soundShoot());
}

void BosonCanvas::play(const QString& fileName)
{
 emit signalPlaySound(fileName);
}

Cell* BosonCanvas::cellAt(Unit* unit) const
{
 if (!unit) {
	return 0;
 }
 return cellAt(unit->x(), unit->y());
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
 if (d->mFogOfWar[cell(x, y)]) {
	kdError() << "tried adding fog of war twice!!!!" << endl;
	return;
 }
 QCanvasSprite* fog = new FogOfWar(d->mFogPixmap, this);
 fog->move(x * BO_TILE_SIZE, y * BO_TILE_SIZE);
 fog->setZ(Z_FOG_OF_WAR);
 fog->show();
 d->mFogOfWar.insert(cell(x, y), fog);
}

void BosonCanvas::unfogLocal(int x, int y)
{
// it seems like this doesn't work sometimes... dunno why...
 QCanvasSprite* s = d->mFogOfWar.take(cell(x, y));
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

QValueList<Unit*> BosonCanvas::unitCollisionsInRange(const QPoint& pos, int radius)
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

QValueList<Unit*> BosonCanvas::unitsAtCell(int x, int y)
{
 QValueList<Unit*> list;
 if (!cell(x, y)) {
	return list;
 }
// qt bug (confirmed). will be fixed in 3.1
#if QT_VERSION >= 310
 QCanvasItemList l = collisions(QRect(x * BO_TILE_SIZE, y * BO_TILE_SIZE,
			BO_TILE_SIZE, BO_TILE_SIZE));
#else
 QCanvasItemList l = collisions(QRect(x * BO_TILE_SIZE, y * BO_TILE_SIZE,
			BO_TILE_SIZE-1, BO_TILE_SIZE-1));
#endif
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
	list.append(u);
 }
 return list;
}

bool BosonCanvas::cellOccupied(int x, int y)
{
 if (!cell(x, y)) {
	return true;
 }
 return !(unitsAtCell(x, y).isEmpty());
 // alternative version (faster but duplicated code):
 /*
// qt bug (confirmed). will be fixed in 3.1
#if QT_VERSION >= 310
 QCanvasItemList list = collisions(QRect(x * BO_TILE_SIZE, y * BO_TILE_SIZE,
		BO_TILE_SIZE, BO_TILE_SIZE));
#else
 QCanvasItemList list = collisions(QRect(x * BO_TILE_SIZE, y * BO_TILE_SIZE,
		BO_TILE_SIZE-1, BO_TILE_SIZE-1));
#endif
 if(list.isEmpty()) {
	return false;
 }
 for (unsigned int i = 0; i < list.count(); i++) {
	if (!RTTI::isUnit(list[i]->rtti()))
		continue;
	Unit* u = (Unit*)list[i];
	if(u->isDestroyed())
		continue;
	if(u->isFlying())
		continue;
	return true;
 }
 return false;
 */
}

