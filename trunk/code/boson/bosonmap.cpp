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
#include "bowater.h"
#include "botexture.h"

#include <qdatastream.h>
#include <qimage.h>
#include <qdom.h>

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


bool BoTexMap::save(QDataStream& stream)
{
 boDebug() << k_funcinfo << endl;
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
 boDebug() << k_funcinfo << endl;
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
 boDebug() << "done" << endl;
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

 boDebug() << k_funcinfo << endl;

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
 delete mColorMap;
 delete mNormalMap;
 delete mHeightMap;
 delete mTexMap;
 delete d;
}

void BosonMap::init()
{
 d = new BosonMapPrivate;
 mCells = 0;
 mColorMap = 0;
 mHeightMap = 0;
 mNormalMap = 0;
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
 if (mTexMap->textureCount() != mGroundTheme->textureCount()) {
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
 if (!loadGroundTheme(groundTheme)) {
	boError(270) << k_funcinfo << "Could not load the ground theme" << endl;
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
 boWaterManager->setMap(this);
 boWaterManager->loadFromXML(root);
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
 createCells();
 if (!mCells) {
	boError() << k_funcinfo << "NULL cells" << endl;
	return false;
 }
 // should this be done somewhere else?
 boDebug() << k_funcinfo << "creating colormap" << endl;
 createColorMap();
 boDebug() << k_funcinfo << "created colormap" << endl;
 return true;
}

bool BosonMap::loadMapGeo(unsigned int width, unsigned int height)
{
 boDebug() << k_funcinfo << endl;
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
 boDebug() << k_funcinfo << "deleting colormap" << endl;
 delete mColorMap;
 mColorMap = 0;
 delete mTexMap;
 mTexMap = 0;

 return true;
}

bool BosonMap::loadGroundTheme(const QString& id)
{
 mGroundTheme = (BosonGroundTheme*)BosonData::bosonData()->groundTheme(id);
 if (!mGroundTheme) {
	boError() << k_funcinfo << "Cannot find groundTheme with id=" << id << endl;
	return false;
 }
 emit signalGroundThemeChanged(mGroundTheme);
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
 boDebug() << k_funcinfo << "loading height map from network stream" << endl;
 bool ret = mHeightMap->load(stream);
 if (ret) {
	recalculateNormalMap();
 }
 return ret;
}

bool BosonMap::loadTexMap(QDataStream& stream)
{
 boDebug() << k_funcinfo << endl;

 // we allocate memory for all possible textures, even if this map doesn't use
 // them all. we also have to initialize all textures.
 // one day we may want to change this to save a few kb of memory on some maps
 // (none yet)
 if (mTexMap) {
	boWarning() << k_funcinfo << "already a texmap present - deleting..." << endl;
	delete mTexMap;
	mTexMap = 0;
 }
 mTexMap = new BoTexMap(groundTheme()->textureCount(), width() + 1, height() + 1);
 return mTexMap->load(stream);
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
 if (groundTheme()->textureCount() != mTexMap->textureCount()) {
	boError() << k_funcinfo << "groundTheme()->texturCount() differs from texMap->textureCount() !" << endl;
	return false;
 }
 return mTexMap->save(stream);
}

QByteArray BosonMap::saveMapToFile()
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

QByteArray BosonMap::saveWaterToFile()
{
 QDomDocument doc(QString::fromLatin1("Water"));
 QDomElement root = doc.createElement(QString::fromLatin1("Water"));
 doc.appendChild(root);
 boWaterManager->saveToXML(root);
 return doc.toCString();
}

bool BosonMap::saveCompleteMap(QDataStream& stream)
{
 // AB: we may have a problem here - this stream is meant to be sent through
 // network, but it is very big! (sometimes several MB)
 // we should compress it!

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

QByteArray BosonMap::saveTexMapImage(unsigned int texture)
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
 boDebug() << k_funcinfo << "texmap: " << image.width() << "x" << image.height() << endl;
 if (!mHeightMap) {
	boDebug() << k_funcinfo << "dummy texmap..." << endl;
	image.fill(0);
 } else {
	boDebug() << k_funcinfo << "real texmap" << endl;
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

void BosonMap::setHeightAtCorner(int x, int y, float h)
{
 BO_CHECK_NULL_RET(mHeightMap);
 BO_CHECK_NULL_RET(mNormalMap);
 boDebug() << k_funcinfo << endl;
 mHeightMap->setHeightAt(x, y, h);
 // Update affected normals
 // Whenever a corner's height changes, normals of four cells, that have this
 //  corner, also changes and every corner that these cells have must be updated
 //  If changed corner is not at the edge of the map, 9 corners have to be
 //  updated.
 recalculateNormalsInRect(QMAX(0, x - 1), QMAX(0, y - 1),
		QMIN((int)width(), x + 1), QMIN((int)height(), y + 1));
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

BoTexture* BosonMap::currentTexture(int texture, int advanceCallsCount) const
{
 BO_CHECK_NULL_RET0(groundTheme());
 BoTextureArray* t = groundTheme()->textures(texture);
 BO_CHECK_NULL_RET0(t);
 return t->texture((advanceCallsCount / groundTheme()->textureAnimationDelay(texture)) % t->count());
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
	// update minimap
	// we may want to group these cells into a single array to save some
	// speed once the editor is able to modify several cells at once.
	// currently it isn't necessary.
	emit signalCellChanged(cellsX[i], cellsY[i]);
 }
}

void BosonMap::recalculateNormalMap()
{
 boDebug() << k_funcinfo << endl;
 if (mNormalMap) {
	// Old normal map should already be deleted
	boWarning() << k_funcinfo << "Old normal map not deleted!" << endl;
	delete mNormalMap;
	mNormalMap = 0;
 }

 mNormalMap = new BoNormalMap(width() + 1, height() + 1);

 recalculateNormalsInRect(0, 0, width(), height());
}

void BosonMap::recalculateNormalsInRect(int x1, int y1, int x2, int y2)
{
 boDebug() << k_funcinfo << "Rect: (" << x1 << "; " << y1 << ")-(" << x2 << "; " << y2 << ")" << endl;
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
 boDebug() << k_funcinfo << "done" << endl;
}


int BosonMap::mapFileFormatVersion()
{
 return BOSONMAP_VERSION;
}

void BosonMap::createColorMap()
{
 boDebug() << k_funcinfo << "" << endl;
 if (mColorMap) {
	// Old normal map should already be deleted
	boWarning() << k_funcinfo << "Old color map not deleted!" << endl;
	delete mColorMap;
	mColorMap = 0;
 }

 boDebug() << k_funcinfo << "creating map" << endl;
 mColorMap = new BoColorMap(width(), height());

 boDebug() << k_funcinfo << "initing map" << endl;
 // Init colormap to gray
 unsigned char color[] = { 128, 128, 128 };
 for (unsigned int y = 0; y < height(); y++) {
	for (unsigned int x = 0; x < width(); x++) {
		mColorMap->setColorAt(x, y, color);
	}
 }
 boDebug() << k_funcinfo << "done" << endl;
}

