/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
/***************************************************************************
                         selectPart.h  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jun 26 16:23:00 CET 1999
                                           
    copyright            : (C) 1999-2000 by Thomas Capricelli                         
    email                : orzel@yalbi.com                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef __SELECTPART_H__
#define __SELECTPART_H__

#include <qcanvas.h>
#include "rtti.h"

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

protected:
	QCanvasPixmapArray* initStatic(SelectPartType type);

private:
	void init(SelectPartType type);

private:
	static QCanvasPixmapArray *mPartUp;
	static QCanvasPixmapArray *mPartDown;
};

#endif // __SELECTPART_H__

