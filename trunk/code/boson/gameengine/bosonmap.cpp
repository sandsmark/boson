/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2006 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2006 Rivo Laks (rivolaks@hot.ee)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bosonmap.h"
#include "bosonmap.moc"

#include "defines.h"
#include "../bomemory/bodummymemory.h"
#include "cell.h"
#include "bosongroundtheme.h"
#include "boson.h" // for an ugly boGame->gameMode() hack!
#include "bodebug.h"
#include "bosondata.h"
#include "bowater.h"
#include "bogroundquadtreenode.h"

#include <qdict.h>
#include <qdatastream.h>
#include <qimage.h>
#include <qdom.h>
#include <qbuffer.h>

#define BOSONMAP_VERSION 0x01 // current version


float BoHeightMap::pixelToHeight(int p)
{
 // we have 255 different values available. we use 0.125 steps, i.e. if
 // the pixel is increased by 1 the height is increased by 0.125.
 // values below 105 cause a negative height, values above 105 a positive
 // height.
 // this means we have a height range from -105/8 to 150/8 aka from -13.125 to
 // 18.75
// boDebug() << k_funcinfo << p << "->" << ((float)(p-105))/10 << endl;
 return ((float)(p - 105)) / 8.0f;
}

int BoHeightMap::heightToPixel(float height)
{
 return ((int)(height * 8.0f)) + 105;
}

bool BoHeightMap::save(QDataStream& stream) const
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


bool BoTexMap::save(QDataStream& stream) const
{
 if (!mTexMap) {
	BO_NULL_ERROR(mTexMap);
	return false;
 }
 if (width() == 0) {
	boError() << k_funcinfo << "width()==0" << endl;
	return false;
 }
 if (height() == 0) {
	boError() << k_funcinfo << "height()==0" << endl;
	return false;
 }
 if (mTextureCount > 100) {
	// this *cant* be true (would be > 100*500*500 bytes on a 500x500 map)
	boError() << k_funcinfo << "texture count > 100: " << mTextureCount << " - won't save anything." << endl;
	return false;
 }
 if (mTextureCount < 1) {
	boError() << k_funcinfo << "need at least one texture!" << endl;
	return false;
 }
 stream << BOSONMAP_TEXMAP_MAGIC_COOKIE;
 stream << (Q_UINT32)BOSONMAP_VERSION;

 // there is a groundTheme identifier in the "map" file, but an invalid texture
 // count would suck greatly (->crash). so we stream it.
 stream << (Q_UINT32)mTextureCount;

 // now the actual texmap for this texture
 for (unsigned int i = 0; i < mTextureCount; i++) {
	for (unsigned int x = 0; x < width(); x++) {
		for (unsigned int y = 0; y < height(); y++) {
			stream << (Q_UINT8)texMapAlpha(i, x, y);
		}
	}
 }
 return true;
}

bool BoTexMap::load(QDataStream& stream)
{
 if (stream.atEnd()) {
	boError() << k_funcinfo << "empty stream" << endl;
	return false;
 }
 if (!mTexMap) {
	BO_NULL_ERROR(mTexMap);
	return false;
 }
 if (width() * height() == 0) {
	boError() << k_funcinfo << "width=" << width() << " height=" << height() << endl;
	return false;
 }
 if (mTextureCount == 0) {
	boError() << k_funcinfo << "0 textures in array??" << endl;
	return false;
 }
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
 if (textures > mTextureCount) {
	boError() << k_funcinfo << "textureCount from map stream must not be greater than texture count from BoTexMap!"
			<< " textureCount=" << textures
			<< " BoTexMap textureCount=" << mTextureCount
			<< endl;
	return false;
 }
 if (stream.atEnd()) {
	boError() << k_funcinfo << "stream at end" << endl;
	return false;
 }
 if (textures != mTextureCount) {
	boWarning() << k_funcinfo << "only " << textures << " textures in stream - expected " << mTextureCount << endl;
	// still possible - the rest will be 0.
 }

 // now load all textures that are actually used here.
 for (unsigned int i = 0; i < textures; i++) {
	for (unsigned int x = 0; x < width(); x++) {
		for (unsigned int y = 0; y < height(); y++) {
			Q_UINT8 c;
			stream >> c;
			setTexMapAlpha(i, x, y, c);
		}
	}
 }

 // reset all textures that are not in the stream to 0
 for (unsigned int i = textures; i < mTextureCount; i++) {
	initialize(i, 0);
 }
 return true;
}

bool BoTexMap::importTexMap(const QImage* img)
{
 // AB: we could use more than 1 texture per component - e.g. use 0-127 for
 // texture 1 and 128-255 for texture 2.
 // but since we support importing multiple texmaps i think that would be too
 // unhandy for the designers anyway.
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
 if (textureCount() < 3) {
	boError() << k_funcinfo << "cannot import an image into an array with less than 3 textures. textureCount() == " << textureCount() << endl;
	return false;
 }
 bool useAlpha = img->hasAlphaBuffer();
 if (useAlpha && textureCount() < 4) {
	boWarning() << k_funcinfo << "won't import alpha buffer" << endl;
	useAlpha = false;
 }
 if ((unsigned int)img->width() != width()) {
	boError() << k_funcinfo << "image width must be "
			<< width() << ", is: " << img->width() << endl;
	return false;
 }
 if ((unsigned int)img->height() != height()) {
	boError() << k_funcinfo << "image height must be "
			<< height() << ", is: " << img->height() << endl;
	return false;
 }
 if (!mTexMap) {
	BO_NULL_ERROR(mTexMap);
	return false;
 }


 for (unsigned int y = 0; y < height(); y++) {
	for (unsigned int x = 0; x < width(); x++) {
		QRgb pixel = img->pixel(x, y);
		setTexMapAlpha(0, x, y, qRed(pixel));
		setTexMapAlpha(1, x, y, qGreen(pixel));
		setTexMapAlpha(2, x, y, qBlue(pixel));
	}
 }
 if (useAlpha) {
	for (unsigned int y = 0; y < height(); y++) {
		for (unsigned int x = 0; x < width(); x++) {
			QRgb pixel = img->pixel(x, y);
			setTexMapAlpha(3, x, y, qAlpha(pixel));
		}
	}
 }

 return true;
}

bool BoTexMap::copyTexture(unsigned int dstTexture, const BoTexMap* src, unsigned int srcTexture)
{
 if (!src) {
	BO_NULL_ERROR(src);
	return false;
 }
 if (src->width() != width() || src->height() != height()) {
	boError() << k_funcinfo << "src and dst must be of the same size!" << endl;
	return false;
 }
 if (srcTexture >= src->textureCount()) {
	boError() << k_funcinfo << "src does not have a texture " << srcTexture << endl;
	return false;
 }
 if (dstTexture >= textureCount()) {
	boError() << k_funcinfo << "we don't have a texture " << dstTexture << endl;
	return false;
 }
 for (unsigned int x = 0; x < width(); x++) {
	for (unsigned int y = 0; y < height(); y++) {
		setTexMapAlpha(dstTexture, x, y, src->texMapAlpha(srcTexture, x, y));
	}
 }
 return true;
}


BoColorMap::BoColorMap(unsigned int width, unsigned int height)
{
 mWidth = width;
 mHeight = height;

 // Load initial black texture
 int dataSize = mWidth * mHeight * 3;
 mData = new unsigned char[dataSize];
 for (int i = 0; i < dataSize; i++) {
	mData[i] = 0;
 }
 mDirtyRect = QRect(0, 0, mWidth, mHeight);
}

BoColorMap::~BoColorMap()
{
 delete[] mData;
}

void BoColorMap::update(unsigned char* data)
{
 updateRect(0, 0, mWidth, mHeight, data);
}

void BoColorMap::updateRect(int _x, int _y, unsigned int w, unsigned int h, unsigned char* data)
{
 QRect r(_x, _y, w, h);
 int pos = 0;
 for (int y = r.y(); y < r.y() + r.height(); y++) {
	for (int x = r.x(); x < r.x() + r.width(); x++) {
		int index = (y * mWidth + x) * 3;
		mData[index + 0] = data[pos + 0];
		mData[index + 1] = data[pos + 1];
		mData[index + 2] = data[pos + 2];
		pos += 3;
	}
 }
 mDirtyRect = mDirtyRect.unite(r);
 mDirtyRect = mDirtyRect.intersect(QRect(0, 0, mWidth, mHeight));
}


class BosonMap::BosonMapPrivate
{
public:
	BosonMapPrivate()
	{
		mWaterManager = 0;
		mQuadTreeCollection = 0;
	}

	QDict<BoColorMap> mColorMaps;
	BoWaterManager* mWaterManager;
	BoGroundQuadTreeCollection* mQuadTreeCollection;
};

BosonMap::BosonMap(QObject* parent) : QObject(parent)
{
 init();
}

BosonMap::~BosonMap()
{
 d->mColorMaps.setAutoDelete(true);
 d->mColorMaps.clear();
 delete[] mCells;
 delete mNormalMap;
 delete mHeightMap;
 delete mTexMap;
 delete d->mWaterManager;
 delete d->mQuadTreeCollection;
 delete d;
}

void BosonMap::init()
{
 d = new BosonMapPrivate;
 mCells = 0;
 mActiveColorMap = 0;
 mHeightMap = 0;
 mNormalMap = 0;
 mGroundTheme = 0;
 mTexMap = 0;
 mMapWidth = 0;
 mMapHeight = 0;
 d->mWaterManager = new BoWaterManager();
 d->mQuadTreeCollection = new BoGroundQuadTreeCollection(this);
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
 if (!applyGroundTheme(theme->identifier())) {
	boError() << k_funcinfo << "invalid groundtheme " << theme->identifier() << endl;
	return false;
 }

 mHeightMap = new BoHeightMap(width + 1, height + 1);
 mTexMap = new BoTexMap(theme->groundTypeCount(), width + 1, height + 1);
 mTexMap->fill(0);
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

 d->mWaterManager->setMap(this);

 QDataStream readStream(buffer, IO_ReadOnly);
 ret = loadCompleteMap(readStream);
 if (!ret) {
	boError() << k_funcinfo << "unable to load previously saved map from stream! looks like an internal error" << endl;
	return ret;
 }

 // check whether map was loaded successfully
 if (!mGroundTheme || !mCells || !mHeightMap || !mTexMap) {
	boError() << k_funcinfo << "map was not loaded correctly" << endl;
	return false;
 }
 if (mGroundTheme != theme) {
	boError() << k_funcinfo << "incorrect theme loaded: " << mGroundTheme << " != " << theme << endl;
	return false;
 }
 if (this->width() != width) {
	boError() << k_funcinfo << "invalid width loaded" << endl;
	return false;
 }
 if (this->height() != height) {
	boError() << k_funcinfo << "invalid height loaded" << endl;
	return false;
 }
 if (mTexMap->textureCount() != mGroundTheme->groundTypeCount()) {
	boError() << k_funcinfo << "texmap has invalid texture count for this groundtheme!" << endl;
	return false;
 }

 return ret;
}

bool BosonMap::loadMapFromFile(const QByteArray& mapXML)
{
 boDebug(270) << k_funcinfo << endl;
 QDomDocument doc(QString::fromLatin1("BosonMap"));
 QString errorMsg;
 int line, column;
 if (!doc.setContent(QString(mapXML), &errorMsg, &line, &column)) {
	boError(270) << k_funcinfo << "unable to load from mapXML (error in line " << line << ", column " << column << ", msg=" << errorMsg << ")" << endl;
	return false;
 }
 QDomElement root = doc.documentElement();
 if (root.isNull()) {
	boError(270) << k_funcinfo << "no root element" << endl;
	return false;
 }
 if (!root.hasAttribute("Version")) {
	boError(270) << k_funcinfo << "missing attribute: Version" << endl;
	return false;
 }
 int version = root.attribute("Version").toInt();
 if (version != BOSONMAP_VERSION) {
	boError(270) << k_funcinfo << "invalid version " << version << " - need: " << BOSONMAP_VERSION << endl;
	return false;
 }
 if (!root.hasAttribute(QString::fromLatin1("GroundTheme"))) {
	boError(270) << k_funcinfo << "no GroundTheme attribute in root element" << endl;
	return false;
 }
 QDomElement geometry = root.namedItem(QString::fromLatin1("Geometry")).toElement();
 if (geometry.isNull()) {
	boError(270) << k_funcinfo << "no geometry element" << endl;
	return false;
 }
 if (!geometry.hasAttribute(QString::fromLatin1("Width"))) {
	boError(270) << k_funcinfo << "no Width attribute in Geometry element" << endl;
	return false;
 }
 if (!geometry.hasAttribute(QString::fromLatin1("Height"))) {
	boError(270) << k_funcinfo << "no Height attribute in Geometry element" << endl;
	return false;
 }
 bool ok = false;
 QString groundTheme = root.attribute(QString::fromLatin1("GroundTheme"));
 int width = geometry.attribute(QString::fromLatin1("Width")).toInt(&ok);
 if (!ok || width < 0) {
	boError(270) << k_funcinfo << "width is not a valid number" << endl;
	return false;
 }
 int height = geometry.attribute(QString::fromLatin1("Height")).toInt(&ok);
 if (!ok || height < 0) {
	boError(270) << k_funcinfo << "height is not a valid number" << endl;
	return false;
 }

 if (!loadMapGeo((unsigned int)width, (unsigned int)height)) {
	boError(270) << k_funcinfo << "Could not load map geo" << endl;
	return false;
 }
 if (!applyGroundTheme(groundTheme)) {
	boError(270) << k_funcinfo << "Could not apply the ground theme " << groundTheme << endl;
	return false;
 }
 return true;
}

bool BosonMap::loadWaterFromFile(const QByteArray& waterXML)
{
 boDebug(270) << k_funcinfo << endl;
 QDomDocument doc(QString::fromLatin1("Water"));
 QString errorMsg;
 int line, column;
 if (!doc.setContent(QString(waterXML), &errorMsg, &line, &column)) {
	boError(270) << k_funcinfo << "unable to load from waterXML (error in line " << line << ", column " << column << ", msg=" << errorMsg << ")" << endl;
	return false;
 }
 QDomElement root = doc.documentElement();
 if (root.isNull()) {
	boError(270) << k_funcinfo << "no root element" << endl;
	return false;
 }
 d->mWaterManager->setMap(this);
 return d->mWaterManager->loadFromXML(root);
}


bool BosonMap::loadCompleteMap(QDataStream& stream)
{
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
 createCells();
 if (!mCells) {
	boError() << k_funcinfo << "NULL cells" << endl;
	return false;
 }
 return true;
}

bool BosonMap::loadMapGeo(unsigned int width, unsigned int height)
{
 if (!isValidMapGeo(width, height)) {
	boError()<< k_funcinfo << "map geo is not valid: " << width << "x" << height << endl;
	return false;
 }

// map is ok - lets apply
 mMapWidth = width; // horizontal cell count
 mMapHeight = height; // vertical cell count

 delete[] mCells;
 mCells = 0;
 delete mHeightMap;
 mHeightMap = 0;
 delete mNormalMap;
 mNormalMap = 0;
 delete mTexMap;
 mTexMap = 0;

 return true;
}

bool BosonMap::applyGroundTheme(const QString& id)
{
 if (id.isEmpty()) {
	boError() << k_funcinfo << "empty id string" << endl;
	return false;
 }
 BosonGroundTheme* theme = (BosonGroundTheme*)BosonData::bosonData()->groundTheme(id);
 if (!theme) {
	boError() << k_funcinfo << "Cannot find groundTheme with id=" << id << endl;
	return false;
 }
 mGroundTheme = theme;
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
 QImage map(heightMapBuffer);
 if (map.isNull()) {
	boError() << k_funcinfo << "received an invalid image buffer - null image" << endl;
	return false;
 }
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

 BoHeightMap heightMap(width() + 1, height() + 1);
 for (unsigned int y = 0; y < height() + 1; y++) {
	// AB: warning: from Qt docs: "If you are accessing 16-bpp image data,
	// you must handle endianness yourself."
	// do we have to care about this? (since we are using 16bpp)
	// AB: the comment about endianness might be obsolete here as it applies
	// to scanLine(), but we use pixel() instead now!
	for (unsigned int x = 0; x < width() + 1; x++) {
		heightMap.setHeightAt(x, y, BoHeightMap::pixelToHeight(qRed(map.pixel(x, y))));
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
 bool ret = mHeightMap->load(stream);
 if (ret) {
	delete mNormalMap;
	mNormalMap = new BoNormalMap(width() + 1, height() + 1);
	heightsInRectChanged(0, 0, width(), height());
 }
 return ret;
}

bool BosonMap::loadTexMap(QDataStream& stream)
{
 // we allocate memory for all possible textures, even if this map doesn't use
 // them all. we also have to initialize all textures.
 // one day we may want to change this to save a few kb of memory on some maps
 // (none yet)
 if (mTexMap) {
	boWarning() << k_funcinfo << "already a texmap present - deleting..." << endl;
	delete mTexMap;
	mTexMap = 0;
 }
 mTexMap = new BoTexMap(groundTheme()->groundTypeCount(), width() + 1, height() + 1);
 return mTexMap->load(stream);
}

bool BosonMap::saveTexMap(QDataStream& stream) const
{
 if (!mTexMap) {
	BO_NULL_ERROR(mTexMap);
	return false;
 }
 if (!groundTheme()) {
	BO_NULL_ERROR(groundTheme());
	return false;
 }
 if (groundTheme()->groundTypeCount() != mTexMap->textureCount()) {
	boError() << k_funcinfo << "groundTheme()->groundTypeCount() differs from texMap->textureCount() !" << endl;
	return false;
 }
 return mTexMap->save(stream);
}

QByteArray BosonMap::saveMapToFile() const
{
 if (!isValidMapGeo(width(), height())) {
	boError() << k_funcinfo << "Map geo is not valid" << endl;
	return false;
 }
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

 QDomDocument doc(QString::fromLatin1("BosonMap"));
 QDomElement root = doc.createElement(QString::fromLatin1("BosonMap"));
 root.setAttribute(QString::fromLatin1("Version"), BOSONMAP_VERSION);
 doc.appendChild(root);
 QDomElement geometry = doc.createElement(QString::fromLatin1("Geometry"));
 root.appendChild(geometry);
 geometry.setAttribute(QString::fromLatin1("Width"), width());
 geometry.setAttribute(QString::fromLatin1("Height"), height());
 root.setAttribute(QString::fromLatin1("GroundTheme"), groundTheme()->identifier());

 return doc.toCString();
}

QByteArray BosonMap::saveWaterToFile() const
{
 QDomDocument doc(QString::fromLatin1("Water"));
 QDomElement root = doc.createElement(QString::fromLatin1("Water"));
 doc.appendChild(root);
 if (!d->mWaterManager->saveToXML(root)) {
	return QByteArray();
 }
 return doc.toCString();
}

QByteArray BosonMap::saveMapPreviewPNGToFile() const
{
 QByteArray imageData;
 QBuffer imageBuffer(imageData);
 imageBuffer.open(IO_WriteOnly);
 QImage image(width(), height(), 32);

 for (unsigned int y = 0; y < height(); y++) {
	unsigned int* line = (unsigned int*)image.scanLine(y);
	for (unsigned int x = 0; x < width(); x++) {
		unsigned int* p = line + x;
		bool isWater;
		int r, g, b;
		if (!calculateMiniMapGround(x, y, &r, &g, &b, &isWater)) {
			boError() << k_funcinfo << "unable to calculate color at " << x << "," << y << endl;
			return QByteArray();
		}
		if (isWater) {
			// uff - ugly hardcoded color...
			// see bosonglminimap.cpp for the most up to date
			// implementation
			*p = qRgb(0,64,192);
		} else {
			*p = qRgb(r,g,b);
		}
	}
 }


 if (!image.save(&imageBuffer, "PNG", 75)) {
	boError() << k_funcinfo << "error saving PNG" << endl;
	return QByteArray();
 }
 return imageData;
}


bool BosonMap::calculateMiniMapGround(int x, int y, int* _r, int* _g, int* _b, bool* coveredByWater) const
{
 if (!groundTheme()) {
	BO_NULL_ERROR(groundTheme());
	return false;
 }
 if (!texMap()) {
	BO_NULL_ERROR(texMap());
	return false;
 }
 if (!isValidCell(x, y)) {
	boError() << k_funcinfo << "invalid cell: (" << x << ", " << y << ")" << endl;
	return false;
 }

 // every cell has four corners - we mix them together to get the actual minimap
 // color.
 unsigned int cornerX[4] = { x, x + 1, x + 1,     x };
 unsigned int cornerY[4] = { y,     y, y + 1, y + 1 };
 int r = 0;
 int g = 0;
 int b = 0;
 for (int j = 0; j < 4; j++) {
	int alphaSum = 0; // sum of all textures
	int cornerRed = 0;
	int cornerGreen = 0;
	int cornerBlue = 0;

	for (unsigned int i = 0; i < groundTheme()->groundTypeCount(); i++) {
		int alpha = (int)texMapAlpha(i, cornerX[j], cornerY[j]);
		alphaSum += alpha;

		QRgb rgb = groundTheme()->groundType(i)->color;
		int red = qRed(rgb);
		int green = qGreen(rgb);
		int blue = qBlue(rgb);
		cornerRed += red * alpha / 255;
		cornerGreen += green * alpha / 255;
		cornerBlue += blue * alpha / 255;
	}
	if (alphaSum == 0) {
		// nothing to do for this corner.
		continue;
	}
	cornerRed = cornerRed * 255 / alphaSum;
	cornerGreen = cornerGreen * 255 / alphaSum;
	cornerBlue = cornerBlue * 255 / alphaSum;

	r += cornerRed;
	g += cornerGreen;
	b += cornerBlue;
 }

 // TODO: maybe also take the heightmap/normalmap into account?
 //       heightmap: something like this is acceptable:
 //       h = pixelValueAtHeightMapForCell(x,y)
 //       h += 113; // "113 := 1.0f" -> 113 .. 368
 //       h /= 255;
 //       h = sqrtf(h);
 //       h = sqrtf(h); // optional
 // another idea: use the normal of the cell to actually calculate the light at
 // that position
 // -> for the mappreview we might simply use a random light position or so

 r /= 4;
 g /= 4;
 b /= 4;

 (*_r) = r;
 (*_g) = g;
 (*_b) = b;
 (*coveredByWater) = cell(x, y)->isWater();

 return true;
}

bool BosonMap::saveCompleteMap(QDataStream& stream) const
{
 // AB: note we don't save the map preview here - it can be reconstructed from
 //     the map data.

 QByteArray buffer;
 buffer = saveMapToFile();
 if (buffer.size() == 0) {
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
 return true;
}

bool BosonMap::saveHeightMap(QDataStream& stream) const
{
 if (!mHeightMap) {
	BoHeightMap heightMap(width() + 1, height() + 1);
	return heightMap.save(stream);
 }
 return mHeightMap->save(stream);
}

QByteArray BosonMap::saveHeightMapImage() const
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
 if (!mHeightMap) {
	int l = BoHeightMap::heightToPixel(0.0f);
	image.fill(l);
 } else {
	// AB: this *might* be correct, but i am not sure about this. (02/11/22)
	for (int y = 0; y < image.height(); y++) {
		uint* p = (uint*)image.scanLine(y); // AB: maybe use setPixel() instead, due to endianness
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

QByteArray BosonMap::saveTexMapImage(unsigned int texture) const
{
 // this function is sloooow !
 if (!width() || !height()) {
	boError() << k_funcinfo << "Cannot save empty map" << endl;
	return QByteArray();
 }
 QImage image;
 if (!image.create(width() + 1, height() + 1, 32, 0)) { // AB: 16bpp isnt available for X11 (only for qt embedded)
	boError() << k_funcinfo << "Unable to create texmap!" << endl;
	return QByteArray();
 }
 if (!mHeightMap) {
	image.fill(0);
 } else {
	// AB: this *might* be correct, but i am not sure about this. (02/11/22)
	for (int y = 0; y < image.height(); y++) {
		uint* p = (uint*)image.scanLine(y); // AB: maybe use setPixel() instead, due to endianness
		for (int x = 0; x < image.width(); x++) {
			int v = (int)texMapAlpha(texture, x, y);
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
	for (unsigned int y = 0; y < height(); y++) {
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

void BosonMap::setHeightsAtCorners(const QValueList< QPair<QPoint, float> >& heights)
{
 BO_CHECK_NULL_RET(mHeightMap);
 BO_CHECK_NULL_RET(mNormalMap);

 int minX = 0;
 int maxX = 0;
 int minY = 0;
 int maxY = 0;
 QValueList< QPair<QPoint, float> >::const_iterator it;
 for (it = heights.begin(); it != heights.end(); ++it) {
	int x = (*it).first.x();
	int y = (*it).first.y();
	mHeightMap->setHeightAt(x, y, (*it).second);

	minX = QMIN(minX, x);
	minY = QMIN(minY, y);
	maxX = QMAX(maxX, x);
	maxY = QMAX(maxY, y);
 }

 // Update affected normals
 minX = QMAX(0, minX - 1);
 maxX = QMIN((int)width(), maxX + 1);
 minY = QMAX(0, minY - 1);
 maxY = QMIN((int)height(), maxY + 1);
 heightsInRectChanged(minX, minY, maxX, maxY);
}

void BosonMap::setHeightAtCorner(int x, int y, float h)
{
 QValueList< QPair<QPoint, float> > heights;
 heights.append(QPair<QPoint, float>(QPoint(x, y), h));
 setHeightsAtCorners(heights);
}

float BosonMap::waterDepthAtCorner(int x, int y) const
{
 if (!d->mWaterManager) {
	boError() << k_funcinfo << "NULL water manager" << endl;
	return 0.0f;
 }
 return d->mWaterManager->waterDepthAtCorner(x, y);
}

const QPtrList<BoLake>* BosonMap::lakes() const
{
 BO_CHECK_NULL_RET0(d->mWaterManager);
 return d->mWaterManager->lakes();
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

 loadMapGeo(width, height);
}

void BosonMap::fill(unsigned int groundtype)
{
 if (!mCells) {
	boError() << k_funcinfo << "NULL cells" << endl;
	return;
 }
 BO_CHECK_NULL_RET(mTexMap);
 BO_CHECK_NULL_RET(groundTheme());
 if (groundtype >= groundTheme()->groundTypeCount()) {
	boError() << k_funcinfo << "invalid groundtype " << groundtype << endl;
	return;
 }

 mTexMap->fill(groundtype);
}

float BosonMap::cellAverageHeight(int x, int y) const
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

bool BosonMap::generateCellsFromTexMap()
{
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
 if (groundTheme()->groundTypeCount() == 0) {
	boError() << k_funcinfo << "0 ground types in ground theme" << endl;
	return false;
 }
 return true;
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
 if (x < 0 || (uint)x > width()) {
	boError() << k_funcinfo << "invalid x coordinate: " << x << endl;
	return;
 }
 if (y < 0 || (uint)y > height()) {
	boError() << k_funcinfo << "invalid y coordinate: " << y << endl;
	return;
 }
 if (texCount > groundTheme()->groundTypeCount()) {
	boError() << k_funcinfo << "invalid groundTypeCount " << texCount << " must be <= " << groundTheme()->groundTypeCount() << endl;
	return;
 }
 BO_CHECK_NULL_RET(alpha);
 BO_CHECK_NULL_RET(textures);
 for (unsigned int i = 0; i < texCount; i++) {
	unsigned int texture = textures[i];
	if (texture >= groundTheme()->groundTypeCount()) {
		boError() << k_funcinfo << "invalid texture " << texture << endl;
		continue;
	}
	setTexMapAlpha(texture, x, y, alpha[i]);
 }

 d->mQuadTreeCollection->cellTextureChanged(this, x, y, x, y);
}

void BosonMap::recalculateNormalsInRect(int x1, int y1, int x2, int y2)
{
 // FIXME: simplify this method? Is it possible?

 // First calculate plane normals for all cells in rect
 // We need to have additional 1-cell border to correctly calculate normals at
 //  the border of given rect.
 // Here we calculate rect for cells, taking this border and map size into
 //  account
 int cx1 = QMAX(0, x1 - 1);
 int cy1 = QMAX(0, y1 - 1);
 int cx2 = QMIN((int)width() - 1, x2 + 1);
 int cy2 = QMIN((int)height() - 1, y2 + 1);

 int w = cx2 - cx1 + 1;  // Width and height of the rect
 int h = cy2 - cy1 + 1;
 if (w * h <= 0) {
	boWarning() << k_funcinfo << "w*h <= 0 is not valid" << endl;
	return;
 }
 BoVector3Float a, b, c, n;
 BoVector3Float* normals = new BoVector3Float[w * h];
#define NORM(x, y) normals[(y - cy1) * w + (x - cx1)]
 // FIXME: this is not entirely correct: our cells consist of a single quad atm,
 //  however, they can be "folded" so that it seems to consist of two triangles.
 //  In this case, the cell actually has two planes - one for each half.
 for (int y = cy1; y <= cy2; y++) {
	for (int x = cx1; x <= cx2; x++) {
		a.set(x, y, heightAtCorner(x, y));
		b.set(x + 1, y, heightAtCorner(x + 1, y));
		c.set(x, y + 1, heightAtCorner(x, y + 1));
		n = BoVector3Float::crossProduct(c - b, a - b);
		n.normalize();
		NORM(x, y) = n;
	}
 }


 // Pass 2: calculate normal for each corner by taking average of normals of all
 //  cells that have this corner.
 int count = 0;
 for (int y = y1; y <= y2; y++) {
	for (int x = x1; x <= x2; x++) {
		// Add all cell normals to n
		n.reset();
		count = 0;
		// upper-left cell
		if (x > cx1 && y > cy1) {
			n.add(NORM(x - 1, y - 1));
			count++;
		}
		// upper-right cell
		if (x <= cx2 && y > cy1) {
			n.add(NORM(x, y - 1));
			count++;
		}
		// lower-right cell
		if (x <= cx2 && y <= cy2) {
			n.add(NORM(x, y));
			count++;
		}
		// lower-left cell
		if (x > cx1 && y <= cy2) {
			n.add(NORM(x - 1, y));
			count++;
		}
		n.scale(1.0f / count);
		n.normalize();
		// Cells will be drawn with y-coordinate reversed, so to make lighting look
		//  correct, we need to reverse normal's y-component here as well.
		n.setY(-n.y());
		mNormalMap->setNormalAt(x, y, n);
	}
 }
#undef NORM
 delete[] normals;
}

void BosonMap::heightsInRectChanged(int minX, int minY, int maxX, int maxY)
{
 recalculateNormalsInRect(minX, minY, maxX, maxY);
 d->mQuadTreeCollection->cellHeightChanged(this, minX, minY, maxX, maxY);
}


int BosonMap::mapFileFormatVersion()
{
 return BOSONMAP_VERSION;
}

void BosonMap::addColorMap(BoColorMap* map, const QString& name)
{
 if (d->mColorMaps[name]) {
	boWarning() << k_funcinfo << "already have a colormap named " << name << endl;
	BoColorMap* old = d->mColorMaps[name];
	delete old;
 }
 d->mColorMaps.insert(name, map);
 emit signalColorMapsChanged();
}

QDict<BoColorMap>* BosonMap::colorMaps()
{
 return &d->mColorMaps;
}

void BosonMap::removeColorMap(const QString& name)
{
 BoColorMap* map = d->mColorMaps.take(name);
 if (map == mActiveColorMap) {
	mActiveColorMap = 0;
 }
 delete map;
 emit signalColorMapsChanged();
}

void BosonMap::registerQuadTree(BoGroundQuadTreeNode* tree)
{
 d->mQuadTreeCollection->registerTree(tree);
}

void BosonMap::unregisterQuadTree(BoGroundQuadTreeNode* tree)
{
 d->mQuadTreeCollection->unregisterTree(tree);
}

