/***************************************************************************
                          miniEvent.cpp  -  description                              
                             -------------------                                         

    version              :                                   
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

#include <qpainter.h>
#include "../common/log.h"
#include "viewMap.h"
#include "miniMap.h"

#include "../map/map.h"


//static int oldX, oldY;

void miniMap::mousePressEvent(QMouseEvent *e)
{
int x, y;

//bool found = FALSE;

//x = e->x() / 32;
//y = e->y() / 32;
x = e->x();
y = e->y();

if (e->button() & RightButton) {
	emit reCenterView(x,y);
	return;
	}
}

/*
void miniMap::mouseMoveEvent(QMouseEvent *e)
{
}
*/

/*
void miniMap::resizeEvent(QResizeEvent *e)
{
//reSizeView(width()/32, height()/32);
emit reSizeView((width()+32+1)/32, (height()+32+1)/32);
}

*/
