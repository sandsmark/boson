/*
    This file is part of the Boson game
    Copyright (C) 2001-2006 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonglminimapview.h"
#include "bosonglminimapview.moc"

#include "../../bomemory/bodummymemory.h"
#include "../bo3dtools.h"
#include "bodebug.h"
#include "../bosonprofiling.h"
#include "../defines.h"
#include "../boufo/boufozoomscrollviewporthandler.h"
#include <bogl.h>

#include <klocale.h>

#include <qvaluelist.h>
#include <qpair.h>

static void cut_line_segment_at_plane(const BoPlane& plane, BoVector3Float& linePoint1, BoVector3Float& linePoint2);
static void cutLineZ0(BoVector3Float& p1_, BoVector3Float& p2_);
static void drawLine(const BoVector3Float& p1_, const BoVector3Float& p2_, int w, int h);
static void keepLinesInRect(int w, int h, BoVector3Float& p1, BoVector3Float& p2, bool* skip);

class CameraLines
{
public:
	CameraLines();

	BoVector3Float BLF; // bottom-left-far
	BoVector3Float BRF; // bottom-right-far
	BoVector3Float BRN; // bottom-right-near
	BoVector3Float BLN; // bottom-left-near
	BoVector3Float TLF; // top-left-far
	BoVector3Float TRF; // top-right-far
	BoVector3Float TRN; // top-right-near
	BoVector3Float TLN; // top-left-near

	QValueList< QPair<BoVector3Float*, BoVector3Float*> > mAllLines;
	QValueList<BoVector3Float*> mAllPoints;

	/**
	 * Estimated (!!) center of the view frustum on the z=0 plane. In cell
	 * coordinates.
	 *
	 * This value is calculated from the camera lines, so if they are
	 * calculated incorrectly, this value will be, too, most of the time.
	 **/
	BoVector3Float mCenterCell;

	/**
	 * Retrieve the viewfrustum lines from the given @p viewFrustum.
	 *
	 * The lines are those that intersect the z=0 plane and are meant to be
	 * displayed in the minimap.
	 **/
	void retrieve(const BoFrustum& viewFrustum);

protected:
	void calculateCenterCell();
};

CameraLines::CameraLines()
{
 mAllLines.append(QPair<BoVector3Float*, BoVector3Float*>(&BLF, &BRF));
 mAllLines.append(QPair<BoVector3Float*, BoVector3Float*>(&BRF, &BRN));
 mAllLines.append(QPair<BoVector3Float*, BoVector3Float*>(&BRN, &BLN));
 mAllLines.append(QPair<BoVector3Float*, BoVector3Float*>(&BLN, &BLF));
 mAllLines.append(QPair<BoVector3Float*, BoVector3Float*>(&TLF, &TRF));
 mAllLines.append(QPair<BoVector3Float*, BoVector3Float*>(&TRF, &TRN));
 mAllLines.append(QPair<BoVector3Float*, BoVector3Float*>(&TRN, &TLN));
 mAllLines.append(QPair<BoVector3Float*, BoVector3Float*>(&TLN, &TLF));
 mAllLines.append(QPair<BoVector3Float*, BoVector3Float*>(&BLF, &TLF));
 mAllLines.append(QPair<BoVector3Float*, BoVector3Float*>(&BRF, &TRF));
 mAllLines.append(QPair<BoVector3Float*, BoVector3Float*>(&BRN, &TRN));
 mAllLines.append(QPair<BoVector3Float*, BoVector3Float*>(&BLN, &TLN));

 mAllPoints.append(&BLF);
 mAllPoints.append(&BRF);
 mAllPoints.append(&BRN);
 mAllPoints.append(&BLN);
 mAllPoints.append(&TLF);
 mAllPoints.append(&TRF);
 mAllPoints.append(&TRN);
 mAllPoints.append(&TLN);
}

void CameraLines::retrieve(const BoFrustum& viewFrustum)
{
 // extract points from the viewfrustum
 // we have planes RIGHT, LEFT, BOTTOM, TOP, FAR, NEAR, we are going to name
 // lines and points accordingly (point at LEFT/BOTTOM/NEAR planes is LBN)
 const BoPlane& planeRight  = viewFrustum.right();
 const BoPlane& planeLeft   = viewFrustum.left();
 const BoPlane& planeBottom = viewFrustum.bottom();
 const BoPlane& planeTop    = viewFrustum.top();
 const BoPlane& planeFar    = viewFrustum.far();
 const BoPlane& planeNear   = viewFrustum.near();

 // intersecting lines first
 // every line consists of a point and a direction
 BoVector3Float LF_point;
 BoVector3Float LF_dir;
 BoPlane::intersectPlane(planeLeft, planeFar, &LF_point, &LF_dir);

 BoVector3Float RF_point;
 BoVector3Float RF_dir;
 BoPlane::intersectPlane(planeRight, planeFar, &RF_point, &RF_dir);

 BoVector3Float RN_point;
 BoVector3Float RN_dir;
 BoPlane::intersectPlane(planeRight, planeNear, &RN_point, &RN_dir);

 BoVector3Float LN_point;
 BoVector3Float LN_dir;
 BoPlane::intersectPlane(planeLeft, planeNear, &LN_point, &LN_dir);

 // AB: we do not need all possible lines
#if 0
 BoVector3Float TN_point;
 BoVector3Float TN_dir;
 BoPlane::intersectPlane(planeTop, planeNear, &TN_point, &TN_dir);

 BoVector3Float TL_point;
 BoVector3Float TL_dir;
 BoPlane::intersectPlane(planeTop, planeLeft, &TL_point, &TL_dir);

 BoVector3Float TF_point;
 BoVector3Float TF_dir;
 BoPlane::intersectPlane(planeTop, planeFar, &TF_point, &TF_dir);

 BoVector3Float TR_point;
 BoVector3Float TR_dir;
 BoPlane::intersectPlane(planeTop, planeRight, &TR_point, &TR_dir);

 BoVector3Float BN_point;
 BoVector3Float BN_dir;
 BoPlane::intersectPlane(planeBottom, planeNear, &BN_point, &BN_dir);

 BoVector3Float BL_point;
 BoVector3Float BL_dir;
 BoPlane::intersectPlane(planeBottom, planeLeft, &BL_point, &BL_dir);

 BoVector3Float BF_point;
 BoVector3Float BF_dir;
 BoPlane::intersectPlane(planeBottom, planeFar, &BF_point, &BF_dir);

 BoVector3Float BR_point;
 BoVector3Float BR_dir;
 BoPlane::intersectPlane(planeBottom, planeRight, &BR_point, &BR_dir);
#endif

 // now retrieve all points using the lines.
 // note that we must not do line-line intersection, as that would be highly
 // inaccurate. we use line-plane intersection instead, which provides more
 // accurate results
 planeBottom.intersectLine(LF_point, LF_dir, &BLF);
 planeBottom.intersectLine(RF_point, RF_dir, &BRF);
 planeBottom.intersectLine(RN_point, RN_dir, &BRN);
 planeBottom.intersectLine(LN_point, LN_dir, &BLN);
 planeTop.intersectLine(LF_point, LF_dir, &TLF);
 planeTop.intersectLine(RF_point, RF_dir, &TRF);
 planeTop.intersectLine(RN_point, RN_dir, &TRN);
 planeTop.intersectLine(LN_point, LN_dir, &TLN);

 // now intersect with the z=0 plane.
 // this is a special case intersection and it can be done much simpler:
 // AB: maybe use plane_line_intersect anyway because of consistency?
 //     or rather implement plane_line_segment_intersect
 //     -> if segment doesnt intersect: do nothing. if it does: replace point of
 //     segment with z < 0 by the intersection point
 cutLineZ0(BLF, BRF);
 cutLineZ0(BRF, BRN);
 cutLineZ0(BRN, BLN);
 cutLineZ0(BLN, BLF);
 cutLineZ0(TLF, TRF);
 cutLineZ0(TRF, TRN);
 cutLineZ0(TRN, TLN);
 cutLineZ0(TLN, TLF);
 cutLineZ0(BLF, TLF);
 cutLineZ0(BRF, TRF);
 cutLineZ0(BRN, TRN);
 cutLineZ0(BLN, TLN);

 calculateCenterCell();
}

void CameraLines::calculateCenterCell()
{
 // fallback
 mCenterCell = BoVector3Float(0.0f, 0.0f, 0.0f);

 float x = 0.0f;
 float y = 0.0f;
 float z = 0.0f;
 float count = 0.0f;
 for (QValueList<BoVector3Float*>::iterator it = mAllPoints.begin(); it != mAllPoints.end(); ++it) {
	if (fabsf((*it)->z()) > 2.0f) {
		// in theory all points should be at z=0.0f
		// this one is too far away from that, so we won't use it
		continue;
	}

	// protect against large (!) errors.
	//
	// note that checks agains x < 0, or x >= mapWidth won't work correctly,
	// as the camera actually _can_ go off the map partially!
	if (fabsf((*it)->x()) > MAX_MAP_WIDTH + 500.0f) {
		continue;
	}
	if (fabsf((*it)->y()) > MAX_MAP_HEIGHT+ 500.0f) {
		continue;
	}

	x += (*it)->x();
	y += (*it)->y();
	count += 1.0f;
 }
 if (count > 0.0f) {
	x /= count;
	y /= count;
 }
 mCenterCell = BoVector3Float(x, -y, z);
}


class BosonGLMiniMapViewPrivate
{
public:
	BosonGLMiniMapViewPrivate()
	{
		mGameGLMatrices = 0;
	}

	const BoGLMatrices* mGameGLMatrices;
	CameraLines mCameraLines;

	BoUfoZoomScrollViewportHandler* mViewport;
};

BosonGLMiniMapView::BosonGLMiniMapView(const BoGLMatrices* gameGLMatrices, QObject* parent)
	: BosonGLCompleteMiniMap(parent)
{
 d = new BosonGLMiniMapViewPrivate;
 d->mViewport = new BoUfoZoomScrollViewportHandler(this);

 d->mGameGLMatrices = gameGLMatrices;
}

BosonGLMiniMapView::~BosonGLMiniMapView()
{
 delete d->mViewport;
 delete d;
}

void BosonGLMiniMapView::createMap(unsigned int w, unsigned int h)
{
 BosonGLCompleteMiniMap::createMap(w, h);
 d->mViewport->setDataSize(w, h);
}

void BosonGLMiniMapView::setViewSize(unsigned int w, unsigned int h)
{
 d->mViewport->setViewSize(w, h);
}

void BosonGLMiniMapView::centerViewOnCell(int x, int y)
{
 d->mViewport->centerViewOnDataPoint(x, y);
}

int BosonGLMiniMapView::viewCenterX() const
{
 return d->mViewport->viewCenterX();
}

int BosonGLMiniMapView::viewCenterY() const
{
 return d->mViewport->viewCenterY();
}

void BosonGLMiniMapView::zoomIn()
{
 d->mViewport->zoomIn();
}

void BosonGLMiniMapView::zoomOut()
{
 d->mViewport->zoomOut();
}

void BosonGLMiniMapView::render()
{
 const BoFrustum& viewFrustum = d->mGameGLMatrices->viewFrustum();
 d->mCameraLines.retrieve(viewFrustum);
 centerViewOnCell((int)d->mCameraLines.mCenterCell.x(), (int)d->mCameraLines.mCenterCell.y());

 glPushMatrix();

 glMultMatrixf(d->mViewport->transformationMatrix().data());

 BosonGLCompleteMiniMap::render();
 renderCamera();

 glPopMatrix();
}

void BosonGLMiniMapView::renderCamera()
{
 BoVector3Float BLF = d->mCameraLines.BLF;
 BoVector3Float BRF = d->mCameraLines.BRF;
 BoVector3Float BRN = d->mCameraLines.BRN;
 BoVector3Float BLN = d->mCameraLines.BLN;
 BoVector3Float TLF = d->mCameraLines.TLF;
 BoVector3Float TRF = d->mCameraLines.TRF;
 BoVector3Float TRN = d->mCameraLines.TRN;
 BoVector3Float TLN = d->mCameraLines.TLN;

 glDisable(GL_TEXTURE_2D);

 // map world-coordinates to minimap-coordinates
 BLF.setX((BLF.x()) / mapWidth());
 BLF.setY((BLF.y()) / mapHeight() + 1.0f);
 BRF.setX((BRF.x()) / mapWidth());
 BRF.setY((BRF.y()) / mapHeight() + 1.0f);
 BRN.setX((BRN.x()) / mapWidth());
 BRN.setY((BRN.y()) / mapHeight() + 1.0f);
 BLN.setX((BLN.x()) / mapWidth());
 BLN.setY((BLN.y()) / mapHeight() + 1.0f);
 TLF.setX((TLF.x()) / mapWidth());
 TLF.setY((TLF.y()) / mapHeight() + 1.0f);
 TRF.setX((TRF.x()) / mapWidth());
 TRF.setY((TRF.y()) / mapHeight() + 1.0f);
 TRN.setX((TRN.x()) / mapWidth());
 TRN.setY((TRN.y()) / mapHeight() + 1.0f);
 TLN.setX((TLN.x()) / mapWidth());
 TLN.setY((TLN.y()) / mapHeight() + 1.0f);

 // Render a semitransparent yellow quad to better illustrate the visible area
// glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);
 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glShadeModel(GL_SMOOTH);
 glBegin(GL_QUADS);
	// Far side of the quad is more transparent...
	glColor4f(1.0, 1.0, 0.0, 0.15);
	glVertex3fv(TLF.data());
	glVertex3fv(TRF.data());
	// ... and the front one is less
	glColor4f(1.0, 1.0, 0.0, 0.25);
	glVertex3fv(BRF.data());
	glVertex3fv(BLF.data());
 glEnd();

 // now the points should be final - we can draw our lines onto the minimap
 glColor3ub(160, 160, 160);
 glEnable(GL_LINE_SMOOTH);
 glBegin(GL_LINES);
	drawLine(BLF, BRF, 1, 1);
	//drawLine(BRF, BRN, 1, 1);
	//drawLine(BRN, BLN, 1, 1);
	//drawLine(BLN, BLF, 1, 1);
	drawLine(TLF, TRF, 1, 1);
	//drawLine(TRF, TRN, 1, 1);
	//drawLine(TRN, TLN, 1, 1);
	//drawLine(TLN, TLF, 1, 1);
	drawLine(BLF, TLF, 1, 1);
	drawLine(BRF, TRF, 1, 1);
	//drawLine(BRN, TRN, 1, 1);
	//drawLine(BLN, TLN, 1, 1);
 glEnd();
 glDisable(GL_LINE_SMOOTH);
 glDisable(GL_BLEND);
 glColor3ub(255, 255, 255);
}

QPoint BosonGLMiniMapView::widgetToCell(const QPoint& pos) const
{
 return d->mViewport->widgetPointToDataPoint(pos);
}

// cuts the line at z=0.0
static void cutLineZ0(BoVector3Float& p1_, BoVector3Float& p2_)
{
#if 0
 BoVector3Float* p1 = &p1_;
 BoVector3Float* p2 = &p2_;
 if (p1->z() < 0.0f && p2->z() < 0.0f) {
	return;
 }
 if (p2->z() >= 0.0f) {
	if (p1->z() >= 0.0f) {
		return;
	}
	p2 = &p1_;
	p1 = &p2_;
 }
 // now p1 >= 0 && p2 < 0

 BoVector3Float u = *p1 - *p2;
 float s = -p2->z() / u.z();
 p2->setX(p2->x() + s * u.x());
 p2->setY(p2->y() + s * u.y());
 p2->setZ(0.0f);
#else
 BoPlane zPlane(BoVector3Float(0.0f, 0.0f, 1.0f), BoVector3Float(0.0f, 0.0f, 0.0f));
 cut_line_segment_at_plane(zPlane, p1_, p2_);
#endif
}

static void keepLinesInRect(int w, int h, BoVector3Float& p1, BoVector3Float& p2, bool* skip)
{
 // x=0 plane
 BoPlane x_0(BoVector3Float(1.0f, 0.0f, 0.0f), BoVector3Float(0.0f, 0.0f, 0.0f));
 // y=0 plane
 BoPlane y_0(BoVector3Float(0.0f, 1.0f, 0.0f), BoVector3Float(0.0f, 0.0f, 0.0f));
 // x=w plane
 BoPlane x_x(BoVector3Float(-1.0f, 0.0f, 0.0f), BoVector3Float(w, 0.0f, 0.0f));
 // y=h plane
 BoPlane y_y(BoVector3Float(0.0f, -1.0f, 0.0f), BoVector3Float(0.0f, h, 0.0f));


 cut_line_segment_at_plane(x_0, p1, p2);
 cut_line_segment_at_plane(x_x, p1, p2);
 cut_line_segment_at_plane(y_0, p1, p2);
 cut_line_segment_at_plane(y_y, p1, p2);

 if (x_0.behindPlane(p1) && x_0.behindPlane(p2)) {
	*skip = true;
	return;
 }
 if (y_0.behindPlane(p1) && y_0.behindPlane(p2)) {
	*skip = true;
	return;
 }
 if (x_x.behindPlane(p1) && x_x.behindPlane(p2)) {
	*skip = true;
	return;
 }
 if (y_y.behindPlane(p1) && y_y.behindPlane(p2)) {
	*skip = true;
	return;
 }
}

static void drawLine(const BoVector3Float& p1_, const BoVector3Float& p2_, int w, int h)
{
 BoVector3Float p1(p1_);
 BoVector3Float p2(p2_);
 bool skip = false;
 keepLinesInRect(w, h, p1, p2, &skip);
 if (skip) {
	return;
 }
 glVertex3fv(p1.data());
 glVertex3fv(p2.data());
}


/**
 * Cut the line segment defined by @p linePoint1 and @p linePoint2 at the plane.
 *
 * The line that is behind the plane (see @ref BoPlane::behindPlane) is
 * replaced by the intersection point.
 **/
static void cut_line_segment_at_plane(const BoPlane& plane, BoVector3Float& linePoint1, BoVector3Float& linePoint2)
{
 BoVector3Float intersection;
 if (!plane.intersectLineSegment(linePoint1, linePoint2, &intersection)) {
	return;
 }
 if (plane.behindPlane(linePoint1)) {
	linePoint1 = intersection;
 } else {
	linePoint2 = intersection;
 }
}

