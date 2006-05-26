/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonglminimap.h"
#include "bosonglminimap.moc"

#include "defines.h"
#include "../bomemory/bodummymemory.h"
#include "gameengine/cell.h"
#include "gameengine/bosonmap.h"
#include "bosonconfig.h"
#include "gameengine/bosongroundtheme.h"
#include "gameengine/boitemlist.h"
#include "bo3dtools.h"
#include "gameengine/unit.h"
#include "gameengine/player.h"
#include "gameengine/playerio.h"
#include "bodebug.h"
#include "botexture.h"
#include "bosonglwidget.h"
#include "gameengine/rtti.h"
#include "gameengine/unitplugins.h"
#include "gameengine/bosoncanvas.h"
#include "borendertarget.h"
#include "bosonprofiling.h"
#include <bogl.h>

#include <klocale.h>
#include <kstandarddirs.h>

#include <qpixmap.h>
#include <qimage.h>
#include <qpointarray.h>
#include <qvaluelist.h>
#include <qfileinfo.h>
#include <qptrvector.h>
#include <qvaluevector.h>
#include <qptrlist.h>
#include <qgl.h>

#define COLOR_UNKNOWN Qt::black // unexplored terrain
#define ZOOM_STEP 0.5

static void cut_line_segment_at_plane(const BoPlane& plane, BoVector3Float& linePoint1, BoVector3Float& linePoint2);
static void cutLineZ0(BoVector3Float& p1_, BoVector3Float& p2_);
static void drawLine(const BoVector3Float& p1_, const BoVector3Float& p2_, int w, int h);
static void keepLinesInRect(int w, int h, BoVector3Float& p1, BoVector3Float& p2, bool* skip);



#define USE_EXPERIMENTAL_QUADTREE_THINGY
#ifdef USE_EXPERIMENTAL_QUADTREE_THINGY
#include "boquadtreenode.h"

class BosonMiniMapQuadtreeNode : public BoQuadTreeNode
{
public:
	// TODO: use BoRect<int> instead???
	class IntRect
	{
	public:
		IntRect(int _l, int _t, int _r, int _b) { l = _l; t = _t; r = _r; b = _b; }
		IntRect() {}

		bool operator==(const IntRect& rect) { return l == rect.l && t == rect.t && r == rect.r && b == rect.b; };

		int l, t, r, b;
	};


	BosonMiniMapQuadtreeNode(int l, int t, int r, int b, int depth)
			: BoQuadTreeNode(l, t, r, b, depth)
	{
		size = QMAX(r - l, b - t) + 1;
	}

	static BosonMiniMapQuadtreeNode* createTree(unsigned int width, unsigned int height);
	virtual BoQuadTreeNode* createNode(int l, int t, int r, int b, int depth) const;

	void addUnit(Unit* u);
	void removeUnit(Unit* u);
	void unitMoved(Unit* u, bofixed oldX, bofixed oldY);

	BoItemList* approximateUnitsInRect(const BoRect2Fixed& rect) const;
	void approximateUnitsInRect(const IntRect& rect, BoItemList* result) const;

protected:
	class UnitMovementInfo
	{
	public:
		// Old bounding rect of the unit
		IntRect oldRect;
		// New bounding rect of the unit
		IntRect newRect;
	};

	void addUnit(Unit* u, const IntRect& rect);
	void removeUnit(Unit* u, const IntRect& rect);
	void unitMoved(Unit* u, const UnitMovementInfo& moveinfo);

	void addUnitToSelf(Unit* u);
	void removeUnitFromSelf(Unit* u);


	// TODO: move to parent class
	inline bool contains(const BoRect2Fixed& r) const
	{
		return BoQuadTreeNode::contains((int)r.left(), (int)r.top(), (int)r.right(), (int)r.bottom());
	}
	inline bool intersects(const BoRect2Fixed& r) const
	{
		return BoQuadTreeNode::intersects((int)r.left(), (int)r.top(), (int)r.right(), (int)r.bottom());
	}

	inline bool contains(const IntRect& r) const
	{
		return BoQuadTreeNode::contains(r.l, r.t, r.r, r.b);
	}
	inline bool intersects(const IntRect& r) const
	{
		return BoQuadTreeNode::intersects(r.l, r.t, r.r, r.b);
	}

private:
	int size;
	static const int minLeafSize = 4;
	// TODO: sort those by pointer address so that they can be binary searched
	QValueVector<Unit*> flyingUnits;
	QValueVector<Unit*> groundUnits;
};


BosonMiniMapQuadtreeNode* BosonMiniMapQuadtreeNode::createTree(unsigned int w, unsigned int h)
{
 if (w < 1) {
	boError() << k_funcinfo << "invalid width: " << w << endl;
	w = 1;
 }
 if (h < 1) {
	boError() << k_funcinfo << "invalid height: " << h << endl;
	h = 1;
 }
 BosonMiniMapQuadtreeNode* root = new BosonMiniMapQuadtreeNode(0, 0, w - 1, h - 1, 0);
 root->createChilds(w, h);
 return root;
}

BoQuadTreeNode* BosonMiniMapQuadtreeNode::createNode(int l, int t, int r, int b, int depth) const
{
 return new BosonMiniMapQuadtreeNode(l, t, r, b, depth);
}

BoItemList* BosonMiniMapQuadtreeNode::approximateUnitsInRect(const BoRect2Fixed& rect) const
{
 BoItemList* items = new BoItemList;
 // TODO: round right and bottom upwards!
 IntRect irect;
 irect.l = (int)rect.left();
 irect.t = (int)rect.top();
 irect.r = (int)rect.right();
 irect.b = (int)rect.bottom();
 approximateUnitsInRect(irect, items);
 return items;
}

void BosonMiniMapQuadtreeNode::approximateUnitsInRect(const IntRect& rect, BoItemList* result) const
{
 if (contains(rect) || size <= minLeafSize ||
		((rect.l - rect.r >= size/2) && (rect.t - rect.b >= size/2))) {
	for(unsigned int i = 0; i < groundUnits.count(); i++) {
		result->append(groundUnits[i]);
	}
	for(unsigned int i = 0; i < flyingUnits.count(); i++) {
		result->append(flyingUnits[i]);
	}
 } else {
	// Query children
	if (((BosonMiniMapQuadtreeNode*)topLeftNode())->intersects(rect)) {
		((BosonMiniMapQuadtreeNode*)topLeftNode())->approximateUnitsInRect(rect, result);
	}
	if (((BosonMiniMapQuadtreeNode*)topRightNode())->intersects(rect)) {
		((BosonMiniMapQuadtreeNode*)topRightNode())->approximateUnitsInRect(rect, result);
	}
	if (((BosonMiniMapQuadtreeNode*)bottomLeftNode())->intersects(rect)) {
		((BosonMiniMapQuadtreeNode*)bottomLeftNode())->approximateUnitsInRect(rect, result);
	}
	if (((BosonMiniMapQuadtreeNode*)bottomRightNode())->intersects(rect)) {
		((BosonMiniMapQuadtreeNode*)bottomRightNode())->approximateUnitsInRect(rect, result);
	}
 }
}

void BosonMiniMapQuadtreeNode::addUnit(Unit* u)
{
 //boDebug() << "QUADTREE: " << k_funcinfo << u->id() << endl;
 addUnit(u, IntRect((int)u->leftEdge(), (int)u->topEdge(), (int)u->rightEdge(), (int)u->bottomEdge()));
}

void BosonMiniMapQuadtreeNode::addUnit(Unit* u, const IntRect& rect)
{
 // Add to self
 addUnitToSelf(u);

 if (size > minLeafSize) {
	// Add to children
	if (((BosonMiniMapQuadtreeNode*)topLeftNode())->intersects(rect)) {
		((BosonMiniMapQuadtreeNode*)topLeftNode())->addUnit(u, rect);
	}
	if (((BosonMiniMapQuadtreeNode*)topRightNode())->intersects(rect)) {
		((BosonMiniMapQuadtreeNode*)topRightNode())->addUnit(u, rect);
	}
	if (((BosonMiniMapQuadtreeNode*)bottomLeftNode())->intersects(rect)) {
		((BosonMiniMapQuadtreeNode*)bottomLeftNode())->addUnit(u, rect);
	}
	if (((BosonMiniMapQuadtreeNode*)bottomRightNode())->intersects(rect)) {
		((BosonMiniMapQuadtreeNode*)bottomRightNode())->addUnit(u, rect);
	}
 }
}

void BosonMiniMapQuadtreeNode::addUnitToSelf(Unit* u)
{
 // Add to self
 if (u->isFlying()) {
	//boDebug() << "QUADTREE: " << k_funcinfo << "size; " << size << "; adding flying unit " << u->id() << endl;
	flyingUnits.append(u);
 } else {
	//boDebug() << "QUADTREE: " << k_funcinfo << "size; " << size << "; adding ground unit " << u->id() << endl;
	groundUnits.append(u);
 }
 //boDebug() << "QUADTREE: " << k_funcinfo << "s: " << size << "; loc: (" << left() << "; " << top() <<
//		"); I now have " << flyingUnits.count() << " flying and " << groundUnits.count() << " ground units" << endl;
}

void BosonMiniMapQuadtreeNode::removeUnit(Unit* u)
{
 //boDebug() << "QUADTREE: " << k_funcinfo << u->id() << endl;
 removeUnit(u, IntRect((int)u->leftEdge(), (int)u->topEdge(), (int)u->rightEdge(), (int)u->bottomEdge()));
}

void BosonMiniMapQuadtreeNode::removeUnit(Unit* u, const IntRect& rect)
{
 // Remove from self
 removeUnitFromSelf(u);

 if (size > minLeafSize) {
	// Remove from children
	if (((BosonMiniMapQuadtreeNode*)topLeftNode())->intersects(rect)) {
		((BosonMiniMapQuadtreeNode*)topLeftNode())->removeUnit(u, rect);
	}
	if (((BosonMiniMapQuadtreeNode*)topRightNode())->intersects(rect)) {
		((BosonMiniMapQuadtreeNode*)topRightNode())->removeUnit(u, rect);
	}
	if (((BosonMiniMapQuadtreeNode*)bottomLeftNode())->intersects(rect)) {
		((BosonMiniMapQuadtreeNode*)bottomLeftNode())->removeUnit(u, rect);
	}
	if (((BosonMiniMapQuadtreeNode*)bottomRightNode())->intersects(rect)) {
		((BosonMiniMapQuadtreeNode*)bottomRightNode())->removeUnit(u, rect);
	}
 }
}

void BosonMiniMapQuadtreeNode::removeUnitFromSelf(Unit* u)
{
 //boDebug() << "QUADTREE: " << k_funcinfo << "size: " << size << endl;
 // Remove from self
 QValueVector<Unit*>::Iterator it;
 QValueVector<Unit*>* container;
 if (u->isFlying()) {
	container = &flyingUnits;
 } else {
	container = &groundUnits;
 }
 bool removed = false;
 for (it = container->begin(); it != container->end(); ++it) {
	if (*it == u) {
		//boDebug() << "QUADTREE: " << k_funcinfo << "removing " << u->id() << endl;
		container->erase(it);
		--it;
		removed = true;
	}
 }
 if(!removed) {
	boError() << "QUADTREE: " << k_funcinfo << u->id() << " was not removed!" << endl;
 }
}

void BosonMiniMapQuadtreeNode::unitMoved(Unit* u, bofixed oldX, bofixed oldY)
{
 bofixed deltax = u->x() - oldX;
 bofixed deltay = u->y() - oldY;

 UnitMovementInfo moveinfo;
 moveinfo.newRect.l = (int)u->leftEdge();
 moveinfo.newRect.t = (int)u->topEdge();
 moveinfo.newRect.r = (int)u->rightEdge();
 moveinfo.newRect.b = (int)u->bottomEdge();
 moveinfo.oldRect.l = (int)(u->leftEdge() - deltax);
 moveinfo.oldRect.t = (int)(u->topEdge() - deltay);
 moveinfo.oldRect.r = (int)(u->rightEdge() - deltax);
 moveinfo.oldRect.b = (int)(u->bottomEdge() - deltay);

 if (moveinfo.newRect == moveinfo.oldRect) {
	return;
 }


 unitMoved(u, moveinfo);
}

void BosonMiniMapQuadtreeNode::unitMoved(Unit* u, const UnitMovementInfo& moveinfo)
{
 bool didContain = intersects(moveinfo.oldRect);
 bool nowContains = intersects(moveinfo.newRect);

 if (didContain && !nowContains) {
	// Unit is removed from this node as well as from it's child nodes
	removeUnit(u, moveinfo.oldRect);
 } else if (!didContain && nowContains) {
	// Unit is added to this node as well as to it's child nodes
	addUnit(u, moveinfo.newRect);
 } else if(nowContains && size > minLeafSize) {
	// Unit moved within this node. Notify children
	((BosonMiniMapQuadtreeNode*)topLeftNode())->unitMoved(u, moveinfo);
	((BosonMiniMapQuadtreeNode*)topRightNode())->unitMoved(u, moveinfo);
	((BosonMiniMapQuadtreeNode*)bottomLeftNode())->unitMoved(u, moveinfo);
	((BosonMiniMapQuadtreeNode*)bottomRightNode())->unitMoved(u, moveinfo);
 }
}

#endif



class BosonGLMiniMapPrivate
{
public:
	BosonGLMiniMapPrivate()
	{
		mUnitTree = 0;
	}
	QString mImageTheme;

	QPointArray mSelectionRect;

	float mZoom;

	bool mShowMiniMap;
	QImage mLogo;
	QImage mZoomIn;
	QImage mZoomOut;
	QImage mZoomDefault;

	QPtrList<Unit> mRadars;
	BosonMiniMapQuadtreeNode* mUnitTree;
};

BosonGLMiniMap::BosonGLMiniMap(QObject* parent, const char* name) : QObject(parent, name ? name : "glminimap")
{
 d = new BosonGLMiniMapPrivate;
 d->mShowMiniMap = false;
 d->mZoom = 1.0f;
 d->mSelectionRect.resize(8);

 mRenderer = 0;
 mLocalPlayerIO = 0;
 mCanvas = 0;

 setImageTheme(QString::fromLatin1("standard"));
 slotShowMiniMap(false);
}

BosonGLMiniMap::~BosonGLMiniMap()
{
 boDebug() << k_funcinfo << endl;
 delete mRenderer;
 delete d;
}

void BosonGLMiniMap::quitGame()
{
 setLocalPlayerIO(0);
 delete mRenderer;
 mRenderer = 0;
 mCanvas = 0;
}

void BosonGLMiniMap::setLocalPlayerIO(PlayerIO* io)
{
 mLocalPlayerIO = io;
 if (!io) {
	slotShowMiniMap(false);
 } else {
	initializeItems();
 }
}

bool BosonGLMiniMap::showMiniMap() const
{
 return d->mShowMiniMap;
}

void BosonGLMiniMap::slotShowMiniMap(bool s)
{
 d->mShowMiniMap = s;
}

void BosonGLMiniMap::slotMoveRect(const QPoint& topLeft, const QPoint& topRight, const QPoint& bottomLeft, const QPoint& bottomRight)
{
 if (!hasMap()) {
	return;
 }
 d->mSelectionRect.setPoint(0, topLeft);
 d->mSelectionRect.setPoint(1, topRight);

 d->mSelectionRect.setPoint(2, topRight);
 d->mSelectionRect.setPoint(3, bottomRight);

 d->mSelectionRect.setPoint(4, bottomRight);
 d->mSelectionRect.setPoint(5, bottomLeft);

 d->mSelectionRect.setPoint(6, bottomLeft);
 d->mSelectionRect.setPoint(7, topLeft);
}

void BosonGLMiniMap::slotUnitMoved(Unit* unit, bofixed oldX, bofixed oldY)
{
 if (!hasMap()) {
	return;
 }
#ifdef USE_EXPERIMENTAL_QUADTREE_THINGY
 BosonProfiler p("d->mUnitTree->unitMoved()");
 if (d->mUnitTree) {
	d->mUnitTree->unitMoved(unit, oldX, oldY);
 }
#endif
}

void BosonGLMiniMap::slotUnitRemoved(Unit* unit)
{
 if (!hasMap()) {
	return;
 }
 BO_CHECK_NULL_RET(unit);
#ifdef USE_EXPERIMENTAL_QUADTREE_THINGY
 if (d->mUnitTree) {
	d->mUnitTree->removeUnit(unit);
 }
#endif
 d->mRadars.remove(unit);
}

void BosonGLMiniMap::slotItemAdded(BosonItem* item)
{
 if (!hasMap()) {
	return;
 }
 BO_CHECK_NULL_RET(mLocalPlayerIO);
 BO_CHECK_NULL_RET(item);
 // We're interested only in radars of the localplayer
 if (!RTTI::isUnit(item->rtti())) {
	return;
 }
 Unit* u = (Unit*)item;
#ifdef USE_EXPERIMENTAL_QUADTREE_THINGY
 if (d->mUnitTree) {
	d->mUnitTree->addUnit(u);
 }
#endif
 if (u->owner() != localPlayerIO()->player()) {
	return;
 }
 const RadarPlugin* prop = (RadarPlugin*)u->plugin(UnitPlugin::Radar);
 if (!prop) {
	return;
 }
 // Add it to our radars list
 d->mRadars.append(u);
}

void BosonGLMiniMap::slotFacilityConstructed(Unit* fac)
{
 // Facilities have the construction phase so they have to be rechecked
 //  (whether they are radars) once they are constructed.
 slotItemAdded(fac);
}

QPtrList<Unit>* BosonGLMiniMap::radarList() const
{
 return &d->mRadars;
}

BosonMiniMapQuadtreeNode* BosonGLMiniMap::unitTree() const
{
 return d->mUnitTree;
}

void BosonGLMiniMap::slotUpdateTerrainAtCorner(int x, int y)
{
 calculateGround(x, y);
}

void BosonGLMiniMap::initFogOfWar(PlayerIO* p)
{
 if (!hasMap()) {
	return;
 }
 BO_CHECK_NULL_RET(canvas());
 boDebug() << k_funcinfo << endl;

 if (mRenderer) {
	mRenderer->setUpdatesEnabled(false);
 }

 // AB: add a global PlayerIO to the editor. only use non-NULL IOs here then.
 if (!p) {
	// a NULL playerIO means that we should display the complete map. fog of
	// war gets disabled.
	for (unsigned int i = 0; i < canvas()->mapWidth(); i++) {
		for (unsigned int j = 0; j < canvas()->mapHeight(); j++) {
			slotExplored(i, j);
		}
	}
 } else {
	for (unsigned int i = 0; i < canvas()->mapWidth(); i++) {
		for (unsigned int j = 0; j < canvas()->mapHeight(); j++) {
			if (p && !p->canSee(i, j)) {
				slotUnexplored(i, j);
			} else {
				slotExplored(i, j);
			}
		}
	}
 }

 if (mRenderer) {
	mRenderer->setUpdatesEnabled(true);
 }
}

float BosonGLMiniMap::zoom() const
{
 return 1.0f;
}

BosonGroundTheme* BosonGLMiniMap::groundTheme() const
{
 if (!canvas()) {
	return 0;
 }
 return canvas()->map()->groundTheme();
}

void BosonGLMiniMap::calculateGround(int x, int y)
{
 if (!hasMap()) {
	return;
 }
 BO_CHECK_NULL_RET(mRenderer);
 BO_CHECK_NULL_RET(groundTheme());
 BO_CHECK_NULL_RET(canvas());
 BosonMap* map = canvas()->map();
 BO_CHECK_NULL_RET(map->texMap());
 if (!map->isValidCell(x, y)) {
	return;
 }

 // every cell has four corners - we mix them together to get the actual minimap
 // color.
 unsigned int cornerX[4] = { x, x + 1, x + 1,     x };
 unsigned int cornerY[4] = { y,     y, y + 1, y + 1 };
 int r = 0;
 int g = 0;
 int b = 0;
 for (int j = 0; j < 4; j++) {
	int alphaSum = 0; // sum of all textures
	int cornerRed = 0;
	int cornerGreen = 0;
	int cornerBlue = 0;

	for (unsigned int i = 0; i < groundTheme()->groundTypeCount(); i++) {
		int alpha = (int)map->texMapAlpha(i, cornerX[j], cornerY[j]);
		alphaSum += alpha;

		QRgb rgb = groundTheme()->groundType(i)->color;
		int red = qRed(rgb);
		int green = qGreen(rgb);
		int blue = qBlue(rgb);
		cornerRed += red * alpha / 255;
		cornerGreen += green * alpha / 255;
		cornerBlue += blue * alpha / 255;
	}
	if (alphaSum == 0) {
		// nothing to do for this corner.
		continue;
	}
	cornerRed = cornerRed * 255 / alphaSum;
	cornerGreen = cornerGreen * 255 / alphaSum;
	cornerBlue = cornerBlue * 255 / alphaSum;

	r += cornerRed;
	g += cornerGreen;
	b += cornerBlue;
 }

 r /= 4;
 g /= 4;
 b /= 4;

 mRenderer->setTerrainPoint(x, y, QColor(r, g, b));
 mRenderer->setWaterPoint(x, y, canvas()->cell(x, y)->isWater());
}

void BosonGLMiniMap::setImageTheme(const QString& theme)
{
 d->mImageTheme = theme;
 if (!mRenderer) {
	// this method will be called again once a renderer got created
	return;
 }
 QImage image = imageFromTheme(QString::fromLatin1("minimap-logo.png"), theme);
 if (image.isNull()) {
	boError() << k_funcinfo << "Could not load minimap-logo.png from " << theme << endl;
	if (d->mLogo.isNull()) {
		// create a dummy pixmap to avoid a crash
		int w, h;
		if (canvas()) {
			w = canvas()->mapWidth();
			h = canvas()->mapHeight();
		} else {
			w = 100;
			h = 100;
		}
		d->mLogo = QImage(w, h, 32);
		d->mLogo.fill(Qt::red.rgb());
	}
 } else {
	d->mLogo = image;
 }
 mRenderer->setLogo(d->mLogo);

 image = imageFromTheme(QString::fromLatin1("minimap-zoom-in.png"), theme);
 if (image.isNull()) {
	boError() << k_funcinfo << "Could not load minimap-zoom-in.png from " << theme << endl;
	d->mZoomIn = QImage(16, 16, 32);
 } else {
	d->mZoomIn = image;
 }

 image = imageFromTheme(QString::fromLatin1("minimap-zoom-out.png"), theme);
 if (image.isNull()) {
	boError() << k_funcinfo << "Could not load minimap-zoom-out.png from " << theme << endl;
	d->mZoomOut = QImage(16, 16, 32);
 } else {
	d->mZoomOut = image;
 }

 image = imageFromTheme(QString::fromLatin1("minimap-zoom-default.png"), theme);
 if (image.isNull()) {
	boError() << k_funcinfo << "Could not load minimap-zoom-default.png from " << theme << endl;
	d->mZoomDefault = QImage(16, 16, 32);
 } else {
	d->mZoomDefault = image;
 }
 mRenderer->setZoomImages(d->mZoomIn, d->mZoomOut, d->mZoomDefault);
}

void BosonGLMiniMap::slotZoomIn()
{
 if (boConfig->doubleValue("MiniMapZoom") + ZOOM_STEP > 3.0) {
	return;
 }
 boConfig->setDoubleValue("MiniMapZoom", boConfig->doubleValue("MiniMapZoom") + ZOOM_STEP);
}

void BosonGLMiniMap::slotZoomOut()
{
 if (boConfig->doubleValue("MiniMapZoom") - ZOOM_STEP <= 0.1) {
	return;
 }
 boConfig->setDoubleValue("MiniMapZoom", boConfig->doubleValue("MiniMapZoom") - ZOOM_STEP);
}

void BosonGLMiniMap::slotZoomDefault()
{
 boConfig->setDoubleValue("MiniMapZoom", 1.0);
}

QImage BosonGLMiniMap::imageFromTheme(const QString& file, const QString& theme) const
{
 QString f = locate("data", QString::fromLatin1("boson/themes/ui/%1/%2").arg(theme).arg(file));
 QFileInfo info(f);
 if (!info.exists()) {
	f = locate("data", QString::fromLatin1("boson/themes/ui/%1/%2").arg(QString::fromLatin1("standard").arg(file)));
	info.setFile(f);
	if (!info.exists()) {
		boError() << k_funcinfo << "Can't find " << f << " in " << theme << " or standard" << endl;
		return QImage();
	}
 }
 return QImage(f);
}


void BosonGLMiniMap::createMap(BosonCanvas* c, const BoGLMatrices* gameGLMatrices)
{
 BO_CHECK_NULL_RET(c);
 boDebug() << k_funcinfo << endl;
 mCanvas = c;

#ifdef USE_EXPERIMENTAL_QUADTREE_THINGY
 delete d->mUnitTree;
 d->mUnitTree = BosonMiniMapQuadtreeNode::createTree(c->mapWidth(), c->mapHeight());
#endif

 delete mRenderer;
 mRenderer = new BosonGLMiniMapRenderer(gameGLMatrices);
 mRenderer->createMap(c->mapWidth(), c->mapHeight(), c->map()->groundTheme());
 if (!d->mImageTheme.isEmpty()) {
	setImageTheme(d->mImageTheme);
 }
 boDebug() << k_funcinfo << "initializing ground" << endl;
 for (unsigned int x = 0; x < c->mapWidth(); x++) {
	for (unsigned int y = 0; y < c->mapHeight(); y++) {
		calculateGround(x, y);
	}
 }
 boDebug() << k_funcinfo << "initializing ground done" << endl;
 if (mLocalPlayerIO) {
	initializeItems();
 } else {
	boDebug() << k_funcinfo << "no localplayerIO yet - initiale items later" << endl;
 }
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonGLMiniMap::initializeItems()
{
 BO_CHECK_NULL_RET(mLocalPlayerIO);
 BO_CHECK_NULL_RET(mCanvas);
 boDebug() << k_funcinfo << "initializing items" << endl;
 d->mRadars.clear();
 BoItemList* allItems = mCanvas->allItems();
 for (BoItemList::iterator it = allItems->begin(); it != allItems->end(); ++it) {
	slotItemAdded(*it);
 }
 boDebug() << k_funcinfo << "initializing items done" << endl;
}

void BosonGLMiniMap::renderMiniMap()
{
 if (!hasMap()) {
	return;
 }
 BO_CHECK_NULL_RET(mRenderer);

 // AB: we set all settings in the renderer in every frame. this is a small
 // overhead (a few set*() functions) but saves us from a bigger overhead (and
 // more probable bugs) in a correct settings framework.
 //
 // also it will make maintaining the renderer a lot more easy - changing the
 // renderer code does not necessarily involve modifying the settings framework
 // this way.
 if (d->mShowMiniMap) {
	mRenderer->setType(BosonGLMiniMapRenderer::MiniMap);
 } else {
	mRenderer->setType(BosonGLMiniMapRenderer::Logo);
 }
 mRenderer->setZoom(zoom());
 mRenderer->setAlignment(Qt::Left | Qt::Top);
 mRenderer->setCanvas(mCanvas);
 mRenderer->setLocalPlayerIO(localPlayerIO());

 mRenderer->render(radarList(), unitTree());
}

void BosonGLMiniMap::slotAdvance(unsigned int advanceCallsCount)
{
 BO_CHECK_NULL_RET(mRenderer);
 mRenderer->advance(advanceCallsCount);
}

unsigned int BosonGLMiniMap::miniMapWidth() const
{
 if (!mRenderer) {
	return 0;
 }
 return mRenderer->miniMapWidth();
}

unsigned int BosonGLMiniMap::miniMapHeight() const
{
 if (!mRenderer) {
	return 0;
 }
 return mRenderer->miniMapHeight();
}

void BosonGLMiniMap::slotExplored(int x, int y)
{
 if (!hasMap()) {
	return;
 }
 if (!canvas()) {
	return;
 }
 BO_CHECK_NULL_RET(canvas()->cell(x, y));
 BO_CHECK_NULL_RET(mRenderer);
 mRenderer->setExploredPoint(x, y, true);
}

void BosonGLMiniMap::slotUnexplored(int x, int y)
{
 if (!hasMap()) {
	return;
 }
 if (!canvas()) {
	return;
 }
 BO_CHECK_NULL_RET(canvas()->cell(x, y));
 BO_CHECK_NULL_RET(mRenderer);
 mRenderer->setExploredPoint(x, y, false);
}

bool BosonGLMiniMap::mouseEvent(KGameIO*, QDataStream&, QMouseEvent* e, bool* send)
{
 if (!hasMap()) {
	return false;
 }
 *send = false;
 static int state = Qt::NoButton;
 if (!mRenderer) {
	state = Qt::NoButton;
	return false;
 }
 if (!d->mShowMiniMap) {
	state = Qt::NoButton;
	return false;
 }
 QPoint cell;
 bool inside = mRenderer->windowToCell(e->pos(), &cell);

 if (!inside) {
	state = state & QEvent::MouseButtonMask; // we dont care about keyboard buttons
	if (state != Qt::NoButton) { // the minimap is responsible for this event
		if (e->type() == QEvent::MouseButtonRelease) {
			// we grab these, but dont use them
			state = state & ~e->button();
			return true;
		}
		return true;
	}
	return false;
 }

 // the mouse is in the minimap, but the user is pressing a button down that was
 // not pressed inside the minimap. dont catch it
 if (state != e->state()) {
	return false;
 }
 switch (e->type()) {
	case QEvent::MouseButtonRelease:
		state = e->stateAfter();
		return true;
	case QEvent::MouseButtonPress:
		state = e->state() | e->button();
		break;
	case QEvent::MouseMove:
		if (e->state() == state && state != Qt::NoButton) {
			return true;
		} else {
			return false;
		}
		break;
	case QEvent::MouseButtonDblClick:
		state = e->state() | e->button();
		return true;
		break;
	default:
		return true;
 }
 if (e->type() != QEvent::MouseButtonPress) {
	boWarning() << k_funcinfo << "oops - not a press event?!" << endl;
	state = e->state();
	return true;
 }
 if (e->button() == Qt::LeftButton) {
	emitSignalReCenterView(cell);
	return true;
 }
 if (e->button() == Qt::RightButton) {
	emitSignalMoveSelection(cell);
	return true;
 }
 return false;
}

void BosonGLMiniMap::emitSignalReCenterView(const QPoint& cell)
{
 emit signalReCenterView(cell);
}

void BosonGLMiniMap::emitSignalMoveSelection(const QPoint& cell)
{
 emit signalMoveSelection(cell.x(), cell.y());
}

bool BosonGLMiniMap::hasMap() const
{
 return (mCanvas && mRenderer);
}



class BosonGLMiniMapRendererPrivate
{
public:
	BosonGLMiniMapRendererPrivate()
	{
		mGameGLMatrices = 0;
		mTerrainTexture = 0;
		mWaterTexture = 0;
		mExploredTexture = 0;
		mGLTerrainTexture = 0;
		mGLWaterTexture = 0;
		mGLUnitsTexture = 0;
		mGLExploredTexture = 0;
		mUnitTarget = 0;

		mLogoTexture = 0;
		mRadarRangeTexture = 0;
	}
	BoMatrix mModelviewMatrix;
	QImage mOrigLogo;
	const BoGLMatrices* mGameGLMatrices;

	GLubyte* mTerrainTexture;
	GLubyte* mWaterTexture;
	GLubyte* mExploredTexture;
	int mMapTextureWidth;
	int mMapTextureHeight;
	BoTexture* mGLTerrainTexture;
	BoTexture* mGLWaterTexture;
	BoTexture* mGLUnitsTexture;
	BoTexture* mGLExploredTexture;

	BoRenderTarget* mUnitTarget;

	BoTexture* mLogoTexture;
	BoTexture* mRadarRangeTexture;


	bool mUpdatesEnabled;
	int mMiniMapChangesSinceRendering;
	int mAdvanceCallsSinceLastUpdate;
	QMap<GLubyte*, bool> mTextureUpdatesEnabled;
};

BosonGLMiniMapRenderer::BosonGLMiniMapRenderer(const BoGLMatrices* gameGLMatrices)
{
 d = new BosonGLMiniMapRendererPrivate;

 d->mGameGLMatrices = gameGLMatrices;

 mMapWidth = 0;
 mMapHeight = 0;
 mUseExplored = true;
 mType = Logo;
 mTextureMaxWidth = 1.0f;
 mTextureMaxHeight = 1.0f;
 mMiniMapWidth = 0;
 mMiniMapHeight = 0;
 d->mMapTextureWidth = 0;
 d->mMapTextureHeight = 0;
 d->mUpdatesEnabled = true;
 d->mMiniMapChangesSinceRendering = 0;
 d->mAdvanceCallsSinceLastUpdate = 10000;

 mPosX = distanceFromEdge();
 mPosY = distanceFromEdge();

 // default size of the displayed minimap quad
 setMiniMapSize(150, 150);

 // Load the radar range texture
 QString path = KGlobal::dirs()->findResourceDir("data", "boson/themes/ui/standard/circle-constant.png");
 if (path.isNull()) {
	boError() << k_funcinfo << "Couldn't find radar range texture" << endl;
	return;
 }
 d->mRadarRangeTexture = new BoTexture(path + "boson/themes/ui/standard/circle-constant.png",
		BoTexture::FilterLinearMipmapLinear | BoTexture::FormatRGBA);
}

BosonGLMiniMapRenderer::~BosonGLMiniMapRenderer()
{
 delete d->mUnitTarget;
 delete d->mGLTerrainTexture;
 delete d->mGLWaterTexture;
 delete d->mGLUnitsTexture;
 delete d->mGLExploredTexture;
 delete[] d->mTerrainTexture;
 delete[] d->mWaterTexture;
 delete[] d->mExploredTexture;
 delete d->mLogoTexture;
 delete d->mRadarRangeTexture;
 delete d;
}

void BosonGLMiniMapRenderer::setUpdatesEnabled(bool e)
{
 bool wasEnabled = d->mUpdatesEnabled;
 d->mUpdatesEnabled = e;
 if (!e) {
	return;
 }
 if (!wasEnabled || !d->mTextureUpdatesEnabled[d->mTerrainTexture]) {
	delete d->mGLTerrainTexture;
	d->mGLTerrainTexture = new BoTexture(d->mTerrainTexture,
			d->mMapTextureWidth, d->mMapTextureHeight,
			BoTexture::FilterLinear | BoTexture::FormatRGBA |
			BoTexture::DontCompress | BoTexture::DontGenMipmaps | BoTexture::ClampToEdge);
	d->mTextureUpdatesEnabled[d->mTerrainTexture] = true;
 }
 if (!wasEnabled || !d->mTextureUpdatesEnabled[d->mWaterTexture]) {
	delete d->mGLWaterTexture;
	d->mGLWaterTexture = new BoTexture(d->mWaterTexture,
			d->mMapTextureWidth, d->mMapTextureHeight,
			BoTexture::FilterLinear | BoTexture::FormatRGBA |
			BoTexture::DontCompress | BoTexture::DontGenMipmaps | BoTexture::ClampToEdge);
	d->mTextureUpdatesEnabled[d->mWaterTexture] = true;
 }
 if (!wasEnabled) {
	delete d->mUnitTarget;
	delete d->mGLUnitsTexture;
	d->mGLUnitsTexture = new BoTexture(0,
			d->mMapTextureWidth, d->mMapTextureHeight,
			BoTexture::FilterLinear | BoTexture::FormatRGBA |
			BoTexture::DontCompress | BoTexture::DontGenMipmaps | BoTexture::ClampToEdge);
	d->mUnitTarget = new BoRenderTarget(d->mMapTextureWidth, d->mMapTextureHeight,
			BoRenderTarget::RGBA, d->mGLUnitsTexture);
 }
 if (!wasEnabled || !d->mTextureUpdatesEnabled[d->mExploredTexture]) {
	delete d->mGLExploredTexture;
	d->mGLExploredTexture = new BoTexture(d->mExploredTexture,
			d->mMapTextureWidth, d->mMapTextureHeight,
			BoTexture::FilterLinear | BoTexture::FormatRGBA |
			BoTexture::DontCompress | BoTexture::DontGenMipmaps | BoTexture::ClampToEdge);
	d->mTextureUpdatesEnabled[d->mExploredTexture] = true;
 }
}

void BosonGLMiniMapRenderer::setMiniMapSize(unsigned int width, unsigned int height)
{
 mMiniMapWidth = width;
 mMiniMapHeight = height;
}

void BosonGLMiniMapRenderer::createMap(unsigned int w, unsigned int h, BosonGroundTheme* theme)
{
 BO_CHECK_NULL_RET(theme);

 mMapWidth = w;
 mMapHeight = h;

 unsigned int w2 = BoTexture::nextPower2(mMapWidth);
 unsigned int h2 = BoTexture::nextPower2(mMapHeight);
 mTextureMaxWidth = (float)w / (float)w2;
 mTextureMaxHeight = (float)h / (float)h2;

 delete[] d->mTerrainTexture;
 delete[] d->mWaterTexture;
 delete[] d->mExploredTexture;
 d->mTerrainTexture = new GLubyte[w2 * h2 * 4];
 d->mWaterTexture = new GLubyte[w2 * h2 * 4];
 d->mExploredTexture = new GLubyte[w2 * h2 * 4];
 d->mMapTextureWidth = w2;
 d->mMapTextureHeight = h2;

 QValueList<GLubyte*> textures;
 textures.append(d->mTerrainTexture);
 textures.append(d->mWaterTexture);
 textures.append(d->mExploredTexture);
 for (QValueList<GLubyte*>::iterator it = textures.begin(); it != textures.end(); ++it) {
	GLubyte* texture = (*it);
	for (int y = 0; y < d->mMapTextureHeight; y++) {
		for (int x = 0; x < d->mMapTextureWidth; x++) {
			texture[(y * w + x) * 4 + 0] = 0;
			texture[(y * w + x) * 4 + 1] = 0;
			texture[(y * w + x) * 4 + 2] = 0;
			texture[(y * w + x) * 4 + 3] = 0;
		}
	}
 }

 // AB: d->mMapTexture has been resized to 2^n * 2^m. replace the alpha values for
 // all coordinates that are relevant to us.
 for (QValueList<GLubyte*>::iterator it = textures.begin(); it != textures.end(); ++it) {
	GLubyte* texture = (*it);

	GLubyte alpha = 255;
	if (texture != d->mTerrainTexture && texture != d->mExploredTexture) {
		alpha = 0;
	}
	for (unsigned int x = 0; x < w; x++) {
		for (unsigned int y = 0; y < h; y++) {
			// FIXME: endianness?
			texture[(y * w + x) * 4 + 3] = alpha;
		}
	}
 }

 setUpdatesEnabled(false);
 setUpdatesEnabled(true);
}

void BosonGLMiniMapRenderer::advance(unsigned int advanceCallsCount)
{
 d->mAdvanceCallsSinceLastUpdate++;
}

void BosonGLMiniMapRenderer::render(QPtrList<Unit>* radars, BosonMiniMapQuadtreeNode* unitTree)
{
 glColor3ub(255, 255, 255);
 d->mMiniMapChangesSinceRendering = 0;
 renderGimmicks();
 switch (mType) {
	case MiniMap:
		renderMiniMap(radars, unitTree);
		break;
	case Logo:
		renderLogo();
		break;
	default:
		boError() << k_funcinfo << "invalid type " << (int)mType << endl;
 }
}

void BosonGLMiniMapRenderer::renderGimmicks()
{
}

void BosonGLMiniMapRenderer::renderLogo()
{
 BO_CHECK_NULL_RET(d->mLogoTexture);
 glPushAttrib(GL_ENABLE_BIT);
 glEnable(GL_TEXTURE_2D);
 d->mLogoTexture->bind();

 // AB: note that renderQuad() is for renderMiniMap() only, not for this one.
 glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2i(0, 0);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2i(miniMapWidth(), 0);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2i(miniMapWidth(), miniMapHeight());

	glTexCoord2f(0.0f, 1.0f);
	glVertex2i(0, miniMapHeight());
 glEnd();

 glPopAttrib();
 boTextureManager->invalidateCache();
}

void BosonGLMiniMapRenderer::renderMiniMap(QPtrList<Unit>* radars, BosonMiniMapQuadtreeNode* unitTree)
{
 BO_CHECK_NULL_RET(d->mTerrainTexture);
 BO_CHECK_NULL_RET(d->mWaterTexture);
 BO_CHECK_NULL_RET(d->mExploredTexture);
 BO_CHECK_NULL_RET(d->mGLTerrainTexture);
 BO_CHECK_NULL_RET(d->mGLWaterTexture);
 BO_CHECK_NULL_RET(d->mGLExploredTexture);
 glPushMatrix();

 // AB: this is only for pre-ufo use
// glLoadIdentity();

 if (d->mAdvanceCallsSinceLastUpdate >= 40) {
	d->mUnitTarget->enable();
	updateRadarTexture(radars, unitTree);
	d->mUnitTarget->disable();
 }

 glPushAttrib(GL_ENABLE_BIT);
 glEnable(GL_TEXTURE_2D);
 d->mModelviewMatrix.loadIdentity();
 d->mModelviewMatrix.translate((float)mPosX, mPosY, 0.0f);

 d->mModelviewMatrix.scale(mZoom, mZoom, 1.0f); // AB: maybe do this on the texture matrix stack
// glScalef(mZoom, mZoom, 1.0f); // AB: maybe do this on the texture matrix stack

 setUpdatesEnabled(true);

 glDisable(GL_BLEND);
 d->mGLTerrainTexture->bind();
 renderQuad();

 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 d->mGLWaterTexture->bind();
 renderQuad();
 if (mUseExplored) {
	d->mGLExploredTexture->bind();
	renderQuad();
 }
 // Darken the terrain to make radar blips more visible
 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glDisable(GL_TEXTURE_2D);
 glColor4f(0.0, 0.0, 0.0, 0.6);
 renderQuad();
 glColor4f(1.0, 1.0, 1.0, 1.0);
 glEnable(GL_TEXTURE_2D);

 d->mGLUnitsTexture->bind();
 glBlendFunc(GL_SRC_ALPHA, GL_ONE);
 renderQuad();
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

 renderRadarRangeIndicators(radars);

 glDisable(GL_BLEND);

 renderCamera();

 glPopAttrib();
 glPopMatrix();
}

void BosonGLMiniMapRenderer::updateRadarTexture(QPtrList<Unit>* radarlist, BosonMiniMapQuadtreeNode* unitTree)
{
 d->mAdvanceCallsSinceLastUpdate = 0;
 // Init rendering
 glPushAttrib(GL_ALL_ATTRIB_BITS);
 glViewport(0, 0, d->mMapTextureWidth, d->mMapTextureHeight);
 glDisable(GL_SCISSOR_TEST);
 glClear(GL_COLOR_BUFFER_BIT);
 if (radarlist->isEmpty()) {
	glPopAttrib();
	return;
 }

 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 glLoadIdentity();
 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadIdentity();
 gluOrtho2D(0.0f, miniMapWidth(), 0.0f, miniMapHeight());
 glDisable(GL_DEPTH_TEST);

 // Create a vector of radars and their ranges
 QValueVector<const RadarPlugin*> radars;
 QValueVector<float> ranges;
 QValueVector<BoVector2Float> centers;

 float minx = 1000000.0f, maxx = -1000000.0f, miny = 1000000.0f, maxy = -1000000.0f;
 for (QPtrListIterator<Unit> it(*radarlist); it.current(); ++it) {
	const RadarPlugin* prop = (RadarPlugin*)it.current()->plugin(UnitPlugin::Radar);
	// Add radar and it's center to the vectors
	radars.append(prop);
	centers.append(it.current()->center().toFloat());
	// Maximum range of the radar
	// See below for the radar equation, here we calculate maximum distance of an
	//  object with size = 0.5 so that it's still detected by the radar
	float maxrange = powf((prop->transmittedPower() * 3.0f) / prop->minReceivedPower(), 0.25f);
	ranges.append(maxrange * maxrange);
	// Update bbox of the radar-affected area
	minx = QMIN(minx, (float)it.current()->x() - maxrange);
	maxx = QMAX(maxx, (float)it.current()->x() + maxrange);
	miny = QMIN(miny, (float)it.current()->y() - maxrange);
	maxy = QMAX(maxy, (float)it.current()->y() + maxrange);
 }
 BoRect2Fixed area((int)QMAX(0.0f, minx),  (int)QMAX(0.0f, miny),
		(int)QMIN(mMapWidth, maxx + 1),  (int)QMIN(mMapHeight, maxy + 1));

 // Get a list of all items in the affected area
#ifdef USE_EXPERIMENTAL_QUADTREE_THINGY
 BoItemList* items;
 {
 BosonProfiler p("unitTree->approximateUnitsInRect(area);");
 items = unitTree->approximateUnitsInRect(area);
 }
#else
 BoItemList* items = mCanvas->collisions()->collisionsAtCells(area, 0, false);
#endif


 // For every item, see if it's visible by the radar and render a dot if it is.
 // To determine whether the item is detected by the radar (and how well it's
 //  detected), we use simplified radar equation:
 //      R = (T * S) / (D^4)
 //  where R - received power
 //        T - transmitter power
 //        S - size of the object (how well it reflects the signal)
 //        D - distance between radar and the object
 // See http://en.wikipedia.org/wiki/Radar for more info about radars and
 //  radar equation.

 // Use additive blending
 glEnable(GL_BLEND);
 glDisable(GL_TEXTURE_2D);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE);
 glEnable(GL_POINT_SMOOTH);
 glPointSize(3.0f);
 float scale = miniMapWidth() / (float)QMAX(d->mMapTextureWidth, d->mMapTextureHeight);
 BoVector4Float basecolor(-0.05, -0.05, -0.02, 0.8);
 BoVector4Float addcolor(0.03, 0.1, 0.01, 0.0);

 glBegin(GL_POINTS);
 BoItemList::ConstIterator it;
 for (it = items->begin(); it != items->end(); ++it) {
	if (!RTTI::isUnit((*it)->rtti())) {
		continue;
	}
	Unit* u = (Unit*)*it;
	if (u->isDestroyed()) {
		continue;
	} else if (u->owner() == mLocalPlayerIO->player()) {
		// Player's own units will be rendered separately
		continue;
	}
	BoVector2Float itempos = u->center().toFloat();
	if (mLocalPlayerIO->canSee(u) && mLocalPlayerIO->isEnemy(u)) {
		// Visible enemies will be shown as red dots
		// TODO: render all visible enemies, not just those in radar range
		glColor4f(0.7, 0.0, 0.0, 1.0);
		glVertex3f(itempos.x() * scale, itempos.y() * scale, 0.0f);
		continue;
	}
	bool flying = u->isFlying();
	float strongestsignal = 0.0f;
	// Go through all the radars and pick the one with the strongest signal
	for (unsigned int i = 0; i < radars.count(); i++) {
		if ((flying && !radars[i]->detectsAirUnits()) || (!flying && !radars[i]->detectsLandUnits())) {
			// This radar can't detect this unit
			continue;
		}
		float distsqr = (itempos - centers[i]).dotProduct();
		if (distsqr > ranges[i]) {
			// Most likely not visible to this radar
			continue;
		} else {
			float receivedpower = (radars[i]->transmittedPower() * (float)u->width()) / (distsqr * distsqr);
			// We additonally divide by minReceivedPower() to get "signal strength"
			strongestsignal = QMAX(strongestsignal, receivedpower / radars[i]->minReceivedPower());
		}
	}
	if (strongestsignal >= 1.0f) {
		// Signal was picked up by at least one radar.
		// Render the dot
		glColor4fv((basecolor + addcolor * strongestsignal).data());
		glVertex3f(itempos.x() * scale, itempos.y() * scale, 0.0f);
	}
 }
 QPtrListIterator<Unit> playerUnitsIt(*mLocalPlayerIO->allMyUnits());
 glColor4f(0.2, 0.2, 1.0, 1.0);
 while (playerUnitsIt.current()) {
	if (playerUnitsIt.current()->isDestroyed()) {
		continue;
	}
	BoVector2Float itempos = playerUnitsIt.current()->center().toFloat();
	glVertex3f(itempos.x() * scale, itempos.y() * scale, 0.0f);
	++playerUnitsIt;
 }
 glEnd();

 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();
 glPopAttrib();
}

void BosonGLMiniMapRenderer::renderRadarRangeIndicators(QPtrList<Unit>* radarlist)
{
 BO_CHECK_NULL_RET(d->mRadarRangeTexture);

 if (d->mAdvanceCallsSinceLastUpdate >= 20) {
	return;
 }

 d->mRadarRangeTexture->bind();
 glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_SCISSOR_BIT);
 glEnable(GL_BLEND);
 glScissor(0, d->mGameGLMatrices->viewport()[3] - miniMapHeight(), miniMapWidth(), miniMapHeight());
 glEnable(GL_SCISSOR_TEST);
 float scale = miniMapWidth() / (float)QMAX(mMapWidth, mMapHeight);
 glPushMatrix();
 glTranslatef(0.0f, ((float)miniMapHeight()), 0.0f);
 glScalef(scale, scale, 0.0f);

 float progress = d->mAdvanceCallsSinceLastUpdate / 20.0f;
 for (QPtrListIterator<Unit> it(*radarlist); it.current(); ++it) {
	BoVector2Float pos = it.current()->center().toFloat();
	const RadarPlugin* prop = (RadarPlugin*)it.current()->plugin(UnitPlugin::Radar);
	float range = powf(prop->transmittedPower() / prop->minReceivedPower(), 0.25f) * progress;
	glColor4f(0.0, 0.8, 0.0, 0.5 - 0.5 * progress);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(pos.x() - range, -(pos.y() + range), 0);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(pos.x() + range, -(pos.y() + range), 0);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(pos.x() + range, -(pos.y() - range), 0);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(pos.x() - range, -(pos.y() - range), 0);
	glEnd();
 }

 glPopMatrix();
 glPopAttrib();
}

void BosonGLMiniMapRenderer::renderQuad()
{
// glMultMatrixf(d->mModelviewMatrix.data());
 // AB: I'd like to use glTexCoord2i(), but at least ATIs implementation makes a
 // glTexCoord2f(1,1) out of glTexCoord2i(1, 1) which is not what I expect here.
 // so instead we use alpha blending to avoid having large parts of the rendered
 // quad just black.
 glBegin(GL_QUADS);
	// AB: note that the y-coordinate needs to be flipped
	// (mapHeight() -> 0.0 ; 0 -> 1.0)
	glTexCoord2f(0.0f, mTextureMaxHeight);
	glVertex3i(0, 0, 0);

	glTexCoord2f(mTextureMaxWidth, mTextureMaxHeight);
	glVertex3i(miniMapWidth(), 0, 0);

	glTexCoord2f(mTextureMaxWidth, 0.0f);
	glVertex3i(miniMapWidth(), miniMapHeight(), 0);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3i(0, miniMapHeight(), 0);
 glEnd();
}

void BosonGLMiniMapRenderer::renderCamera()
{
 // extract points from the viewfrustum
 // we have planes RIGHT, LEFT, BOTTOM, TOP, FAR, NEAR, we are going to name
 // lines and points accordingly (point at LEFT/BOTTOM/NEAR planes is LBN)
 const BoFrustum& viewFrustum = d->mGameGLMatrices->viewFrustum();
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
 BoVector3Float BLF;
 BoVector3Float BRF;
 BoVector3Float BRN;
 BoVector3Float BLN;
 BoVector3Float TLF;
 BoVector3Float TRF;
 BoVector3Float TRN;
 BoVector3Float TLN;
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

 glDisable(GL_TEXTURE_2D);

 // map world-coordinates to minimap-coordinates
 BLF.setX((BLF.x() * miniMapWidth()) / mMapWidth);
 BLF.setY((BLF.y() * miniMapHeight()) / mMapHeight + (float)miniMapHeight());
 BRF.setX((BRF.x() * miniMapWidth()) / mMapWidth);
 BRF.setY((BRF.y() * miniMapHeight()) / mMapHeight + (float)miniMapHeight());
 BRN.setX((BRN.x() * miniMapWidth()) / mMapWidth);
 BRN.setY((BRN.y() * miniMapHeight()) / mMapHeight + (float)miniMapHeight());
 BLN.setX((BLN.x() * miniMapWidth()) / mMapWidth);
 BLN.setY((BLN.y() * miniMapHeight()) / mMapHeight + (float)miniMapHeight());
 TLF.setX((TLF.x() * miniMapWidth()) / mMapWidth);
 TLF.setY((TLF.y() * miniMapHeight()) / mMapHeight + (float)miniMapHeight());
 TRF.setX((TRF.x() * miniMapWidth()) / mMapWidth);
 TRF.setY((TRF.y() * miniMapHeight()) / mMapHeight + (float)miniMapHeight());
 TRN.setX((TRN.x() * miniMapWidth()) / mMapWidth);
 TRN.setY((TRN.y() * miniMapHeight()) / mMapHeight + (float)miniMapHeight());
 TLN.setX((TLN.x() * miniMapWidth()) / mMapWidth);
 TLN.setY((TLN.y() * miniMapHeight()) / mMapHeight + (float)miniMapHeight());

 // Render a semitransparent yellow quad to better illustrate the visible area
 glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_SCISSOR_BIT);
 glEnable(GL_BLEND);
 glShadeModel(GL_SMOOTH);
 glScissor(0, d->mGameGLMatrices->viewport()[3] - miniMapHeight(), miniMapWidth(), miniMapHeight());
 glEnable(GL_SCISSOR_TEST);
 glBegin(GL_QUADS);
	// Far side of the quad is more transparent...
	glColor4f(1.0, 1.0, 0.0, 0.15);
	glVertex3fv(TLF.data());
	glVertex3fv(TRF.data());
	// ... and the front one is less
	glColor4f(1.0, 1.0, 0.0, 0.3);
	glVertex3fv(BRF.data());
	glVertex3fv(BLF.data());
 glEnd();
 glPopAttrib();

 // now the points should be final - we can draw our lines onto the minimap
 glColor3ub(192, 192, 192);
 glBegin(GL_LINES);
	drawLine(BLF, BRF, miniMapWidth(), miniMapHeight());
	drawLine(BRF, BRN, miniMapWidth(), miniMapHeight());
	drawLine(BRN, BLN, miniMapWidth(), miniMapHeight());
	drawLine(BLN, BLF, miniMapWidth(), miniMapHeight());
	drawLine(TLF, TRF, miniMapWidth(), miniMapHeight());
	drawLine(TRF, TRN, miniMapWidth(), miniMapHeight());
	drawLine(TRN, TLN, miniMapWidth(), miniMapHeight());
	drawLine(TLN, TLF, miniMapWidth(), miniMapHeight());
	drawLine(BLF, TLF, miniMapWidth(), miniMapHeight());
	drawLine(BRF, TRF, miniMapWidth(), miniMapHeight());
	drawLine(BRN, TRN, miniMapWidth(), miniMapHeight());
	drawLine(BLN, TLN, miniMapWidth(), miniMapHeight());
 glEnd();
 glColor3ub(255, 255, 255);
}

void BosonGLMiniMapRenderer::setTerrainPoint(int x, int y, const QColor& color)
{
 setColor(x, y, color, 255, d->mTerrainTexture, d->mGLTerrainTexture);
}

void BosonGLMiniMapRenderer::setWaterPoint(int x, int y, bool isWater)
{
 // AB: maybe mix with the terrain texture? (i.e. use alpha < 255)
 if (isWater) {
	int alpha = 255;
	setColor(x, y, QColor(0, 64, 192), alpha, d->mWaterTexture, d->mGLWaterTexture);
 } else {
	unsetPoint(x, y, d->mWaterTexture, d->mGLWaterTexture);
 }
}

void BosonGLMiniMapRenderer::setExploredPoint(int x, int y, bool explored)
{
 if (!mUseExplored) {
	return;
 }
 if (x < 0 || y < 0 || (unsigned int)x >= mapWidth() || (unsigned int)y >= mapHeight()) {
	boError() << k_funcinfo << "invalid cell " << x << "," << y << endl;
	return;
 }
 if (!explored) {
	setPoint(x, y, COLOR_UNKNOWN, d->mExploredTexture, d->mGLExploredTexture);
 } else {
	unsetPoint(x, y, d->mExploredTexture, d->mGLExploredTexture);
 }
}


void BosonGLMiniMapRenderer::unsetPoint(int x, int y, GLubyte* textureData, BoTexture* texture)
{
 if (textureData == d->mTerrainTexture) {
	boWarning() << k_funcinfo << "a point on the terrain texture should never be unset. terrain is never transparent." << endl;
	return;
 }
 setColor(x, y, QColor(0, 0, 0), 0, textureData, texture);
}

void BosonGLMiniMapRenderer::setPoint(int x, int y, const QColor& color, GLubyte* textureData, BoTexture* texture)
{
 setColor(x, y, color, 255, textureData, texture);
}

void BosonGLMiniMapRenderer::setColor(int x, int y, const QColor& color, int alpha, GLubyte* textureData, BoTexture* texture)
{
 BO_CHECK_NULL_RET(textureData);
 if (d->mMapTextureWidth <= 0 || d->mMapTextureHeight <= 0) {
	boError() << k_funcinfo << "invalid map texture size" << endl;
 }
 // FIXME: endianness ?
 textureData[(y * d->mMapTextureWidth + x) * 4 + 0] = color.red();
 textureData[(y * d->mMapTextureWidth + x) * 4 + 1] = color.green();
 textureData[(y * d->mMapTextureWidth + x) * 4 + 2] = color.blue();
 textureData[(y * d->mMapTextureWidth + x) * 4 + 3] = alpha;
 if (texture && d->mUpdatesEnabled && d->mTextureUpdatesEnabled[textureData]) {
	texture->bind();
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
			&textureData[(y * d->mMapTextureWidth + x) * 4]);
	d->mMiniMapChangesSinceRendering++;
	if (d->mMiniMapChangesSinceRendering > 20) {
		// we've done many changes to the minimap without ever rendering
		// a single change. probably there are lots of changes going on
		// and we expect even more.
		// disable updates until minimap is being rendered again
		d->mTextureUpdatesEnabled[textureData] = false;
	}
 }
}

void BosonGLMiniMapRenderer::setAlignment(int f)
{
 BO_CHECK_NULL_RET(d->mGameGLMatrices);
 if (f & Qt::AlignLeft) {
	mPosX = distanceFromEdge();
 } else {
	mPosX = d->mGameGLMatrices->viewport()[2] - miniMapWidth() - distanceFromEdge();
 }
 if (f & Qt::AlignBottom) {
	mPosY = distanceFromEdge();
 } else {
	mPosY = d->mGameGLMatrices->viewport()[3] - miniMapHeight() - distanceFromEdge();
 }
}

bool BosonGLMiniMapRenderer::windowToCell(const QPoint& pos, QPoint* cell) const
{
 if (!d->mGameGLMatrices) {
	BO_NULL_ERROR(d->mGameGLMatrices);
	return false;
 }
 int realy = d->mGameGLMatrices->viewport()[3] - pos.y();
 if (pos.x() < (int)mPosX) {
	return false;
 }
 if (realy < (int)mPosY) {
	return false;
 }
 if (pos.x() >= (int)(mPosX + miniMapWidth())) {
	return false;
 }
 if (realy >= (int)(mPosY + miniMapHeight())) {
	return false;
 }
 if (mType != MiniMap) {
	return false;
 }

 BoMatrix invModelviewMatrix;
 d->mModelviewMatrix.invert(&invModelviewMatrix);
 BoVector3Float v(pos.x(), realy, 0.0f);
 BoVector3Float v2;
 invModelviewMatrix.transform(&v2, &v);
 v2.setX(v2.x() * ((float)mapWidth() / (float)miniMapWidth()));
 v2.setY(v2.y() * ((float)mapHeight() / (float)miniMapHeight()));

 cell->setX((int)v2.x());
 cell->setY(mapHeight() - (int)(v2.y()));

 return true;
}

void BosonGLMiniMapRenderer::setLogo(const QImage& image)
{
 boDebug() << k_funcinfo << endl;
 if (miniMapWidth() * miniMapHeight() == 0) {
	boError() << k_funcinfo << "invalid minimap size" << endl;
	return;
 }
 if (image.isNull()) {
	boError() << k_funcinfo << "invalid logo image" << endl;
	return;
 }
 QImage image_ = image;
 image_.setAlphaBuffer(true);
 d->mOrigLogo = image_;

 QImage gl = QGLWidget::convertToGLFormat(d->mOrigLogo);

 delete d->mLogoTexture;
 d->mLogoTexture = new BoTexture(gl.bits(),
		gl.width(), gl.height(),
		BoTexture::FilterLinear | BoTexture::FormatRGBA |
		BoTexture::DontCompress | BoTexture::DontGenMipmaps);
}

void BosonGLMiniMapRenderer::setZoomImages(const QImage& in_, const QImage& out_, const QImage& defaultZoom_)
{
 int w = in_.width();
 w = QMAX(w, out_.width());
 w = QMAX(w, defaultZoom_.width());
 int h = in_.height();
 h = QMAX(h, out_.height());
 h = QMAX(h, defaultZoom_.height());
 QImage in, out, defaultZoom;
 in = in_.smoothScale(w, h, QImage::ScaleMin);
 out = out_.smoothScale(w, h, QImage::ScaleMin);
 defaultZoom = defaultZoom_.smoothScale(w, h, QImage::ScaleMin);
#if 0
 // AB: we could use plib/pui here...
 // maybe we should link to plib? after all we already use fnt in boson and
 // could make use of at least pui.
 d->mZoomInButton->setImage(in);
 d->mZoomOutButton->setImage(out);
 d->mZoomDefaultButton->setImage(defaultZoom);
#endif
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
