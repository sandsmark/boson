/***************************************************************************
                          visualBigDisplay.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
    copyright            : (C) 1999-2000 by Thomas Capricelli                         
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

	setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding) );

// QScrollView stuff 
setResizePolicy(QScrollView::AutoOne);
setVScrollBarMode( AlwaysOff);
setHScrollBarMode( AlwaysOff);

connect(this, SIGNAL(relativeReCenterView(QPoint)), vtl, SLOT(relativeReCenterView(QPoint)));
connect(this, SIGNAL(reSizeView(QSize)), vtl, SLOT(reSizeView(QSize)));

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
			if (oldPos!=selectPos)
				drawRectSelect(selectPos, oldPos, p);
			/* draw present rect */
			oldPos = QPoint (e->x(), e->y());
			if (oldPos!=selectPos)
				drawRectSelect(selectPos, oldPos, p);
			p.end();
			break;

		case visualTopLevel::SELECT_FILL:
			selectPos = QPoint (e->x(), e->y()) + vtl->_pos() * BO_TILE_SIZE;
			if (oldPos == selectPos)
				return;
			oldPos = selectPos;
			actionClicked( oldPos, e->state());
			break;

		case visualTopLevel::SELECT_PUT:
			break;
	}
}

void visualBigDisplay::viewportMouseReleaseEvent(QMouseEvent *)
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
			if (oldPos!=selectPos)
				drawRectSelect(selectPos, oldPos, p);
			p.end();
		
			/* generate multiple selection */
			vtl->selectArea( QRect(selectPos, oldPos));
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

	emit reSizeView (	QSize((width()+BO_TILE_SIZE-1)/BO_TILE_SIZE,
				(height()+BO_TILE_SIZE-1)/BO_TILE_SIZE ) );
}


/*
void visualBigDisplay::putSomething(void)
{
	vtl->setSelectionMode( visualTopLevel::SELECT_PUT);
	oldPos = selectPos = QPoint(0,0);
	vtl->unSelectFix();
	return;
}
*/


void visualBigDisplay::viewportMousePressEvent(QMouseEvent *e)
{
	QPoint pos(e->x(), e->y());
	
	if (e->button() & MidButton) {
		emit relativeReCenterView( pos/BO_TILE_SIZE );
		return;
		}

	/* Now we transpose coo into the map referential */
	pos += vtl->_pos()*BO_TILE_SIZE;
	
	if (e->button() & LeftButton) {	

		if (vtl->getSelectionMode() == visualTopLevel::SELECT_PUT) {
			vtl->object_put( QPoint(e->x(), e->y()) );
			return;
		}

		/* Control -> multiselection, else... */
		if (! (e->state()&ControlButton)) {
			vtl->unSelectAll();
			}
	
		QCanvasItem *sfg = vcanvas->findUnitAt(pos);

		if (!sfg) {
			// nothing has been found : it's a ground-click
			// Here, we have to draw a "selection box"...
			vtl->setSelectionMode( visualTopLevel::SELECT_RECT);
			selectPos = oldPos = QPoint (e->x(), e->y());
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
		actionClicked( pos, e->state());
		oldPos = pos;
		return;
		}
	
}


