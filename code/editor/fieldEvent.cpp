/***************************************************************************
                          fieldEvent.cpp  -  description                              
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

#include <qpainter.h>
#include <qpopupmenu.h>

#include "../common/log.h"
#include "../map/map.h"

#include "viewMap.h"
#include "editorFieldMap.h"
//#include "orderWin.h"
#include "editorMap.h"

// orzel : quite ugly ....
#define mobileList	(((editorMap*)(view->phys))->mobile)
#define facilityList	(((editorMap*)(view->phys))->facility)
#define eMap		(((editorMap*)(view->phys))->map)

static int selectX, selectY;
static int oldX, oldY;

/* ugly orzel hack......*/
void fieldMap::mousePressEvent(QMouseEvent *e)
{
}


void editorFieldMap::mousePressEvent(QMouseEvent *e)
{
int x, y;

bool found = FALSE;

x = e->x();
y = e->y();

if (e->button() & RightButton) {
	emit relativeReCenterView( x/BO_TILE_SIZE , y/BO_TILE_SIZE);
	return;
}

if (e->button() & LeftButton) {	
	/* Here we transpose coo into the map referential */
	x += view->X()*BO_TILE_SIZE; y += view->Y()*BO_TILE_SIZE;


	if (SELECT_MOVE == view->getSelectionMode()) {
		view->leftClicked( x, y);
		return;
	}

	/* Control -> multiselection, else... */
	if (! (e->state()&ControlButton)) {
		unSelectAll();
	}

	QIntDictIterator<visualMobUnit> mobIt(mobileList);
	QIntDictIterator<visualFacility> fixIt(facilityList);

	visualMobUnit	*m;
	visualFacility	*f;

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
			view->selectFix(f);
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
			unSelectFix();
		
			if ((e->state()&ControlButton) && view->mobSelected.find(mobIt.currentKey()))
				unSelectMob(mobIt.currentKey());
			else
				view->selectMob(mobIt.currentKey(), m);
			found = TRUE;
		}
	}

	if (!found) {
	// Here, we have to draw a "selection box"...
		view->setSelectionMode( SELECT_RECT);
		oldX = selectX = e->x();
		oldY = selectY = e->y();
		unSelectFix();
	}
} /* left button */

if (e->button() & MidButton) {
	x = e->x() / BO_TILE_SIZE; y = e->y() / BO_TILE_SIZE;
	x += view->X(); y += view->Y();
	boAssert(x>0);
	boAssert(x<eMap.width);
	boAssert(y>0);
	boAssert(y<eMap.height);
	selectedCell = &(eMap.cells[x][y]);
	popup->exec(QCursor::pos());
	return;
}
}

void fieldMap::mouseMoveEvent(QMouseEvent *e)
{
QPainter p;
QPen pen(green, 2);

if (SELECT_RECT != view->getSelectionMode()) return;

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
QIntDictIterator<visualMobUnit> mobIt(mobileList);
QIntDictIterator<visualFacility> fixIt(facilityList);
visualMobUnit	*m;
int		t;

if (SELECT_RECT != view->getSelectionMode()) return;

p.begin(this);
p.setPen(pen);
p.setRasterOp(XorROP);
/* erase rect */
if (oldX != selectX && oldY != selectY)	
	drawRectSelect(selectX, selectY, oldX, oldY, p);
p.end();
view->setSelectionMode( SELECT_NONE);

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
	    selectY<=m->_y() && oldY>m->_y() + m->getHeight() && !view->mobSelected.find(mobIt.currentKey()) ) {
		view->selectMob(mobIt.currentKey(), m);
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
QIntDictIterator<visualMobUnit> selIt(view->mobSelected);


/* deal with fix */
unSelectFix();

/* deal with mobiles */
for (selIt.toFirst(); selIt;) { // ++ not needed, selIt should be increased
	selIt.current()->unSelect();
	unSelectMob(selIt.currentKey()); // by the .remove() in unselect
	}
boAssert(view->mobSelected.isEmpty());
if (!view->mobSelected.isEmpty()) view->mobSelected.clear();

view->unSelectAll();
//logf(LOG_INFO, "deselecting all");
}

///orzel : those two should become inlined in .h
void fieldMap::unSelectFix(void)
{
	view->unSelectFix();
}

void fieldMap::unSelectMob(long key)
{
	view->unSelectMob(key);
}
