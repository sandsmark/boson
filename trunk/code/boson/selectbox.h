/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef __SELECTBOX_H__
#define __SELECTBOX_H__

#include <qcanvas.h>
#include "rtti.h"

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>
 **/
class SelectBox : public QCanvasSprite
{
public:
	/**
	 * Construct one part of the selection box.
	 * @ref canvas Guess what?
	 * @param width The width of the <em>unit</em>. The width of the
	 * selectbox will be higher!
	 * @param height The height of the <em>unit</em>. The height of the
	 * selectbox will be higher!
	 * @param z See @ref Unit::z
	 **/
	SelectBox(int x, int y, int width, int height, int z,QCanvas* canvas);

	virtual int rtti() const 
	{ 
		return RTTI::SelectPart;
	}

	/**
	 * @return How many frames does a SelectPart have
	 **/
	static int frames();

	/**
	 * @param factor the number that should be muliplited with @ref frames
	 * - 1. Should be health/maxHealth.
	 **/
	void update(double factor);

protected:
	QCanvasPixmapArray* initPixmapArray();

	void drawSelectBox(QPainter& painter, bool mask);
	void drawHealthBar(QPainter& painter, bool mask, int power = 0);

	/**
	 * @return The width of the health bar when health is at factor.
	 * @param frame The frame number. Note that frame==frames()-1 means full
	 * health
	 **/
	int barWidth(int frame);

	/**
	 * @return The height of the health bar
	 **/
	int barHeight();

	int boxWidth() const;
	int boxHeight() const;

	/**
	 * @return The length of a line of a corner. It must be bigger for big
	 * units and smaller for smaller units. Otherwise we don't have 4
	 * corners but a single rect (or 4 very small corners compared to the
	 * unit's size)
	 **/
	int cornerLength() const;

private:
	void init();

private:
	QCanvasPixmapArray *mPixmapArray;

	int mWidth;
	int mHeight;
};

#endif // __SELECTBOX_H__

