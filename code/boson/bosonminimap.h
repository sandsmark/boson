/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef __BOSONMINIMAP_H__
#define __BOSONMINIMAP_H__

#include <qwidget.h>

class Player;
class Unit;
class BosonMap;

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
	void initMap();
	

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

protected:
	void setPoint(int x, int y, const QColor& color);
	virtual void paintEvent(QPaintEvent*);
	virtual void mousePressEvent(QMouseEvent*);
	
private:
	class BosonMiniMapPrivate;
	BosonMiniMapPrivate* d;
};
#endif
