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

#include "defines.h"
#include "cell.h"
#include "bosonmap.h"
#include "bosoncanvas.h"
#include "bosonconfig.h"
#include "unit.h"
#include "player.h"
#include "bodebug.h"

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
};

BosonMiniMap::BosonMiniMap(QWidget* parent) : QWidget(parent)
{
 d = new BosonMiniMapPrivate;
 mGround = 0;
 mUnZoomedGround = 0;
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
 QToolTip::add(d->mZoomIn, i18n("Zoom in"));
 grid->addWidget(d->mZoomIn, 0, 2);
 d->mZoomOut = new QPushButton(this);
 QToolTip::add(d->mZoomOut, i18n("Zoom out"));
 grid->addWidget(d->mZoomOut, 2, 2);
 d->mZoomDefault = new QPushButton(this);
 QToolTip::add(d->mZoomOut, i18n("Default zoom factor"));
 grid->addWidget(d->mZoomDefault, 4, 2);

 d->mSelectionRect.resize(8);

 setPixmapTheme(QString::fromLatin1("standard"));
 d->mShowMap = false;

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
 BO_CHECK_NULL_RET0(map())
 return map()->width();
}

int BosonMiniMap::mapHeight() const
{
 BO_CHECK_NULL_RET0(map())
 return map()->height();
}

QPixmap* BosonMiniMap::ground() const
{
 return mGround;
}

void BosonMiniMap::createMap()
{
 BO_CHECK_NULL_RET(map())
 delete mGround;
 mGround = 0;
 mUnZoomedGround = new QPixmap(mapWidth(), mapHeight());
 mUnZoomedGround->fill(COLOR_UNKNOWN);
 createGround();

 d->mPixmap->setFixedWidth(mapWidth());
 d->mPixmap->setFixedHeight(mapHeight());
 updateGeometry();
}

void BosonMiniMap::slotChangeCell(int x, int y, int groundType, unsigned char version)
{
 if (!ground()) {
	boError() << k_funcinfo << "map not yet created" << endl;
	return;
 }
 BO_CHECK_NULL_RET(mCanvas)
 if (!mCanvas->cell(x, y)) {
	boError() << k_funcinfo << x << "," << y << " is no valid cell!" << endl;
	return;
 }
 // AB: note that mLocalPlayer == NULL is valid in editor mode here!
 if (mLocalPlayer && mLocalPlayer->isFogged(x, y)) {
	// we can't see this cell
	return;
 }
 if (mCanvas->findUnitAtCell(x, y)) {
	// there is a unit on the cell, so do not paint the cell.
	return;
 }
 changeCell(x, y, groundType, version);

 // we need to repaint now - especially in editor mode when a cell was changed.
 d->mPixmap->repaint(false);// performance - units will be added and therefore repaint()
}

void BosonMiniMap::changeCell(int x, int y, int groundType, unsigned char)
{
 if (!ground()) {
	boError() << k_funcinfo << "map not yet created" << endl;
	return;
 }
 if (x < 0 || x >= mapWidth()) {
	return;
 }
 if (y < 0 || y >= mapHeight()) {
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
// d->mPixmap->repaint(false);// performance - this gets called for map init -
// more will happen with the mini map anyway, so it'll be repainted later
}

void BosonMiniMap::setPoint(int x, int y, const QColor& color)
{
 BO_CHECK_NULL_RET(ground())
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
	boWarning() << k_funcinfo << "invalid color" << endl;
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
 // x and y are canvas coordinates
 moveUnit(unit, makeCellList(unit, x, y), QPointArray());
}

void BosonMiniMap::slotMoveUnit(Unit* unit, float oldX, float oldY)
{
 BO_CHECK_NULL_RET(unit)
 QPointArray newCells = makeCellList(unit, unit->x(), unit->y());
 QPointArray oldCells = makeCellList(unit, oldX, oldY);
 moveUnit(unit, newCells, oldCells);
}

void BosonMiniMap::moveUnit(Unit* unit, const QPointArray& newCells, const QPointArray& oldCells)
{
 // all parameters use cell coordinates!
 // note that using unit->x() and unit->y() as well as unit->cells() and such
 // stuff can be undefined at this point! especially when adding units
 // (oldX==oldY==-1)!
 BO_CHECK_NULL_RET(mLocalPlayer)
 BO_CHECK_NULL_RET(mCanvas)
 BO_CHECK_NULL_RET(unit)
 BO_CHECK_NULL_RET(map())
 if (newCells.count() == oldCells.count()) {
	if (!unit->isDestroyed()) {
		bool moved = false;
		for (unsigned int i = 0; i < newCells.count() && !moved; i++) {
			if (newCells[i] != oldCells[i]) {
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

 if (oldCells.count() != 0) {
	// unit is moving.
	// pretty much everything can happen here now. the cell that the unit
	// left can be fogged for the local player, can have another unit, ...
	// so we need to update all cells that the unit has left.
	for (unsigned int i = 0; i < oldCells.count(); i++) {
		bool found = false;
		for (unsigned int j = 0; j < newCells.count(); j++) {
			if (newCells[j] == oldCells[i]) {
				found = true;
				break;
			}
		}
		if (!found) {
			updateCell(oldCells[i].x(), oldCells[i].y());
		}
	}
 }
 QColor color = unit->owner()->teamColor();
 for (unsigned int i = 0; i < newCells.count(); i++) {
	int x = newCells[i].x();
	int y = newCells[i].y();
	if (!mCanvas->cell(x, y)) {
		continue;
	}
	if (mLocalPlayer && mLocalPlayer->isFogged(x, y)) {
		// we don't call slotFog() here now, as it should be fogged
		// already. maybe we should do this anyway?
		continue;
	}
	setPoint(x, y, color);
 }
 d->mPixmap->repaint(false);
}

void BosonMiniMap::updateCell(int x, int y)
{
 BO_CHECK_NULL_RET(map())
 BO_CHECK_NULL_RET(ground())
 BO_CHECK_NULL_RET(mCanvas)
 if (!map()->cell(x, y)) {
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
 Cell* cell = map()->cell(x, y);
 if (!cell) {
	return;
 }
 QValueList<Unit*> list = mCanvas->unitsAtCell(x, y);
 if (list.isEmpty()) {
	changeCell(x, y, cell->groundType(), cell->version());
 } else {
	setPoint(x, y, list.first()->owner()->teamColor());
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

 d->mPixmap->repaint(false);
}

void BosonMiniMap::setCanvas(BosonCanvas* c)
{
 mCanvas = c;
}

BosonMap* BosonMiniMap::map() const
{
 if (!mCanvas) {
	return 0;
 }
 return mCanvas->map();
}

void BosonMiniMap::initMap()
{
 BO_CHECK_NULL_RET(map())
 createMap();
 bool oldFog = mUseFog;
 mUseFog = true;
 for (unsigned int i = 0; i < map()->width(); i++) {
	for (unsigned int j = 0; j < map()->height(); j++) {
		slotUnfog(i, j);
	}
 }
 mUseFog = oldFog;
}

void BosonMiniMap::slotUnitDestroyed(Unit* unit)
{
 BO_CHECK_NULL_RET(unit)
 BO_CHECK_NULL_RET(map())
 int x = (int)(unit->x() / BO_TILE_SIZE);
 int y = (int)(unit->y() / BO_TILE_SIZE);
 Cell* cell = map()->cell(x, y);
 BO_CHECK_NULL_RET(cell)
 slotUnfog(x, y);
}

void BosonMiniMap::slotUnfog(int x, int y)
{
 BO_CHECK_NULL_RET(map())
 if (!mUseFog) {
	return;
 }
 Cell* cell = map()->cell(x, y);
 if (!cell) {
	boError() << k_funcinfo << "invalid cell " << x << "," << y << endl;
	return;
 }
 QValueList<Unit*> list = mCanvas->unitsAtCell(x, y);
 if (!list.isEmpty()) {
	Unit* u = list.first();
	slotAddUnit(u, (int)u->x(), (int)u->y());
 } else {
	slotChangeCell(x, y, cell->groundType(), cell->version());
 }
}

void BosonMiniMap::slotFog(int x, int y)
{
 BO_CHECK_NULL_RET(map())
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
 if (!p) {
	boError() << k_funcinfo << "NULL player" << endl;
	return;
 }
 BO_CHECK_NULL_RET(map())
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
 d->mPixmap->repaint();
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
 if (mGround) {
	delete mGround;
 }
 mGround = new QPixmap((int)(mapWidth() * zoom() * scale()), (int)(mapHeight() * zoom() * scale()));
 QPainter p;
 p.begin(mGround);
 p.scale(zoom() * scale(), zoom() * scale());
 p.drawPixmap(0, 0, *mUnZoomedGround);
 p.end();
}

QPointArray BosonMiniMap::makeCellList(Unit* unit, float x, float y)
{
 QPointArray array;
 if (!unit) {
	BO_NULL_ERROR(unit)
	return QPointArray();
 }
 if (!map()) {
	BO_NULL_ERROR(map())
	return QPointArray();
 }
 int left, right, top, bottom;
 BosonItem::leftTopCell(&left, &top, x, y);
 BosonItem::rightBottomCell(&right, &bottom, x + unit->width() - 1, y + unit->height() - 1);
 right = QMIN(right, QMAX((int)mapWidth() - 1, 0));
 bottom = QMIN(bottom, QMAX((int)mapHeight() - 1, 0));
 return BosonItem::cells(left, right, top, bottom);
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
	d->mPixmap->repaint();
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

