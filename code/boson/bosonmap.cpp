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
#include "bosondata.h"

#include <qtimer.h>
#include <qdatetime.h>
#include <qdatastream.h>
#include <qimage.h>
#include <qvaluevector.h>

#include <kapplication.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#define BOSONMAP_VERSION 0x01 // current version

float BoHeightMap::pixelToHeight(int p)
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

int BoHeightMap::heightToPixel(float height)
{
 return (int)(height * 10 + 105);
}

bool BoHeightMap::save(QDataStream& stream)
{
 if (width() == 0) {
	boError() << k_funcinfo << "width()==0" << endl;
	return false;
 }
 if (height() == 0) {
	boError() << k_funcinfo << "height()==0" << endl;
	return false;
 }
 if (!mHeightMap) {
	BO_NULL_ERROR(mHeightMap);
	return false;
 }
 boDebug() << k_funcinfo << "saving real heightmap to network stream" << endl;
 for (unsigned int x = 0; x < width(); x++) {
	for (unsigned int y = 0; y < height(); y++) {
		float h = heightAt(x, y);
		stream << (float)h;
	}
 }
 return true;
}

bool BoHeightMap::load(QDataStream& stream)
{
 boDebug() << k_funcinfo << endl;
 if (!mHeightMap) {
	BO_NULL_ERROR(mHeightMap);
	return false;
 }
 for (unsigned int x = 0; x < width(); x++) {
	for (unsigned int y = 0; y < height(); y++) {
		float h;
		stream >> h;
		setHeightAt(x, y, h);
	}
 }
 return true;
}



class BosonMap::BosonMapPrivate
{
public:
	BosonMapPrivate()
	{
	}
};

BosonMap::BosonMap(QObject* parent) : QObject(parent)
{
 init();
}

BosonMap::~BosonMap()
{
 delete[] mCells;
 delete mHeightMap;
 delete mTexMap;
 delete d;
}

void BosonMap::init()
{
 d = new BosonMapPrivate;
 mCells = 0;
 mHeightMap = 0;
 mGroundTheme = 0;
 mTexMap = 0;
 mMapWidth = 0;
 mMapHeight = 0;
 setModified(false);
}

bool BosonMap::createNewMap(unsigned int width, unsigned int height, BosonGroundTheme* theme)
{
 if (!isValidMapGeo(width, height)) {
	boError() << k_funcinfo << width << " * " << height << " is no valid map geo!" << endl;
	return false;
 }
 if (!theme) {
	BO_NULL_ERROR(theme);
	return false;
 }
 mMapWidth = width;
 mMapHeight = height;
 mGroundTheme = theme;

 mHeightMap = new BoHeightMap(width + 1, height + 1);
 mTexMap = new BoTexMap(theme->textureCount(), width + 1, height + 1);
 bool ret = generateCellsFromTexMap();
 if (!ret) {
	boError() << k_funcinfo << "unable to generate cells from texmap" << endl;
	// don't return yet! (must delete the arrays)
 }

 // the map is saved completely first, and then loaded from the stream.
 // we do this to catch errors early.
 QByteArray buffer;
 QDataStream stream(buffer, IO_WriteOnly);
 if (ret) {
	ret = saveCompleteMap(stream);
 }

 delete mHeightMap;
 mHeightMap = 0;
 delete mTexMap;
 mTexMap = 0;
 delete[] mCells;
 mCells = 0;

 if (!ret) {
	boError() << k_funcinfo << "map could not be saved to stream" << endl;
	mMapWidth = 0;
	mMapHeight = 0;
	mGroundTheme = 0;
	return false;
 }

 QDataStream readStream(buffer, IO_ReadOnly);
 ret = loadCompleteMap(readStream);
 if (!ret) {
	boError() << k_funcinfo << "unable to load previously saved map from stream! looks like an internal error" << endl;
	return ret;
 }

 return ret;
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
 if (!loadGroundTheme(stream)) {
	boError() << k_funcinfo << "Could not load the ground theme" << endl;
	return false;
 }
 if (!stream.atEnd()) {
	boWarning() << k_funcinfo << "stream is not at end after groundTheme!" << endl;
 }
 return true;
}


bool BosonMap::loadCompleteMap(QDataStream& stream)
{
 boDebug() << k_funcinfo << endl;
 QByteArray mapBuffer;
 stream >> mapBuffer;
 if (!loadMapFromFile(mapBuffer)) {
	boError() << k_funcinfo << "Could not load basic map" << endl;
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
 boDebug() << k_funcinfo << endl;
 Q_UINT32 mapWidth;
 Q_UINT32 mapHeight;

 stream >> mapWidth;
 stream >> mapHeight;

 if (!isValidMapGeo(mapWidth, mapHeight)) {
	boError()<< k_funcinfo << "map geo is not valid: " << mapWidth << "x" << mapHeight << endl;
	return false;
 }

// map is ok - lets apply
 mMapWidth = mapWidth; // horizontal cell count
 mMapHeight = mapHeight; // vertical cell count

 delete[] mCells;
 mCells = 0;
 delete mHeightMap;
 mHeightMap = 0;
 delete mTexMap;
 mTexMap = 0;

 return true;
}

bool BosonMap::loadGroundTheme(QDataStream& stream)
{
 QString id;
 stream >> id;
 mGroundTheme = (BosonGroundTheme*)BosonData::bosonData()->groundTheme(id);
 if (!mGroundTheme) {
	boError() << k_funcinfo << "Cannot find groundTheme with id=" << id << endl;
	return false;
 }
 emit signalGroundThemeChanged(mGroundTheme);
 return true;
}

bool BosonMap::loadCells(QDataStream& stream)
{
 createCells();
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
	boWarning() << k_funcinfo << "image is of size "
			<< image.width()  << "*" << image.height()
			<< " we need: " << width() + 1 << "*" << height() + 1 << endl;
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

bool BosonMap::loadHeightMapImage(const QByteArray& heightMapBuffer)
{
 boDebug() << k_funcinfo << endl;
 if (!isValidMapGeo(width(), height())) {
	boError() << k_funcinfo << "invalid map geo" << endl;
	return false;
 }
 if (heightMapBuffer.size() == 0) {
	// initialize the height map with 0.0
//	boDebug() << k_funcinfo << "loading dummy height map" << endl;
	QByteArray buffer;
	QDataStream writeStream(buffer, IO_WriteOnly);
	BoHeightMap heightMap(width() + 1, height() + 1);
	bool ret = heightMap.save(writeStream);
	if (ret) {
		QDataStream readStream(buffer, IO_ReadOnly);
		ret = loadHeightMap(readStream);
		if (!ret) {
			boError() << k_funcinfo << "unable to load heightmap from stream" << endl;
		}
	} else {
		boError() << k_funcinfo << "unable to create heightMap stream" << endl;
	}
	return ret;
 }
 boDebug() << k_funcinfo << "loading real height map" << endl;
 QImage map(heightMapBuffer);
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


 BoHeightMap heightMap(width() + 1, height() + 1);
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
		heightMap.setHeightAt(x, y, BoHeightMap::pixelToHeight(line[imageX]));
	}
 }
 QByteArray buffer;
 QDataStream writeStream(buffer, IO_WriteOnly);
 bool ret = heightMap.save(writeStream);
 if (ret) {
	QDataStream readStream(buffer, IO_ReadOnly);
	ret = loadHeightMap(readStream);
	if (!ret) {
		boError() << k_funcinfo << "unable to load heightmap from stream" << endl;
	}
 } else {
	boError() << k_funcinfo << "unable to create heightMap stream" << endl;
 }
 return ret;
}

bool BosonMap::loadHeightMap(QDataStream& stream)
{
 if (mHeightMap) {
	boWarning() << k_funcinfo << "heightmap already present - deleting" << endl;
	delete mHeightMap;
	mHeightMap = 0;
 }
 mHeightMap = new BoHeightMap(width() + 1, height() + 1);
 boDebug() << k_funcinfo << "loading height map from network stream" << endl;
 return mHeightMap->load(stream);
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
	delete mTexMap;
	mTexMap = 0;
 }
 if (width() * height() <= 0) {
	boError() << k_funcinfo << "width=" << width() << " height=" << height() << endl;
	return false;
 }
 if (!groundTheme()) {
	BO_NULL_ERROR(groundTheme());
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
 if (textures == 0) {
	boError() << k_funcinfo << "0 textures is not possible" << endl;
	return false;
 }
 if (textures > groundTheme()->textureCount()) {
	boError() << k_funcinfo << "textureCount from map stream must not be greater than texture count from groundTheme!"
			<< " textureCount=" << textures
			<< " theme textureCount=" << groundTheme()->textureCount()
			<< endl;
	return false;
 }
 if (stream.atEnd()) {
	boError() << k_funcinfo << "stream at end" << endl;
	return false;
 }
 // we allocate memory for all possible textures, even if this map doesn't use
 // them all. we also have to initialize all textures.
 // one day we may want to change this to save a few kb of memory on some maps
 // (none yet)
 mTexMap = new BoTexMap(groundTheme()->textureCount(), width() + 1, height() + 1);

 // now load all textures that are actually used here.
 for (unsigned int i = 0; i < textures; i++) {
	for (unsigned int x = 0; x < width() + 1; x++) {
		for (unsigned int y = 0; y < height() + 1; y++) {
			Q_UINT8 c;
			stream >> c;
			setTexMapAlpha(i, x, y, c);
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
 if (!groundTheme()) {
	BO_NULL_ERROR(groundTheme());
	return false;
 }
 if (groundTheme()->textureCount() > 100) {
	// this *cant* be true (would be > 100*500*500 bytes on a 500x500 map)
	boError() << k_funcinfo << "texture count > 100: " << groundTheme()->textureCount() << " - won't save anything." << endl;
	return false;
 }
 if (groundTheme()->textureCount() < 1) {
	boError() << k_funcinfo << "need at least one texture!" << endl;
	return false;
 }
 stream << BOSONMAP_TEXMAP_MAGIC_COOKIE;
 stream << (Q_UINT32)BOSONMAP_VERSION;

 // there is a groundTheme identifier in the "map" file, but an invalid texture
 // count would suck greatly (->crash). so we stream it.
 stream << (Q_UINT32)groundTheme()->textureCount();

 // now the actual texmap for this texture
 for (unsigned int i = 0; i < groundTheme()->textureCount(); i++) {
	for (unsigned int x = 0; x < width() + 1; x++) {
		for (unsigned int y = 0; y < height() + 1; y++) {
			stream << (Q_UINT8)texMapAlpha(i, x, y);
		}
	}
 }
 return true;
}

bool BosonMap::saveMapToFile(QDataStream& stream)
{
 stream << BOSONMAP_MAP_MAGIC_COOKIE;
 stream << (Q_UINT32)BOSONMAP_VERSION;
 if (!saveMapGeo(stream)) {
	boError() << k_funcinfo << "Could not save map geo" << endl;
	return false;
 }
 if (!saveGroundTheme(stream)) {
	boError() << k_funcinfo << "Could not save groundTheme" << endl;
	return false;
 }
 return true;
}

bool BosonMap::saveCompleteMap(QDataStream& stream)
{
 // AB: we may have a problem here - this stream is meant to be sent through
 // network, but it is very big! (sometimes several MB)
 // we should compress it!

 QByteArray buffer;
 QDataStream mapStream(buffer, IO_WriteOnly);
 if (!saveMapToFile(mapStream)) { // AB: bad name. we don't actually save to file - it is just the "map" file in the .bpf file that is created in that function
	boError() << k_funcinfo << "Could not save basic map" << endl;
	return false;
 }
 stream << buffer;

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
 if (!isValidMapGeo(width(), height())) {
	boError() << k_funcinfo << "Map geo is not valid" << endl;
	return false;
 }
// boDebug() << k_funcinfo << endl;
 stream << (Q_INT32)width();
 stream << (Q_INT32)height();
 return true;
}

bool BosonMap::saveGroundTheme(QDataStream& stream)
{
 if (!groundTheme()) {
	BO_NULL_ERROR(groundTheme());
	return false;
 }
 if (groundTheme()->identifier().isEmpty()) {
	// we might use an empty identifier to store the complete theme into the
	// stream (including textures). Advantage: someone (3rd party) who
	// designs a map doesn't need to install his custom groundTheme, but can
	// install a single .bpf file only.
	boError() << k_funcinfo << "empty groundTheme identifier" << endl;
	return false;
 }
 stream << groundTheme()->identifier();
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
	BoHeightMap heightMap(width() + 1, height() + 1);
	return heightMap.save(stream);
 }
 return mHeightMap->save(stream);
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
	int l = BoHeightMap::heightToPixel(0.0f);
	image.fill(l);
 } else {
	boDebug() << k_funcinfo << "real height map" << endl;
	// AB: this *might* be correct, but i am not sure about this. (02/11/22)
	for (int y = 0; y < image.height(); y++) {
		uint* p = (uint*)image.scanLine(y);
		for (int x = 0; x < image.width(); x++) {
			float value = mHeightMap->heightAt(x, y);
			int v = BoHeightMap::heightToPixel(value);
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

void BosonMap::createCells()
{
 if (mCells) {
	boWarning() << k_funcinfo << "cells have already been created! deleting now" << endl;
	delete[] mCells;
	mCells = 0;
 }
 mCells = new Cell[width() * height()];
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
}

bool BosonMap::isValidMapGeo(unsigned int width, unsigned int height)
{
 // check 'realityness'
 if (width < 10) {
	boError() << k_funcinfo << "width < 10" << endl;
	return false;
 }
 if (width > MAX_MAP_WIDTH) {
	boError() << k_funcinfo << "mapWidth > " << MAX_MAP_WIDTH << endl;
	return false;
 }
 if (height < 10) {
	boError() << k_funcinfo << "height < 10" << endl;
	return false;
 }
 if (height > MAX_MAP_HEIGHT) {
	boError() << k_funcinfo << "mapHeight > " << MAX_MAP_HEIGHT << endl;
	return false;
 }
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
 return mHeightMap->heightAt(x, y);
}

void BosonMap::setHeightAtCorner(int x, int y, float h)
{
 BO_CHECK_NULL_RET(mHeightMap);
 mHeightMap->setHeightAt(x, y, h);
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
 BO_CHECK_NULL_RET0(groundTheme());
 return groundTheme()->textures();
}


void BosonMap::fill(unsigned int texture)
{
 if (!mCells) {
	boError() << k_funcinfo << "NULL cells" << endl;
	return;
 }
 BO_CHECK_NULL_RET(mTexMap);
 BO_CHECK_NULL_RET(groundTheme());
 if (texture >= groundTheme()->textureCount()) {
	boError() << k_funcinfo << "invalid texture " << texture << endl;
	return;
 }

 mTexMap->fill(texture);
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
 if (!groundTheme()) {
	BO_NULL_ERROR(groundTheme());
 }
 if ((unsigned int)texturesPerComponent * (useAlpha ? 4 : 3) > groundTheme()->textureCount()) {
	boWarning() << k_funcinfo << "this map doesn't have " 
			<< texturesPerComponent * (useAlpha ? 4 : 3)
			<< " textures. reset to " << groundTheme()->textureCount() << " textures, i.e. "
			<< groundTheme()->textureCount() / (useAlpha ? 4 : 3) 
			<< " textures per component" << endl;
	texturesPerComponent = groundTheme()->textureCount() / (useAlpha ? 4 : 3);
 }

 if (texturesPerComponent != 1) {
	boError() << k_funcinfo << "currently we are supporting only 1 texture per component!" << endl;
	return false;
 }
 if (!mTexMap) {
	BO_NULL_ERROR(mTexMap);
	return false;
 }

boDebug() << k_funcinfo << endl;
 for (unsigned int y = 0; y < height() + 1; y++) {
	QRgb* line = (QRgb*)img->scanLine(y);
	for (unsigned int x = 0; x < width() + 1; x++) {
		QRgb pixel = line[x];
//		for (int i = 0; i < texturesPerComponent; i++) {
			setTexMapAlpha(0, x, y, qRed(pixel));
			setTexMapAlpha(1, x, y, qGreen(pixel));
			setTexMapAlpha(2, x, y, qBlue(pixel));
//		}
	}
 }
 return true;
}

bool BosonMap::generateCellsFromTexMap()
{
 boDebug() << k_funcinfo << endl;
 if (!groundTheme()) {
	BO_NULL_ERROR(groundTheme());
	return false;
 }
 if (mCells) {
	boWarning() << k_funcinfo << "cells already constructed!" << endl;
	delete[] mCells;
	mCells = 0;
 }
 if (width() * height() <= 0) {
	boError() << k_funcinfo << "invalid map size - width=" << width() << " height=" << height() << endl;
	return false;
 }
 createCells();
 if (groundTheme()->textureCount() == 0) {
	boError() << k_funcinfo << "0 textures in map" << endl;
	return false;
 }
 for (unsigned int x = 0; x < width(); x++) {
	for (unsigned int y = 0; y < height(); y++) {
		recalculateCell(x, y);
	}
 }
 return true;
}

QRgb BosonMap::miniMapColor(unsigned int texture) const
{
 if (!groundTheme()) {
	boWarning() << k_funcinfo << "NULL groundTheme" << endl;
	return 0;
 }
 return groundTheme()->miniMapColor(texture);
}

void BosonMap::recalculateCell(int x, int y)
{
 Cell* c = cell(x, y);
 BO_CHECK_NULL_RET(c);
 BO_CHECK_NULL_RET(mTexMap);
 BO_CHECK_NULL_RET(groundTheme());
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
 int* alpha = new int[4 * groundTheme()->textureCount()];
 for (unsigned int i = 0; i < groundTheme()->textureCount(); i++) {
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
		int l = (int)groundTheme()->amountOfLand(i);
		int w = (int)groundTheme()->amountOfWater(i);
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
 slotChangeCell(x, y, amountOfLand, amountOfWater);
}

void BosonMap::slotChangeTexMap(int x, int y, unsigned int texCount, unsigned int* textures, unsigned char* alpha)
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
 BO_CHECK_NULL_RET(groundTheme());
 if (x < 0 || (uint)x >= width()) {
	boError() << k_funcinfo << "invalid x coordinate: " << x << endl;
	return;
 }
 if (y < 0 || (uint)y >= height()) {
	boError() << k_funcinfo << "invalid y coordinate: " << y << endl;
	return;
 }
 if (texCount > groundTheme()->textureCount()) {
	boError() << k_funcinfo << "invalid textureCount " << texCount << " must be <= " << groundTheme()->textureCount() << endl;
	return;
 }
 BO_CHECK_NULL_RET(alpha);
 BO_CHECK_NULL_RET(textures);
 for (unsigned int i = 0; i < texCount; i++) {
	unsigned int texture = textures[i];
	if (texture >= groundTheme()->textureCount()) {
		boError() << k_funcinfo << "invalid texture " << texture << endl;
		continue;
	}
	setTexMapAlpha(texture, x, y, alpha[i]);
 }

 // now we update up to 4 cells.
 int cellsX[10]; // we use 10, so that we don't crash for bugs
 int cellsY[10];
 unsigned int count = 0;
 if (x == 0) { // left border
	if (y == 0) { // top border
		cellsX[count] = x;
		cellsY[count] = y;
		count++;
	} else if ((uint)y == height()) { // bottom border
		cellsX[count] = x;
		cellsY[count] = y - 1;
		count++;
	} else { // somewhere between top and bottom
		cellsX[count] = x;
		cellsY[count] = y;
		count++;
		cellsX[count] = x;
		cellsY[count] = y - 1;
		count++;
	}
 } else if ((uint)x == width()) { // right border
	if (y == 0) { // top border
		cellsX[count] = x - 1;
		cellsY[count] = y;
		count++;
	} else if ((uint)y == height()) { // bottom border
		cellsX[count] = x - 1;
		cellsY[count] = y - 1;
		count++;
		recalculateCell(x - 1, y - 1);
	} else { // somewhere between top and bottom
		cellsX[count] = x - 1;
		cellsY[count] = y;
		count++;
		cellsX[count] = x - 1;
		cellsY[count] = y - 1;
		count++;
	}
 } else if (y == 0) {
	// top border (can't be left or right border)
	cellsX[count] = x;
	cellsY[count] = y;
	count++;
	cellsX[count] = x - 1;
	cellsY[count] = y;
	count++;
 } else if ((uint)y == height()) {
	// bottom border (can't be left or right border)
	cellsX[count] = x;
	cellsY[count] = y - 1;
	count++;
	cellsX[count] = x - 1;
	cellsY[count] = y - 1;
	count++;
 } else {
	// no border at all. 4 cells are adjacent.
	cellsX[count] = x;
	cellsY[count] = y;
	count++;
	cellsX[count] = x;
	cellsY[count] = y - 1;
	count++;
	cellsX[count] = x - 1;
	cellsY[count] = y;
	count++;
	cellsX[count] = x - 1;
	cellsY[count] = y - 1;
	count++;
 }
 if (count > 4) {
	boError() << k_funcinfo << "a cell cannot have more than 4 corners! count=" << count << endl;
	count = 4;
 }
 for (unsigned int i = 0; i < count; i++) {
	recalculateCell(cellsX[i], cellsY[i]);

	// update minimap
	// we may want to group these cells into a single array to save some
	// speed once the editor is able to modify several cells at once.
	// currently it isn't necessary.
	emit signalCellChanged(cellsX[i], cellsY[i]);
 }
}



int BosonMap::mapFileFormatVersion()
{
 return BOSONMAP_VERSION;
}

