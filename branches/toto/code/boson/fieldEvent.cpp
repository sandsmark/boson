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

x = e->x() / BO_TILE_SIZE;
y = e->y() / BO_TILE_SIZE;

if (e->button() & RightButton) {
	emit relativeReCenterView(x,y);
	return;
	}

if (e->button() & LeftButton) {	
	/* Here we transpose coo into the map referential */
	x += view->X(); y += view->Y();


	if (SELECT_MOVE == order->getSelectionMode()) {
		order->leftClicked(
			e->x() + BO_TILE_SIZE * view->X()  ,
			e->y() + BO_TILE_SIZE * view->Y()  );
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

	for (fixIt.toFirst(); fixIt; ++fixIt) {
		f = fixIt.current();
//printf("\n\tFix at (%d,%d)+(%d,%d)", f->x, f->y, f->getWidth(), f->getHeight());
	/*	if (x>=f->_x() && x<f->_x() + f->getWidth() &&
		    y>=f->_y() && y<f->_y() + f->getHeight() ) { */
		if (f->rect().contains( QPoint( x, y) )) {
			unSelectAll();
			order->selectFix(f);
			found = TRUE;
			break;
			}
		}

	/* Attention : Referential geometry is changed here !! (expanded) */
	x = e->x() + BO_TILE_SIZE * view->X();
	y = e->y() + BO_TILE_SIZE * view->Y();
	for (mobIt.toFirst(); mobIt; ++mobIt) {
		m = mobIt.current();
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

	if (found) drawSelected();
	else {
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
for (selIt.toFirst(); selIt;) // ++ not needed, selIt should be increased
	unSelectMob(selIt.currentKey()); // by the .remove() in unselect
boAssert(order->mobSelected.isEmpty());
if (!order->mobSelected.isEmpty()) order->mobSelected.clear();

order->unSelectAll();
//logf(LOG_INFO, "deselecting all");
}


void fieldMap::unSelectFix(void)
{
playerFacility *f = order->unSelectFix();
if (!f) return; // already done

repaint(
	(f->_x()-view->X()) * BO_TILE_SIZE - BO_SELECT_MARGIN,
	(f->_y()-view->Y()) * BO_TILE_SIZE - BO_SELECT_MARGIN,
	f->getWidth()* BO_TILE_SIZE + BO_SELECT_MARGIN + BO_SELECT_MARGIN,
	f->getHeight()* BO_TILE_SIZE + BO_SELECT_MARGIN + BO_SELECT_MARGIN,
	FALSE);
}

void fieldMap::unSelectMob(long key)
{
playerMobUnit *m = order->mobSelected[key];

order->unSelectMob(key);
//mobSelected.remove(key);

repaint(
	m->_x() - view->X() * BO_TILE_SIZE - BO_SELECT_MARGIN,
	m->_y() - view->Y() * BO_TILE_SIZE - BO_SELECT_MARGIN,
	m->getWidth() + BO_SELECT_MARGIN + BO_SELECT_MARGIN,
	m->getHeight() + BO_SELECT_MARGIN + BO_SELECT_MARGIN,
	FALSE);
}
