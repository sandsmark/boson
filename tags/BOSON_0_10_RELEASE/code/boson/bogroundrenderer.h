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
class BoVector3;

class QRect;

class BoGroundRenderer : public QObject
{
	Q_OBJECT
public:
	enum Renderer {
		// note that the numbers must be in order! (i.e. 0, 1, 2, ...)
		Default = 0,
		Fast = 1,

		// this _must_ remain the last item of the enum!
		Last
	};

public:
	BoGroundRenderer();
	virtual ~BoGroundRenderer();

	/**
	 * This takes an @rtti (see @ref Renderer) and converts it into a @ref
	 * QString.
	 **/
	static QString rttiToName(int rtti);

	virtual int rtti() const = 0;

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
	void renderCellGrid(Cell** cells, int cellsCount, const float* heightMap, int heightMapWidth);


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
	Cell** createVisibleCellList(int* cellCount, PlayerIO* playerIO);

	void setRenderCells(Cell** renderCells, int renderCellsSize);
	void setRenderCellsCount(unsigned int count);

	/**
	 * @return The array containing the cells that will be rendered. The
	 * array can take up to @ref renderCellsSize elements, but only the
	 * first @ref renderCellsCount elements are actually used. The rest can
	 * (and should) be NULL.
	 **/
	Cell** renderCells() const
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

	virtual void renderVisibleCells(Cell** cells, unsigned int cellsCount, const BosonMap* map) = 0;

private:
	const BoMatrix* mModelviewMatrix;
	const BoMatrix* mProjectionMatrix;
	const int* mViewport;
	const float* mViewFrustum;
	class PlayerIO* mLocalPlayerIO;

	//AB: we should use a float* here which can be used as vertex array. we
	//should store x,y,z and texture x,y there. for this we need to have
	//cells in a single texture!
	//--> then we could also use GL_QUAD_STRIP
	Cell** mRenderCells;
	int mRenderCellsSize; // max. number of cells in the array
	unsigned int mRenderCellsCount; // actual number of cells in the array
};

#endif
