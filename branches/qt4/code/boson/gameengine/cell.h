/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2005 Rivo Laks (rivolaks@hot.ee)

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
#ifndef CELL_H
#define CELL_H

#include "boitemlist.h"

class UnitProperties;
class BosonPathSubregion;

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class Cell
{
public:
	Cell();
	~Cell();

	void setPosition(int x, int y);
	inline int x() const { return mX; }
	inline int y() const { return mY; }


	/**
	 * The moving cost of a cell is the value that should influence the
	 * speed of a unit. A unit should move fastter in grass while it should
	 * get slower on desert. Note that aircrafts should ignore this value!
	 * @return The moving cost of this cell. Higher value means the unit
	 * should move slower here, 0 means not to touch the speed.
	 **/
	int moveCost() const; // leave signed, maybe we can use negative values for roads one day?


	inline void addItem(BosonItem* u) { mItems.append(u); }

	/**
	 * Remove a previously added item from the cell
	 **/
	inline void removeItem(BosonItem* u) { mItems.remove(u); }


	/**
	 * @param Consider moving units as collisions, too if TRUE, otherwise
	 * not.
	 * @return @ref BoItemList::isOccupied
	 **/
	inline bool isOccupied(Unit* forUnit, bool includeMoving = true) const { return mItems.isOccupied(forUnit, includeMoving); }
	inline bool isOccupied(bool includeMoving = true) const { return mItems.isOccupied(includeMoving); }
	inline void isOccupied(Unit* forUnit, bool& hasmoving, bool& hasany) const { mItems.isOccupied(forUnit, hasmoving, hasany); }

	inline const BoItemList* items() const { return &mItems; }
	unsigned int unitCount() const { return mItems.count(); }

	inline void setIsWater(bool w) { mIsWater = w; }
	/**
	 * @return Whether this cell is under water.
	 * Note that cell being under water does NOT imply that water units can and
	 *  land units can't go on that cell.
	 **/
	inline bool isWater() const { return mIsWater; }

private:
	int mX;
	int mY;

	BoItemList mItems;

	bool mIsWater;
};

#endif
