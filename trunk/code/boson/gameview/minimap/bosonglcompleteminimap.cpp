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

#include "bosonglcompleteminimap.h"
#include "bosonglcompleteminimap.moc"

#include "../../bomemory/bodummymemory.h"
#include "../gameengine/cell.h"
#include "../gameengine/bosonmap.h"
#include "../gameengine/boitemlist.h"
#include "../bo3dtools.h"
#include "../gameengine/unit.h"
#include "../gameengine/player.h"
#include "../gameengine/playerio.h"
#include "bodebug.h"
#include "../botexture.h"
#include "../gameengine/rtti.h"
#include "../gameengine/unitplugins.h"
#include "../gameengine/bosoncanvas.h"
#include "../borendertarget.h"
#include "../bosonprofiling.h"
#include <bogl.h>

#include <klocale.h>
#include <kstandarddirs.h>

#include <qvaluelist.h>
#include <qvaluevector.h>
#include <qptrlist.h>

#define COLOR_UNKNOWN Qt::black // unexplored terrain



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




class BosonGLCompleteMiniMapPrivate
{
public:
	BosonGLCompleteMiniMapPrivate()
	{
		mTerrainTexture = 0;
		mWaterTexture = 0;
		mExploredTexture = 0;
		mGLTerrainTexture = 0;
		mGLWaterTexture = 0;
		mGLExploredTexture = 0;

		mUnitTarget = 0;
		mGLUnitsTexture = 0;

		mRadarRangeTexture = 0;

		mCanvas = 0;
		mLocalPlayerIO = 0;

		mUnitTree = 0;
	}

	bool mMapCreated;
	int mMapTextureWidth;
	int mMapTextureHeight;
	unsigned int mMapWidth;
	unsigned int mMapHeight;
	float mTextureMaxWidth;
	float mTextureMaxHeight;

	GLubyte* mTerrainTexture;
	GLubyte* mWaterTexture;
	GLubyte* mExploredTexture;
	BoTexture* mGLTerrainTexture;
	BoTexture* mGLWaterTexture;
	BoTexture* mGLExploredTexture;

	BoRenderTarget* mUnitTarget;
	BoTexture* mGLUnitsTexture;

	BoTexture* mRadarRangeTexture;

	bool mUpdatesEnabled;
	int mMiniMapChangesSinceRendering;
	int mAdvanceCallsSinceLastUpdate;
	QMap<GLubyte*, bool> mTextureUpdatesEnabled;

	BosonCanvas* mCanvas;
	PlayerIO* mLocalPlayerIO;

	QPtrList<const Unit> mRadars;
	BosonMiniMapQuadtreeNode* mUnitTree;
};

BosonGLCompleteMiniMap::BosonGLCompleteMiniMap(QObject* parent)
	: QObject(parent)
{
 d = new BosonGLCompleteMiniMapPrivate;

 d->mMapCreated = false;
 d->mMapWidth = 0;
 d->mMapHeight = 0;
 d->mTextureMaxWidth = 1.0f;
 d->mTextureMaxHeight = 1.0f;
 d->mMapTextureWidth = 0;
 d->mMapTextureHeight = 0;
 d->mUpdatesEnabled = true;
 d->mMiniMapChangesSinceRendering = 0;
 d->mAdvanceCallsSinceLastUpdate = 10000;

 // Load the radar range texture
 QString path = KGlobal::dirs()->findResourceDir("data", "boson/themes/ui/standard/circle-constant.png");
 if (path.isNull()) {
	boError() << k_funcinfo << "Couldn't find radar range texture" << endl;
	d->mRadarRangeTexture = 0;
 } else {
	d->mRadarRangeTexture = new BoTexture(path + "boson/themes/ui/standard/circle-constant.png",
			BoTexture::FilterLinearMipmapLinear | BoTexture::FormatRGBA);
 }
}

BosonGLCompleteMiniMap::~BosonGLCompleteMiniMap()
{
 delete d->mUnitTarget;
 delete d->mGLTerrainTexture;
 delete d->mGLWaterTexture;
 delete d->mGLUnitsTexture;
 delete d->mGLExploredTexture;
 delete[] d->mTerrainTexture;
 delete[] d->mWaterTexture;
 delete[] d->mExploredTexture;
 delete d->mRadarRangeTexture;
 delete d;
}

void BosonGLCompleteMiniMap::setCanvas(BosonCanvas* c)
{
 if (d->mCanvas) {
	boError() << k_funcinfo << "canvas already set, cannot set new canvas. delete this object and create a new one!" << endl;
	return;
 }
 d->mCanvas = c;
}

BosonCanvas* BosonGLCompleteMiniMap::canvas() const
{
 return d->mCanvas;
}

void BosonGLCompleteMiniMap::setLocalPlayerIO(PlayerIO* io)
{
 d->mLocalPlayerIO = io;
 if (d->mLocalPlayerIO && d->mMapCreated) {
	initializeItems();
 }
}

PlayerIO* BosonGLCompleteMiniMap::localPlayerIO() const
{
 return d->mLocalPlayerIO;
}

void BosonGLCompleteMiniMap::setUpdatesEnabled(bool e)
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

unsigned int BosonGLCompleteMiniMap::mapWidth() const
{
 return d->mMapWidth;
}

unsigned int BosonGLCompleteMiniMap::mapHeight() const
{
 return d->mMapHeight;
}

void BosonGLCompleteMiniMap::createMap(unsigned int w, unsigned int h)
{
 BO_CHECK_NULL_RET(canvas());
 if (d->mMapCreated) {
	boError() << k_funcinfo << "map already created. delete this object and create a new one!" << endl;
	return;
 }
 boDebug() << k_funcinfo << endl;

 d->mMapCreated = true;
 d->mMapWidth = w;
 d->mMapHeight = h;

 unsigned int w2 = BoTexture::nextPower2(d->mMapWidth);
 unsigned int h2 = BoTexture::nextPower2(d->mMapHeight);
 d->mTextureMaxWidth = (float)w / (float)w2;
 d->mTextureMaxHeight = (float)h / (float)h2;

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

#ifdef USE_EXPERIMENTAL_QUADTREE_THINGY
 delete d->mUnitTree;
 d->mUnitTree = BosonMiniMapQuadtreeNode::createTree(canvas()->mapWidth(), canvas()->mapHeight());
#endif

 boDebug() << k_funcinfo << "initializing ground" << endl;
 for (unsigned int x = 0; x < canvas()->mapWidth(); x++) {
	for (unsigned int y = 0; y < canvas()->mapHeight(); y++) {
		calculateGround(x, y);
	}
 }
 boDebug() << k_funcinfo << "initializing ground done" << endl;

 if (localPlayerIO()) {
	initializeItems();
 } else {
	boDebug() << k_funcinfo << "no localPlayerIO yet - initialize items later" << endl;
 }

 setUpdatesEnabled(true);
 boDebug() << k_funcinfo << "done" << endl;
}

void BosonGLCompleteMiniMap::slotAdvance(unsigned int advanceCallsCount)
{
 Q_UNUSED(advanceCallsCount);
 d->mAdvanceCallsSinceLastUpdate++;
}

void BosonGLCompleteMiniMap::render()
{
 glColor3ub(255, 255, 255);
 d->mMiniMapChangesSinceRendering = 0;
 renderMiniMap();
}

void BosonGLCompleteMiniMap::renderMiniMap()
{
 BO_CHECK_NULL_RET(d->mTerrainTexture);
 BO_CHECK_NULL_RET(d->mWaterTexture);
 BO_CHECK_NULL_RET(d->mExploredTexture);
 BO_CHECK_NULL_RET(d->mGLTerrainTexture);
 BO_CHECK_NULL_RET(d->mGLWaterTexture);
 BO_CHECK_NULL_RET(d->mGLExploredTexture);

 glPushMatrix();

 if (d->mAdvanceCallsSinceLastUpdate >= 40) {
	d->mUnitTarget->enable();
	updateRadarTexture(&d->mRadars, d->mUnitTree);
	d->mUnitTarget->disable();
 }

 glPushAttrib(GL_ENABLE_BIT);

 glEnable(GL_TEXTURE_2D);

 setUpdatesEnabled(true);

 glDisable(GL_BLEND);
 d->mGLTerrainTexture->bind();
 renderQuad();

 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 d->mGLWaterTexture->bind();
 renderQuad();

 d->mGLExploredTexture->bind();
 renderQuad();

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

 renderRadarRangeIndicators(radarList());

 glDisable(GL_BLEND);

 glPopAttrib();
 glPopMatrix();
}

void BosonGLCompleteMiniMap::updateRadarTexture(const QPtrList<const Unit>* radarlist, BosonMiniMapQuadtreeNode* unitTree)
{
 BO_CHECK_NULL_RET(localPlayerIO());

 d->mAdvanceCallsSinceLastUpdate = 0;
 // Init rendering
 glPushAttrib(GL_ALL_ATTRIB_BITS);
 glViewport(0, 0, d->mMapWidth, d->mMapHeight);
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

 gluOrtho2D(0.0f, d->mMapWidth, 0.0f, d->mMapHeight);
 glDisable(GL_DEPTH_TEST);

 // Create a vector of radars and their ranges
 QValueVector<const RadarPlugin*> radars;
 QValueVector<float> ranges;
 QValueVector<BoVector2Float> centers;

 float minx = 1000000.0f, maxx = -1000000.0f, miny = 1000000.0f, maxy = -1000000.0f;
 for (QPtrListIterator<const Unit> it(*radarlist); it.current(); ++it) {
	const Unit* unit = it.current();
	const RadarPlugin* prop = (const RadarPlugin*)unit->plugin(UnitPlugin::Radar);
	if (!prop) {
		continue;
	}

	radars.append(prop);
	centers.append(unit->center().toFloat());

	// Maximum range of the radar
	// See below for the radar equation, here we calculate maximum distance of an
	//  object with size = 0.5 so that it's still detected by the radar
	float maxrange = powf((prop->transmittedPower() * 3.0f) / prop->minReceivedPower(), 0.25f);
	ranges.append(maxrange * maxrange);

	// Update bbox of the radar-affected area
	minx = QMIN(minx, (float)unit->x() - maxrange);
	maxx = QMAX(maxx, (float)unit->x() + maxrange);
	miny = QMIN(miny, (float)unit->y() - maxrange);
	maxy = QMAX(maxy, (float)unit->y() + maxrange);
 }
 BoRect2Fixed area((int)QMAX(0.0f, minx),  (int)QMAX(0.0f, miny),
		(int)QMIN(d->mMapWidth, maxx + 1),  (int)QMIN(d->mMapHeight, maxy + 1));

 // Get a list of all items in the affected area
#ifdef USE_EXPERIMENTAL_QUADTREE_THINGY
 BoItemList* items;
 {
 BosonProfiler p("unitTree->approximateUnitsInRect(area);");
 items = unitTree->approximateUnitsInRect(area);
 }
#else
 BoItemList* items = d->mCanvas->collisions()->collisionsAtCells(area, 0, false);
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

 // Size of the radar point (blip) in pixels after the minimap has been
 //  rendered. This is dependant on the minimap size and map size
 // AB: I think we don't need this with the current design anymore
 //     -> previously we displayed the whole minimap in a 150x150 quad, now we
 //        display only a part of the minimap there and thus the blips are
 //        larger anyway
 float visiblepointsize = 2.0f;
#if 0
 const float radarBlipScaleFactor = QMAX(d->mMapWidth, d->mMapHeight) / (float)miniMapScreenWidth()
#else
 const float radarBlipScaleFactor = 1.0f;
#endif
 glPointSize(visiblepointsize * radarBlipScaleFactor);

 BoVector4Float basecolor(0.2, 0.4, 0.15, 0.8);
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
	} else if (u->owner() == localPlayerIO()->player()) {
		// Player's own units will be rendered separately
		continue;
	}
	BoVector2Float itempos = u->center().toFloat();
	if (localPlayerIO()->canSee(u) && localPlayerIO()->isEnemy(u)) {
		// Visible enemies will be shown as red dots
		// TODO: render all visible enemies, not just those in radar range
		glColor4f(0.7, 0.0, 0.0, 1.0);
		glVertex3f(itempos.x(), itempos.y(), 0.0f);
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
		glVertex3f(itempos.x(), itempos.y(), 0.0f);
	}
 }
 QPtrListIterator<Unit> playerUnitsIt(*localPlayerIO()->allMyUnits());
 glColor4f(0.2, 0.2, 1.0, 1.0);
 while (playerUnitsIt.current()) {
	if (playerUnitsIt.current()->isDestroyed()) {
		continue;
	}
	BoVector2Float itempos = playerUnitsIt.current()->center().toFloat();
	glVertex3f(itempos.x(), itempos.y(), 0.0f);
	++playerUnitsIt;
 }
 glEnd();

 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();
 glPopAttrib();
}

void BosonGLCompleteMiniMap::renderRadarRangeIndicators(const QPtrList<const Unit>* radarlist)
{
 BO_CHECK_NULL_RET(d->mRadarRangeTexture);

 if (d->mAdvanceCallsSinceLastUpdate >= 20) {
	return;
 }

 d->mRadarRangeTexture->bind();
 glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);
 glEnable(GL_BLEND);
 glPushMatrix();

 // make sure that 0.0 is top and -1.0 is bottom
 // (-> we will use coordinates just like in the actual map)
 glTranslatef(0.0f, 1.0f, 0.0f);

 glScalef(1.0f / d->mMapWidth, 1.0f / d->mMapHeight, 1.0f);

 float progress = d->mAdvanceCallsSinceLastUpdate / 20.0f;
 for (QPtrListIterator<const Unit> it(*radarlist); it.current(); ++it) {
	const Unit* unit = it.current();
	const RadarPlugin* prop = (const RadarPlugin*)unit->plugin(UnitPlugin::Radar);
	if (!prop) {
		continue;
	}
	BoVector2Float pos = prop->unit()->center().toFloat();
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

void BosonGLCompleteMiniMap::renderQuad()
{
 // AB: we use a unit size quad. we can use glScalef() to achieve any desired
 // actual size.
 const float miniMapWidth = 1.0f;
 const float miniMapHeight = 1.0f;

// glMultMatrixf(d->mModelviewMatrix.data());
 // AB: I'd like to use glTexCoord2i(), but at least ATIs implementation makes a
 // glTexCoord2f(1,1) out of glTexCoord2i(1, 1) which is not what I expect here.
 // so instead we use alpha blending to avoid having large parts of the rendered
 // quad just black.
 glBegin(GL_QUADS);
	// AB: note that the y-coordinate needs to be flipped
	// (mapHeight() -> 0.0 ; 0 -> 1.0)
	glTexCoord2f(0.0f, d->mTextureMaxHeight);
	glVertex3f(0.0f, 0.0f, 0.0f);

	glTexCoord2f(d->mTextureMaxWidth, d->mTextureMaxHeight);
	glVertex3f(miniMapWidth, 0.0f, 0.0f);

	glTexCoord2f(d->mTextureMaxWidth, 0.0f);
	glVertex3f(miniMapWidth, miniMapHeight, 0.0f);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(0.0f, miniMapHeight, 0.0f);
 glEnd();
}

void BosonGLCompleteMiniMap::setTerrainPoint(int x, int y, const QColor& color)
{
 setColor(x, y, color, 255, d->mTerrainTexture, d->mGLTerrainTexture);
}

void BosonGLCompleteMiniMap::setWaterPoint(int x, int y, bool isWater)
{
 // AB: maybe mix with the terrain texture? (i.e. use alpha < 255)
 if (isWater) {
	int alpha = 255;
	setColor(x, y, QColor(0, 64, 192), alpha, d->mWaterTexture, d->mGLWaterTexture);
 } else {
	unsetPoint(x, y, d->mWaterTexture, d->mGLWaterTexture);
 }
}

void BosonGLCompleteMiniMap::setExploredPoint(int x, int y, bool explored)
{
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


void BosonGLCompleteMiniMap::unsetPoint(int x, int y, GLubyte* textureData, BoTexture* texture)
{
 if (textureData == d->mTerrainTexture) {
	boWarning() << k_funcinfo << "a point on the terrain texture should never be unset. terrain is never transparent." << endl;
	return;
 }
 setColor(x, y, QColor(0, 0, 0), 0, textureData, texture);
}

void BosonGLCompleteMiniMap::setPoint(int x, int y, const QColor& color, GLubyte* textureData, BoTexture* texture)
{
 setColor(x, y, color, 255, textureData, texture);
}

void BosonGLCompleteMiniMap::setColor(int x, int y, const QColor& color, int alpha, GLubyte* textureData, BoTexture* texture)
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

void BosonGLCompleteMiniMap::slotUnitMoved(Unit* unit, bofixed oldX, bofixed oldY)
{
 BO_CHECK_NULL_RET(unit);
#ifdef USE_EXPERIMENTAL_QUADTREE_THINGY
 BosonProfiler p("d->mUnitTree->unitMoved()");
 if (d->mUnitTree) {
	d->mUnitTree->unitMoved(unit, oldX, oldY);
 }
#endif
}

void BosonGLCompleteMiniMap::slotUnitRemoved(Unit* unit)
{
 BO_CHECK_NULL_RET(unit);
#ifdef USE_EXPERIMENTAL_QUADTREE_THINGY
 if (d->mUnitTree) {
	d->mUnitTree->removeUnit(unit);
 }
#endif
 d->mRadars.remove(unit);
}

void BosonGLCompleteMiniMap::slotItemAdded(BosonItem* item)
{
 BO_CHECK_NULL_RET(localPlayerIO());
 BO_CHECK_NULL_RET(item);
 if (!RTTI::isUnit(item->rtti())) {
	return;
 }
 Unit* u = (Unit*)item;
#ifdef USE_EXPERIMENTAL_QUADTREE_THINGY
 BO_CHECK_NULL_RET(d->mUnitTree);
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
 d->mRadars.append(u);
}

void BosonGLCompleteMiniMap::slotFacilityConstructed(Unit* fac)
{
 // Facilities have the construction phase so they have to be rechecked
 //  (whether they are radars) once they are constructed.
 slotItemAdded(fac);
}

const QPtrList<const Unit>* BosonGLCompleteMiniMap::radarList() const
{
 return &d->mRadars;
}

BosonMiniMapQuadtreeNode* BosonGLCompleteMiniMap::unitTree() const
{
 return d->mUnitTree;
}

void BosonGLCompleteMiniMap::slotUpdateTerrainAtCorner(int x, int y)
{
 calculateGround(x, y);
}

void BosonGLCompleteMiniMap::slotExplored(int x, int y)
{
 if (!canvas()) {
	return;
 }
 BO_CHECK_NULL_RET(canvas()->cell(x, y));
 setExploredPoint(x, y, true);
}

void BosonGLCompleteMiniMap::slotUnexplored(int x, int y)
{
 if (!canvas()) {
	return;
 }
 BO_CHECK_NULL_RET(canvas()->cell(x, y));
 setExploredPoint(x, y, false);
}

void BosonGLCompleteMiniMap::calculateGround(int x, int y)
{
 BO_CHECK_NULL_RET(canvas());
 BO_CHECK_NULL_RET(canvas()->map());
 BosonMap* map = canvas()->map();
 int r = 0;
 int g = 0;
 int b = 0;
 bool coveredByWater = false;

 if (!map->calculateMiniMapGround(x, y, &r, &g, &b, &coveredByWater)) {
	return;
 }

 setTerrainPoint(x, y, QColor(r, g, b));
 setWaterPoint(x, y, coveredByWater);
}

void BosonGLCompleteMiniMap::initializeItems()
{
 BO_CHECK_NULL_RET(canvas());
 BO_CHECK_NULL_RET(localPlayerIO());
 if (!d->mMapCreated) {
	return;
 }
 boDebug() << k_funcinfo << "initializing items" << endl;
 d->mRadars.clear();
 BoItemList* allItems = canvas()->allItems();
 for (BoItemList::iterator it = allItems->begin(); it != allItems->end(); ++it) {
	slotItemAdded(*it);
 }
 boDebug() << k_funcinfo << "initializing items done" << endl;
}

void BosonGLCompleteMiniMap::initFogOfWar(PlayerIO* p)
{
 BO_CHECK_NULL_RET(canvas());
 BO_CHECK_NULL_RET(localPlayerIO());
 if (!d->mMapCreated) {
	boError() << k_funcinfo << "map has not yet been created" << endl;
	return;
 }
 boDebug() << k_funcinfo << endl;

 setUpdatesEnabled(false);

 // AB: add a global PlayerIO to the editor. only use non-NULL IOs here then.
 if (!p) {
	// a NULL playerIO means that we should display the complete map. fog of
	// war gets disabled.
	for (unsigned int x = 0; x < canvas()->mapWidth(); x++) {
		for (unsigned int y = 0; y < canvas()->mapHeight(); y++) {
			slotExplored(x, y);
		}
	}
 } else {
	for (unsigned int x = 0; x < canvas()->mapWidth(); x++) {
		for (unsigned int y = 0; y < canvas()->mapHeight(); y++) {
			if (p && !p->canSee(x, y)) {
				slotUnexplored(x, y);
			} else {
				slotExplored(x, y);
			}
		}
	}
 }

 setUpdatesEnabled(true);
}
