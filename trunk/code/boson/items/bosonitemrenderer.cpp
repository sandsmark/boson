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
#include "bosonitemrenderer.h"

#include "../bosonmodel.h"
#include "../bo3dtools.h"
#include "bosonitem.h"
#include "bodebug.h"

BosonItemRenderer::BosonItemRenderer(BosonItem* item)
{
 mItem = item;

 mGLDepthMultiplier = 1.0f;
 mShowGLConstructionSteps = true;

 // 1.732 == sqrt(3) i.e. lenght of vector whose all components are 1
 mBoundingSphereRadius = 1.732f; // TODO: can we extract this from the model? this probably needs to change with different frames!
}


BosonItemRenderer::~BosonItemRenderer()
{
}

const QColor* BosonItemRenderer::teamColor() const
{
 return mItem->teamColor();
}

void BosonItemRenderer::setGLDepthMultiplier(float d)
{
 mGLDepthMultiplier = d;
}

void BosonItemRenderer::renderItem(unsigned int lod)
{
}

bool BosonItemRenderer::itemInFrustum(const float* frustum) const
{
 if (!frustum) {
	return false;
 }
 if (!mItem) {
	BO_NULL_ERROR(mItem);
	return false;
 }
 // FIXME: can't we use BoVector3 and it's conversion methods here?
 GLfloat x = (mItem->x() + mItem->width() / 2) * BO_GL_CELL_SIZE / BO_TILE_SIZE;
 GLfloat y = -((mItem->y() + mItem->height() / 2) * BO_GL_CELL_SIZE / BO_TILE_SIZE);
 GLfloat z = mItem->z(); // this is already in the correct format!
 BoVector3 pos(x, y, z);
 return Bo3dTools::sphereInFrustum(frustum, pos, boundingSphereRadius());
}


BosonItemModelRenderer::BosonItemModelRenderer(BosonItem* item)
	: BosonItemRenderer(item)
{
 mModel = 0;

 mCurrentFrame = 0;
 mCurrentAnimation = 0;
 mFrame = 0;
 mGLConstructionStep = 0;
 mAnimationCounter = 0;
}

BosonItemModelRenderer::~BosonItemModelRenderer()
{
}

bool BosonItemModelRenderer::setModel(BosonModel* model)
{
 mModel = model;
 if (!mModel) {
	boError() << k_funcinfo << "NULL model - we will crash!" << endl;
	return false;
 }

 setAnimationMode(UnitAnimationIdle);

 // FIXME the correct frame must be set after this constructor!
 if (mGLConstructionStep >= glConstructionSteps() && showGLConstructionSteps()) {
	setCurrentFrame(mModel->frame(frame()));
 } else {
	setCurrentFrame(mModel->constructionStep(mGLConstructionStep));
 }
 return true;
}

void BosonItemModelRenderer::setGLConstructionStep(unsigned int s)
{
 BO_CHECK_NULL_RET(model());
 // note: in case of s >= model()->constructionSteps() we use the last
 // constructionStep that is defined in the model until an actual frame is set.
 BoFrame* f = model()->constructionStep(s);
 if (!f) {
	boWarning() << k_funcinfo << "NULL construction step " << s << endl;
	return;
 }
 mGLConstructionStep = s;
 if (showGLConstructionSteps()) {
	setCurrentFrame(f);
 }
}

unsigned int BosonItemModelRenderer::glConstructionSteps() const
{
 BO_CHECK_NULL_RET0(model());
 return model()->constructionSteps();
}

void BosonItemModelRenderer::setFrame(int _frame)
{
 BO_CHECK_NULL_RET(model());
 if (mGLConstructionStep < glConstructionSteps() && showGLConstructionSteps()) {
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

unsigned int BosonItemModelRenderer::frameCount() const
{
 return model() ? model()->frames() : 0;
}

void BosonItemModelRenderer::setCurrentFrame(BoFrame* frame)
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

void BosonItemModelRenderer::setAnimationMode(int mode)
{
 BO_CHECK_NULL_RET(model());
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

void BosonItemModelRenderer::animate()
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

void BosonItemModelRenderer::renderItem(unsigned int lod)
{
 BO_CHECK_NULL_RET(mModel);
 BO_CHECK_NULL_RET(mCurrentFrame);
 mModel->prepareRendering();
 mCurrentFrame->renderFrame(teamColor(), lod);
}

unsigned int BosonItemModelRenderer::lodCount() const
{
 if (!mModel) {
	return 1;
 }
 return mModel->lodCount();
}

unsigned int BosonItemModelRenderer::preferredLod(float dist) const
{
 if (!mModel) {
	return 0;
 }
 return mModel->preferredLod(dist);
}

void BosonItemModelRenderer::setShowGLConstructionSteps(bool s)
{
 BO_CHECK_NULL_RET(model());
 BosonItemRenderer::setShowGLConstructionSteps(s);
 if (showGLConstructionSteps()) {
	setGLConstructionStep(mGLConstructionStep);
 } else {
	setCurrentFrame(mModel->frame(frame()));
 }
}

