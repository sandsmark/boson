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
#ifndef CELL_H
#define CELL_H

#include "boitemlist.h"

class UnitProperties;

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class Cell
{
public:
	enum GroundType {
		GroundUnknown = 0,
		GroundDeepWater = 1,
		GroundWater = 2,
		GroundGrass = 3,
		GroundDesert = 4,
		
		GroundGrassMineral = 5,
		GroundGrassOil = 6,

		GroundLast = 7
	};
        enum TransType {
		TransGrassWater = 0,
		TransGrassDesert,
		TransDesertWater,
		TransDeepWater,
		TransLast
	};
	enum Transition {
		TransUpLeft = 0,
		TransUpRight,
		TransDownLeft,
		TransDownRight,

		TransUp,
		TransDown,
		TransLeft,
	       	TransRight,
		
		TransUpLeftInverted,
		TransUpRightInverted,
		TransDownLeftInverted,
		TransDownRightInverted
	};


	Cell();
	~Cell();

	/**
	 * Use this to initialize the cell. You should call this only once in
	 * the game (when creating the map). In the editor this is called
	 * whenever the cell changes.
	 **/
	void makeCell(int groundType, unsigned char version);

	/**
	 * @return Whether the specified unit can go over this ground. Note
	 * that this does <em>not</em> check whether the cell is occupied.
	 **/
	bool canGo(const UnitProperties* unit) const;
	static bool canGo(const UnitProperties* unit, GroundType ground);

	/**
	 * The moving cost of a cell is the value that should influence the
	 * speed of a unit. A unit should move fastter in grass while it should
	 * get slower on desert. Note that aircrafts should ignore this value!
	 * @return The moving cost of this cell. Higher value means the unit
	 * should move slower here, 0 means not to touch the speed.
	 **/
	int moveCost() const; // leave signed, maybe we can use negative values for roads one day?


	/**
	 * This has nothing to do with @ref GroundType! While @ref GroundType
	 * contains the basic plain tiles only this can also be a transtition!!
	 **/
	int groundType() const
	{
		return mType;
	}

	/**
	 * @return The version of the groundType. 
	 **/
	unsigned char version() const
	{
		return mVersion;
	}

	/**
	 * @return The number of different groundTypes.
	 **/
	static int groundTypeCount()
	{
		return GroundLast;
	}

	/**
	 * @return The number of the tile in the tile file (earth.png)
	 **/
	static int tile(int groundType, unsigned char version)
	{
		return (groundType << 2 | (version & 0x3) );
	}

	/**
	 * @return Whether ground is a plain tile
	 **/
	static bool isPlain(int ground);
	static bool isValidGround(int ground);
	static bool isTrans(int ground);
	static bool isSmallTrans(int g);
	static bool isBigTrans(int g);

	/**
	 * @return How many tiles of every transition exist.
	 **/
	static int tilesPerTransition();
	static int groundTilesNumber();
	static int bigTilesPerTransition();
	static int smallTilesPerTransition();

	static int getTransRef(int g); // does this return a @ref TransType ??

	/**
	 * @return The tile number with transRef and transTile
	 **/
	static int getTransNumber(TransType transRef, int transTile);
	
	/**
	 * @return The number of the big tile with transRef and transTile
	 **/
	static int getBigTransNumber(TransType transRef, int transTile);

	static int smallTileNumber(int smallNo, TransType trans, bool inverted);

	/**
	 * @return With which groundtype this transition starts.
	 **/
	static GroundType from(TransType trans);

	/**
	 * @return With which groundtype this transition ends.
	 **/
	static GroundType to(TransType trans);

	static int getTransTile(int g);

	/**
	 * Add a unit to this cell. All this function does is to increase the
	 * value returned by @ref unitCount by one.
	 **/
	void addUnit(QCanvasItem* u) { mItems.appendUnit(u); }

	/**
	 * Remove a previously added unit from the cell
	 **/
	void removeUnit(QCanvasItem* u) { mItems.removeUnit(u); }


	/**
	 * @param Consider moving units as collisions, too if TRUE, otherwise
	 * not.
	 * @return @ref BoItemList::isOccupied
	 **/
	bool isOccupied(Unit* forUnit = 0, bool includeMoving = true) const { return mItems.isOccupied(forUnit, includeMoving); }

	const BoItemList* items() const { return &mItems; }
	unsigned int unitCount() const { return mItems.count(); }

protected:
	void setVersion(unsigned char v)
	{
		mVersion = v;
	}

	void setGroundType(GroundType type);

private:
	GroundType mType;
	unsigned char mVersion;

	BoItemList mItems;
};

#endif
