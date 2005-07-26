/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann <b_mann@gmx.de>

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

#include "boinfoglcache.h"

#include "../../bomemory/bodummymemory.h"
#include "boinfo.h"
#include "bodebug.h"

#include <qstringlist.h>
#include <qregexp.h>

static int getIntFromList(const QStringList& list, const QString& start, int default_ = 0)
{
 QRegExp reg(QString("^%1 = ").arg(start));
 QStringList tmp = list.grep(reg);
 if (tmp.isEmpty()) {
	boWarning() << k_funcinfo << "no such entry " << start << endl;
	for (int i = 0; i < list.count(); i++) {
		boDebug() << list[i] << endl;
	}
	return default_;
 }
 QString s = tmp[0];
 QString s2 = s.remove(reg);
 bool ok;
 int ret = s2.toInt(&ok);
 if (!ok) {
	boWarning() << k_funcinfo << "not a number " << s2 << " from " << s << endl;
	return default_;
 }
 return ret;
}

class BoInfoGLCachePrivate
{
public:
	BoInfoGLCachePrivate()
	{
	}

	bool mCacheDirty;

	bool mIsDirect;

	QString mGLXClientVersionString;
	QString mGLXClientVendorString;
	QString mGLXServerVersionString;
	QString mGLXServerVendorString;
	QStringList mGLXClientExtensions;
	QStringList mGLXServerExtensions;
	int mGLXVersionMajor;
	int mGLXVersionMinor;

	QString mGLUVersionString;
	QStringList mGLUExtensions;

	QString mOpenGLVersionString;
	QString mOpenGLVendorString;
	QString mOpenGLRendererString;
	unsigned int mOpenGLVersion;
	QStringList mOpenGLExtensions;

	int mMaxTextureSize;
	bool mSupportsGenerateMipmap;
	bool mSupportsTextureCompressionS3TC;
	bool mSupportsTextureCube;
	bool mSupportsTexture3D;
};

BoInfoGLCache::BoInfoGLCache(BoInfo* info)
{
 mInfo = info;
 d = new BoInfoGLCachePrivate;
 d->mCacheDirty = true;
}

void BoInfoGLCache::setDirty(bool dirty)
{
 d->mCacheDirty = dirty;
}

int BoInfoGLCache::getInt(int key) const
{
 return mInfo->getInt(key);
}

unsigned int BoInfoGLCache::getUInt(int key) const
{
 return mInfo->getUInt(key);
}

QString BoInfoGLCache::getString(int key) const
{
 return mInfo->getString(key);
}

bool BoInfoGLCache::getBool(int key) const
{
 return mInfo->getBool(key);
}

void BoInfoGLCache::updateConst() const
{
 BoInfoGLCache* c = (BoInfoGLCache*)this;
 c->update();
}

void BoInfoGLCache::update()
{
 d->mIsDirect = getBool(BoInfo::IsDirect);
 d->mGLXClientVersionString = getString(BoInfo::GLXClientVersionString);
 d->mGLXClientVendorString = getString(BoInfo::GLXClientVendorString);
 d->mGLXClientExtensions = QStringList::split('\n', getString(BoInfo::GLXClientExtensionsString));
 d->mGLXServerExtensions = QStringList::split('\n', getString(BoInfo::GLXServerExtensionsString));
 d->mGLXVersionMajor = getInt(BoInfo::GLXVersionMajor);
 d->mGLXVersionMinor = getInt(BoInfo::GLXVersionMinor);

 d->mGLUVersionString = getString(BoInfo::GLUVersionString);
 d->mGLUExtensions = QStringList::split('\n', getString(BoInfo::GLUExtensionsString));

 d->mOpenGLVersionString = getString(BoInfo::OpenGLVersionString);
 d->mOpenGLVendorString = getString(BoInfo::OpenGLVendorString);
 d->mOpenGLRendererString = getString(BoInfo::OpenGLRendererString);
 d->mOpenGLVersion = getUInt(BoInfo::OpenGLVersion);
 d->mOpenGLExtensions = QStringList::split('\n', getString(BoInfo::OpenGLExtensionsString));

 QStringList glValues = mInfo->openGLValues();
 d->mMaxTextureSize = getIntFromList(glValues, "GL_MAX_TEXTURE_SIZE", 1);

 d->mSupportsGenerateMipmap = false;
 if (d->mOpenGLVersion >= MAKE_VERSION(1,4,0) || d->mOpenGLExtensions.contains("GL_SGIS_generate_mipmap")) {
	d->mSupportsGenerateMipmap = true;
 }

 d->mSupportsTextureCompressionS3TC = false;
 if (d->mOpenGLExtensions.contains("GL_EXT_texture_compression_s3tc") &&
		(d->mOpenGLVersion >= MAKE_VERSION(1,3,0) || d->mOpenGLExtensions.contains("GL_ARB_texture_compression"))) {
	d->mSupportsTextureCompressionS3TC = true;
 }
 d->mSupportsTexture3D = false;
 if (d->mOpenGLVersion >= MAKE_VERSION(1,2,0) || d->mOpenGLExtensions.contains("GL_EXT_texture3D")) {
	d->mSupportsTexture3D = true;
 }

  // Cube maps are part of the core since OpenGL 1.3
 d->mSupportsTextureCube = false;
 if (d->mOpenGLVersion >= MAKE_VERSION(1,3,0) || d->mOpenGLExtensions.contains("GL_ARB_texture_cube_map")) {
	d->mSupportsTextureCube = true;
 }

#if 0
 d->mMaxCubeTextureSize = 0;
 if (d->mSupportsTextureCube) {
	d->mMaxCubeTextureSize = 0;
 }

 
d->mSupportsShaders = false;
#endif
}

bool BoInfoGLCache::isDirect() const
{
 return d->mIsDirect;
}

const QStringList& BoInfoGLCache::openGLExtensions() const
{
 if (d->mCacheDirty) {
	updateConst();
 }
 return d->mOpenGLExtensions;
}

const QStringList& BoInfoGLCache::gluExtensions() const
{
 if (d->mCacheDirty) {
	updateConst();
 }
 return d->mGLUExtensions;
}

const QString& BoInfoGLCache::gluVersionString() const
{
 if (d->mCacheDirty) {
	updateConst();
 }
 return d->mGLUVersionString;
}

const QStringList& BoInfoGLCache::glXClientExtensions() const
{
 if (d->mCacheDirty) {
	updateConst();
 }
 return d->mGLXClientExtensions;
}

const QStringList& BoInfoGLCache::glXServerExtensions() const
{
 if (d->mCacheDirty) {
	updateConst();
 }
 return d->mGLXServerExtensions;
}

const QString& BoInfoGLCache::glXClientVersionString() const
{
 if (d->mCacheDirty) {
	updateConst();
 }
 return d->mGLXClientVersionString;
}

const QString& BoInfoGLCache::glXClientVendorString() const
{
 if (d->mCacheDirty) {
	updateConst();
 }
 return d->mGLXClientVendorString;
}

const QString& BoInfoGLCache::glXServerVersionString() const
{
 if (d->mCacheDirty) {
	updateConst();
 }
 return d->mGLXServerVersionString;
}

const QString& BoInfoGLCache::glXServerVendorString() const
{
 if (d->mCacheDirty) {
	updateConst();
 }
 return d->mGLXServerVendorString;
}

bool BoInfoGLCache::hasOpenGLVersion(unsigned int version) const
{
 return (openGLVersion() >= version);
}

bool BoInfoGLCache::hasOpenGLVersion(unsigned int major, unsigned int minor, unsigned int release) const
{
 return hasOpenGLVersion(MAKE_VERSION(major, minor, release));
}

const QString& BoInfoGLCache::openGLVersionString() const
{
 return d->mOpenGLVersionString;
}

const QString& BoInfoGLCache::openGLVendorString() const
{
 return d->mOpenGLVendorString;
}

const QString& BoInfoGLCache::openGLRendererString() const
{
 return d->mOpenGLRendererString;
}

unsigned int BoInfoGLCache::openGLVersion() const
{
 if (d->mCacheDirty) {
	updateConst();
 }
 return d->mOpenGLVersion;
}

void BoInfoGLCache::glXVersion(int* major, int* minor) const
{
 if (d->mCacheDirty) {
	updateConst();
 }
 *major = d->mGLXVersionMajor;
 *minor = d->mGLXVersionMinor;
}

int BoInfoGLCache::maxTextureSize() const
{
 if (d->mCacheDirty) {
	updateConst();
 }
 boDebug() << d->mMaxTextureSize << endl;
 return d->mMaxTextureSize;
}

bool BoInfoGLCache::supportsGenerateMipmap() const
{
 if (d->mCacheDirty) {
	updateConst();
 }
 return d->mSupportsGenerateMipmap;
}

bool BoInfoGLCache::supportsTextureCompressionS3TC() const
{
 if (d->mCacheDirty) {
	updateConst();
 }
 return d->mSupportsTextureCompressionS3TC;
}

bool BoInfoGLCache::supportsTexture3D() const
{
 if (d->mCacheDirty) {
	updateConst();
 }
 return d->mSupportsTexture3D;
}

bool BoInfoGLCache::supportsTextureCube() const
{
 if (d->mCacheDirty) {
	updateConst();
 }
 return d->mSupportsTextureCube;
}


