#include "bosontiles.h"

#include "defines.h"

#include <kdebug.h>

BosonTiles::BosonTiles(const QString& fileName) : QPixmap(fileName)
{
}

BosonTiles::~BosonTiles()
{
}

QPixmap BosonTiles::plainTile(Cell::GroundType type)
{
 if (type <= Cell::GroundUnknown || type >= Cell::GroundLast) {
	kdError() << "invalid groundtype " << (int)type << endl;
	return QPixmap();
 }
 return tile((int)type);
}

QPixmap BosonTiles::big1(int bigNo, TransType trans, bool inverted) // bigNo = 0..4
{
 return tile(getBigTransNumber(trans, (inverted ? 4 : 0) + bigNo));
}

QPixmap BosonTiles::big2(int bigNo, TransType trans, bool inverted) // bigNo = 0..4
{
 return tile(getBigTransNumber(trans, (inverted ? 12 : 8) + bigNo));
}

int BosonTiles::smallTileNumber(int smallNo, TransType trans, bool inverted)
{
 int tileNo;
 switch (smallNo) {
	case 0:
		tileNo = getTransNumber(trans,
				inverted ? TransUpLeftInverted
				: TransUpLeft);
		break;
	case 1:
		tileNo = getTransNumber(trans, inverted ? 
				TransDown : TransUp);
		break;
	case 2:
		tileNo = getTransNumber(trans,
				inverted ? TransUpRightInverted
				: TransUpRight);
		break;
	case 3:
		tileNo = getTransNumber(trans,
				inverted ? TransRight
				: TransLeft);
		break;
	case 4:
		tileNo = getTransNumber(trans,
				inverted ? to(trans)
				: from(trans));
		break;
	case 5:
		tileNo = getTransNumber(trans,
				inverted ? TransLeft
				: TransRight);
		break;
	case 6:
		tileNo = getTransNumber(trans,
				inverted ? TransDownLeftInverted
				: TransDownLeft);
		break;
	case 7:
		tileNo = getTransNumber(trans,
				inverted ? TransUp
				: TransDown);
		break;
	case 8:
		tileNo = getTransNumber(trans,
				inverted ? TransDownRightInverted
				: TransDownRight);
		break;
	default:
		kdError() << "Unknwon small tile " << smallNo << endl;
		return 0;
 }
 return tileNo;
}

QPixmap BosonTiles::small(int smallNo, TransType trans, bool inverted)
{
 return tile(smallTileNumber(smallNo, trans, inverted));
}
	
// call this like the original fillGroundPixmap() in editorTopLevel.cpp
QPixmap BosonTiles::tile(int g)
{
 QPixmap p;
 if (isBigTrans(g)) {
	p.resize(2 * BO_TILE_SIZE, 2 * BO_TILE_SIZE);
 } else {
	p.resize(BO_TILE_SIZE, BO_TILE_SIZE);
 }

 g<<=2;

 bitBlt(&p, 0, 0, this, big_x(g), big_y(g), BO_TILE_SIZE, BO_TILE_SIZE);

 // a big tile is 2*2 normal tiles - the upper left was painted
 // above. The following will paint the remaining 3 rects
 if (isBigTrans(g>>2)) {
	g+=4;
	bitBlt(&p, BO_TILE_SIZE, 0, this, big_x(g), big_y(g),
			BO_TILE_SIZE, BO_TILE_SIZE);
	g+=4;
	bitBlt(&p, 0, BO_TILE_SIZE, this, big_x(g), big_y(g),
			BO_TILE_SIZE, BO_TILE_SIZE);
	g+=4;
	bitBlt(&p, BO_TILE_SIZE, BO_TILE_SIZE, this, big_x(g),
			big_y(g), BO_TILE_SIZE, BO_TILE_SIZE);
 }

 return p;
}

int BosonTiles::big_w() const
{
 return 32;
}

int BosonTiles::big_x(int g) const
{ 
 return ((g % big_w()) * BO_TILE_SIZE);
}

int BosonTiles::big_y(int g) const
{
 return ((g / big_w()) * BO_TILE_SIZE);
}

int BosonTiles::smallTilesPerTransition()
{
 return 12;
}

int BosonTiles::bigTilesPerTransition() 
{
 return 16;
}

int BosonTiles::tilesPerTransition() 
{
 return smallTilesPerTransition() + 4 * bigTilesPerTransition();
}

int BosonTiles::groundTilesNumber() 
{
 return Cell::GroundLast + TransLast * tilesPerTransition(); 
}

int BosonTiles::getTransNumber(TransType transRef, int transTile)
{
 return Cell::GroundLast + tilesPerTransition() * (int)transRef + transTile;
}

int BosonTiles::getBigTransNumber(TransType transRef, int transTile)
{
 return getTransNumber(transRef, smallTilesPerTransition() + 4 * transTile);
}

int BosonTiles::getTransRef(int g) 
{
 return ((g - Cell::GroundLast) / tilesPerTransition()); 
}

int BosonTiles::getTransTile(int g)
{
 return ((g - Cell::GroundLast) % tilesPerTransition());
}

bool BosonTiles::isTrans(int g)
{
 return (g >= Cell::GroundLast && g < groundTilesNumber());
}

bool BosonTiles::isSmallTrans(int g)
{
 return (isTrans(g) && getTransTile(g) < smallTilesPerTransition());
}

bool BosonTiles::isBigTrans(int g)
{
 return (isTrans(g) && getTransTile(g) >= smallTilesPerTransition());
}

Cell::GroundType BosonTiles::from(TransType trans)
{
 switch (trans) {
	case TransGrassWater:
		return Cell::GroundGrass;
	case TransGrassDesert:
		return Cell::GroundGrass;
	case TransDesertWater:
		return Cell::GroundDesert;
	case TransDeepWater:
		return Cell::GroundDeepWater;
	default:
		kdError() << "Unknown trans " << (int)trans << endl;
		return Cell::GroundUnknown;
 }
}

Cell::GroundType BosonTiles::to(TransType trans)
{
 switch (trans) {
	case TransGrassWater:
		return Cell::GroundWater;
	case TransGrassDesert:
		return Cell::GroundDesert;
	case TransDesertWater:
		return Cell::GroundWater;
	case TransDeepWater:
		return Cell::GroundWater;
	default:
		kdError() << "Unknown trans " << (int)trans
				<< endl;
		return Cell::GroundUnknown;
 }
}
