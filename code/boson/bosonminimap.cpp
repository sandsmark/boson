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
#include "bosonconfig.h"
#include "unit.h"
#include "player.h"
#include "defines.h"

#include <kdebug.h>
#include <klocale.h>

#include <qpixmap.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include "bosonminimap.moc"

#define COLOR_UNKNOWN black // fog of war
#define ZOOM_STEP 0.5

class BosonMiniMap::BosonMiniMapPrivate
{
public:
	BosonMiniMapPrivate()
	{
		mMapWidth = -1;
		mMapHeight = -1;

		mPixmap = 0;
		mZoomIn = 0;
		mZoomOut = 0;
		mZoomDefault = 0;
	}

	int mMapWidth;
	int mMapHeight;

	QPointArray mSelectionRect;

	double mScale;
	double mZoom;
	double mPainterMoveX;
	double mPainterMoveY;

	QWidget* mPixmap;
	QPushButton* mZoomIn;
	QPushButton* mZoomOut;
	QPushButton* mZoomDefault;
};

BosonMiniMap::BosonMiniMap(QWidget* parent) : QWidget(parent)
{
 d = new BosonMiniMapPrivate;
 mGround = 0;
 mUnZoomedGround = 0;
 mMap = 0;
 mLocalPlayer = 0;
 mCanvas = 0;
 mUseFog = false;
 d->mScale = 1.0;
 d->mZoom = 1.0;
 d->mPainterMoveX = 0;
 d->mPainterMoveY = 0;

 setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

 QGridLayout* grid  = new QGridLayout(this, 5, 3);
 d->mPixmap = new QWidget(this);
 d->mPixmap->installEventFilter(this);
 grid->addMultiCellWidget(d->mPixmap, 0, 5, 0, 1);
 d->mZoomIn = new QPushButton(this);
 d->mZoomIn->setText(i18n("Zoom In"));
 grid->addWidget(d->mZoomIn, 0, 2);
 d->mZoomOut = new QPushButton(this);
 d->mZoomOut->setText(i18n("Zoom Out"));
 grid->addWidget(d->mZoomOut, 2, 2);
 d->mZoomDefault = new QPushButton(this);
 d->mZoomDefault->setText(i18n("Zoom Default"));
 grid->addWidget(d->mZoomDefault, 4, 2);

 d->mSelectionRect.resize(8);

 connect(d->mZoomIn, SIGNAL(clicked()), this, SLOT(slotZoomIn()));
 connect(d->mZoomOut, SIGNAL(clicked()), this, SLOT(slotZoomOut()));
 connect(d->mZoomDefault, SIGNAL(clicked()), this, SLOT(slotZoomDefault()));
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
 if (!w && !h) {
	return;
 }
 if(mGround) {
	delete mGround;
	mGround = 0;
 }
 d->mMapWidth = w;
 d->mMapHeight = h;
 mUnZoomedGround = new QPixmap(w, h);
 mUnZoomedGround->fill(COLOR_UNKNOWN);
 createGround();

 d->mPixmap->setFixedWidth(mapWidth());
 d->mPixmap->setFixedHeight(mapHeight());
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
	kdError() << k_funcinfo << "map not yet created" << endl;
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
// d->mPixmap->repaint(false);// performance - units will be added and therefore repaint()
// is already called
}

void BosonMiniMap::setPoint(int x, int y, const QColor& color)
{
 if (!ground()) {
	kdError() << k_funcinfo << "NULL ground" << endl;
	return;
 }
 QPainter p;
 QPainter p2;
 p.begin(ground());
 p.setPen(color);
 p.setBrush(color);
 p2.begin(mUnZoomedGround);
 p2.setPen(color);
 int pointsize = (int)(d->mScale * d->mZoom);
 if (color.isValid()) {
	p.drawRect(x * pointsize, y * pointsize, pointsize, pointsize);
	p2.drawPoint(x, y);
 } else {
	kdWarning() << k_funcinfo << "invalid color" << endl;
	p.setPen(COLOR_UNKNOWN);
	p2.setPen(COLOR_UNKNOWN);
	p.drawRect(x * pointsize, y * pointsize, pointsize, pointsize);
	p2.drawPoint(x, y);
 }
 p.end();
 p2.end();
}

void BosonMiniMap::mousePressEvent(QMouseEvent *e)
{
 if (e->type() != QEvent::MouseButtonPress) {
	return;
 }
 if (e->button() == LeftButton) {
	emit signalReCenterView( QPoint((int)(e->pos().x() / (scale() * zoom()) - d->mPainterMoveX), (int)(e->pos().y() / (scale() * zoom()) - d->mPainterMoveY)) );
	e->accept();
	return;
 } else if (e->button() == RightButton) {
	emit signalMoveSelection((int)(e->pos().x() / (scale() * zoom()) - d->mPainterMoveX),
			(int)(e->pos().y() / (scale() * zoom()) - d->mPainterMoveY));
 }
}

void BosonMiniMap::slotAddUnit(Unit* unit, int x, int y)
{
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 if(!mLocalPlayer) {
	return;
 }
 // x and y are pixel coordinates
 x = x / BO_TILE_SIZE;
 y = y / BO_TILE_SIZE;
 if (mLocalPlayer->isFogged(x, y)) {
	return;
 }
 QColor color = unit->owner()->teamColor();
 if (unit->isFacility()) {
	setPoint(x, y, color);
 } else {
	setPoint(x, y, color);
 }
 d->mPixmap->repaint(false);
}

void BosonMiniMap::slotMoveRect(const QPoint& topLeft, const QPoint& topRight, const QPoint& bottomLeft, const QPoint& bottomRight)
{
 d->mSelectionRect.setPoint(0, topLeft);
 d->mSelectionRect.setPoint(1, topRight);

 d->mSelectionRect.setPoint(2, topRight);
 d->mSelectionRect.setPoint(3, bottomRight);

 d->mSelectionRect.setPoint(4, bottomRight);
 d->mSelectionRect.setPoint(5, bottomLeft);

 d->mSelectionRect.setPoint(6, bottomLeft);
 d->mSelectionRect.setPoint(7, topLeft);

 d->mPixmap->repaint(false);
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
	kdError() << k_funcinfo << "NULL map" << endl;
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

void BosonMiniMap::slotMoveUnit(Unit* unit, float oldX, float oldY)
{
 if (!mMap) {
	kdError() << k_funcinfo << "NULL map" << endl;
	return;
 }
 if (!unit) {
	kdError() << k_funcinfo << "NULL unit" << endl;
	return;
 }
 if(!mLocalPlayer) {
	return;
 }
 int x = (int)(oldX / BO_TILE_SIZE);
 int y = (int)(oldY / BO_TILE_SIZE);
 if((x == (int)(unit->x() / BO_TILE_SIZE)) && (y == (int)(unit->y() / BO_TILE_SIZE))) {
	// Unit is still on the same cell. Don't update (performance)
	return;
 }
 if (!mLocalPlayer->isFogged(x, y)) {
	Cell* c = mMap->cell(x, y);
	if (!c) {
		kdError() << k_funcinfo << "NULL cell" << endl;
		return;
	}
	slotAddCell(x, y, c->groundType(), c->version());
 }
 x = (int)unit->x();
 y = (int)unit->y();
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
	kdError() << k_funcinfo << "NULL cell" << endl;
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
	slotAddUnit(u, (int)u->x(), (int)u->y());
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

double BosonMiniMap::scale() const
{
 return d->mScale;
}

double BosonMiniMap::zoom() const
{
 return d->mZoom;
}

void BosonMiniMap::slotShowMap(bool s)
{
 if (s) {
	show();
 } else {
	hide();
 }
}

void BosonMiniMap::slotZoomIn()
{
 if (boConfig->miniMapZoom() + ZOOM_STEP > 3.0) {
	return;
 }
 boConfig->setMiniMapZoom(boConfig->miniMapZoom() + ZOOM_STEP);
 d->mPixmap->repaint();
}

void BosonMiniMap::slotZoomOut()
{
 if (boConfig->miniMapZoom() - ZOOM_STEP <= 0.1) {
	return;
 }
 boConfig->setMiniMapZoom(boConfig->miniMapZoom() - ZOOM_STEP);
 d->mPixmap->repaint();
}

void BosonMiniMap::slotZoomDefault()
{
 boConfig->setMiniMapZoom(1.0);
 d->mPixmap->repaint();
}

bool BosonMiniMap::eventFilter(QObject* o, QEvent* e)
{
 if (e->type() != QEvent::Paint) {
	return QWidget::eventFilter(o, e);
	
 }
 if (!ground()) {
	return QWidget::eventFilter(o, e);
 }
 if (scale() != boConfig->miniMapScale()
		|| zoom() != boConfig->miniMapZoom()) {
	d->mScale = boConfig->miniMapScale();
	d->mZoom = boConfig->miniMapZoom();
	d->mPixmap->setFixedWidth((int)(mapWidth() * scale()));
	d->mPixmap->setFixedHeight((int)(mapHeight() * scale()));
	d->mPixmap->erase();

	// Resize pixmap (slow)
	createGround();
 }

 // it might be possible that the minimap is bigger than the size of the
 // displayed map. Then we can display only a part of the pixmap, but we must
 // ensure that the selectionRect is inside this part.
 double moveX[4];
 double moveY[4];
 for (int i = 0; i < 8; i += 2) {
	QPoint p = d->mSelectionRect.point(i);
	if (p.x() + mapWidth() / zoom() <= mapWidth()) {
		// the point is outside of the display map. move the displayed
		// map
		moveX[i / 2] = -p.x();
	} else {
		// hmm.. AB: why not 0 ? // FIXME
		// 0 doesn't work .. why?
		moveX[i / 2] = -(mapWidth() - mapWidth() / zoom());
	}
	if (p.y() + mapHeight() / zoom() <= mapWidth()) {
		// see above.
		moveY[i / 2] = -p.y();
	} else {
		moveY[i / 2]  = -(mapHeight() - mapHeight() / zoom());
	}
 }
 
 // TODO: there are four different points which might be out of the display.
 // check all of them! remeber that it might be that if we move the display so
 // that the third point can be displayed the second might be out of the
 // display then!
 d->mPainterMoveX = moveX[0];
 d->mPainterMoveY = moveY[0];
 
 // Using bitBlt() is MUCH faster than using QPainter::drawPixmap(), especially
 //  when you scale QPainter (difference may be hundreds of times)
 bitBlt(d->mPixmap, 0, 0, ground(), (int)-(d->mPainterMoveX * scale() * zoom()), 
		(int)-(d->mPainterMoveY * scale() * zoom()),
		d->mPixmap->width(), d->mPixmap->height());


 // the little rectangle
 QPainter p;
 p.begin(d->mPixmap);
 p.scale(scale(), scale());
 p.scale(zoom(), zoom());
 p.translate(d->mPainterMoveX, d->mPainterMoveY); // moves the coordinate system, *not* the objects' coordinate system (i.e. different to OpenGL translates)
 p.setPen(white);
 p.setRasterOp(XorROP);
 p.drawLineSegments(d->mSelectionRect);

 p.end();
 return QWidget::eventFilter(o, e);
}

void BosonMiniMap::createGround()
{
 if(mGround) {
	delete mGround;
 }
 mGround = new QPixmap((int)(mapWidth() * zoom() * scale()), (int)(mapHeight() * zoom() * scale()));
 QPainter p;
 p.begin(mGround);
 p.scale(zoom() * scale(), zoom() * scale());
 p.drawPixmap(0, 0, *mUnZoomedGround);
 p.end();
}

