/***************************************************************************
                          miniEvent.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Feb 17, 1999
                                           
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


#include "miniMap.h"


void miniMap::mousePressEvent(QMouseEvent *e)
{
	int x, y;

	x = e->x();
	y = e->y();

	if (e->button() & LeftButton) {
		emit reCenterView(x,y);
		return;
		}

}
