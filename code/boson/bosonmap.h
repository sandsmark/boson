/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONMAPDOM_H
#define BOSONMAPDOM_H

#include <qobject.h>

class Cell;
class UnitBase;
class Boson;
class QStringList;
class QDataStream;
class BosonTiles;

/**
 * This class represents a boson map. It is part of a @ref BosonPlayField (a
 * .bpf file) and gets stored on disk as binary.
 *
 * The map consists of cells (rectangles of fixed width/height). There are @ref
 * width x @ref height cells on a map, you can retrieve a cell using @ref cell.
 * See also @ref Cell class.
 *
 * Another (small) part of the map is the height map. This file (stored as .png
 * on disk) specifies the height of a corner of a cell and is used to represent
 * 3d terrain.
 *
 * @short Representation of maps and cells in boson.
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonMap : public QObject
{
	Q_OBJECT
public:
	BosonMap(QObject* parent = 0);
	~BosonMap();

	/**
	 * @return vertical cell count
	 **/
	inline unsigned int height() const { return mMapHeight; }

	/**
	 * @return Horizontal cell count
	 **/
	inline unsigned int width() const { return mMapWidth; }

	/**
	 * This function does error checking on @p x, @p y and will return NULL
	 * if they are invalid. If you don't want this error checking you can
	 * use @ref cells + @ref cellArrayPos instead, but you should prefer
	 * this one!
	 * @param x x-coordinate of the requested cell. Must be 0 < x < @ref
	 * width
	 * @param y y-coordinate of the requested cell. Must be 0 < y < @ref
	 * height
	 * @return The cell at x,y (in cell positions!) or NULL if the
	 * coordinates have been invalid.
	 **/
	Cell* cell(int x, int y) const;

	/**
	 * If you <em>really</em> need some speedup and can't use @ref cell you
	 * may want to use this one. @ref cells + cellArrayPos(x,y) will return
	 * the same cell as @ref cell as long as @p x and @p y are valid for
	 * this map. You need to ensure that on your own. (e.g. using @ref
	 * isValidCell)
	 *
	 * You are meant to prefer @ref cell.
	 * @return The position of the cell at @p x, @p y in the array @p cells
	 * if valid. Note that NO error checking will be done!! You should
	 * prefer @ref cell if possible
	 **/
	inline int cellArrayPos(int x, int y) const
	{
		return x + y * width();
	}

	/**
	 * @return Whether x,y are valid cell positions for this map.
	 **/
	inline bool isValidCell(int x, int y) const
	{
		if (x < 0 || (unsigned int)x >= width()) {
			return false;
		}
		if (y < 0 || (unsigned int)y >= height()) {
			return false;
		}
		return true;
	}

	bool loadHeightMapImage(const QByteArray&);

	bool saveMapToFile(QDataStream& stream);
	bool saveMap(QDataStream& stream);
	bool loadMapFromFile(QDataStream& stream);
	bool loadMap(QDataStream& stream);

	QByteArray saveHeightMapImage();

	/**
	 * @return TRUE if the current map is valid i.e. can be transmitted
	 * savely using @ref saveMapGeo
	 **/
	bool isValid() const;

	/**
	 * You should rather use @ref cell ! If you use this, you need to ensure
	 * yourself, that all cells you want ta access are actually valid.
	 * @return The internal cell array
	 **/
	inline Cell* cells() const { return mCells; }

	void setModified(bool m) { mModified = m; }
	bool modified() const { return mModified; }

	/**
	 * Tiles are loaded externally, so they need to be placed into the map.
	 *
	 * See BosonCanvas, where tile loading takes place (currently)
	 **/
	void setTileSet(BosonTiles* t);
	BosonTiles* tileSet() const { return mTiles; }

	/**
	 * Load the tileset - see @ref BosonTiles
	 *
	 * Note that the actual loading happens in @ref slotLoadTiles using
	 * a @ref QTimer::singleShot. This gives us a non-blocking UI as we can
	 * use @ref QApplication::processEvents
	 * @param tiles currently always "earth.png", currently
	 * @param withtimer whether to use timer
	 **/
	void loadTiles(const QString& tiles, bool withtimer = true);

	/**
	 * Resize the map. Currently this works only if the cells have
	 * <em>not</em> yet been created.
	 **/
	void resize(unsigned int width, unsigned int height);

	/**
	 * Fill all cells with @p ground.
	 * @param ground See @ref Cell::GroundType
	 **/
	void fill(int ground);

	inline float* heightMap() const { return mHeightMap; }

	/**
	 * Note that you can specify (width() + 1) * (height() + 1) corners here!
	 * I.e. if you do this
	 * <pre>
	 * heightAtCorner(width(), height())
	 * </pre>
	 * it will be valid (althought cell() would return NULL). This example
	 * would return the height of the <em>lower right</em> corner of the
	 * cell at (width()-1, height()-1), i.e. the cell in the lower right
	 * corner of the map.
	 * @return The height of the upper left corner of the cell at @p x, @p
	 * y or 1.0 if invalid coordinates were specified.
	 **/
	float heightAtCorner(int x, int y) const;
	void setHeightAtCorner(int x, int y, float height);

	float cellAverageHeight(int x, int y);

public slots:
	void slotChangeCell(int x, int y, int groundType, unsigned char b);

signals:
	void signalTileSetChanged(BosonTiles*);

	/**
	 * See @ref BosonTiles::signalTilesLoading
	 **/
	void signalTilesLoading(int tiles);

	/**
	 * See @ref BosonTiles::signalTilesLoaded
	 **/
	void signalTilesLoaded();

protected:
	bool loadCell(QDataStream& stream, int& groundType, unsigned char& b);

	void saveCell(QDataStream& stream, int groundType, unsigned char b);

	/**
	 * Save the map geo into stream. This creates a stream in the format
	 * used by @ref loadMapGeo. You can use this to send the map geo to
	 * another client.
	 *
	 * Note that this doesn't add all the units of the player but just the
	 * basic settings of a map! The units should be loaded by the server
	 * only.
	 * @param stream The stream to write to
	 **/
	bool saveMapGeo(QDataStream& stream);
	bool saveCells(QDataStream& stream);
	bool saveHeightMap(QDataStream& stream);

	/**
	 * Read the map geo from stream. This only reads map size, playercount
	 * and something like this. Use @ref loadCells to load the cells.
	 *
	 * This does <em>not</em> read the units
	 * of the player. Usually you will transmit the geo parts of a map (see
	 * @ref saveMapGeo) to all clients but only the server loads the units
	 * which will be added.
	 **/
	bool loadMapGeo(QDataStream& stream);

	/**
	 * Load the cells from the stream.
	 * @param stream The stream to read from
	 **/
	bool loadCells(QDataStream& stream);

	bool loadHeightMap(QDataStream& stream);


	static float pixelToHeight(int p);
	static int heightToPixel(float height);

protected slots:
	/**
	 * Load the tileset that has been specified using @ref loadTiles. We use
	 * this slot to provide a non-blocking tile loading.
	 **/
	void slotLoadTiles();

private:
	void init();

private:
	class BosonMapPrivate;
	BosonMapPrivate* d;
	Cell* mCells;
	float* mHeightMap;
	bool mModified;

	unsigned int mMapWidth;
	unsigned int mMapHeight;

	BosonTiles* mTiles;
};

#endif
