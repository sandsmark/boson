/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

class QImage;
class QPixmap;

class BosonMap;
class BosonTextureArray;

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

	BosonTextureArray* textures(int texture) const;

	/**
	 * Create a list of BosonGroundTheme objects by searching for
	 * index.ground files. You can retrieve all available objects from @ref
	 * BosonData.
	 **/
	static bool createGroundThemeList();

	/**
	 * @return The number of textures (groundTypes) available in this
	 * groundTheme.
	 **/
	unsigned int textureCount() const;

	QRgb miniMapColor(unsigned int texture) const;

	/**
	 * @return The name of the file (relative to the groundTheme directory)
	 * for @p texture.
	 **/
	QString textureFileName(unsigned int texture) const;

	/**
	 * @return The filename (relative to the groundTheme directory) of the pixmap
	 * for @p texture. This pixmap will be used in editor
	 **/
	QString texturePixmapFileName(unsigned int texture) const;

	int textureAnimationDelay(unsigned int texture) const;

	QPixmap pixmap(unsigned int texture);

protected:
	/**
	 * @param dir The directory to load the image from. Including the theme
	 * name part (e.g. "earth"). Should end with a "/".
	 * @param texture Which texture should be loaded. Must be < @ref
	 * textureCount
	 **/
	void loadTextureImages(const QString& dir, unsigned int texture);

private:
	BosonGroundThemePrivate* d;
	QIntDict<BosonTextureArray> mTextures;
	QIntDict<QPixmap> mPixmaps;
};

#endif
