/***************************************************************************
                          fieldEvent.cpp  -  description                              
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
#include "fieldMap.h"
#include "../map/map.h"


static int selectX, selectY;
static int oldX, oldY;

void fieldMap::mousePressEvent(QMouseEvent *e)
{
int x, y;

bool found = FALSE;

//x = e->x() / BO_TILE_SIZE;
//y = e->y() / BO_TILE_SIZE;
x = e->x();
y = e->y();

if (e->button() & RightButton) {
	emit relativeReCenterView( x/BO_TILE_SIZE , y/BO_TILE_SIZE);
	return;
	}

if (e->button() & LeftButton) {	
	/* Here we transpose coo into the map referential */
	x += view->X()*BO_TILE_SIZE; y += view->Y()*BO_TILE_SIZE;


	if (SELECT_MOVE == order->getSelectionMode()) {
		order->leftClicked( x, y);
		return;
		}

	/* Control -> multiselection, else... */
	if (! (e->state()&ControlButton)) {
		unSelectAll();
		}

	QIntDictIterator<playerMobUnit> mobIt(view->phys->mobile);
	QIntDictIterator<playerFacility> fixIt(view->phys->facility);

	playerMobUnit	*m;
	playerFacility	*f;

//printf("\n\nselection , x=%d,y=%d\n", x, y);
	for (fixIt.toFirst(); fixIt; ++fixIt) {
		f = fixIt.current();
/*		printf("f->rect() : (x,y)=%d,%d, (w,h)=%d,%d\n", 
			f->rect().x(),
			f->rect().y(),
			f->rect().width(),
			f->rect().height()
			); */
		if (f->rect().contains( QPoint( x, y) )) {
			unSelectAll();
			order->selectFix(f);
			found = TRUE;
			break;
			}
		}

	for (mobIt.toFirst(); mobIt; ++mobIt) {
		m = mobIt.current();
/*		printf("m->rect() : (x,y)=%d,%d, (w,h)=%d,%d\n", 
			m->rect().x(),
			m->rect().y(),
			m->rect().width(),
			m->rect().height()
			); */
		if (m->rect().contains( QPoint( x, y) )) {
	/*	if (x>=m->_x() && x<m->_x() + m->getWidth() &&
		    y>=m->_y() && y<m->_y() + m->getHeight() ) { */
			unSelectFix();
		
			if ((e->state()&ControlButton) && order->mobSelected.find(mobIt.currentKey()))
				unSelectMob(mobIt.currentKey());
			else
				order->selectMob(mobIt.currentKey(), m);
//				order->mobSelected.insert(mobIt.currentKey(), m);
//			logf(LOG_INFO, "select mobile");
			found = TRUE;
			}
		}

	if (!found) {
	// Here, we have to draw a "selection box"...
		order->setSelectionMode( SELECT_RECT);
		oldX = selectX = e->x();
		oldY = selectY = e->y();
		unSelectFix();
		}
	}
}

void fieldMap::mouseMoveEvent(QMouseEvent *e)
{
QPainter p;
QPen pen(green, 2);

if (SELECT_RECT != order->getSelectionMode()) return;

p.begin(this);
p.setPen(pen);
p.setRasterOp(XorROP);
/* erase previous rect */
if (oldX != selectX && oldY != selectY)	
	drawRectSelect(selectX, selectY, oldX, oldY, p);
/* draw present rect */
oldX = e->x();
oldY = e->y();
if (oldX != selectX && oldY != selectY)	
	drawRectSelect(selectX, selectY, oldX, oldY, p);
p.end();
}

void fieldMap::mouseReleaseEvent(QMouseEvent *e)
{
QPainter p;
QPen pen(green, 2);
QIntDictIterator<playerMobUnit>	mobIt(view->phys->mobile);
QIntDictIterator<playerFacility> fixIt(view->phys->facility);
playerMobUnit	*m;
int		t;

if (SELECT_RECT != order->getSelectionMode()) return;

p.begin(this);
p.setPen(pen);
p.setRasterOp(XorROP);
/* erase rect */
if (oldX != selectX && oldY != selectY)	
	drawRectSelect(selectX, selectY, oldX, oldY, p);
p.end();
order->setSelectionMode( SELECT_NONE);

/* generate multiple selection */

if (selectX > oldX) {
	t = oldX; 
	oldX = selectX;
	selectX = t;
	}

if (selectY > oldY) {
	t = oldY; 
	oldY = selectY;
	selectY = t;
	}

selectX += BO_TILE_SIZE * view->X();
selectY += BO_TILE_SIZE * view->Y();
oldX += BO_TILE_SIZE * view->X();
oldY += BO_TILE_SIZE *view->Y();
	
for (mobIt.toFirst(); mobIt; ++mobIt) {
	m = mobIt.current();
	if (selectX<=m->_x() && oldX>m->_x() + m->getWidth() &&
	    selectY<=m->_y() && oldY>m->_y() + m->getHeight() && !order->mobSelected.find(mobIt.currentKey()) ) {
//		order->mobSelected.insert(mobIt.currentKey(), m);
		order->selectMob(mobIt.currentKey(), m);
		}
	}

selectX -= BO_TILE_SIZE * view->X();
selectY -= BO_TILE_SIZE * view->Y();
oldX -= BO_TILE_SIZE * view->X();
oldY -= BO_TILE_SIZE * view->Y();
repaint (selectX, selectY, oldX, oldY, FALSE);
}

void fieldMap::resizeEvent(QResizeEvent *e)
{
emit reSizeView((width()+BO_TILE_SIZE+1)/BO_TILE_SIZE, (height()+BO_TILE_SIZE+1)/BO_TILE_SIZE);
}




void fieldMap::unSelectAll(void)
{
QIntDictIterator<playerMobUnit> selIt(order->mobSelected);


/* deal with fix */
unSelectFix();

/* deal with mobiles */
for (selIt.toFirst(); selIt;) { // ++ not needed, selIt should be increased
	selIt.current()->unSelect();
	unSelectMob(selIt.currentKey()); // by the .remove() in unselect
	}
boAssert(order->mobSelected.isEmpty());
if (!order->mobSelected.isEmpty()) order->mobSelected.clear();

order->unSelectAll();
//logf(LOG_INFO, "deselecting all");
}


void fieldMap::unSelectFix(void)
{
	order->unSelectFix();
}

void fieldMap::unSelectMob(long key)
{
	order->unSelectMob(key);
}
