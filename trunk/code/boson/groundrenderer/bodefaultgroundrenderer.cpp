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

#include "bodefaultgroundrenderer.h"
#include "bodefaultgroundrenderer.moc"

#include "bogroundrendererbase.h"
#include "../bosonmap.h"
#include "../bosonconfig.h"
#include "../defines.h"
#include "../bosongroundtheme.h"
#include "../bomaterial.h"
#include "../boson.h"
#include <bodebug.h>

#include <GL/gl.h>
#include <math.h>


BoDefaultGroundRenderer::BoDefaultGroundRenderer(bool useCellTree)
	: BoGroundRendererBase(useCellTree)
{
}

BoDefaultGroundRenderer::~BoDefaultGroundRenderer()
{
}

void BoDefaultGroundRenderer::renderVisibleCells(int* renderCells, unsigned int cellsCount, const BosonMap* map)
{
 BO_CHECK_NULL_RET(renderCells);
 BO_CHECK_NULL_RET(map);
 BO_CHECK_NULL_RET(map->texMap());
 BO_CHECK_NULL_RET(map->heightMap());
 BO_CHECK_NULL_RET(map->normalMap());
 BO_CHECK_NULL_RET(map->groundTheme());

 BosonGroundTheme* groundTheme = map->groundTheme();

 // AB: we can increase performance even more here. lets replace d->mRenderCells
 // by two array defining the coordinates of cells and the heightmap values.
 // we could use that as vertex array for example.

 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

 // we draw the cells in different stages. depth test is now enabled in all
 //  stages to prevent drawing errors. Depth func GL_LEQUAL makes sure all
 //  layers get rendered (they have same z values)
 // Maybe it should be set back to GL_LESS later?
 glDepthFunc(GL_LEQUAL);

 unsigned int usedTextures = 0;
 unsigned int renderedQuads = 0;
 for (unsigned int i = 0; i < groundTheme->textureCount(); i++) {
	if (i == 1) {
		glEnable(GL_BLEND);
	}
	glBindTexture(GL_TEXTURE_2D, map->currentTexture(i, boGame->advanceCallsCount()));
	unsigned int quads = renderCellsNow(renderCells, cellsCount, map->width() + 1, map->heightMap(), map->normalMap(), map->texMap(i));
	if (quads != 0) {
		usedTextures++;
	}
	renderedQuads += quads;
 }
 statistics()->setRenderedQuads(renderedQuads);
 statistics()->setUsedTextures(usedTextures);

 if (boConfig->enableColormap()) {
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	renderCellColors(renderCells, cellsCount, map->width(), map->colorMap()->colorMap(), map->heightMap());
	glPopAttrib();

 }

 glDisable(GL_BLEND);
 glColor4ub(255, 255, 255, 255);
}

unsigned int BoDefaultGroundRenderer::renderCellsNow(int* cells, int count, int cornersWidth, const float* heightMap, const float* normalMap, const unsigned char* texMapStart)
{
 const int offsetCount = 5;
 const float offset = 1.0f / (float)offsetCount;
 const float texOffsets[] = { 0.0f, 0.2f, 0.4f, 0.6f, 0.8f };  // texOffsets[x] = offset * x

 unsigned int renderedQuads = 0;
 glBegin(GL_QUADS);

 for (int i = 0; i < count; i++) {
	int x;
	int y;
	int w;
	int h;
	BoGroundRenderer::getCell(cells, i, &x, &y, &w, &h);
	if (x < 0 || y < 0 || w < 0 || h < 0) {
		boError() << k_funcinfo << x << " " << y << " " << w << " " << h << endl;
		continue;
	}

	const int cellOffset = y * cornersWidth + x;
	const unsigned char* texMapUpperLeft = texMapStart + cellOffset;
	const float* heightMapUpperLeft = heightMap + cellOffset;

	// offsets for corner arrays. just for convenience.
	const int upperRightOffset = w;
	const int lowerLeftOffset = cornersWidth * h;
	const int lowerRightOffset = cornersWidth * h + w;

	unsigned char upperLeftAlpha = *texMapUpperLeft;
	unsigned char upperRightAlpha = *(texMapUpperLeft + upperRightOffset);
	unsigned char lowerLeftAlpha = *(texMapUpperLeft + lowerLeftOffset);
	unsigned char lowerRightAlpha = *(texMapUpperLeft + lowerRightOffset);

	if ((upperLeftAlpha == 0) && (upperRightAlpha == 0) && (lowerLeftAlpha == 0) && (lowerRightAlpha == 0)) {
		continue;
	}

	GLfloat cellXPos = (float)x * BO_GL_CELL_SIZE;
	GLfloat cellYPos = -(float)y * BO_GL_CELL_SIZE;

	float upperLeftHeight = *heightMapUpperLeft;
	float upperRightHeight = *(heightMapUpperLeft + upperRightOffset);
	float lowerLeftHeight = *(heightMapUpperLeft + lowerLeftOffset);
	float lowerRightHeight = *(heightMapUpperLeft + lowerRightOffset);

	const float* upperLeftNormal = normalMap + (y * cornersWidth + x) * 3;
	const float* lowerLeftNormal = normalMap + ((y + 1) * cornersWidth + x) * 3;
	const float* lowerRightNormal = normalMap + ((y + 1) * cornersWidth + (x + 1)) * 3;
	const float* upperRightNormal = normalMap + (y * cornersWidth + (x + 1)) * 3;

	// Map cell's y-coordinate to range (offsetCount - 1) ... 0
	// FIXME: texy might be a bit confusing since we don't have texx
	int texy = offsetCount - (y % offsetCount) - 1;

	// the material settings are ignored when light disabled, the color is
	// ignored when light enabled.
	BoMaterial::setDefaultAlpha((float)upperLeftAlpha / 255.0f);
	glColor4ub(255, 255, 255, upperLeftAlpha);
	glNormal3fv(upperLeftNormal);
#warning FIXME: do we have to take w,h into account for the texture offsets? what do they do anyway?
	glTexCoord2f(texOffsets[x % offsetCount], texOffsets[texy % offsetCount] + offset);
	glVertex3f(cellXPos, cellYPos, upperLeftHeight);

	BoMaterial::setDefaultAlpha((float)lowerLeftAlpha / 255.0f);
	glColor4ub(255, 255, 255, lowerLeftAlpha);
	glNormal3fv(lowerLeftNormal);
	glTexCoord2f(texOffsets[x % offsetCount], texOffsets[texy % offsetCount]);
	glVertex3f(cellXPos, cellYPos - h * BO_GL_CELL_SIZE, lowerLeftHeight);

	BoMaterial::setDefaultAlpha((float)lowerRightAlpha / 255.0f);
	glColor4ub(255, 255, 255, lowerRightAlpha);
	glNormal3fv(lowerRightNormal);
	glTexCoord2f(texOffsets[x % offsetCount] + offset, texOffsets[texy % offsetCount]);
	glVertex3f(cellXPos + w * BO_GL_CELL_SIZE, cellYPos - h * BO_GL_CELL_SIZE, lowerRightHeight);

	BoMaterial::setDefaultAlpha((float)upperRightAlpha / 255.0f);
	glColor4ub(255, 255, 255, upperRightAlpha);
	glNormal3fv(upperRightNormal);
	glTexCoord2f(texOffsets[x % offsetCount] + offset, texOffsets[texy % offsetCount] + offset);
	glVertex3f(cellXPos + w * BO_GL_CELL_SIZE, cellYPos, upperRightHeight);

	renderedQuads++;
 }
 glEnd();
 BoMaterial::setDefaultAlpha(1.0f);

 return renderedQuads;
}

void BoDefaultGroundRenderer::renderCellColors(int* cells, int count, int width, const unsigned char* colorMap, const float* heightMap)
{
 const unsigned char alpha = 128;
 int cornersWidth = width + 1;

 glBegin(GL_QUADS);

 for (int i = 0; i < count; i++) {
	int x;
	int y;
	int w;
	int h;
	BoGroundRenderer::getCell(cells, i, &x, &y, &w, &h);

	int coloroffset = y * width + x;
	int heightoffset = y * cornersWidth + x;
	const unsigned char* color = colorMap + coloroffset * 3;
	const float* heightMapUpperLeft = heightMap + heightoffset;

	GLfloat cellXPos = (float)x * BO_GL_CELL_SIZE;
	GLfloat cellYPos = -(float)y * BO_GL_CELL_SIZE;

	float upperLeftHeight = *heightMapUpperLeft;
	float upperRightHeight = *(heightMapUpperLeft + w);
	float lowerLeftHeight = *(heightMapUpperLeft + cornersWidth * h);
	float lowerRightHeight = *(heightMapUpperLeft + cornersWidth * h + w);

	glColor4ub(color[0], color[1], color[2], alpha);
	glVertex3f(cellXPos, cellYPos, upperLeftHeight + 0.05);

	glColor4ub(color[0], color[1], color[2], alpha);
	glVertex3f(cellXPos, cellYPos - h * BO_GL_CELL_SIZE, lowerLeftHeight + 0.05);

	glColor4ub(color[0], color[1], color[2], alpha);
	glVertex3f(cellXPos + w * BO_GL_CELL_SIZE, cellYPos - h * BO_GL_CELL_SIZE, lowerRightHeight + 0.05);

	glColor4ub(color[0], color[1], color[2], alpha);
	glVertex3f(cellXPos + w * BO_GL_CELL_SIZE, cellYPos, upperRightHeight + 0.05);
 }
 glEnd();
}


