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
#include "../defines.h"
#include "../cell.h"
#include "../bo3dtools.h"
#include <bodebug.h>

// not nice in this file. we need it for boGame->status() == KGame::Init
// maybe we should require KGame not to be in init state before constructin the
// class
#include "../boson.h"



#include <klocale.h>

#include <GL/gl.h>
#include <math.h>

// a couple of helper functions. these should be in Bo3dTools.
static bool isInFrontOfPlane(const float* plane, const BoVector3& vector);
static bool lineIntersects(const float* plane, const BoVector3& pos, const BoVector3& direction, BoVector3* intersection);
static bool lineIntersects_points(const float* plane, const BoVector3& start, const BoVector3& end, BoVector3* intersection);
static bool lineSegmentIntersects(const float* plane, const BoVector3& start, const BoVector3& end, BoVector3* intersection);
#define EPSILON 0.0001f // zero for floating point numbers

BoGroundRendererBase::BoGroundRendererBase()
{
}

BoGroundRendererBase::~BoGroundRendererBase()
{
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

void BoGroundRendererBase::calculateWorldRect(const QRect& rect, int mapWidth, int mapHeight, float* minX, float* minY, float* maxX, float* maxY)
{
 BO_CHECK_NULL_RET(viewFrustum());
 GLfloat posX, posY;
 GLfloat posZ;

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


 const float mapRight = (mapWidth - 1) * BO_GL_CELL_SIZE;
 const float mapBottom = -(mapHeight - 1) * BO_GL_CELL_SIZE;

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

	BoVector3 mStart;
	BoVector3 mEnd;
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
		BoVector3 intersection; // point where the line intersects
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
		BoVector3 intersection; // point where the line intersects
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
 *minX = (mapWidth - 1) * BO_GL_CELL_SIZE;
 *minY = (mapHeight - 1) * BO_GL_CELL_SIZE;
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
 *maxX = QMIN((mapWidth - 1) * BO_GL_CELL_SIZE, *maxX);
 *minX = QMIN((mapWidth - 1) * BO_GL_CELL_SIZE, *minX);
 *maxY = QMIN((mapHeight - 1) * BO_GL_CELL_SIZE, *maxY);
 *minY = QMIN((mapHeight - 1) * BO_GL_CELL_SIZE, *minY);

 *minY *= -1;
 *maxY *= -1;

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

 // re-generate the list of to-be-rendered cells:
 Cell* allCells = map->cells();
 if (!allCells) {
	boError() << k_funcinfo << "NULL cells!" << endl;
	return;
 }
 float maxX = 0.0f, maxY = 0.0f;
 float minX = 0.0f, minY = 0.0f;
 calculateWorldRect(QRect(QPoint(0, 0), QPoint(viewport()[2], viewport()[3])),
		map->width(), map->height(),
		&minX, &minY, &maxX, &maxY);
// boDebug() << "world rect: minX=" << minX << " maxX=" << maxX << " minY=" << minY << " maxY=" << maxY << endl;
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
 if (size > renderCellsSize()) {
	setRenderCells(new Cell*[size], size);
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
			renderCells()[count] = c;
			count++;
		}
	}
 }
 setRenderCellsCount(count);
}


