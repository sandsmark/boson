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
#ifndef __BOSONCANVAS_H__
#define __BOSONCANVAS_H__

#include <qcanvas.h>

#include <qpixmap.h>

class BosonMap;
class Cell;
class KPlayer;
class Player;
class Unit;
class UnitProperties;

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonCanvas : public QCanvas
{
	Q_OBJECT
public:
	BosonCanvas(QPixmap p, unsigned int w, unsigned int h);
	BosonCanvas(QObject* parent);
	~BosonCanvas();

	/**
	 * Create the @ref Cell array
	 **/
	void createCells(int w, int h);

	/**
	 * Initialize this @ref Cell.
	 *
	 * All this currently does is to set the tile. See @ref QCanvas::setTile
	 **/
	void initCell(int x, int y);

	/**
	 * @return The unit on this coordinates of the canvas
	 **/
	Unit* findUnitAt(const QPoint& pos);

	/**
	 * Test whether the unit can go over rect. This method only tests for
	 * the ground (see @ref Cell) <em>not</em> for collisions with other
	 * units. See @ref Unit for this.
	 **/
	bool canGo(const UnitProperties* prop, const QRect& rect) const;

	void setMap(BosonMap* map);
	BosonMap* map() const;
	
	/**
	 * @param tileFile currently always "earth.png
	 **/
	void initMap(const QString& tileFile);

	/**
	 * Reimlemented from QCanvas::addAnimation because of @ref advance
	 **/
	virtual void addAnimation(QCanvasItem*);
	/**
	 * Reimlemented from QCanvas::removeAnimation because of @ref advance
	 **/
	virtual void removeAnimation(QCanvasItem*);

	/**
	 * Called by @ref Unit. This informs the canvas about a moved
	 * unit. Should e.g. adjust the destination of units which have this
	 * unit as target.
	 *
	 * Also adjust the mini map - see @ref signalUnitMoved
	 **/
	void unitMoved(Unit* unit, double oldX, double oldY);

	/**
	 * Called by @ref Unit. One unit damages/shoots at another unit.
	 **/
	void shootAtUnit(Unit* target, Unit* damagedBy, long int damage);

	Cell* cellAt(Unit* unit) const;
	Cell* cellAt(double x, double y) const;

public slots:
	/**
	 * The game (@ref Boson) reports that a unit shall be added - lets do
	 * that :-)
	 **/
	void slotAddUnit(Unit* unit, int x, int y);
	virtual void advance();

	void slotAddCell(int x, int y, int groundType, unsigned char b);
	
	
signals:
	void signalUnitMoved(Unit* unit, double oldX, double oldY);
	void signalUnitDestroyed(Unit* unit);

protected:
	void loadTiles(const QString&);
	Cell* cell(int x, int y) const;
	void play(const QString& fileName); // perhaps in public

protected slots:


private:
	void init();

private:
	class BosonCanvasPrivate;
	BosonCanvasPrivate* d;
};

#endif
