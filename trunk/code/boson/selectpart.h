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

#ifndef __SELECTPART_H__
#define __SELECTPART_H__

#include <qcanvas.h>
#include "rtti.h"

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>
 **/
class SelectPart : public QCanvasSprite
{
public:
	enum SelectPartType { 
		PartUp,
		PartDown
	};

	/**
	 * Construct one part of the selection box.
	 * @param z See the z coordinate of the selected unit. It it
	 * automatically increased so that the box is above the unit.
	 * @param type Which type of the box is this - the upper or the lower
	 * part. See @ref SelectPartType
	 * @ref canvas Guess what?
	 **/
	SelectPart(int z, SelectPartType type, QCanvas* canvas);

	/**
	 * Construct one part of the selection box.
	 * @param frame The start frame of the box. Different frames show
	 * different value of power.
	 * @param z See the z coordinate of the selected unit. It it
	 * automatically increased so that the box is above the unit.
	 * @param type Which type of the box is this - the upper or the lower
	 * part. See @ref SelectPartType
	 * @ref canvas Guess what?
	 **/
	SelectPart(int frame, int z, SelectPartType type, QCanvas* canvas);
	
	virtual int rtti() const 
	{ 
		// unclean!
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
	QCanvasPixmapArray* initStatic(SelectPartType type);

	static void drawSelectBox(QPainter& painter, bool bw, int power = 0);

private:
	void init(SelectPartType type);

private:
	static QCanvasPixmapArray *mPartUp;
	static QCanvasPixmapArray *mPartDown;
};

#endif // __SELECTPART_H__

