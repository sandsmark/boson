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

	/**
	 * @param alignmentFlags See @ref Qt::AlignmentFlags. You can OR
	 * together either @ref Qt::Alignleft or @ref Qt::AlignRight and @ref
	 * Qt::AlignTop or @ref Qt::AlignBottom.
	 **/
	void setAlignment(int alignmentFlags);

	void setMiniMapScreenSize(unsigned int w, unsigned int h);

	/**
	 * @return The width of the minimap, i.e. the width of the rectangle
	 * that is rendered - in pixels. Note: this is just the width of the
	 * quad that is rendered - the actual minimap size can differ, due to
	 * internal reasons.
	 **/
	unsigned int miniMapScreenWidth() const;
	unsigned int miniMapScreenHeight() const;

	/**
	 * Convert a window-coordinate (e.g. a click) to cell-coordinates. This
	 * can be used e.g. to move units to the point where the user clicked.
	 *
	 * This is dependant on the current mini map position (e.g. @ref
	 * setAlignment) as well as on the current view properties (such as @ref setZoom).
	 *
	 * @return TRUE if @p pos was inside the minimap, otherwise FALSE.
	 **/
#if 0
	bool windowToCell(const QPoint& pos, QPoint* cell) const;
#endif

	void render();

protected:
	void renderCamera();

	unsigned int distanceFromEdge() const
	{
		return 5;
	}

private:
	void renderQuad();

private:
	BosonGLMiniMapViewPrivate* d;
};

#endif

