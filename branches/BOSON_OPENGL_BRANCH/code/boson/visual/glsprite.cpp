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
#ifndef NO_OPENGL

#include <kdebug.h>

#include <qrect.h>

GLSprite::GLSprite(BosonTextureArray* array, BosonCanvas* canvas)
{
 mCanvas = canvas;
 for (int i = 0; i < 3*4; i++) {
	mVertexPointer[i] = 0.0;
 }
 mX = mY = mZ = 0.0;

 mIsVisible = false;

 mFrame = 0;

 mXVelocity = 0.0;
 mYVelocity = 0.0;

 mTextures = array;
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




#endif
