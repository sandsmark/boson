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

#include "defines.h"

#include <qobject.h>

class BosonMap;
class Cell;
class Player;
class Unit;
class UnitProperties;
class BosonTiles;
class BoItemList;
class BosonItem;
class ProductionPlugin;
class BosonParticleSystem;
class BosonShot;
class BoVector3;

class KPlayer;
template<class T> class QPtrList;
template<class T> class QValueList;



/**
 * @short Class that takes care of game management
 *
 * BosonCanvas is one of the most important classes in Boson.
 * It holds lists with all items and calls advance() methods for them
 *
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonCanvas : public QObject
{
	Q_OBJECT
public:
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
	 * @param pos The position to check for presence of a unit. In
	 * <em>canvas</em>-coordinates. See also @ref findUnitAtCell
	 * @return The unit on this coordinates of the canvas. Won't return a
	 * destroyed unit (wreckage)
	 **/
	Unit* findUnitAt(const QPoint& pos);

	/**
	 * @param x The x-coordinate of the cell
	 * @param y The y-coordinate of the cell
	 * @return The unit on this cell. Won't return a
	 * destroyed unit (wreckage)
	 **/
	Unit* findUnitAtCell(int x, int y);

	/**
	 * Test whether the unit can go over rect. This method only tests for
	 * the ground (see @ref Cell) <em>not</em> for collisions with other
	 * units. See @ref Unit for this.
	 **/
	bool canGo(const UnitProperties* prop, const QRect& rect) const;

	void setMap(BosonMap* map);
	BosonMap* map() const;
	unsigned int mapHeight() const;
	unsigned int mapWidth() const;

	virtual void addAnimation(BosonItem* item);
	virtual void removeAnimation(BosonItem* item);
	unsigned int animationsCount() const;

	/**
	 * Add @p item to the list of items in the canvas. This should get
	 * called in the constructor of @ref BosonItem and you should not need
	 * to care about this.
	 *
	 * See also @ref removeItem
	 **/
	void addItem(BosonItem* item);

	/**
	 * Remove @p item from the list of items. You should not need to call
	 * this yourself, as it is called by the @ref BosonItem destructor.
	 *
	 * Be <em>very</em> careful when you call this manually!
	 **/
	void removeItem(BosonItem* item);

	/**
	 * WARNING: you must call @ref updateItemCount before calling this!
	 * Otherwise you'll get old results!
	 * @return The number of items on the canvas with @p rtti. See @ref
	 * BosonItem::rtti. Note that <em>all</em> units get added to @p rtti =
	 * @ref RTTI::UnitStart!
	 **/
	unsigned int itemCount(int rtti) const;

	/**
	 * Update the values for @ref itemCount. This function iterates over all
	 * items and therefore might take a little bit time. Don't call it when
	 * you render frames or so!
	 **/
	void updateItemCount();

	/**
	 * @return A complete list of <em>all</em> items on the canvas. See @ref
	 * addItem
	 **/
	BoItemList allItems() const;
	unsigned int allItemsCount() const;


	BoItemList collisionsAtCells(const QPointArray& cells, const BosonItem* item, bool exact) const;
	BoItemList collisions(const QRect& rect) const;

	/**
	 * @param pos Position in <em>canvas</em> coordinates, i.e. not cell
	 * values
	 **/
	BoItemList collisions(const QPoint& pos) const;

	/**
	 * @param pos Position in <em>cell</em>-coordinates.
	 **/
	BoItemList collisionsAtCell(const QPoint& pos) const;

	/**
	 * Called by @ref Unit. This informs the canvas about a moved
	 * unit. Should e.g. adjust the destination of units which have this
	 * unit as target.
	 *
	 * Also adjust the mini map - see @ref signalUnitMoved
	 **/
	void unitMoved(Unit* unit, float oldX, float oldY);

	/**
	 * Called by @ref Unit. One unit damages/shoots at another unit.
	 * All it does is to create new @ref BosonMissile and play shooting sound
	 **/
	void newShot(BosonShot* shot);

	/**
	 * Called when missile explodes. This just calls @ref explosion with correct
	 * values
	 **/
	void shotHit(BosonShot* m);

	/**
	 * Called when something explodes. Usually it's unit or missile.
	 * @param pos Position of the center of the explosion
	 * @param damage How much unit will be damaged if it's in explosion area
	 * @param range Radius of explosion. All units range or less cells away will be damaged
	 * @param owner Player who caused the explosion. Used for statistics. May be null
	 **/
	void explosion(const BoVector3& pos, long int damage, float range, Player* owner);

	/**
	 * Called when unit is damaged (usually by missile).
	 * It calculates new health for the unit, creates particle systems if needed
	 * and marks unit as destoyed if it doesn't have any hitpoints left anymore.
	 **/
	void unitDamaged(Unit* unit, long int damage);

	/**
	 * Mark the unit as destroyed and play the destroyed sound.
	 * The unit will be deleted after a certain time.
	 **/
	void destroyUnit(Unit* unit);

	/**
	 * Prepare the unit to be deleted. Remove the unit from the player and
	 * so on. This doesn't play any sound or so, so it can be used in
	 * editor, too.
	 *
	 * For game mode please use @ref destroyUnit instead, which is a
	 * frontend for this.
	 *
	 * Note that this function doesn't add the unit to any deletion list and
	 * it doesn't delete the unit either.
	 **/
	void removeUnit(Unit* unit);

	void updateSight(Unit*, float oldX, float oldY);

	Cell* cellAt(Unit* unit) const;

	/**
	 * @return The cell at @p x, @p y in <em>canvas</em>-coordinates
	 **/
	Cell* cellAt(float x, float y) const;

	/**
	 * @return The cell at @p x, @p y (in cell coordinates - see @ref cellAt
	 * for other coordinates)
	 **/
	Cell* cell(int x, int y) const;

	void deleteDestroyed();
	void deleteUnusedShots();

	/**
	 * Usually you don't need a @ref QCanvasItemList of all units in a
	 * certain rect but rather a list of all units in a certain circle. This
	 * function does exactly that. Note that it's speed can be improved as
	 * it first uses @ref bosonCollisions for a rect and then checks for
	 * units inside the rect which are also in the circle. Maybe we could
	 * check for the circle directly.
	 **/
	QValueList<Unit*> unitCollisionsInRange(const QPoint& pos, int radius) const;

	/**
	 * Same as @ref unitCollisionInRange, but also checks for z-coordinate and
	 * operates in 3d space
	 **/
	QValueList<Unit*> unitCollisionsInSphere(const BoVector3& pos, int radius) const;

	QValueList<Unit*> unitsAtCell(int x, int y) const;

	/**
	 * Returns whether cell is occupied (there is non-destroyed mobile or
	 * facility on it) or not
	 * Note that if there is aircraft on this tile, it returns false
	 **/
	bool cellOccupied(int x, int y) const;

	/**
	 * Like previous one, but unit u can be on cell
	 * Can be used from inside Unit class
	 * If excludemoving is true moving units can be on cell
	 */
	bool cellOccupied(int x, int y, Unit* u, bool excludemoving = false) const;

	/**
	 * Check if any cell in rect is occupied. Note that rect consists of
	 * <em>canvas coordinates</em>, not of cell-coordinates.
	 * @param rect Check all cells on this rect
	 * @param u This unit may be on the cells, it is not relevant for
	 * occupation status
	 * @param excludeMoving Only valid if u is non-NULL. If TRUE moving
	 * units are ignored for occupation checking
	 * @return TRUE if any cell in rect is occupied, otherwise FALSE.
	 **/
	bool cellsOccupied(const QRect& rect, Unit* u = 0, bool excludeMoving = false) const;

	/**
	 * @param pos The location where the unit should get placed. This is in
	 * <em>cell</em>-coordinates. This point specifies the
	 * <em>upper-left</em> corner of the unit.
	 * @param factory If NULL then BUILD_RANGE is ignored. Otherwise facilities
	 * must be in range of BUILD_RANGE of any player unit and mobile units
	 * in BUILD_RANGE of the facility.
	 * @return TRUE if the unit can be placed at pos, otherwise FALSE
	 **/
	bool canPlaceUnitAt(const UnitProperties* unit, const QPoint& pos, ProductionPlugin* factory) const;

	void quitGame();

	/**
	 * Remove all remaining units of player (if any). After this point the
	 * player is not able to do anything!
	 **/
	void killPlayer(Player* player);

	void addToCells(BosonItem* u);
	void removeFromCells(BosonItem* u);

	BosonTiles* tileSet() const;

	bool onCanvas(const QPoint& pos) const
	{
		return onCanvas(pos.x(), pos.y());
	}
	bool onCanvas(int x, int y) const
	{
		return x >= 0 && y >= 0 && (unsigned int)x < mapWidth() * BO_TILE_SIZE && (unsigned int)y < mapHeight() * BO_TILE_SIZE;
	}

	bool advanceFunctionLocked() const { return mAdvanceFunctionLocked; }

	int particleSystemsCount() const;
	void updateParticleSystems(float elapsed);
	QPtrList<BosonParticleSystem>* particleSystems();
	void addParticleSystem(BosonParticleSystem* s);
	void addParticleSystems(const QPtrList<BosonParticleSystem> systems);

public slots:
	/**
	 * The game (@ref Boson) reports that a unit shall be added - lets do
	 * that :-)
	 *
	 * Note that unit->x() and unit->y() may return values different to @p x
	 * and @p y, as the slot which moves the unit to its actual position
	 * might not have been called yet. So you should prefer the @p x and @p
	 * y parameters here.
	 **/
	void slotAddUnit(Unit* unit, int x, int y);

	/**
	 * @param See @ref Boson::signalAdvance
	 **/
	void slotAdvance(unsigned int advanceCount, bool advanceFlag);
	
signals:
	void signalUnitMoved(Unit* unit, float oldX, float oldY);
	void signalUnitRemoved(Unit* unit);
	void signalOutOfGame(Player*);

protected:
	void lockAdvanceFunction() { mAdvanceFunctionLocked = true; }
	void unlockAdvanceFunction() { mAdvanceFunctionLocked = false; }

private:
	void init();

private:
	class BosonCanvasPrivate;
	BosonCanvasPrivate* d;

	bool mAdvanceFunctionLocked;
};

#endif
