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
class BosonGroundTheme;
class BosonTextureArray;

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

	/**
	 * Load an image from @p buffer and convert it to the internal
	 * heightmap format of @ref BosonMap.
	 *
	 * This function can be used to import a height map in the editor or
	 * (more important) to load the heightmap from a .bpf file (where it is
	 * stored as a png image).
	 **/
	bool loadHeightMapImage(const QByteArray& buffer);

	bool importHeightMapImage(const QImage& image);

	bool saveMapToFile(QDataStream& stream);

	/**
	 * Save the complete map into the stream, even the parts that are
	 * usually stored in different files, such as the height map.
	 *
	 * Use this to send the map over network, but remember that there will
	 * be a lot of data! (probably more a few MB for 500x500 maps!)
	 **/
	bool saveCompleteMap(QDataStream& stream);

	/**
	 * Load the "main" map, i.e. the map geo (it's size) and it's cells from
	 * the file on disk. This will not load things such as height map, which
	 * are in a different file.
	 **/
	bool loadMapFromFile(const QByteArray& map);

	/**
	 * Load the complete map, even those data that are stored in different
	 * files in the .bpf file. This is e.g. the height map.
	 *
	 * The stream must have been creates using @ref saveCompleteMap,
	 * <em>not</em> @ref saveMapToFile!
	 **/
	bool loadCompleteMap(QDataStream& stream);

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
	 * Load a @ref groundTheme - see @ref BosonGroundTheme. The groundTheme
	 * contains the actual textures used by the @ref texMap.
	 *
	 * @param theme The groundTheme to be loaded, such as "earth"
	 **/
	void loadGroundTheme(const QString& theme);
	BosonGroundTheme* groundTheme() const { return mGroundTheme; }

	BosonTextureArray* textures() const;

	/**
	 * Resize the map. Currently this works only if the cells have
	 * <em>not</em> yet been created.
	 **/
	void resize(unsigned int width, unsigned int height);

	/**
	 * Fill the @ref texMap with 100% of @p texture.
	 **/
	void fill(unsigned int texture);

	inline float* heightMap() const { return mHeightMap; }

	/**
	 * Note that you can specify (width() + 1) * (height() + 1) corners here!
	 * See also @ref cornerArrayPos for detailed description of possible
	 * values.
	 * @return The height of the upper left corner of the cell at @p x, @p
	 * y or 1.0 if invalid coordinates were specified.
	 **/
	float heightAtCorner(int x, int y) const;
	void setHeightAtCorner(int x, int y, float height);

	float cellAverageHeight(int x, int y);

	/**
	 * @return The number of different textures used in this map. See also
	 * @ref texMap
	 **/
	unsigned int textureCount() const { return mTextureCount; }

	/**
	 * @return The internal texmap array, which defines how much percent
	 * of every texture (see also @ref textureCount) are used in the corners
	 * of the cells.
	 * @param texture This defines which values should be returned. Use 0
	 * for the entire texMap, or use a value less than @ref textureCount
	 * to get the values for a certain texture (0 is also the first
	 * texture).
	 **/
	inline unsigned char* texMap(int texture = 0) const
	{
		return (mTexMap + texture * (width() + 1) * (height() + 1));
	}

	/**
	 * @param x The x-coordinate of the left corner that is wanted.
	 * @param y The y-coordinate of the top corner that is wanted.
	 * @param texture The index/number of the texture (grass, desert, water,
	 * ...) that is wanted.
	 **/
	inline int texMapArrayPos(unsigned int texture, int x, int y) const
	{
		return ((texture) * (width() + 1) * (height() + 1) + cornerArrayPos(x, y));
	}

	inline unsigned char texMapAlpha(unsigned int texture, int x, int y) const
	{
		return mTexMap[texMapArrayPos(texture, x, y)];
	}

	/**
	 * @return @ref BosonGroundTheme::miniMapColor
	 **/
	QRgb miniMapColor(unsigned int texture) const;

	/**
	 * Note that you can specify (width() + 1) * (height() + 1) corners here!
	 * I.e. if you do this
	 * <pre>
	 * int pos = cornerArrayPos(width(), height())
	 * </pre>
	 * it will be valid (althought cell() would return NULL). This example
	 * would return the position of the <em>lower right</em> corner of the
	 * cell at (width()-1, height()-1), i.e. the cell in the lower right
	 * corner of the map.
	 *
	 * @param x The x-coordinate of the wanted corner. Note that counting
	 * begins at top-left, i.e. 0 is the left corner of the leftmost cell
	 * and @ref width is the right corner of the rightmost cell.
	 * @param y The y-coordinate of the wanted corner. Note that counting
	 * begins at top-left, i.e. 0 is the top corner of the topmost cell
	 * and @ref height is the bottom corner of the bottom cell.
	 * @return The array-coordinates of the corner at x,y. This array
	 * position can be used for all map-arrays which are based on corners
	 * (currently that are height map and texmap).
	 **/
	inline int cornerArrayPos(int x, int y) const
	{
		return x + y * (width() + 1);
	}

	/**
	 * Load the @ref texMap from @p stream.
	 *
	 * If there is no data in this stream then this function will (try to)
	 * generate a texmap according to the @ref Cell::groundType of the
	 * already loaded cells. The default number of textures is used then.
	 **/
	bool loadTexMap(QDataStream& stream);
	bool saveTexMap(QDataStream& stream);

	/**
	 * Import a texmap from an image.
	 **/
	bool importTexMap(const QImage* image, int texturesPerComponent = 1, bool useAlpha = false);
	bool importTexMap(const QString& file, int texturesPerComponent = 1, bool useAlpha = false);

	static float pixelToHeight(int p);
	static int heightToPixel(float height);

	bool generateCellsFromTexMap();

	/**
	 * @return The file format version of the map, that is used when
	 * saving a map file (see @ref saveMapToFile).
	 **/
	static int mapFileFormatVersion();

public slots:
	void slotChangeCell(int x, int y, unsigned char amountOfLand, unsigned char amountOfWater);

	/**
	 * Change the alpha value of the texmap at @þ x, @p y for @þ texture.
	 *
	 * This should <em>not</em> be used when initializing the map, as it
	 * will also update all 4 adjacent cells.
	 *
	 * Also note that it must <em>no</em> (!!!) be called in gameMode at
	 * all, also because the 4 adjacent cells are updated (see @ref
	 * recalculateCell).
	 *
	 * This slot is meant for editor use only.
	 *
	 * @param textureCount The numbe of textures in @p textures and @p
	 * alpha.
	 * @param textures Indices of the textures used in @p alpha, as they
	 * will get used in @ref BosonGroundTheme
	 **/
	void slotChangeTexMap(int x, int y, unsigned int textureCount, unsigned int* textures, unsigned char* alpha);

signals:
	void signalGroundThemeChanged(BosonGroundTheme*);

protected:
	bool loadCell(QDataStream& stream, unsigned char* amountOfLand, unsigned char* amountOfLand) const;

	void saveCell(QDataStream& stream, unsigned char amountOfLand, unsigned char amountOfWater);

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
	bool saveGroundTheme(QDataStream& stream);
	bool saveCells(QDataStream& stream);
	bool saveHeightMap(QDataStream& stream);

	static bool saveHeightMap(QDataStream& stream, unsigned int mapWidth, unsigned int mapHeight, float* heightMap);

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
	 * Load the groundTheme from the stream. Note that we "load" the
	 * groundTheme identifier only, not the complete theme. The actual theme
	 * will be on the disk, in the grounds/ dir. The config of that theme is
	 * already in memory, but the actual theme will be loaded later.
	 * @return TRUE on success, FALSE otherwise (e.g. if the requested
	 * groundTheme is not available)
	 **/
	bool loadGroundTheme(QDataStream& stream);

	/**
	 * Load the cells from the stream.
	 * @param stream The stream to read from
	 **/
	bool loadCells(QDataStream& stream);

	bool loadHeightMap(QDataStream& stream);

	/**
	 * Recalculate the amountOfLand/Water values for the cell at @þ x,@p y.
	 *
	 * These values are based on the alpha values in the textures at all 4
	 * corners of the cell.
	 *
	 * This should be called on map construction only (see @ref 
	 * generateCellsFromTexMap), except in editor mode.
	 *
	 * Also note that the calculated values for this cell are <em>not</em>
	 * fully network safe! The ADMIN will calculate these values and send
	 * them out through network (see @ref loadCompleteMap), so that all
	 * clients will use the same values.
	 **/
	void recalculateCell(int x, int y);

	/**
	 * Create the map array accordint to @ref width and @ref height.
	 **/
	void createCells();

private:
	void init();

private:
	class BosonMapPrivate;
	BosonMapPrivate* d;
	Cell* mCells;
	float* mHeightMap;
	unsigned int mTextureCount;
	unsigned char* mTexMap;
	bool mModified;

	unsigned int mMapWidth;
	unsigned int mMapHeight;

	BosonGroundTheme* mGroundTheme;
};

#endif
