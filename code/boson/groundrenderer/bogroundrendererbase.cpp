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

#include "bogroundrendererbase.h"
#include "bogroundrendererbase.moc"

#include "../../bomemory/bodummymemory.h"
#include "boquadtreenode.h"
#include "../bosonmap.h"
#include "../bosonprofiling.h"
#include "../defines.h"
#include "../cell.h"
#include "../bo3dtools.h"
#include "../bosonconfig.h" // WARNING: groundrenderer needs to be re-installed if bosonconfig.h changes!
#include "../botexture.h"
#include "../playerio.h"
#include "../bosonviewdata.h"
#include "../bosongroundtheme.h"
#include "bocolormaprenderer.h"
#include <bogl.h>
#include <bodebug.h>

// not nice in this file. we need it for boGame->status() == KGame::Init
// maybe we should require KGame not to be in init state before constructin the
// class
#include "../boson.h"

#include <math.h>

#define FIX_EDGES 1

static int g_cellsVisibleCalls = 0;

// Whether to use glTexSubImage2D() to update texture. It seems to be buggy.
#define USE_TEXSUBIMAGE

/**
 * This class takes care of building a list (an array in this case) of cells
 * that are visible in the current @ref viewFrustum.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class CellListBuilder
{
public:
	CellListBuilder()
	{
		mViewFrustum = 0;
		mViewport = 0;

		mMinX = 0;
		mMaxX = 0;
		mMinY = 0;
		mMaxY = 0;

		mLODObject = 0;
		setLODObject(new BoGroundRendererCellListLOD());
	}
	virtual ~CellListBuilder()
	{
		delete mLODObject;
	}

	/**
	 * Set an LOD object. This object is used to decide whether (and when)
	 * LOD should be used.
	 *
	 * The object is deleted on destruction of this object.
	 **/
	void setLODObject(BoGroundRendererCellListLOD* lod)
	{
		delete mLODObject;
		mLODObject = lod;
		if (mLODObject) {
			mLODObject->setViewFrustum(viewFrustum());
		}
	}

	void copyHeightMap(float* vertexArray, float* heightMap, const BosonMap* map);

	void setViewport(const int* p) { mViewport = p; }
	const int* viewport() const { return mViewport; }
	void setViewFrustum(const BoFrustum* f)
	{
		mViewFrustum = f;
		if (mLODObject) {
			mLODObject->setViewFrustum(viewFrustum());
		}
	}
	const BoFrustum* viewFrustum() const { return mViewFrustum; }

	/**
	 * @param renderCells The original list of cells. You should use this
	 * if @p renderCellsSize is big enough for your needs. Return this
	 * pointer, if you use this array
	 * @param renderCellsSize The size of the @p renderCells array. Set it
	 * to the new value, if you return a pointer which is not equal to @p
	 * renderCells.
	 * @param renderCellsCount Set this to the number of cells in @p
	 * renderCells that are used (starting from 0), i.e. should be rendered.
	 **/
	virtual int* generateCellList(const BosonMap* map, int* renderCells, int* renderCellsSize, unsigned int* renderCellsCount) = 0;

protected:
	virtual void copyCustomHeightMap(float* vertexArray, float* heightMap, const BosonMap* map) = 0;

protected:
	int mMinX;
	int mMaxX;
	int mMinY;
	int mMaxY;
	BoGroundRendererCellListLOD* mLODObject;

private:
	const BoFrustum* mViewFrustum;
	const int* mViewport;
};



/**
 * This class uses a tree find out whether cells are visible. Whenever @ref
 * addVisibleCells is called on a valid rect, it calls itself again 4 times. Once for
 * the top-left quarter, the top-right, bottom-left and bottom-right quarter of
 * the rect.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class CellListBuilderTree : public CellListBuilder
{
public:
	CellListBuilderTree() : CellListBuilder()
	{
		mCount = 0;
		mMap = 0;
		mRoot = 0;
	}
	~CellListBuilderTree()
	{
		for (unsigned int i = 0; i < mLeafs.size(); i++) {
			delete mLeafs[i];
		}
		delete mRoot;
	}

	virtual int* generateCellList(const BosonMap* map, int* renderCells, int* renderCellsSize, unsigned int* renderCellsCount);


protected:
	/**
	 * This is the main method of this class. It checks (recursively)
	 * whether the cells in ((x,y),(x2,y2)) are visible and adds all visible
	 * cells to @p cells.
	 **/
	void addVisibleCells(int* cells, const BoQuadTreeNode* node, int depth = 0);

	/**
	 * @return Whether the cells in the rect of the node are visible.
	 * If they are visible, @p partially is set to FALSE, when all cells are
	 * visible, otherwise to TRUE (rect is partially visible only). If no
	 * cell is visible (i.e. this returns FALSE) @p partially is set to
	 * FALSE.
	 **/
	bool cellsVisible(const BoQuadTreeNode* node, bool* partially) const;

	/**
	 * Add all cells in the rect of the node to @p cells
	 **/
	void addCells(int* cells, const BoQuadTreeNode* node, int depth);

	void recreateTree(const BosonMap* map);

	virtual void copyCustomHeightMap(float* vertexArray, float* heightMap, const BosonMap* map);

private:
	// these variables are valid only while we are in generateCellList() !
	const BosonMap* mMap;
	unsigned int mCount;

	BoQuadTreeNode* mRoot;

	QMemArray< QPtrList<const BoQuadTreeNode>* > mLeafs;
};

void CellListBuilder::copyHeightMap(float* vertexArray, float* heightMap, const BosonMap* map)
{
 BO_CHECK_NULL_RET(heightMap);
 BO_CHECK_NULL_RET(map);
 BosonProfiler prof("copyHeightMap");
 if (mMinX < 0 || mMinY < 0) {
	boError() << k_funcinfo << "minx=" << mMinX << " miny=" << mMinY << endl;
	mMinX = mMinY = 0;
	mMaxX = map->width() - 1;
	mMaxY = map->height() - 1;

	for (int x = mMinX; x <= mMaxX + 1; x++) { // +1 because we need _corners_ not cells
		for (int y = mMinY; y <= mMaxY + 1; y++) {
			// AB: note: we copy to both, heightMap and vertexArray.
			// heightMap is deprecated from now on (default renderer
			// does not use it anymore!), but some renderers may
			// still use it.
			// it (mHeightMap2) should be removed
			heightMap[map->cornerArrayPos(x, y)] = map->heightAtCorner(x, y);
			int index = map->cornerArrayPos(x, y) * 3 + 2;
			vertexArray[index] = map->heightAtCorner(x, y);
		}
	}
 }
 copyCustomHeightMap(vertexArray, heightMap, map);
}

void CellListBuilderTree::copyCustomHeightMap(float* vertexArray, float* heightMap, const BosonMap* map)
{
 BosonProfiler prof("copyCustomHeightMap");
 if (mLeafs.size() <= 0) {
	return;
 }
 for (int i = (int)mLeafs.size() - 1; i >= 0; i--) {
	QPtrList<const BoQuadTreeNode>* list = mLeafs[i];
	if (!list || list->isEmpty()) {
		continue;
	}
	QPtrListIterator<const BoQuadTreeNode> it(*list);
	while (it.current()) {
		const BoQuadTreeNode* node = it.current();
		++it;
		const int l = node->left();
		const int r = node->right();
		const int t = node->top();
		const int b = node->bottom();
		const float tl = map->heightAtCorner(l, t);
		const float bl = map->heightAtCorner(l, b + 1);
		const float tr = map->heightAtCorner(r + 1, t);
		const float br = map->heightAtCorner(r + 1, b + 1);

		const int w = (r - l) + 1;
		const int h = (b - t) + 1;
		const float topStep = (tr - tl) / w;
		const float bottomStep = (br - bl) / w;
		const float leftStep = (bl - tl) / h;
		const float rightStep = (br - tr) / h;

		// top and bottom border
		for (int x = 0; x <= w; x++) {
			const int pos1 = map->cornerArrayPos(l + x, t);
			const int pos2 = map->cornerArrayPos(l + x, b + 1);
			const float h1 = tl + topStep * x;
			const float h2 = bl + bottomStep * x;
			heightMap[pos1] = h1;
			heightMap[pos2] = h2;
			vertexArray[pos1 * 3 + 2] = h1;
			vertexArray[pos2 * 3 + 2] = h2;
		}

		// just like above, but for left/right border
		// note that y==0 and y==h are not required, as it is covered
		// above anyqay
		for (int y = 1; y < h; y++) {
			const int pos1 = map->cornerArrayPos(l, t + y);
			const int pos2 = map->cornerArrayPos(r + 1, t + y);
			const float h1 = tl + leftStep * y;
			const float h2 = tr + rightStep * y;
			heightMap[pos1] = h1;
			heightMap[pos2] = h2;
			vertexArray[pos1 * 3 + 2] = h1;
			vertexArray[pos2 * 3 + 2] = h2;
		}
	}
 }
}

int* CellListBuilderTree::generateCellList(const BosonMap* map, int* origRenderCells, int* renderCellsSize, unsigned int* renderCellsCount)
{
 mMinX = mMinY = -1;
 mMaxX = mMaxY = 0;
 if (!map) {
	BO_NULL_ERROR(map);
	return origRenderCells;
 }
 BosonProfiler prof("generateCellList");
 int* renderCells = origRenderCells;
 if (*renderCellsSize < (int)(map->width() * map->height())) {
	// we don't know in advance how many cells we might need, so we allocate
	// width * height
	*renderCellsSize = map->width() * map->height();
	renderCells = BoGroundRenderer::makeCellArray(*renderCellsSize);
 }
 if (mMap != map) {
	mMap = 0;
	boDebug() << k_funcinfo << "recreating map tree" << endl;
	BosonProfiler prof("mapTreeGeneration");
	recreateTree(map);
 }
 mMap = map;
 mCount = 0;

 for (int i = 0; i < (int)mLeafs.size(); i++) {
	QPtrList<const BoQuadTreeNode>* list = mLeafs[i];
	if (list) {
		list->clear();
	}
 }

 g_cellsVisibleCalls = 0;
 addVisibleCells(renderCells, mRoot, 0);
// boDebug() << k_funcinfo << g_cellsVisibleCalls << " calls - will render cells: " << mCount << endl;

 *renderCellsCount = mCount;
// mMap = 0;
 mCount = 0;
 return renderCells;
}

void CellListBuilderTree::addCells(int* cells, const BoQuadTreeNode* node, int depth)
{
 if (!node) {
	return;
 }
 const int l = node->left();
 const int t = node->top();
 const int r = node->right();
 const int b = node->bottom();
 if (mLODObject && mLODObject->doLOD(mMap, node)) {
	BoGroundRenderer::setCell(cells, mCount, l, t, r - l + 1, b - t + 1);
	mCount++;
	if ((int)mLeafs.size() < depth + 1) {
		int s = mLeafs.size();
		mLeafs.resize(depth + 1);
		for (int i = s; i < (int)mLeafs.size(); i++) {
			mLeafs[i] = new QPtrList<const BoQuadTreeNode>();
		}
	}
	mLeafs[depth]->append(node);
	if (l < mMinX || mMinX < 0) {
		mMinX = l;
	}
	if (r > mMaxX || mMaxX < 0) {
		mMaxX = r;
	}
	if (t < mMinY || mMinY < 0) {
		mMinY = t;
	}
	if (b > mMaxY || mMaxY < 0) {
		mMaxY = b;
	}
 } else {
	addCells(cells, node->topLeftNode(), depth + 1);
	addCells(cells, node->topRightNode(), depth + 1);
	addCells(cells, node->bottomLeftNode(), depth + 1);
	addCells(cells, node->bottomRightNode(), depth + 1);
 }
}

bool CellListBuilderTree::cellsVisible(const BoQuadTreeNode* node, bool* partially) const
{
 g_cellsVisibleCalls++;
 if (!node) {
	*partially = false;
	return false;
 }
 const int x = node->left();
 const int x2 = node->right();
 const int y = node->top();
 const int y2 = node->bottom();

 int w = (x2 + 1) - x; // + 1 because we need the right border of the cell!
 int h = (y2 + 1) - y;
 if (w * h <= 4) {
	*partially = false;
	return true;
 }
 float hmid = (float)x + ((float)w) / 2.0f;
 float vmid = (float)y + ((float)h) / 2.0f;

 float topLeftZ = mMap->heightAtCorner(x, y);
 float topRightZ = mMap->heightAtCorner(x2 + 1, y);
 float bottomRightZ = mMap->heightAtCorner(x2 + 1, y2 + 1);
 float bottomLeftZ = mMap->heightAtCorner(x, y2 + 1);
 float z = topLeftZ + topRightZ + bottomRightZ + bottomLeftZ;
 z /= 4;
 // calculate the distance from the center (hmid,vmid,z) to all 4
 // corners. the greatest distance is our radius.
 // note that we use the dotProduct() instead of length(), as it is
 // faster. sqrt(dotProduct()) is exactly the length.
 float r1 = BoVector3Float(hmid - (float)x,
		vmid - (float)y, z - topLeftZ).dotProduct();
 float r2 = BoVector3Float(hmid - ((float)x + (float)w),
		vmid - (float)y, z - topRightZ).dotProduct();
 float r3 = BoVector3Float(hmid - ((float)x + (float)w),
		vmid - ((float)y + (float)h), z - bottomRightZ).dotProduct();
 float r4 = BoVector3Float(hmid - (float)x,
		vmid - ((float)y + (float)h), z - bottomLeftZ).dotProduct();

 float radius = QMAX(r1, r2);
 radius = QMAX(radius, r3);
 radius = QMAX(radius, r4);
 radius = sqrtf(radius); // turn dotProduct() into length()
 BoVector3Float center(hmid, -vmid, z);

 int ret = viewFrustum()->sphereCompleteInFrustum(center, radius);
 if (ret == 0) {
	*partially = false;
	return false;
 } else if (ret == 2 || (w == 1 && h == 1)) {
	*partially = false;
 } else {
	*partially = true;
 }
 return true;
}

void CellListBuilderTree::addVisibleCells(int* cells, const BoQuadTreeNode* node, int depth)
{
 if (!node) {
	return;
 }
 bool partially = false;
 if (cellsVisible(node, &partially)) {
	if (!partially || (mLODObject && mLODObject->doLOD(mMap, node))) {
		// all cells visible
		addCells(cells, node, depth);
	} else {
		addVisibleCells(cells, node->topLeftNode(), depth + 1);
		addVisibleCells(cells, node->topRightNode(), depth + 1);
		addVisibleCells(cells, node->bottomLeftNode(), depth + 1);
		addVisibleCells(cells, node->bottomRightNode(), depth + 1);
	}
 }
}

void CellListBuilderTree::recreateTree(const BosonMap* map)
{
 BO_CHECK_NULL_RET(map);

 delete mRoot;
 mRoot = BoQuadTreeNode::createTree(map->width(), map->height());
}


bool BoGroundRendererCellListLOD::doLOD(const BosonMap* map, const BoQuadTreeNode* node) const
{
 if (!node) {
	return false;
 }
 const int count = node->nodeSize();
 if (count == 1) {
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
//	boDebug() << d << endl;
	return true;
 }
 return false;
}

float BoGroundRendererCellListLOD::distanceFromPlane(const BoPlane& plane, const BoQuadTreeNode* node, const BosonMap* map) const
{
 const int l = node->left();
 const int t = node->top();
 const int r = node->right();
 const int b = node->bottom();
 const float x = (float)l;
 const float y = (float)-t;
 const float x2 = ((float)r) + 1.0f;
 const float y2 = ((float)-b) - 1.0f;
 const float zTopLeft = map->heightAtCorner(l, t);
 const float zTopRight = map->heightAtCorner(r + 1, t);
 const float zBottomLeft = map->heightAtCorner(l, b + 1);
 const float zBottomRight = map->heightAtCorner(r + 1, b + 1);
 const float d1 = plane.distance(BoVector3Float(x, y, zTopLeft));
 const float d2 = plane.distance(BoVector3Float(x2, y, zTopRight));
 const float d3 = plane.distance(BoVector3Float(x, y2, zBottomLeft));
 const float d4 = plane.distance(BoVector3Float(x2, y2, zBottomRight));
 float d = QMAX(d1, d2);
 d = QMAX(d, d3);
 d = QMAX(d, d4);
 return d;
}


FogTexture::~FogTexture()
{
 delete[] mFogTextureData;
 delete mFogTexture;
}

void FogTexture::start(const BosonMap* map)
{
 if (boConfig->boolValue("TextureFOW")) {
	// Enable fog texture (TU 1)
	initFogTexture(map);
	boTextureManager->activateTextureUnit(1);
	updateFogTexture();
	boTextureManager->bindTexture(mFogTexture);
	// Use automatic texcoord generation to map fog texture to cells
	const float texPlaneS[] = { 1.0, 0.0, 0.0, 0.0 };
	const float texPlaneT[] = { 0.0, 1.0, 0.0, 0.0 };
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
	glScalef(1.0f / mFogTextureDataW, 1.0f / mFogTextureDataH, 1.0f);
	glScalef(1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	boTextureManager->activateTextureUnit(0);
 }
}

void FogTexture::stop(const BosonMap*)
{
 if (boConfig->boolValue("TextureFOW")) {
	// end using fog texture
	boTextureManager->activateTextureUnit(1);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	boTextureManager->unbindTexture();
	{
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
	}
	boTextureManager->activateTextureUnit(0);
 }
}

void FogTexture::initFogTexture(const BosonMap* map)
{
 if (mLastMapWidth != map->width() || mLastMapHeight != map->height()) {
	// Map size has changed. Delete fog texture (new one will be created)
	delete[] mFogTextureData;
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
				if (!mSmoothEdges || (x > 1 && y > 1 && x < mLastMapWidth && y < mLastMapHeight)) {
					value = 255;
				}
			}
			mFogTextureData[(y * w + x) * 4 + 0] = value;
			mFogTextureData[(y * w + x) * 4 + 1] = value;
			mFogTextureData[(y * w + x) * 4 + 2] = value;
			mFogTextureData[(y * w + x) * 4 + 3] = 255;
		}
	}
	mFogTexture = new BoTexture(mFogTextureData, mFogTextureDataW, mFogTextureDataH,
			BoTexture::FilterLinear | BoTexture::FormatRGBA);

	mFogTextureDirty = false;

	// Update dirty area
	mFogTextureDirtyAreaX1 = 1000000;
	mFogTextureDirtyAreaY1 = 1000000;
	mFogTextureDirtyAreaX2 = -1;
	mFogTextureDirtyAreaY2 = -1;
 }
}

void FogTexture::updateFogTexture()
{
 if (!mFogTextureDirty) {
	// No need to update
	return;
 }

#ifdef USE_TEXSUBIMAGE
 mFogTexture->bind();
 // Because of (possible) texture compression, we can't update a single pixel
 //  of the texture. Instead, we have to update the whole 4x4 block that the
 //  pixel is in.
 int blockx1 = (mFogTextureDirtyAreaX1 + 1) / 4;
 int blocky1 = (mFogTextureDirtyAreaY1 + 1) / 4;
 int blockx2 = (mFogTextureDirtyAreaX2 + 1) / 4;
 int blocky2 = (mFogTextureDirtyAreaY2 + 1) / 4;
 unsigned int x = blockx1 * 4;
 unsigned int y = blocky1 * 4;
 int w = (blockx2 - blockx1 + 1) * 4;
 int h = (blocky2 - blocky1 + 1) * 4;
 // Create temporary array for the 4x4 block
 unsigned char blockdata[w * h * 4];
 // Copy data from mFogTextureData to blockdata
 for(int i = 0; i < w; i++) {
	for(int j = 0; j < h; j++) {
		// Use black if the point is not on the map
		if (((x + i) >= mLastMapWidth) || ((y + j) >= mLastMapHeight)) {
			blockdata[((j * w) + i) * 4 + 0] = blockdata[((j * w) + i) * 4 + 1] =
					blockdata[((j * w) + i) * 4 + 2] = blockdata[((j * w) + i) * 4 + 3] = 0;
		}
		blockdata[((j * w) + i) * 4 + 0] = mFogTextureData[((y + j) * mFogTextureDataW + (x + i)) * 4 + 0];
		blockdata[((j * w) + i) * 4 + 1] = mFogTextureData[((y + j) * mFogTextureDataW + (x + i)) * 4 + 1];
		blockdata[((j * w) + i) * 4 + 2] = mFogTextureData[((y + j) * mFogTextureDataW + (x + i)) * 4 + 2];
		blockdata[((j * w) + i) * 4 + 3] = mFogTextureData[((y + j) * mFogTextureDataW + (x + i)) * 4 + 3];
	}
 }
 // Update texture
 glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, blockdata);

#else
 delete mFogTexture;
 // Create new fog texture
 mFogTexture = new BoTexture(mFogTextureData, mFogTextureDataW, mFogTextureDataH,
		BoTexture::FilterLinear | BoTexture::FormatRGBA);
#endif

 mFogTextureDirty = false;

 // Update dirty area
 mFogTextureDirtyAreaX1 = 1000000;
 mFogTextureDirtyAreaY1 = 1000000;
 mFogTextureDirtyAreaX2 = -1;
 mFogTextureDirtyAreaY2 = -1;
}

void FogTexture::cellChanged(int x1, int y1, int x2, int y2)
{
 if (!boConfig->boolValue("TextureFOW")) {
	return;
 }
 if (!mFogTextureData) {
	return;
 }
 x1 = QMAX(x1, 1);
 y1 = QMAX(y1, 1);
 x2 = QMIN(x2, (int)mLastMapWidth - 2);
 y2 = QMIN(y2, (int)mLastMapHeight - 2);
 if(x2 < x1 || y2 < y1) {
	return;
 }

 // TODO: don't go over every single cell!
 for (int y = y1; y <= y2; y++) {
	for (int x = x1; x <= x2; x++) {
		unsigned char value = 0;
		if (!localPlayerIO()->isFogged(x, y)) {
			value = 255;
		}

		// 'x + 1' and 'y + 1' because we use 1-texel border
		mFogTextureData[((y + 1) * mFogTextureDataW + (x + 1)) * 4 + 0] = value;
		mFogTextureData[((y + 1) * mFogTextureDataW + (x + 1)) * 4 + 1] = value;
		mFogTextureData[((y + 1) * mFogTextureDataW + (x + 1)) * 4 + 2] = value;
	}
 }

 // Fog texture is now dirty
 mFogTextureDirty = true;

 // Update dirty area
 mFogTextureDirtyAreaX1 = QMIN(mFogTextureDirtyAreaX1, x1);
 mFogTextureDirtyAreaY1 = QMIN(mFogTextureDirtyAreaY1, y1);
 mFogTextureDirtyAreaX2 = QMAX(mFogTextureDirtyAreaX2, x2);
 mFogTextureDirtyAreaY2 = QMAX(mFogTextureDirtyAreaY2, y2);
}



BoGroundRendererBase::BoGroundRendererBase()
{
 mCellListBuilder = 0;
 mCurrentMap = 0;
 mCurrentGroundThemeData = 0;
 mHeightMap2 = 0; // AB: kinda obsolete due to mVertexArray!
 mVertexArray = 0;
 mColorArray = 0;
 mCellListBuilder = 0;
 mFogTexture = 0;
}

BoGroundRendererBase::~BoGroundRendererBase()
{
 boDebug() << k_funcinfo << endl;
 mColorMapRenderers.setAutoDelete(true);
 mColorMapRenderers.clear();
 delete mFogTexture;
 delete mCellListBuilder;
#if FIX_EDGES
 delete[] mHeightMap2;
#endif
 delete[] mVertexArray;
 delete[] mColorArray;
}

bool BoGroundRendererBase::initGroundRenderer()
{
 if (!BoGroundRenderer::initGroundRenderer()) {
	return false;
 }

 mCellListBuilder = new CellListBuilderTree();
 mFogTexture = new FogTexture();

 return true;
}

void BoGroundRendererBase::setLODObject(BoGroundRendererCellListLOD* lod)
{
 mCellListBuilder->setLODObject(lod);
}

void BoGroundRendererBase::updateMapCache(const BosonMap* map)
{
 if (mCurrentMap == map) {
	return;
 }
 mCurrentGroundThemeData = 0;
 mCurrentMap = map;
 BO_CHECK_NULL_RET(mCurrentMap);
 BO_CHECK_NULL_RET(boViewData);
 BO_CHECK_NULL_RET(mCurrentMap->groundTheme());
 mCurrentGroundThemeData = boViewData->groundThemeData(mCurrentMap->groundTheme());
 BO_CHECK_NULL_RET(mCurrentGroundThemeData);

 mColorMapRenderers.setAutoDelete(true);
 mColorMapRenderers.clear();

#if FIX_EDGES
 delete[] mHeightMap2;
 mHeightMap2 = new float[map->cornerArrayPos(map->width(), map->height()) + 1];
#else
 mHeightMap2 = (float*)map->heightMap();
 if (!mHeightMap2) {
	BO_NULL_ERROR(mHeightMap2);
 }
#endif
 int vertexCount = map->cornerArrayPos(map->width(), map->height()) + 1;
 mVertexArray = new float[vertexCount * 3];
 for (unsigned int x = 0; x <= map->width(); x++) {
	for (unsigned int y = 0; y <= map->height(); y++) {
		int pos = map->cornerArrayPos(x, y);
		mVertexArray[pos * 3 + 0] = (float)x;
		mVertexArray[pos * 3 + 1] = -((float)y);
//		z coordinates (heights) are initialized later
	}
 }
 mColorArray = new unsigned char[map->groundTheme()->groundTypeCount() * vertexCount * 4];
 for (unsigned int t = 0; t < map->groundTheme()->groundTypeCount(); t++) {
	for (unsigned int x = 0; x <= map->width(); x++) {
		for (unsigned int y = 0; y <= map->height(); y++) {
			unsigned char* colorArray = mColorArray + vertexCount * 4 * t;
			int pos = map->cornerArrayPos(x, y);
			colorArray[pos * 4 + 0] = 255;
			colorArray[pos * 4 + 1] = 255;
			colorArray[pos * 4 + 2] = 255;
			colorArray[pos * 4 + 3] = 255;
		}
	}
 }
 cellTextureChanged(0, 0, map->width(), map->height());
 boDebug() << k_funcinfo << "created arrays for " << vertexCount << " vertices" << endl;
}

void BoGroundRendererBase::generateCellList(const BosonMap* map)
{
 // we need to regenerate the cell list whenever the modelview or the projection
 // matrix changes. then the displayed cells have most probably changed.

 if (!map) {
	setRenderCells(0, 0);
	setRenderCellsCount(0);
	return;
 }

 if (boGame->gameStatus() == KGame::Init) {
	// we construct the display before the map is received
	return;
 }

 // update mCurrentMap and mHeightMap2
 // derived classes may do their own updates, too
 updateMapCache(map);

 int renderCellsSize = 0;
 unsigned int renderCellsCount = 0;
 int* originalList = renderCells();
 mCellListBuilder->setViewFrustum(viewFrustum());
 mCellListBuilder->setViewport(viewport());
 int* renderCells = mCellListBuilder->generateCellList(map, originalList, &renderCellsSize, &renderCellsCount);
 if (renderCells != originalList) {
	setRenderCells(renderCells, renderCellsSize);
 }
 setRenderCellsCount(renderCellsCount);
#if FIX_EDGES
 if (renderCellsCount > 0) {
	mCellListBuilder->copyHeightMap(mVertexArray, mHeightMap2, map);
 }
#endif
}

void BoGroundRendererBase::cellFogChanged(int x1, int y1, int x2, int y2)
{
 mFogTexture->setLocalPlayerIO(localPlayerIO());
 mFogTexture->cellChanged(x1, y1, x2, y2);
}

QString BoGroundRendererBase::debugStringForPoint(const BoVector3Fixed& pos) const
{
 QString s;
 s += QString("Mouse pos: (%1,%2,%3) ").
	arg(pos[0], 6, 'f', 3).
	arg(pos[1], 6, 'f', 3).
	arg(pos[2], 6, 'f', 3);
 s += QString("Mouse canvas pos: (%1,%2,%3) ").
	arg(pos[0], 6, 'f', 3).
	arg(-pos[1], 6, 'f', 3).
	arg(pos[2], 6, 'f', 3);

 if (!viewFrustum()) {
	s += "NULL viewFrustum() - cannot do anything";
	return s;
 }
// const bottomPlane* bottomPlane = &viewFrustum()->bottom();
 const BoPlane* nearPlane = &viewFrustum()->near();
// const BoPlane* farPlane = &viewFrustum()->far();

#if 0
 s += QString("\nPlane: (%1,%2,%3,%4)").
	arg(plane[0], 6, 'f', 3).
	arg(plane[1], 6, 'f', 3).
	arg(plane[2], 6, 'f', 3).
	arg(plane[3], 6, 'f', 3);
#endif

 s += QString("\n");

// s += QString("distance from BOTTOM plane: %1\n").arg(bottomPlane->distance(pos), 6, 'f', 3);
 s += QString("distance from NEAR plane: %1\n").arg(nearPlane->distance(pos.toFloat()), 6, 'f', 3);
// s += QString("distance from FAR plane: %1\n").arg(farPlane->distance(pos), 6, 'f', 3);

 return s;
}

void BoGroundRendererBase::renderVisibleCellsStart(const BosonMap* map)
{
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "before method" << endl;
 }
 updateMapCache(map);
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "after updating map cache" << endl;
 }

 mFogTexture->setLocalPlayerIO(localPlayerIO());
 mFogTexture->start(map);
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "at end of method" << endl;
 }
}

void BoGroundRendererBase::renderVisibleCellsStop(const BosonMap* map)
{
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "before method" << endl;
 }
 mFogTexture->stop(map);
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "at end of method" << endl;
 }
}

BoColorMapRenderer* BoGroundRendererBase::getUpdatedColorMapRenderer(BoColorMap* map)
{
 BoColorMapRenderer* r = mColorMapRenderers[map];
 if (r) {
	r->update();
	return r;
 }
 boDebug() << k_funcinfo << "creating new colormap renderer" << endl;
 r = new BoColorMapRenderer(map);
 mColorMapRenderers.insert(map, r);
 return r;
}

void BoGroundRendererBase::cellHeightChanged(int, int, int, int)
{
#if FIX_EDGES
 mCellListBuilder->copyHeightMap(mVertexArray, mHeightMap2, mCurrentMap);
#endif
}

void BoGroundRendererBase::cellTextureChanged(int x1, int y1, int x2, int y2)
{
 BO_CHECK_NULL_RET(mCurrentMap);
 BO_CHECK_NULL_RET(mColorArray);
 for (unsigned int t = 0; t < mCurrentMap->groundTheme()->groundTypeCount(); t++) {
	for (int x = x1; x <= x2; x++) {
		for (int y = y1; y <= y2; y++) {
			int pos = mCurrentMap->cornerArrayPos(x, y)
				+ (mCurrentMap->cornerArrayPos(mCurrentMap->width(), mCurrentMap->height()) + 1) * t;
			mColorArray[pos * 4 + 3] = mCurrentMap->texMapAlpha(t, x, y);
		}
	}
 }
}

