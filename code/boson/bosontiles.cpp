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

QPixmap BosonTiles::big1(int bigNo, Cell::TransType trans, bool inverted) // bigNo = 0..4
{
 return tile(Cell::getBigTransNumber(trans, (inverted ? 4 : 0) + bigNo));
}

QPixmap BosonTiles::big2(int bigNo, Cell::TransType trans, bool inverted) // bigNo = 0..4
{
 return tile(Cell::getBigTransNumber(trans, (inverted ? 12 : 8) + bigNo));
}

QPixmap BosonTiles::small(int smallNo, Cell::TransType trans, bool inverted)
{
 return tile(Cell::smallTileNumber(smallNo, trans, inverted));
}
	
// call this like the original fillGroundPixmap() in editorTopLevel.cpp
QPixmap BosonTiles::tile(int g)
{
 QPixmap p;
 if (Cell::isBigTrans(g)) {
	p.resize(2 * BO_TILE_SIZE, 2 * BO_TILE_SIZE);
 } else {
	p.resize(BO_TILE_SIZE, BO_TILE_SIZE);
 }

 g<<=2;

 bitBlt(&p, 0, 0, this, big_x(g), big_y(g), BO_TILE_SIZE, BO_TILE_SIZE);

 // a big tile is 2*2 normal tiles - the upper left was painted
 // above. The following will paint the remaining 3 rects
 if (Cell::isBigTrans(g>>2)) {
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
