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

#include "bogroundrendererbase.h"
#include "bogroundrendererbase.moc"

#include "boquadtreenode.h"
#include "../bosonmap.h"
#include "../bosonprofiling.h"
#include "../defines.h"
#include "../cell.h"
#include "../bo3dtools.h"
#include <bodebug.h>

// not nice in this file. we need it for boGame->status() == KGame::Init
// maybe we should require KGame not to be in init state before constructin the
// class
#include "../boson.h"

#include <GL/gl.h>
#include <math.h>

#define FIX_EDGES 1
#define FIX_EDGES_1 0

static int g_cellsVisibleCalls = 0;

static float distanceFromPlane(const float* plane, const BoQuadTreeNode* node, const BosonMap* map);



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
	}
	virtual ~CellListBuilder() {}

	void copyHeightMap(float* heightMap, const BosonMap* map);

	void setViewport(const int* p) { mViewport = p; }
	const int* viewport() const { return mViewport; }
	void setViewFrustum(const float* f) { mViewFrustum = f; }
	const float* viewFrustum() const { return mViewFrustum; }

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
	virtual void copyCustomHeightMap(float* heightMap, const BosonMap* map) = 0;

protected:
	int mMinX;
	int mMaxX;
	int mMinY;
	int mMaxY;

private:
	const float* mViewFrustum;
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
		delete mRoot;
	}

	virtual int* generateCellList(const BosonMap* map, int* renderCells, int* renderCellsSize, unsigned int* renderCellsCount);


	QMemArray< QPtrList<const BoQuadTreeNode>* > mLeafs;

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

	/**
	 * @return TRUE if the @p node is supposed to be displayed as a single
	 * quad. This is either the case if the node contains exactly one cell
	 * only, or if the distance from the player is high enough for this
	 * level of detail.
	 **/
	bool doLOD(const BoQuadTreeNode* node) const;

	virtual void copyCustomHeightMap(float* heightMap, const BosonMap* map);

private:
	// these variables are valid only while we are in generateCellList() !
	const BosonMap* mMap;
	unsigned int mCount;

	BoQuadTreeNode* mRoot;
};

void CellListBuilder::copyHeightMap(float* heightMap, const BosonMap* map)
{
 static int profiling_id = boProfiling->requestEventId("copyHeightMap");
 BosonProfiler prof(profiling_id);
 if (mMinX < 0 || mMinY < 0) {
	boError() << k_funcinfo << "minx=" << mMinX << " miny=" << mMinY << endl;
	mMinX = mMinY = 0;
	mMaxX = map->width() - 1;
	mMaxY = map->height() - 1;

	for (int x = mMinX; x <= mMaxX + 1; x++) { // +1 because we need _corners_ not cells
		for (int y = mMinY; y <= mMaxY + 1; y++) {
			heightMap[map->cornerArrayPos(x, y)] = map->heightAtCorner(x, y);
		}
	}
 } else {
#if FIX_EDGES_1
	const float* h = map->heightMap();
	for (int x = mMinX; x <= mMaxX; x++) { // <= because we need _corners_ not cells
		for (int y = mMinY; y <= mMaxY; y++) {
			const int index = map->cornerArrayPos(x, y);
			heightMap[index] = h[index];
		}
	}
#endif
 }
 copyCustomHeightMap(heightMap, map);
}

void CellListBuilderTree::copyCustomHeightMap(float* heightMap, const BosonMap* map)
{
 static int profiling_id = boProfiling->requestEventId("copyCustomHeightMap");
 BosonProfiler prof(profiling_id);
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
#if FIX_EDGES_1
		if (node->mNodeSize == 1) {
			continue;
		}
#endif
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
#if FIX_EDGES_1
		for (int x = 1; x < w; x++) {
#else
		for (int x = 0; x <= w; x++) {
#endif
			heightMap[map->cornerArrayPos(l + x, t)] = tl + topStep * x;
			heightMap[map->cornerArrayPos(l + x, b + 1)] = bl + bottomStep * x;
		}

		// just like above, but for left/right border
		// note that y==0 and y==h are not required, as it is covered
		// above anyqay
		for (int y = 1; y < h; y++) {
			heightMap[map->cornerArrayPos(l, t + y)] = tl + leftStep * y;
			heightMap[map->cornerArrayPos(r + 1, t + y)] = tr + rightStep * y;
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
 static int profiling_id = boProfiling->requestEventId("generateCellList");
 BosonProfiler prof(profiling_id);
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
	static int profTreeCreation = boProfiling->requestEventId("mapTreeGeneration");
	BosonProfiler prof(profTreeCreation);
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

bool CellListBuilderTree::doLOD(const BoQuadTreeNode* node) const
{
 if (!node) {
	return false;
 }
 const int count = node->nodeSize();
 if (count == 1) {
	return true;
 }
 const float* plane = &viewFrustum()[5 * 4]; // NEAR plane

 // FIXME: distanceFromPlane() tests the distance of all 4 corners of the rect
 // only. this is perfectly legal if the whole rect is inside the viewfrustum,
 // however if it is partially visible only, this may not be sufficient!
 float d = distanceFromPlane(plane, node, mMap);
 if (d > 240.0f && count <= 64 ||
		d > 120.0f && count <= 16 ||
		d > 40.0f && count <= 8 ||
		d > 20.0f && count <= 2) {
//	boDebug() << d << endl;
	return true;
 }
 return false;
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
 if (doLOD(node)) {
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

 int ret = Bo3dTools::sphereCompleteInFrustum(viewFrustum(), center, radius);
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
	if (!partially || doLOD(node)) {
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

static float distanceFromPlane(const float* plane, const BoQuadTreeNode* node, const BosonMap* map)
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
 const float d1 = Bo3dTools::distanceFromPlane(plane, BoVector3Float(x, y, zTopLeft));
 const float d2 = Bo3dTools::distanceFromPlane(plane, BoVector3Float(x2, y, zTopRight));
 const float d3 = Bo3dTools::distanceFromPlane(plane, BoVector3Float(x, y2, zBottomLeft));
 const float d4 = Bo3dTools::distanceFromPlane(plane, BoVector3Float(x2, y2, zBottomRight));
 float d = QMAX(d1, d2);
 d = QMAX(d, d3);
 d = QMAX(d, d4);
 return d;
}



BoGroundRendererBase::BoGroundRendererBase()
{
 mCellListBuilder = 0;
 mMap = 0;
 mHeightMap2 = 0;
 mCellListBuilder = new CellListBuilderTree();
}

BoGroundRendererBase::~BoGroundRendererBase()
{
 boDebug() << k_funcinfo << endl;
 delete mCellListBuilder;
#if FIX_EDGES
 delete[] mHeightMap2;
#endif
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
 if (mMap != map) {
#if FIX_EDGES
	delete[] mHeightMap2;
	mHeightMap2 = new float[map->cornerArrayPos(map->width(), map->height()) + 1];
#else
	mHeightMap2 = (float*)map->heightMap();
#endif
 }
 mMap = map;
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
	mCellListBuilder->copyHeightMap(mHeightMap2, map);
 }
#endif
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
// const float* bottomPlane = &viewFrustum()[2 * 4];
 const float* nearPlane = &viewFrustum()[5 * 4];
// const float* farPlane = &viewFrustum()[4 * 4];

#if 0
 s += QString("\nPlane: (%1,%2,%3,%4)").
	arg(plane[0], 6, 'f', 3).
	arg(plane[1], 6, 'f', 3).
	arg(plane[2], 6, 'f', 3).
	arg(plane[3], 6, 'f', 3);
#endif

 s += QString("\n");

// s += QString("distance from BOTTOM plane: %1\n").arg(Bo3dTools::distanceFromPlane(bottomPlane, pos), 6, 'f', 3);
 s += QString("distance from NEAR plane: %1\n").arg(Bo3dTools::distanceFromPlane(nearPlane, pos.toFloat()), 6, 'f', 3);
// s += QString("distance from FAR plane: %1\n").arg(Bo3dTools::distanceFromPlane(farPlane, pos), 6, 'f', 3);

 return s;
}

