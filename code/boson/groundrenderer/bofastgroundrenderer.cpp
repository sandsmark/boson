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

#include "bofastgroundrenderer.h"
#include "bofastgroundrenderer.moc"

#include "bogroundrendererbase.h"
#include "../bosonmap.h"
#include "../defines.h"
#include "../cell.h"
#include "../bosongroundtheme.h"
#include "../bomaterial.h"
#include "../boson.h"
#include <bodebug.h>

#include <GL/gl.h>
#include <math.h>

BoFastGroundRenderer::BoFastGroundRenderer(bool useCellTree)
	: BoGroundRendererBase(useCellTree)
{
}

BoFastGroundRenderer::~BoFastGroundRenderer()
{
}

void BoFastGroundRenderer::renderVisibleCells(int* renderCells, unsigned int cellsCount, const BosonMap* map)
{
 BO_CHECK_NULL_RET(renderCells);
 BO_CHECK_NULL_RET(map);
 BO_CHECK_NULL_RET(map->texMap());
 BO_CHECK_NULL_RET(map->heightMap());
 BO_CHECK_NULL_RET(map->groundTheme());

 BosonGroundTheme* groundTheme = map->groundTheme();
 const float* heightMap = map->heightMap();

 unsigned int* cellTextures = new unsigned int[cellsCount];
 for (unsigned int i = 0; i < cellsCount; i++) {
	int x, y;
	int w, h;
	BoGroundRenderer::getCell(renderCells, i, &x, &y, &w, &h);
	if (x < 0 || y < 0) {
		boError() << k_funcinfo << "invalid cell" << endl;
		continue;
	}
	cellTextures[i] = 0;
	unsigned int maxValue = 0;
	for (unsigned int j = 0; j < groundTheme->textureCount(); j++) {
		unsigned int v = 0;
		v += (int)map->texMapAlpha(j, x, y);
		v += (int)map->texMapAlpha(j, x + w, y);
		v += (int)map->texMapAlpha(j, x, y + h);
		v += (int)map->texMapAlpha(j, x + w, y + h);
		if (v > maxValue) {
			maxValue = v;

			// this texture has highest alpha values in the four
			// corners
			cellTextures[i] = j;
		}

	}
 }

 unsigned int usedTextures = 0;
 unsigned int renderedQuads = 0;
 int count = 0;
 for (unsigned int i = 0; i < groundTheme->textureCount(); i++) {
	glBindTexture(GL_TEXTURE_2D, map->currentTexture(i, boGame->advanceCallsCount()));

	const int offsetCount = 5;
	const float offset = 1.0f / (float)offsetCount;
	const float texOffsets[] = { 0.0f, 0.2f, 0.4f, 0.6f, 0.8f };

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
		if (w != 1 || h != 1 || x < 0 || y < 0) {
			boError() << k_funcinfo << x << " " << y << " " << w << " " << h << endl;
			continue;
		}
		count++;

		int celloffset = y * cornersWidth + x;
		const float* heightMapUpperLeft = heightMap + celloffset;

		GLfloat cellXPos = (float)x * BO_GL_CELL_SIZE;
		GLfloat cellYPos = -(float)y * BO_GL_CELL_SIZE;

		float upperLeftHeight = *heightMapUpperLeft;
		float upperRightHeight = *(heightMapUpperLeft + w);
		float lowerLeftHeight = *(heightMapUpperLeft + cornersWidth * h);
		float lowerRightHeight = *(heightMapUpperLeft + cornersWidth * h + w);


		// Map cell's y-coordinate to range (offsetCount - 1) ... 0
		y = offsetCount - (y % offsetCount) - 1;

		glTexCoord2f(texOffsets[x % offsetCount], texOffsets[y % offsetCount] + offset);
		glVertex3f(cellXPos, cellYPos, upperLeftHeight);

#warning see default ground renderer - do we have to take w,h into account for the offsets?
		glTexCoord2f(texOffsets[x % offsetCount], texOffsets[y % offsetCount]);
		glVertex3f(cellXPos, cellYPos - h * BO_GL_CELL_SIZE, lowerLeftHeight);

		glTexCoord2f(texOffsets[x % offsetCount] + offset, texOffsets[y % offsetCount]);
		glVertex3f(cellXPos + w * BO_GL_CELL_SIZE, cellYPos - h * BO_GL_CELL_SIZE, lowerRightHeight);

		glTexCoord2f(texOffsets[x % offsetCount] + offset, texOffsets[y % offsetCount] + offset);
		glVertex3f(cellXPos + w * BO_GL_CELL_SIZE, cellYPos, upperRightHeight);
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

 glDisable(GL_BLEND);
}

