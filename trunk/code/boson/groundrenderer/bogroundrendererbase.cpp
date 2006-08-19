/*
    This file is part of the Boson game
    Copyright (C) 2003-2006 Andreas Beckermann (b_mann@gmx.de)

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
#include "../gameengine/bosonmap.h"
#include "../bosonprofiling.h"
#include "../defines.h"
#include "../gameengine/cell.h"
#include "../bo3dtools.h"
#include "../bosonconfig.h" // WARNING: groundrenderer needs to be re-installed if bosonconfig.h changes!
#include "../botexture.h"
#include "../gameengine/playerio.h"
#include "../bosonviewdata.h"
#include "../gameengine/bosongroundtheme.h"
#include "bocolormaprenderer.h"
#include <bogl.h>
#include <bodebug.h>

// not nice in this file. we need it for boGame->status() == KGame::Init
// maybe we should require KGame not to be in init state before constructin the
// class
#include "../gameengine/boson.h"

#include <math.h>

#define FIX_EDGES 1

static int g_cellsVisibleCalls = 0;


BoGroundRendererQuadTreeNode* BoGroundRendererQuadTreeNode::createTree(unsigned int w, unsigned int h)
{
 if (w < 1) {
	boError() << k_funcinfo << "invalid width: " << w << endl;
	w = 1;
 }
 if (h < 1) {
	boError() << k_funcinfo << "invalid height: " << h << endl;
	h = 1;
 }
 BoGroundRendererQuadTreeNode* root = new BoGroundRendererQuadTreeNode(0, 0, w - 1, h - 1, 0);
 root->createChilds(w, h);
 return root;
}

BoQuadTreeNode* BoGroundRendererQuadTreeNode::createNode(int l, int t, int r, int b, int depth) const
{
 return new BoGroundRendererQuadTreeNode(l, t, r, b, depth);
}

void BoGroundRendererQuadTreeNode::cellTextureChanged(const BosonMap* map, int x1, int y1, int x2, int y2)
{
 if (!intersects(x1, y1, x2, y2)) {
	return;
 }
 BoGroundQuadTreeNode::cellTextureChanged(map, x1, y1, x2, y2);

 calculateRoughness(map);
}

void BoGroundRendererQuadTreeNode::cellHeightChanged(const BosonMap* map, int x1, int y1, int x2, int y2)
{
 if (!intersects(x1, y1, x2, y2)) {
	return;
 }
 BoGroundQuadTreeNode::cellHeightChanged(map, x1, y1, x2, y2);

 calculateRoughness(map);
}

// TODO: also take neighboring cells into account!
void BoGroundRendererQuadTreeNode::calculateRoughness(const BosonMap* map)
{
 // TODO: we should calculate this for leafs only and use the value of the
 // children for the parents.
 // however this requires the getRoughnessInRect() method to be
 // a) bugfree
 // b) precise (e.g. no or only little rounding errors)
 // therefore for now we go the slow way and calculate this for every node
#if 0
 if (childCount > 0) {
	// ...
	return;
 }
#else
 float r;
 float t;
 BoGroundRendererBase::getRoughnessInRect(map, &r, &t, left(), top(), right(), bottom());
 // AB: do NOT set directly! always use setRoughness()
 setRoughness(r, t);
#endif
}

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
	 * @return TRUE for @ref CellListBuilderTree, otherwise FALSE (this is
	 * the RTTI of this class)
	 **/
	virtual bool isTreeBuilder() const { return false; }

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
	 * @param mindist If non-null, distance between near plane and closest
	 * visible cell.
	 * @param maxdist If non-null, distance between near plane and farthest
	 * visible cell.
	 **/
	virtual int* generateCellList(const BosonMap* map, int* renderCells, int* renderCellsSize, unsigned int* renderCellsCount, float* minDist = 0, float* maxDist = 0) = 0;

	virtual void updateMapCache(const BosonMap* map)
	{
		Q_UNUSED(map);
	}

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
 * addVisibleNodes is called on a valid rect, it calls itself again 4 times. Once for
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

	virtual bool isTreeBuilder() const { return true; }

	virtual int* generateCellList(const BosonMap* map, int* renderCells, int* renderCellsSize, unsigned int* renderCellsCount, float* minDist = 0, float* maxDist = 0);

	const BoGroundRendererQuadTreeNode* findVisibleNodeAt(int x, int y);

	virtual void updateMapCache(const BosonMap* map);

protected:
	/**
	 * Uses @ref addVisibleNodes to add all visible cells to @p cells.
	 **/
	void addVisibleCells(int* cells, const BoGroundRendererQuadTreeNode* node);

	/**
	 * This is the main method of this class. It checks (recursively)
	 * whether the cells in the node are visible and adds all visible
	 * nodes to @p nodes.
	 *
	 * @param allVisible If FALSE (the default), the node is checked for
	 * visibility before being added. If set to TRUE, the method assumes
	 * that all cells in @p node are visible and adds the nodes according to
	 * the LOD settings only.
	 **/
	void addVisibleNodes(QPtrList<const BoGroundRendererQuadTreeNode>* nodes, const BoGroundRendererQuadTreeNode* node, bool allVisible = false);

	/**
	 * @return Whether the cells in the rect of the node are visible.
	 * If they are visible, @p partially is set to FALSE, when all cells are
	 * visible, otherwise to TRUE (rect is partially visible only). If no
	 * cell is visible (i.e. this returns FALSE) @p partially is set to
	 * FALSE.
	 **/
	bool cellsVisible(const BoGroundRendererQuadTreeNode* node, bool* partially);

	/**
	 * Add all cells in the rect of the node to @p cells
	 **/
	void addCells(int* cells, const BoGroundRendererQuadTreeNode* node);

	void recreateTree(const BosonMap* map);

	virtual void copyCustomHeightMap(float* vertexArray, float* heightMap, const BosonMap* map);

private:
	// these variables are valid only while we are in generateCellList() !
	const BosonMap* mMap;
	unsigned int mCount;

	BoGroundRendererQuadTreeNode* mRoot;

	float mMinDistance;
	float mMaxDistance;

	QMemArray< QPtrList<const BoGroundRendererQuadTreeNode>* > mLeafs;
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
	QPtrList<const BoGroundRendererQuadTreeNode>* list = mLeafs[i];
	if (!list || list->isEmpty()) {
		continue;
	}
	QPtrListIterator<const BoGroundRendererQuadTreeNode> it(*list);
	while (it.current()) {
		const BoGroundRendererQuadTreeNode* node = it.current();
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

void CellListBuilderTree::updateMapCache(const BosonMap* map)
{
 if (mMap != map) {
	mMap = 0;
	boDebug() << k_funcinfo << "recreating map tree" << endl;
	BosonProfiler prof("mapTreeGeneration");
	recreateTree(map);
	boDebug() << k_funcinfo << "recreating map tree done" << endl;
 }
 mMap = map;
}

int* CellListBuilderTree::generateCellList(const BosonMap* map, int* origRenderCells, int* renderCellsSize, unsigned int* renderCellsCount, float* minDist, float* maxDist)
{
 mMinX = mMinY = -1;
 mMaxX = mMaxY = 0;
 mMinDistance = 1000000.0f;
 mMaxDistance = -1000000.0f;
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
 updateMapCache(map);
 mCount = 0;

 for (int i = 0; i < (int)mLeafs.size(); i++) {
	QPtrList<const BoGroundRendererQuadTreeNode>* list = mLeafs[i];
if (list) {
		list->clear();
	}
 }

 g_cellsVisibleCalls = 0;
 addVisibleCells(renderCells, mRoot);
// boDebug() << k_funcinfo << g_cellsVisibleCalls << " calls - will render cells: " << mCount << endl;

 *renderCellsCount = mCount;
// mMap = 0;
 mCount = 0;
 if (minDist) {
	*minDist = QMAX(0, mMinDistance);
 }
 if (maxDist) {
	*maxDist = QMAX(0, mMaxDistance);
 }
 return renderCells;
}

const BoGroundRendererQuadTreeNode* CellListBuilderTree::findVisibleNodeAt(int x, int y)
{
 if (!mMap) {
	return 0;
 }
 if (!mRoot) {
	return 0;
 }

 int x1 = x;
 int x2 = x;
 int y1 = y;
 int y2 = y;

 QPtrList<const BoGroundRendererQuadTreeNode> nodes;
 addVisibleNodes(&nodes, mRoot);
 for (QPtrListIterator<const BoGroundRendererQuadTreeNode> it(nodes); it.current(); ++it) {
	if (it.current()->intersects(x1, y1, x2, y2)) {
		return it.current();
	}
 }
 return 0;
}

void CellListBuilderTree::addCells(int* cells, const BoGroundRendererQuadTreeNode* node)
{
 if (!node) {
	return;
 }
 const int l = node->left();
 const int t = node->top();
 const int r = node->right();
 const int b = node->bottom();
 BoGroundRenderer::setCell(cells, mCount, l, t, r - l + 1, b - t + 1);
 mCount++;
 if ((int)mLeafs.size() < node->depth() + 1) {
	int s = mLeafs.size();
	mLeafs.resize(node->depth() + 1);
	for (int i = s; i < (int)mLeafs.size(); i++) {
		mLeafs[i] = new QPtrList<const BoGroundRendererQuadTreeNode>();
	}
 }
 mLeafs[node->depth()]->append(node);
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
}

bool CellListBuilderTree::cellsVisible(const BoGroundRendererQuadTreeNode* node, bool* partially)
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

 float dist;
 int ret = viewFrustum()->sphereCompleteInFrustum(center, radius, &dist);
 if (ret == 0) {
	*partially = false;
	return false;
 }
 if (ret == 2 || (w == 1 && h == 1) || w * h <= 4) {
	*partially = false;
	mMinDistance = QMIN(mMinDistance, dist - 2*radius);
	mMaxDistance = QMAX(mMaxDistance, dist);
 } else {
	*partially = true;
 }
 return true;
}

void CellListBuilderTree::addVisibleCells(int* cells, const BoGroundRendererQuadTreeNode* root)
{
 BO_CHECK_NULL_RET(cells);
 BO_CHECK_NULL_RET(root);
 QPtrList<const BoGroundRendererQuadTreeNode> nodes;
 addVisibleNodes(&nodes, root);
 for (QPtrListIterator<const BoGroundRendererQuadTreeNode> it(nodes); it.current(); ++it) {
	addCells(cells, it.current());
 }
}

void CellListBuilderTree::addVisibleNodes(QPtrList<const BoGroundRendererQuadTreeNode>* ret, const BoGroundRendererQuadTreeNode* node, bool allVisible)
{
 BO_CHECK_NULL_RET(ret);
 if (!node) {
	// AB: totally valid! supposed to be a noop
	return;
 }
 bool partially = false;
 bool visible = false;
 if (allVisible) {
	partially = false;
	visible = true;
 } else {
	visible = cellsVisible(node, &partially);
	if (!partially) {
		allVisible = true;
	}
 }
 if (visible) {
	if ((mLODObject && mLODObject->doLOD(mMap, node))) {
		ret->append(node);
	} else {
		BoQuadTreeNode* children[4];
		node->getChildren(children);
		for (int i = 0; i < 4; i ++) {
			addVisibleNodes(ret, (BoGroundRendererQuadTreeNode*)children[i], allVisible);
		}
	}
 }
}

void CellListBuilderTree::recreateTree(const BosonMap* map)
{
 BO_CHECK_NULL_RET(map);

 BosonProfiler prof("recreateTree");

 delete mRoot;
 mRoot = BoGroundRendererQuadTreeNode::createTree(map->width(), map->height());

 // AB: we can safely do that, as registerQuadTree() does not modify the map, but
 // stores a pointer to the tree only.
 BosonMap* nonConstMap = (BosonMap*)map;
 nonConstMap->registerQuadTree(mRoot);

 BosonProfiler prof2("initialize tree");
 mRoot->cellHeightChanged(map, 0, 0, map->width() - 1, map->height() - 1);
 mRoot->cellTextureChanged(map, 0, 0, map->width() - 1, map->height() - 1);
}

// TODO: atm these settings are too aggressive for editor!
bool BoGroundRendererCellListLOD::doLOD(const BosonMap* map, const BoGroundRendererQuadTreeNode* node) const
{
 if (!node) {
	return false;
 }

 // number of cells in this node.
 const int count = node->nodeSize();
 if (count == 1) {
	return true;
 }
 const BoPlane& plane = viewFrustum()->near();

 // FIXME: distanceFromPlane() tests the distance of all 4 corners of the rect
 // only. this is perfectly legal if the whole rect is inside the viewfrustum,
 // however if it is partially visible only, this may not be sufficient!
 float d = distanceFromPlane(plane, node, map);

 if (d < 8.0f) {
	return false;
 }

 // roughness indicates how many height/texture differences are in a node.
 // this should be the primary indicator for LOD.
 //
 // plain roughness, independent of camera.
 // high values mean more differences (show high detailed version ; don't do
 // LOD)
 float r = node->roughnessValue(1.0f);

 // here we always do LOD, no matter how far from away from the camera we are
 if (r < 2.0f && count < 50) {
	return true;
 }

 // roughness divided by distance to camera.
 // the lower the value, the less details should be shown.
 // note that this is the opposite behaviour of plain roughness!
 float e = r / d;

 // this is like the previous one, but also takes the size of the node into
 // account.
 // in addition the quadratic roughness is used, making it more important
 float e2 = (r * r) / (d / count);

 if (e2 < 0.5f) {
	return true;
 }
 if (e2 < 1.25f) {
	return true;
 }
 if (e2 < 3.0f) {
	return true;
 }
 if (e2 < 6.0f) {
	return true;
 }
 if (e2 < 10.0f) {
	return true;
 }


 // do LOD independent of roughness, only dependent on distance and size of node
 if (d > 240.0f && count <= 64 ||
		d > 150.0f && count <= 16 ||
		d > 60.0f && count <= 8 ||
		d > 20.0f && count <= 2) {
	return true;
 }
 return false;
}

float BoGroundRendererCellListLOD::distanceFromPlane(const BoPlane& plane, const BoGroundRendererQuadTreeNode* node, const BosonMap* map) const
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
 if (mLastMap != map) {
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
	mLastMap = map;
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
			if (!mSmoothEdges || (x > 1 && y > 1 && x < mLastMapWidth && y < mLastMapHeight)) {
				if (localPlayerIO()->isExplored(x - 1, y - 1)) {
					if (localPlayerIO()->isFogged(x - 1, y - 1)) {
						value = 205;
					} else {
						value = 255;
					}
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
		if (!localPlayerIO()->isExplored(x, y)) {
			value = 0;
		} else if(localPlayerIO()->isFogged(x, y)) {
			value = 205;
		} else {
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
 mUsedTextures = 0;
 mUsedTexturesDirty = true;
}

BoGroundRendererBase::~BoGroundRendererBase()
{
 mColorMapRenderers.setAutoDelete(true);
 mColorMapRenderers.clear();
 delete mFogTexture;
 delete mCellListBuilder;
#if FIX_EDGES
 delete[] mHeightMap2;
#endif
 delete[] mVertexArray;
 delete[] mColorArray;
 delete[] mUsedTextures;
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
 boDebug() << k_funcinfo << endl;
 mCurrentGroundThemeData = 0;
 mCurrentMap = map;
 BO_CHECK_NULL_RET(mCurrentMap);
 BO_CHECK_NULL_RET(boViewData);
 BO_CHECK_NULL_RET(mCurrentMap->groundTheme());
 mCurrentGroundThemeData = boViewData->groundThemeData(mCurrentMap->groundTheme());
 BO_CHECK_NULL_RET(mCurrentGroundThemeData);

 mColorMapRenderers.setAutoDelete(true);
 mColorMapRenderers.clear();

 delete[] mUsedTextures;
 mUsedTextures = new bool[map->groundTheme()->groundTypeCount()];
 for (unsigned int i = 0; i < map->groundTheme()->groundTypeCount(); i++) {
	mUsedTextures[i] = true;
 }

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
 mCellListBuilder->updateMapCache(mCurrentMap);
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
 float mindist, maxdist;
 int* renderCells = mCellListBuilder->generateCellList(map, originalList, &renderCellsSize, &renderCellsCount, &mindist, &maxdist);
 statistics()->setMinDistance(mindist);
 statistics()->setMaxDistance(maxdist);
 if (renderCells != originalList) {
	setRenderCells(renderCells, renderCellsSize);
 }
 setRenderCellsCount(renderCellsCount);
 for (unsigned int i = 0; i < map->groundTheme()->groundTypeCount(); i++) {
	mUsedTextures[i] = true;
 }
 mUsedTexturesDirty = true;
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

void BoGroundRendererBase::cellExploredChanged(int x1, int y1, int x2, int y2)
{
 mFogTexture->setLocalPlayerIO(localPlayerIO());
 mFogTexture->cellChanged(x1, y1, x2, y2);
}

QString BoGroundRendererBase::debugStringForPoint(const BoVector3Fixed& pos) const
{
 QString s;
 s += QString("Mouse pos: (%1,%2,%3) \n").
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

 if (!mCellListBuilder->isTreeBuilder()) {
	return s;
 }

 const BoGroundRendererQuadTreeNode* node = ((CellListBuilderTree*)mCellListBuilder)->findVisibleNodeAt((int)pos.x(), (int)-pos.y());

 if (!node) {
	s += QString("no node in tree for this position\n");
 } else {
	float r_at_1 = node->roughnessValue(1.0f);
	s += QString("node dimensions: l=%1 r=%2 t=%3 b=%4\n")
			.arg(node->left())
			.arg(node->right())
			.arg(node->top())
			.arg(node->bottom());
	s += QString("node size: %1\n").arg(node->nodeSize());
	s += QString("depth: %1\n").arg(node->depth());
	s += QString("roughness of node at distance=1: %1\n").arg(r_at_1);
	s += QString("roughness / distance: %1\n").arg(r_at_1 / nearPlane->distance(pos.toFloat()));
	s += QString("roughness^2 / (distance / nodesize): %1\n").arg((r_at_1 * r_at_1) / (nearPlane->distance(pos.toFloat()) / node->nodeSize()));
 }

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

void BoGroundRendererBase::cellHeightChanged(int x1, int y1, int x2, int y2)
{
 BO_CHECK_NULL_RET(mCellListBuilder);

#if FIX_EDGES
 mCellListBuilder->copyHeightMap(mVertexArray, mHeightMap2, mCurrentMap);
#endif

 // re-generate the visible cells list (with new LOD settings)
 setRenderCellsCount(0);
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

 mUsedTexturesDirty = true;

 // re-generate the visible cells list (with new LOD settings)
 setRenderCellsCount(0);
}

#define TEXROUGHNESS_MULTIPLIER 0.125f
void BoGroundRendererBase::getRoughnessInRect(const BosonMap* map, float* _roughness, float* _textureRoughnessTotal, int x1, int y1, int x2, int y2)
{
 BO_CHECK_NULL_RET(map);
 BO_CHECK_NULL_RET(map->groundTheme());
 BO_CHECK_NULL_RET(_roughness);
 BO_CHECK_NULL_RET(_textureRoughnessTotal);

 if (x2 < x1 || y2 < y1) {
	boError() << k_funcinfo << "invalid parameters " << x1 << " " << x2 << " " << y1 << " " << y2 << endl;
	return;
 }

 unsigned int textureCount = map->groundTheme()->groundTypeCount();

 // AB: the "weight" of a texture is it's alpha value, i.e. the amount of the
 // texture that is to be rendered for a cell.

 // Finds average normal and texture weight of the chunk
 BoVector3Float avgNormal;
 float* avgTexWeight = new float[textureCount];
 for(unsigned int t = 0; t < textureCount; t++) {
	avgTexWeight[t] = 0.0f;
 }
 for (int y = y1; y <= y2; y++) {
	for (int x = x1; x <= x2; x++) {
		for (unsigned int t = 0; t < textureCount; t++) {
			avgTexWeight[t] += map->texMapAlpha(t, x, y);
		}
		avgNormal += BoVector3Float(map->normalMap() + 3 * map->cornerArrayPos(x, y));
	}
 }
 avgNormal.normalize();
 for (unsigned int t = 0; t < textureCount; t++) {
	avgTexWeight[t] /= (x2 - x1 + 1) * (y2 - y1 + 1);
	avgTexWeight[t] /= 255.0f;
 }


 // Finds roughness of the rect by summing dot products of every normal
 //  and average normal
 float roughness = 0.0f;
 float textureRoughnessTotal = 0.0f;
 float* textureRoughness = new float[textureCount];
 for (unsigned int t = 0; t < textureCount; t++) {
	textureRoughness[t] = 0.0f;
 }

 for (int y = y1; y <= y2; y++) {
	for (int x = x1; x <= x2; x++) {
		int pos = map->cornerArrayPos(x, y);
		roughness += (1.0f - BoVector3Float::dotProduct(avgNormal, BoVector3Float(map->normalMap() + 3 * pos)));
		for (unsigned int t = 0; t < textureCount; t++) {
			float texvalue = map->texMapAlpha(t, x, y) / 255.0f;
			textureRoughness[t] += QABS(texvalue - avgTexWeight[t]);
		}
	}
 }
 roughness = sqrt(1.0f + roughness) - 1.05f;
 //boDebug() << k_funcinfo << "Roughness of cell at (" << chunk->minX << "; " << chunk->minY <<
 //    ") is " << chunk->roughness << endl;
 textureRoughnessTotal = 0.0f;
 for (unsigned int t = 0; t < textureCount; t++) {
	textureRoughnessTotal += textureRoughness[t];
	/*textureRoughness[t] = (sqrt(1.0f + textureRoughness[t]) - 1.05f) / TEXROUGHNESS_MULTIPLIER;
	boDebug() << k_funcinfo << "Tex roughness for tex " << t << " is " <<
		textureRoughness[t] << endl;*/
 }
 textureRoughnessTotal = (sqrt(1.0f + textureRoughnessTotal) - 1.05f) * TEXROUGHNESS_MULTIPLIER;
 //boDebug() << k_funcinfo << "Total tex roughness is " << textureRoughnessTotal << endl;
 delete[] avgTexWeight;
 delete[] textureRoughness;

 *_roughness = roughness;
 *_textureRoughnessTotal = textureRoughnessTotal;
}

bool BoGroundRendererBase::isCellInRectVisible(int x1, int y1, int x2, int y2) const
{
 if (!mCellListBuilder->isTreeBuilder()) {
	return true;
 }
 CellListBuilderTree* tree = (CellListBuilderTree*)mCellListBuilder;
 for (int x = x1; x <= x2; x++) {
	for (int y = y1; y <= y2; y++) {
		const BoGroundRendererQuadTreeNode* node = tree->findVisibleNodeAt(x, y);
		if (node) {
			return true;
		}
	}
 }
 return false;
}

