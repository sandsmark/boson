/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef CELL_H
#define CELL_H

#include "boitemlist.h"

class UnitProperties;
class BosonPathRegion;

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
	inline uint removeItem(BosonItem* u) { return mItems.remove(u); }


	/**
	 * @param Consider moving units as collisions, too if TRUE, otherwise
	 * not.
	 * @return @ref BoItemList::isOccupied
	 **/
	inline bool isOccupied(Unit* forUnit, bool includeMoving = true) const { return mItems.isOccupied(forUnit, includeMoving); }
	inline bool isOccupied(bool includeMoving = true) const { return mItems.isOccupied(includeMoving); }
	inline void isOccupied(Unit* forUnit, bool& hasmoving, bool& hasany) const { mItems.isOccupied(forUnit, hasmoving, hasany); }

	inline bool isLandOccupied() const { return mItems.isLandOccupied(); }
	inline bool isAirOccupied() const { return mItems.isAirOccupied(); }
	inline void recalculateLandOccupiedStatus() { return mItems.recalculateLandOccupiedStatus(); }
	inline void recalculateAirOccupiedStatus() { return mItems.recalculateAirOccupiedStatus(); }

	inline float passageCostLand() const { return mItems.passageCostLand(); }
	inline float passageCostAir() const { return mItems.passageCostAir(); }

	// AB: bah. I want to have models for minerals/oil and use a BosonItem,
	// instead of hardcoding them into the ground.
	// we should iterate through mItems and search for the oil/minerals RTTI
	// once we have implemented that!
//	bool hasOil() const { return (groundType() == GroundGrassOil); }
//	bool hasMinerals() const { return (groundType() == GroundGrassMineral); }
	bool hasOil() const { return (false); }
	bool hasMinerals() const { return (false); }

	inline const BoItemList* items() const { return &mItems; }
	unsigned int unitCount() const { return mItems.count(); }

	inline void setRegion(BosonPathRegion* r) { mRegion = r; }
	inline BosonPathRegion* region() const { return mRegion; }

	inline void setPassable(bool p) { mPassable = p; }
	inline bool passable() const { return mPassable; }


private:
	int mX;
	int mY;

	BoItemList mItems;

	BosonPathRegion* mRegion;

	bool mPassable;
};

#endif
