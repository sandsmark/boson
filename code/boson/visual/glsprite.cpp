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

 mIsVisible = false;

 mXVelocity = 0.0;
 mYVelocity = 0.0;

 mModel = model;

 mBoundingSphereRadius = 1.0; // TODO: can we extract this from the model? this probably needs to change with different frames!
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

void GLSprite::setVisible(bool v)
{
 // FIXME: remove from canvas ;
 if (mIsVisible != v) {
	mIsVisible = v;
	if (v) {
		// FIXME: add to cells ?
	} else {
		// FIXME: remove from cells ? 
	}
 }
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

