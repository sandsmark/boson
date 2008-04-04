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
#ifndef BOSONGROUNDTHEMEDATA_H
#define BOSONGROUNDTHEMEDATA_H

#include <qstring.h>
#include <qwindowdefs.h> // QRgb
#include <qintdict.h>
#include <qdict.h>

class QImage;
class QPixmap;

class BosonMap;
class BosonGroundTheme;
class BosonGroundType;
class BoTextureArray;
class BoShader;
class BoTexture;


class BosonGroundTypeData
{
public:
	BosonGroundTypeData();
	~BosonGroundTypeData();

	BoTexture* currentTexture(int advanceCallsCount) const;
	BoTexture* currentBumpTexture(int advanceCallsCount) const;

	BoTextureArray* textures;
	BoTextureArray* bumpTextures;
	BoShader* shader;
	QPixmap* icon;

	const BosonGroundType* groundType;
};

class BosonGroundThemeDataPrivate;
/**
 * Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonGroundThemeData
{
public:
	/**
	 * Create an empty BosonGroundTheme object.
	 *
	 * Use @ref loadGroundTheme to load a theme.
	 **/
	BosonGroundThemeData();
	~BosonGroundThemeData();

	/**
	 * This loads all data that are necessary to use this groundTheme.
	 **/
	bool loadGroundTheme(const BosonGroundTheme* theme);

	/**
	 * @return The @ref BosonGroundTheme object as given to @ref
	 * loadGroundTheme
	 **/
	const BosonGroundTheme* groundTheme() const;

	/**
	 * @return See @ref BosonGroundTheme::identifier
	 **/
	const QString& identifier() const;

	/**
	 * @return See @ref BosonGroundTheme::themeDirectory.
	 *
	 * Your probably do not need this method - it is mainly used internally.
	 **/
	const QString& themeDirectory() const;

	/**
	 * @return See @ref BosonGroundTheme::groundTypeCount
	 **/
	unsigned int groundTypeCount() const;

	/**
	 * @return See @ref BosonGroundTheme::groundType
	 **/
	const BosonGroundType* groundType(unsigned int i) const;

	/**
	 * @return The @ref BosonGroundTypeData object matching the @ref
	 * BosonGroundType object with the same index @p i.
	 **/
	BosonGroundTypeData* groundTypeData(unsigned int i) const;

	static bool shadersSupported();
	static void setUseGroundShaders(bool use);

protected:
	/**
	 * @param dir The directory to load the image from. Including the theme
	 * name part (e.g. "earth"). Should end with a "/".
	 * @param groundtype Which groundtype should be loaded. Must be < @ref
	 * groundTypeCount
	 **/
	bool loadTextures(const QString& dir, unsigned int groundtype);

	void loadShaders(const QString& dir, BosonGroundTypeData* ground);

private:
	BosonGroundThemeDataPrivate* d;
};

#endif
