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

#include "bosonminimap.h"

#include "defines.h"
#include "cell.h"
#include "bosonmap.h"
#include "bosonconfig.h"
#include "bosongroundtheme.h"
#include "boitemlist.h"
#include "unit.h"
#include "player.h"
#include "bodebug.h"
#include "bosonprofiling.h"

#include <klocale.h>
#include <kstandarddirs.h>

#include <qpixmap.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvaluelist.h>
#include <qfileinfo.h>
#include <qtooltip.h>

#include "bosonminimap.moc"

#define COLOR_UNKNOWN black // fog of war
#define ZOOM_STEP 0.5

#define MINIMAP_WIDTH 50
#define MINIMAP_HEIGHT 50

class BosonMiniMap::BosonMiniMapPrivate
{
public:
	BosonMiniMapPrivate()
	{
		mPixmap = 0;
		mZoomIn = 0;
		mZoomOut = 0;
		mZoomDefault = 0;

		mLogo = 0;

		mGroundPainter = 0;
		mUnZoomedGroundPainter = 0;
	}

	QPointArray mSelectionRect;

	double mScale;
	double mZoom;
	double mPainterMoveX;
	double mPainterMoveY;

	QWidget* mPixmap;
	QPushButton* mZoomIn;
	QPushButton* mZoomOut;
	QPushButton* mZoomDefault;

	QPixmap* mLogo;
	bool mShowMap;

	// used for initializing mostly
	QPainter* mGroundPainter;
	QPainter* mUnZoomedGroundPainter;
};

BosonMiniMap::BosonMiniMap(QWidget* parent, const char* name) : QWidget(parent, name ? name : "minimap")
{
 d = new BosonMiniMapPrivate;
 mGround = 0;
 mUnZoomedGround = 0;
 mLocalPlayer = 0;
 mMap = 0;
 mUseFog = false;
 d->mScale = 1.0;
 d->mZoom = 1.0;
 d->mPainterMoveX = 0;
 d->mPainterMoveY = 0;
 d->mShowMap = false;

 setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

 QGridLayout* grid  = new QGridLayout(this, 5, 3);
 d->mPixmap = new QWidget(this);
 this->installEventFilter(this);

 grid->addMultiCellWidget(d->mPixmap, 0, 5, 0, 1);
 d->mZoomIn = new QPushButton(this, "zoomin");
 grid->addWidget(d->mZoomIn, 0, 2);
 d->mZoomDefault = new QPushButton(this, "zoomdefault");
 grid->addWidget(d->mZoomDefault, 2, 2);
 d->mZoomOut = new QPushButton(this, "zoomout");
 grid->addWidget(d->mZoomOut, 4, 2);

 QToolTip::add(d->mZoomIn, i18n("Zoom in"));
 QToolTip::add(d->mZoomOut, i18n("Zoom out"));
 QToolTip::add(d->mZoomOut, i18n("Default zoom factor"));

 d->mSelectionRect.resize(8);

 setPixmapTheme(QString::fromLatin1("standard"));

 connect(d->mZoomIn, SIGNAL(clicked()), this, SLOT(slotZoomIn()));
 connect(d->mZoomOut, SIGNAL(clicked()), this, SLOT(slotZoomOut()));
 connect(d->mZoomDefault, SIGNAL(clicked()), this, SLOT(slotZoomDefault()));

 slotShowMap(false);
}

BosonMiniMap::~BosonMiniMap()
{
 delete mUnZoomedGround;
 delete mGround;
 delete d;
}

int BosonMiniMap::mapWidth() const
{
 BO_CHECK_NULL_RET0(map());
 return map()->width();
}

int BosonMiniMap::mapHeight() const
{
 BO_CHECK_NULL_RET0(map());
 return map()->height();
}

QPixmap* BosonMiniMap::ground() const
{
 return mGround;
}

void BosonMiniMap::createMap()
{
 delete mGround;
 mGround = 0;
 delete mUnZoomedGround;
 mUnZoomedGround = 0;

 BO_CHECK_NULL_RET(map());
 mUnZoomedGround = new QPixmap(mapWidth(), mapHeight());
 mUnZoomedGround->fill(COLOR_UNKNOWN);
 createGround();

// AB: the minimap size is meant to display a MINIMAP_WIDTHxMINIMAP_HEIGHT map
// completely (at default zooming/scaling factor), for all bigger maps we
// display a part of the map only (and scroll it)
 d->mPixmap->setFixedWidth((int)(MINIMAP_WIDTH * DEFAULT_MINIMAP_SCALE));
 d->mPixmap->setFixedHeight((int)(MINIMAP_HEIGHT * DEFAULT_MINIMAP_SCALE));
 updateGeometry();
}

void BosonMiniMap::slotUpdateCell(int x, int y)
{
 updateCell(x, y);
 repaintMiniMapPixmap();
}

void BosonMiniMap::calculateGround(int x, int y)
{
 BO_CHECK_NULL_RET(ground());
 BO_CHECK_NULL_RET(map());
 BO_CHECK_NULL_RET(map()->texMap());
 if (x < 0 || x >= mapWidth()) {
	return;
 }
 if (y < 0 || y >= mapHeight()) {
	return;
 }

 // every cell has four corners - we mix them together to get the actual minimap
 // color.
 unsigned int cornerX[4] = { x, x + 1, x + 1,     x };
 unsigned int cornerY[4] = { y,     y, y + 1, y + 1 };
 int r = 0;
 int g = 0;
 int b = 0;
 for (int j = 0; j < 4; j++) {
	int alphaSum = 0; // sum of all textures
	int cornerRed = 0;
	int cornerGreen = 0;
	int cornerBlue = 0;
	for (unsigned int i = 0; i < map()->groundTheme()->textureCount(); i++) {
		int alpha = (int)map()->texMapAlpha(i, cornerX[j], cornerY[j]);
		alphaSum += alpha;

		QRgb rgb = map()->miniMapColor(i);
		int red = qRed(rgb);
		int green = qGreen(rgb);
		int blue = qBlue(rgb);
		cornerRed += red * alpha / 255;
		cornerGreen += green * alpha / 255;
		cornerBlue += blue * alpha / 255;
	}
	if (alphaSum == 0) {
		// nothing to do for this corner.
		continue;
	}
	cornerRed = cornerRed * 255 / alphaSum;
	cornerGreen = cornerGreen * 255 / alphaSum;
	cornerBlue = cornerBlue * 255 / alphaSum;

	r += cornerRed;
	g += cornerGreen;
	b += cornerBlue;
 }

 r /= 4;
 g /= 4;
 b /= 4;

 setPoint(x, y, QColor(r, g, b));
}

void BosonMiniMap::setPoint(int x, int y, const QColor& color)
{
 // AB: according to cachegrind setPen() and setBrush() take each 37% of the
 // time spent in this function!
 // setPoint() is close to time critical (only close), so we might want to cache
 // that or so.
 BO_CHECK_NULL_RET(ground());
 if (!color.isValid()) {
	boWarning() << k_funcinfo << "invalid color" << endl;
	setPoint(x, y, COLOR_UNKNOWN);
	return;
 }
 QPainter* groundPainter;
 QPainter* unZoomedGroundPainter;
 if (d->mGroundPainter && d->mUnZoomedGroundPainter) {
	groundPainter = d->mGroundPainter;
	unZoomedGroundPainter = d->mUnZoomedGroundPainter;
 } else {
	groundPainter = new QPainter(ground());
	unZoomedGroundPainter = new QPainter(mUnZoomedGround);
 }
 groundPainter->setPen(color);
 groundPainter->setBrush(color);
 unZoomedGroundPainter->setPen(color);
 int pointsize = (int)(d->mScale * d->mZoom);
 groundPainter->drawRect(x * pointsize, y * pointsize, pointsize, pointsize);
 unZoomedGroundPainter->drawPoint(x, y);
 if (!d->mGroundPainter) {
	delete groundPainter;
	delete unZoomedGroundPainter;
 }
}

void BosonMiniMap::mousePressEvent(QMouseEvent *e)
{
 if (e->type() != QEvent::MouseButtonPress) {
	return;
 }
 if (!d->mShowMap) {
	return;
 }
 if (e->pos().x() >= d->mPixmap->width() || e->pos().y() >= d->mPixmap->height() || e->pos().x() < 0 || e->pos().y() < 0) {
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
 // AB: obsolete. units will move anyway (and therefore call slotMoveUnit)
}

void BosonMiniMap::slotMoveUnit(Unit* unit, float oldX, float oldY)
{
 BO_CHECK_NULL_RET(unit);
 QPtrVector<Cell> newCells;
 QPtrVector<Cell> oldCells;
 makeCellList(&newCells, unit, unit->x(), unit->y());
 makeCellList(&oldCells, unit, oldX, oldY);
 moveUnit(unit, &newCells, &oldCells);
}

void BosonMiniMap::moveUnit(Unit* unit, const QPtrVector<Cell>* newCells, const QPtrVector<Cell>* oldCells)
{
 // all parameters use cell coordinates!
 // note that using unit->x() and unit->y() as well as unit->cells() and such
 // stuff can be undefined at this point! especially when adding units
 // (oldX==oldY==-1)!
 BO_CHECK_NULL_RET(mLocalPlayer);
 BO_CHECK_NULL_RET(unit);
 BO_CHECK_NULL_RET(map());
 BO_CHECK_NULL_RET(newCells);
 if (oldCells && newCells->count() == oldCells->count()) {
	if (!unit->isDestroyed()) {
		bool moved = false;
		for (unsigned int i = 0; i < newCells->count() && !moved; i++) {
			if (newCells->at(i) != oldCells->at(i)) {
				moved = true;
			}
		}
		if (!moved) {
			// Unit is still on the same cells. Don't update (performance)
			return;
		}
	} else {
		// don't return - probably remove unit from the minimap
	}
 }

 if (oldCells && oldCells->count() != 0) {
	// unit is moving.
	// pretty much everything can happen here now. the cell that the unit
	// left can be fogged for the local player, can have another unit, ...
	// so we need to update all cells that the unit has left.
	for (unsigned int i = 0; i < oldCells->count(); i++) {
		bool found = false;
		for (unsigned int j = 0; j < newCells->count(); j++) {
			if (newCells->at(j) == oldCells->at(i)) {
				found = true;
				break;
			}
		}
		if (!found) {
			Cell* c = oldCells->at(i);
			if (c) {
				updateCell(c->x(), c->y());
			}
		}
	}
 }
 QColor color = unit->owner()->teamColor();
 for (unsigned int i = 0; i < newCells->count(); i++) {
	Cell* c = newCells->at(i);
	if (!c) {
		continue;
	}
	int x = c->x();
	int y = c->y();
	if (mLocalPlayer && mLocalPlayer->isFogged(x, y)) {
		// we don't call slotFog() here now, as it should be fogged
		// already. maybe we should do this anyway?
		continue;
	}
	if (!unit->isDestroyed()) {
		setPoint(x, y, color);
	} else {
		updateCell(x, y);
	}
 }
 repaintMiniMapPixmap();
}

void BosonMiniMap::updateCell(int x, int y)
{
 BO_CHECK_NULL_RET(map());
 BO_CHECK_NULL_RET(ground());
 Cell* cell = map()->cell(x, y);
 if (!cell) {
	boError() << k_funcinfo << x << "," << y << " is no valid cell!" << endl;
	return;
 }
 // AB: note that mLocalPlayer == NULL is valid in editor mode here!
 if (mLocalPlayer) {
	if (mLocalPlayer->isFogged(x, y)) {
		slotFog(x, y);
		return;
	}
 }
 QValueList<Unit*> list = cell->items()->units(false);
 if (list.isEmpty()) {
	calculateGround(x, y);
 } else {
	Unit* u = list.first();
	QPtrVector<Cell> cells;
	makeCellList(&cells, u, u->x(), u->y());
	moveUnit(u, &cells, 0);
 }
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

 repaintMiniMapPixmap();
}

void BosonMiniMap::setMap(BosonMap* m)
{
 if (!m) {
	boWarning() << k_funcinfo << "NULL map" << endl;
 }
 mMap = m;
}

BosonMap* BosonMiniMap::map() const
{
 return mMap;
}

void BosonMiniMap::initMap()
{
 if (!map()) {
	createMap();
	repaintMiniMapPixmap();
	return;
 }
 BO_CHECK_NULL_RET(map());
 setUpdatesEnabled(false);
 createMap();
 setUpdatesEnabled(true);
// repaintMiniMapPixmap();
}

void BosonMiniMap::slotUnitDestroyed(Unit* unit)
{
 BO_CHECK_NULL_RET(unit)
 QPtrVector<Cell> cells;
 makeCellList(&cells, unit, unit->x(), unit->y());
 moveUnit(unit, &cells, 0);
}

void BosonMiniMap::slotUnfog(int x, int y)
{
 BO_CHECK_NULL_RET(map());
 if (!mUseFog) {
	return;
 }
 Cell* cell = map()->cell(x, y);
 if (!cell) {
	boError() << k_funcinfo << "invalid cell " << x << "," << y << endl;
	return;
 }
 QValueList<Unit*> list = cell->items()->units(false);
 if (!list.isEmpty()) {
	Unit* u = list.first();
	QPtrVector<Cell> cells;
	makeCellList(&cells, u, u->x(), u->y());
	moveUnit(u, &cells, 0);
 } else {
	calculateGround(x, y);
 }
}

void BosonMiniMap::slotFog(int x, int y)
{
 BO_CHECK_NULL_RET(map());
 if (!mUseFog) {
	return;
 }
 Cell* cell = map()->cell(x, y);
 if (!cell) {
	boError() << k_funcinfo << "invalid cell " << x << "," << y << endl;
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
 BO_CHECK_NULL_RET(map());

 // a few performance tricks...
 setUpdatesEnabled(false);
 delete d->mGroundPainter;
 d->mGroundPainter = new QPainter(ground());
 delete d->mUnZoomedGroundPainter;
 d->mUnZoomedGroundPainter = new QPainter(mUnZoomedGround);
 if (!p) {
	// a NULL player means that we should display the complete map. fog of
	// war gets disabled.
	mUseFog = true; // needs to be enabled to initialize the map...
	for (unsigned int i = 0; i < map()->width(); i++) {
		for (unsigned int j = 0; j < map()->height(); j++) {
			slotUnfog(i, j);
		}
	}
	mUseFog = false;
 } else {
	mUseFog = true;
	for (unsigned int i = 0; i < map()->width(); i++) {
		for (unsigned int j = 0; j < map()->height(); j++) {
			if (p && p->isFogged(i, j)) {
				slotFog(i, j);
			} else {
				slotUnfog(i, j);
			}
		}
	}
 }
 delete d->mGroundPainter;
 d->mGroundPainter = 0;
 delete d->mUnZoomedGroundPainter;
 d->mUnZoomedGroundPainter = 0;
 setUpdatesEnabled(true);

 repaintMiniMapPixmap();
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
 d->mZoomIn->setEnabled(s);
 d->mZoomOut->setEnabled(s);
 d->mZoomDefault->setEnabled(s);
 d->mShowMap = s;
 repaintMiniMapPixmap();
}

void BosonMiniMap::slotZoomIn()
{
 if (boConfig->miniMapZoom() + ZOOM_STEP > 3.0) {
	return;
 }
 boConfig->setMiniMapZoom(boConfig->miniMapZoom() + ZOOM_STEP);
 repaintMiniMapPixmap();
}

void BosonMiniMap::slotZoomOut()
{
 if (boConfig->miniMapZoom() - ZOOM_STEP <= 0.1) {
	return;
 }
 boConfig->setMiniMapZoom(boConfig->miniMapZoom() - ZOOM_STEP);
 repaintMiniMapPixmap();
}

void BosonMiniMap::slotZoomDefault()
{
 boConfig->setMiniMapZoom(1.0);
 repaintMiniMapPixmap();
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
	d->mPixmap->erase();

	// Resize pixmap (slow)
	createGround();
 }

 if (!d->mShowMap) {
	// don't show the map, but the logo only.
	if (!d->mLogo || d->mLogo->isNull()) {
		boWarning() << k_funcinfo << "oops - invalid logo for minimap" << endl;
		return QWidget::eventFilter(o, e);
	}
	bitBlt(d->mPixmap, 0, 0, d->mLogo, 0, 0);

	return QWidget::eventFilter(o, e);
 }

 // it might be possible that the minimap is bigger than the size of the
 // displayed map. Then we can display only a part of the pixmap, but we must
 // ensure that the selectionRect is inside this part.
 double moveX[4];
 double moveY[4];
 for (int i = 0; i < 8; i += 2) {
	QPoint p = d->mSelectionRect.point(i);
	if (p.x() + MINIMAP_WIDTH / zoom() < mapWidth()) {
		moveX[i / 2] = -p.x();
	} else {
		moveX[i / 2] = -(mapWidth() - MINIMAP_WIDTH / zoom());
	}
	if (p.y() + MINIMAP_HEIGHT / zoom() < mapHeight()) {
		moveY[i / 2] = -p.y();
	} else {
		moveY[i / 2] = -(mapHeight() - MINIMAP_HEIGHT / zoom());
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
 delete mGround;
 mGround = new QPixmap((int)(mapWidth() * zoom() * scale()), (int)(mapHeight() * zoom() * scale()));
 QPainter p;
 p.begin(mGround);
 p.scale(zoom() * scale(), zoom() * scale());
 if (mUnZoomedGround) {
	p.drawPixmap(0, 0, *mUnZoomedGround);
 }
 p.end();
}

void BosonMiniMap::makeCellList(QPtrVector<Cell>* cells, const Unit* unit, float x, float y)
{
 BO_CHECK_NULL_RET(unit);
 BO_CHECK_NULL_RET(map());
 int left, right, top, bottom;
 BosonItem::leftTopCell(&left, &top, x, y);
 BosonItem::rightBottomCell(&right, &bottom, x + unit->width() - 1, y + unit->height() - 1);
 right = QMIN(right, QMAX((int)mapWidth() - 1, 0));
 bottom = QMIN(bottom, QMAX((int)mapHeight() - 1, 0));
 return BosonItem::makeCells(map()->cells(), cells, left, right, top, bottom, map()->width(), map()->height());
}

void BosonMiniMap::setPixmapTheme(const QString& theme)
{
 QPixmap pixmap = pixmapFromTheme(QString::fromLatin1("minimap-logo.png"), theme);
 if (pixmap.isNull()) {
	boError() << k_funcinfo << "Could not load minimap-logo.png from " << theme << endl;
	if (!d->mLogo) {
		// create a dummy pixmap to avoid a crash
		d->mLogo = new QPixmap(100, 100);
		d->mLogo->fill(Qt::red);
	}
 } else {
	delete d->mLogo;
	d->mLogo = new QPixmap(pixmap);
 }

 pixmap = pixmapFromTheme(QString::fromLatin1("minimap-zoom-in.png"), theme);
 if (pixmap.isNull()) {
	boError() << k_funcinfo << "Could not load minimap-zoom-in.png from " << theme << endl;
	// we don't set a dummy pixmap here
 } else {
	d->mZoomIn->setPixmap(pixmap);
 }

 pixmap = pixmapFromTheme(QString::fromLatin1("minimap-zoom-out.png"), theme);
 if (pixmap.isNull()) {
	boError() << k_funcinfo << "Could not load minimap-zoom-out.png from " << theme << endl;
	// we don't set a dummy pixmap here
 } else {
	d->mZoomOut->setPixmap(pixmap);
 }

 pixmap = pixmapFromTheme(QString::fromLatin1("minimap-zoom-default.png"), theme);
 if (pixmap.isNull()) {
	boError() << k_funcinfo << "Could not load minimap-zoom-default.png from " << theme << endl;
	// we don't set a dummy pixmap here
 } else {
	d->mZoomDefault->setPixmap(pixmap);
 }

 if (!d->mShowMap) {
	repaintMiniMapPixmap();
 }
}

QPixmap BosonMiniMap::pixmapFromTheme(const QString& file, const QString& theme) const
{
 QString f = locate("data", QString::fromLatin1("boson/themes/ui/%1/%2").arg(theme).arg(file));
 QFileInfo info(f);
 if (!info.exists()) {
	f = locate("data", QString::fromLatin1("boson/themes/ui/%1/%2").arg(QString::fromLatin1("standard").arg(file)));
	info.setFile(f);
	if (!info.exists()) {
		boError() << k_funcinfo << "Can't find " << f << " in " << theme << " or standard" << endl;
		return QPixmap();
	}
 }
 return QPixmap(f);
}

void BosonMiniMap::repaintMiniMapPixmap()
{
 if (isUpdatesEnabled()) {
	// Do not repaint immediately, but post an event instead
	// This code is taken from QWidget::update()
	// AB: we post the event to this, but use the visible rect of the
	// pixmap.
	QApplication::postEvent(this, new QPaintEvent(d->mPixmap->visibleRect(), false));
 }
}
