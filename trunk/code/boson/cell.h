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
#ifndef __CELL_H__
#define __CELL_H__

class UnitProperties;
class QCanvas;
class QCanvasSprite;

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

	bool isFogged() const { return mFog != 0; }
	
	/**
	 * Place fog of war on this cell
	 * @param canvas The canvas where to create the fog
	 * @param x The The horizontal number of this cell
	 * @param y The The vertical number of this cell
	 **/
	void fog(QCanvas* canvas, int x, int y);

	/**
	 * Remove any fog of war from this cell
	 **/
	void unfog();

protected:
	void setVersion(unsigned char v)
	{
		mVersion = v;
	}

	void setGroundType(GroundType type);

private:
	GroundType mType;
	unsigned char mVersion;
	QCanvasSprite* mFog;
};

#endif
