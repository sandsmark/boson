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
#ifndef __BOSONMINIMAP_H__
#define __BOSONMINIMAP_H__

#include <qwidget.h>

class Player;
class Unit;
class BosonMap;
class BosonCanvas;

class QPixmap;
class QPainter;
class QPaintEvent;
class QMouseEvent;

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonMiniMap : public QWidget
{
	Q_OBJECT
public:
	BosonMiniMap(QWidget* parent);
	~BosonMiniMap();

	QPixmap* ground() const;

	int mapWidth() const;
	int mapHeight() const;

	void setMap(BosonMap* map);
	void setCanvas(BosonCanvas*);
	void initMap();

	void setLocalPlayer(Player*);
	
	/**
	 * Display only those parts of the map that are visible to the player
	 * @param p Display this players sight. If NULL show the entire map
	 **/
	void initFogOfWar(Player* p);

	void setPointSize(int p);
	int pointSize() const;

signals:
	void signalReCenterView(const QPoint& pos);

public slots:
	void slotCreateMap(int w, int h);
	/**
	  * @param x The x - coordinate of the cell
	  * @param y The x - coordinate of the cell
	  * @param groundType The type of the cell. See @ref Cell::GroundType
	  * @param b Unused
	  **/
	void slotAddCell(int x, int y, int groundType, unsigned char b);
	void slotAddUnit(Unit* unit, int x, int y);

	void slotMoveRect(int x, int y);
	void slotResizeRect(int w, int h);

	void slotMoveUnit(Unit* unit, double oldX, double oldY);
	void slotUnitDestroyed(Unit* unit);

	void slotUnfog(int x, int y);
	void slotFog(int x, int y);

	/**
	 * Show or hide the minimap, depending on show. This slot is called when
	 * the radar station is constructed or destroyed.
	 **/
	void slotShowMap(bool show);

protected:
	void setPoint(int x, int y, const QColor& color);
	virtual void paintEvent(QPaintEvent*);
	virtual void mousePressEvent(QMouseEvent*);
	
private:
	class BosonMiniMapPrivate;
	BosonMiniMapPrivate* d;

	QPixmap* mGround;
	bool mUseFog; // useful for the editor to disable the fog of war
	
	BosonMap* mMap;
	BosonCanvas* mCanvas;
	Player* mLocalPlayer; // needed to distinguish between movements (->fog of war)
	
};
#endif
