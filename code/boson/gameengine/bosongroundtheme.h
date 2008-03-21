/*
    This file is part of the Boson game
    Copyright (C) 2003-2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONGROUNDTHEME_H
#define BOSONGROUNDTHEME_H

#include <qstring.h>
#include <qwindowdefs.h> // QRgb
#include <qintdict.h>

class QImage;
class QPixmap;
class QStringList;

class BosonMap;


class BosonGroundType
{
public:
	BosonGroundType();
	~BosonGroundType();

	int index;
	QString textureFile;
	QString bumpTextureFile;
	float bumpScale;
	float bumpBias;
	float textureSize;
	QString shaderFile;
	int animationDelay;
	QString name;
	// Maybe change to BoVector3Float?
	QRgb color;
	QString iconFile;
};

class BosonGroundThemePrivate;
/**
 * Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonGroundTheme
{
public:
	/**
	 * Create an empty BosonGroundTheme object.
	 *
	 * Use @ref loadGroundTheme to load a theme.
	 **/
	BosonGroundTheme();
	~BosonGroundTheme();

	const QString& identifier() const;

	const QString& themeDirectory() const;

	/**
	 * This should load all data from the config file (i.e. index.ground),
	 * including number and kind of textures, how much amount of land/water
	 * every groundType has and so on.
	 *
	 * These data can (and should) be used in boson even after the map has
	 * been played, i.e. the @ref BosonMap object that uses this groundTheme
	 * has been destroyed. The groundTheme should remain in memory - the
	 * config data uses only very little memory, but enables us to easily
	 * display and parse a complete list of available groundThemes.
	 *
	 * This method does <em>not</em> load any actual textures.
	 **/
	bool loadGroundThemeConfig(const QString& file);

	/**
	 * Create a list of BosonGroundTheme objects by searching for
	 * index.ground files. You can retrieve all available objects from @ref
	 * BosonData.
	 **/
	static bool createGroundThemeList();

	/**
	 * @return A list of index.ground files available on the system. This is
	 * used to load the ground themes in @ref createGroundThemeList.
	 * Usually you don't need this method but rather @ref
	 * BosonData::groundTheme.
	 **/
	static QStringList groundThemeFiles();

	/**
	 * @return The number of groundTypes available in this
	 * groundTheme.
	 **/
	unsigned int groundTypeCount() const;

	BosonGroundType* groundType(unsigned int i) const;

private:
	BosonGroundThemePrivate* d;
	QIntDict<BosonGroundType> mGroundTypes;
};

#endif
