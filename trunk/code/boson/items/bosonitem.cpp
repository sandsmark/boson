/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bosonitem.h"

#include "../bosoncanvas.h"
#include "../rtti.h"
#include "../selectbox.h"
#include "../cell.h" // for deleteitem. i dont want this. how can we avoid this? don't use qptrvector probably.
#include "../bosoneffect.h"
#include "../bosonpropertyxml.h"
#include "../bosonconfig.h"
#include "../bo3dtools.h"
#include "bosonitempropertyhandler.h"
#include "bosonitemrenderer.h"
#include "bodebug.h"

#include <qptrlist.h>
#include <qptrvector.h>
#include <qdom.h>

#include <kstaticdeleter.h>

#include <math.h>

QMap<int, QString>* BosonItemProperties::mPropertyMap = 0;
static KStaticDeleter< QMap<int, QString> > sd;

BosonItemProperties::BosonItemProperties()
{
 if (!mPropertyMap) {
	initStatic();
 }
 mProperties = new BosonItemPropertyHandler(this);
 mProperties->setPolicy(KGamePropertyBase::PolicyLocal); // fallback
}

BosonItemProperties::~BosonItemProperties()
{
 dataHandler()->clear();
 delete mProperties;
}

void BosonItemProperties::initStatic()
{
 if (mPropertyMap) {
	return;
 }
 delete mPropertyMap;
 mPropertyMap = new QMap<int, QString>;
 sd.setObject(mPropertyMap);
}

KGamePropertyHandler* BosonItemProperties::dataHandler() const
{
 return (KGamePropertyHandler*)mProperties;
}

void BosonItemProperties::registerData(KGamePropertyBase* prop, int id, bool local)
{
 if (!prop) {
	boError() << k_funcinfo << "NULL property" << endl;
	return;
 }
 if (id < KGamePropertyBase::IdUser) {
	boWarning() << k_funcinfo << "ID < KGamePropertyBase::IdUser" << endl;
	// do not return - might still work
 }
 QString name = propertyName(id);
 if (name.isNull()) {
	boWarning() << k_funcinfo << "Invalid property name for " << id << endl;
	// a name isn't strictly necessary, so don't return
 }
 prop->registerData(id, dataHandler(),
		local ? KGamePropertyBase::PolicyLocal : KGamePropertyBase::PolicyClean,
		name);
}

void BosonItemProperties::addPropertyId(int id, const QString& name)
{
 if (mPropertyMap->contains(id)) {
	boError() << k_funcinfo << "Cannot add " << id << " twice!" << endl;
	boError() << k_funcinfo << "Existing name: " << *mPropertyMap->find(id) << " ; provided name: " << name << endl;
	return;
 }
/* if (mPropertyMap->values().contains(name)) {
	boError() << k_funcinfo << "Cannot add " << name << " twice!" << endl;
	return;
 }*/
 mPropertyMap->insert(id, name);
}

QString BosonItemProperties::propertyName(int id)
{
 if (!mPropertyMap->contains(id)) {
	return QString::null;
 }
 return (*mPropertyMap)[id];
}


BosonItem::BosonItem(Player* owner, BosonCanvas* canvas)
	: BosonItemProperties()
{
 mOwner = owner;
 mCanvas = canvas;

 mId = 0;
 mX = mY = mZ = 0;
 mWidth = mHeight = 0;
 mDepth = 0;
 mCellsDirty = true;
 mRotation = 0;
 mXRotation = 0;
 mYRotation = 0;
 mIsVisible = true;
 mEffects = 0;

 mXVelocity = 0;
 mYVelocity = 0;
 mZVelocity = 0;

 mCurrentSpeed = 0;
 mMaxSpeed = 0;
 mAccelerationSpeed = 0;
 mDecelerationSpeed = 0;

 mIsAnimated = true; // obsolete! remove!
 mSelectBox = 0;

 mCells = new QPtrVector<Cell>();
 mRenderer = 0;

 if (!boConfig->disableModelLoading()) {
	mRenderer = new BosonItemModelRenderer(this);
 } else {
	mRenderer = new BosonItemRenderer(this);
 }
}

BosonItem::~BosonItem()
{
 clearEffects();
 unselect();
 if (canvas()) {
	canvas()->removeFromCells(this);
	if (canvas()->allItems()->contains(this)) {
		// must happen BEFORE the item is deleted!
		boError() << k_funcinfo << "the item has not yet been removed from the canvas!!" << endl;
	}
 }
 delete mCells;
 delete mEffects;
 delete mRenderer;
}



QPtrVector<Cell>* BosonItem::cells()
{
 if (mCellsDirty) {
	makeCells(canvas()->cells(), mCells, boundingRect(), canvas()->mapWidth(), canvas()->mapHeight());
	mCellsDirty = false;
 }
 return mCells;
}

QPtrVector<Cell>* BosonItem::cellsConst() const
{
 return mCells;
}

void BosonItem::makeCells(Cell* allCells, QPtrVector<Cell>* cells, const BoRectFixed& rect, int mapWidth, int mapHeight)
{
 BO_CHECK_NULL_RET(allCells);
 int left = QMAX((int)rect.left(), 0);
 int top = QMAX((int)rect.top(), 0);
 int right = QMIN(ceil(rect.right()), mapWidth);
 int bottom = QMIN(ceil(rect.bottom()), mapHeight);

 // AB: WARNING: we do direct array/pointer calculations here, so
 // right/bottom/left/top MUST be valid for the allCells array!
 // it is WAY MORE IMPORTANT to ensure valid values here than making it fast!!


 int size = (right - left) * (bottom - top);
 cells->resize(size);
 if (size == 0) {
	return;
 }

 int n = 0;
 for (int i = left; i < right; i++) {
	for (int j = top; j < bottom; j++) {
		// note: we calculate the cell in the array on our own here,
		// because a) it's a bit faster than map->cell() and b) (more
		// important) we don't have to include bosonmap.h
		Cell* c = allCells + i + j * mapWidth;
		cells->insert(n, c);
		n++;
	}
 }
}

bool BosonItem::bosonCollidesWith(BosonItem* item) const
{
 BoVector3Fixed itempos(item->x() + xVelocity(), item->y() + yVelocity(), item->z() + zVelocity());
 return bosonCollidesWith(itempos, itempos + BoVector3Fixed(item->width(), item->height(), item->depth()));
}

bool BosonItem::bosonCollidesWith(const BoVector3Fixed& v1, const BoVector3Fixed& v2) const
{
// boDebug() << "  ++> " << k_funcinfo << "Item coords: (" << x() << "; " << y() << "; " << z() <<
//		"); size: (" << width() << "; " << height() << "; " << depth() << ")" << endl;

 // Check z-coord first
 bofixed v1_z = v1.z();
 bofixed v2_z = v2.z();
 if (QMAX(z() + zVelocity(), v1_z) >= QMIN(z() + zVelocity() + depth(), v2_z)) {
	// z-coordinates don't intersect
	return false;
 }

 // Half the width and height of the other box
 bofixed halfw = (v2.x() - v1.x()) / 2;
 bofixed halfh = (v2.y() - v1.y()) / 2;

 bofixed centerx = v1.x() + halfw;
 bofixed centery = v1.y() + halfh;

 // BB 1
 bofixed minx1, miny1, maxx1, maxy1;
 minx1 = x() + xVelocity() - halfw;
 miny1 = y() + xVelocity() - halfh + 1;
 maxx1 = x() + yVelocity() + width() + halfw;
 maxy1 = y() + yVelocity() + height() + halfw - 1;
 if ((centerx > minx1) && (centerx < maxx1) && (centery > miny1) && (centery < maxy1)) {
	// Box's center is in BB 1
//	boDebug() << "        " << k_funcinfo << "Items COLLIDE (1)!!!" << endl;
	return true;
 }

 // BB 2
 bofixed minx2, miny2, maxx2, maxy2;
 minx2 = x() + xVelocity() - halfw + 1;
 miny2 = y() + xVelocity() - halfh;
 maxx2 = x() + yVelocity() + width() + halfw - 1;
 maxy2 = y() + yVelocity() + height() + halfw;
 if ((centerx > minx2) && (centerx < maxx2) && (centery > miny2) && (centery < maxy2)) {
	// Box's center is in BB 2
//	boDebug() << "        " << k_funcinfo << "Items COLLIDE (2)!!!" << endl;
	return true;
 }

 // Check manhattan dist between centers
 bofixed mycenterx = x() + xVelocity() + width() / 2;
 bofixed mycentery = y() + yVelocity() + height() / 2;
 if (QABS(mycenterx - centerx) + QABS(mycentery - centery) < (width() / 2 + halfw + height() / 2 + halfh - 1)) {
	// Box's center still collides with us
//	boDebug() << "        " << k_funcinfo << "Items COLLIDE! (3)!!" << endl;
	return true;
 }

// boDebug() << "        " << k_funcinfo << "Items don't collide" << endl;
 return false;
}

void BosonItem::select(bool markAsLeader)
{
 if (mSelectBox) {
	// already selected
	return;
 }
 mSelectBox = new SelectBox(this, canvas(), markAsLeader);
}

void BosonItem::unselect()
{
 delete mSelectBox;
 mSelectBox = 0;
}

BoVector2Fixed BosonItem::center() const
{
  return BoVector2Fixed(centerX(), centerY());
}

BoRectFixed BosonItem::boundingRect() const
{
 return BoRectFixed(leftEdge(), topEdge(), leftEdge() + width(), topEdge() + height());
}

BoRectFixed BosonItem::boundingRectAdvanced() const
{
 bofixed left = leftEdge() + xVelocity();
 bofixed top = topEdge() + yVelocity();
 return BoRectFixed(left, top, left + width(), top + height());
}

void BosonItem::addToCells()
{
 // We don't do anything for shots (at the moment)
 if (RTTI::isShot(rtti())) {
	return;
 }
 canvas()->addToCells(this);
}

void BosonItem::removeFromCells()
{
 // We don't do anything for shots (at the moment)
 if (RTTI::isShot(rtti())) {
	return;
 }
 canvas()->removeFromCells(this);
}

void BosonItem::setSize(bofixed width, bofixed height, bofixed depth)
{
 removeFromCells();
 mWidth = width;
 mHeight = height;
 mDepth = depth;
 mCellsDirty = true;
 addToCells();
}

void BosonItem::renderItem(unsigned int lod)
{
 if (itemRenderer()) {
	itemRenderer()->renderItem(lod);
 }
}

BosonCollisions* BosonItem::collisions() const
{
 return canvas()->collisions();
}

void BosonItem::setEffectsPosition(bofixed x, bofixed y, bofixed z)
{
 if (effects() && effects()->count() > 0) {
	BoVector3Fixed pos(x + width() / 2, y + height() / 2, z);
	pos.canvasToWorld();
	QPtrListIterator<BosonEffect> it(*effects());
	for (; it.current(); ++it) {
		it.current()->setPosition(pos);
	}
 }
}

void BosonItem::setEffectsRotation(bofixed xrot, bofixed yrot, bofixed zrot)
{
 BoVector3Fixed rot(xrot, yrot, zrot);
 if (effects() && effects()->count() > 0) {
	QPtrListIterator<BosonEffect> it(*effects());
	for (; it.current(); ++it) {
		it.current()->setRotation(rot);
	}
 }
}

const QPtrList<BosonEffect>* BosonItem::effects() const
{
 return mEffects;
}

void BosonItem::setEffects(const QPtrList<BosonEffect>& effects, bool addtocanvas)
{
 clearEffects();
 delete mEffects;
 mEffects = new QPtrList<BosonEffect>(effects);
 // Make effects owned by us
 QPtrListIterator<BosonEffect> it(*mEffects);
 while (it.current()) {
	it.current()->setOwnerId(id());
	++it;
 }
 // Add effects to canvas if necessary
 if (addtocanvas) {
	canvas()->addEffects(*mEffects);
 }
}

void BosonItem::addEffect(BosonEffect* e, bool addtocanvas)
{
 if (!mEffects) {
	mEffects = new QPtrList<BosonEffect>;
 }
 e->setOwnerId(id());
 mEffects->append(e);
 // Add effect to canvas if necessary
 if (addtocanvas) {
	canvas()->addEffect(e);
 }
}

void BosonItem::clearEffects()
{
 if (!mEffects) {
	return;
 }
 // Set ownerid of all effects to 0
 QPtrListIterator<BosonEffect> it(*mEffects);
 while (it.current()) {
	it.current()->setOwnerId(0);
	++it;
 }
 mEffects->clear();
}

bool BosonItem::saveAsXML(QDomElement& root)
{
 if (root.isNull()) {
	boError() << k_funcinfo << "NULL root node" << endl;
	return false;
 }
 root.setAttribute(QString::fromLatin1("Rtti"), (int)rtti());

 // dummy attributes, if they are used in derived classes, they should get
 // replaced
 root.setAttribute(QString::fromLatin1("Type"), (int)0);
 root.setAttribute(QString::fromLatin1("Group"), (int)0);
 root.setAttribute(QString::fromLatin1("GroupType"), (int)0);


 root.setAttribute(QString::fromLatin1("x"), x());
 root.setAttribute(QString::fromLatin1("y"), y());
 root.setAttribute(QString::fromLatin1("z"), z());
 root.setAttribute(QString::fromLatin1("Rtti"), (int)rtti());
 root.setAttribute(QString::fromLatin1("Id"), (unsigned int)id());

 // the data handler
 BosonCustomPropertyXML propertyXML;
 QDomDocument doc = root.ownerDocument();
 QDomElement handler = doc.createElement(QString::fromLatin1("DataHandler"));
 if (!propertyXML.saveAsXML(handler, dataHandler())) {
	boError() << k_funcinfo << "Unable to save datahandler of item" << endl;
	return false;
 }
 root.setAttribute(QString::fromLatin1("DataHandlerId"), dataHandler()->id());
 root.appendChild(handler);

 return true;
}

bool BosonItem::loadFromXML(const QDomElement& root)
{
 if (root.isNull()) {
	boError() << k_funcinfo << "NULL root node" << endl;
	return false;
 }
 // load the data handler
 BosonCustomPropertyXML propertyXML;
 QDomElement handler = root.namedItem(QString::fromLatin1("DataHandler")).toElement();
 if (handler.isNull()) {
	boError(260) << k_funcinfo << "No DataHandler tag found for Item" << endl;
	return false;
 }
 if (!propertyXML.loadFromXML(handler, dataHandler())) {
	boError(260) << k_funcinfo << "unable to load item data handler" << endl;
	return false;
 }


 return true;
}

void BosonItem::animate(unsigned int)
{
 if (itemRenderer()) {
	itemRenderer()->animate();
 }
}

unsigned int BosonItem::preferredLod(float dist) const
{
 if (itemRenderer()) {
	return itemRenderer()->preferredLod(dist);
 }
 return 0;
}

float BosonItem::glDepthMultiplier() const
{
 if (itemRenderer()) {
	return itemRenderer()->glDepthMultiplier();;
 }
 return 1.0f;
}

void BosonItem::setRendererToEditorMode()
{
 if (!itemRenderer()) {
	return;
 }
 // editor won't display the construction but always completed
 // facilities. otherwise it's hard to recognize where they
 // were actually placed
 itemRenderer()->setShowGLConstructionSteps(false);
 itemRenderer()->setAnimationMode(UnitAnimationIdle);
}

bool BosonItem::initItemRenderer()
{
 if (itemRenderer()) {
	return itemRenderer()->setModel(getModelForItem());
 }
 return true;
}

bool BosonItem::itemInFrustum(const float* frustum) const
{
 if (itemRenderer()) {
	return itemRenderer()->itemInFrustum(frustum);
 }
 return false;
}

