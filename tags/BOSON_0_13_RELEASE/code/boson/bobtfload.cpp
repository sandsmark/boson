/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bobtfload.h"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kmdcodec.h>

#include <qfile.h>
#include <qdatastream.h>
#include <qimage.h>
#include <qgl.h>

#include <math.h>

#define BTF_FILE_ID "BTF "
#define BTF_FILE_ID_LEN 4

#define BTF_VERSION_MAJOR 0
#define BTF_VERSION_MINOR 0
#define BTF_VERSION_RELEASE 1

#define BTF_MAKE_VERSION(a, b, c)  ( ((a) << 16) | ((b) << 8) | (c) )
#define BTF_CURRENT_VERSION \
	BTF_MAKE_VERSION(BTF_VERSION_MAJOR, BTF_VERSION_MINOR, BTF_VERSION_RELEASE)


// Magics for different chunks
#define BTF_MAGIC_TEXTURE              0x100000
#define BTF_MAGIC_TEXTURE_END          0x100001
#define BTF_MAGIC_TEXTURE_INFO         0x100010
#define BTF_MAGIC_TEXTURE_INFO_END     0x100011
#define BTF_MAGIC_TEXTURE_LEVEL        0x100020
#define BTF_MAGIC_TEXTURE_LEVEL_END    0x100021
#define BTF_MAGIC_TEXTURE_LEVEL_INFO   0x100022
#define BTF_MAGIC_TEXTURE_LEVEL_DATA   0x100023
#define BTF_MAGIC_END                  0xFFFFFF



BoBTFLoad::BoBTFLoad(const QString& file)
{
 mFile = file;
 mLevels = 0;
}

BoBTFLoad::~BoBTFLoad()
{
 for (unsigned int i = 0; i < mLevels; i++) {
	delete[] mTextureLevelData[i];
 }
 mTextureLevelData.clear();
}

const QString& BoBTFLoad::file() const
{
 return mFile;
}

bool BoBTFLoad::loadTexture()
{
 QString btf = createBTFFile();
 if (btf.isEmpty()) {
	boError() << k_funcinfo << "cannot create .btf file" << endl;
	return false;
 }

 if (mFile.isEmpty()) {
	boError() << k_funcinfo << "No file has been specified for loading" << endl;
	return false;
 }
 QFile f(btf);
 if (!f.open(IO_ReadOnly)) {
	boError() << k_funcinfo << "can't open " << btf << endl;
	return false;
 }

 QDataStream stream(&f);

 char header[BTF_FILE_ID_LEN];
 stream.readRawBytes(header, BTF_FILE_ID_LEN);
 if (strncmp(header, BTF_FILE_ID, BTF_FILE_ID_LEN) != 0) {
	boError() << k_funcinfo << "This file doesn't seem to be in BTF format (invalid header)!" << endl;
	return false;
 }

 Q_UINT32 versionCode;
 stream >> versionCode;
 if (versionCode < BTF_MAKE_VERSION(0, 0, 1)) {
	boError() << k_funcinfo << "Unsupported BMF version 0x"
			<< QString::number(versionCode, 16) << endl;
	boError() << k_funcinfo << "Last supported version is 0x"
			<< QString::number(BTF_MAKE_VERSION(0, 0, 1), 16) << endl;
	boError() << k_funcinfo << "Current version is 0x"
			<< QString::number(BTF_CURRENT_VERSION, 16) << endl;
	return false;
 }

 Q_UINT32 magic;
 stream >> magic;
 if (magic != BTF_MAGIC_TEXTURE) {
	boError() << k_funcinfo << "Loading failed (invalid texture magic)!" << endl;
	return false;
 }


 if (!loadInfo(stream)) {
	return false;
 }

 for (unsigned int i = 0; i < mLevels; i++) {
	if (!loadTextureLevel(stream)) {
		return false;
	}
 }



 stream >> magic;
 if (magic != BTF_MAGIC_TEXTURE_END) {
	boError() << k_funcinfo << "Loading failed (invalid texture end magic)!" << endl;
	return false;
 }

  // Verify that loading was successful
 stream >> magic;
 if (magic != BTF_MAGIC_END) {
	boError() << k_funcinfo << "Loading failed (invalid end magic)!" << endl;
	return false;
 }

 return true;
}

bool BoBTFLoad::loadInfo(QDataStream& stream)
{
 Q_UINT32 magic;
 stream >> magic;
 if (magic != BTF_MAGIC_TEXTURE_INFO) {
	boError() << k_funcinfo << "Loading failed (no info section)!" << endl;
	return false;
 }

 Q_UINT32 w;
 Q_UINT32 h;
 Q_UINT32 levels;

 // width/height of unscaled image
 stream >> w;
 stream >> h;

 const unsigned int maxSize = 65536;
 if (w > maxSize || h > maxSize) {
	boError() << k_funcinfo << "invalid size: width=" << w << " height=" << h << endl;
	return false;
 }

 // number of mipmap levels
 // this is kinda redundant, as we can retrieve it from w and h
 stream >> levels;

 unsigned int maxLevels = log2(QMAX(w, h)) + 2; // +2 to catch rounding errors
 if (levels > maxLevels) {
	boError() << k_funcinfo << "broken file: tried to allocate " << levels << " mipmap levels for a original size of width=" << w << " height=" << h << endl;
	return false;
 }

 mLevels = levels;


 // TODO: load options
 // * RGB vs RGBA
 // * ...


 mTextureLevelData.resize(mLevels);
 mTextureLevelWidth.resize(mLevels);
 mTextureLevelHeight.resize(mLevels);
 for (unsigned int i = 0; i < mLevels; i++) {
	mTextureLevelData[i] = 0;
	mTextureLevelWidth[i] = 0;
	mTextureLevelHeight[i] = 0;
 }

 // Check end magic
 stream >> magic;
 if (magic != BTF_MAGIC_TEXTURE_INFO_END) {
	boError() << k_funcinfo << "Loading failed (info section end magic not found)!" << endl;
	return false;
 }

 return true;
}

bool BoBTFLoad::loadTextureLevel(QDataStream& stream)
{
 Q_UINT32 magic;
 stream >> magic;
 if (magic != BTF_MAGIC_TEXTURE_LEVEL) {
	boError() << k_funcinfo << "Loading failed (expected texture level section)! " << magic << endl;
	return false;
 }

 stream >> magic;
 if (magic != BTF_MAGIC_TEXTURE_LEVEL_INFO) {
	boError() << k_funcinfo << "Loading failed (expected texture level info)!" << endl;
	return false;
 }
 Q_UINT32 w;
 Q_UINT32 h;
 Q_UINT32 level;
 stream >> w;
 stream >> h;
 stream >> level;

 mTextureLevelWidth[level] = w;
 mTextureLevelHeight[level] = h;

 stream >> magic;
 if (magic != BTF_MAGIC_TEXTURE_LEVEL_DATA) {
	boError() << k_funcinfo << "Loading failed (expected texture level data)!" << endl;
	return false;
 }
 // the image is _always_ saved as 32 bits per pixel
 char* data;
 unsigned int length;
 stream.readBytes(data, length);
 if (!data) {
	boError() << k_funcinfo << "could not read data for mipmap level " << level << endl;
	return false;
 }
 if (length != w * h * 4) {
	boError() << k_funcinfo << "expected " << w * h * 4 << " bytes at mipmap level " << level << " but read " << length << endl;
	delete[] data;
	return false;
 }
 mTextureLevelData[level] = data;


 // Check end magic
 stream >> magic;
 if (magic != BTF_MAGIC_TEXTURE_LEVEL_END) {
	boError() << k_funcinfo << "Loading failed (texture level section end magic not found)!" << endl;
	return false;
 }

 return true;
}

QString BoBTFLoad::createBTFFile()
{
 QFile textureFile(file());
 if (!textureFile.open(IO_ReadOnly)) {
	boError() << k_funcinfo << "cannot open " << file() << " for reading" << endl;
	return QString::null;
 }
 KMD5 md5(textureFile.readAll());
 mMD5 = md5.hexDigest();
 textureFile.close();

 QString cacheFileName = KGlobal::dirs()->findResource("data", QString("%1/texture-%2.btf").arg("boson/texturecache").arg(mMD5));

 if (!cacheFileName.isEmpty()) {
	// file already exists. nothing to do.
	return cacheFileName;
 }

 QImage img;
 if (!img.load(file())) {
	boError() << k_funcinfo << "Could not load image from file " << file() << endl;
	return QString::null;
 }

 if (img.width() == 0 || img.height() == 0) {
	boError() << k_funcinfo << "invalid image " << file() << endl;
	return QString::null;
 }

 cacheFileName = KGlobal::dirs()->saveLocation("data", "boson/texturecache/");
 cacheFileName += QString("texture-%1.btf").arg(mMD5);

 QFile cacheFile(cacheFileName);
 if (!cacheFile.open(IO_WriteOnly)) {
	boError() << k_funcinfo << "Could not open " << cacheFileName << " for writing" << endl;
	return QString::null;
 }

 QDataStream stream(&cacheFile);

 int w = 1;
 int h = 1;
 while (w < img.width()) {
	w *= 2;
 }
 while (h < img.height()) {
	h *= 2;
 }
 QImage scaledImage = img.smoothScale(w, h, QImage::ScaleFree);

 if (!createBTFFile(scaledImage, stream)) {
	boError() << k_funcinfo << "error while saving .btf file" << endl;
	cacheFile.close();
	cacheFile.remove();
	return QString::null;
 }

 return cacheFileName;
}

bool BoBTFLoad::createBTFFile(const QImage& img, QDataStream& stream) const
{
 stream.writeRawBytes(BTF_FILE_ID, BTF_FILE_ID_LEN);
 stream << (Q_UINT32)BTF_CURRENT_VERSION;
 stream << (Q_UINT32)BTF_MAGIC_TEXTURE;

 // Save Info
 stream << (Q_UINT32)BTF_MAGIC_TEXTURE_INFO;
 stream << (Q_UINT32)img.width();
 stream << (Q_UINT32)img.height();
 Q_UINT32 levels = 1;
 int w = img.width();
 int h = img.height();
 while (w > 1 || h > 1) {
	if (w > 1) {
		w /= 2;
	}
	if (h > 1) {
		h /= 2;
	}
	levels++;
 }
 stream << (Q_UINT32)levels;
 stream << (Q_UINT32)BTF_MAGIC_TEXTURE_INFO_END;

 w = 0;
 h = 0;
 unsigned int level = levels;
 do {
	if (level == 0) {
		boError() << k_funcinfo << "tried to save more than " << levels << " mipmap levels" << endl;
		return false;
	}
	level--;
	if (w < img.width()) {
		w *= 2;
	}
	if (h < img.height()) {
		h *= 2;
	}
	if (w == 0 || h == 0) {
		w = 1;
		h = 1;
	}
	QImage levelTexture = img.smoothScale(w, h, QImage::ScaleFree);
	if (!saveTextureLevel(level, levelTexture, stream)) {
		boError() << k_funcinfo << "Could not save texture mipmap level " << level << endl;
		return false;
	}
 } while (w < img.width() || h < img.height());
 if (level != 0) {
	boError() << k_funcinfo << "not all levels saved" << endl;
	return false;
 }


 stream << (Q_UINT32)BTF_MAGIC_TEXTURE_END;
 stream << (Q_UINT32)BTF_MAGIC_END;

 return true;
}

bool BoBTFLoad::saveTextureLevel(unsigned int level, const QImage& img, QDataStream& stream) const
{
 QImage gl = QGLWidget::convertToGLFormat(img);
 if (gl.isNull()) {
	boError() << k_funcinfo << "unable to convert image to GL format" << endl;
	return false;
 }

 stream << (Q_UINT32)BTF_MAGIC_TEXTURE_LEVEL;

 stream << (Q_UINT32)BTF_MAGIC_TEXTURE_LEVEL_INFO;
 stream << (Q_UINT32)gl.width();
 stream << (Q_UINT32)gl.height();
 stream << (Q_UINT32)level;

 stream << (Q_UINT32)BTF_MAGIC_TEXTURE_LEVEL_DATA;
 stream.writeBytes((char*)gl.bits(), gl.numBytes());

 stream << (Q_UINT32)BTF_MAGIC_TEXTURE_LEVEL_END;

 return true;
}


