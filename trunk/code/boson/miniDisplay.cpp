/***************************************************************************
                          miniDisplay.cpp  -  description                              
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

#include <assert.h>

#include <qpainter.h>
#include <qpixmap.h>
#include <qcolor.h>

#include "../common/log.h"
#include "../map/map.h"

#include "miniMap.h"
#include "playerCell.h"
#include "speciesTheme.h"
#include "groundTheme.h"
#include "viewMap.h"
#include "game.h"

void miniMap::paintEvent(QPaintEvent *evt)
{
QPainter p;


p.begin(this);

/* map is buffered */
p.drawPixmap(0,0,*ground);

/* the little rectangle */
p.setPen(white);
p.setRasterOp(XorROP);
p.drawRect(view->X(), view->Y(), view->L()-1, view->H()-1);

p.end();

}

void miniMap::newCell(int i, int j, groundType g) //, QPainter *p)
{
QPainter p;
assert(i<view->maxX());
assert(j<view->maxY());

//printf("miniMap::newCell : receiving %d\n", (int)g);


if (g>= GROUND_LAST)
	g = groundTransProp[(g-GROUND_LAST)/TILES_PER_TRANSITION].from;

p.begin(ground);
switch(g) {
	default:
		logf(LOG_ERROR, "miniMap::drawCell : unexpected groundType");
	case GROUND_WATER :
		setPoint( i, j, blue, &p);
		break;
	case GROUND_GRASS :
		setPoint( i, j, green, &p);
		break;
	case GROUND_DESERT :
		setPoint( i, j, darkYellow, &p);
		break;

	}
p.end();
repaint(FALSE);
}

void miniMap::drawMobile(playerMobUnit *unit)
{
QPainter p;
p.begin(ground);
setPoint(unit->_x()/BO_TILE_SIZE, unit->_y()/BO_TILE_SIZE, (unit->who==gameProperties.who_am_i)?magenta:darkMagenta, &p);
p.end();
repaint(FALSE);
}

void miniMap::drawFix(playerFacility *fix)
{
QPainter p;
p.begin(ground);
setPoint(fix->_x(), fix->_y(), (fix->who==gameProperties.who_am_i)?magenta:darkMagenta, &p);
p.end();
repaint(FALSE);
}

void miniMap::setPoint(int x, int y, const QColor &color, QPainter *p)
{
if (!p) {
	logf(LOG_ERROR, "setPoint: p == 0...");
	return;
	}
p->setPen(color);
p->drawPoint(x,y);
}
/*
void fieldMap::drawRectSelect(int x1, int y1, int x2, int y2, QPainter &movPainter)
{
movPainter.drawRect(x1, y1, x2-x1, y2-y1); // inch Allah

}
*/
