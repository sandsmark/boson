/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bosontiles.h"
#include "bodebug.h"

#include <qtimer.h>
#include <qdatetime.h>
#include <qdatastream.h>
#include <qdom.h>
#include <qimage.h>

#include <kapplication.h>
#include <kglobal.h>
#include <kstandarddirs.h>

class BosonMap::BosonMapPrivate
{
public:
	BosonMapPrivate()
	{
	}
	QString mTilesDir;
};

BosonMap::BosonMap(QObject* parent) : QObject(parent)
{
 init();
}

BosonMap::~BosonMap()
{
 delete mTiles;
 if (mCells) {
	delete[] mCells;
 }
 if (mHeightMap) {
	delete[] mHeightMap;
 }
 delete d;
}

void BosonMap::init()
{
 d = new BosonMapPrivate;
 mCells = 0;
 mHeightMap = 0;
 mTiles = 0;
 setModified(false);
}

bool BosonMap::loadMap(QDomElement& root)
{
 QDomNodeList list;
 list = root.elementsByTagName("MapGeo");
 if (list.count() != 1) {
	boError() << k_funcinfo << "XML error: cannot have tag MapGeo "
			<< list.count() << " times" << endl;
	return false;
 }
 QDomElement geo = list.item(0).toElement();
 if (geo.isNull()) {
	boError() << k_funcinfo << "XML error: geo is not an QDomElement" << endl;
	return false;
 }
 if (!loadMapGeo(geo)) {
	boError() << k_funcinfo << "XML error: failed loading map geo" << endl;
	return false;
 }

 list = root.elementsByTagName("MapCells");
 if (list.count() != 1) {
	boError() << k_funcinfo << "XML error: cannot have tag Map Geo " 
			<< list.count() << " times" << endl;
	return false;
 }
 QDomElement cells = list.item(0).toElement();
 if (geo.isNull()) {
	boError() << k_funcinfo << "XML error: cells is not an QDomElement" << endl;
	return false;
 }
 if (!loadCells(cells)) {
	boError() << k_funcinfo << "XML error: failed loading map geo" << endl;
	return false;
 }

 return true;
}

bool BosonMap::loadMapFromFile(QDataStream& stream)
{
 if (!loadMapGeo(stream)) {
	boError() << k_funcinfo << "Could not load map geo" << endl;
	return false;
 }
 if (!loadCells(stream)) {
	boError() << k_funcinfo << "Could not load map cells" << endl;
	return false;
 }
 return true;
}


bool BosonMap::loadMap(QDataStream& stream)
{
 if (!loadMapGeo(stream)) {
	boError() << k_funcinfo << "Could not load map geo" << endl;
	return false;
 }
 if (!loadCells(stream)) {
	boError() << k_funcinfo << "Could not load map cells" << endl;
	return false;
 }
 if (!loadHeightMap(stream)) {
	boError() << k_funcinfo << "Could not load height map" << endl;
	return false;
 }
 return true;
}

bool BosonMap::loadMapGeo(QDataStream& stream)
{
 Q_INT32 mapWidth;
 Q_INT32 mapHeight;

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

 if (mCells) {
//	boDebug() << "cells created before!! try to delete..." << endl;
	delete[] mCells;
 }
 if (mHeightMap) {
	delete[] mHeightMap;
 }
 mCells = new Cell[width() * height()];
 mHeightMap = new float[(width() + 1) * (height() + 1)];
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
 return loadHeightMapImage(QByteArray());
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
		int groundType = 0;
		unsigned char version = 0;
		if (!loadCell(stream, groundType, version)) {
			return false;
		}
		slotChangeCell(i, j, groundType, version);
	}
 }
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


bool BosonMap::loadMapGeo(QDomElement& node)
{
 if (!node.hasAttribute("Width")) {
	boError() << k_funcinfo << "Map width is mandatory!" << endl;
	return false;
 }
 if (!node.hasAttribute("Height")) {
	boError() << k_funcinfo << "Map height is mandatory!" << endl;
	return false;
 }
 Q_INT32 width = node.attribute("Width").toInt();
 Q_INT32 height = node.attribute("Height").toInt();

// lets use the same function as for loading the binary file:
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 stream << width;
 stream << height;

 QDataStream readStream(buffer, IO_ReadOnly);
 return loadMapGeo(readStream);
}

bool BosonMap::loadCells(QDomElement& node)
{
 QDomNodeList list = node.elementsByTagName("Cell");
 if (list.count() < width() * height()) {
	boError() << k_funcinfo << "XML error: not enough cells" << endl;
	return false;
 }
 if (list.count() != width() * height()) {
	boWarning() << k_funcinfo << "Cell count doesn't match width * height"
			<< endl;
 }

 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 int* groundType = new int[width() * height()];
 unsigned char* version = new unsigned char[width() * height()];
 for (unsigned int i = 0; i < list.count(); i++) {
	QDomElement cell = list.item(i).toElement();
	if (cell.isNull()) {
		boError() << k_funcinfo << "XML error: cell is not an QDomElement" << endl;
	} else {
		int x;
		int y;
		int g;
		unsigned char v;
		if (!loadCell(cell, x, y, g, v)) {
			boError() << k_funcinfo << "XML error: could not load cell" << endl;
			continue;
		}
		groundType[cellArrayPos(x, y)] = g;
		version[cellArrayPos(x, y)] = v;
	}
 }
 for (unsigned int i = 0; i < width(); i++) {
	for (unsigned int j = 0; j < height(); j++) {
		saveCell(stream, groundType[cellArrayPos(i, j)],
				version[cellArrayPos(i, j)]);
	}
 }

 delete[] groundType;
 delete[] version;

 QDataStream readStream(buffer, IO_ReadOnly);
 return loadCells(readStream);
}

bool BosonMap::loadHeightMapImage(const QByteArray& heightMap)
{
 boDebug() << k_funcinfo << endl;
 if (heightMap.size() == 0) {
	// initialize the height map with 0.0
	boDebug() << k_funcinfo << "loading dummy height map" << endl;
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
	boError() << k_funcinfo << "not a grayscale image" << endl;
	return loadHeightMapImage(QByteArray());
 }
 if ((unsigned int)map.height() != height() + 1) {
	boError() << k_funcinfo << "invalid height of heightmap: " <<
			map.height() << " must be: " << height() + 1 << endl;
	return loadHeightMapImage(QByteArray());
 }
 if ((unsigned int)map.width() != width() + 1) {
	boError() << k_funcinfo << "invalid widthof heightmap: "
			<< map.width() << " must be: " << width() + 1 << endl;
	return loadHeightMapImage(QByteArray());
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
 for (unsigned int y = 0; y < width() + 1; y++) {
	// AB: warning: from Qt docs: "If you are accessing 16-bpp image data,
	// you must handle endianness yourself."
	// do we have to care about this? (since we are using 16bpp)
	// AB: we use 32bpp (qt doesnt support 16bpp on X11)
	// AB: hmm accordint to "file" we use 6bpp only... ok thats easier
	// then :)
	int imageX = 0;
	unsigned char* line = map.scanLine(y);
	for (unsigned int x = 0; x < height() + 1; x++, imageX += increment) {
		mHeightMap[y * (width() + 1) + x] = pixelToHeight(line[imageX]);
	}
 }
 return true;
}

bool BosonMap::loadCell(QDomElement& node, int& x, int& y, int& groundType, unsigned char& version)
{
 if (!node.hasAttribute("x")) {
	boError() << k_funcinfo << "XML: attribute x is mandatory!" << endl;
	return false;
 }
 if (!node.hasAttribute("y")) {
	boError() << k_funcinfo << "XML: attribute y is mandatory!" << endl;
	return false;
 }
 if (!node.hasAttribute("GroundType")) {
	boError() << k_funcinfo << "XML: attribute GroundType is mandatory!" << endl;
	return false;
 }
 if (!node.hasAttribute("Version")) {
	boError() << k_funcinfo << "XML: attribute Version is mandatory!" << endl;
	return false;
 }
 x = node.attribute("x").toInt();
 y = node.attribute("y").toInt();
 groundType = node.attribute("GroundType").toInt();
 version = (unsigned char)node.attribute("Version").toInt(); // not nice...

 if (x >= (int)width()) {
	boError() << k_lineinfo << "x >= width" << endl;
	return false;
 }
 if (y >= (int)height()) {
	boError() << k_lineinfo << "y >= height" << endl;
	return false;
 }
 return true;
}

bool BosonMap::saveMapToFile(QDataStream& stream)
{
 if (!saveMapGeo(stream)) {
	boError() << k_funcinfo << "Could not save map geo" << endl;
	return false;
 }
 if (!saveCells(stream)) {
	boError() << k_funcinfo << "Could not save map cells" << endl;
	return false;
 }
 return true;
}

bool BosonMap::saveMap(QDataStream& stream)
{
 if (!saveMapGeo(stream)) {
	boError() << k_funcinfo << "Could not save map geo" << endl;
	return false;
 }
 if (!saveCells(stream)) {
	boError() << k_funcinfo << "Could not save map cells" << endl;
	return false;
 }
 if (!saveHeightMap(stream)) {
	boError() << k_funcinfo << "Could not save height map" << endl;
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
			saveCell(stream, c->groundType(), c->version());
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
	int l = heightToPixel(0.0f);
	image.fill(l);
 } else {
	boDebug() << k_funcinfo << "real height map" << endl;
	// AB: this *might* be correct, but i am not sure about this. (02/11/22)
	for (int y = 0; y < image.height(); y++) {
		uint* p = (uint*)image.scanLine(y);
		for (int x = 0; x < image.width(); x++) {
			float value = mHeightMap[y * (width() + 1) + x];
			int v = heightToPixel(value);
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

bool BosonMap::loadCell(QDataStream& stream, int& groundType, unsigned char& b)
{
 Q_INT32 g;
 Q_INT8 version;

 stream >> g;
 if (!Cell::isValidGround(g)) { 
	return false; 
 }

 stream >> version;
 if (version > 4) {
	boError() << k_funcinfo << "broken map file!" << endl;
	boError() << "invalid cell: version >= 4!" << endl;
	boDebug() << version << endl;
 }
 groundType = g;
 b = version;

 return true;
}

void BosonMap::saveCell(QDataStream& stream, int groundType, unsigned char version)
{
 if (version > 4) {
	boWarning() << k_funcinfo << "Invalid version " << version << endl;
	version = 0;
 }
 stream << (Q_INT32)groundType;
 stream << (Q_INT8)version;
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
 return mHeightMap[y * (width() + 1) + x];
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

void BosonMap::slotChangeCell(int x, int y, int groundType, unsigned char b)
{
//boDebug() << x << " -> " << y << endl;
//boDebug() << width() << " " << height() << endl;
 Cell* c = cell(x, y);
 if (!c) {
	boError() << k_funcinfo << "Invalid cell x=" << x << ",y=" << y << endl;
	return;
 }
 if ((Cell::GroundType)groundType == Cell::GroundUnknown) {
	boWarning() << k_funcinfo << "Unknown ground?! x=" << x << ",y=" << y
			<< endl;
 }
 c->makeCell(groundType, b);
}

void BosonMap::loadTiles(const QString& tiles, bool withtimer)
{
 delete mTiles;
 mTiles = new BosonTiles(this);
 connect(mTiles, SIGNAL(signalTilesLoading(int)),
		this, SIGNAL(signalTilesLoading(int)));
 connect(mTiles, SIGNAL(signalTilesLoaded()),
		this, SIGNAL(signalTilesLoaded()));
 QString dir = KGlobal::dirs()->findResourceDir("data", QString("boson/themes/grounds/%1/index.ground").arg(tiles)) + QString("boson/themes/grounds/%1").arg(tiles);
 if (dir.isNull()) {
	boError() << k_funcinfo << "Cannot find tileset " << tiles << endl;
	return;
 }
 d->mTilesDir = dir;
 if (withtimer) {
	QTimer::singleShot(0, this, SLOT(slotLoadTiles()));
 } else {
	slotLoadTiles();
 }
}

void BosonMap::slotLoadTiles()
{
 boDebug() << k_funcinfo << endl;
 QTime time;
 time.start();
 mTiles->loadTiles(d->mTilesDir);
 boDebug() << k_funcinfo << "loading took: " << time.elapsed() << endl;

 emit signalTileSetChanged(mTiles);
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

void BosonMap::fill(int ground)
{
 if (!mCells) {
	boError() << k_funcinfo << "NULL cells" << endl;
	return;
 }
 for (unsigned int i = 0; i < width(); i++) {
	for (unsigned int j = 0; j < height(); j++) {
		int version = kapp->random() % 4; // note: this is a bad thing for network (although version doesn't influence game logic)
		slotChangeCell(i, j, ground, version);
	}
 }
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

