/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "boveryfastgroundrenderer.h"
#include "boveryfastgroundrenderer.moc"

#include "../../bomemory/bodummymemory.h"
#include "bogroundrendererbase.h"
#include "../gameengine/boquadtreenode.h"
#include "../gameengine/bosonmap.h"
#include "../defines.h"
#include "../gameengine/cell.h"
#include "../gameengine/bosongroundtheme.h"
#include "../bosongroundthemedata.h"
#include "../gameengine/boson.h"
#include "../botexture.h"
#include "../bosonconfig.h"
#include <bogl.h>
#include <bodebug.h>

#warning remove
#include "../bosonprofiling.h"

#include <qimage.h>

#include <math.h>

class BoVeryFastGroundRendererCellListLOD : public BoGroundRendererCellListLOD
{
public:
	BoVeryFastGroundRendererCellListLOD()
		: BoGroundRendererCellListLOD()
	{
	}

	virtual bool doLOD(const BosonMap* map, const BoGroundRendererQuadTreeNode* node) const
	{
		if (!node) {
			return false;
		}
		const int count = node->nodeSize();

		if (count <= 4) {
			return true;
		}

		const BoPlane& plane = viewFrustum()->near();

		// FIXME: distanceFromPlane() tests the distance of all 4 corners of the rect
		// only. this is perfectly legal if the whole rect is inside the viewfrustum,
		// however if it is partially visible only, this may not be sufficient!
		float d = distanceFromPlane(plane, node, map);
		if (d > 240.0f && count <= 64 ||
				d > 120.0f && count <= 16 ||
				d > 40.0f && count <= 8 ||
				d > 20.0f && count <= 2) {
//			boDebug() << d << endl;
			return true;
		}
		return false;
	}
};


BoVeryFastGroundRenderer::BoVeryFastGroundRenderer()
	: BoGroundRendererBase()
{
 mCurrentMap = 0;
 mCellTextures = 0;
 mCurrentThemeData = 0;
 mThemeColors = 0;
}

bool BoVeryFastGroundRenderer::initGroundRenderer()
{
 if (!BoGroundRendererBase::initGroundRenderer()) {
	return false;
 }

 setLODObject(new BoVeryFastGroundRendererCellListLOD());

 return true;
}

BoVeryFastGroundRenderer::~BoVeryFastGroundRenderer()
{
 delete[] mThemeColors;
}

void BoVeryFastGroundRenderer::renderVisibleCellsStart(const BosonMap* map)
{
 bool textureFOW = boConfig->boolValue("TextureFOW");
 boConfig->setBoolValue("TextureFOW", false);

 BoGroundRendererBase::renderVisibleCellsStart(map);

 boConfig->setBoolValue("TextureFOW", textureFOW);
}

void BoVeryFastGroundRenderer::renderVisibleCellsStop(const BosonMap* map)
{
 bool textureFOW = boConfig->boolValue("TextureFOW");
 boConfig->setBoolValue("TextureFOW", false);

 BoGroundRendererBase::renderVisibleCellsStop(map);

 boConfig->setBoolValue("TextureFOW", textureFOW);
}

void BoVeryFastGroundRenderer::renderVisibleCells(int* renderCells, unsigned int cellsCount, const BosonMap* map, RenderFlags flags)
{
 BO_CHECK_NULL_RET(renderCells);
 BO_CHECK_NULL_RET(map);
 BO_CHECK_NULL_RET(map->texMap());
 BO_CHECK_NULL_RET(mHeightMap2);
 BO_CHECK_NULL_RET(map->groundTheme());

 PROFILE_METHOD

 updateMapCache(map);
 BO_CHECK_NULL_RET(currentGroundThemeData());
 updateGroundThemeCache(currentGroundThemeData());
 glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

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
	cellTextures[i] = mCellTextures[BoMapCornerArray::arrayPos(x, y, map->width())];
 }

 unsigned int renderedQuads = 0;

 const int offsetCount = 5;
 const int cornersWidth = map->width() + 1;

 glPushAttrib(GL_ENABLE_BIT);
 glDisable(GL_TEXTURE_2D);
 glBegin(GL_QUADS);
 for (unsigned int i = 0; i < map->groundTheme()->groundTypeCount(); i++) {
	glColor3ubv(mThemeColors + (i * 4));
	for (unsigned int j = 0; j < cellsCount; j++) {
		if (cellTextures[j] != i) {
			continue;
		}
		int x;
		int y;
		int w;
		int h;
		BoGroundRenderer::getCell(renderCells, j, &x, &y, &w, &h);

		int celloffset = y * cornersWidth + x;
		const float* heightMapUpperLeft = heightMap + celloffset;

		GLfloat cellXPos = (float)x;
		GLfloat cellYPos = -(float)y;

		float upperLeftHeight = *heightMapUpperLeft;
		float upperRightHeight = *(heightMapUpperLeft + w);
		float lowerLeftHeight = *(heightMapUpperLeft + cornersWidth * h);
		float lowerRightHeight = *(heightMapUpperLeft + cornersWidth * h + w);

		// Map cell's y-coordinate to range (offsetCount - 1) ... 0
		y = offsetCount - (y % offsetCount) - 1;

		glVertex3f(cellXPos, cellYPos, upperLeftHeight);
		glVertex3f(cellXPos, cellYPos - h, lowerLeftHeight);
		glVertex3f(cellXPos + w, cellYPos - h, lowerRightHeight);
		glVertex3f(cellXPos + w, cellYPos, upperRightHeight);

		renderedQuads++;
	}
 }
 glEnd();
 glPopAttrib();

 delete[] cellTextures;

 statistics()->setRenderedQuads(renderedQuads);
 statistics()->setUsedTextures(0);

}

void BoVeryFastGroundRenderer::updateMapCache(const BosonMap* map)
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

void BoVeryFastGroundRenderer::updateGroundThemeCache(const BosonGroundThemeData* theme)
{
 if (mCurrentThemeData == theme) {
	return;
 }
 BO_CHECK_NULL_RET(theme);
 delete[] mThemeColors;
 mThemeColors = new GLubyte[theme->groundTypeCount() * 4];
 boDebug() << k_funcinfo << "finding average color of ground textures..." << endl;
 // TODO: maybe use minimap's color isntead?
 for (unsigned int i = 0; i < theme->groundTypeCount(); i++) {
	BO_CHECK_NULL_RET(theme->groundTypeData(i));
	const BoTextureArray* a = theme->groundTypeData(i)->textures;
	mThemeColors[i * 4 + 0] = 255;
	mThemeColors[i * 4 + 1] = 0;
	mThemeColors[i * 4 + 2] = 0;
	mThemeColors[i * 4 + 3] = 255;
	if (!a) {
		BO_NULL_ERROR(a);
		continue;
	}
	const BoTexture* t = a->texture(0);
	if (!t) {
		BO_NULL_ERROR(t);
		continue;
	}
	if (t->filePath().isEmpty()) {
		boError() << k_funcinfo << "empty texture filename" << endl;
		continue;
	}
	QImage img;
	if (!img.load(t->filePath())) {
		boError() << k_funcinfo << "could not load file "<< t->filePath() << endl;
		continue;
	}
	unsigned int r = 0;
	unsigned int g = 0;
	unsigned int b = 0;
	for (int x = 0; x < img.width(); x++) {
		for (int y = 0; y < img.height(); y++) {
			QRgb rgb = img.pixel(x, y);
			r += qRed(rgb);
			g += qGreen(rgb);
			b += qBlue(rgb);
		}
	}
	r /= (img.width() * img.height());
	g /= (img.width() * img.height());
	b /= (img.width() * img.height());
	mThemeColors[i * 4 + 0] = r;
	mThemeColors[i * 4 + 1] = g;
	mThemeColors[i * 4 + 2] = b;
 }
 boDebug() << k_funcinfo << "finding average color of ground textures done" << endl;
 mCurrentThemeData = theme;
}

