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

/*
void fieldMap::paintEvent(QPaintEvent *evt)
{
QPainter p;
QRect r = evt->rect();
int i,j;
int im,iM,jm,jM;

p.begin(this);
im = r.left()/BO_TILE_SIZE + view->X();
jm = r.top()/BO_TILE_SIZE + view->Y();
iM = QMIN (r.right()/BO_TILE_SIZE + view->X() + 1, view->maxX());
jM = QMIN (r.bottom()/BO_TILE_SIZE + view->Y() + 1, view->maxY());
for(i=im; i<iM; i++)
	for(j=jm; j<jM; j++)
		drawCell(i,j, &p);

// Dessin des unites mobiles et fixes
QPixmap		*pix;
QIntDictIterator<playerMobUnit>	mobIt(view->phys->mobile);
QIntDictIterator<Facility>	fixIt(view->phys->facility);


for (mobIt.toFirst(); mobIt; ++mobIt) {
	pix = getPixmap(mobIt.current());
	if (pix) drawRelative(mobIt.current()->x, mobIt.current()->y, pix ,&p);
	}

for (fixIt.toFirst(); fixIt; ++fixIt) {
	pix = getPixmap(fixIt.current());
	if (pix) drawRelative(fixIt.current()->x*BO_TILE_SIZE, fixIt.current()->y*BO_TILE_SIZE, pix ,&p);
	}

drawSelected(&p);

p.end();

} */

/*
void fieldMap::drawCell(int i, int j, QPainter *p)
{
groundType g = view->phys->getGround(i,j);
assert(i<view->maxX());
assert(j<view->maxY());

//if ( GROUND_FACILITY == g) return;
drawRelative( BO_TILE_SIZE*i, BO_TILE_SIZE*j, gameProperties.ground->getPixmap(g), p );
}
*/

/*
void fieldMap::drawMobile(playerMobUnit *unit, QPainter *p) {
drawRelative(unit->x, unit->y, gameProperties.species[unit->who]->getPixmap(unit), p );
}

void fieldMap::drawFix(Facility *fix, QPainter *p)
{
drawRelative(fix->x * BO_TILE_SIZE , fix->y * BO_TILE_SIZE, gameProperties.species[fix->who]->getPixmap(fix), p );
}
*/


/*

void fieldMap::drawRelative(int x, int y, QPixmap *pix, QPainter *painter)
{
x -= BO_TILE_SIZE * view->X();
y -= BO_TILE_SIZE * view->Y();

printf("drawrelative : %d, %d\n", x, y);

if (painter)
	painter->drawPixmap(x,y,*pix);
else {
	painter=new QPainter();
	painter->begin(this);
	painter->drawPixmap(x,y,*pix);
	painter->end();
	}
	
}
*/


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
//playerMobUnit *m;
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

//printf("roundRect : %d.%d %dx%d\n", r.x(), r.y(), r.width(), r.height());

	painter->drawRoundRect(
		(fix->x()-view->X()) * BO_TILE_SIZE - BO_SELECT_MARGIN,
		(fix->y()-view->Y()) * BO_TILE_SIZE - BO_SELECT_MARGIN,
		fix->getWidth()* BO_TILE_SIZE + BO_SELECT_MARGIN + BO_SELECT_MARGIN,
		fix->getHeight()* BO_TILE_SIZE + BO_SELECT_MARGIN + BO_SELECT_MARGIN, 
	//	r,
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

//printf("roundRect 3: %d.%d %dx%d\n", r.x(), r.y(), r.width(), r.height());
	painter->drawRoundRect(
		r,
		BO_SELECT_ROUND, BO_SELECT_ROUND
		);
	}
}

/*
void fieldMap::drawRectSelect(int x1, int y1, int x2, int y2, QPainter &movPainter)
{
movPainter.drawRect(x1, y1, x2-x1, y2-y1); // inch Allah
}
*/


