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
#include "glsprite.h"

#include <kdebug.h>

#include <qrect.h>

GLSprite::GLSprite(BosonModel* model, BosonCanvas* canvas)
{
 mCanvas = canvas;
 mX = mY = mZ = 0.0;
 mWidth = mHeight = 0;
 mGLDepthMultiplier = 1.0;
 mDisplayList = 0;
 mFrame = 0;
 mGLConstructionStep = 0;

 mXVelocity = 0.0;
 mYVelocity = 0.0;

 mModel = model;

 mBoundingSphereRadius = 1.0; // TODO: can we extract this from the model? this probably needs to change with different frames!

 if (!mModel) {
	kdError() << k_funcinfo << "NULL model - we will crash!" << endl;
	return;
 }
 // FIXME the correct frame must be set after this constructor!
 if (mGLConstructionStep >= mModel->constructionSteps()) {
	setCurrentFrame(mModel->frame(frame()));
 } else {
	setCurrentFrame(mModel->constructionStep(mGLConstructionStep));
 }
}

GLSprite::~GLSprite()
{
}

QRect GLSprite::boundingRect() const
{
 return QRect((int)leftEdge(), (int)topEdge(), width(), height());
}

QRect GLSprite::boundingRectAdvanced() const
{
 return QRect((int)(leftEdge() + xVelocity()),
		(int)(topEdge() + yVelocity()),
		(int)(width() + xVelocity()),
		(int)(height() + yVelocity()));
}

void GLSprite::setCanvas(BosonCanvas* c)
{
 mCanvas = c;
}

void GLSprite::setWidth(int w)
{
 mWidth = w;
}

void GLSprite::setHeight(int h)
{
 mHeight = h;
}

void GLSprite::setGLDepthMultiplier(float d)
{
 mGLDepthMultiplier = d;
}

void GLSprite::setGLConstructionStep(unsigned int s)
{
 // note: in case of s >= model()->constructionSteps() we use the last
 // constructionStep that is defined in the model until an actual frame is set.
 BoFrame* f = model()->constructionStep(s);
 setCurrentFrame(f);
}

