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

#include "bofastgroundrenderer.h"
#include "bofastgroundrenderer.moc"

#include "../../bomemory/bodummymemory.h"
#include "bogroundrendererbase.h"
#include "../gameengine/bosonmap.h"
#include "../defines.h"
#include "../gameengine/cell.h"
#include "../gameengine/bosongroundtheme.h"
#include "../bosongroundthemedata.h"
#include "../gameengine/boson.h"
#include "../botexture.h"
#include <bogl.h>
#include <bodebug.h>

#include <math.h>

BoFastGroundRenderer::BoFastGroundRenderer()
	: BoGroundRendererBase()
{
 mCurrentMap = 0;
 mCellTextures = 0;
}

BoFastGroundRenderer::~BoFastGroundRenderer()
{
 delete mCellTextures;
}

bool BoFastGroundRenderer::initGroundRenderer()
{
 if (!BoGroundRendererBase::initGroundRenderer()) {
	return false;
 }

 return true;
}

void BoFastGroundRenderer::renderVisibleCells(int* renderCells, unsigned int cellsCount, const BosonMap* map, RenderFlags flags)
{
 BO_CHECK_NULL_RET(renderCells);
 BO_CHECK_NULL_RET(map);
 BO_CHECK_NULL_RET(map->texMap());
 BO_CHECK_NULL_RET(mHeightMap2);
 BO_CHECK_NULL_RET(map->groundTheme());
 BO_CHECK_NULL_RET(currentGroundThemeData());

 updateMapCache(map);

 BosonGroundTheme* groundTheme = map->groundTheme();
 const float* heightMap = mHeightMap2;

 unsigned int* cellTextures = new unsigned int[cellsCount];
 for (unsigned int i = 0; i < cellsCount; i++) {
	int x, y;
	int w, h;
	BoGroundRenderer::getCell(renderCells, i, &x, &y, &w, &h);
	if (x < 0 || y < 0) {
		boError() << k_funcinfo << "invalid cell" << endl;
		continue;
	}

	// AB: this is very wrong - we always use the texture of the top-left
	// corner for the whole rect. however it should be "good enough" for the
	// fast renderer.
	cellTextures[i] = mCellTextures[BoMapCornerArray::arrayPos(x, y, map->width())];
 }

 // Texture coordinates will be autmatically generated by OpenGL
 float texPlaneS[] = { 1.0, 0.0, 0.0, 0.0 };
 float texPlaneT[] = { 0.0, 1.0, 0.0, 0.0 };
 glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
 glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
 glEnable(GL_TEXTURE_GEN_S);
 glEnable(GL_TEXTURE_GEN_T);
 glTexGenfv(GL_S, GL_OBJECT_PLANE, texPlaneS);
 glTexGenfv(GL_T, GL_OBJECT_PLANE, texPlaneT);
 glMatrixMode(GL_TEXTURE);

 unsigned int usedTextures = 0;
 unsigned int renderedQuads = 0;
 int count = 0;
 for (unsigned int i = 0; i < groundTheme->groundTypeCount(); i++) {
	BosonGroundTypeData* groundData = currentGroundThemeData()->groundTypeData(i);
	BoTexture* tex = groundData->currentTexture(boGame->advanceCallsCount());
	tex->bind();

	// Set up texture coordinate generation
	glLoadIdentity();
	glScalef(1.0f / groundData->groundType->textureSize, 1.0f / groundData->groundType->textureSize, 1.0);

	const int cornersWidth = map->width() + 1;


	unsigned int quads = 0;
	glBegin(GL_QUADS);
	for (unsigned int j = 0; j < cellsCount; j++) {
		if (cellTextures[j] != i) {
			continue;
		}
		int x;
		int y;
		int w;
		int h;
		BoGroundRenderer::getCell(renderCells, j, &x, &y, &w, &h);
		count++;

		int celloffset = y * cornersWidth + x;
		const float* heightMapUpperLeft = heightMap + celloffset;

		GLfloat cellXPos = (float)x;
		GLfloat cellYPos = -(float)y;

		float upperLeftHeight = *heightMapUpperLeft;
		float upperRightHeight = *(heightMapUpperLeft + w);
		float lowerLeftHeight = *(heightMapUpperLeft + cornersWidth * h);
		float lowerRightHeight = *(heightMapUpperLeft + cornersWidth * h + w);


		glVertex3f(cellXPos, cellYPos, upperLeftHeight);

		glVertex3f(cellXPos, cellYPos - h, lowerLeftHeight);

		glVertex3f(cellXPos + w, cellYPos - h, lowerRightHeight);

		glVertex3f(cellXPos + w, cellYPos, upperRightHeight);
		quads++;
	}
	glEnd();

	renderedQuads += quads;
	if (quads != 0) {
		usedTextures++;
	}
 }
 delete[] cellTextures;

 statistics()->setRenderedQuads(renderedQuads);
 statistics()->setUsedTextures(usedTextures);
 glLoadIdentity();
 glMatrixMode(GL_MODELVIEW);

 glDisable(GL_TEXTURE_GEN_S);
 glDisable(GL_TEXTURE_GEN_T);
 glDisable(GL_BLEND);
}

void BoFastGroundRenderer::updateMapCache(const BosonMap* map)
{
 BoGroundRendererBase::updateMapCache(map);
 if (mCurrentMap == map) {
	return;
 }
 BO_CHECK_NULL_RET(map);
 BO_CHECK_NULL_RET(map->groundTheme());

 delete mCellTextures;
 mCellTextures = new unsigned char[map->width() * map->height()];
 for (unsigned int x = 0; x < map->width(); x++) {
	for (unsigned int y = 0; y < map->height(); y++) {
		unsigned int maxValue = 0;
		for (unsigned int j = 0; j < map->groundTheme()->groundTypeCount(); j++) {
			unsigned int v = 0;
			v += (int)map->texMapAlpha(j, x, y);
			v += (int)map->texMapAlpha(j, x + 1, y);
			v += (int)map->texMapAlpha(j, x, y + 1);
			v += (int)map->texMapAlpha(j, x + 1, y + 1);
			if (v > maxValue) {
				maxValue = v;

				// this texture has highest alpha values in the four
				// corners
				mCellTextures[BoMapCornerArray::arrayPos(x, y, map->width())] = j;
			}
		}
	}
 }
 mCurrentMap = map;
}

