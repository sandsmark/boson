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

	/**
	 * @return TRUE if the object is selected, i.e. a select box should be
	 * drawn around it. Otherwise FALSE.
	 **/
	inline bool isSelected() const { return mSelectBox != 0; }

	/**
	 * @return The select box of this item, or NULL if it is not selected.
	 * See @ref isSelected.
	 **/
	inline SelectBox* selectBox() const { return mSelectBox; }

	/**
	 * Select this unit, i.e. construct the select box - see @ref isSelected.
	 * Note that @ref SelectBox is constructed very fast - the box is
	 * constructed only once and the display list is stored as a static
	 * variable.
	 * @param markAsLeader The leader of a group of units/items will have a
	 * slightly different select box (e.g. another color or so).
	 **/
	virtual void select(bool markAsLeader = false);

	/**
	 * Unselect the unit, i.e. delete the select box.
	 **/
	virtual void unselect();

	/**
	 * @return Unique identifier of this object type. E.g. RTTI::Unit for
	 * all units.
	 **/
	virtual int rtti() const = 0;

	/**
	 * @return The (cell-)coordinates of the left-top cell this object
	 * occupies. This can be used for very efficient collision detection.
	 * See @ref rightBottomCell
	 **/
	inline void leftTopCell(int* left, int* top)  const
	{
		*left = (int)(leftEdge() / BO_TILE_SIZE);
		*top = (int)(topEdge() / BO_TILE_SIZE);
	}

	/**
	 * @return The (cell-)coordinates of the lower-right cell this object
	 * occupies. This can be used for very efficient collision detection.
	 * See @ref leftTopCell
	 **/
	inline void rightBottomCell(int* right, int* bottom) const
	{
		*right = (int)(rightEdge() / BO_TILE_SIZE);
		*bottom= (int)(bottomEdge() / BO_TILE_SIZE);
	}

	bool bosonCollidesWith(BosonSprite* item) const;

	/**
	 * Note that this function calculates the returned array - so if you are
	 * doing time critical operations then cache the result, instead of
	 * calling this twice.
	 * @return An array of all cells this unit occupies.
	 **/
	QPointArray cells() const;

	/**
	 * Make this item animated, i.e. call @ref advance on it. You don't need
	 * to set it animated if you don't reimplement @ref advance. See also
	 * @ref BosonCanvas::addAnimation and @ref BosonCanvas::slotAdvance
	 **/
	void setAnimated(bool a);
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

