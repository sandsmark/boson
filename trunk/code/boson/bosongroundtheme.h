/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONGROUNDTHEME_H
#define BOSONGROUNDTHEME_H

#include <qstring.h>
#include <qwindowdefs.h> // QRgb
#include <qintdict.h>
#include <qdict.h>

class QImage;
class QPixmap;

class BosonMap;
class BoTextureArray;
class BoShader;
class BoTexture;


class BosonGroundType
{
public:
	BosonGroundType();
	~BosonGroundType();

	int id;
	BoTextureArray* textures;
	QString texturefile;
	BoTextureArray* bumptextures;
	QString bumptexturefile;
	float bumpscale;
	float bumpbias;
	float texturesize;
	QString shaderfile;
	BoShader* shader;
	int animationDelay;
	QString name;
	// Maybe change to BoVector3Float?
	QRgb color;
	// TODO: remove!
	QPixmap* icon;
	QString iconfile;
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
	 * This loads all data that are necessary to use this groundTheme.
	 * Before you can call this you must first load the config file, see
	 * @ref loadGroundThemeConfig.
	 **/
	// AB: the textures should be unloaded after the map has been played (at
	// least if a different theme is used for the next map)
	bool loadGroundTheme(QString dir);

	/**
	 * Create a list of BosonGroundTheme objects by searching for
	 * index.ground files. You can retrieve all available objects from @ref
	 * BosonData.
	 **/
	static bool createGroundThemeList();

	/**
	 * @return The number of groundTypes available in this
	 * groundTheme.
	 **/
	unsigned int groundTypeCount() const;

	BosonGroundType* groundType(unsigned int i) const;

	static bool shadersSupported();
	static void setUseGroundShaders(bool use);

protected:
	/**
	 * @param dir The directory to load the image from. Including the theme
	 * name part (e.g. "earth"). Should end with a "/".
	 * @param groundtype Which groundtype should be loaded. Must be < @ref
	 * groundTypeCount
	 **/
	void loadTextures(const QString& dir, unsigned int groundtype);

	void loadShaders(const QString& dir, BosonGroundType* ground);

private:
	BosonGroundThemePrivate* d;
	QIntDict<BosonGroundType> mGroundTypes;
	QDict<BoTexture> mBumpTextures;
	QDict<BoShader> mShaders;
};

#endif
