/*
    This file is part of the Boson game
    Copyright (C) 2003-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOGROUNDRENDERER_H
#define BOGROUNDRENDERER_H

#include <qobject.h>

class Cell;
class PlayerIO;
class QString;
class BosonMap;
class BoMatrix;
class bofixed;
class BoTexture;
template<class T> class BoVector3;
typedef BoVector3<bofixed> BoVector3Fixed;
typedef BoVector3<float> BoVector3Float;

class QRect;

/**
 * Contains statistics about the most recently rendered frame
 **/
class BoGroundRendererStatistics
{
public:
	BoGroundRendererStatistics()
	{
		clear();
	}
	~BoGroundRendererStatistics()
	{
	}

	void clear()
	{
		mRenderedCells = 0;
		mRenderedQuads = 0;
		mUsedTextures = 0;
	}

	QString statisticsData() const;

	void setRenderedCells(unsigned int c)
	{
		mRenderedCells = c;
	}
	void setRenderedQuads(unsigned int q)
	{
		mRenderedQuads = q;
	}
	void setUsedTextures(unsigned int t)
	{
		mUsedTextures = t;
	}

	unsigned int renderedCells() const
	{
		return mRenderedCells;
	}
	unsigned int renderedQuads() const
	{
		return mRenderedQuads;
	}
	unsigned int usedTextures() const
	{
		return mUsedTextures;
	}
private:
	unsigned int mRenderedCells;
	unsigned int mRenderedQuads;
	unsigned int mUsedTextures;
};

class BoGroundRenderer : public QObject
{
	Q_OBJECT
public:
	BoGroundRenderer();
	virtual ~BoGroundRenderer();

	/**
	 * Apply pointers for all OpenGL matrices to this class. You must not
	 * free those matrices as long as there are still objects of this class!
	 **/
	void setMatrices(const BoMatrix* modelviewMatrix, const BoMatrix* projectionMatrix, const int* viewport);

	/**
	 * @param viewFrustum A pointer to the view frustum which must be a 6*4
	 * array. Do <em>not</em> use this class after freeing the pointer
	 **/
	void setViewFrustum(const float* viewFrustum)
	{
		mViewFrustum = viewFrustum;
	}

	/**
	 * Render a grid for all cells in @p cells.
	 *
	 * It does render the grid only if @ref BosonConfig::debugShowCellGrid
	 * returns true.
	 **/
	void renderCellGrid(int* cells, int cellsCount, const float* heightMap, int heightMapWidth);


	/**
	 * @return How many cells got actually rendered. (note: a single cell
	 * could have been rendered three times, always with a different
	 * texture, but will occur only once here!)
	 **/
	unsigned int renderCells(const BosonMap* map);

	void setLocalPlayerIO(PlayerIO* p)
	{
		mLocalPlayerIO = p;
	}

	PlayerIO* localPlayerIO() const
	{
		return mLocalPlayerIO;
	}

	/**
	 * Generate a list of cells that are (or may) be visible at the moment.
	 * @param map The map that contains the @ref Cell pointers. Use 0 to
	 * delete the current list of cells.
	 **/
	virtual void generateCellList(const BosonMap* map) = 0;

	QString statisticsData() const;
	BoGroundRendererStatistics* statistics() const
	{
		return mStatistics;
	}

	virtual QString debugStringForPoint(const BoVector3Fixed& pos) const
	{
		return QString::null;
	}

// helper function that sets the cell @p cellCount in @p renderCells to @p x, @p
// y.
	static void setCell(int* renderCells, unsigned int cellCount, int x, int y, int w, int h);
	static void getCell(int* renderCells, unsigned int cellCount, int* x, int* y, int* w, int* h);
	static int* makeCellArray(unsigned int count);

	/**
	 * Call this when fogged status of a cell changes.
	 * This updates the fog texture accordingly.
	 **/
	void cellChanged(int x, int y);
	void initFogTexture(const BosonMap* map);

protected:
	/**
	 * This generates an array of visible cells for the @p playerIO. It works
	 * on the list previously created by @ref generateCellList. (i.e. on
	 * @ref renderCells)
	 *
	 * This mainly checks for whether the cells are fogged.
	 *
	 * Note that you _MUST_ delete[] the array when you are done using it!
	 * @param cellCount The number of cells in the array is returned here.
	 **/
	int* createVisibleCellList(int* cellCount, PlayerIO* playerIO);

	void setRenderCells(int* renderCells, int renderCellsSize);
	void setRenderCellsCount(unsigned int count);

	/**
	 * @return The array containing the cells that will be rendered. The
	 * array can take up to @ref renderCellsSize elements, but only the
	 * first @ref renderCellsCount elements are actually used. The rest can
	 * (and should) be NULL.
	 **/
	int* renderCells() const
	{
		return mRenderCells;
	}

	/**
	 * @return The size of the @ref renderCells array (NOT the number of
	 * elements in it! See @ref renderCellsCount)
	 **/
	int renderCellsSize() const
	{
		return mRenderCellsSize;
	}

	/**
	 * @return The number of cells that were marked to be rendered by @ref
	 * generateCellList
	 **/
	inline unsigned int renderCellsCount() const
	{
		return mRenderCellsCount;
	}

	const BoMatrix* modelviewMatrix() const
	{
		return mModelviewMatrix;
	}
	const BoMatrix* projectionMatrix() const
	{
		return mProjectionMatrix;
	}
	const int* viewport() const
	{
		return mViewport;
	}
	const float* viewFrustum() const
	{
		return mViewFrustum;
	}

	virtual void renderVisibleCells(int* cells, unsigned int cellsCount, const BosonMap* map) = 0;

	/**
	 * Updates fog texture if it's dirty
	 **/
	void updateFogTexture();


private:
	const BoMatrix* mModelviewMatrix;
	const BoMatrix* mProjectionMatrix;
	const int* mViewport;
	const float* mViewFrustum;
	class PlayerIO* mLocalPlayerIO;
	BoGroundRendererStatistics* mStatistics;

	//AB: we should use a float* here which can be used as vertex array. we
	//should store x,y,z and texture x,y there. for this we need to have
	//cells in a single texture!
	//--> then we could also use GL_QUAD_STRIP
	int* mRenderCells;
	int mRenderCellsSize; // max. number of cells in the array
	unsigned int mRenderCellsCount; // actual number of cells in the array

	BoTexture* mFogTexture;
	unsigned char* mFogTextureData;
	int mFogTextureDataW;
	int mFogTextureDataH;
	unsigned int mLastMapWidth;
	unsigned int mLastMapHeight;
	bool mFogTextureDirty;
	int mFogTextureDirtyAreaX1;
	int mFogTextureDirtyAreaY1;
	int mFogTextureDirtyAreaX2;
	int mFogTextureDirtyAreaY2;
};

#endif

