/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "bosoncanvas.h"
#include "boson.h"
#include "player.h"
#include "cell.h"
#include "unit.h"
#include "bosonmap.h"
#include "speciestheme.h"
#include "unitproperties.h"
#include "boshot.h"

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <arts/soundserver.h>

#include <qintdict.h>

#include "defines.h"

#include "bosoncanvas.moc"

class BosonCanvasPrivate
{
public:
	BosonCanvasPrivate()
	{
		mMap = 0;
	}
	
	QPixmap mPix;
	QPtrList<QCanvasItem> mAnimList; // see BosonCanvas::advance()
	QPtrList<Unit> mDestroyUnits;
	QPtrList<Unit> mDestroyedUnits;

	BosonMap* mMap; // just a pointer - no memory allocated

	Arts::SimpleSoundServer* mSoundServer;
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
 // note: I do not know anything about this sound/arts stuff. It is just a
 // cut'n'paste from the original Boson code
 new Arts::Dispatcher(); // AB: what is this?
 // AB: the following code is slightly adjusted - based on the example in the
 // KDE2Development book
 d->mSoundServer = new Arts::SimpleSoundServer(Arts::Reference("global:Arts_SimpleSoundServer"));
 if (d->mSoundServer->isNull()) {
	kdWarning() << "Sound could not be initialized - sound disabled" << endl;
 }
 d->mDestroyedUnits.setAutoDelete(true);
}

BosonCanvas::~BosonCanvas()
{
// kdDebug() << k_funcinfo << endl;
 d->mDestroyUnits.clear();
 d->mDestroyedUnits.clear();
 d->mAnimList.clear();
 delete d->mSoundServer;
 delete d;
}


void BosonCanvas::slotLoadTiles(const QString& name)
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
		emit signalUnitDestroyed(unit); // currently unused
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
 slotLoadTiles(tileFile); // FIXME: rename: loadTiles
 for (int i = 0; i < d->mMap->width(); i++) {
	for (int j = 0; j < d->mMap->height(); j++) {
		Cell* c = d->mMap->cell(i, j);
		if (!c) {
			kdError() << k_funcinfo << ": NULL cell" << endl;
			continue;
		}
		slotAddCell(i, j, c->groundType(), c->version());
	}
 }
 update();
}

void BosonCanvas::slotAddCell(int x, int y, int groundType, unsigned char version)
{
 int tile = Cell::tile(groundType, version);
 if (tile < 0 || tile >= d->mMap->width() * d->mMap->height()) {
	kdWarning() << "Invalid tile " << tile << endl;
	return;
 }
// kdDebug() << "g=" << c->groundType() << ",v=" << c->version() <<
//		"==>tile=" << tile << endl;
 setTile(x, y, tile);

}

void BosonCanvas::addAnimation(QCanvasItem* item)
{
 d->mAnimList.append(item);
}

void BosonCanvas::removeAnimation(QCanvasItem* item)
{
 kdDebug() << "remove Anim" << endl;
 d->mAnimList.removeRef(item);
}

void BosonCanvas::unitMoved(Unit* unit, double oldX, double oldY)
{
// test if any unit has this unit as target. If sou then adjust the destination. 
//TODO 

// used to adjust the mini map
 emit signalUnitMoved(unit, oldX, oldY);
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
 play(attackedBy->owner()->speciesTheme()->themePath() + "sounds/shoot.wav");
}

void BosonCanvas::play(const QString& fileName)
{
 if (d->mSoundServer->isNull()) {
	return;
 }
// kdDebug() << "play " << fileName << endl;
 d->mSoundServer->play(fileName.latin1());
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
