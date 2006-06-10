/*
    This file is part of the Boson game
    Copyright (C) 2001-2006 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONUFOMINIMAP_H
#define BOSONUFOMINIMAP_H

#include "../boufo/boufocustomwidget.h"

#include "../bomath.h"
#include <bogl.h>

class Player;
class PlayerIO;
class Unit;
class BosonCanvas;
class BosonGroundTheme;
class Cell;
class KGameIO;
class BoGLMatrices;
class BoTexture;
class BosonItem;
class BosonMiniMapQuadtreeNode;

class QPixmap;
class QPainter;
class QPaintEvent;
class QMouseEvent;
template<class T> class QPtrVector;
template<class T> class QPtrList;

class BosonUfoMiniMapPrivate;
/**
 * A minimap.
 *
 * This class provides a @ref BosonGLMiniMapView object in a @ref BoUfoWidget.
 * The actual minimap is extended by features like a "logo" that is displayed
 * when no minimap is available, or zooming buttons.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUfoMiniMap : public BoUfoCustomWidget
{
	Q_OBJECT
public:
	BosonUfoMiniMap();
	virtual ~BosonUfoMiniMap();

	void quitGame();

	void createMap(BosonCanvas* canvas, const BoGLMatrices*);
	void setLocalPlayerIO(PlayerIO*);

	void render();

	unsigned int miniMapScreenWidth() const;
	unsigned int miniMapScreenHeight() const;
	bool showMiniMap() const;

	BosonCanvas* canvas() const;
	PlayerIO* localPlayerIO() const;
	float zoom() const;

	/**
	 * Set the theme where to find the images (logo, zoom images, ...) in.
	 * Default is "standard".
	 **/
	void setImageTheme(const QString& theme);

	void initFogOfWar(PlayerIO* p);

	virtual void paintWidget();

public slots:
	/**
	 * Change the appearance of the rect that displays the current viewport.
	 * All params are cell coordinates.
	 **/
	void slotMoveRect(const QPoint& topLeft, const QPoint& topRight, const QPoint& bottomLeft, const QPoint& bottomRight);

	/**
	 * Show or hide the minimap, depending on @p show. This slot is called
	 * when the radar station is constructed or destroyed.
	 **/
	void slotShowMiniMap(bool show);

	void slotZoomIn();
	void slotZoomOut();
	void slotZoomDefault();


	void slotAdvance(unsigned int advanceCallsCount);
	void slotUnitMoved(Unit* unit, bofixed oldX, bofixed oldY);
	void slotUnitRemoved(Unit* unit);
	void slotItemAdded(BosonItem* item);
	void slotFacilityConstructed(Unit* fac);
	void slotUpdateTerrainAtCorner(int x, int y);
	void slotExplored(int x, int y);
	void slotUnexplored(int x, int y);


signals:
	void signalReCenterView(const QPoint& pos);
	void signalMoveSelection(int cellX, int cellY);

protected slots:
	void slotMouseEvent(QMouseEvent*);

protected:
	/**
	 * @return A pixmap for @p file in the theme @p theme if it is existing.
	 * If not it will search for @p file in the "standard" theme and if it's
	 * not there either it'll return a null pixmap.
	 **/
	QImage imageFromTheme(const QString& file, const QString& theme) const;

	void renderLogo();
	void renderMiniMap();

	QPoint widgetToCell(const QPoint& pos);

	void setZoomImages(const QImage& in_, const QImage& out_, const QImage& defaultZoom_);

private:
	BosonUfoMiniMapPrivate* d;
};


#endif

