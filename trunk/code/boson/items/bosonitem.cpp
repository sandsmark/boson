/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bodebug.h"

#include <qrect.h>
#include <qptrlist.h>

BosonItem::BosonItem(BosonModel* model, BosonCanvas* canvas)
{
 mCanvas = canvas;
 mModel = model;
 mCurrentAnimation = 0;
 mX = mY = mZ = 0.0f;
 mWidth = mHeight = 0;
 mCellsDirty = true;
 mRotation = 0.0f;
 mXRotation = 0.0f;
 mYRotation = 0.0f;
 mGLDepthMultiplier = 1.0f;
 mDisplayList = 0;
 mFrame = 0;
 mGLConstructionStep = 0;
 mAnimationCounter = 0;
 mCurrentFrame = 0;

 mXVelocity = 0.0f;
 mYVelocity = 0.0f;
 mZVelocity = 0.0f;

 mCurrentAnimation = 0;
 // 1.732 == sqrt(3) i.e. lenght of vector whose all components are 1
 mBoundingSphereRadius = 1.732f; // TODO: can we extract this from the model? this probably needs to change with different frames!

 mIsAnimated = false;
 mSelectBox = 0;

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
 setAnimationMode(0);

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
	right = QMIN(right, QMAX((int)canvas()->mapWidth() - 1, 0));
	bottom = QMIN(bottom, QMAX((int)canvas()->mapHeight() - 1, 0));
	makeCells(canvas(), &mCells, left, right, top, bottom);
	mCellsDirty = false;
 }
 return &mCells;
}

void BosonItem::makeCells(const BosonCanvas* canvas, QPtrVector<Cell>* cells, int left, int right, int top, int bottom)
{
 left = QMAX(left, 0);
 top = QMAX(top, 0);
 right = QMAX(left, right);
 bottom = QMAX(top, bottom);

 int size = (right - left + 1) * (bottom - top + 1);
 cells->resize(size);
 if (size == 0) {
	return;
 }
 int n = 0;
 for (int i = left; i <= right; i++) {
	for (int j = top; j <= bottom; j++) {
		// AB: canvas->cell() is not really efficient here. we should
		// prefer direct access on BosonMap::cells() using
		// BosonMap::cellArrayPos(). But we would have to include
		// bosonmap.h for this, so we do it this way.
		// we may want to replace if it turns out that we need more
		// speed (i dont think so, btw).
		Cell* c = canvas->cell(i, j);
		if (!c) {
			continue;
		}
		cells->insert(n, c);
		n++;
	}
 }
}

bool BosonItem::bosonCollidesWith(BosonItem* item) const
{
  // New collision-check method for units
 if (!RTTI::isUnit(item->rtti())) {
	switch (item->rtti()) {
		// items we never collide with:
//		case RTTI::Missile:
//		case RTTI::OilField:
//			return false;
		default:
			// we have unknown item here!
			// this must not happen, since an unknown item here is a major
			// performance problem - but at least it'll be important to fix it
			// then :)
			boWarning() << k_funcinfo << "unknown item - rtti=" << item->rtti() << endl;
			return false;
	}
 }

 // I use centers of units as positions here
 double myx, myy, itemx, itemy;
 QRect r = boundingRectAdvanced();
 QRect r2 = item->boundingRectAdvanced();
 myx = r.center().x();
 myy = r.center().y();
 itemx = r2.center().x();
 itemy = r2.center().y();

 double itemw, itemh;
 itemw = r2.width();
 itemh = r2.height();

 if (itemw <= BO_TILE_SIZE && itemh <= BO_TILE_SIZE) {
	double dist = QABS(itemx - myx) + QABS(itemy - myy);
	return (dist < BO_TILE_SIZE);
 } else {
	for (int i = 0; i < itemw; i += BO_TILE_SIZE) {
		for (int j = 0; j < itemh; j += BO_TILE_SIZE) {
			double dist = QABS((itemx + i) - myx) + QABS((itemy + j) - myy);
			if (dist < BO_TILE_SIZE) {
				return true;
			}
		}
	}
 }
 return false;
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
 return QRect((int)leftEdge(), (int)topEdge(), width() - 1, height() - 1);
}

QRect BosonItem::boundingRectAdvanced() const
{
 return QRect((int)(leftEdge() + xVelocity()),
		(int)(topEdge() + yVelocity()),
		width() + (int)xVelocity() - 1,
		height() + (int)yVelocity() - 1);
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
 mCurrentFrame = frame;

 // the following values cache values from BoFrame, so that we can use them in
 // inline functions (or with direct access). otherwise we'd have to #include
 // bosonmodel.h in bosonitem.h (-> bad)
 setDisplayList(frame->displayList());
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

void BosonItem::setSize(int width, int height)
{
 removeFromCells();
 mWidth = width;
 mHeight = height;
 mCellsDirty = true;
 addToCells();
}

void BosonItem::renderItem()
{
 mModel->enablePointer();
 if (displayList() != 0) {
	glCallList(displayList());
 } else {
	mCurrentFrame->renderFrame();
 }
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
 if (particleSystems() && particleSystems()->count() > 0) {
	QPtrListIterator<BosonParticleSystem> it(*particleSystems());
	for (; it.current(); ++it) {
		it.current()->setRotation(-angle, x, y, z);
	}
 }
}

QPtrList<BosonParticleSystem>* BosonItem::particleSystems() const
{
 return 0l;
}

