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
#include "bosontiles.h"

#include "defines.h"

#include <kdebug.h>

#include <qimage.h>

BosonTiles::BosonTiles(const QString& fileName) : QPixmap(fileName)
{
 mTilesImage = 0;
}

BosonTiles::BosonTiles()
{
 mTilesImage = new QImage(big_w() * BO_TILE_SIZE, big_h() * BO_TILE_SIZE, 32);
}

BosonTiles::~BosonTiles()
{
 if (mTilesImage) {
	delete mTilesImage;
 }
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

int BosonTiles::big_w()
{
 return 32;
}

int BosonTiles::big_h()
{
 return ((Cell::groundTilesNumber() * 4 + big_w() - 1) / big_w());
}

int BosonTiles::big_x(int g)
{ 
 return ((g % big_w()) * BO_TILE_SIZE);
}

int BosonTiles::big_y(int g)
{
 return ((g / big_w()) * BO_TILE_SIZE);
}

bool BosonTiles::loadTiles(const QString& dir, bool debug)
{
 // dir is e.g. /opt/kde3/share/apps/boson/themes/grounds/earth/ -> "earth" is
 // the important part!
 mDebug = debug;
 mTilesImage->fill(0x00000000); // black filling, FOW _is_ black

 for (int i = 0; i < Cell::GroundLast; i++)    {       // load non-transitions
	if (!loadGround(i, dir + groundType2Name((Cell::GroundType)i))) {
		return false;
	}
 }
 for (int i = 0; i < Cell::TransLast; i++) {                // load transitions
	int j = 0;
	for (j = 0; j < Cell::smallTilesPerTransition(); j++) {
		if (!loadTransition(dir, Cell::getTransNumber((Cell::TransType)i, j))) {
			return false;
		}
	}
	for ( ; j < Cell::tilesPerTransition(); j += 4) {
		if (!loadTransition(dir, Cell::getTransNumber((Cell::TransType)i, j))) {
			return false;
		}
	}
 }
 return true;
}

bool BosonTiles::save(const QString& fileName)
{
 if (!mTilesImage) {
	kdError() << k_funcinfo << ": NULL image" << endl;
	return false;
 }
 return mTilesImage->save(fileName, "PNG");
}

bool BosonTiles::loadGround(int j, const QString& path)
{
 QString tile;
 QImage p;
 for (int i = 0; i < 4; i++) {
	tile.sprintf("-%.2d.bmp", i);
	QString file = path + tile;
	p.load(file);
	if (p.isNull()) {
		kdError() << k_funcinfo << ": couldn't load image " << file << endl;
		return false;
	}
	putOne(4 * j + i, p);
	if (Cell::isBigTrans(j)) {
		putOne(4 * (j + 1) + i, p, BO_TILE_SIZE, 0);
		putOne(4 * (j + 2) + i, p, 0, BO_TILE_SIZE);
		putOne(4 * (j + 3) + i, p, BO_TILE_SIZE, BO_TILE_SIZE);
	}
 }
 return true;
}

void BosonTiles::putOne(int z, QImage& p, int xoffset, int yoffset)
{
// AB it seems that this copies the image p into the image (and if _debug is
// true puts some extra information on it)
 int x = BosonTiles::big_x(z);
 int y = BosonTiles::big_y(z);

 
 #define SETPIXEL(x,y) p.setPixel( xoffset+(x) , yoffset+(y) , 0x00ff0000 )
 #define SETPIXEL2(x,y) \
	SETPIXEL(2*(x), 2*(y));		\
	SETPIXEL(2*(x)+1, 2*(y));	\
	SETPIXEL(2*(x), 2*(y)+1);	\
	SETPIXEL(2*(x)+1, 2*(y)+1)
 #define SETPIXEL3(x,y) \
	SETPIXEL2(2*(x), 2*(y));	\
	SETPIXEL2(2*(x)+1, 2*(y));	\
	SETPIXEL2(2*(x), 2*(y)+1);	\
	SETPIXEL2(2*(x)+1, 2*(y)+1)
 
 if (mDebug) {
	int i;
	for(i = 0; i < BO_TILE_SIZE; i++) {
		SETPIXEL(0,i);
		SETPIXEL(BO_TILE_SIZE-1,i);
		SETPIXEL(i,0);
		SETPIXEL(i,BO_TILE_SIZE-1);
	}	// print the # version
	switch(z%4) {
		case 0:
			SETPIXEL3(3,4);
			SETPIXEL3(4,3);
			SETPIXEL3(5,3);
			SETPIXEL3(5,4);
			SETPIXEL3(5,5);
			SETPIXEL3(5,6);
			SETPIXEL3(5,7);
			break;
		case 1:
			SETPIXEL3(4,4);
			SETPIXEL3(5,3);
			SETPIXEL3(6,3);
			SETPIXEL3(7,4);
			SETPIXEL3(6,5);
			SETPIXEL3(5,6);
			SETPIXEL3(4,7);
			SETPIXEL3(5,7);
			SETPIXEL3(6,7);
			SETPIXEL3(7,7);
			break;
		case 2:
			SETPIXEL3(4,3);
			SETPIXEL3(5,3);

			SETPIXEL3(4,5);
			SETPIXEL3(5,5);

			SETPIXEL3(4,7);
			SETPIXEL3(5,7);

			SETPIXEL3(6,4);
			SETPIXEL3(6,5);
			SETPIXEL3(6,6);
			break;
		case 3:
			SETPIXEL3(6,3);
			SETPIXEL3(5,4);
			SETPIXEL3(4,5);
			SETPIXEL3(4,6);
			SETPIXEL3(5,6);
			SETPIXEL3(6,6);
			SETPIXEL3(7,6);

			SETPIXEL3(7,5);
			SETPIXEL3(7,7);
			break;
		default:
			kdError() << k_funcinfo << ": Unexpected value" << endl;
			return;
	}

 }

 #undef SETPIXEL3
 #undef SETPIXEL2
 #undef SETPIXEL

 bitBlt(mTilesImage, x, y, &p, xoffset, yoffset, BO_TILE_SIZE, BO_TILE_SIZE);
}

QString BosonTiles::groundType2Name(Cell::GroundType g)
{
 switch (g) {
	case Cell::GroundUnknown:
		return QString::fromLatin1("hidden");
	case Cell::GroundDeepWater:
		return QString::fromLatin1("dwater");
	case Cell::GroundWater:
		return QString::fromLatin1("water");
	case Cell::GroundGrass:
		return QString::fromLatin1("grass");
	case Cell::GroundDesert:
		return QString::fromLatin1("desert");
	case Cell::GroundGrassMineral:
		return QString::fromLatin1("grass_mineral");
	case Cell::GroundGrassOil:
		return QString::fromLatin1("grass_oil");
	default:
		kdError() << "Invalid GroundType " << (int)g << endl;
		break;
 }
 return QString::fromLatin1("");
}

QString BosonTiles::transition2Name(Cell::TransType t)
{
 QString name = QString("%1_%2").arg(groundType2Name(Cell::from(t))).
		arg(groundType2Name(Cell::to(t)));
 QString s = name + "/" + name;
 return s;
}

bool BosonTiles::loadTransition(const QString& dir, int gt)
{
 int ref = Cell::getTransRef(gt);
 int t, tile;


 if (!Cell::isTrans(gt)) {
	kdError() << "No transition " << gt << endl;
	return false;
 }

 t = Cell::getTransTile(gt);
 if (t < Cell::smallTilesPerTransition()) {
	tile = t;
 } else {
	t -= Cell::smallTilesPerTransition(); // bigtile #
	t /= 4; // which one
	tile = t + Cell::smallTilesPerTransition(); // tile is the index in trans_ext
 }
 return loadGround(gt, dir + transition2Name((Cell::TransType)ref) + trans_ext(tile));
}

QString BosonTiles::trans_ext(int t)
{
/*
 static const char *trans_ext[Cell::tilesPerTransition()] = {
	".01", ".03", ".07", ".05",	// 48x48 transitions
	".02", ".06", ".08", ".04",
	".09", ".10", ".12", ".11",
	".13", ".14", ".15", ".16",	// 96x96 transitions
	".17", ".18", ".19", ".20",
	".21", ".22", ".23", ".24",
	".25", ".26", ".27", ".28",
	};
 */
 QString s;
 if (t > 11) {
	s.sprintf("_%.2d", t + 1);
 } else {
	// AB: this is unclean. can we do  this without switch, i.e. like above?
	switch (t) {
		case 0:
			s.sprintf("_%.2d", 1);
			break;
		case 1:
			s.sprintf("_%.2d", 3);
			break;
		case 2:
			s.sprintf("_%.2d", 7);
			break;
		case 3:
			s.sprintf("_%.2d", 5);
			break;
		case 4:
			s.sprintf("_%.2d", 2);
			break;
		case 5:
			s.sprintf("_%.2d", 6);
			break;
		case 6:
			s.sprintf("_%.2d", 8);
			break;
		case 7:
			s.sprintf("_%.2d", 4);
			break;
		case 8:
			s.sprintf("_%.2d", 9);
			break;
		case 9:
			s.sprintf("_%.2d", 10);
			break;
		case 10:
			s.sprintf("_%.2d", 12);
			break;
		case 11:
			s.sprintf("_%.2d", 11);
			break;

	}
 }
 return s;
}
