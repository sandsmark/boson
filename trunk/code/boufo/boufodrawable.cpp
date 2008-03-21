/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <bogl.h>

// AB: first include the ufo headers, otherwise we conflict with Qt
#include <ufo/ufo.hpp>

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufodrawable.h"

class UMyDrawable : public ufo::UDrawable
{
public:
	UMyDrawable(BoUfoDrawable* drawable)
	{
		mDrawable = drawable;
	}

	virtual void paintDrawable(ufo::UGraphics*, const ufo::URectangle& r)
	{
		mDrawable->render(r.x, r.y, r.w, r.h);
	}
	virtual ufo::UDimension getDrawableSize() const
	{
		return ufo::UDimension(mDrawable->drawableWidth(), mDrawable->drawableHeight());
	}

private:
	BoUfoDrawable* mDrawable;
};

BoUfoDrawable::BoUfoDrawable()
{
 mDrawable = new UMyDrawable(this);
 mDrawable->reference();
}

BoUfoDrawable::~BoUfoDrawable()
{
 mDrawable->unreference();
}


