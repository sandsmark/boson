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

// a couple of helper functions. these should be in Bo3dTools.
static bool isInFrontOfPlane(const float* plane, const BoVector3Float& vector);
static bool lineIntersects(const float* plane, const BoVector3Float& pos, const BoVector3Float& direction, BoVector3Float* intersection);
static bool lineIntersects_points(const float* plane, const BoVector3Float& start, const BoVector3Float& end, BoVector3Float* intersection);
static bool lineSegmentIntersects(const float* plane, const BoVector3Float& start, const BoVector3Float& end, BoVector3Float* intersection);
#define EPSILON 0.0001f // zero for floating point numbers

static int g_cellsVisibleCalls = 0;

class CellTreeNode;
static float distanceFromPlane(const float* plane, const BoVector3Float& pos);
static float distanceFromPlane(const float* plane, const CellTreeNode* node, const BosonMap* map);


// a quadtree of the cells on the map
class CellTreeNode
{
public:
	/**
	 * @param l The left side of the rect (i.e. the x coordinate of the
	 * most-left cell).
	 * @param t The top side of the rect (i.e. the y coordinate of the
	 * most-top cell).
	 * @param r The right side of the rect (i.e. the x coordinate of the
	 * most-right cell). This eqals @p l, if width of the rect is 1.
	 * @param b The bottom side of the rect (i.e. the y coordinate of the
	 * most-bottom cell). This equals @p t if the height of the rect is 1.
	 **/
	CellTreeNode(int l, int t, int r, int b)
	{
		mTopLeft = 0;
		mTopRight = 0;
		mBottomLeft = 0;
		mBottomRight = 0;
		mLeft = l;
		mTop = t;
		mRight = r;
		mBottom = b;
		mCellsCount = (r - l + 1) * (b - t + 1);
		if (mRight < mLeft || mBottom < mTop) {
			boError() << k_funcinfo << "invalid coordinates" << endl;
			mCellsCount = 1;
		}
	}
	~CellTreeNode()
	{
		delete mTopLeft;
		delete mTopRight;
		delete mBottomLeft;
		delete mBottomRight;
	}
	void createChilds(const BosonMap* map);


	// AB: note that we cannot easily save additional information such as
	// maxheight or texmap information here, because we use the tree in
	// editor mode as well, and there these information may change at any
	// time without letting the tree know.
//	bool mIsLeaf;
	int mLeft;
	int mTop;
	int mRight;
	int mBottom;
	int mCellsCount;
	CellTreeNode* mTopLeft;
	CellTreeNode* mTopRight;
	CellTreeNode* mBottomLeft;
	CellTreeNode* mBottomRight;
};

void CellTreeNode::createChilds(const BosonMap* map)
{
 const int l = mLeft;
 const int t = mTop;
 const int r = mRight;
 const int b = mBottom;

 if (l == r && b == t) {
	// we are a leaf
	return;
 }
 if (l > r || t > b || l < 0 || t < 0) {
	boError() << k_funcinfo << "invalid values: left="
			<< l << ", top=" << t
			<< ", right=" << r
			<< ", bottom=" << b << endl;
	return;
 }
 if (r >= (int)map->width()) {
	boError() << k_funcinfo << "right side is too high: " << r << ", map width: " << map->width() << endl;
	return;
 }
 if (b >= (int)map->height()) {
	boError() << k_funcinfo << "bottom side is too high: " << b << ", map height: " << map->height() << endl;
	return;
 }
 int hmid = l + (r - l) / 2;
 int vmid = t + (b - t) / 2;

 mTopLeft = new CellTreeNode(l, t, hmid, vmid);
 mTopLeft->createChilds(map);

 if (vmid + 1 <= b) {
	// (t != b) => we have a bottom rect.
	mBottomLeft = new CellTreeNode(l, vmid + 1, hmid, b);
	mBottomLeft->createChilds(map);
 }
 if (hmid + 1 <= r) {
	// (l != r) => we have a right rect
	mTopRight = new CellTreeNode(hmid + 1, t, r, vmid);
	mTopRight->createChilds(map);
 }
 if (vmid + 1 <= b && hmid + 1 <= r) {
	// ((l != r) && (t != b)) => we have a bottom-right rect
	mBottomRight = new CellTreeNode(hmid + 1, vmid + 1, r, b);
	mBottomRight->createChilds(map);
 }
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
	virtual void copyCustomHeightMap(float* heightMap, const BosonMap* map)
	{
		Q_UNUSED(heightMap);
		Q_UNUSED(map);
	}

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


	QMemArray< QPtrList<const CellTreeNode>* > mLeafs;

protected:
	/**
	 * This is the main method of this class. It checks (recursively)
	 * whether the cells in ((x,y),(x2,y2)) are visible and adds all visible
	 * cells to @p cells.
	 **/
	void addVisibleCells(int* cells, const CellTreeNode* node, int depth = 0);

	/**
	 * @return Whether the cells in the rect of the node are visible.
	 * If they are visible, @p partially is set to FALSE, when all cells are
	 * visible, otherwise to TRUE (rect is partially visible only). If no
	 * cell is visible (i.e. this returns FALSE) @p partially is set to
	 * FALSE.
	 **/
	bool cellsVisible(const CellTreeNode* node, bool* partially) const;

	/**
	 * Add all cells in the rect of the node to @p cells
	 **/
	void addCells(int* cells, const CellTreeNode* node, int depth);

	void recreateTree(const BosonMap* map);

	/**
	 * @return TRUE if the @p node is supposed to be displayed as a single
	 * quad. This is either the case if the node contains exactly one cell
	 * only, or if the distance from the player is high enough for this
	 * level of detail.
	 **/
	bool doLOD(const CellTreeNode* node) const;

	virtual void copyCustomHeightMap(float* heightMap, const BosonMap* map);

private:
	// these variables are valid only while we are in generateCellList() !
	const BosonMap* mMap;
	unsigned int mCount;

	CellTreeNode* mRoot;
};

/**
 * This class is the "mathematical" way of doing this task. In a perfect world
 * this should be the most efficient way (since we need only a formula, not a
 * loop) and the best way (no false positives) to do this.
 *
 * But unfortunately, it isn't this easy. The mathematical formulas for this
 * aren't really easy and retrieving the correct z values for the correct cells
 * isn't so easy either.
 *
 * Therefore this class contains more code than the tree version, is slower and
 * is probably less dependable. It is not recommended and provided only for
 * experiments. Feel free to make it work correctly if you can, but I believe it
 * is removed sooner or later.
 *
 * On performance: on the day of adding the tree version, the tree version took
 * about 20% of the time of generating a "realistic" cell list (zoomed out a bit
 * for getting overview, but not far). For "unrealistic" maps (zoomed out to the
 * maximum, even beyond the FAR plane) the difference is even greater (iirc the
 * tree took about 10-15% of the non-tree version only!)
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class CellListBuilderNoTree : public CellListBuilder
{
public:
	CellListBuilderNoTree() : CellListBuilder()
	{
	}
	~CellListBuilderNoTree() {}

	virtual int* generateCellList(const BosonMap* map, int* renderCells, int* renderCellsSize, unsigned int* renderCellsCount);

protected:
	void calculateWorldRect(const QRect& rect,
			int mapWidth, int mapHeight,
			float* minX, float* minY,
			float* maxX, float* maxY);
};

BoGroundRendererBase::BoGroundRendererBase(bool useCellTree)
{
 mCellListBuilder = 0;
 mMap = 0;
 mHeightMap2 = 0;
 if (useCellTree) {
	mCellListBuilder = new CellListBuilderTree();
 } else {
	mCellListBuilder = new CellListBuilderNoTree();
 }
}

BoGroundRendererBase::~BoGroundRendererBase()
{
 boDebug() << k_funcinfo << endl;
 delete mCellListBuilder;
#if FIX_EDGES
 delete[] mHeightMap2;
#endif
}

static bool isInFrontOfPlane(const float* plane, const BoVector3Float& vector)
{
 float d = vector[0] * plane[0] + vector[1] * plane[1] + vector[2] * plane[2] + plane[3];
 if (d >= 0.0f) {
	return true;
 }
 return false;
}

/**
 * @return FALSE if the line segment does not intersect with the plane,
 * otherwise TRUE. If the line segment is parallel to the @p plane, @p
 * intersection is kept untouched, otherwise it will be the intersection point
 * of the @p plane and the line (a line is infinite long and goes through @p
 * start and @p end). If this function returns TRUE, that point will be on the
 * line segment.
 * @param t internal
 **/
static bool lineIntersects(const float* plane, const BoVector3Float& pos, const BoVector3Float& direction, BoVector3Float* intersection)
{
 const float planeDistance = plane[3];
 const BoVector3Float planeNormal = BoVector3Float(plane[0], plane[1], plane[2]);

 // see count 2.1.0, base/SbPlane.cpp
 if (fabsf(planeDistance) <= EPSILON) {
	boError() << k_funcinfo << "plane distance is 0" << endl;
	return false;
 }

 // we will _always_ have an intersection of line and plane if the line is not
 // parallel to the plane.
 // (AB: note that I said _line_ not line segment)
 if (fabsf(BoVector3Float::dotProduct(direction, planeNormal)) <= EPSILON) {
	return false;
 }

 // this formula is from coin 2.1.0, base/SbPlane.cpp, SbPlane::intersect(). it
 // is documented there very well.
 // AB: to me this looks like an error in coin. plib1.5 uses a slightly
 // different version and this seems to work better
 float t = -(planeDistance + BoVector3Float::dotProduct(planeNormal, pos)) / BoVector3Float::dotProduct(planeNormal, direction);

 *intersection = pos + direction * t;

 // AB: t is between 0 and 1 if the intersection is on the segment (at least it
 // should be)

 return true;
}

// just like above, but takes 2 points on the line, not 1 point and a direction
static bool lineIntersects_points(const float* plane, const BoVector3Float& start, const BoVector3Float& end, BoVector3Float* intersection)
{
 const BoVector3Float pos = start;
 BoVector3Float direction = end - start;
 return lineIntersects(plane, pos, direction, intersection);
}

static bool lineSegmentIntersects(const float* plane, const BoVector3Float& start, const BoVector3Float& end, BoVector3Float* intersection)
{
 bool ret = lineIntersects_points(plane, start, end, intersection);

 if (!ret) {
	return false;
 }

 // we have the intersection of the plane with the line now. we still have to
 // find out whether it is on the line segment.

 bool f1 = isInFrontOfPlane(plane, start);
 bool f2 = isInFrontOfPlane(plane, end);
 if (f1 == f2) {
	return false;
 }
 // one point is in front of and one behind the plane. so there must be an
 // intersection between them. there can be at most one intersection with a
 // plane, as the line (segment) is not parallel to the plane.
 return true;
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
 mCellListBuilder->copyHeightMap(mHeightMap2, map);
#endif
}

void CellListBuilder::copyHeightMap(float* heightMap, const BosonMap* map)
{
 static int profiling_id = boProfiling->requestEventId("copyHeightMap");
 BosonProfiler prof(profiling_id);
 if (mMinX < 0 || mMinY < 0) {
	// AB: in the NoTree version this is always the case.
	// (note the we have basically dropped NoTree already)
	boError() << k_funcinfo << "minx=" << mMinX << " miny=" << mMinY << endl;
	mMinX = mMinY = 0;
	mMaxX = map->width();
	mMaxY = map->height();

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
	QPtrList<const CellTreeNode>* list = mLeafs[i];
	if (!list || list->isEmpty()) {
		continue;
	}
	QPtrListIterator<const CellTreeNode> it(*list);
	while (it.current()) {
		const CellTreeNode* node = it.current();
		++it;
#if FIX_EDGES_1
		if (node->mCellsCount == 1) {
			continue;
		}
#endif
		const int l = node->mLeft;
		const int r = node->mRight;
		const int t = node->mTop;
		const int b = node->mBottom;
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

int* CellListBuilderNoTree::generateCellList(const BosonMap* map, int* origRenderCells, int* renderCellsSize, unsigned int* renderCellsCount)
{
 mMinX = mMinY = -1;
 mMaxX = mMaxY = 0;
 if (!viewport()) {
	BO_NULL_ERROR(viewport());
	return origRenderCells;
 }
 if (!viewFrustum()) {
	BO_NULL_ERROR(viewFrusutm());
	return origRenderCells;
 }
 // re-generate the list of to-be-rendered cells:
 Cell* allCells = map->cells();
 if (!allCells) {
	boError() << k_funcinfo << "NULL cells!" << endl;
	return origRenderCells;
 }
 float maxX = 0.0f, maxY = 0.0f;
 float minX = 0.0f, minY = 0.0f;
 calculateWorldRect(
		QRect(QPoint(0, 0), QPoint(viewport()[2], viewport()[3])),
		map->width(), map->height(),
		&minX, &minY, &maxX, &maxY);
// boDebug() << "world rect: minX=" << minX << " maxX=" << maxX << " minY=" << minY << " maxY=" << maxY << endl;
 minY *= -1;
 maxY *= -1;

 // if everything went fine we need to add those cells that are in the
 // ((minX,minY),(maxX,maxY)) rectangle only.

 int cellMinX = (int)(minX); // AB: *no* +1 for min values!
 int cellMaxX = (int)(maxX) + 1; // +1 because of a modulo (very probably at this point)
 int cellMinY = (int)(minY);
 int cellMaxY = (int)(maxY) + 1;

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

 int* renderCells = origRenderCells;
 int size = (cellMaxX - cellMinX + 1) * (cellMaxY - cellMinY + 1);
 size = QMIN((int)(map->width() * map->height()), size);
 if (size > *renderCellsSize) {
	renderCells = BoGroundRenderer::makeCellArray(size);
	*renderCellsSize = size;
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

		glX = (float)c->x() + 1.0f / 2;
		glY = -((float)c->y() + 1.0f / 2);

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

		if (Bo3dTools::sphereInFrustum(viewFrustum(), BoVector3Float(glX, glY, (minz + maxz) / 2), sqrt(2 * (1.0f / 2) * (1.0f / 2) + z * z))) {
			// AB: instead of storing the cell here we should store
			// cell coordinates and create a vertex array with that
			BoGroundRenderer::setCell(renderCells, count, c->x(), c->y(), 1, 1);
			count++;
		}
	}
 }
 *renderCellsCount = count;
 return renderCells;
}

void CellListBuilderNoTree::calculateWorldRect(const QRect& rect, int mapWidth, int mapHeight, float* minX, float* minY, float* maxX, float* maxY)
{
 BO_CHECK_NULL_RET(viewFrustum());

 // AB: fallback. if everything fails, _nothing_ is visible at all.
 *minX = *minY = *maxX = *maxY = 0.0f;

 for (int i = 0; i < 6; i++) {
	for (int j = 0; j < 4; j++) {
		if (isnan(viewFrustum()[i * 4 + j])) {
			boError() << k_funcinfo << "plane " << i << " is invalid" << endl;
			return;
		}
	}
	if (fabsf(viewFrustum()[i * 4 + 3]) <= EPSILON) {
		boError() << k_funcinfo << "plane distance is 0" << endl;
		return;
	}
 }


 const float mapRight = (mapWidth - 1);
 const float mapBottom = -(mapHeight - 1);

 class LineSegment {
 public:
	// AB: a line has infinte length, a line segment has two end points.
	LineSegment()
	{
		mNext = 0;
		mPrevious = 0;
		id = 0;
	}
	~LineSegment()
	{
		if (mNext) {
			mNext->mPrevious = 0;
		}
		if (mPrevious) {
			mPrevious->mNext = 0;
		}
	}

#if 0
	bool checkSegment(int maxLines) const
	{
		const LineSegment* start = this;
		if (!start->mNext || !start->mPrevious) {
			boError() << k_funcinfo << "oops - start has invalid next/prev pointer" << endl;
			return false;
		}
		const LineSegment* prev = start;
		const LineSegment* l = prev->mNext;
		int lines = 0;
		while (l && l != start) {
			if (!checkPointers(l)) {
				return false;
			}
			l = l->mNext;
			if (lines > maxLines) {
				boError() << k_funcinfo << "too many lines: " << lines << endl;
				dumpLinkedList(maxLines);
				return false;
			}
		}
		return true;
	}
	static bool checkPointers(const LineSegment* l)
	{
		if (!l) {
			BO_NULL_ERROR(l);
			if (!l->mNext) {
				boError() << k_funcinfo << l->id << " has invalid next pointer" << endl;
				return false;
			}
			if (!l->mPrevious) {
				boError() << k_funcinfo << l->id << " has invalid previous pointer" << endl;
				return false;
			}
			if (l == l->mNext) {
				if (l != l->mPrevious) {
					boError() << k_funcinfo << l->id << ": next points to this, but previous does not" << endl;
					return false;
				}
			} else {
				if (l == l->mPrevious) {
					boError() << k_funcinfo << l->id << ": previous points to this, but next does not" << endl;
					return false;
				}
			}
			if (l->mNext->mPrevious != l) {
				boError() << k_funcinfo << "previous of next doesnt point to this" << endl;
				return false;
			}
			if (l->mPrevious->mNext != l) {
				boError() << k_funcinfo << "next of previous doesnt point to this" << endl;
				return false;
			}
		}
		return true;
	}
	void dumpLinkedList(int maxLines = 5000) const
	{
		const LineSegment* first = this;
		const LineSegment* l = first;
		QString list = QString::number(first->id);
		int lines = 1;
		do {
			l = l->mNext;
			if (l) {
				list += QString(" -> %1").arg(l->id);
			} else {
				list += QString(" -> (null)");
			}
		} while (l && l != first && lines < maxLines);
		if (l == first) {
			list += QString(" -> %1").arg(l->id);
		}
		boDebug() << "list: " << list << endl;
		if (lines >= maxLines) {
			boDebug() << "(maxLines=" << maxLines << " reached)" << endl;
		}
	}
#endif

	BoVector3Float mStart;
	BoVector3Float mEnd;
	LineSegment* mNext;
	LineSegment* mPrevious;
	int id;
 };

 // AB: we use at most 30 lines. this is a random value and we will never use
 // all of them. if someone is willing to find out how many we need at most and
 // is able to prove that - let me know and/or change this!
#define MAX_LINES 30
 LineSegment line[MAX_LINES];
 for (int i = 0; i < MAX_LINES; i++) {
	line[i].id = i;
 }
 LineSegment* first = &line[0];

 line[0].mStart.set(0.0f, 0.0f, 0.0f);
 line[0].mEnd.set(mapRight, 0.0f, 0.0f);
 line[0].mNext = &line[1];

 line[1].mPrevious = &line[0];
 line[1].mStart.set(mapRight, 0.0f, 0.0f);
 line[1].mEnd.set(mapRight, mapBottom, 0.0f);
 line[1].mNext = &line[2];

 line[2].mPrevious = &line[1];
 line[2].mStart.set(mapRight, mapBottom, 0.0f);
 line[2].mEnd.set(0.0f, mapBottom, 0.0f);
 line[2].mNext = &line[3];

 line[3].mPrevious = &line[2];
 line[3].mStart.set(0.0f, mapBottom, 0.0f);
 line[3].mEnd.set(0.0f, 0.0f, 0.0f);
 line[3].mNext = &line[0];

 line[0].mPrevious = &line[3];
 int newLine = 4;

 for (int i = 0; i < 6; i++) {
	const float* plane = &viewFrustum()[i * 4];
	// the possibilities:
	// - the line is completely in front of the plane. nothing to do then.
	// - the line intersects with the plane at some point.
	//    we "cut" the line here, i.e. replace the start/end by the
	//    intersecting point. this will create an additional line with
	//    the start=intersecting point and end=start/end of the
	//    next/previous line.
	// - the line is completely behind the plane.
	//    here we will have intersections with the plane and the next and
	//    previous line (otherwise nothing would be visible at all). we
	//    replace this line by the intersecting points of the next/previous
	//    lines.
	//
	// UPDATE: the code using the above possibilities is working now
	// (03/09/10). the above 3 possibilities are correct, but the code is
	// gonna be a lot more complex, es a plane might intersect with several
	// lines at once and we must fix the start/end points of these lines
	// before adding the new lines.

	// first pass - test whether the line segments intersect at all.
	// non-intersecting non-visible lines get removed.
	LineSegment* l = first;
	do {
		if (!first) {
			first = l;
		}
#if 0
		// for debugging. this will catch most pointer errors in the
		// list, except of one. imagine:
		//  1->2->3->2
		// => "1" is "first" pointer, all pointers are valid, but we
		// will never terminate.
		// of course, such an error must not occur anyway! (not even
		// those that will be catched by this code should ever be
		// reached)
		if (!l->checkPointers(l)) {
			boError() << k_funcinfo << "pass1: argh! at line segment " << l->id << endl;
			return;
		}
		if (!l->checkSegment(MAX_LINES)) {
			boError() << k_funcinfo << "pass1: argh2! at line segment " << l->id << endl;
			return;
		}
#endif
		bool intersects = false;
		BoVector3Float intersection; // point where the line intersects
		intersects = lineSegmentIntersects(plane, l->mStart, l->mEnd, &intersection);
		if (!l->mNext || !l->mPrevious) {
			boError() << k_funcinfo << "oops null next/previous pointer (pass1)" << endl;
			return;
		}
		if (!intersects) {
			if (isInFrontOfPlane(plane, l->mStart)) {
				// good - the entire line is in front of the
				// plane. we are done here.
			} else {
				// the entire line is behind the plane
				// it is removed now.
				if (l->mNext == l) {
					if (l->mPrevious != l) {
						boError() << k_funcinfo << "next line points to line, but previous line does not!" << endl;
						return;
					}
					boWarning() << k_funcinfo << "removing last line. nothing visible?!" << endl;
					return;
				}
				l->mPrevious->mNext = l->mNext;
				l->mNext->mPrevious = l->mPrevious;
//				boDebug() << "removing line " << l->id << " start=" << l->mStart.debugString() << " end=" << l->mEnd.debugString() << endl;
				if (l == first) {
					first = 0;
				}
			}
		} else {
			// we care about this in the 2nd pass
		}
		l = l->mNext;
	} while (l && l != first);


	if (!first) {
		return;
	}

	bool fixStart[MAX_LINES];
	bool fixEnd[MAX_LINES];
	LineSegment* addedLines[MAX_LINES];
	int addedPos = 0;
	for (int j = 0; j < MAX_LINES; j++) {
		fixStart[j] = fixEnd[j] = false;
		addedLines[j] = 0;
	}

	// 2nd pass. all lines are either in front of the plane or do intersect
	// with it.
	l = first;
	do {
		bool intersects = false;
		BoVector3Float intersection; // point where the line intersects
		intersects = lineSegmentIntersects(plane, l->mStart, l->mEnd, &intersection);
		if (!intersects) {
//			boDebug() << "no intersection - line start=" << l->mStart.debugString() << " end=" << l->mEnd.debugString() << endl;
			if (isInFrontOfPlane(plane, l->mStart)) {
				// good - the entire line is in front of the
				// plane. we are done here.
			} else {
				// should not be possible in pass 2!
				boError() << k_funcinfo << "line does not intersect with plane! baaaad at this point - line " << l->id << endl;
				return;
			}
		} else {
			// here we also have to add a new line!
//			boDebug() << "line " << l->id << " intersects start=" << l->mStart.debugString() << " end=" << l->mEnd.debugString() << " intersection=" << intersection.debugString() << endl;
			if (isInFrontOfPlane(plane, l->mStart)) {
				// mEnd must be replaced by intersection
				l->mEnd = intersection;
				line[newLine].mPrevious = l;
				line[newLine].mNext = l->mNext;
				line[newLine].mEnd = l->mNext->mStart;
				line[newLine].mStart = intersection;
				fixEnd[newLine] = true;
				newLine++;
			} else {
				// mStart must be replaced by intersection
				l->mStart = intersection;
				line[newLine].mNext = l;
				line[newLine].mPrevious = l->mPrevious;
				line[newLine].mStart = l->mPrevious->mEnd;
				line[newLine].mEnd = intersection;
				fixStart[newLine] = true;
				newLine++;
			}
			addedLines[addedPos] = &line[newLine-1];
			addedPos++;
//			boDebug() << " added line " << newLine-1 << " start=" << line[newLine-1].mStart.debugString() << " end=" << line[newLine-1].mEnd.debugString() << endl;
			if (newLine >= MAX_LINES) {
				// something evil must have happened
				boError() << k_funcinfo << "too many new lines!" << endl;
				return;
			}
		}
		l = l->mNext;
	} while (l && l != first);

	if (!first) {
		boWarning() << "nothing visible?" << endl;
		return;
	}

	// 3rd pass. add new lines to the list
	for (int j = 0; j < MAX_LINES && addedLines[j] != 0; j++) {
		l = addedLines[j];
		if (l->id == -1) {
			continue;
		}
		LineSegment* n = l->mNext;
		LineSegment* p = l->mPrevious;
		if (!n || !p) {
			boError() << k_funcinfo << "invalid line segment" << endl;
			continue;
		}
		if (n->mPrevious != p) {
			boError() << k_funcinfo << "cannot insert at invalid position - n->previous != previous" << endl;
			continue;
//			return;
		}
		if (p->mNext != n) {
			boError() << k_funcinfo << "cannot insert at invalid position - p->next != next" << endl;
			continue;
//			return;
		}
		// nothing went wrong. let's insert.
		p->mNext = l;
		n->mPrevious = l;

		// fix position
		if (fixStart[l->id]) {
			l->mStart = l->mPrevious->mEnd;
		} else if (fixEnd[l->id]) {
			l->mEnd = l->mNext->mStart;
		}
		for (int k = j + 1; k < MAX_LINES && addedLines[k] != 0; k++) {
			LineSegment* s = addedLines[k];
			if (s->mNext == n) {
				if (s->mPrevious != p) {
					// s->next == next && s->previous !=
					// previous must not happen.
					return;
				}
				if (s->mStart.isEqual(l->mEnd)) {
					s->mPrevious = l;
				} else if (s->mEnd.isEqual(l->mStart)) {
					s->mNext = l;
				} else if (s->mEnd.isEqual(l->mEnd) && s->mStart.isEqual(l->mStart)) {
					// we already have this line.
					s->id = -1;
				} else {
					// something weird happened.
					// dont return - just skip it
					s->id = -1;
				}
			} else if (s->mPrevious == p) {
				// s->next == next && s->previous !=
				// previous must not happen.
				return;
			}
		}
	}
 }


 if (!first) {
	boWarning() << k_funcinfo << "nothing visible" << endl;
	return;
 }


 // we return an axis aligned rect - in a future version we should drop that and
 // return all lines
 LineSegment* l = first;
 // initialize max x/y to the minimum and min x/y to the maximum.
 *maxX = *maxY = 0.0f;
 *minX = (mapWidth - 1);
 *minY = (mapHeight - 1);
 int lineCount = 0;
 do {
	*maxX = QMAX(*maxX, l->mStart[0]);
	*maxX = QMAX(*maxX, l->mEnd[0]);
	*minX = QMIN(*minX, l->mStart[0]);
	*minX = QMIN(*minX, l->mEnd[0]);

	*maxY = QMAX(*maxY, -l->mStart[1]);
	*maxY = QMAX(*maxY, -l->mEnd[1]);
	*minY = QMIN(*minY, -l->mStart[1]);
	*minY = QMIN(*minY, -l->mEnd[1]);
//	boDebug() << l->id << ": " << l->mStart.debugString() << " " << l->mEnd.debugString() << endl;
	l = l->mNext;
	lineCount++;
	if (lineCount >= MAX_LINES) {
		boError() << k_funcinfo << "internal error. lineCount >= MAX_LINES. must never happen!" << endl;
		return;
	}
 } while (l && l != first);

// boDebug() << "relevant lines: " << lineCount << endl;

 *maxX = QMAX(0, *maxX);
 *maxY = QMAX(0, *maxY);
 *minX = QMAX(0, *minX);
 *minY = QMAX(0, *minY);
 *maxX = QMIN((mapWidth - 1), *maxX);
 *minX = QMIN((mapWidth - 1), *minX);
 *maxY = QMIN((mapHeight - 1), *maxY);
 *minY = QMIN((mapHeight - 1), *minY);

 *minY *= -1;
 *maxY *= -1;

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
	QPtrList<const CellTreeNode>* list = mLeafs[i];
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

bool CellListBuilderTree::doLOD(const CellTreeNode* node) const
{
 if (!node) {
	return false;
 }
 const int count = node->mCellsCount;
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

void CellListBuilderTree::addCells(int* cells, const CellTreeNode* node, int depth)
{
 if (!node) {
	return;
 }
 const int l = node->mLeft;
 const int t = node->mTop;
 const int r = node->mRight;
 const int b = node->mBottom;
 if (doLOD(node)) {
	BoGroundRenderer::setCell(cells, mCount, l, t, r - l + 1, b - t + 1);
	mCount++;
	if ((int)mLeafs.size() < depth + 1) {
		int s = mLeafs.size();
		mLeafs.resize(depth + 1);
		for (int i = s; i < (int)mLeafs.size(); i++) {
			mLeafs[i] = new QPtrList<const CellTreeNode>();
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
	addCells(cells, node->mTopLeft, depth + 1);
	addCells(cells, node->mTopRight, depth + 1);
	addCells(cells, node->mBottomLeft, depth + 1);
	addCells(cells, node->mBottomRight, depth + 1);
 }
}

bool CellListBuilderTree::cellsVisible(const CellTreeNode* node, bool* partially) const
{
 g_cellsVisibleCalls++;
 if (!node) {
	*partially = false;
	return false;
 }
 const int x = node->mLeft;
 const int x2 = node->mRight;
 const int y = node->mTop;
 const int y2 = node->mBottom;

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

void CellListBuilderTree::addVisibleCells(int* cells, const CellTreeNode* node, int depth)
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
		addVisibleCells(cells, node->mTopLeft, depth + 1);
		addVisibleCells(cells, node->mTopRight, depth + 1);
		addVisibleCells(cells, node->mBottomLeft, depth + 1);
		addVisibleCells(cells, node->mBottomRight, depth + 1);
	}
 }
}

void CellListBuilderTree::recreateTree(const BosonMap* map)
{
 BO_CHECK_NULL_RET(map);

 delete mRoot;
 mRoot = new CellTreeNode(0, 0, map->width() - 1, map->height() - 1);
 mRoot->createChilds(map);
}

static float distanceFromPlane(const float* plane, const BoVector3Float& pos)
{
 return pos.x() * plane[0] + pos.y() * plane[1] + pos.z() * plane[2] + plane[3];
}
static float distanceFromPlane(const float* plane, const CellTreeNode* node, const BosonMap* map)
{
 const int l = node->mLeft;
 const int t = node->mTop;
 const int r = node->mRight;
 const int b = node->mBottom;
 const float x = (float)node->mLeft;
 const float y = (float)-node->mTop;
 const float x2 = ((float)node->mRight) + 1.0f;
 const float y2 = ((float)-node->mBottom) - 1.0f;
 const float zTopLeft = map->heightAtCorner(l, t);
 const float zTopRight = map->heightAtCorner(r + 1, t);
 const float zBottomLeft = map->heightAtCorner(l, b + 1);
 const float zBottomRight = map->heightAtCorner(r + 1, b + 1);
 const float d1 = distanceFromPlane(plane, BoVector3Float(x, y, zTopLeft));
 const float d2 = distanceFromPlane(plane, BoVector3Float(x2, y, zTopRight));
 const float d3 = distanceFromPlane(plane, BoVector3Float(x, y2, zBottomLeft));
 const float d4 = distanceFromPlane(plane, BoVector3Float(x2, y2, zBottomRight));
 float d = QMAX(d1, d2);
 d = QMAX(d, d3);
 d = QMAX(d, d4);
 return d;
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

// s += QString("distance from BOTTOM plane: %1\n").arg(distanceFromPlane(bottomPlane, pos), 6, 'f', 3);
 s += QString("distance from NEAR plane: %1\n").arg(distanceFromPlane(nearPlane, pos.toFloat()), 6, 'f', 3);
// s += QString("distance from FAR plane: %1\n").arg(distanceFromPlane(farPlane, pos), 6, 'f', 3);

 return s;
}

