/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonminimap.h"

#include "cell.h"
#include "bosonmap.h"
#include "bosoncanvas.h"
#include "unit.h"
#include "player.h"
#include "defines.h"

#include <kdebug.h>

#include <qpixmap.h>
#include <qpainter.h>

#include "bosonminimap.moc"

#define COLOR_UNKNOWN black // fog of war

class BosonMiniMap::BosonMiniMapPrivate
{
public:
	BosonMiniMapPrivate()
	{
		mMapWidth = -1;
		mMapHeight = -1;

		mPointSize = 0;
	}

	int mMapWidth;
	int mMapHeight;

	QSize mSize;
	QPoint mPos;

	int mPointSize;
};

BosonMiniMap::BosonMiniMap(QWidget* parent) : QWidget(parent)
{
 d = new BosonMiniMapPrivate;
 mGround = 0;
 mMap = 0;
 mLocalPlayer = 0;
 mCanvas = 0;
 mUseFog = false;

 setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
 setPointSize(2);
}

BosonMiniMap::~BosonMiniMap()
{
 delete mGround;
 delete d;
}

int BosonMiniMap::mapWidth() const
{
 return d->mMapWidth;
}

int BosonMiniMap::mapHeight() const
{
 return d->mMapHeight;
}

QPixmap* BosonMiniMap::ground() const
{
 return mGround;
}

void BosonMiniMap::slotCreateMap(int w, int h)
{
 if (w == mapWidth() && h == mapHeight()) {
	return;
 }
 delete mGround;
 d->mMapWidth = w;
 d->mMapHeight = h;
 mGround = new QPixmap(mapWidth() * pointSize(), mapHeight() * pointSize());
 mGround->fill(COLOR_UNKNOWN);

 setMinimumWidth(mGround->width() + 5);
 setMinimumHeight(mGround->height() + 5);
 updateGeometry();
}

void BosonMiniMap::slotAddCell(int x, int y, int groundType, unsigned char)
{
 if (x < 0 || x >= mapWidth()) {
	return;
 }
 if (y < 0 || y >= mapHeight()) {
	return;
 }
 if (!ground()) {
	kdError() << k_funcinfo << ": map not yet created" << endl;
	return;
 }
 switch (groundType) {
	case Cell::GroundWater:
		setPoint(x, y, blue);
		break;
	case Cell::GroundGrass:
	case Cell::GroundGrassOil:
	case Cell::GroundGrassMineral:
		setPoint(x, y, darkGreen);
		break;
	case Cell::GroundDesert:
		setPoint(x, y, darkYellow);
		break;
	default:
		setPoint(x, y, COLOR_UNKNOWN);
		break;
 }
// repaint(false);// performance - units will be added and therefore repaint()
// is already called
}

void BosonMiniMap::setPoint(int x, int y, const QColor& color)
{
 if (!ground()) {
	kdError() << k_funcinfo << ": NULL ground" << endl;
	return;
 }
 QPainter p;
 p.begin(ground());
 if (color.isValid()) {
	p.fillRect(x*pointSize(), y*pointSize(), pointSize(), pointSize(), QBrush(color));
 } else {
	kdWarning() << k_funcinfo << "invalid color" << endl;
	p.fillRect(x*pointSize(), y*pointSize(), pointSize(), pointSize(), QBrush(COLOR_UNKNOWN));
 }
 p.end();
}

void BosonMiniMap::paintEvent(QPaintEvent*)
{
 if (!ground()) {
	return;
 }
 QPainter p;
 p.begin(this);
 p.drawPixmap(0, 0, *ground());
 
 // the little rectangle
 // there seems to be a small bug - if you scroll to the lower right corner
 // there are at both sides (right + bottom) a few pixels left. the rect should
 // be at the border (frame?).
 p.setPen(white);
 p.setRasterOp(XorROP);
 p.drawRect( QRect( d->mPos * pointSize(), d->mSize * pointSize()) );

 p.end();
}

void BosonMiniMap::mousePressEvent(QMouseEvent *e)
{
 if (e->button() & LeftButton) {
	
	emit signalReCenterView( e->pos() / pointSize() );
	return;
 }
}

void BosonMiniMap::slotAddUnit(Unit* unit, int x, int y)
{
 if (!unit) {
	kdError() << k_funcinfo << ": NULL unit" << endl;
	return;
 }
 if (mLocalPlayer->isFogged(x, y)) {
	return;
 }
 QColor color = unit->owner()->teamColor();
 if (unit->isFacility()) {
	setPoint(x, y, color);
 } else {
	setPoint(x, y, color);
 }
 repaint(false);
}

void BosonMiniMap::slotMoveRect(int x, int y)
{
 d->mPos = QPoint(x / BO_TILE_SIZE, y / BO_TILE_SIZE);
 repaint(false);
}

void BosonMiniMap::slotResizeRect(int w, int h)
{
 d->mSize = QSize(w / BO_TILE_SIZE, h / BO_TILE_SIZE);
 repaint(false);
}

void BosonMiniMap::setMap(BosonMap* map)
{
 mMap = map;
}

void BosonMiniMap::setCanvas(BosonCanvas* c)
{
 mCanvas = c;
}

void BosonMiniMap::initMap()
{
 if (!mMap) {
	kdError() << k_funcinfo << ": NULL map" << endl;
	return;
 }
 slotCreateMap(mMap->width(), mMap->height());
 bool oldFog = mUseFog;
 mUseFog = true;
 for (unsigned int i = 0; i < mMap->width(); i++) {
	for (unsigned int j = 0; j < mMap->height(); j++) {
		slotUnfog(i, j);
	}
 }
 mUseFog = oldFog;
}

void BosonMiniMap::slotMoveUnit(Unit* unit, double oldX, double oldY)
{
 if (!mMap) {
	kdError() << k_funcinfo << ": NULL map" << endl;
	return;
 }
 if (!unit) {
	kdError() << k_funcinfo << ": NULL unit" << endl;
	return;
 }
 int x = (int)(oldX / BO_TILE_SIZE);
 int y = (int)(oldY / BO_TILE_SIZE);
 if (!mLocalPlayer->isFogged(x, y)) {
	Cell* c = mMap->cell(x, y);
	if (!c) {
		kdError() << k_lineinfo << "NULL cell" << endl;
		return;
	}
	slotAddCell(x, y, c->groundType(), c->version());
 }
 x = (int)(unit->x() / BO_TILE_SIZE);
 y = (int)(unit->y() / BO_TILE_SIZE);
 slotAddUnit(unit, x, y);
}

void BosonMiniMap::slotUnitDestroyed(Unit* unit)
{
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 int x = (int)(unit->x() / BO_TILE_SIZE);
 int y = (int)(unit->y() / BO_TILE_SIZE);
 Cell* c = mMap->cell(x, y);
 if (!c) {
	kdError() << k_funcinfo << ": NULL cell" << endl;
	return;
 }
 slotUnfog(x, y);
}

void BosonMiniMap::slotUnfog(int x, int y)
{
 if (!mUseFog) {
	return;
 }
 Cell* c = mMap->cell(x, y);
 if (!c) {
	kdError() << k_funcinfo << "invalid cell " << x << "," << y << endl;
	return;
 }
 slotAddCell(x, y, c->groundType(), c->version());
 QValueList<Unit*> list = mCanvas->unitsAtCell(x, y);
 if (!list.isEmpty()) {
	Unit* u = list.first();
	slotAddUnit(u, u->x() / BO_TILE_SIZE, u->y() / BO_TILE_SIZE);
 }
}

void BosonMiniMap::slotFog(int x, int y)
{
 if (!mUseFog) {
	return;
 }
 Cell* c = mMap->cell(x, y);
 if (!c) {
	kdError() << k_funcinfo << "invalid cell " << x << "," << y << endl;
	return;
 }
 setPoint(x, y, COLOR_UNKNOWN);
}

void BosonMiniMap::setLocalPlayer(Player* p)
{
 mLocalPlayer = p;
}

void BosonMiniMap::initFogOfWar(Player* p)
{
 mUseFog = true;
 for (unsigned int i = 0; i < mMap->width(); i++) {
	for (unsigned int j = 0; j < mMap->height(); j++) {
		if (p && p->isFogged(i, j)) {
			slotFog(i, j);
		} else {
			slotUnfog(i, j);
		}
	}
 }
}

void BosonMiniMap::setPointSize(int p)
{
 if (d->mPointSize == p) {
	return;
 }
 d->mPointSize = p;
 //TODO: regenerate the current map!!
 
}

int BosonMiniMap::pointSize() const
{
 return d->mPointSize;
}

void BosonMiniMap::slotShowMap(bool s)
{
 if (s) {
	show();
 } else {
	hide();
 }
}
