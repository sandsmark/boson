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
#ifndef BOSONGLMINIMAPVIEW_H
#define BOSONGLMINIMAPVIEW_H

#include "bosonglcompleteminimap.h"

#include "../bomath.h"
#include <bogl.h>

class Player;
class PlayerIO;
class Unit;
class BosonCanvas;
class BosonGLMiniMapRenderer;
class BosonGroundTheme;
class Cell;
class KGameIO;
class BoGLMatrices;
class BoTexture;
class BosonItem;

class QPixmap;
class QPainter;
class QPaintEvent;
class QMouseEvent;
template<class T> class QPtrVector;
template<class T> class QPtrList;


class BosonGLMiniMapViewPrivate;
/**
 * Displays a small (usually rectangle) part of @ref BosonGLCompleteMiniMap,
 * i.e. this is a "view" class on the minimap.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonGLMiniMapView : public BosonGLCompleteMiniMap
{
	Q_OBJECT
public:
	BosonGLMiniMapView(const BoGLMatrices*, QObject* parent);
	virtual ~BosonGLMiniMapView();

	void setViewSize(unsigned int w, unsigned int h);
	int viewCenterX() const;
	int viewCenterY() const;

	void render();

	void zoomIn();
	void zoomOut();

	QPoint widgetToCell(const QPoint& pos) const;

protected:
	void renderCamera();

	void centerViewOnCell(int x, int y);
	int xTranslation() const;
	int yTranslation() const;
	float zoomOutFactor() const;

private:
	void renderQuad();

private:
	BosonGLMiniMapViewPrivate* d;
};

#endif

