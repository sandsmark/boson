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

class QImage;

class BosonMap;
class BosonTextureArray;

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

	bool loadGroundTheme(BosonMap* map, QString dir);

	inline BosonTextureArray* textures() const
	{
		return mTextures;
	}

protected:
	/**
	 * @param dir The directory to load the image from. Including the theme
	 * name part (e.g. "earth"). Must end with a "/".
	 * @param groundType Which texture should be loaded. See @ref
	 * BosonMap::groundType
	 * @param amountOfLand See @ref BosonMap::amountOfLand. This can be used
	 * when the correct groundType can't be found in the theme, to pick a
	 * close replacement.
	 * @param amountOfWater See @ref BosonMap::amountOfWater. This can be used
	 * when the correct groundType can't be found in the theme, to pick a
	 * close replacement.
	 **/
	QImage loadTextureImage(const QString& dir, int groundType, unsigned char amountOfLand, unsigned char amountOfWater);

	bool loadGround(int j, const QString& path);

	/**
	 * @return a name (e.g. "desert") for the specified groundType. Note
	 * that this name is necesary for creating the file path of the tiles so
	 * don't change it.
	 **/
	static QString groundType2Name(int groundType);

private:
	BosonTextureArray* mTextures;

	QString mTilesDir;
};

#endif
