#ifndef __DUNNO_H__
#define __DUNNO_H__

#include "cell.h"

// I dont know what this stuff actually means... :-(((
// and where to place it...


class Dunno
{
public:
	Dunno()
	{
	}

	~Dunno()
	{
	}


	enum TransType {
		TransGrassWater = 0, // TRANS_GW
		TransGrassDesert,    // TRANS_GW
		TransDesertWater,    // TRANS_DW
		TransDeepWater,      // TRANS_DWD
		TransLast
	};

	static int smallTilesPerTransition()
	{
		return 12;
	}
	static int bigTilesPerTransition()
	{
		return 16;
	}
	
	static int tilesPerTransition()
	{
		return smallTilesPerTransition() + bigTilesPerTransition() * 4;
	}

	static int groundTilesNumber()
	{
		return Cell::GroundLast + TransLast * tilesPerTransition();
	}

	static int getTransNumber(TransType transRef, int transTile)
	{
		return Cell::GroundLast + tilesPerTransition() * (int)transRef + transTile;
	}

	static int getBigTransNumber(TransType transRef, int transTile)
	{
		return getTransNumber(transRef, smallTilesPerTransition() + 4 * transTile);
	}

	static int getTransRef(int g)
	{
		return ((g - Cell::GroundLast) / tilesPerTransition());
	}
	static int getTransTile(int g)
	{
		return ((g - Cell::GroundLast) % tilesPerTransition());
	}

	/**
	 * @return Maybe: is this a transition?
	 **/
	static bool isTrans(int g)
	{
		return (g >= Cell::GroundLast && g < groundTilesNumber());
	}
	
	/**
	 * @return Maybe: is this a small transition?
	 **/
	static bool isSmallTrans(int g)
	{
		return (isTrans(g) && getTransTile(g) < smallTilesPerTransition());
	}
	
	/**
	 * @return Maybe: is this a big transition?
	 **/
	static bool isBigTrans(int g)
	{
		return (isTrans(g) && getTransTile(g) >= smallTilesPerTransition());
	}
};

#warning Dont include this
#endif

