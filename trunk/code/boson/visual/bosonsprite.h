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
#include "glsprite.h"

class BosonCanvas;
class QPointArray;
class SelectBox;
class BosonModel;

class BosonSprite : public GLSprite
{
public:
	/**
	 * Note: when you subclass this class you must set the width/height in
	 * order to make correct use of it! See @ref GLSprite::setWidth
	 **/
	BosonSprite(BosonModel* model, BosonCanvas*);
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
		return GLSprite::canvas();
	}

	bool bosonCollidesWith(BosonSprite* item) const;

	QPointArray cells() const;

	virtual void setAnimated(bool a);
	inline bool isAnimated() const { return mIsAnimated; }

	/**
	 * See @ref Unit::advance
	 **/
	virtual void advance(unsigned int ) {}

	/**
	 * See @ref Unit::advanceFunction
	 **/
	inline virtual void advanceFunction(unsigned int /*advanceCount*/) { }

	/**
	 * See @ref Unit::advanceFunction2
	 **/
	inline virtual void advanceFunction2(unsigned int /*advanceCount*/) { }

private:
	bool mIsAnimated;
	SelectBox* mSelectBox;

};

#endif

