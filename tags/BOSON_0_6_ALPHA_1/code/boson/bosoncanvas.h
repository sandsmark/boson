/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONCANVAS_H
#define BOSONCANVAS_H

#include <qcanvas.h>

class KPlayer;
class BosonMap;
class Cell;
class Player;
class Unit;
class UnitProperties;
class BoShot;
class BoDisplayManager;

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
	 * @return The unit on this coordinates of the canvas. Won't return a
	 * destroyed unit (wreckage)
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
	 * Load the tileset. Note that you <em>must not</em> call @ref
	 * slotAddCell before the tileset has been loaded completely!
	 *
	 * Also note that the actual loading happens in @ref slotLoadTiles using
	 * a @ref QTimer::singleShot. This gives us a non-blocking UI as we can
	 * use @ref QApplication::processEvents
	 * @param tileFile currently always "earth.png
	 **/
	void loadTiles(const QString& tileFile);

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

	void leaderMoved(Unit* unit, double oldX, double oldY);
	void leaderDestroyed(Unit* unit);
	void leaderStopped(Unit* unit);

	/**
	 * Called by @ref Unit. One unit damages/shoots at another unit.
	 **/
	void shootAtUnit(Unit* target, Unit* damagedBy, long int damage);

	/**
	 * Mark the unit as destroyed and play the destroyed sound.
	 * The unit will be deleted after a certain time.
	 **/
	void destroyUnit(Unit* unit);

	void deleteShot(BoShot*);

	void updateSight(Unit*, double oldX, double oldY);

	Cell* cellAt(Unit* unit) const;
	Cell* cellAt(double x, double y) const;
	Cell* cell(int x, int y) const;

	void fogLocal(int x, int y);
	void unfogLocal(int x, int y);

	/**
	 * Initialize the fog of war. If you don't call this the fow pixmap is
	 * not created at all and therefore neithere @ref fogLocal nor
	 * unfogLocal do anything. 
	 * @param player Whose sight shall be shown. If 0 create the fow pixmap
	 * only - fog is put on the map by @ref fogLocal only.
	 **/
	void initFogOfWar(Player* player);

	void deleteDestroyed();

	/**
	 * Usually you don't need a @ref QCanvasItemList of all units in a
	 * certain rect but rather a list of all units in a certain circle. This
	 * function does exactly that. Note that it's speed can be improved as
	 * it first uses @ref QCanvas::collisions for a rect and then checks for
	 * units inside the rect which are also in the circle. Maybe we could
	 * check for the circle directly.
	 **/
	QValueList<Unit*> unitCollisionsInRange(const QPoint& pos, int radius);

	QValueList<Unit*> unitsAtCell(int x, int y);

	/**
	 * Returns whether cell is occupied (there is non-destroyed mobile or
	 * facility on it) or not
	 * Note that if there is aircraft on this tile, it returns false
	 **/
	bool cellOccupied(int x, int y);

	/**
	 * Like previous one, but unit u can be on cell
	 * Can be used from inside Unit class
	 * If excludemoving is true moving units can be on cell
	 */
	bool cellOccupied(int x, int y, Unit* u, bool excludemoving = false);

	void quitGame();

	void setWorkChanged(Unit* u);


	/**
	 * Called from another threead to set the tileset. Note that until this
	 * was called there is <em>no</em> cell and <em>no</em> tile on the
	 * canvas!
	 *
	 * You should not call @ref QCanvas::update before this is completed!
	 **/
	void setTileSet(QPixmap* tileSet);

	/**
	 * Remove all remaining units of player (if any). After this point the
	 * player is not able to do anything!
	 **/
	void killPlayer(Player* player);

	void addToCells(Unit* u);
	void removeFromCells(Unit* u);

	void setDisplayManager(BoDisplayManager* m);

public slots:
	/**
	 * The game (@ref Boson) reports that a unit shall be added - lets do
	 * that :-)
	 **/
	void slotAddUnit(Unit* unit, int x, int y);

	/**
	 * @param See @ref Boson::signalAdvance
	 **/
	void slotAdvance(unsigned int advanceCount);
	
	/**
	 * Reimplemented for internal reasons. Don't use it. It is obsolete. Use
	 * @ref slotAdvance instead.
	 **/
	virtual void advance();

	void slotAddCell(int x, int y, int groundType, unsigned char b);

	void slotNewGroup(Unit* leader, QPtrList<Unit> members);

	virtual void update();

signals:
	void signalUnitMoved(Unit* unit, double oldX, double oldY);
	void signalUnitDestroyed(Unit* unit);
	void signalOutOfGame(Player*);

protected:
	/**
	 * Change the work of all units which have called @ref setWork using
	 * @ref Unit::setWork. This is used to increase the speed of @ref
	 * slotAdvance
	 * @param oldWork See @ref UnitBase::WorkType
	 **/
	void changeWork();

protected slots:
	/**
	 * ad the tileset that has been specified using @ref loadTiles. We use
	 * this slot to provide a non-blocking tile loading.
	 **/
	void slotLoadTiles();


private:
	void init();

private:
	class BosonCanvasPrivate;
	BosonCanvasPrivate* d;
};

#endif
