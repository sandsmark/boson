#ifndef __BOSONTILES_H__
#define __BOSONTILES_H__

#include "cell.h"

#include <qpixmap.h>

class BosonTiles : public QPixmap
{
public:
	BosonTiles(const QString& fileName);
	~BosonTiles();

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

	enum TransType {
		TransGrassWater = 0,
		TransGrassDesert,
		TransDesertWater,
		TransDeepWater,
		TransLast
	};

	QPixmap plainTile(Cell::GroundType type);

	QPixmap big1(int bigNo, TransType trans, bool inverted); // bigNo = 0..4

	QPixmap big2(int bigNo, TransType trans, bool inverted); // bigNo = 0..4

	QPixmap small(int smallNo, TransType trans, bool inverted);
	/**
	 * @return a tile number for @ref tile
	 **/
	static int smallTileNumber(int smallNo, TransType transRef, bool inverted);
	
	// call this like the original fillGroundPixmap() in editorTopLevel.cpp
	QPixmap tile(int g);

	static int getBigTransNumber(TransType transRef, int transTile);

protected:
	int big_w() const;
	int big_x(int g) const;
	int big_y(int g) const;
	static int smallTilesPerTransition();
	static int bigTilesPerTransition();
	static int tilesPerTransition();
	static int groundTilesNumber(); 
	static int getTransNumber(TransType transRef, int transTile);
	static int getTransRef(int g);
	static int getTransTile(int g);
	static bool isTrans(int g);
	static bool isSmallTrans(int g);
	static bool isBigTrans(int g);

	static Cell::GroundType from(TransType trans);
	static Cell::GroundType to(TransType trans);

};

#endif
