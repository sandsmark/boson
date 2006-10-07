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
#ifndef BOINFOGLCACHE_H
#define BOINFOGLCACHE_H

#include <qstring.h>

class BoInfo;
class QStringList;

class BoInfoGLCachePrivate;
class BoInfoGLCache
{
public:
	BoInfoGLCache(BoInfo*);

	void update();

	void setDirty(bool);

	/**
	 * Note that the list is cached internally, so no lookup is required
	 * when calling this method.
	 *
	 * @return A list of available OpenGL extensions.
	 **/
	const QStringList& openGLExtensions() const;

	/**
	 * @return A string describing the GLU version.
	 **/
	const QString& gluVersionString() const;

	const QStringList& gluExtensions() const;

	void glXVersion(int* major, int* minor) const;
	const QString& glXClientVersionString() const;
	const QString& glXClientVendorString() const;
	const QStringList& glXClientExtensions() const;
	const QString& glXServerVersionString() const;
	const QString& glXServerVendorString() const;
	const QStringList& glXServerExtensions() const;

	/**
	 * @return TRUE if the rendering context is direct (should be so, as
	 * non-direct is slow)
	 **/
	bool isDirect() const;

	/**
	 * Note that we don't have the OpenGL version available boson was
	 * compiled with (at least not yet)!
	 * @return A string containing the OpenGL version (with both, vendor
	 * version such as mesa 3.2 and OpenGL version such as 1.1).
	 **/
	const QString& openGLVersionString() const;

	/**
	 * @return Runtime OpenGL version which is encoded using MAKE_VERSION macro.
	 **/
	unsigned int openGLVersion() const;

	/**
	 * @return Whether runtime OpenGL version is equal or greater than given
	 *  version.
	 * vesion must be encoded using MAKE_VERSION macro.
	 **/
	bool hasOpenGLVersion(unsigned int version) const;

	/**
	 * @return Whether runtime OpenGL version is equal or greater than given
	 *  version.
	 **/
	bool hasOpenGLVersion(unsigned int major, unsigned int minor, unsigned int release) const;

	/**
	 * @return A string containing the vendor of the installed OpenGL
	 * version (runtime). Usually mesa or nvidia.
	 **/
	const QString& openGLVendorString() const;

	const QString& openGLRendererString() const;

	int maxTextureSize() const;
	int maxTextureUnits() const;
	int maxCubeMapTextureSize() const;
	int max3DTextureSize() const;
	int maxTextureMaxAnisotropy() const;
	bool supportsGenerateMipmap() const;
	bool supportsTextureCompressionS3TC() const;
	bool supportsTexture3D() const;
	bool supportsTextureCube() const;
	bool supportsNPOTTextures() const;

protected:
	int getInt(int key) const;
	unsigned int getUInt(int key) const;
	QString getString(int key) const;
	bool getBool(int key) const;

	/**
	 * Calls @ref update. Note that the const modifier will be discarded,
	 * the cache is actually being updated.
	 **/
	void updateConst() const;

private:
	BoInfoGLCachePrivate* d;
	BoInfo* mInfo;
};

#endif

