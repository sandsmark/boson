/***************************************************************************
                          visualBigDisplay.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
    copyright            : (C) 1999 by Thomas Capricelli                         
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

#include <assert.h>

#include <kapp.h>

#include "common/log.h"
#include "common/bomap.h"

#include "visualTopLevel.h"
#include "visualBigDisplay.h"
#include "speciesTheme.h"
  

visualBigDisplay::visualBigDisplay(/*orderWin *o,*/ visualTopLevel *v, QWidget*parent, const char *name, WFlags f)
	: QCanvasView(vcanvas,parent,name,f)
	, vtl(v)
{

//setBackgroundColor(black);
//setBackgroundMode(fixedColor);

// QScrollView stuff 
setResizePolicy(QScrollView::AutoOne);
setVScrollBarMode( AlwaysOff);
setHScrollBarMode( AlwaysOff);

connect(this, SIGNAL(relativeReCenterView(int, int)), vtl, SLOT(relativeReCenterView(int, int)));
connect(this, SIGNAL(reSizeView(int, int)), vtl, SLOT(reSizeView(int, int)));

}

visualBigDisplay::~visualBigDisplay()
{
}


void visualBigDisplay::viewportMouseMoveEvent(QMouseEvent *e)
{
	QPainter p;
	QPen pen(green, 2);
	
	switch( vtl->getSelectionMode()) {
		default:
			logf(LOG_WARNING, "visualBigDisplay::viewportMouseMoveEvent : unknown selectionMode(1), mode is %d", vtl->getSelectionMode());
		case visualTopLevel::SELECT_NONE:
			break;
		case visualTopLevel::SELECT_RECT:
			p.begin( viewport() );
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
			break;

		case visualTopLevel::SELECT_FILL:
			selectX =  e->x() + vtl->X()*BO_TILE_SIZE;
			selectY =  e->y() + vtl->Y()*BO_TILE_SIZE;
			if (oldX==selectX && oldY==selectY)
				return;
			oldX = selectX; oldY = selectY;
			actionClicked( oldX, oldY, e->state());
			break;

		case visualTopLevel::SELECT_PUT:
			break;
	}
}

void visualBigDisplay::viewportMouseReleaseEvent(QMouseEvent *e)
{
	QPainter p;
	QPen pen(green, 2);

	switch( vtl->getSelectionMode()) {
		default:
			logf(LOG_WARNING, "visualBigDisplay::viewportMouseReleaseEvent : unknown selectionMode(2), mode is %d", vtl->getSelectionMode());
		case visualTopLevel::SELECT_NONE:
		case visualTopLevel::SELECT_FILL:
			break;
		case visualTopLevel::SELECT_RECT:
			p.begin(this);
			p.setPen(pen);
			p.setRasterOp(XorROP);
			/* erase rect */
			if (oldX != selectX && oldY != selectY)	
				drawRectSelect(selectX, selectY, oldX, oldY, p);
			p.end();
		
			/* generate multiple selection */
			selectX	+= BO_TILE_SIZE * vtl->X();
			selectY	+= BO_TILE_SIZE * vtl->Y();
			oldX	+= BO_TILE_SIZE * vtl->X();
			oldY	+= BO_TILE_SIZE * vtl->Y();
			
			vtl->selectArea(selectX, selectY, oldX, oldY);
			vcanvas->update();

			break;
		case visualTopLevel::SELECT_PUT:
			break;
			return;
	}
	vtl->setSelectionMode( visualTopLevel::SELECT_NONE);
}

void visualBigDisplay::resizeEvent(QResizeEvent *e)
{
	QCanvasView::resizeEvent(e);

	emit reSizeView (	(width()+BO_TILE_SIZE-1)/BO_TILE_SIZE,
				(height()+BO_TILE_SIZE-1)/BO_TILE_SIZE  );
}


/*
void visualBigDisplay::putSomething(void)
{
	vtl->setSelectionMode( visualTopLevel::SELECT_PUT);
	oldX = selectX = 0;
	oldY = selectY = 0;
	vtl->unSelectFix();
	return;
}
*/


void visualBigDisplay::viewportMousePressEvent(QMouseEvent *e)
{
	int x, y;
	
	x = e->x();
	y = e->y();
	
	if (e->button() & MidButton) {
		emit relativeReCenterView( x/BO_TILE_SIZE , y/BO_TILE_SIZE);
		return;
		}

	/* Now we transpose coo into the map referential */
	x += vtl->X()*BO_TILE_SIZE; y += vtl->Y()*BO_TILE_SIZE;
	
	if (e->button() & LeftButton) {	

		if (vtl->getSelectionMode() == visualTopLevel::SELECT_PUT) {
			vtl->object_put(e->x(), e->y());
			return;
		}

		/* Control -> multiselection, else... */
		if (! (e->state()&ControlButton)) {
			vtl->unSelectAll();
			}
	
		QCanvasItem *sfg = vcanvas->findUnitAt( x, y);

		if (!sfg) {
			// nothing has been found : it's a ground-click
			// Here, we have to draw a "selection box"...
			vtl->setSelectionMode( visualTopLevel::SELECT_RECT);
			oldX = selectX = e->x();
			oldY = selectY = e->y();
			vtl->unSelectFix();
			return;
		}
	
	
		if ( IS_MOBILE(sfg->rtti())) {
			visualMobUnit *m = (visualMobUnit *) sfg;
	
			vtl->unSelectFix();
			if ((e->state()&ControlButton) && vtl->mobSelected.find(m->key))
				vtl->unSelectMob(m->key);
			else
				vtl->selectMob(m->key, m);

			vcanvas->update();
			return;
		}

		if ( IS_FACILITY(sfg->rtti())) {
			visualFacility *f = (visualFacility *) sfg;
			vtl->unSelectAll();		// anyway 
			vtl->selectFix(f);

			vcanvas->update();
			return;
		}

		// should never be reached !
		logf(LOG_ERROR, "visual/fieldEvent.c, unexpeted vcanvas->findUnitAt() result");
	
	} // LeftButton 

	if (e->button() & RightButton) {
		actionClicked( x, y, e->state());
		oldX = x; oldY = y;
		return;
		}
	
}


