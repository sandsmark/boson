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
#ifndef BOSONMODEL_H
#define BOSONMODEL_H

#include <qmap.h>
#include <GL/gl.h>

#include <lib3ds/types.h>

class BosonTextureArray;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 */
class BosonModel
{
public:
	/**
	 * Construct a model using an already created display list
	 **/
	BosonModel(GLuint list, int width, int height);
	BosonModel(const QString& dir, const QString& file);
	~BosonModel();

	void setFrame(unsigned int frame);
	unsigned int frame() const { return mFrame; }

	/**
	 * @return The display list of the current @ref frame.
	 **/
	inline GLuint displayList() const 
	{
		return mDisplayList;
	}

	//FIXME!!
	//FIXME: do we need frame at all? we already demand all frames to be at
	//the same size
	inline int width(int /*frame*/) const { return mWidth; }
	inline int height(int /*frame*/) const { return mHeight; }
	inline unsigned int frames() const { return mFrames.count(); }

protected:
	void loadTextures();
	void createDisplayLists();
	void renderNode(Lib3dsNode* node);

	/**
	 * Convert a 3ds texture name to a clean name. That means call
	 * QString::lower() on it, currently.
	 **/
	QString cleanTextureName(const char*);
	const QString& baseDirectory() const { return mDirectory; }
	QString textureDirectory() const;

private:
	void init();

private:
	GLuint mDisplayList;
	Lib3dsFile* m3ds;
	QMap<QString, GLuint> mTextures;
	QMap<int, GLuint> mFrames;
	BosonTextureArray* mTextureArray;
	QString mDirectory;
	unsigned int mFrame;

	// warning! width and height are still in canvas sizes!
	int mWidth;
	int mHeight;

	int mPoints; // for debugging
};
#endif

