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
#ifndef BOGROUNDRENDERER_H
#define BOGROUNDRENDERER_H

#include <qobject.h>

#include "global.h"

class Cell;
class PlayerIO;
class QString;
class BosonMap;
class BoMatrix;
class BoFrustum;
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
		mMinDistance = 0.0;
		mMaxDistance = 0.0;
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
		// Do not clear min/max distances here!
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
	void setMinDistance(float d)
	{
		mMinDistance = d;
	}
	void setMaxDistance(float d)
	{
		mMaxDistance = d;
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
	float minDistance() const
	{
		return mMinDistance;
	}
	float maxDistance() const
	{
		return mMaxDistance;
	}
private:
	unsigned int mRenderedCells;
	unsigned int mRenderedQuads;
	unsigned int mUsedTextures;
	float mMinDistance;
	float mMaxDistance;
};

/**
 * @short Class that handles terrain rendering.
 *
 * The main method here is @ref renderCells that is called by boson. This
 * method makes sure that a list of visible cells is available that should be
 * rendered and calls @ref renderVisibleCells that should render these cells.
 *
 * @ref renderVisibleCells is implemented by derived classes, that is by the
 * plugin library. This way it can easily be changed with little recompilation
 * and even without restarting boson.
 *
 * Note this class is supposed to be an interface only. Most rendering should go
 * to derived classes in the plugin.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoGroundRenderer : public QObject
{
	Q_OBJECT
public:
	BoGroundRenderer();
	virtual ~BoGroundRenderer();

	virtual bool initGroundRenderer();

	/**
	 * Apply pointers for all OpenGL matrices to this class. You must not
	 * free those matrices as long as there are still objects of this class!
	 **/
	void setMatrices(const BoMatrix* modelviewMatrix, const BoMatrix* projectionMatrix, const int* viewport);

	/**
	 * Do <em>not</em> use this class after freeing the pointer
	 **/
	void setViewFrustum(const BoFrustum* viewFrustum)
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
	unsigned int renderCells(const BosonMap* map, RenderFlags flags);

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
		Q_UNUSED(pos);
		return QString::null;
	}

// helper function that sets the cell @p cellCount in @p renderCells to @p x, @p
// y.
	static void setCell(int* renderCells, unsigned int cellCount, int x, int y, int w, int h);
	static void getCell(int* renderCells, unsigned int cellCount, int* x, int* y, int* w, int* h);
	static int* makeCellArray(unsigned int count);

	/**
	 * Called when fogged status of a cell or cells changes.
	 * x1, y1, x2, y2 define rectangle which includes all the changed cells.
	 * E.g. if x1 = x2 = 3 and y1 = y2 = 4, then only cell at (3; 4) changed.
	 **/
	virtual void cellFogChanged(int x1, int y1, int x2, int y2);

	/**
	 * Called when explored status of a cell or cells changes.
	 * x1, y1, x2, y2 define rectangle which includes all the changed cells.
	 * E.g. if x1 = x2 = 3 and y1 = y2 = 4, then only cell at (3; 4) changed.
	 **/
	virtual void cellExploredChanged(int x1, int y1, int x2, int y2);

	/**
	 * Called when height of a corner or corners changes.
	 * x1, y1, x2, y2 define rectangle which includes all the changed corners.
	 * E.g. if x1 = x2 = 3 and y1 = y2 = 4, then only corner at (3; 4) changed.
	 **/
	virtual void cellHeightChanged(int x1, int y1, int x2, int y2);

	/**
	 * Called when at least one texture of a corner or corners changes.
	 * x1, y1, x2, y2 define rectangle which includes all the changed corners.
	 * E.g. if x1 = x2 = 3 and y1 = y2 = 4, then only corner at (3; 4) changed.
	 **/
	virtual void cellTextureChanged(int x1, int y1, int x2, int y2);

	/**
	 * @return Whether this ground renderer can be used.
	 * You should check this before using a ground render.
	 * This method should e.g. check for hardware support, default implementation
	 * always returns true.
	 **/
	virtual bool usable() const;

protected:
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
	const BoFrustum* viewFrustum() const
	{
		return mViewFrustum;
	}

	/**
	 * Actually render the currently visible cells. Implemented by the
	 * plugin.
	 **/
	virtual void renderVisibleCells(int* cells, unsigned int cellsCount, const BosonMap* map, RenderFlags flags) = 0;
	virtual void renderVisibleCellsStart(const BosonMap* map) { Q_UNUSED(map) }
	virtual void renderVisibleCellsStop(const BosonMap* map) { Q_UNUSED(map) }

private:
	const BoMatrix* mModelviewMatrix;
	const BoMatrix* mProjectionMatrix;
	const int* mViewport;
	const BoFrustum* mViewFrustum;
	class PlayerIO* mLocalPlayerIO;
	BoGroundRendererStatistics* mStatistics;

	//AB: we should use a float* here which can be used as vertex array. we
	//should store x,y,z and texture x,y there. for this we need to have
	//cells in a single texture!
	//--> then we could also use GL_QUAD_STRIP
	int* mRenderCells;
	int mRenderCellsSize; // max. number of cells in the array
	unsigned int mRenderCellsCount; // actual number of cells in the array
};

Q_DECLARE_INTERFACE(BoGroundRenderer, "BoGroundRenderer")

#endif

