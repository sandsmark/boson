/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONTEXTUREARRAY_H
#define BOSONTEXTUREARRAY_H

#include <qvaluelist.h>
#include <qpointarray.h>
#include <GL/gl.h>

class QImage;
class BosonTextureArray
{
public:
	BosonTextureArray(QValueList<QImage> images, GLenum mode = GL_TEXTURE_2D);
	BosonTextureArray();

	/**
	 * Frees all textures
	 **/
	~BosonTextureArray();

	// AB: a hotspot of 0,0 (defalt) means that nothing should be changed
	// FIXME: hotspots in this file are nonsense. we need to use them
	// together with the vertices, not the textures.
	void setHotspots(QPointArray hotspots);
	QPoint hotspot(unsigned int i)
	{
		return (i >= mCount) ? QPoint(0,0) : mHotspots[i];
	}

	inline unsigned int count() const
	{
		return mCount;
	}

	bool isValid() const
	{
		return mTextures != 0;
	}

	/**
	 * @return The name of the texture specified or 0 if the texture array
	 * is not yet initialized.
	 **/
	inline GLuint texture(unsigned int i)
	{
		if (i >= mCount) {
			return 0;
		}
		return mTextures ? mTextures[i] : 0;
	}

	/**
	 * @param images The width and height all images should be 2^m, minimal
	 * 64x64. The size should not (for compatibility reasons) exceed
	 * 256x256. If the width or height do not meet the 2^m condition the
	 * image is scaled.
	 **/
	bool createTextures(QValueList<QImage> images, GLenum mode = GL_TEXTURE_2D);

	/**
	 * BosonTextureArray scales images if they don't fit the necessary sizes
	 * (i.e. 2^m). If you need the original sizes you can use this.
	 * @return The width of the original image of the ith texture
	 **/
	inline int width(unsigned int i) const
	{
		if (i >= mCount) {
			return 0;
		}
		return mWidths ? mWidths[i] : 0;
	}

	/**
	 * BosonTextureArray scales images if they don't fit the necessary sizes
	 * (i.e. 2^m). If you need the original sizes you can use this.
	 * @return The height of the original image of the ith texture
	 **/
	inline int height(unsigned int i) const
	{
		if (i >= mCount) {
			return 0;
		}
		return mHeights ? mHeights[i] : 0;
	}


private:
	/**
	 * Inspired by source code of Mesa-4.0.1 see teximage.c for the original
	 * implementation - function logbase2()
	 * @return n, if it is an axact power of 2. Otherwise The next bigger
	 * value. E.g. if you provide 48, this will return 64.
	 **/
	int nextPower2(int n) const;

	void init();

private:
	unsigned int mCount;
	GLuint* mTextures;
	int* mWidths;
	int* mHeights;
	QPointArray mHotspots;

	GLenum mMode;
};

#endif

