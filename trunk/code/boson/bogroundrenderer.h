/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

class Cell;
class Player;
class PlayerIO;

class BosonMap;
class BoMatrix;

class QRect;

class BoGroundRendererPrivate;
class BoGroundRenderer
{
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
	void setViewFrustum(const double* viewFrustum);


	/**
	 * @return How many cells got actually rendered. (note: a single cell
	 * could have been rendered three times, always with a different
	 * texture, but will occur only once here!)
	 **/
	virtual unsigned int renderCells(const BosonMap* map);

	/**
	 * Render a grid for all cells in @p cells.
	 *
	 * It does render the grid only if @ref BosonConfig::debugShowCellGrid
	 * returns true.
	 **/
	void renderCellGrid(Cell** cells, int cellsCount, float* heightMap, int heightMapWidth);

	// AB: replace by localPlayerIO()
	void setLocalPlayer(Player* p);
	Player* localPlayer() const;

#if 0
	void setLocalPlayerIO(PlayerIO* io);
	PlayerIO* localPlayerIO() const
	{
		return 0;
	}
#endif

	/**
	 * This generates an array of visible cells for the @p player. It works
	 * on the list previously created by @ref generateCellList.
	 *
	 * This mainly checks for whether the cells are fogged.
	 *
	 * Note that you _MUST_ delete[] the array when you are done using it!
	 * @param cellCount The number of cells in the array is returned here.
	 **/
	Cell** createVisibleCellList(int* cellCount, Player* player); // TODO: replace by a PlayerIO

	/**
	 * Generate a list of cells that are (or may) be visible at the moment.
	 * @param map The map that contains the @ref Cell pointers. Use 0 to
	 * delete the current list of cells.
	 **/
	void generateCellList(const BosonMap* map);

protected:
	void calculateWorldRect(const QRect& rect, int mapWidth, int mapHeight, float* minX, float* minY, float* maxX, float* maxY);

	const double* viewFrustum() const;

private:
	/**
	 * Render the @p cells with the current texture.
	 *
	 * This is used for texture mixing (blending must be enabled) - first
	 * the cells are rendered with the first texture, then with the
	 * second, ...
	 *
	 * One could optimize this by using multitexturing for example!
	 **/
	void renderCellsNow(Cell** cells, int count, int cornersWidth, float* heightMap, unsigned char* texMapStart);

private:
	BoGroundRendererPrivate* d;
};

#endif

