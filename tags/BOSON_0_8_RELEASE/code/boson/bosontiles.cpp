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
#include "bosontiles.moc"

#include "bosontexturearray.h"
#include "bosonconfig.h"
#include "bodebug.h"
#include "defines.h"

#include <qimage.h>
#include <qapplication.h>
#include <qpixmap.h>

BosonTiles::BosonTiles(QObject* parent) : QObject(parent)
{
 mTilesImage = new QImage(big_w() * BO_TILE_SIZE, big_h() * BO_TILE_SIZE, 32);
 mTextures = 0;
}

BosonTiles::~BosonTiles()
{
 delete mTilesImage;
 delete mTextures;
}
/*
QPixmap BosonTiles::plainTile(Cell::GroundType type)
{
 if (type <= Cell::GroundUnknown || type >= Cell::GroundLast) {
	boError() << "invalid groundtype " << (int)type << endl;
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
*/

QPixmap BosonTiles::tile(int g)
{
 QPixmap p;
 if (Cell::isBigTrans(g)) {
	p.resize(2 * BO_TILE_SIZE, 2 * BO_TILE_SIZE);
 } else {
	p.resize(BO_TILE_SIZE, BO_TILE_SIZE);
 }

 g<<=2;

 bitBlt(&p, 0, 0, mTilesImage, big_x(g), big_y(g), BO_TILE_SIZE, BO_TILE_SIZE);

 // a big tile is 2*2 normal tiles - the upper left was painted
 // above. The following will paint the remaining 3 rects
 if (Cell::isBigTrans(g>>2)) {
	g+=4;
	bitBlt(&p, BO_TILE_SIZE, 0, mTilesImage, big_x(g), big_y(g),
			BO_TILE_SIZE, BO_TILE_SIZE);
	g+=4;
	bitBlt(&p, 0, BO_TILE_SIZE, mTilesImage, big_x(g), big_y(g),
			BO_TILE_SIZE, BO_TILE_SIZE);
	g+=4;
	bitBlt(&p, BO_TILE_SIZE, BO_TILE_SIZE, mTilesImage, big_x(g),
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

bool BosonTiles::loadTiles(QString dir, bool debug)
{
 // dir is e.g. /opt/kde3/share/apps/boson/themes/grounds/earth/ -> "earth" is
 // the important part!
 if (dir.right(1) != QString::fromLatin1("/")) {
	dir += QString::fromLatin1("/");
 }
 if (mTilesDir == dir) {
	boDebug() << k_funcinfo << "already loaded from " << dir << ". skipping..." << endl;
	// we have already loaded this. no need to do it again.
	emit signalTilesLoaded();
	return true;
 }
 if (mTextures) {
	boWarning() << k_funcinfo << "already loaded ?!" << endl;
 }
 delete mTextures;
 mTextures = 0;
 mTilesDir = dir;
 mDebug = debug;
 // Variables for progress information
 mLoaded = 0;
 mTilesImage->fill(0x00000000); // black filling, FOW _is_ black


 // AB: note that most of the texture-loading code is a hack here. this class
 // was never meant to use textures...
 // we simply place every image to mTextureImages in putOn(). Then we init
 // mTextures.
 mTextureImages.clear();

 // this can be useful for profiling some stuff. the display will become
 // useless, no useful cell will be rendered, but also no startup time is spent
 // here.
 if (boConfig->loadTiles()) {
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

 } else {
	QImage img(64, 64, 32);
	mTextureImages.insert(0, img);
 }

 generateTextures();

 // AB: tiles are loaded - but the texturs cannot yet be generated! must be done
 // after construction of the gl-context, aka the BosonGLWidget
 emit signalTilesLoaded();
 return true;
}

void BosonTiles::generateTextures()
{
 if (mTextureImages.count() == 0) {
	boError() << k_funcinfo << "0 texture images available!" << endl;
	return;
 }
 QValueList<QImage> images;
 QMap<int, QImage>::Iterator it;
 for (it = mTextureImages.begin(); it != mTextureImages.end(); ++it) {
	images.append(it.data());
 }
 mTextures = new BosonTextureArray(images);
 images.clear();
 mTextureImages.clear(); // free some space - we won't need it anymore, except for reloading the game/map.
}

bool BosonTiles::loadGround(int j, const QString& path)
{
 for (int i = 0; i < 4; i++) {
	QString tile;
	tile.sprintf("-%.2d.png", i);
	QString file = path + tile;
	QImage p(file);
	if (p.isNull()) {
		boError() << k_funcinfo << "couldn't load image " << file << endl;
		return false;
	}
	putOne(4 * j + i, p);
	mLoaded++;
	if (Cell::isBigTrans(j)) {
		putOne(4 * (j + 1) + i, p, BO_TILE_SIZE, 0);
		putOne(4 * (j + 2) + i, p, 0, BO_TILE_SIZE);
		putOne(4 * (j + 3) + i, p, BO_TILE_SIZE, BO_TILE_SIZE);
		mLoaded += 3;
	}
 }
 if ((mLoaded % 10) == 0) {
	emit signalTilesLoading(mLoaded);
 }
 return true;
}

void BosonTiles::putOne(int z, QImage& p, int xoffset, int yoffset)
{
// AB: copy the image p to the big pixmap (used by e.g. the map editor and the QCanvas version) abd to the texture images list (used by OpenGL only)
// TODO: remove the big pixmap for non-QCanvas version. the image list can provide the same functionality in Editor mode
// TODO: clear the image list in non-editor mode once the textures were generated
 int x = BosonTiles::big_x(z);
 int y = BosonTiles::big_y(z);

 if (mTextureImages.contains(z)) {
	boError() << k_funcinfo << z << " is already there" << endl;
	return;
 }
 QImage small(BO_TILE_SIZE, BO_TILE_SIZE, 32);
 bitBlt(&small, 0, 0, &p, xoffset, yoffset, BO_TILE_SIZE, BO_TILE_SIZE);
 mTextureImages.insert(z, small);

 bitBlt(mTilesImage, x, y, &p, xoffset, yoffset, BO_TILE_SIZE, BO_TILE_SIZE);
 if (qApp->hasPendingEvents()) {
//	boDebug() << "process events; mLoaded = " << mLoaded << endl;
	qApp->processEvents(10);
 }
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
		boError() << "Invalid GroundType " << (int)g << endl;
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
	boError() << "No transition " << gt << endl;
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

QPixmap BosonTiles::pixmap() const
{
 QPixmap p;
 p.convertFromImage(*mTilesImage);
 return p;
}

