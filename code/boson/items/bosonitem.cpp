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
#include "../bo3dtools.h"
#include "bosonitempropertyhandler.h"
#include "bosonitemrenderer.h"
#include "bodebug.h"

#include <qrect.h>
#include <qptrlist.h>
#include <qptrvector.h>
#include <qdom.h>

#include <kstaticdeleter.h>

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
 mX = mY = mZ = 0.0f;
 mWidth = mHeight = 0;
 mDepth = 0.0;
 mCellsDirty = true;
 mRotation = 0.0f;
 mXRotation = 0.0f;
 mYRotation = 0.0f;
 mIsVisible = true;
 mEffects = 0;

 mXVelocity = 0.0f;
 mYVelocity = 0.0f;
 mZVelocity = 0.0f;

 mCurrentSpeed = 0;
 mMaxSpeed = 0;
 mAccelerationSpeed = 0;
 mDecelerationSpeed = 0;

 mIsAnimated = true; // obsolete! remove!
 mSelectBox = 0;

 mCells = new QPtrVector<Cell>();
 mRenderer = new BosonItemRenderer(this);
}

BosonItem::~BosonItem()
{
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
}



QPtrVector<Cell>* BosonItem::cells()
{
 if (mCellsDirty) {
	int left, right, top, bottom;
	leftTopCell(&left, &top);
	rightBottomCell(&right, &bottom);
	makeCells(canvas()->cells(), mCells, left, right, top, bottom, canvas()->mapWidth(), canvas()->mapHeight());
	mCellsDirty = false;
 }
 return mCells;
}

QPtrVector<Cell>* BosonItem::cellsConst() const
{
 return mCells;
}

void BosonItem::makeCells(Cell* allCells, QPtrVector<Cell>* cells, int left, int right, int top, int bottom, int mapWidth, int mapHeight)
{
 BO_CHECK_NULL_RET(allCells);
 left = QMAX(left, 0);
 top = QMAX(top, 0);
 right = QMAX(left, right);
 bottom = QMAX(top, bottom);

 right = QMIN(right, QMAX(mapWidth - 1, 0));
 bottom = QMIN(bottom, QMAX(mapHeight - 1, 0));
 left = QMIN(left, right);
 top = QMIN(top, bottom);

 // AB: WARNING: we do direct array/pointer calculations here, so
 // right/bottom/left/top MUST be valid for the allCells array!
 // it is WAY MORE IMPORTANT to ensure valid values here than making it fast!!


 int size = (right - left + 1) * (bottom - top + 1);
 cells->resize(size);
 if (size == 0) {
	return;
 }

 int n = 0;
 for (int i = left; i <= right; i++) {
	for (int j = top; j <= bottom; j++) {
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
 BoVector3 itempos(item->x() + xVelocity(), item->y() + yVelocity(), item->z() + zVelocity());
 return bosonCollidesWith(itempos, itempos + BoVector3(item->width(), item->height(), item->depth()));
}

bool BosonItem::bosonCollidesWith(const BoVector3& v1, const BoVector3& v2) const
{
// boDebug() << "  ++> " << k_funcinfo << "Item coords: (" << x() << "; " << y() << "; " << z() <<
//		"); size: (" << width() << "; " << height() << "; " << depth() << ")" << endl;

 // Check z-coord first
 if (QMAX(z() + zVelocity(), v1.z()) >= QMIN(z() + zVelocity() + depth(), v2.z())) {
	// z-coordinates don't intersect
	return false;
 }

 // Half the width and height of the other box
 float halfw = (v2.x() - v1.x()) / 2;
 float halfh = (v2.y() - v1.y()) / 2;

 float centerx = v1.x() + halfw;
 float centery = v1.y() + halfh;

 // BB 1
 float minx1, miny1, maxx1, maxy1;
 minx1 = x() + xVelocity() - halfw;
 miny1 = y() + xVelocity() - halfh + BO_TILE_SIZE;
 maxx1 = x() + yVelocity() + width() + halfw;
 maxy1 = y() + yVelocity() + height() + halfw - BO_TILE_SIZE;
 if ((centerx > minx1) && (centerx < maxx1) && (centery > miny1) && (centery < maxy1)) {
	// Box's center is in BB 1
//	boDebug() << "        " << k_funcinfo << "Items COLLIDE (1)!!!" << endl;
	return true;
 }

 // BB 2
 float minx2, miny2, maxx2, maxy2;
 minx2 = x() + xVelocity() - halfw + BO_TILE_SIZE;
 miny2 = y() + xVelocity() - halfh;
 maxx2 = x() + yVelocity() + width() + halfw - BO_TILE_SIZE;
 maxy2 = y() + yVelocity() + height() + halfw;
 if ((centerx > minx2) && (centerx < maxx2) && (centery > miny2) && (centery < maxy2)) {
	// Box's center is in BB 2
//	boDebug() << "        " << k_funcinfo << "Items COLLIDE (2)!!!" << endl;
	return true;
 }

 // Check manhattan dist between centers
 float mycenterx = x() + xVelocity() + width() / 2;
 float mycentery = y() + yVelocity() + height() / 2;
 if (QABS(mycenterx - centerx) + QABS(mycentery - centery) < (width() / 2 + halfw + height() / 2 + halfh - BO_TILE_SIZE)) {
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

QRect BosonItem::boundingRect() const
{
 return QRect(QPoint((int)leftEdge(), (int)topEdge()),
		QPoint((int)leftEdge() + width() - 1, (int)topEdge() + height() - 1));
}

QRect BosonItem::boundingRectAdvanced() const
{
 int left = (int)(leftEdge() + xVelocity());
 int top = (int)(topEdge() + yVelocity());
 return QRect(QPoint(left, top),
		QPoint(left + width() - 1, top + height() - 1));
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

void BosonItem::setSize(int width, int height, float depth)
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
 mRenderer->renderItem(lod);
}

BosonCollisions* BosonItem::collisions() const
{
 return canvas()->collisions();
}

void BosonItem::setEffectsPosition(float x, float y, float z)
{
 if (effects() && effects()->count() > 0) {
	BoVector3 pos(x + width() / 2, y + height() / 2, z);
	pos.canvasToWorld();
	QPtrListIterator<BosonEffect> it(*effects());
	for (; it.current(); ++it) {
		it.current()->setPosition(pos);
	}
 }
}

void BosonItem::setEffectsRotation(float xrot, float yrot, float zrot)
{
 BoVector3 rot(xrot, yrot, zrot);
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
 // FIXME: do we need to do anything with old effects?
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
 mEffects->append(e);
 // Add effect to canvas if necessary
 if (addtocanvas) {
	canvas()->addEffect(e);
 }
}

void BosonItem::clearEffects()
{
 if (mEffects) {
	mEffects->clear();
 }
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

void BosonItem::advance(unsigned int)
{
 mRenderer->animate();
}


