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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "bogroundrenderer.h"
#include "bogroundrenderer.moc"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "bosonmap.h"
#include "bosonconfig.h"
#include "defines.h"
#include "cell.h"
#include "bo3dtools.h"
#include "bomaterial.h"
#include "bowater.h"
#include "botexture.h"
#include <bogl.h>

#include "playerio.h"

#include <klocale.h>

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

 mStatistics = 0;
}

BoGroundRenderer::~BoGroundRenderer()
{
 delete[] mRenderCells;
 delete mStatistics;
}

bool BoGroundRenderer::initGroundRenderer()
{
 mStatistics = new BoGroundRendererStatistics();
 return true;
}

void BoGroundRenderer::setRenderCells(int* renderCells, int renderCellsSize)
{
 delete[] mRenderCells;
 mRenderCells = renderCells;
 mRenderCellsSize = renderCellsSize;
 if (mRenderCellsSize > 4194304) {
	boError() << k_funcinfo << "invalid mRenderCellsSize: " << mRenderCellsSize << endl;
	mRenderCellsSize = 4194304;
 }
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


void BoGroundRenderer::setMatrices(const BoMatrix* modelviewMatrix, const BoMatrix* projectionMatrix, const int* viewport)
{
 mModelviewMatrix = modelviewMatrix;
 mProjectionMatrix = projectionMatrix;
 mViewport = viewport;
}

unsigned int BoGroundRenderer::renderCells(const BosonMap* map, RenderFlags flags)
{
 BO_CHECK_NULL_RET0(map);
 BO_CHECK_NULL_RET0(statistics());

 if (renderCellsCount() == 0) {
	// this happens either when we have to generate the list first or if no
	// cell is visible at all. The latter case isn't speed relevant, so we
	// can simply re-generate then.
	boDebug() << k_funcinfo << "generating cell list" << endl;
	generateCellList(map);
 }

 BO_CHECK_NULL_RET0(map);
 BO_CHECK_NULL_RET0(map->heightMap());

 statistics()->clear();

 const float* heightMap = map->heightMap(); // FIXME: use the heightmap of the cell renderer
 int heightMapWidth = map->width() + 1;

 BO_CHECK_NULL_RET0(mRenderCells);

 // Render cells
 renderVisibleCellsStart(map);
 if (renderCellsCount() > 0) {
	renderVisibleCells(renderCells(), renderCellsCount(), map, flags);
 } else {
 // AB: we call renderVisibleCellsStart()/Stop() only to make sure that
 // the GL states are in the expected states after ground rendering
 // but no actual rendering required.
 }
 renderVisibleCellsStop(map);

 if (!(flags & DepthOnly)) {
	glEnable(GL_DEPTH_TEST);
	renderCellGrid(renderCells(), renderCellsCount(), heightMap, heightMapWidth);
 }

 statistics()->setRenderedCells(renderCellsCount());
 if (statistics()->renderedQuads() == 0) {
	// fill only, if the renderer didn't do itself
	statistics()->setRenderedQuads(renderCellsCount());
 }

 return renderCellsCount();
}

void BoGroundRenderer::renderCellGrid(int* cells, int cellsCount, const float* heightMap, int heightMapWidth)
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
 if (boConfig->boolValue("debug_cell_grid")) {
	glDisable(GL_LIGHTING);
	glDisable(GL_NORMALIZE);
	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor3ub(127, 127, 127);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	for (int i = 0; i < cellsCount; i++) {
		int x;
		int y;
		int w;
		int h;
		BoGroundRenderer::getCell(cells, i, &x, &y, &w, &h);
		const float dist = 0.0f;
		GLfloat cellXPos = (float)x;
		GLfloat cellYPos = -(float)y;
			glVertex3f(cellXPos, cellYPos, heightMap[y * heightMapWidth + x] + dist);
			glVertex3f(cellXPos, cellYPos - h, heightMap[(y+h) * heightMapWidth + x] + dist);
			glVertex3f(cellXPos + w, cellYPos - h, heightMap[(y+h) * heightMapWidth + (x+w)] + dist);
			glVertex3f(cellXPos + w, cellYPos, heightMap[y * heightMapWidth + (x+w)] + dist);
	}
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	if (boConfig->boolValue("UseLight")) {
		glEnable(GL_LIGHTING);
		glEnable(GL_NORMALIZE);
	}
 }
}


QString BoGroundRenderer::statisticsData() const
{
 if (!mStatistics) {
	return i18n("No statistics available");
 }
 return mStatistics->statisticsData();
}

void BoGroundRenderer::setCell(int* renderCells, unsigned int cellCount, int x, int y, int w, int h)
{
 renderCells[cellCount * 4 + 0] = x;
 renderCells[cellCount * 4 + 1] = y;
 renderCells[cellCount * 4 + 2] = w;
 renderCells[cellCount * 4 + 3] = h;
}

void BoGroundRenderer::getCell(int* renderCells, unsigned int cellCount, int *x, int *y, int* w, int* h)
{
 *x = renderCells[cellCount * 4 + 0];
 *y = renderCells[cellCount * 4 + 1];
 *w = renderCells[cellCount * 4 + 2];
 *h = renderCells[cellCount * 4 + 3];
}

int* BoGroundRenderer::makeCellArray(unsigned int count)
{
 const unsigned int maxCount = 4194304; // (2^11)^2
 if (count > maxCount) {
	// AB: we will never have that many cells, so if we exceed this value,
	// we must have a nasty bug around.
	count = maxCount;
 }
 return new int[count * 4];
}

void BoGroundRenderer::cellFogChanged(int, int, int, int)
{
}

void BoGroundRenderer::cellHeightChanged(int, int, int, int)
{
}

void BoGroundRenderer::cellTextureChanged(int, int, int, int)
{
}

bool BoGroundRenderer::usable() const
{
 return true;
}

QString BoGroundRendererStatistics::statisticsData() const
{
 QString data = i18n("Cells rendered: %1 (quads: %2)").arg(renderedCells()).arg(renderedQuads());
 if (renderedQuads() > 0 && usedTextures() > 0) {
	data += i18n("\n Used Textures: %1").arg(usedTextures());
 }
 return data;
}

