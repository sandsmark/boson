/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "../bosonmodel.h"
#include "../cell.h" // for deleteitem. i dont want this. how can we avoid this? don't use qptrvector probably.
#include "../bosonparticlesystem.h"
#include "../bosonpropertyxml.h"
#include "bosonitempropertyhandler.h"
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


BosonItem::BosonItem(Player* owner, BosonModel* model, BosonCanvas* canvas)
	: BosonItemProperties()
{
 mOwner = owner;
 mCanvas = canvas;
 mModel = model;

 mCurrentAnimation = 0;
 mX = mY = mZ = 0.0f;
 mWidth = mHeight = 0;
 mDepth = 0.0;
 mCellsDirty = true;
 mRotation = 0.0f;
 mXRotation = 0.0f;
 mYRotation = 0.0f;
 mGLDepthMultiplier = 1.0f;
 mFrame = 0;
 mGLConstructionStep = 0;
 mAnimationCounter = 0;
 mCurrentFrame = 0;
 mIsVisible = true;

 mXVelocity = 0.0f;
 mYVelocity = 0.0f;
 mZVelocity = 0.0f;

 mCurrentSpeed = 0;
 mMaxSpeed = 0;
 mAccelerationSpeed = 0;
 mDecelerationSpeed = 0;

 mCurrentAnimation = 0;
 // 1.732 == sqrt(3) i.e. lenght of vector whose all components are 1
 mBoundingSphereRadius = 1.732f; // TODO: can we extract this from the model? this probably needs to change with different frames!

 mIsAnimated = false;
 mSelectBox = 0;

 mCells = new QPtrVector<Cell>();

 if (mCanvas) {
	mCanvas->addItem(this);
 } else {
	boWarning() << k_funcinfo << "NULL canvas" << endl;
 }

 if (!mModel) {
	boError() << k_funcinfo << "NULL model - we will crash!" << endl;
	return;
 }

 // set the default animation mode
 setAnimationMode(UnitAnimationIdle);

 // FIXME the correct frame must be set after this constructor!
 if (mGLConstructionStep >= glConstructionSteps()) {
	setCurrentFrame(mModel->frame(frame()));
 } else {
	setCurrentFrame(mModel->constructionStep(mGLConstructionStep));
 }

}

BosonItem::~BosonItem()
{
 unselect();
 if (canvas()) {
	canvas()->removeFromCells(this);
	canvas()->removeAnimation(this);
	canvas()->removeItem(this);
 }
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
 // First we test z-coordinate
 if (QMAX(z(), item->z()) > QMIN(z() + depth(), item->z() + item->depth())) {
	// z-coordinates doesn't intersect. Then items doesn't collide either
	return false;
 }

 // Then bounding rect intersect test
 // Taken from QRect::intersects() but I didn't use this method for speed reasons
 if ((QMAX(x(), item->x()) > QMIN(x() + width() - 1, item->x() + item->width() - 1)) ||
		(QMAX(y(), item->y()) > QMIN(y() + height() - 1, item->y() + item->height() - 1))) {
	// Bounding rects does not intersect
	return false;
 }


 if (!(RTTI::isUnit(rtti()) && RTTI::isUnit(item->rtti()))) {
	// At least one item is not unit
	// Non-unit items collide if their bounding boxes collide
	return true;
 }

 // Both items are units
 // Units are special because they must be able to move diagonally. When moving
 //  diagonally, it's ok if their bounding boxes intersect on xy plane. Then we
 //  check if this item's center isn't inside other item's bounding rect
 // FIXME: maybe we need same for items. ATM, units can't diagonally cross tiles
 //  with mines
 return item->boundingRectAdvanced().contains(QPoint((int)(x() + xVelocity() + width() / 2), (int)(y() + yVelocity() + height() / 2)), true);
}

void BosonItem::setAnimated(bool a)
{
 if (mIsAnimated != a) {
	mIsAnimated = a;
	if (a) {
		canvas()->addAnimation(this);
	} else {
		canvas()->removeAnimation(this);
	}
 }
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

void BosonItem::setGLDepthMultiplier(float d)
{
 mGLDepthMultiplier = d;
}

void BosonItem::setGLConstructionStep(unsigned int s)
{
 // note: in case of s >= model()->constructionSteps() we use the last
 // constructionStep that is defined in the model until an actual frame is set.
 BoFrame* f = model()->constructionStep(s);
 if (!f) {
	boWarning() << k_funcinfo << "NULL construction step " << s << endl;
	return;
 }
 mGLConstructionStep = s;
 setCurrentFrame(f);
}

unsigned int BosonItem::glConstructionSteps() const
{
 return model()->constructionSteps();
}

void BosonItem::setFrame(int _frame)
{
 if (mGLConstructionStep < glConstructionSteps()) {
	// this unit (?) has not yet been constructed
	// completely.
	// Note that mGLConstructionStep is totally different
	// from Unit::constructionStep() !
	_frame = frame();
 }

 // FIXME: this if is pretty much nonsense, since e.g. frame()
 // might be 0 and _frame, too - but the frame still changed,
 // since we had a construction list before!
 // we mustn't change the frame when moving and so on. these are
 // old QCanvas compatible functions. need to be fixed.
 if (_frame != frame()) {
		BoFrame* f = model()->frame(_frame);
		if (f) {
			setCurrentFrame(f);
			mFrame = _frame;
		} else {
			boWarning() << k_funcinfo << "invalid frame " << _frame << endl;
		}
	}
}

unsigned int BosonItem::frameCount() const
{
 return model() ? model()->frames() : 0;
}

void BosonItem::setCurrentFrame(BoFrame* frame)
{
 if (!frame) {
	boError() << k_funcinfo << "NULL frame" << endl;
	return;
 }
 mCurrentFrame = frame;

 // the following values cache values from BoFrame, so that we can use them in
 // inline functions (or with direct access). otherwise we'd have to #include
 // bosonmodel.h in bosonitem.h (-> bad)

 setGLDepthMultiplier(frame->depthMultiplier());
}

void BosonItem::setAnimationMode(int mode)
{
 if (mGLConstructionStep < glConstructionSteps()) {
	return;
 }
 BosonAnimation* anim = model()->animation(mode);
 if (!anim) {
	if (mCurrentAnimation) {
		return;
	}
	anim = model()->animation(0);
	if (!anim) {
		boError() << k_funcinfo << "NULL default animation mode!" << endl;
		return;
	}
 }
 mCurrentAnimation = anim;
 setFrame(mCurrentAnimation->start());
}

void BosonItem::animate()
{
 if (!mCurrentAnimation || !mCurrentAnimation->speed()) {
	return;
 }
 mAnimationCounter++;
 if (mAnimationCounter >= mCurrentAnimation->speed()) {
	unsigned int f = frame() + 1;
	if (f >= mCurrentAnimation->start() + mCurrentAnimation->range()) {
		f = mCurrentAnimation->start();
	}
	setFrame(f);
	mAnimationCounter = 0;
 }
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
 BO_CHECK_NULL_RET(mModel);
 BO_CHECK_NULL_RET(mCurrentFrame);
 mModel->enablePointer();
 mCurrentFrame->renderFrame(teamColor(), lod);
}

BosonCollisions* BosonItem::collisions() const
{
 return canvas()->collisions();
}

void BosonItem::moveParticleSystems(float x, float y, float z)
{
 if (particleSystems() && particleSystems()->count() > 0) {
	BoVector3 pos(x + width() / 2, y + height() / 2, z);
	pos.canvasToWorld();
	QPtrListIterator<BosonParticleSystem> it(*particleSystems());
	for (; it.current(); ++it) {
		it.current()->setPosition(pos);
	}
 }
}

void BosonItem::rotateParticleSystems(float angle, float x, float y, float z)
{
 if (angle == 0.0) {
	return;
 }
 if (particleSystems() && particleSystems()->count() > 0) {
	QPtrListIterator<BosonParticleSystem> it(*particleSystems());
	for (; it.current(); ++it) {
		it.current()->setRotation(angle, x, y, z);
	}
 }
}

const QPtrList<BosonParticleSystem>* BosonItem::particleSystems() const
{
 return 0l;
}

void BosonItem::clearParticleSystems()
{
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

unsigned int BosonItem::lodCount() const
{
 if (!mModel) {
	return 1;
 }
 return mModel->lodCount();
}

