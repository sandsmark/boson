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

#include "bosonminimap.h"

#include "cell.h"
#include "bosonmap.h"
#include "speciestheme.h"
#include "unit.h"
#include "defines.h"

#include <kdebug.h>

#include <qpixmap.h>
#include <qpainter.h>

#include "bosonminimap.moc"

#define COLOR_UNKNOWN black // should never appear

class BosonMiniMapPrivate
{
public:
	BosonMiniMapPrivate()
	{
		mGround = 0;

		mMapWidth = -1;
		mMapHeight = -1;

		mMap = 0;
	}

	QPixmap* mGround;
	int mMapWidth;
	int mMapHeight;

	QSize mSize;
	QPoint mPos;

	BosonMap* mMap;
};

BosonMiniMap::BosonMiniMap(QWidget* parent) : QWidget(parent)
{
 d = new BosonMiniMapPrivate;
 setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
}

BosonMiniMap::~BosonMiniMap()
{
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
 return d->mGround;
}

void BosonMiniMap::slotCreateMap(int w, int h)
{
 if (w == mapWidth() && h == mapHeight()) {
	return;
 }
 if (d->mGround) {
	delete d->mGround;
 }
 d->mMapWidth = w;
 d->mMapHeight = h;
 d->mGround = new QPixmap(mapWidth(), mapHeight());
 d->mGround->fill(black);

 setMinimumWidth(mapWidth() + 5);
 setMinimumHeight(mapHeight() + 5);
 updateGeometry();
}

void BosonMiniMap::slotAddCell(int x, int y, int groundType, unsigned char)
{
 if (x < 0 || x >= mapWidth()) {
//	kdError() << k_funcinfo << ": invalid cell! x=" << x << endl;
	return;
 }
 if (y < 0 || y >= mapHeight()) {
//	kdError() << k_funcinfo << ": invalid cell! y=" << y << endl;
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
		setPoint(x, y, green);
		break;
	case Cell::GroundDesert:
		setPoint(x, y, darkYellow);
		break;
	default:
		setPoint(x, y, black);
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
 p.setPen(color);
 p.drawPoint(x, y);
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
 p.setPen(white);
 p.setRasterOp(XorROP);
 p.drawRect( QRect( d->mPos, d->mSize) ); 

 p.end();
}

void BosonMiniMap::mousePressEvent(QMouseEvent *e)
{
 if (e->button() & LeftButton) {
	emit signalReCenterView( e->pos() );
	return;
 }
}

void BosonMiniMap::slotAddUnit(Unit* unit, int x, int y)
{
 if (!unit) {
	kdError() << k_funcinfo << ": NULL unit" << endl;
	return;
 }
 SpeciesTheme* theme = unit->speciesTheme();
 QColor color;
 if (!theme) {
	kdError() << k_funcinfo << ": NULL species theme" << endl;
	color = COLOR_UNKNOWN;
 } else {
	color = theme->teamColor();
 }
 if (color == green) { // green on gren ...
	color = darkGreen;
 }
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
 d->mMap = map;
}

void BosonMiniMap::initMap()
{
 if (!d->mMap) {
	kdError() << k_funcinfo << ": NULL map" << endl;
	return;
 }
 slotCreateMap(d->mMap->width(), d->mMap->height());
 for (int i = 0; i < d->mMap->width(); i++) {
	for (int j = 0; j < d->mMap->height(); j++) {
		Cell* c = d->mMap->cell(i, j);
		if (c) {
			slotAddCell(i, j, c->groundType(), c->version());
		}
	}
 }
}

void BosonMiniMap::slotMoveUnit(Unit* unit, double oldX, double oldY)
{
 if (!d->mMap) {
	kdError() << k_funcinfo << ": NULL map" << endl;
	return;
 }
 if (!unit) {
	kdError() << k_funcinfo << ": NULL unit" << endl;
	return;
 }
 int x = (int)(oldX / BO_TILE_SIZE);
 int y = (int)(oldY / BO_TILE_SIZE);
 Cell* c = d->mMap->cell(x, y);
 if (c) {
	slotAddCell(x, y, c->groundType(), c->version()); // FIXME not yet fully working
 } else {
	kdWarning() << k_funcinfo << ": NULL cell" << endl;
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
 Cell* c = d->mMap->cell(x, y);
 if (!c) {
	kdError() << k_funcinfo << ": NULL cell" << endl;
	return;
 }
 slotAddCell(x, y, c->groundType(), c->version());
}

