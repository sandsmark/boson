/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONCOLLISIONS_H
#define BOSONCOLLISIONS_H

class BosonMap;
class Cell;
class Unit;
class BoItemList;
class BosonItem;
class bofixed;
template<class T> class BoVector2;
template<class T> class BoVector3;
template<class T> class BoRect2;
typedef BoVector2<bofixed> BoVector2Fixed;
typedef BoVector3<bofixed> BoVector3Fixed;
typedef BoRect2<bofixed> BoRect2Fixed;

template<class T> class QPtrList;
template<class T> class QValueList;
template<class T> class QPtrVector;

#include "../bomath.h"



/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonCollisions
{
public:
	BosonCollisions();
	~BosonCollisions();

	/**
	 * @return The cell at @p x, @p y (in cell coordinates)
	 **/
	Cell* cell(int x, int y) const;

	/**
	 * @param pos The position to check for presence of a unit. In
	 * <em>canvas</em>-coordinates. See also @ref findUnitAtCell.
	 * @return The unit on this coordinates of the canvas. Won't return a
	 * destroyed unit (wreckage)
	 **/
	Unit* findUnitAt(const BoVector3Fixed& pos) const;

	BosonItem* findItemAt(const BoVector3Fixed& pos) const;

	/**
	 * See @ref findItemAtCell.
	 * @param x The x-coordinate of the cell
	 * @param y The y-coordinate of the cell
	 * @param z The desired z coordinate of the unit. Unused if only one
	 * unit is there, if there are multiple units on the cell the one with
	 * the closest z value is chosen
	 * @return The unit on this cell. Won't return a
	 * destroyed unit (wreckage)
	 **/
	Unit* findUnitAtCell(int x, int y, bofixed z) const;

	/*
	 * @param x The x-coordinate of the cell
	 * @param y The y-coordinate of the cell
	 * @param z The desired z coordinate of the item. Unused if only one
	 * item is there, if there are multiple units on the cell the one with
	 * the closest z value is chosen
	 * @param unitOnly If TRUE this returns units only (see @ref
	 * findUnitAtCell), otherwise any item.
	 * @return The first item that is found on that cell. If that item is a
	 * unit then it'll be returned only if it is not destroyed.
	 **/
	BosonItem* findItemAtCell(int x, int y, bofixed z, bool unitOnly) const;

	void setMap(BosonMap* map) { mMap = map; }
	inline BosonMap* map() const { return mMap; }

	BoItemList* collisionsAtCells(const QPtrVector<Cell>* cells, const BosonItem* item, bool exact) const;
	BoItemList* collisions(const BoRect2Fixed& rect, const BosonItem* item = 0, bool exact = true) const; // note: exact == true has n effec for item != 0 ONLY!
	BoItemList* collisionsAtCells(const BoRect2Fixed& rect, const BosonItem* item = 0, bool exact = true) const; // note: exact == true has n effec for item != 0 ONLY!

	/**
	 * @param x x-Position in <em>cell</em>-coordinates.
	 * @param y y-Position in <em>cell</em>-coordinates.
	 **/
	BoItemList* collisionsAtCell(int x, int y) const;

	/**
	 * @param pos Position in <em>canvas</em> coordinates, i.e. not cell
	 * values
	 **/
	BoItemList* collisions(const BoVector2Fixed& pos) const;

	/**
	 * Usually you don't need a @ref QCanvasItemList of all units in a
	 * certain rect but rather a list of all units in a certain circle. This
	 * function does exactly that. Note that it's speed can be improved as
	 * it first uses @ref bosonCollisions for a rect and then checks for
	 * units inside the rect which are also in the circle. Maybe we could
	 * check for the circle directly.
	 **/
	QValueList<Unit*> unitCollisionsInRange(const BoVector2Fixed& pos, bofixed radius) const;

	/**
	 * Same as @ref unitCollisionInRange, but also checks for z-coordinate and
	 * operates in 3d space
	 **/
	QValueList<Unit*> unitCollisionsInSphere(const BoVector3Fixed& pos, bofixed radius) const;

	/**
	 * Returns whether cell is occupied (there is non-destroyed mobile or
	 * facility on it) or not
	 * Note that if there is aircraft on this tile, it returns false
	 **/
	bool cellOccupied(int x, int y) const;

	/**
	 * Like previous one, but unit u can be on cell
	 * Can be used from inside Unit class
	 * If excludeMoving is true moving units can be on cell
	 */
	bool cellOccupied(int x, int y, Unit* u, bool excludeMoving = false) const;

	/**
	 * Check if any cell in rect is occupied. Note that rect consists of
	 * <em>canvas coordinates</em>, not of cell-coordinates.
	 * @param rect Check all cells on this rect
	 * @return TRUE if any cell in rect is occupied, otherwise FALSE.
	 **/
	bool cellsOccupied(const BoRect2Fixed& rect) const;

	QValueList<Unit*> collisionsInBox(const BoVector3Fixed& v1, const BoVector3Fixed& v2, BosonItem* exclude) const;

private:
	void init();

private:
	BosonMap* mMap;
};

#endif
