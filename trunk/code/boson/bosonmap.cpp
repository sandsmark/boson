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

#include "bosonmap.h"
#include "bosonmap.moc"

#include "defines.h"
#include "cell.h"
#include "bosongroundtheme.h"
#include "boson.h" // for an ugly boGame->gameMode() hack!
#include "bodebug.h"

#include <qtimer.h>
#include <qdatetime.h>
#include <qdatastream.h>
#include <qimage.h>
#include <qvaluevector.h>

#include <kapplication.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#define BOSONMAP_VERSION 0x01 // current version

struct TextureGroundType
{
	int mGroundType; // AB: we need a groundType enum for this. of any kind... this variable mustn't depend on that enum though.
	unsigned char mAmountOfLand;
	unsigned char mAmountOfWater;

	// AB: maybe add a minimap color here!
	QRgb mMiniMapColor;
};

class BosonMap::BosonMapPrivate
{
public:
	BosonMapPrivate()
	{
	}
	QValueVector<TextureGroundType> mTextureGroundType;
};

BosonMap::BosonMap(QObject* parent) : QObject(parent)
{
 init();
}

BosonMap::~BosonMap()
{
 delete mGroundTheme;
 delete[] mCells;
 delete[] mHeightMap;
 delete[] mTexMap;
 delete d;
}

void BosonMap::init()
{
 d = new BosonMapPrivate;
 mCells = 0;
 mHeightMap = 0;
 mGroundTheme = 0;
 mTexMap = 0;
 mTextureCount = 0;
 mMapWidth = 0;
 mMapHeight = 0;
 setModified(false);
}

bool BosonMap::loadMapFromFile(const QByteArray& map)
{
 boDebug() << k_funcinfo << endl;
 QDataStream stream(map, IO_ReadOnly);
 QString magic;
 stream >> magic;
 if (magic != BOSONMAP_MAP_MAGIC_COOKIE) {
	boError() << k_funcinfo << "invalid magic cookie" << endl;
	return false;
 }
 Q_UINT32 version;
 stream >> version;
 if (version != BOSONMAP_VERSION) {
	boError() << k_funcinfo << "version " << version << " not supported" << endl;
	return false;
 }
 if (!loadMapGeo(stream)) {
	boError() << k_funcinfo << "Could not load map geo" << endl;
	return false;
 }
 if (!stream.atEnd()) {
	boWarning() << k_funcinfo << "stream is not at end after map geo!" << endl;
 }
 return true;
 // AB: cells are NOT loaded here anymore!
#if 0
 if (!loadCells(stream)) {
	boError() << k_funcinfo << "Could not load map cells" << endl;
	return false;
 }
 return true;
#endif
}


bool BosonMap::loadCompleteMap(QDataStream& stream)
{
 boDebug() << k_funcinfo << endl;
 if (!loadMapGeo(stream)) {
	boError() << k_funcinfo << "Could not load map geo" << endl;
	return false;
 }
 if (!loadHeightMap(stream)) {
	boError() << k_funcinfo << "Could not load height map" << endl;
	return false;
 }
 if (stream.atEnd()) {
	// do NOT try to generate a texmap. here we are loading a remote stream,
	// which must contain the correct texmap.
	boError() << k_funcinfo << "stream has no texmap! must not happen here!" << endl;
	return false;
 }
 if (!loadTexMap(stream)) {
	boError() << k_funcinfo << "Could not load texmap" << endl;
	return false;
 }
 if (!loadCells(stream)) {
	boError() << k_funcinfo << "Could not load map cells" << endl;
	return false;
 }
 return true;
}

bool BosonMap::loadMapGeo(QDataStream& stream)
{
 Q_UINT32 mapWidth;
 Q_UINT32 mapHeight;

 stream >> mapWidth;
 stream >> mapHeight;

 // check 'realityness'
 if (mapWidth < 10) {
	boError() << k_funcinfo << "broken map file!" << endl;
	boError() << "mapWidth < 10" << endl;
	return false;
 }
 if (mapHeight < 10) {
	boError() << k_funcinfo << "broken map file!" << endl;
	boError() << "mapHeight < 10" << endl;
	return false;
 }
 if (mapWidth > MAX_MAP_WIDTH) {
	boError() << k_funcinfo << "broken map file!" << endl;
	boError() << "mapWidth > " << MAX_MAP_WIDTH << endl;
	return false;
 }
 if (mapHeight > MAX_MAP_HEIGHT) {
	boError() << k_funcinfo << "broken map file!" << endl;
	boError() << "mapHeight > " << MAX_MAP_HEIGHT << endl;
	return false;
 }

// map is ok - lets apply
 mMapWidth = mapWidth; // horizontal cell count
 mMapHeight = mapHeight; // vertical cell count

 delete[] mCells;
 delete[] mHeightMap;
 delete[] mTexMap;

 boDebug() << k_funcinfo << endl;
 mCells = new Cell[width() * height()];
 mHeightMap = new float[(width() + 1) * (height() + 1)];
 mTexMap = 0; // it is NOT loaded here
 for (unsigned int x = 0; x < width(); x++) {
	for (unsigned y = 0; y < height(); y++) {
		Cell* c = cell(x, y);
		if (!c) {
			boError() << k_funcinfo << "Evil internal error!" << endl;
			continue;
		}
		c->setPosition(x, y);
	}
 }
 // AB: is it clever to load a height map here? we have to allocate a lot of
 // memory for it - even when preloading maps that will never get used
 return true;
}

bool BosonMap::loadCells(QDataStream& stream)
{
 if (!mCells) {
	boError() << k_funcinfo << "NULL cells" << endl;
	return false;
 }
// load all cells:
 for (unsigned int i = 0; i < width(); i++) {
	for (unsigned int j = 0; j < height(); j++) {
		unsigned char amountOfLand = 0;
		unsigned char amountOfWater = 0;
		if (!loadCell(stream, &amountOfLand, &amountOfWater)) {
			return false;
		}
		slotChangeCell(i, j, amountOfLand, amountOfWater);
	}
 }
 return true;
}

bool BosonMap::importHeightMapImage(const QImage& image)
{
 if (image.isNull()) {
	return false;
 }
 if ((unsigned int)image.width() != width() + 1 ||
		(unsigned int)image.height() != height() + 1) {
	return false;
 }
 QByteArray b;
 QDataStream s(b, IO_WriteOnly);
 QImageIO io;
 io.setIODevice(s.device());
 io.setFormat("PNG");
 io.setImage(image);
 io.write();
 return loadHeightMapImage(b);
}

bool BosonMap::loadHeightMapImage(const QByteArray& heightMap)
{
 boDebug() << k_funcinfo << endl;
 if (heightMap.size() == 0) {
	// initialize the height map with 0.0
//	boDebug() << k_funcinfo << "loading dummy height map" << endl;
	for (unsigned int x = 0; x < width() + 1; x++) {
		for (unsigned int y = 0; y < height() + 1; y++) {
			mHeightMap[y * (width() + 1) + x] = 0.0;
		}
	}
	return true;
 }
 boDebug() << k_funcinfo << "loading real height map" << endl;
 QImage map(heightMap);
 if (!map.isGrayscale()) {
	// we load a valid height map (i.e. a dummy height map) but still return
	// false for error checking.
	boError() << k_funcinfo << "not a grayscale image" << endl;
	loadHeightMapImage(QByteArray());
	return false;
 }
 if ((unsigned int)map.height() != height() + 1) {
	boError() << k_funcinfo << "invalid height of heightmap: " <<
			map.height() << " must be: " << height() + 1 << endl;
	loadHeightMapImage(QByteArray());
	return false;
 }
 if ((unsigned int)map.width() != width() + 1) {
	boError() << k_funcinfo << "invalid widthof heightmap: "
			<< map.width() << " must be: " << width() + 1 << endl;
	loadHeightMapImage(QByteArray());
	return false;
 }
 int increment = 1;
 if (map.bytesPerLine() > map.width() + 1) {
	// QT doesn't save images as grayscale. it returns bytesPerLine() ==
	// width() (i.e. 8bits per pixel) for all *actual* grayscale images
	// and bytesPerLine() == width() * 4 for grayscale images that Qt
	// has created (i.e. RGB/RGBA images). we only use grayscale here, that
	// means red=blue=green component. so we can just pick the first and
	// skip all (including alpha) other values.
	increment = 4;
 }
 for (unsigned int y = 0; y < height() + 1; y++) {
	// AB: warning: from Qt docs: "If you are accessing 16-bpp image data,
	// you must handle endianness yourself."
	// do we have to care about this? (since we are using 16bpp)
	// AB: we use 32bpp (qt doesnt support 16bpp on X11)
	// AB: hmm accordint to "file" we use 6bpp only... ok thats easier
	// then :)
	int imageX = 0;
	unsigned char* line = map.scanLine(y);
	for (unsigned int x = 0; x < width() + 1; x++, imageX += increment) {
		mHeightMap[y * (width() + 1) + x] = pixelToHeight(line[imageX]);
	}
 }

 // No need to recalculate cell values here since actual values will be loaded
 //  from network stream later... right?
 boDebug() << k_funcinfo << "done" << endl;
 return true;
}

bool BosonMap::loadHeightMap(QDataStream& stream)
{
 if (!mHeightMap) {
	boError() << k_funcinfo << "NULL heightmap" << endl;
	return false;
 }
 boDebug() << k_funcinfo << "loading height map from network stream" << endl;
 for (unsigned int x = 0; x < width() + 1; x++) {
	for (unsigned int y = 0; y < height() + 1; y++) {
		stream >> mHeightMap[y * (width() + 1) + x];
	}
 }
 return true;
}

bool BosonMap::loadTexMap(QDataStream& stream)
{
 boDebug() << k_funcinfo << endl;
 if (stream.atEnd()) {
	boError() << k_funcinfo << "empty stream" << endl;
	return false;
 }
 if (mTexMap) {
	boWarning() << k_funcinfo << "already a texmap present - deleting..." << endl;
	delete[] mTexMap;
	mTexMap = 0;
 }
 if (width() * height() <= 0) {
	boError() << k_funcinfo << "width=" << width() << " height=" << height() << endl;
	return false;
 }
 boDebug() << k_funcinfo << "loading texmap from stream" << endl;
 QString cookie;
 Q_UINT32 version;
 stream >> cookie;
 if (cookie != BOSONMAP_TEXMAP_MAGIC_COOKIE) {
	boError() << k_funcinfo << "invalid cookie" << endl;
	return false;
 }
 stream >> version;
 if (version != BOSONMAP_VERSION) {
	boError() << k_funcinfo << "version " << version << " not supported" << endl;
	return false;
 }
 Q_UINT32 textures;
 stream >> textures;
 mTextureCount = textures;
 if (mTextureCount < 1) {
	boError() << k_funcinfo << textures << " textures is not possible" << endl;
	return false;
 }
 if (stream.atEnd()) {
	boError() << k_funcinfo << "stream at end" << endl;
	return false;
 }
 mTexMap = new unsigned char[texMapArrayPos(mTextureCount - 1, width(), height())];
 for (unsigned int i = 0; i < mTextureCount; i++) {
	Q_INT32 groundType; //AB: note this is NOT Cell::groundType()! although it means the same thing! but it operates on different numbers
	QRgb miniMapColor;
	Q_UINT8 amountOfLand;
	Q_UINT8 amountOfWater;
	stream >> groundType;
	stream >> miniMapColor;
	stream >> amountOfLand;
	stream >> amountOfWater;
	setTextureGroundType(i, groundType, miniMapColor, amountOfLand, amountOfWater);
	for (unsigned int x = 0; x < width() + 1; x++) {
		for (unsigned int y = 0; y < height() + 1; y++) {
			Q_UINT8 c;
			stream >> c;
			mTexMap[texMapArrayPos(i, x, y)] = c;
		}
	}
 }
 boDebug() << "done" << endl;
 return true;
}

bool BosonMap::saveTexMap(QDataStream& stream)
{
 boDebug() << k_funcinfo << endl;
 if (!mTexMap) {
	BO_NULL_ERROR(mTexMap);
	return false;
 }
 if (mTextureCount > 100) {
	// this *cant* be true (would be > 100*500*500 bytes on a 500x500 map)
	boError() << k_funcinfo << "texture count > 100: " << mTextureCount << " - won't save anything." << endl;
	return false;
 }
 if (textureCount()< 1) {
	boError() << k_funcinfo << "need at least one texture!" << endl;
	return false;
 }
 if (d->mTextureGroundType.count() < textureCount()) {
	boError() << k_funcinfo << "groundType information for "
			<< d->mTextureGroundType.count()
			<< " groundTypes available only. need "
			<< textureCount() << endl;
	return false;
 }
 stream << BOSONMAP_TEXMAP_MAGIC_COOKIE;
 stream << (Q_UINT32)BOSONMAP_VERSION;
 stream << (Q_UINT32)mTextureCount;
 for (unsigned int i = 0; i < mTextureCount; i++) {
	// which type of ground is this (grass, desert, water, vulcan, ...)
	stream << (Q_INT32)groundType(i);
	stream << (QRgb)miniMapColor(i);

	// amount of land/water of this texture
	stream << (Q_UINT8)amountOfLand(i);
	stream << (Q_UINT8)amountOfWater(i);

	// now the actual texmap for this texture
	for (unsigned int x = 0; x < width() + 1; x++) {
		for (unsigned int y = 0; y < height() + 1; y++) {
			stream << (Q_UINT8)mTexMap[texMapArrayPos(i, x, y)];
		}
	}
 }
 return true;
}

bool BosonMap::saveMapToFile(QDataStream& stream)
{
 if (!saveMapGeo(stream)) {
	boError() << k_funcinfo << "Could not save map geo" << endl;
	return false;
 }
#if 0
 // obsolete.
 if (!saveCells(stream)) {
	boError() << k_funcinfo << "Could not save map cells" << endl;
	return false;
 }
#endif
 return true;
}

bool BosonMap::saveCompleteMap(QDataStream& stream)
{
 // AB: we may have a problem here - this stream is meant to be sent through
 // network, but it is very big! (sometimes several MB)
 // we should compress it!
 if (!saveMapGeo(stream)) {
	boError() << k_funcinfo << "Could not save map geo" << endl;
	return false;
 }
 if (!saveHeightMap(stream)) {
	boError() << k_funcinfo << "Could not save height map" << endl;
	return false;
 }
 if (!saveTexMap(stream)) {
	boError() << k_funcinfo << "Could not save texmap" << endl;
 }
 if (!saveCells(stream)) {
	boError() << k_funcinfo << "Could not save map cells" << endl;
	return false;
 }
 return true;
}

bool BosonMap::saveMapGeo(QDataStream& stream)
{
 if (!isValid()) {
	boError() << k_funcinfo << "Map geo is not valid" << endl;
	return false;
 }
// boDebug() << k_funcinfo << endl;
 stream << (Q_INT32)width();
 stream << (Q_INT32)height();
 return true;
}

bool BosonMap::saveCells(QDataStream& stream)
{
 for (unsigned int i = 0; i < width(); i++) {
	for (unsigned int j = 0; j < height(); j++) {
		Cell* c = cell(i, j);
		if (!c) {
			boError() << k_funcinfo << "NULL Cell" << endl;
			// do not abort - otherwise all clients receiving this
			// stream are completely broken as we expect
			// width()*height() cells
			saveCell(stream, 0, 0);
		} else {
			saveCell(stream, c->amountOfLand(), c->amountOfWater());
		}
	}
 }
 return true;
}

bool BosonMap::saveHeightMap(QDataStream& stream)
{
 if (!mHeightMap) {
	stream << QImage();
	return true;
 }
 if (!mHeightMap) {
	boDebug() << k_funcinfo << "saving dummy heightmap to network stream" << endl;
	for (unsigned int x = 0; x < width() + 1; x++) {
		for (unsigned int y = 0; y < height() + 1; y++) {
			stream << (float)0.0;
		}
	}
	return true;
 }
 boDebug() << k_funcinfo << "saving real heightmap to network stream" << endl;
 for (unsigned int x = 0; x < width() + 1; x++) {
	for (unsigned int y = 0; y < height() + 1; y++) {
		stream << (float)mHeightMap[y * (width() + 1) + x];
	}
 }
 return true;
}

QByteArray BosonMap::saveHeightMapImage()
{
 // this function is sloooow !
 if (!width() || !height()) {
	boError() << k_funcinfo << "Cannot save empty map" << endl;
	return QByteArray();
 }
 QImage image;
 if (!image.create(width() + 1, height() + 1, 32, 0)) { // AB: 16bpp isnt available for X11 (only for qt embedded)
	boError() << k_funcinfo << "Unable to create height map!" << endl;
	return QByteArray();
 }
 boDebug() << k_funcinfo << "heightmap: " << image.width() << "x" << image.height() << endl;
 if (!mHeightMap) {
	boDebug() << k_funcinfo << "dummy height map..." << endl;
	int l = BosonMap::heightToPixel(0.0f);
	image.fill(l);
 } else {
	boDebug() << k_funcinfo << "real height map" << endl;
	// AB: this *might* be correct, but i am not sure about this. (02/11/22)
	for (int y = 0; y < image.height(); y++) {
		uint* p = (uint*)image.scanLine(y);
		for (int x = 0; x < image.width(); x++) {
			float value = mHeightMap[y * (width() + 1) + x];
			int v = BosonMap::heightToPixel(value);
			*p = qRgb(v, v, v);
			p++;
		}
	}
 }

 if (!image.isGrayscale()) {
	boError() << k_funcinfo << "not a grayscale image!!" << endl;
	return QByteArray();
 }

 QByteArray array;
 QDataStream stream(array, IO_WriteOnly);
 QImageIO io;
 io.setIODevice(stream.device());
 io.setFormat("PNG");
 io.setImage(image);
 io.write();
 return array;
}

bool BosonMap::isValid() const
{
 if (width() < 10) {
	boError() << k_funcinfo << "width < 10" << endl;
	return false;
 }
 if (width() > MAX_MAP_WIDTH) {
	boError() << k_funcinfo << "mapWidth > " << MAX_MAP_WIDTH << endl;
	return false;
 }
 if (height() < 10) {
	boError() << k_funcinfo << "height < 10" << endl;
	return false;
 }
 if (height() > MAX_MAP_HEIGHT) {
	boError() << k_funcinfo << "mapHeight > " << MAX_MAP_HEIGHT << endl;
	return false;
 }
 if (!mCells) {
	boError() << k_funcinfo << "NULL cells" << endl;
	return false;
 }
 if (!mHeightMap) {
	boError() << k_funcinfo << "NULL heightmap" << endl;
	return false;
 }

 //TODO: check cells!

 return true;
}

bool BosonMap::loadCell(QDataStream& stream, unsigned char* amountOfLand, unsigned char* amountOfWater) const
{
 Q_UINT8 land;
 Q_UINT8 water;
 stream >> land;
 stream >> water;

 (*amountOfLand) = land;
 (*amountOfWater) = water;
 return true;
}

void BosonMap::saveCell(QDataStream& stream, unsigned char amountOfLand, unsigned char amountOfWater)
{
 stream << (Q_UINT8)amountOfLand;
 stream << (Q_UINT8)amountOfWater;
}

Cell* BosonMap::cell(int x, int y) const
{
 if (!mCells) {
	boError() << k_funcinfo << "Cells not yet created" << endl;
	return 0;
 }
 if (!isValidCell(x, y)) {
	return 0;
 }
 return mCells + cellArrayPos(x, y);
}

float BosonMap::heightAtCorner(int x, int y) const
{
 if (!mHeightMap) {
	boError() << k_funcinfo << "NULL height map" << endl;
	return 1.0f;
 }
 // note: isValidCell(x,y) might return false, even though the coordinates are
 // valid.
 // if x=width() and y=height() we will return the lower right corner of the
 // cell at x=width()-1 and y=height()-1.
 // This value is equal to the upper left corner of x=width() and y=height() if
 // such a cell would exist.
 if (x < 0 || (unsigned int)(x + 1) >= width()) {
	return 1.0f;
 }
 if (y < 0 || (unsigned int)(y + 1) >= height()) {
	return 1.0f;
 }
 return mHeightMap[cornerArrayPos(x, y)];
}

void BosonMap::setHeightAtCorner(int x, int y, float h)
{
 BO_CHECK_NULL_RET(mHeightMap);
 if (x < 0 || (unsigned int)(x + 1) >= width()) {
	return;
 }
 if (y < 0 || (unsigned int)(y + 1) >= height()) {
	return;
 }

 // AB: see pixelToHeight() for explanation on these restrictions
 h = QMIN(h, 15.0f);
 h = QMAX(h, -10.5f);
 mHeightMap[y * (width() + 1) + x] = h;
}

void BosonMap::slotChangeCell(int x, int y, unsigned char amountOfLand, unsigned char amountOfWater)
{
//boDebug() << x << " -> " << y << endl;
//boDebug() << width() << " " << height() << endl;
 Cell* c = cell(x, y);
 if (!c) {
	boError() << k_funcinfo << "Invalid cell x=" << x << ",y=" << y << endl;
	return;
 }
 if ((int)amountOfLand + (int)amountOfWater > 255) {
	boWarning() << k_funcinfo << "invalid amounts of land/water: " << amountOfLand << "/" << amountOfWater << endl;
	// we do not return - we have to keep in sync with network
 }
 c->makeCell(amountOfLand, amountOfWater);
}

void BosonMap::loadGroundTheme(const QString& theme)
{
 delete mGroundTheme;
 mGroundTheme = new BosonGroundTheme();
 QString dir = KGlobal::dirs()->findResourceDir("data", QString("boson/themes/grounds/%1/index.ground").arg(theme));
 if (dir.isNull()) {
	boError() << k_funcinfo << "Cannot find tileset " << theme << endl;
	return;
 }
 dir += QString("boson/themes/grounds/%1/").arg(theme);
 mGroundTheme->loadGroundTheme(this, dir);
}

void BosonMap::resize(unsigned int width, unsigned int height)
{
 if (!width || !height) {
	boError() << k_funcinfo << "invalid map dimensions: " << width << "x" << height << endl;
	return;
 }
 if (mCells) {
	// TODO: store old cells, create new ones and apply values from old to
	// new cells.
	boError() << k_funcinfo << "only resizing from NULL cells is implemented!" << endl;
	return;
 }

 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);

 // WARNING: this is close to duplicated code. try to merge with saveMapGeo() !
 stream << (Q_INT32)width;
 stream << (Q_INT32)height;

 QDataStream readStream(buffer, IO_ReadOnly);
 loadMapGeo(readStream);
}

BosonTextureArray* BosonMap::textures() const
{
 BO_CHECK_NULL_RET0(mGroundTheme);
 return mGroundTheme->textures();
}


void BosonMap::fill(unsigned int texture)
{
 if (!mCells) {
	boError() << k_funcinfo << "NULL cells" << endl;
	return;
 }
 BO_CHECK_NULL_RET(mTexMap);
 if (texture >= textureCount()) {
	boError() << k_funcinfo << "invalid texture " << texture << endl;
	return;
 }

 // initialize to 0 first
 for (unsigned int x = 0; x < width() + 1; x++) {
	for (unsigned int y = 0; y < height() + 1; y++) {
		for (unsigned int i = 0; i < textureCount(); i++) {
			mTexMap[texMapArrayPos(0, x, y)] = 0;
		}
	}
 }
 for (unsigned int x = 0; x < width() + 1; x++) {
	for (unsigned int y = 0; y < height() + 1; y++) {
		mTexMap[texMapArrayPos(texture, x, y)] = 255;
	}
 }
}

float BosonMap::cellAverageHeight(int x, int y)
{
 Cell* c = cell(x, y);
 if (!c) {
	return 0;
 }

 float minz = 1000.0f;
 float maxz = -1000.0f;

 for (int i = x; i <= x + 1; i++) {
	for (int j = y; j <= y + 1; j++) {
		minz = QMIN(minz, heightAtCorner(i, j));
		maxz = QMAX(maxz, heightAtCorner(i, j));
	}
 }

 return (minz + maxz) / 2.0f;
}

bool BosonMap::importTexMap(const QString& file, int texturesPerComponent, bool useAlpha)
{
 QImage* img = new QImage(file);
 if (img->isNull()) {
	boError() << k_funcinfo << "could not load from " << file << endl;
	delete img;
	return false;
 }
 boDebug() << k_funcinfo << "importing from " << file << endl;
 bool ret = importTexMap(img, texturesPerComponent, useAlpha);
 delete img;
 return ret;
}

bool BosonMap::importTexMap(const QImage* img, int texturesPerComponent, bool useAlpha)
{
 boWarning() << k_funcinfo << "this function is mos probably broken at the moment!" << endl;
 if (!img) {
	BO_NULL_ERROR(img);
	return false;
 }
 if (img->isNull()) {
	boError() << k_funcinfo << "null image" << endl;
	return false;
 }
 if (img->depth() != 32) {
	boError() << k_funcinfo << "only 32 bits per pixel are supported! image has: "
			<< img->depth() << endl;
	return false;
 }
 if (useAlpha && !img->hasAlphaBuffer()) {
	boError() << k_funcinfo << "cannot use alpha, as image doesn't have alpha buffer!" << endl;
	return false;
 }
 if ((unsigned int)img->width() != width() + 1) {
	boError() << k_funcinfo << "image width must be "
			<< width() + 1 << ", is: " << img->width() << endl;
	return false;
 }
 if ((unsigned int)img->height() != height() + 1) {
	boError() << k_funcinfo << "image height must be "
			<< height() + 1 << ", is: " << img->height() << endl;
	return false;
 }
 if (texturesPerComponent <= 0  || texturesPerComponent > 8) {
	boError() << k_funcinfo << "invalid texturesPerComponent: " << texturesPerComponent << endl;
	return false;
 }
 if ((unsigned int)texturesPerComponent * (useAlpha ? 4 : 3)> textureCount()) {
	boWarning() << k_funcinfo << "this map doesn't have " 
			<< texturesPerComponent * (useAlpha ? 4 : 3)
			<< " textures. reset to " << textureCount() << " textures, i.e. "
			<< textureCount() / (useAlpha ? 4 : 3) 
			<< " textures per component" << endl;
	texturesPerComponent = textureCount() / (useAlpha ? 4 : 3);
 }

 if (texturesPerComponent != 1) {
	boError() << k_funcinfo << "currently we are supporting only 1 texture per component!" << endl;
	return false;
 }
 if (!mTexMap) {
	boError() << k_funcinfo << "NULL texmap" << endl;
	return false;
 }

boDebug() << k_funcinfo << endl;
 for (unsigned int y = 0; y < height() + 1; y++) {
	QRgb* line = (QRgb*)img->scanLine(y);
	for (unsigned int x = 0; x < width() + 1; x++) {
		QRgb pixel = line[x];
//		for (int i = 0; i < texturesPerComponent; i++) {
			mTexMap[texMapArrayPos(0, x, y)] = qRed(pixel);
			mTexMap[texMapArrayPos(1, x, y)] = qGreen(pixel);
			mTexMap[texMapArrayPos(2, x, y)] = qBlue(pixel);
//		}
	}
 }
 return true;
}

float BosonMap::pixelToHeight(int p)
{
 // we have 255 different values available. we use 0.1 steps, i.e. if
 // the pixel is increased by 1 the height is increased by 0.1.
 // values below 105 cause a negative height, values above 105 a positive
 // height.
 // this means we have a height range from -105/10 to 150/10 aka from -10.5 to
 // 15.0
// boDebug() << k_funcinfo << p << "->" << ((float)(p-105))/10 << endl;
 return ((float)(p - 105)) / 10;
}

int BosonMap::heightToPixel(float height)
{
 return (int)(height * 10 + 105);
}

bool BosonMap::generateCellsFromTexMap()
{
 boDebug() << k_funcinfo << endl;
 if (width() * height() <= 0) {
	boError() << k_funcinfo << "invalid map size - width=" << width() << " height=" << height() << endl;
	return false;
 }
 if (textureCount() == 0) {
	boError() << k_funcinfo << "0 textures in map" << endl;
	return false;
 }
 if (d->mTextureGroundType.count() < textureCount()) {
	boError() << k_funcinfo << "have groundType information on " << d->mTextureGroundType.count() << " groundTypes only, need " << textureCount() << endl;
	return false;
 }
 for (unsigned int x = 0; x < width(); x++) {
	for (unsigned int y = 0; y < height(); y++) {
		recalculateCell(x, y);
	}
 }
 return true;
}

void BosonMap::setTextureGroundType(unsigned int texture, int groundType, QRgb miniMapColor, unsigned char amountOfLand, unsigned char amountOfWater)
{
 struct TextureGroundType type;
 type.mGroundType = groundType;
 type.mMiniMapColor = miniMapColor;
 type.mAmountOfLand = amountOfLand;
 type.mAmountOfWater = amountOfWater;
 if (d->mTextureGroundType.count() < texture + 1) {
	d->mTextureGroundType.resize(texture + 1);
 }
 d->mTextureGroundType[texture] = type;
}

unsigned char BosonMap::amountOfLand(unsigned int texture) const
{
 if (texture >= d->mTextureGroundType.count()) {
	boError() << k_funcinfo << "invalid texture " << texture << endl;
	return 0;
 }
 return d->mTextureGroundType[texture].mAmountOfLand;
}

unsigned char BosonMap::amountOfWater(unsigned int texture) const
{
 if (texture >= d->mTextureGroundType.count()) {
	boError() << k_funcinfo << "invalid texture " << texture << endl;
	return 0;
 }
 return d->mTextureGroundType[texture].mAmountOfWater;
}

int BosonMap::groundType(unsigned int texture) const
{
 if (texture >= d->mTextureGroundType.count()) {
	boError() << k_funcinfo << "invalid texture " << texture << endl;
	return 0;
 }
 return d->mTextureGroundType[texture].mGroundType;
}

void BosonMap::recalculateCell(int x, int y)
{
 Cell* c = cell(x, y);
 BO_CHECK_NULL_RET(c);
 BO_CHECK_NULL_RET(mTexMap);
 if (x < 0 || (uint)x >= width()) {
	boError() << k_funcinfo << "invalid x: " << x << endl;
	return;
 }
 if (y < 0 || (uint)y >= height()) {
	boError() << k_funcinfo << "invalid y: " << y << endl;
	return;
 }

 // how much land/water is on the cell. maximum is 255 per corner, i.e. 255*4
 // sum of both must be <= 4*255
 int land = 0;
 int water = 0;

 // every (!) cell has exactly 4 corners. every corner has
 // textureCount() alpha values.
 int* alpha = new int[4 * textureCount()];
 for (unsigned int i = 0; i < textureCount(); i++) {
	// top-left corner
	alpha[4 * i] = (int)texMapAlpha(i, x, y);
	// top-right corner
	alpha[4 * i + 1] = (int)texMapAlpha(i, x + 1, y);
	// bottom-left corner
	alpha[4 * i + 2] = (int)texMapAlpha(i, x, y + 1);
	// bottom-right corener
	alpha[4 * i + 3] = (int)texMapAlpha(i, x + 1, y + 1);
	for (int j = 0; j < 4; j++) {
		int a = alpha[4 * i + j];
		if (a == 0) {
			// no need to do anything.
			continue;
		}
		int l = (int)amountOfLand(i);
		int w = (int)amountOfWater(i);
		if (l != 0) {
			land += (int)(l * a / 255);
		}
		if (w != 0) {
			water += (int)(w * a / 255);
		}
	}
 }
 delete[] alpha;
 alpha = 0;

 if (land + water == 0) {
	boWarning() << k_funcinfo << "land + water == 0 for cell at "
			<< x << "," << y << endl;
	land = 0;
	water = 4 * 255;
 }

 // in the optimal case sum should be 4 * 255 (as of 4 corners).
 // but we can't be 100% sure (bugs, rounding errors).
 // also we have to scale the numbers down (4*255 to 255)
 int sum = land + water;
 unsigned char amountOfLand = (unsigned char)(land * 255 / sum);
 unsigned char amountOfWater = (unsigned char)(water * 255 / sum);

 // amountOfLand + amountOfWater must be 255.
 amountOfLand += (255 - amountOfLand - amountOfWater);
 c->makeCell(amountOfLand, amountOfWater);
}

void BosonMap::slotChangeTexMap(int x, int y, unsigned int texture, unsigned char alpha)
{
 // AB: this is an ugly hack, we shouldn't connect any signals in game mode to
 // this slot at all.
 // but i am sure one day noone will read the comments/docs for this and start
 // to use it in gamemode, so we forbid this here.
 // (we also shouldn't call this on construction, as it is slow when many points
 // are changed)
 if (boGame->gameMode()) {
	boError() << k_funcinfo << "must not be called in gameMode" << endl;
	return;
 }
 if (x < 0 || (uint)x >= width()) {
	boError() << k_funcinfo << "invalid x coordinate: " << x << endl;
	return;
 }
 if (y < 0 || (uint)y >= height()) {
	boError() << k_funcinfo << "invalid y coordinate: " << y << endl;
	return;
 }
 if (texture >= textureCount()) {
	boError() << k_funcinfo << "invalid texture " << texture << " must be < " << textureCount() << endl;
	return;
 }
 mTexMap[texMapArrayPos(texture, x, y)] = alpha;

 // now we update up to 4 cells.
 if (x == 0) { // left border
	if (y == 0) { // top border
		recalculateCell(x, y);
	} else if ((uint)y == height()) { // bottom border
		recalculateCell(x, y - 1);
	} else { // somewhere between top and bottom
		recalculateCell(x, y);
		recalculateCell(x, y - 1);
	}
 } else if ((uint)x == width()) { // right border
	if (y == 0) { // top border
		recalculateCell(x - 1, y);
	} else if ((uint)y == height()) { // bottom border
		recalculateCell(x - 1, y - 1);
	} else { // somewhere between top and bottom
		recalculateCell(x - 1, y);
		recalculateCell(x - 1, y - 1);
	}
 } else if (y == 0) {
	// top border (can't be left or right border)
	recalculateCell(x, y);
	recalculateCell(x - 1, y);
 } else if ((uint)y == height()) {
	// bottom border (can't be left or right border)
	recalculateCell(x, y - 1);
	recalculateCell(x - 1, y - 1);
 } else {
	// no border at all. 4 cells are adjacent.
	recalculateCell(x, y);
	recalculateCell(x, y - 1);
	recalculateCell(x - 1, y);
	recalculateCell(x - 1, y - 1);
 }
}

// AB: maybe this should be in BosonGroundTheme!
QRgb BosonMap::miniMapColor(unsigned int texture) const
{
 if (texture >= d->mTextureGroundType.count()) {
	boError() << k_funcinfo << "invalid texture " << texture << endl;
	return 0;
 }
 return d->mTextureGroundType[texture].mMiniMapColor;
}

int BosonMap::mapFileFormatVersion()
{
 return BOSONMAP_VERSION;
}

