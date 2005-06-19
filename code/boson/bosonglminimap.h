/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2005 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONGLMINIMAP_H
#define BOSONGLMINIMAP_H

#include <qobject.h>

#include "bomath.h"

class Player;
class PlayerIO;
class Unit;
class BosonMap;
class BosonCanvas;
class BosonGLMiniMapRenderer;
class BosonGroundTheme;
class Cell;
class KGameIO;
class BoGLMatrices;

class QPixmap;
class QPainter;
class QPaintEvent;
class QMouseEvent;
template<class T> class QPtrVector;

class BosonGLMiniMapPrivate;
/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonGLMiniMap : public QObject
{
	Q_OBJECT
public:
	BosonGLMiniMap(QObject* parent, const char* name = 0);
	virtual ~BosonGLMiniMap();

	void quitGame();

	void createMap(BosonMap* map, const BoGLMatrices*);
	void setLocalPlayerIO(PlayerIO*);

	void renderMiniMap();

	unsigned int miniMapWidth() const;
	unsigned int miniMapHeight() const;
	bool showMiniMap() const;

	BosonMap* map() const { return mMap; }
	PlayerIO* localPlayerIO() const { return mLocalPlayerIO; }
	float zoom() const;
	BosonGroundTheme* groundTheme() const;

	/**
	 * Display only those parts of the map that are visible to the player
	 * @param p Display this players sight. If NULL show the entire map
	 **/
	void initFogOfWar(PlayerIO* p);

	/**
	 * Set the theme where to find the images (logo, zoom images, ...) in.
	 * Default is "standard".
	 **/
	void setImageTheme(const QString& theme);

	/**
	 * @return TRUE if the mouse event was catched, i.e. no other widget
	 * should process it. Otherwise FALSE - the event might be processed in
	 * the parent widget then.
	 **/
	bool mouseEvent(KGameIO*, QDataStream& stream, QMouseEvent* e, bool* send);

	void emitSignalReCenterView(const QPoint& pos);
	void emitSignalMoveSelection(const QPoint& pos);


public slots:
	/**
	 * Change the appearance of the rect that displays the current viewport.
	 * All params are cell coordinates.
	 **/
	void slotMoveRect(const QPoint& topLeft, const QPoint& topRight, const QPoint& bottomLeft, const QPoint& bottomRight);

	void slotUnitMoved(Unit* unit, bofixed oldX, bofixed oldY);
	void slotUnitDestroyed(Unit* unit);

	void slotUnfog(int x, int y);
	void slotFog(int x, int y);

	/**
	 * Show or hide the minimap, depending on @p show. This slot is called
	 * when the radar station is constructed or destroyed.
	 **/
	void slotShowMiniMap(bool show);

	void slotZoomIn();
	void slotZoomOut();
	void slotZoomDefault();

	/**
	 * @param x The x - coordinate of the cell
	 * @param y The x - coordinate of the cell
	 **/
	void slotUpdateCell(int x, int y);

signals:
	void signalReCenterView(const QPoint& pos);
	void signalMoveSelection(int cellX, int cellY);

protected:
	/**
	 * @return TRUE if a map has been set and should be displayed. Otherwise
	 * FALSE (minimap is disabled). All function calls that update
	 * units/cells should be ignored.
	 **/
	bool hasMap() const;

	void setPoint(int x, int y, const QColor& color);
	void makeCellList(QPtrVector<Cell>* cells, const Unit* unit, bofixed x, bofixed y);
	/**
	 * Move a unit. if oldCells is NULL they are ignored.
	 * (added a unit)
	 **/
	void moveUnit(Unit* unit, const QPtrVector<Cell>* newCells, const QPtrVector<Cell>* oldCells);

	/**
	 * Update the cell. Checks whether cell is fogged, checks for units on
	 * the cell, ...
	 **/
	void updateCell(int x, int y);

	/**
	 * @return A pixmap for @p file in the theme @p theme if it is existing.
	 * If not it will search for @p file in the "standard" theme and if it's
	 * not there either it'll return a null pixmap.
	 **/
	QImage imageFromTheme(const QString& file, const QString& theme) const;

	/**
	 * Calculate the color of the cell at @p x, @p y accordint to the @ref
	 * BosonMap::texMap. This method doesn't check for units and will
	 * display the color of the ground at this point.
	 * @param x The x - coordinate of the cell
	 * @param y The x - coordinate of the cell
	 **/
	void calculateGround(int x, int y);

private:
	BosonGLMiniMapPrivate* d;
	PlayerIO* mLocalPlayerIO;
	BosonMap* mMap;

	BosonGLMiniMapRenderer* mRenderer;
};


class BosonGLMiniMapRendererPrivate;
/**
 * This class does the actual rendering. It does not inherit @ref QObject so
 * that it could be moved easily to different things (e.g. currently we use a
 * separatw widget to render it. later we may render directly into the big
 * display)
 *
 * This class also creates/stores/manages all minimap textures/images (the 
 * actual map as well as the logo that is displayed when no map is visible and
 * all other minimap textures/images).
 **/
class BosonGLMiniMapRenderer
{
public:
	enum MiniMapType {
		MiniMap = 0,
		Logo = 1
	};
public:
	BosonGLMiniMapRenderer(const BoGLMatrices* );
	virtual ~BosonGLMiniMapRenderer();

	void setMiniMapSize(unsigned int w, unsigned int h);

	void createMap(unsigned int w, unsigned int h, BosonGroundTheme* theme);

	/**
	 * Set an image for the logo. Use @ref createMap before calling this
	 **/
	void setLogo(const QImage& image);
	void setZoomImages(const QImage& in, const QImage& out, const QImage& _default);

	void setType(MiniMapType type)
	{
		mType = type;
	}
	void setZoom(float zoom)
	{
		mZoom = zoom;
	}
	/**
	 * @param alignmentFlags See @ref Qt::AlignmentFlags. You can OR
	 * together either @ref Qt::Alignleft or @ref Qt::AlignRight and @ref
	 * Qt::AlignTop or @ref Qt::AlignBottom.
	 **/
	void setAlignment(int alignmentFlags);

	/**
	 * @return The width of the minimap, i.e. the width of the rectangle
	 * that is rendered - in pixels. Note: this is just the width of the
	 * quad that is rendered - the actual minimap size can differ, due to
	 * internal reasons.
	 **/
	unsigned int miniMapWidth() const { return mMiniMapWidth; }
	unsigned int miniMapHeight() const { return mMiniMapHeight; }

	/**
	 * Convert a window-coordinate (e.g. a click) to cell-coordinates. This
	 * can be used e.g. to move units to the point where the user clicked.
	 *
	 * This is dependant on the current mini map position (e.g. @ref
	 * setAlignment) as well as on the current view properties (such as @ref setZoom).
	 *
	 * @return TRUE if @p pos was inside the minimap, otherwise FALSE.
	 **/
	bool windowToCell(const QPoint& pos, QPoint* cell) const;

	void render();

	void setFogged(int x, int y);
	void unfog(int x, int y);

	void setPoint(int x, int y, const QColor& color);

	/**
	 * Disable updates before calling @ref setPoint (e.g. using @ref
	 * setFogged or @ref unfog) many times. After you've called them, enable
	 * updates again. Before enabling updates again, previous updates are
	 * incorporated into the minimap then.
	 **/
	void setUpdatesEnabled(bool);

protected:
	void renderMiniMap();
	void renderCamera();
	void renderLogo();
	void renderGimmicks(); // zoom buttons, minimap frame, ...


	/**
	 * @return The width of the map (in cells). This does <em>not</em>
	 * change while playing a game.
	 **/
	unsigned int mapWidth() const { return mMapWidth; }

	/**
	 * @return The height of the map (in cells). This does <em>not</em>
	 * change while playing a game.
	 **/
	unsigned int mapHeight() const { return mMapHeight; }

	unsigned int distanceFromEdge() const
	{
		return 5;
	}

private:
	void renderQuad();

private:
	BosonGLMiniMapRendererPrivate* d;

	MiniMapType mType;
	float mZoom;

	unsigned int mMapWidth;
	unsigned int mMapHeight;
	float mTextureMaxWidth;
	float mTextureMaxHeight;

	unsigned int mMiniMapWidth;
	unsigned int mMiniMapHeight;
	unsigned int mPosX;
	unsigned int mPosY;

	bool mUseFog; // useful for the editor to disable the fog of war
};

#endif

