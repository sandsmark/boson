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
#ifndef BOBTFLOAD_H
#define BOBTFLOAD_H

#include <qstring.h>
#include <qvaluevector.h>

class QImage;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 */
class BoBTFLoad
{
public:
	BoBTFLoad(const QString& file);
	~BoBTFLoad();

	bool loadTexture();

	/**
	 * @return The absolute filename to the original texture file
	 **/
	const QString& file() const;

	unsigned int mipmapLevels() const
	{
		return mLevels;
	}
	char* data(unsigned int level = 0) const
	{
		if (level >= mLevels) {
			return 0;
		}
		return mTextureLevelData[level];
	}

	int width(unsigned int level = 0) const
	{
		if (level >= mLevels) {
			return 0;
		}
		return mTextureLevelWidth[level];
	}
	int height(unsigned int level = 0) const
	{
		if (level >= mLevels) {
			return 0;
		}
		return mTextureLevelHeight[level];
	}


protected:
	bool loadInfo(QDataStream& stream);
	bool loadTextureLevel(QDataStream& stream);

	QString createBTFFile();

	bool createBTFFile(const QImage&, QDataStream& stream) const;
	bool saveTextureLevel(unsigned int level, const QImage& img, QDataStream&) const;

private:
	QString mFile;
	QString mMD5;

	unsigned int mLevels;

	QValueVector<char*> mTextureLevelData;
	QValueVector<int> mTextureLevelWidth;
	QValueVector<int> mTextureLevelHeight;
};

#endif

