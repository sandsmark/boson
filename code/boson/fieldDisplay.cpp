/***************************************************************************
                          fieldDisplay.cpp  -  description                              
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
#include <qcolor.h>
#include <assert.h>
#include "../common/log.h"
#include "../map/map.h"
#include "fieldMap.h"
#include "playerCell.h"
#include "speciesTheme.h"
#include "groundTheme.h"
#include "viewMap.h"
#include "game.h"

QRect fieldMap::viewArea() const
{
//printf("fieldMap::viewArea = %d.%d, %dx%d\n", BO_TILE_SIZE * view->X(), BO_TILE_SIZE * view->Y(), width(), height());
return QRect(BO_TILE_SIZE * view->X(), BO_TILE_SIZE * view->Y(), width(), height());
boAssert(width() / BO_TILE_SIZE == view->L());
boAssert(height() / BO_TILE_SIZE == view->H());
}

void fieldMap::flush(const  QRect & area)
{
/* nothing special.. */
///orzel : to change is some kind of off-screen buffering is used
}

void fieldMap::beginPainter (QPainter &p)
{
///orzel : to change if some kind of off-screen buffering is used

p.begin(this);
p.translate( - BO_TILE_SIZE * view->X(), - BO_TILE_SIZE * view->Y());
p.setBackgroundColor(black);

boAssert(p.backgroundColor() == black);

//p.setBackgroundMode(OpaqueMode);
}


void fieldMap::paintEvent(QPaintEvent *evt)
{
	if (viewing) {
		QRect r = evt->rect();
		r = rect();
//printf("r = %d.%d, %dx%d\n", r.x(), r.y(), r.width(), r.height());
		r.moveBy(view->X() * BO_TILE_SIZE, view->Y() * BO_TILE_SIZE);
//printf("fieldMap::paintEvents, moved : %d.%d, %dx%d\n", r.x(), r.y(), r.width(), r.height());
		viewing->updateInView(this, r);
	}

}

void fieldMap::drawSelected(void)
{
QPainter *painter=new QPainter();

painter->begin(this);
drawSelected(painter);
painter->end();
}

void fieldMap::drawSelected(QPainter *painter)
{
QIntDictIterator<playerMobUnit> selIt(order->mobSelected);
QPen	pen(green, 3, DashDotLine);
QRect	r;

painter->setPen(pen);

playerFacility *fix = order->fixSelected;
if(fix) {
	r = fix->rect();
	r.moveBy(
		- BO_SELECT_MARGIN - view->X()*BO_TILE_SIZE,
		- BO_SELECT_MARGIN - view->Y()*BO_TILE_SIZE
		);
	r.setWidth(r.width()+2*BO_SELECT_MARGIN); 
	r.setHeight(r.height()+2*BO_SELECT_MARGIN); 

	painter->drawRoundRect(
		r,
		BO_SELECT_ROUND, BO_SELECT_ROUND
		);
	boAssert(order->mobSelected.isEmpty());
	return; // if a fix is selected, no mobile should be selected
	}


for (selIt.toFirst(); selIt; ++selIt) {

	r = selIt.current()->rect();
	r.moveBy(
		- BO_SELECT_MARGIN - view->X()*BO_TILE_SIZE,
		- BO_SELECT_MARGIN - view->Y()*BO_TILE_SIZE
		);
	r.setWidth(r.width()+2*BO_SELECT_MARGIN); 
	r.setHeight(r.height()+2*BO_SELECT_MARGIN); 

	painter->drawRoundRect(
		r,
		BO_SELECT_ROUND, BO_SELECT_ROUND
		);
	}
}
