/***************************************************************************
                          viewMap.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
    copyright            : (C) 1999 by Thomas Capricelli                         
    email                : capricel@enst.fr                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include "viewMap.h"


viewMap::viewMap(physMap *p, QObject *parent, const char *name=0L)
	: QObject(parent, name)
	,fixSelected( 0L )
	,selectionMode(SELECT_NONE)
{
	/* map geometry */
	viewL = viewH = 5; ///orzel : arbitraire, (doit etre/)sera fixe par un mainMap..
	viewX = viewY = 0;
	phys = p;
}


void viewMap::reCenterView(int x, int y)
{
	int oldX = viewX, oldY = viewY;

	viewX  = x - viewL/2;
	viewY  = y - viewH/2;

	viewX = QMIN(viewX, phys->maxX - viewL);
	viewY = QMIN(viewY, phys->maxY - viewH);

	viewX = QMAX(viewX, 0);
	viewY = QMAX(viewY, 0);

	if (viewX != oldX || viewY != oldY) {
		emit repaint(FALSE);
		}
}


void viewMap::reSizeView(int l, int h)
{
	int	Xcenter = viewX + viewL/2,
		Ycenter = viewY + viewH/2;

	viewL = l;
	viewH = h;

	reCenterView(Xcenter, Ycenter);
}

