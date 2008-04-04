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
#ifndef BOSONGLCOMPLETEMINIMAP_H
#define BOSONGLCOMPLETEMINIMAP_H

#include <qobject.h>

#include "../bomath.h"
#include <bogl.h>

class Player;
class PlayerIO;
class Unit;
class BosonCanvas;
class BosonGLMiniMapRenderer;
class Cell;
class KGameIO;
class BoGLMatrices;
class BoTexture;
class RadarPlugin;

class QPixmap;
class QPainter;
class QPaintEvent;
class QMouseEvent;
template<class T> class QPtrVector;
template<class T> class QValueList;


class BosonGLCompleteMiniMapPrivate;
/**
 * The whole minimap, i.e. a small version of the complete (!) map.
 *
 * This class really manages the complete minimap, it is up to other classes to
 * display a subset of this minimap.
 *
 * The minimap is rendered into a quad of size (1,1). In order to actually
 * display it on the screen, call glScale() with appropriate values prior to
 * rendering the minimap.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonGLCompleteMiniMap : public QObject
{
	Q_OBJECT
public:
	BosonGLCompleteMiniMap(QObject* parent);
	virtual ~BosonGLCompleteMiniMap();

	virtual void createMap(unsigned int w, unsigned int h);

	void setCanvas(BosonCanvas* canvas);
	BosonCanvas* canvas() const;

	void setLocalPlayerIO(PlayerIO* io);
	PlayerIO* localPlayerIO() const;

	virtual void render();

	/**
	 * Disable updates before calling @ref setPoint (e.g. using @ref
	 * setExploredPoint) many times. After you've called them, enable
	 * updates again. Before enabling updates again, previous updates are
	 * incorporated into the minimap then.
	 **/
	void setUpdatesEnabled(bool);

	/**
	 * @return The width of the map (in cells). This does <em>not</em>
	 * change while playing a game.
	 **/
	unsigned int mapWidth() const;

	/**
	 * @return The height of the map (in cells). This does <em>not</em>
	 * change while playing a game.
	 **/
	unsigned int mapHeight() const;

	/**
	 * Display only those parts of the map that are visible to the player
	 * @param p Display this players sight. If NULL show the entire map
	 **/
	void initFogOfWar(PlayerIO* p);

public slots:
	void slotAdvance(unsigned int advanceCallsCount);
	void slotUpdateTerrainAtCorner(int x, int y);
	void slotExplored(int x, int y);
	void slotUnexplored(int x, int y);

private:
	void initializeItems();

	void renderMiniMap();
	void updateRadarTexture(const QValueList<const Unit*>* radars);
	void renderRadarRangeIndicators(const QValueList<const Unit*>* radarlist);

	/**
	 * Calculate the color of the cell at @p x, @p y according to the @ref
	 * BosonMap::texMap. This method doesn't check for units and will
	 * display the color of the ground at this point.
	 * @param x The x - coordinate of the cell
	 * @param y The x - coordinate of the cell
	 **/
	void calculateGround(int x, int y);

	const QValueList<const Unit*>* radarList() const;

	void setPoint(int x, int y, const QColor& color, GLubyte* textureData, BoTexture* texture);
	void unsetPoint(int x, int y, GLubyte* textureData, BoTexture* texture);
	void setColor(int x, int y, const QColor& color, int alpha, GLubyte* textureData, BoTexture* texture);

	void setTerrainPoint(int x, int y, const QColor& color);
	void setWaterPoint(int x, int y, bool isWater);
	void setExploredPoint(int x, int y, bool isExplored);

private:
	void renderQuad();

private:
	BosonGLCompleteMiniMapPrivate* d;
};

#endif

