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
#ifndef BOSONSPRITE_H
#define BOSONSPRITE_H

#include "../defines.h"

#ifndef NO_OPENGL
#include "glsprite.h"
#else
#include <qcanvas.h>
#endif

class BosonCanvas;
class QPointArray;
class SelectBox;

#ifndef NO_OPENGL
class BosonSprite : public GLSprite
#else
class BosonSprite : public QCanvasSprite
#endif
{
public:
#ifndef NO_OPENGL
	BosonSprite(BosonTextureArray* array, BosonCanvas*);
#else
	BosonSprite(QCanvasPixmapArray* array, BosonCanvas*);
#endif
	virtual ~BosonSprite();

	inline bool isSelected() const { return mSelectBox != 0; }
	inline SelectBox* selectBox() const { return mSelectBox; }
	virtual void select(bool markAsLeader = false);
	virtual void unselect();

	virtual void setCanvas(BosonCanvas* c);

	virtual int rtti() const = 0;

	inline void leftTopCell(int* left, int* top)  const
	{
		*left = (int)(leftEdge() / BO_TILE_SIZE);
		*top = (int)(topEdge() / BO_TILE_SIZE);
	}

	inline void rightBottomCell(int* right, int* bottom) const
	{
		*right = (int)(rightEdge() / BO_TILE_SIZE);
		*bottom= (int)(bottomEdge() / BO_TILE_SIZE);
	}

	inline BosonCanvas* boCanvas() const
	{
#ifndef NO_OPENGL
		return GLSprite::canvas();
#else
		//FIXME: can we use explicit conversion here without problems?
		return (BosonCanvas*)canvas();
#endif
	}

#ifdef NO_OPENGL
	bool collidesWith(QCanvasItem* item) const;
#endif

	bool bosonCollidesWith(BosonSprite* item) const;

	QPointArray cells() const;

	virtual void setAnimated(bool a);
	inline bool isAnimated() const { return mIsAnimated; }

	/**
	 * We reimplement this in Unit anyway. See @ref QCanvasSprite::advance
	 * for comments on how to use advance methods.
	 **/
	virtual void advance(int ) {}

	
#ifdef NO_OPENGL
	virtual void moveBy(double dx, double dy)
	{
		moveBy((float)dx, (float)dy, 0.0);
	}
	virtual void moveBy(float dx, float dy, float dz)
	{
		QCanvasSprite::moveBy((double)dx, (double)dy);
		if (dz) {
			setZ(z() + dz);
		}
	}
#endif

private:
	bool mIsAnimated;
	SelectBox* mSelectBox;
};

#endif
