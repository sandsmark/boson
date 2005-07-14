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

#include "bogroundrenderer.h"
#include "bogroundrenderer.moc"

#include "bodebug.h"
#include "bosonmap.h"
#include "bosonconfig.h"
#include "defines.h"
#include "cell.h"
#include "bosongroundtheme.h"
#include "bosontexturearray.h"
#include "bo3dtools.h"
#include "bomaterial.h"

#include "playerio.h"

#include <klocale.h>

#include <GL/gl.h>
#include <math.h>

BoGroundRenderer::BoGroundRenderer()
{
 mModelviewMatrix = 0;
 mProjectionMatrix = 0;
 mViewport = 0;
 mViewFrustum = 0;
 mLocalPlayerIO = 0;

 mRenderCells = 0;
 mRenderCellsSize = 0;
 mRenderCellsCount = 0;
}

BoGroundRenderer::~BoGroundRenderer()
{
 delete[] mRenderCells;
}

void BoGroundRenderer::setRenderCells(Cell** renderCells, int renderCellsSize)
{
 delete[] mRenderCells;
 mRenderCells = renderCells;
 mRenderCellsSize = renderCellsSize;
 if (mRenderCellsSize < 0) {
	mRenderCellsCount = 0;
 } else if (mRenderCellsCount > (unsigned int)mRenderCellsSize) {
	mRenderCellsCount = mRenderCellsSize;
 }
}

void BoGroundRenderer::setRenderCellsCount(unsigned int count)
{
 if (renderCellsSize() < 0 && count > 0) {
	boError() << k_funcinfo << "invalid negative array size - can't have any elements. wanted to set to: " << count << endl;
	count = 0;
 } else if (count > (unsigned int)renderCellsSize()) {
	boError() << k_funcinfo << "can't have more than " << renderCellsSize() << " elements in array. wanted to set to: " << count << endl;
	count = renderCellsSize();
 }
 mRenderCellsCount = count;
}

QString BoGroundRenderer::rttiToName(int rtti)
{
 switch ((Renderer)rtti) {
	case Default:
		return i18n("Default");
	case Fast:
		return i18n("Fast");
	case Last:
		return i18n("Invalid entry - please report a bug");
 }
 return i18n("Unknwon (%1)").arg(rtti);
}

void BoGroundRenderer::setMatrices(const BoMatrix* modelviewMatrix, const BoMatrix* projectionMatrix, const int* viewport)
{
 mModelviewMatrix = modelviewMatrix;
 mProjectionMatrix = projectionMatrix;
 mViewport = viewport;
}

unsigned int BoGroundRenderer::renderCells(const BosonMap* map)
{
 BO_CHECK_NULL_RET0(map);

 if (renderCellsCount() == 0) {
	// this happens either when we have to generate the list first or if no
	// cell is visible at all. The latter case isn't speed relevant, so we
	// can simply re-generate then.
	boDebug() << k_funcinfo << "generating cell list" << endl;
	generateCellList(map);
 }

 BO_CHECK_NULL_RET0(localPlayerIO());

 BO_CHECK_NULL_RET0(map);
 BO_CHECK_NULL_RET0(map->heightMap());

 const float* heightMap = map->heightMap();
 int heightMapWidth = map->width() + 1;

 int cellsCount = 0;
 Cell** renderCells = createVisibleCellList(&cellsCount, localPlayerIO());
 BO_CHECK_NULL_RET0(renderCells);

 renderVisibleCells(renderCells, cellsCount, map);

 glEnable(GL_DEPTH_TEST);
 renderCellGrid(renderCells, cellsCount, heightMap, heightMapWidth);

 delete[] renderCells;

 return cellsCount;
}

void BoGroundRenderer::renderCellGrid(Cell** cells, int cellsCount, const float* heightMap, int heightMapWidth)
{
 BO_CHECK_NULL_RET(cells);
 BO_CHECK_NULL_RET(heightMap);
 if (heightMapWidth <= 0) {
	boError() << k_funcinfo << "invalid heightmap width " << heightMapWidth << endl;
	return;
 }
 if (cellsCount <= 0) {
	return;
 }
 if (boConfig->debugShowCellGrid()) {
	glDisable(GL_LIGHTING);
	glDisable(GL_NORMALIZE);
	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor3ub(255, 255, 255);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	for (int i = 0; i < cellsCount; i++) {
		Cell* c = cells[i];
		int x = c->x();
		int y = c->y();
		const float dist = 0.0f;
		GLfloat cellXPos = (float)x * BO_GL_CELL_SIZE;
		GLfloat cellYPos = -(float)y * BO_GL_CELL_SIZE;
			glVertex3f(cellXPos, cellYPos, heightMap[y * heightMapWidth + x] + dist);
			glVertex3f(cellXPos, cellYPos - BO_GL_CELL_SIZE, heightMap[(y+1) * heightMapWidth + x] + dist);
			glVertex3f(cellXPos + BO_GL_CELL_SIZE, cellYPos - BO_GL_CELL_SIZE, heightMap[(y+1) * heightMapWidth + (x+1)] + dist);
			glVertex3f(cellXPos + BO_GL_CELL_SIZE, cellYPos, heightMap[y * heightMapWidth + (x+1)] + dist);
	}
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	if (boConfig->useLight()) {
		glEnable(GL_LIGHTING);
		glEnable(GL_NORMALIZE);
	}
 }
}


Cell** BoGroundRenderer::createVisibleCellList(int* cells, PlayerIO* playerIO)
{
 BO_CHECK_NULL_RET0(playerIO);
 BO_CHECK_NULL_RET0(cells);
 Cell** renderCells = 0; // FIXME: store two arrays. one with x, one with y coordinate (or both in one array). don't store pointers to Cell
 if (renderCellsCount() > 0) {
	renderCells = new Cell*[renderCellsCount()];
 } else {
	// an array of size 0 isn't good.
	renderCells = new Cell*[1];
	renderCells[0] = 0;
 }
 int cellsCount = 0;
 for (unsigned int i = 0; i < renderCellsCount(); i++) {
	Cell* c = this->renderCells()[i];
	if (!c) {
		continue;
	}

	// AB: better solution: check *before* the cells get assigned to this
	// class. localPlayerIO() is *very* ugly in this class
	if (playerIO->isFogged(c->x(), c->y())) {
		// don't draw anything at all. the cell will just be black,
		// because of the glClear() call.
		continue;
	}
	renderCells[cellsCount] = c;
	cellsCount++;
 }
 *cells = cellsCount;
 return renderCells;
}
