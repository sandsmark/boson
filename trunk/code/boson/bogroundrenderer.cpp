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

#include "bogroundrenderer.h"

#include "bodebug.h"
#include "bosonmap.h"
#include "bosonconfig.h"
#include "defines.h"
#include "cell.h"
#include "bosongroundtheme.h"
#include "bosontexturearray.h"
#include "bo3dtools.h"

// not nice in this file. we need it for boGame->status() == KGame::Init
// maybe we should require KGame not to be in init state before constructin the
// class
#include "boson.h"


// ugly in this file
#include "player.h"
//#include "playerio.h"

#include <GL/gl.h>
#include <math.h>

static void renderCellsNow(Cell** cells, int count, int mapCorners, float* heightMap, unsigned char* texMapStart);

class BoGroundRendererPrivate
{
public:
	BoGroundRendererPrivate()
	{
		mRenderCells = 0;

		mViewFrustum = 0;

		mLocalPlayer = 0;
	}

	//AB: we should use a float* here which can be used as vertex array. we
	//should store x,y,z and texture x,y there. for this we need to have
	//cells in a single texture!
	//--> then we could also use GL_QUAD_STRIP
	Cell** mRenderCells;
	int mRenderCellsSize; // max. number of cells in the array
	int mRenderCellsCount; // actual number of cells in the array


	const BoMatrix* mModelviewMatrix;
	const BoMatrix* mProjectionMatrix;
	const int* mViewport;
	const double* mViewFrustum;

	// how many cells got rendered by renderCells()
	unsigned int mRenderedCells;

	// FIXME: replace by PlayerIO
	Player* mLocalPlayer;
};

BoGroundRenderer::BoGroundRenderer()
{
 d = new BoGroundRendererPrivate;
 d->mRenderedCells = 0;
 d->mRenderCellsSize = 0;
 d->mRenderCellsCount = 0;
}

BoGroundRenderer::~BoGroundRenderer()
{
 delete[] d->mRenderCells;
 delete d;
}

unsigned int BoGroundRenderer::renderedCells() const
{
 return d->mRenderedCells;
}

void BoGroundRenderer::setMatrices(const BoMatrix* modelviewMatrix, const BoMatrix* projectionMatrix, const int* viewport)
{
 // AB: maybe we should maintain our own context class,
 // i.e. a class that contains all matrices.
 d->mModelviewMatrix = modelviewMatrix;
 d->mProjectionMatrix = projectionMatrix;
 d->mViewport = viewport;
}

void BoGroundRenderer::setViewFrustum(const double* viewFrustum)
{
 d->mViewFrustum = viewFrustum;
}

const double* BoGroundRenderer::viewFrustum() const
{
 return d->mViewFrustum;
}

void BoGroundRenderer::setLocalPlayer(Player* p)
{
 d->mLocalPlayer = p;
}

Player* BoGroundRenderer::localPlayer() const
{
 return d->mLocalPlayer;
}

void renderCellsNow(Cell** cells, int count, int cornersWidth, float* heightMap, unsigned char* texMapStart)
{
 // Texture offsets
 const int offsetCount = 5;
 const float offset = 1 / (float)offsetCount;
 const float texOffsets[] = { 0.0f, 0.2f, 0.4f, 0.6f, 0.8f };  // texOffsets[x] = offset * x

 glBegin(GL_QUADS);
 for (int i = 0; i < count; i++) {
	Cell* c = cells[i];
	int x = c->x();
	int y = c->y();

	int celloffset = y * cornersWidth + x;
	unsigned char* texMapUpperLeft = texMapStart + celloffset;
	float* heightMapUpperLeft = heightMap + celloffset;

	GLfloat cellXPos = (float)x * BO_GL_CELL_SIZE;
	GLfloat cellYPos = -(float)y * BO_GL_CELL_SIZE;

	float upperLeftHeight = *heightMapUpperLeft;
	float upperRightHeight = *(heightMapUpperLeft + 1);
	float lowerLeftHeight = *(heightMapUpperLeft + cornersWidth);
	float lowerRightHeight = *(heightMapUpperLeft + cornersWidth + 1);

	unsigned char upperLeftAlpha = *texMapUpperLeft;
	unsigned char upperRightAlpha = *(texMapUpperLeft + 1);
	unsigned char lowerLeftAlpha = *(texMapUpperLeft + cornersWidth);
	unsigned char lowerRightAlpha = *(texMapUpperLeft + cornersWidth + 1);

	// Map cell's y-coordinate to range (offsetCount - 1) ... 0
	y = offsetCount - (y % offsetCount) - 1;

	glColor4ub(255, 255, 255, upperLeftAlpha);
	glTexCoord2f(texOffsets[x % offsetCount], texOffsets[y % offsetCount] + offset);
	glVertex3f(cellXPos, cellYPos, upperLeftHeight);

	glColor4ub(255, 255, 255, lowerLeftAlpha);
	glTexCoord2f(texOffsets[x % offsetCount], texOffsets[y % offsetCount]);
	glVertex3f(cellXPos, cellYPos - BO_GL_CELL_SIZE, lowerLeftHeight);

	glColor4ub(255, 255, 255, lowerRightAlpha);
	glTexCoord2f(texOffsets[x % offsetCount] + offset, texOffsets[y % offsetCount]);
	glVertex3f(cellXPos + BO_GL_CELL_SIZE, cellYPos - BO_GL_CELL_SIZE, lowerRightHeight);

	glColor4ub(255, 255, 255, upperRightAlpha);
	glTexCoord2f(texOffsets[x % offsetCount] + offset, texOffsets[y % offsetCount] + offset);
	glVertex3f(cellXPos + BO_GL_CELL_SIZE, cellYPos, upperRightHeight);
 }
 glEnd();
}


void BoGroundRenderer::renderCells(const BosonMap* map)
{
 BO_CHECK_NULL_RET(map);
#if 0
 BosonTextureArray* textures = map->textures();
 if (!textures) {
	makeCurrent();
	// TODO: load a default theme
#if 0
	tiles->generateTextures();
	textures = tiles->textures();
#endif
	if (!textures) {
		boWarning() << k_funcinfo << "NULL textures for cells" << endl;
		return;
	}
 }
#endif

 if (d->mRenderCellsCount == 0) {
	// this happens either when we have to generate the list first or if no
	// cell is visible at all. The latter case isn't speed relevant, so we
	// can simply re-generate then.
	generateCellList(map);
 }

 BO_CHECK_NULL_RET(localPlayer());

 BO_CHECK_NULL_RET(map);
 BO_CHECK_NULL_RET(map->texMap());
 BO_CHECK_NULL_RET(map->heightMap());
 BO_CHECK_NULL_RET(map->groundTheme());
 BO_CHECK_NULL_RET(map->textures());

 BosonGroundTheme* groundTheme = map->groundTheme();
 float* heightMap = map->heightMap();
 BosonTextureArray* textures = map->textures();

 // AB: we can increase performance even more here. lets replace d->mRenderCells
 // by two array defining the coordinates of cells and the heightmap values.
 // we could use that as vertex array for example.
 int heightMapWidth = map->width() + 1;
 d->mRenderedCells = 0;

 int cellsCount = 0;
 Cell** renderCells = createVisibleCellList(&cellsCount, localPlayer());
 BO_CHECK_NULL_RET(renderCells);

 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

 // we draw the cells in different stages. the depth test must get enabled
 // before the last stage, so that the new information (i.e. the z pos of the
 // cells) get into the depth buffer.
 // we can safely disable the test completely for all other stages, as cells are
 // the first objects we render.
 glDisable(GL_DEPTH_TEST);

 for (unsigned int i = 0; i < groundTheme->textureCount(); i++) {
	GLuint tex = textures->texture(i);
	if (i == 1) {
		glEnable(GL_BLEND);
	} else if (i == groundTheme->textureCount() - 1) {
		glEnable(GL_DEPTH_TEST);
	}
	glBindTexture(GL_TEXTURE_2D, tex);
	renderCellsNow(renderCells, cellsCount, map->width() + 1, heightMap, map->texMap(i));
 }
 d->mRenderedCells += cellsCount;

 glDisable(GL_BLEND);
 glEnable(GL_DEPTH_TEST);

 renderCellGrid(renderCells, cellsCount, heightMap, heightMapWidth);

 delete[] renderCells;
}

void BoGroundRenderer::renderCellGrid(Cell** cells, int cellsCount, float* heightMap, int heightMapWidth)
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


Cell** BoGroundRenderer::createVisibleCellList(int* cells, Player* player)
{
 BO_CHECK_NULL_RET0(player);
 BO_CHECK_NULL_RET0(cells);
 Cell** renderCells = new Cell*[d->mRenderCellsCount]; // FIXME: store two arrays. one with x, one with y coordinate (or both in one array). don't store pointers to Cell
 int cellsCount = 0;
 for (int i = 0; i < d->mRenderCellsCount; i++) {
	Cell* c = d->mRenderCells[i];
	if (!c) {
		continue;
	}

	// AB: better solution: check *before* the cells get assigned to this
	// class. localPlayerIO() is *very* ugly in this class
	if (player->isFogged(c->x(), c->y())) {
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

void BoGroundRenderer::generateCellList(const BosonMap* map)
{
 // we need to regenerate the cell list whenever the modelview or the projection
 // matrix changes. then the displayed cells have most probably changed.

 if (!map) {
	delete[] d->mRenderCells;
	d->mRenderCells = 0;
	d->mRenderCellsSize = 0;
	d->mRenderCellsCount = 0;
	return;
 }

 if (boGame->gameStatus() == KGame::Init) {
	// we construct the display before the map is received
	return;
 }

 // re-generate the list of to-be-rendered cells:
 Cell* allCells = map->cells();
 if (!allCells) {
	boError() << k_funcinfo << "NULL cells!" << endl;
	return;
 }
 float maxX = 0.0f, maxY = 0.0f;
 float minX = 0.0f, minY = 0.0f;
 calculateWorldRect(QRect(QPoint(0, 0), QPoint(d->mViewport[2], d->mViewport[3])),
		map->width(), map->height(),
		&minX, &minY, &maxX, &maxY);
 minY *= -1;
 maxY *= -1;

 // if everything went fine we need to add those cells that are in the
 // ((minX,minY),(maxX,maxY)) rectangle only.

 int cellMinX = (int)(minX / BO_GL_CELL_SIZE); // AB: *no* +1 for min values!
 int cellMaxX = (int)(maxX / BO_GL_CELL_SIZE) + 1; // +1 because of a modulo (very probably at this point)
 int cellMinY = (int)(minY / BO_GL_CELL_SIZE);
 int cellMaxY = (int)(maxY / BO_GL_CELL_SIZE) + 1;

 // finally we ensure that the cell values are valid, too.
 // after these lines we mustn't modify cellM* anymore!
 cellMinX = QMAX(cellMinX, 0);
 cellMinY = QMAX(cellMinY, 0);
 cellMaxX = QMAX(cellMaxX, 0);
 cellMaxY = QMAX(cellMaxY, 0);
 cellMinX = QMIN(cellMinX, (int)map->width() - 1);
 cellMinY = QMIN(cellMinY, (int)map->height() - 1);
 cellMaxX = QMIN(cellMaxX, (int)map->width() - 1);
 cellMaxY = QMIN(cellMaxY, (int)map->height() - 1);

 int size = (cellMaxX - cellMinX + 1) * (cellMaxY - cellMinY + 1);
 size = QMIN((int)(map->width() * map->height()), size);
 if (size > d->mRenderCellsSize) {
	delete[] d->mRenderCells;
	d->mRenderCells = new Cell*[size];
	d->mRenderCellsSize = size;
 }

 // all cells between those min/max values above might be visible. unfortunately
 // we need to add *all* visible cells to our list, but we need to add as *few*
 // as possible.
 // we could improve speed (important for big maps!) here if we would group
 // several cells into a single sphereInFrustum() call for example.
 //
 // note that the current implementation is very fast at default zoom, but if
 // you zoom out (and therefore there are lots of cells visible) it is still too
 // slow.

 int count = 0;
 Cell* c;
 GLfloat glX, glY, minz, maxz, z;
 for (int x = cellMinX; x <= cellMaxX; x++) {
	for (int y = cellMinY; y <= cellMaxY; y++) {
		// WARNING: x,y MUST be valid!!! there is *no* additional check
		// here!
		c = &allCells[map->cellArrayPos(x, y)];

		glX = (float)c->x() * BO_GL_CELL_SIZE + BO_GL_CELL_SIZE / 2;
		glY = -((float)c->y() * BO_GL_CELL_SIZE + BO_GL_CELL_SIZE / 2);

		// Calculate average height and radius of bounding sphere of the cell
		// Reset variables
		minz = 1000.0f;
		maxz = -1000.0f;

		for (int i = x; i <= x + 1; i++) {
			for (int j = y; j <= y + 1; j++) {
				minz = QMIN(minz, map->heightAtCorner(i, j));
				maxz = QMAX(maxz, map->heightAtCorner(i, j));
			}
		}
		z = (maxz - minz) / 2;

		if (Bo3dTools::sphereInFrustum(viewFrustum(), BoVector3(glX, glY, (minz + maxz) / 2), sqrt(2 * (BO_GL_CELL_SIZE/2) * (BO_GL_CELL_SIZE/2) + z * z))) {
			// AB: instead of storing the cell here we should store
			// cell coordinates and create a vertex array with that
			d->mRenderCells[count] = c;
			count++;
		}
	}
 }
 d->mRenderCellsCount = count;
}

void BoGroundRenderer::calculateWorldRect(const QRect& rect, int mapWidth, int mapHeight, float* minX, float* minY, float* maxX, float* maxY)
{
 GLfloat posX, posY;
 GLfloat posZ;
 Bo3dTools::mapCoordinates(*d->mModelviewMatrix, *d->mProjectionMatrix, d->mViewport, rect.topLeft(), &posX, &posY, &posZ);
 *maxX = *minX = posX;
 *maxY = *minY = -posY;
 Bo3dTools::mapCoordinates(*d->mModelviewMatrix, *d->mProjectionMatrix, d->mViewport, rect.topRight(), &posX, &posY, &posZ);
 *maxX = QMAX(*maxX, posX);
 *maxY = QMAX(*maxY, -posY);
 *minX = QMIN(*minX, posX);
 *minY = QMIN(*minY, -posY);
 Bo3dTools::mapCoordinates(*d->mModelviewMatrix, *d->mProjectionMatrix, d->mViewport, rect.bottomLeft(), &posX, &posY, &posZ);
 *maxX = QMAX(*maxX, posX);
 *maxY = QMAX(*maxY, -posY);
 *minX = QMIN(*minX, posX);
 *minY = QMIN(*minY, -posY);
 Bo3dTools::mapCoordinates(*d->mModelviewMatrix, *d->mProjectionMatrix, d->mViewport, rect.bottomRight(), &posX, &posY, &posZ);
 *maxX = QMAX(*maxX, posX);
 *maxY = QMAX(*maxY, -posY);
 *minX = QMIN(*minX, posX);
 *minY = QMIN(*minY, -posY);

 *maxX = QMAX(0, *maxX);
 *maxY = QMAX(0, *maxY);
 *minX = QMAX(0, *minX);
 *minY = QMAX(0, *minY);
 *maxX = QMIN((mapWidth - 1) * BO_GL_CELL_SIZE, *maxX);
 *minX = QMIN((mapWidth - 1) * BO_GL_CELL_SIZE, *minX);
 *maxY = QMIN((mapHeight - 1) * BO_GL_CELL_SIZE, *maxY);
 *minY = QMIN((mapHeight - 1) * BO_GL_CELL_SIZE, *minY);
 *minY *= -1;
 *maxY *= -1;
}


