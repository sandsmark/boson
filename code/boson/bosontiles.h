/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONTILES_H
#define BOSONTILES_H

#include "cell.h"

#include <qobject.h>
#include <qmap.h>

class QImage;
class QPixmap;
class QString;

class BosonTextureArray;

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonTiles : public QObject
{
Q_OBJECT
public:
	/**
	 * Create an empty BosonTiles object. You want to use this if you are
	 * going to create a pixmap to be used in Boson.
	 *
	 * Use @ref loadTiles to write the single tiles into the pixmap and @ref
	 * save to save the pixmap.
	 **/
	BosonTiles();

	~BosonTiles();

/*
	QPixmap plainTile(Cell::GroundType type);

	QPixmap big1(int bigNo, Cell::TransType trans, bool inverted); // bigNo = 0..4

	QPixmap big2(int bigNo, Cell::TransType trans, bool inverted); // bigNo = 0..4

	QPixmap small(int smallNo, Cell::TransType trans, bool inverted);
	*/
	
	// call this like the original fillGroundPixmap() in editorTopLevel.cpp
	QPixmap tile(int g);

	/**
	 * Load tiles from dir. This creates a pixmap which can be used as a
	 * ground in boson from a lot of small pixmaps (tiles).
	 *
	 * Don't use this if youre actually playing boson but just to create
	 * the pixmap.
	 * @param dir The directory where all the small tiles are for the
	 * pixmap. See data/themes/grounds/README
	 * @param debug Generate a normal pixmap if FALSE or one with a frame
	 * around every tile if TRUE
	 **/
	bool loadTiles(QString dir, bool debug = false);

	/**
	 * Save a pixmap created using @ref loadTiles.
	 **/
	bool save(const QString& fileName);

	QPixmap pixmap() const;

	/**
	 * Call this to create the texture objects. Must be called from a valid
	 * context (i.e. call QGLWidget::makeCurrent or call it from one of the
	 * standard QT-GL methods). At least afaik...
	 **/
	void generateTextures();

	inline BosonTextureArray* textures() const 
	{
		return mTextures;
	}

protected:
	static int big_x(int g);
	static int big_y(int g);

	static int big_w();
	static int big_h();

	bool loadGround(int j, const QString& path);
	bool loadTransition(const QString& dir, int gt);

	void putOne(int z, QImage& p, int xoffset = 0, int yoffset = 0);

	/**
	 * @return a name (e.g. "desert") for the specified groundType. Note
	 * that this name is necesary for creating the file path of the tiles so
	 * don't change it.
	 **/
	static QString groundType2Name(Cell::GroundType t);
	static QString transition2Name(Cell::TransType t);
	static QString trans_ext(int t);

signals:
	void signalTilesLoading(int);
	void signalTilesLoaded();

private:
	QImage* mTilesImage;
	QMap<int, QImage> mTextureImages;
	BosonTextureArray* mTextures;

	bool mDebug; // used in putOne()
	int mLoaded; // Number of tiles loaded
};

#endif
