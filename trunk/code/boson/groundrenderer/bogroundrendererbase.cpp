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

// a couple of helper functions. these should be in Bo3dTools.
static bool isInFrontOfPlane(const float* plane, const BoVector3& vector);
static bool lineIntersects(const float* plane, const BoVector3& pos, const BoVector3& direction, BoVector3* intersection);
static bool lineIntersects_points(const float* plane, const BoVector3& start, const BoVector3& end, BoVector3* intersection);
static bool lineSegmentIntersects(const float* plane, const BoVector3& start, const BoVector3& end, BoVector3* intersection);
#define EPSILON 0.0001f // zero for floating point numbers

static int g_cellsVisibleCalls = 0;


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
	}
	virtual ~CellListBuilder() {}

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
	virtual Cell** generateCellList(const BosonMap* map, Cell** renderCells, int* renderCellsSize, unsigned int* renderCellsCount) = 0;

private:
	const float* mViewFrustum;
	const int* mViewport;
};

/**
 * This class uses a tree find out whether cells are visible. Whenever @ref
 * checkCells is called on a valid rect, it calls itself again 4 times. Once for
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
	}
	~CellListBuilderTree() {}

	virtual Cell** generateCellList(const BosonMap* map, Cell** renderCells, int* renderCellsSize, unsigned int* renderCellsCount);

protected:
	/**
	 * This is the main method of this class. It checks (recursively)
	 * whether the cells in ((x,y),(x2,y2)) are visible and adds all visible
	 * cells to @p cells.
	 **/
	void checkCells(Cell** cells, int x, int y, int x2, int y2);

	/**
	 * @return Whether the cells in the rect (x,y) to (x2,y2) are visible.
	 * If they are visible, @p partially is set to FALSE, when all cells are
	 * visible, otherwise to TRUE (rect is partially visible only). If no
	 * cell is visible (i.e. this returns FALSE) @p partially is set to
	 * FALSE.
	 **/
	bool cellsVisible(int x, int y, int x2, int y2, bool* partially) const;

	/**
	 * Add all cells in the rect (x, y) to (x2, y2) to @p cells
	 **/
	void addCells(Cell** cells, int x, int y, int x2, int y2);

private:
	// these variables are valid only while we are in generateCellList() !
	const BosonMap* mMap;
	unsigned int mCount;
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

BoGroundRendererBase::BoGroundRendererBase(bool useCellTree)
{
 mCellListBuilder = 0;
 if (useCellTree) {
	mCellListBuilder = new CellListBuilderTree();
 }
}

BoGroundRendererBase::~BoGroundRendererBase()
{
 delete mCellListBuilder;
}

static bool isInFrontOfPlane(const float* plane, const BoVector3& vector)
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
static bool lineIntersects(const float* plane, const BoVector3& pos, const BoVector3& direction, BoVector3* intersection)
{
 const float planeDistance = plane[3];
 const BoVector3 planeNormal = BoVector3(plane[0], plane[1], plane[2]);

 // see count 2.1.0, base/SbPlane.cpp
 if (fabsf(planeDistance) <= EPSILON) {
	boError() << k_funcinfo << "plane distance is 0" << endl;
	return false;
 }

 // we will _always_ have an intersection of line and plane if the line is not
 // parallel to the plane.
 // (AB: note that I said _line_ not line segment)
 if (fabsf(BoVector3::dotProduct(direction, planeNormal)) <= EPSILON) {
	return false;
 }

 // this formula is from coin 2.1.0, base/SbPlane.cpp, SbPlane::intersect(). it
 // is documented there very well.
 // AB: to me this looks like an error in coin. plib1.5 uses a slightly
 // different version and this seems to work better
 float t = -(planeDistance + BoVector3::dotProduct(planeNormal, pos)) / BoVector3::dotProduct(planeNormal, direction);

 *intersection = pos + direction * t;

 // AB: t is between 0 and 1 if the intersection is on the segment (at least it
 // should be)

 return true;
}

// just like above, but takes 2 points on the line, not 1 point and a direction
static bool lineIntersects_points(const float* plane, const BoVector3& start, const BoVector3& end, BoVector3* intersection)
{
 const BoVector3 pos = start;
 BoVector3 direction = end - start;
 return lineIntersects(plane, pos, direction, intersection);
}

static bool lineSegmentIntersects(const float* plane, const BoVector3& start, const BoVector3& end, BoVector3* intersection)
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
 int renderCellsSize = 0;
 unsigned int renderCellsCount = 0;
 Cell** originalList = renderCells();
 mCellListBuilder->setViewFrustum(viewFrustum());
 mCellListBuilder->setViewport(viewport());
 Cell** renderCells = mCellListBuilder->generateCellList(map, originalList, &renderCellsSize, &renderCellsCount);
 if (renderCells != originalList) {
	setRenderCells(renderCells, renderCellsSize);
 }
 setRenderCellsCount(renderCellsCount);
}

Cell** CellListBuilderTree::generateCellList(const BosonMap* map, Cell** origRenderCells, int* renderCellsSize, unsigned int* renderCellsCount)
{
 if (!map) {
	BO_NULL_ERROR(map);
	return origRenderCells;
 }
 static int profiling_id = boProfiling->requestEventId("generateCellList");
 BosonProfiler prof(profiling_id);
 Cell** renderCells = origRenderCells;
 if (*renderCellsSize < (int)(map->width() * map->height())) {
	// we don't know in advance how many cells we might need, so we allocate
	// width * height
	*renderCellsSize = map->width() * map->height();
	renderCells = new Cell*[*renderCellsSize];
 }
 mMap = map;
 mCount = 0;

 g_cellsVisibleCalls = 0;
 checkCells(renderCells, 0, 0, map->width() - 1, map->height() - 1);
// boDebug() << k_funcinfo << g_cellsVisibleCalls << " calls - will render cells: " << mCount << endl;

 *renderCellsCount = mCount;
 mMap = 0;
 mCount = 0;
 return renderCells;
}

void CellListBuilderTree::addCells(Cell** cells, int l, int t, int r, int b)
{
// boDebug() << k_funcinfo << "l=" << l << ",t=" << t << ",r=" << r << ",b=" << b << endl;
 for (int x = l; x <= r; x++) {
	for (int y = t; y <= b; y++) {
		cells[mCount] = mMap->cell(x, y);
		mCount++;
	}
 }
}

bool CellListBuilderTree::cellsVisible(int x, int y, int x2, int y2, bool* partially) const
{
 g_cellsVisibleCalls++;
 if (x2 < x || y2 < y) {
	*partially = false;
	return false;
 }
 if (x2 >= (int)mMap->width()) {
	boError() << k_funcinfo << "x2 too high: " << x2 << endl;
	return false; // we want people to notice the bug
 }
 if (y2 >= (int)mMap->height()) {
	boError() << k_funcinfo << "y2 too high: " << y2 << endl;
	return false; // we want people to notice the bug
 }
 if (x < 0) {
	boError() << k_funcinfo << "x is negative: " << x << endl;
	return false; // we want people to notice the bug
 }
 if (y < 0) {
	boError() << k_funcinfo << "y is negative: " << y << endl;
	return false; // we want people to notice the bug
 }

 int w = (x2 + 1) - x; // + 1 because we need the right border of the cell!
 int h = (y2 + 1) - y;
 if (w * h <= 4) {
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
 float r1 = BoVector3(hmid - (float)x,
		vmid - (float)y, z - topLeftZ).dotProduct();
 float r2 = BoVector3(hmid - ((float)x + (float)w),
		vmid - (float)y, z - topRightZ).dotProduct();
 float r3 = BoVector3(hmid - ((float)x + (float)w),
		vmid - ((float)y + (float)h), z - bottomRightZ).dotProduct();
 float r4 = BoVector3(hmid - (float)x,
		vmid - ((float)y + (float)h), z - bottomLeftZ).dotProduct();

 float radius = QMAX(r1, r2);
 radius = QMAX(radius, r3);
 radius = QMAX(radius, r4);
 radius = sqrtf(radius); // turn dotProduct() into length()
 BoVector3 center(hmid, -vmid, z);

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

void CellListBuilderTree::checkCells(Cell** cells, int l, int t, int r, int b)
{
 if (l == r && b == t) {
	return;
 }
 if (l > r || t > b || l < 0 || t < 0) {
	boError() << k_funcinfo << "invalid values: left="
			<< l << ", top=" << t
			<< ", right=" << r
			<< ", bottom=" << b << endl;
	return;
 }
 bool partially = false;
 int hmid = l + (r - l) / 2;
 int vmid = t + (b - t) / 2;
 if (cellsVisible(l, t, hmid, vmid, &partially)) {
	// top-left rect
	if (!partially) {
		// all cells visible
		addCells(cells, l, t, hmid, vmid);
	} else {
		checkCells(cells, l, t, hmid, vmid);
	}
 }
 if (cellsVisible(l, vmid + 1, hmid, b, &partially)) {
	// bottom-left rect
	if (!partially) {
		addCells(cells, l, vmid + 1, hmid, b);
	} else {
		checkCells(cells, l, vmid + 1, hmid, b);
	}
 }
 if (cellsVisible(hmid + 1, t, r, vmid, &partially)) {
	// top-right rect
	if (!partially) {
		// all cells visible
		addCells(cells, hmid + 1, t, r, vmid);
	} else {
		checkCells(cells, hmid + 1, t, r, vmid);
	}
 }
 if (cellsVisible(hmid + 1, vmid + 1, r, b, &partially)) {
	// bottom-right rect
	if (!partially) {
		// all cells visible
		addCells(cells, hmid + 1, vmid + 1, r, b);
	} else {
		checkCells(cells, hmid + 1, vmid + 1, r, b);
	}
 }
}

