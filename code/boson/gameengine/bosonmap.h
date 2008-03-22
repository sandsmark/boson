/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2008 Andreas Beckermann (b_mann@gmx.de)
    Copyright (C) 2001-2006 Rivo Laks (rivolaks@hot.ee)

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
#ifndef BOSONMAPDOM_H
#define BOSONMAPDOM_H

#include <qobject.h>
#include <qrect.h>

#include "../bo3dtools.h"

class Cell;
class UnitBase;
class Boson;
class QStringList;
class QDataStream;
class BosonGroundTheme;
class BoTexture;
class BosonGroundType;
class BosonMap;
class BoGroundQuadTreeNode;
class BoLake;
template<class T> class QDict;
template<class T> class QValueList;
template<class T> class QPtrList;
template<class T1, class T2> class QPair;

/**
 * @short abstract class for data operating of the corners of the cells (like
 * heightmap and texmap)
 **/
class BoMapCornerArray
{
public:
	/**
	 * @param width Number of corners (i.e. mapwidth + 1)
	 * @param width Number of corners (i.e. mapheight + 1)
	 **/
	BoMapCornerArray(unsigned int width, unsigned int height)
	{
		mWidth = width;
		mHeight = height;
	}
	virtual ~BoMapCornerArray()
	{
	}

	virtual bool save(QDataStream& stream) const = 0;
	virtual bool load(QDataStream& stream) = 0;

	/**
	 * @return The width of this array, i.e. the number of corners (@ref
	 * BosonMap::width + 1) of this map.
	 **/
	inline unsigned int width() const
	{
		return mWidth;
	}
	/**
	 * @return The height of this array, i.e. the number of corners (@ref
	 * BosonMap::height + 1) of this map.
	 **/
	inline unsigned int height() const
	{
		return mHeight;
	}

	/**
	 * @return The index of the corner @p x, @p y in the internal array.
	 **/
	inline int arrayPos(int x, int y) const
	{
		return x + y * width();
	}

	/**
	 * @overload
	 *
	 * This is a static version of the function above.
	 * @param _width The number of corners. This is usually @ref
	 * BosonMap::width + 1. See also @ref width.
	 **/
	inline static int arrayPos(int x, int y, int _width)
	{
		return x + y * _width;
	}

private:
	unsigned int mWidth;
	unsigned int mHeight;
};

class BoHeightMap: public BoMapCornerArray
{
public:
	BoHeightMap(unsigned int width, unsigned int height)
			: BoMapCornerArray(width, height)
	{
		mHeightMap = new float[arrayPos(width - 1, height - 1) + 1];
		fill(0.0f);
	}

	virtual ~BoHeightMap()
	{
		delete[] mHeightMap;
	}

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);

	inline float* heightMap() const { return mHeightMap; }

	/**
	 * @return The height at @p x, @þ y. This function is safe, i.e. if @þ x
	 * or @p y are invalid we won't crash.
	 **/
	float heightAt(int x, int y) const
	{
		if (x < 0 || y < 0) {
			return 1.0f;
		}
		if ((unsigned int)x >= width() || (unsigned int)y >= height()) {
			return 1.0f;
		}
		return mHeightMap[arrayPos(x, y)];
	}

	void setHeightAt(int x, int y, float h)
	{
		if (x < 0 || y < 0) {
			return;
		}
		if ((unsigned int)x >= width() || (unsigned int)y >= height()) {
			return;
		}
		// AB: see pixelToHeight() for explanation on these
		// restrictions
		h = QMIN(h, 18.75f);
		h = QMAX(h, -13.125f);
		mHeightMap[arrayPos(x, y)] = h;
	}

	/**
	 * Fill the entire heightMap with @p height
	 **/
	void fill(float h)
	{
		for (unsigned int x = 0; x < width(); x++) {
			for (unsigned int y = 0; y < height(); y++) {
				setHeightAt(x, y, h);
			}
		}
	}

	/*+
	 * Convert the pixel value @p into a height (floating point value from
	 * -10.5 to 15.0). 0.0 is zero height (i.e. the default).
	 **/
	static float pixelToHeight(int p);

	/**
	 * Convert a @p height into a pixel value. See also @ref pixelToHeight.
	 **/
	static int heightToPixel(float height);

private:
	float* mHeightMap;
};

class BoTexMap : public BoMapCornerArray
{
public:
	/**
	 * Create a new texMap array for width x height corners and @p
	 * textureCount different textures.
	 *
	 * The array is initialized to 255 for the first texture and to 0 for
	 * all other textures.
	 **/
	BoTexMap(unsigned int textureCount, unsigned int width, unsigned int height)
			: BoMapCornerArray(width, height)
	{
		mTexMap = new unsigned char[texMapArrayPos(textureCount - 1, width - 1, height - 1) + 1];
		mTextureCount = textureCount;
		initialize(0, 255);
		for (unsigned int i = 1; i < textureCount; i++) {
			initialize(i, 0);
		}
	}
	~BoTexMap()
	{
		delete[] mTexMap;
	}

	virtual bool save(QDataStream& stream) const;
	virtual bool load(QDataStream& stream);

	/**
	 * Import the @p image into this array. @ref textureCount must be at
	 * least 3, if you want the alpha buffer to be imported then at least 4.
	 *
	 * The red component is applied to the first texture, green to the second
	 * and blue to the third. If the image has an alpha component and @ref
	 * textureCount is at least 4, then alpha is applied to the fourth
	 * texture.
	 **/
	bool importTexMap(const QImage* image);

	/**
	 * Copy the texmap for @p dstTexture from the BoTexMap object @p src.
	 * The texture is named @p srcTexture in @p src.
	 **/
	bool copyTexture(unsigned int dstTexture, const BoTexMap* src, unsigned int srcTexture);

	unsigned int textureCount() const
	{
		return mTextureCount;
	}

	/**
	 * @return The internal texmap array, which defines how much percent
	 * of every texture (see also @ref BosonMap::textureCount) are used in the corners
	 * of the cells.
	 * @param texture This defines which values should be returned. Use 0
	 * for the entire texMap, or use a value less than @ref textureCount
	 * to get the values for a certain texture (0 is also the first
	 * texture).
	 **/
	inline unsigned char* texMap(unsigned int texture = 0) const
	{
		return (mTexMap + texMapArrayPos(texture, 0, 0));
	}

	/**
	 * @param x The x-coordinate of the left corner that is wanted.
	 * @param y The y-coordinate of the top corner that is wanted.
	 * @param texture The index/number of the texture (grass, desert, water,
	 * ...) that is wanted.
	 **/
	inline int texMapArrayPos(unsigned int texture, int x, int y) const
	{
		return texMapArrayPos(texture, x, y, width(), height());
	}

	/**
	 * @overload
	 *
	 * This is a static version of the function above.
	 **/
	inline static int texMapArrayPos(unsigned int texture, int x, int y, int width, int height)
	{
		return ((texture) * width * height + arrayPos(x, y, width));
	}

	/**
	 * @return The alpha value in the texMap for @p texture at @p x, @p y.
	 * This is the recommended way of retrieving values from the texmap.
	 * Note that it is inline, so you will have only a very small overhead.
	 **/
	inline unsigned char texMapAlpha(unsigned int texture, int x, int y) const
	{
		return mTexMap[texMapArrayPos(texture, x, y)];
	}

	/**
	 * Set the alpha value for @p texture at @p x, @p y to @p value.
	 *
	 * This function is safe, i.e. we won't crash for invalid x or y values.
	 **/
	void setTexMapAlpha(unsigned int texture, int x, int y, unsigned char value)
	{
		if (x < 0 || y < 0) {
			return;
		}
		if ((unsigned int)x >= width() || (unsigned int)y >= height()) {
			return;
		}
		mTexMap[texMapArrayPos(texture, x, y)] = value;
	}


	/**
	 * Fill the texMap with 100% of texture @p texture.
	 **/
	void fill(unsigned int texture)
	{
		for (unsigned int i = 0; i < mTextureCount; i++) {
			initialize(i, 0);
		}
		initialize(texture, 255);
	}

	/**
	 * Initialize (fill) the @p texture (at all coordinates) with @p alpha. If @p texture
	 * is negative then <em>all</em>textures (i.e. the complete texMap) are
	 * filled with @p alpha.
	 **/
	void initialize(int texture, unsigned char alpha)
	{
		if (texture < 0) {
			for (unsigned int i = 0; i < mTextureCount; i++) {
				initialize(i, alpha);
			}
			return;
		}
		for (unsigned int x = 0; x < width(); x++) {
			for (unsigned int y = 0; y < height(); y++) {
				setTexMapAlpha(texture, x, y, alpha);
			}
		}
	}
private:
	unsigned char* mTexMap;
	unsigned int mTextureCount;
};

/**
 * Data structure for holding OpenGL normals of map.
 * One normal is stored for each corner of the cell and normals are shared
 * between cells.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoNormalMap: public BoMapCornerArray
{
public:
	BoNormalMap(unsigned int width, unsigned int height)
			: BoMapCornerArray(width, height)
	{
		if (width == 0) {
			width = 1;
		}
		if (height == 0) {
			height = 1;
		}
		mNormalMap = new float[normalMapArrayPos(width - 1, height - 1) + 3];
	}

	virtual ~BoNormalMap()
	{
		delete[] mNormalMap;
	}

	// Normal maps won't be saved/loaded (they're auto-generated)
	virtual bool save(QDataStream&) const { return true; }
	virtual bool load(QDataStream&) { return true; }

	inline const float* normalMap() const { return mNormalMap; }

	/**
	 * @return The normal at @p x, @þ y. This function is safe, i.e. if @þ x
	 * or @p y are invalid we won't crash.
	 **/
	const float* normalAt(int x, int y) const
	{
		if (x < 0 || y < 0) {
			// AB: the very first normal in the array is returned. it will
			// be invalid anyway, no matter what we return.
			x = 0;
			y = 0;
		}
		if ((unsigned int)x >= width() || (unsigned int)y >= height()) {
			// AB: the very first normal in the array is returned. it will
			// be invalid anyway, no matter what we return.
			x = 0;
			y = 0;
		}
		return mNormalMap + normalMapArrayPos(x, y);
	}

	void setNormalAt(int x, int y, const BoVector3Float& n)
	{
		if (x < 0 || y < 0) {
			return;
		}
		if ((unsigned int)x >= width() || (unsigned int)y >= height()) {
			return;
		}
		mNormalMap[normalMapArrayPos(x, y) + 0] = n[0];
		mNormalMap[normalMapArrayPos(x, y) + 1] = n[1];
		mNormalMap[normalMapArrayPos(x, y) + 2] = n[2];
	}

	inline int normalMapArrayPos(int x, int y) const
	{
		return arrayPos(x, y) * 3;
	}

private:
	float* mNormalMap;
};


/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoColorMap
{
public:
	BoColorMap(unsigned int width, unsigned int height);

	virtual ~BoColorMap();

	// Color maps won't be saved/loaded (they're auto-generated)
	virtual bool save(QDataStream&) const { return true; }
	virtual bool load(QDataStream&) { return true; }

	void update(unsigned char* data);
	void updateRect(int x, int y, unsigned int w, unsigned int h, unsigned char* data);

	const unsigned char* textureData() const
	{
		return mData;
	}

	unsigned int width() const
	{
		return mWidth;
	}
	unsigned int height() const
	{
		return mHeight;
	}

	/**
	 * @return Whether the colormap changed since @ref setNotDirty was
	 * called.
	 **/
	bool isDirty() const
	{
		return mDirtyRect.isValid();
	}

	/**
	 * @return A rect containing the area that changed since @ref
	 * setNotDirty was called.
	 **/
	QRect dirtyRect() const
	{
		return mDirtyRect;
	}

	/**
	 * Unset any previous dirty settings
	 **/
	void setNotDirty()
	{
		mDirtyRect = QRect();
	}

private:
	unsigned int mWidth;
	unsigned int mHeight;
	unsigned char* mData;

	QRect mDirtyRect;
};


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

	bool createNewMap(unsigned int width, unsigned int height, BosonGroundTheme* theme);

	/**
	 * Fill the @ref texMap with 100% of @p texture.
	 **/
	void fill(unsigned int texture);

	void resetTexMap(unsigned int texture, unsigned char alpha)
	{
		if (!mTexMap) {
			return;
		}
		mTexMap->initialize(texture, alpha);
	}

	bool generateCellsFromTexMap();

	/**
	 * Resize the map. Currently this works only if the cells have
	 * <em>not</em> yet been created.
	 **/
	void resize(unsigned int width, unsigned int height);


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
	 * You should rather use @ref cell ! If you use this, you need to ensure
	 * yourself, that all cells you want ta access are actually valid.
	 * @return The internal cell array
	 **/
	inline Cell* cells() const { return mCells; }

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
		return BoMapCornerArray::arrayPos(x, y, width() + 1);
	}

	inline const float* heightMap() const { return mHeightMap->heightMap(); }
	inline const float* normalMap() const { return mNormalMap->normalMap(); }
	void setActiveColorMap(BoColorMap* map) { mActiveColorMap = map; }
	inline BoColorMap* activeColorMap() const { return mActiveColorMap; }
	BosonGroundTheme* groundTheme() const { return mGroundTheme; }

	void addColorMap(BoColorMap* map, const QString& name);
	QDict<BoColorMap>* colorMaps();
	void removeColorMap(const QString& name);

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
		return mTexMap ? mTexMap->texMap(texture) : 0;
	}

	/**
	 * @return BoTexMap::texMapArrayPos
	 **/
	inline int texMapArrayPos(unsigned int texture, int x, int y) const
	{
		return BoTexMap::texMapArrayPos(texture, x, y, width() + 1, height() + 1);
	}

	/**
	 * @return BoTexMap::texMapAlpha. Will crash if @ref texMap is NULL (we
	 * don't check in favor of performance).
	 **/
	inline unsigned char texMapAlpha(unsigned int texture, int x, int y) const
	{
		return mTexMap->texMapAlpha(texture, x, y);
	}

	/**
	 * @see BoTexMap::setTexMapAlpha. Will crash if @ref texMap is NULL (we
	 * don't check in favor of performance).
	 **/
	void setTexMapAlpha(unsigned int texture, int x, int y, unsigned char alpha)
	{
		mTexMap->setTexMapAlpha(texture, x, y, alpha);
	}

	/**
	 * See @ref BoTexMap::copyTexture
	 **/
	bool copyTexMapTexture(unsigned int dstTexture, const BoTexMap* src, unsigned int srcTexture)
	{
		if (mTexMap) {
			return mTexMap->copyTexture(dstTexture, src, srcTexture);
		}
		return false;
	}

	/**
	 * Note that you can specify (width() + 1) * (height() + 1) corners here!
	 * See also @ref cornerArrayPos for detailed description of possible
	 * values.
	 * @return The height of the upper left corner of the cell at @p x, @p
	 * y or 1.0 if invalid coordinates were specified.
	 *
	 * WARNING this returns the height of the ground at this corner. It does
	 * NOT consider the @ref waterDepth !
	 **/
	float heightAtCorner(int x, int y) const;
	void setHeightAtCorner(int x, int y, float height);
	float cellAverageHeight(int x, int y) const;
	void setHeightsAtCorners(const QValueList< QPair<QPoint, float> >& heights);

	/**
	 * @return The water depth at the specified corner. This is the distance
	 * of the @ref heightAtCorner to the surface of the water.
	 **/
	float waterDepthAtCorner(int x, int y) const;

	const QPtrList<BoLake>* lakes() const;



	/**
	 * Save the map geometry (widht/height, i.e. all datat required to
	 * create the @ref cells array) and groundtheme ID, as well as the file
	 * format version to an XML file. The XML file is returned on success
	 * (as a @ref QByteArray), otherwise an empty @ref QByteArray is
	 * returned
	 **/
	QByteArray saveMapGeomToFile() const;
	QByteArray saveMapPreviewPNGToFile() const;
	QByteArray saveWaterToFile() const;


	/**
	 * Load the "main" map, i.e. the map geo (it's size) and it's cells from
	 * the file on disk. This will not load things such as height map, which
	 * are in a different file.
	 **/
	bool loadMapGeomFromFile(const QByteArray& map);

	bool loadWaterFromFile(const QByteArray& water);

	/**
	 * Load the complete map, even those data that are stored in different
	 * files in the .bpf file. This is e.g. the height map.
	 *
	 * The stream must have been creates using @ref saveCompleteMap,
	 * <em>not</em> @ref saveMapGeomToFile!
	 **/
	bool loadCompleteMap(QDataStream& stream);

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

	QByteArray saveHeightMapImage() const;

	/**
	 * Load the @ref texMap from @p stream.
	 *
	 * If there is no data in this stream then this function will (try to)
	 * generate a texmap according to the @ref Cell::groundType of the
	 * already loaded cells. The default number of textures is used then.
	 **/
	bool loadTexMap(QDataStream& stream);
	bool saveTexMap(QDataStream& stream) const;

	QByteArray saveTexMapImage(unsigned int texture) const;

	/**
	 * Calculate the color of a cell @p x, @p y, as it is meant to be
	 * displayed in the minimap. The calculated value is returned in @p _p,
	 * @p _g, @p _b.
	 *
	 * If @p coveredByWater is set to TRUE, then the calculated color should
	 * not be used, as there is water on the ground.
	 *
	 * @return TRUE if the cell is valid and the values could be
	 * calculated. Otherwise FALSE.
	 **/
	bool calculateMiniMapGround(int x, int y, int* _r, int* _g, int* _b, bool* coveredByWater) const;

	void setModified(bool m) { mModified = m; }
	bool modified() const { return mModified; }

	void registerQuadTree(BoGroundQuadTreeNode* tree);
	void unregisterQuadTree(BoGroundQuadTreeNode* tree);

public:
	/**
	 * @return TRUE if the current map geo is valid.
	 **/
	static bool isValidMapGeo(unsigned int width, unsigned int height);

	/**
	 * @return The file format version of the map, that is used when
	 * saving a map file (see @ref saveMapGeomToFile).
	 **/
	static int mapFileFormatVersion();


public slots:
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
	void signalColorMapsChanged();

protected:
	bool saveHeightMap(QDataStream& stream) const;

	/**
	 * Save @p heightMap into @p stream.
	 * @param heightMap A heightmap array that is big enough for a @p
	 * mapWidth * @p mapHeight map.
	 **/
	static bool saveHeightMap(QDataStream& stream, unsigned int mapWidth, unsigned int mapHeight, BoHeightMap* heightMap);

	bool loadMapGeo(unsigned int width, unsigned int height);

	bool applyGroundTheme(const QString& id);
	bool applyGroundTheme(BosonGroundTheme* theme);

	bool loadHeightMap(QDataStream& stream);

	/**
	 * Create the map array accordint to @ref width and @ref height.
	 **/
	void createCells();

	/**
	 * Save the complete map into the stream, even the parts that are
	 * usually stored in different files, such as the height map.
	 *
	 * Use this to send the map over network, but remember that there will
	 * be a lot of data! (probably more a few MB for 500x500 maps!)
	 **/
	bool saveCompleteMap(QDataStream& stream) const;

	/**
	 * Recalculates all normals within given rect
	 * Normal map must already exist and rect must be valid
	 **/
	void recalculateNormalsInRect(int x1, int y1, int x2, int y2);
	void heightsInRectChanged(int minX, int minY, int maxX, int maxY);

private:
	void init();

private:
	class BosonMapPrivate;
	BosonMapPrivate* d;
	Cell* mCells;
	BoHeightMap* mHeightMap;
	BoNormalMap* mNormalMap;
	BoColorMap* mActiveColorMap;
	BoTexMap* mTexMap;
	bool mModified;

	unsigned int mMapWidth;
	unsigned int mMapHeight;
	BosonGroundTheme* mGroundTheme;
};

#endif
