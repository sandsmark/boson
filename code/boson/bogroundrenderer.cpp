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
#include "bo3dtools.h"
#include "bomaterial.h"
#include "bowater.h"
#include "botexture.h"

#include "playerio.h"

#include <klocale.h>

#include <GL/gl.h>
#include <math.h>


// Whether to use glTexSubImage2D() to update texture. It seems to be buggy.
#define USE_TEXSUBIMAGE


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

 mStatistics = new BoGroundRendererStatistics();

 mFogTexture = 0;
 mFogTextureData = 0;
 mLastMapWidth = 0;
 mLastMapHeight = 0;
}

BoGroundRenderer::~BoGroundRenderer()
{
 delete mFogTextureData;
 delete mFogTexture;
 delete[] mRenderCells;
 delete mStatistics;
}

void BoGroundRenderer::setRenderCells(int* renderCells, int renderCellsSize)
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
 BO_CHECK_NULL_RET0(statistics());

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

 statistics()->clear();

 const float* heightMap = map->heightMap(); // FIXME: use the heightmap of the cell renderer
 int heightMapWidth = map->width() + 1;

 int cellsCount = 0;
 int* renderCells = createVisibleCellList(&cellsCount, localPlayerIO());
 BO_CHECK_NULL_RET0(renderCells);

 if (boConfig->textureFOW()) {
	// Enable fog texture (TU 1)
	initFogTexture(map);
	boTextureManager->activateTextureUnit(1);
#ifndef USE_TEXSUBIMAGE
	// Update fog texture
	if (!mFogTexture) {
		mFogTexture = new BoTexture(mFogTextureData, mFogTextureDataW, mFogTextureDataH,
				BoTexture::FilterLinear | BoTexture::FormatRGBA);
	}
#endif
	boTextureManager->bindTexture(mFogTexture);
	// Use automatic texcoord generation to map fog texture to cells
	const float texPlaneS[] = { 1.0f / mFogTextureDataW, 0.0, 0.0, 0.0 };
	const float texPlaneT[] = { 0.0, 1.0f / mFogTextureDataH, 0.0, 0.0 };
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_S, GL_OBJECT_PLANE, texPlaneS);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, texPlaneT);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	// This compensates for the border that we add to the texture
	glTranslatef(1.0f / mFogTextureDataW, 1.0f / mFogTextureDataH, 0.0);
	glScalef(1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	boTextureManager->activateTextureUnit(0);
 }

 // Render cells
 renderVisibleCells(renderCells, cellsCount, map);

 if (boConfig->textureFOW()) {
	// end using fog texture
	boTextureManager->activateTextureUnit(1);
	boTextureManager->unbindTexture();
	{
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
	}
	boTextureManager->activateTextureUnit(0);
 }


 glEnable(GL_DEPTH_TEST);
 renderCellGrid(renderCells, cellsCount, heightMap, heightMapWidth);

 delete[] renderCells;

 statistics()->setRenderedCells(cellsCount);
 if (statistics()->renderedQuads() == 0) {
	// fill only, if the renderer didn't do itself
	statistics()->setRenderedQuads(cellsCount);
 }

 return cellsCount;
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
 if (boConfig->debugShowCellGrid()) {
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
	if (boConfig->useLight()) {
		glEnable(GL_LIGHTING);
		glEnable(GL_NORMALIZE);
	}
 }
}


int* BoGroundRenderer::createVisibleCellList(int* cells, PlayerIO* playerIO)
{
 BO_CHECK_NULL_RET0(playerIO);
 BO_CHECK_NULL_RET0(cells);
 int* renderCells = 0;
 if (renderCellsCount() > 0) {
	renderCells = makeCellArray(renderCellsCount());
 } else {
	// an array of size 0 isn't good.
	renderCells = makeCellArray(1);
	setCell(renderCells, 0, 0, 0, 1, 1);
 }
 int cellsCount = 0;
 for (unsigned int i = 0; i < renderCellsCount(); i++) {
	int x;
	int y;
	int w;
	int h;
	getCell(this->renderCells(), i, &x, &y, &w, &h);
	if (x < 0 || y < 0) {
		continue;
	}

#warning FIXME: w,h
	// FIXME: if w,h are not both 1, then there are 3 possible cases (see
	// below, at isFogged()).
	if (!boWaterManager->cellVisible(x, y)) {
		// don't draw anything at all. the cell won't be visible

		// AB: I think this mean there is water rendered at this cell,
		// so we dont need to render the cell anymore.
		continue;
	}

#warning FIXME: w,h
	// FIXME: w,h are not used. there are 3 cases possible:
	// 1. the whole rect ((x,y),(x+w,y+h)) is fogged. ignore all cells (i.e.
	//    "continue").
	// 2. the whole rect is visible. leave cells in the array (i.e. do not
	//    "continue").
	// 3. parts of the rect are visible, other parts are not. in this case
	//    we must split the rect up, probably into separate cells (i.e.
	//    chunks with w==h==1). however then the renderCells array may get
	//    _larger_ than it was before!
	//    -> we must make sure that it can actually take all items (can
	//       easily be done if it is at least as large as the number of
	//       containing cells, not the number of its elements).

	// AB: better solution: check *before* the cells get assigned to this
	// class. localPlayerIO() is *very* ugly in this class
	if (playerIO->isFogged(x, y)) {
		// don't draw anything at all. the cell will just be black,
		// because of the glClear() call.
		continue;
	}
	setCell(renderCells, cellsCount, x, y, w, h);
	cellsCount++;
 }
 *cells = cellsCount;
 return renderCells;
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
 return new int[count * 4];
}

void BoGroundRenderer::cellChanged(int x, int y)
{
 if (!boConfig->textureFOW()) {
	return;
 }
 if (!mFogTextureData) {
	return;
 }
 boDebug() << "FOGTEX: " << k_funcinfo << "x: " << x << "; y: " << y << endl;
 unsigned char value = 0;
 if (!localPlayerIO()->isFogged(x, y)) {
	value = 255;
 }

 // 'x + 1' and 'y + 1' because we use 1-texel border
 mFogTextureData[((y + 1) * mFogTextureDataW + (x + 1)) * 4 + 0] = value;
 mFogTextureData[((y + 1) * mFogTextureDataW + (x + 1)) * 4 + 1] = value;
 mFogTextureData[((y + 1) * mFogTextureDataW + (x + 1)) * 4 + 2] = value;

#ifdef USE_TEXSUBIMAGE
 mFogTexture->bind();
 // Because of (possible) texture compression, we can't update a single pixel
 //  of the texture. Instead, we have to update the whole 4x4 block that the
 //  pixel is in.
 int blockx = ((x + 1) / 4) * 4;
 int blocky = ((y + 1) / 4) * 4;
 // Create temporary array for the 4x4 block
 unsigned char blockdata[4 * 4 * 4];
 // Copy data from mFogTextureData to blockdata
 for(int i = 0; i < 4; i++) {
	for(int j = 0; j < 4; j++) {
		blockdata[((j * 4) + i) * 4 + 0] = mFogTextureData[((blocky + j) * mFogTextureDataW + (blockx + i)) * 4 + 0];
		blockdata[((j * 4) + i) * 4 + 1] = mFogTextureData[((blocky + j) * mFogTextureDataW + (blockx + i)) * 4 + 1];
		blockdata[((j * 4) + i) * 4 + 2] = mFogTextureData[((blocky + j) * mFogTextureDataW + (blockx + i)) * 4 + 2];
		blockdata[((j * 4) + i) * 4 + 3] = mFogTextureData[((blocky + j) * mFogTextureDataW + (blockx + i)) * 4 + 3];
	}
 }
 // Update texture
 glTexSubImage2D(GL_TEXTURE_2D, 0, blockx, blocky, 4, 4, GL_RGBA, GL_UNSIGNED_BYTE, blockdata);

#else
 delete mFogTexture;
 mFogTexture = 0;
#endif
}

void BoGroundRenderer::initFogTexture(const BosonMap* map)
{
 if (mLastMapWidth != map->width() || mLastMapHeight != map->height()) {
	// Map size has changed. Delete fog texture (new one will be created)
	delete mFogTextureData;
	delete mFogTexture;
	mFogTextureData = 0;
	mFogTexture = 0;
 }
 if (!mFogTextureData) {
	// Init fog texture
	// +2 because we want 1-pixel border
	mLastMapWidth = map->width();
	mLastMapHeight = map->height();
	int w = BoTexture::nextPower2(mLastMapWidth + 2);
	int h = BoTexture::nextPower2(mLastMapHeight + 2);
	boDebug() << "FOGTEX: " << k_funcinfo << "w: " << w << "; h: " << h  << endl;
	mFogTextureDataW = w;
	mFogTextureDataH = h;
	mFogTextureData = new unsigned char[w * h * 4];
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			mFogTextureData[(y * w + x) * 4 + 0] = 0;
			mFogTextureData[(y * w + x) * 4 + 1] = 0;
			mFogTextureData[(y * w + x) * 4 + 2] = 0;
			mFogTextureData[(y * w + x) * 4 + 3] = 255;
		}
	}
	for (unsigned int y = 1; y <= mLastMapHeight; y++) {
		for (unsigned int x = 1; x <= mLastMapWidth; x++) {
			unsigned char value = 0;
			if (!localPlayerIO()->isFogged(x - 1, y - 1)) {
				value = 255;
			}
			mFogTextureData[(y * w + x) * 4 + 0] = value;
			mFogTextureData[(y * w + x) * 4 + 1] = value;
			mFogTextureData[(y * w + x) * 4 + 2] = value;
			mFogTextureData[(y * w + x) * 4 + 3] = 255;
		}
	}
#ifdef USE_TEXSUBIMAGE
	mFogTexture = new BoTexture(mFogTextureData, mFogTextureDataW, mFogTextureDataH,
			BoTexture::FilterLinear | BoTexture::FormatRGBA);
#endif
 }
}


QString BoGroundRendererStatistics::statisticsData() const
{
 QString data = i18n("Cells rendered: %1 (quads: %2)").arg(renderedCells()).arg(renderedQuads());
 if (renderedQuads() > 0 && usedTextures() > 0) {
	data += i18n("\n Used Textures: %1").arg(usedTextures());
 }
 return data;
}

