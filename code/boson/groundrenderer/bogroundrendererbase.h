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
#ifndef BOGROUNDRENDERERBASE_H
#define BOGROUNDRENDERERBASE_H

#include "../bogroundrenderer.h"

class Cell;
class PlayerIO;
class QString;

class BosonMap;
class BoMatrix;
class BoVector3;

class QRect;

class BoGroundRendererBase : public BoGroundRenderer
{
	Q_OBJECT
public:
	BoGroundRendererBase();
	virtual ~BoGroundRendererBase();


	/**
	 * Generate a list of cells that are (or may) be visible at the moment.
	 * @param map The map that contains the @ref Cell pointers. Use 0 to
	 * delete the current list of cells.
	 **/
	virtual void generateCellList(const BosonMap* map);

protected:
	void calculateWorldRect(const QRect& rect, int mapWidth, int mapHeight, float* minX, float* minY, float* maxX, float* maxY);

};

class BoDefaultGroundRenderer : public BoGroundRendererBase
{
	Q_OBJECT
public:
	BoDefaultGroundRenderer();
	virtual ~BoDefaultGroundRenderer();

	virtual int rtti() const { return Default; }

protected:
	virtual void renderVisibleCells(Cell** cells, unsigned int cellsCount, const BosonMap* map);

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
	void renderCellsNow(Cell** cells, int count, int cornersWidth, const float* heightMap, const float* normalMap, const unsigned char* texMapStart);
	void renderCellColors(Cell** cells, int count, int width, const unsigned char* colorMap, const float* heightMap);
};

class BoFastGroundRenderer : public BoGroundRendererBase
{
	Q_OBJECT
public:
	BoFastGroundRenderer();
	virtual ~BoFastGroundRenderer();

	virtual int rtti() const { return Fast; }

	virtual void renderVisibleCells(Cell** cells, unsigned int cellsCount, const BosonMap* map);
};
#endif

