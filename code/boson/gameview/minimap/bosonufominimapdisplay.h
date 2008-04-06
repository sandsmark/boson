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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOSONUFOMINIMAPDISPLAY_H
#define BOSONUFOMINIMAPDISPLAY_H

#include "../boufo/boufocustomwidget.h"

#include "../bomath.h"
#include <bogl.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <Q3PtrList>
#include <QPixmap>

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
class BosonGLMiniMapView;

class QPixmap;
class QPainter;
class QPaintEvent;
class QMouseEvent;
class QWheelEvent;
template<class T> class Q3PtrVector;
template<class T> class Q3PtrList;

class BosonUfoMiniMapDisplayPrivate;
/**
 * Internal helper class for @ref BosonUfoMiniMap.
 *
 * This class simply displays the minimap/logo. It does not contain any other
 * widgets like zooming buttons.
 *
 * This class exists to make size calculations easier: width() and height()
 * match exactly the size of the @ref BosonGLMiniMapView.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUfoMiniMapDisplay : public BoUfoCustomWidget
{
	Q_OBJECT
public:
	BosonUfoMiniMapDisplay();
	~BosonUfoMiniMapDisplay();

	BosonGLMiniMapView* miniMapView() const;
	PlayerIO* localPlayerIO() const;
	bool showMiniMap() const;

	void createMap(BosonCanvas* canvas, const BoGLMatrices* gameGLMatrices);
	void setLocalPlayerIO(PlayerIO* io);
	void setLogoTexture(BoTexture* texture);
	void setShowMiniMap(bool show);
	void quitGame();

	virtual void paintWidget();

signals:
	void signalReCenterView(const QPoint& pos);
	void signalMoveSelection(int cellX, int cellY);

protected slots:
	void slotMouseEvent(QMouseEvent* e);
	void slotWheelEvent(QWheelEvent* e);
	void slotWidgetResized();

protected:
	void render();
	void renderLogo();
	void renderMiniMap();
	unsigned int mapWidth() const;
	unsigned int mapHeight() const;

private:
	BosonUfoMiniMapDisplayPrivate* d;
};

#endif

