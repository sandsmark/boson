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

#include "../common/log.h"
#include "../common/map.h"

#include "visualBigDisplay.h"
#include "visualCell.h"
#include "speciesTheme.h"
#include "groundTheme.h"
  

visualBigDisplay::visualBigDisplay(/*orderWin *o,*/ visualView *v, QWidget*parent, const char *name, WFlags f)
	: QWidget(parent, name, f)
	, QwAbsSpriteFieldView(v->field)
{

//setBackgroundColor(black);
//setBackgroundMode(fixedColor);

/* related orderWindows */
//order = o;

/* the visualView */
view = v;

// connect(, SIGNAL(), this, SLOT());
connect(view, SIGNAL(repaint(bool)), this, SLOT(repaint(bool)));
connect(this, SIGNAL(relativeReCenterView(int, int)), view, SLOT(relativeReCenterView(int, int)));
connect(this, SIGNAL(reSizeView(int, int)), view, SLOT(reSizeView(int, int)));

}

visualBigDisplay::~visualBigDisplay()
{
	QwAbsSpriteFieldView::view(0);
}


QRect visualBigDisplay::viewArea() const
{
//printf("visualBigDisplay::viewArea = %d.%d, %dx%d\n", BO_TILE_SIZE * view->X(), BO_TILE_SIZE * view->Y(), width(), height());
return QRect(BO_TILE_SIZE * view->X(), BO_TILE_SIZE * view->Y(), width(), height());
boAssert(width() / BO_TILE_SIZE == view->L());
boAssert(height() / BO_TILE_SIZE == view->H());
}

void visualBigDisplay::flush(const  QRect & area)
{
/* nothing special.. */
///orzel : to change is some kind of off-screen buffering is used
}

void visualBigDisplay::beginPainter (QPainter &p)
{
///orzel : to change if some kind of off-screen buffering is used

p.begin(this);
p.translate( - BO_TILE_SIZE * view->X(), - BO_TILE_SIZE * view->Y());
p.setBackgroundColor(black);

boAssert(p.backgroundColor() == black);

//p.setBackgroundMode(OpaqueMode);
}


void visualBigDisplay::paintEvent(QPaintEvent *evt)
{
	if (viewing) {
		QRect r = evt->rect();
///orzel : should be removed :
		r = rect();
//printf("r = %d.%d, %dx%d\n", r.x(), r.y(), r.width(), r.height());
		r.moveBy(view->X() * BO_TILE_SIZE, view->Y() * BO_TILE_SIZE);
//printf("visualBigDisplay::paintEvents, moved : %d.%d, %dx%d\n", r.x(), r.y(), r.width(), r.height());
		viewing->updateInView(this, r);
	}

}


void visualBigDisplay::mouseMoveEvent(QMouseEvent *e)
{
	QPainter p;
	QPen pen(green, 2);
	
	switch( view->getSelectionMode()) {
		default:
			logf(LOG_WARNING, "visualBigDisplay::mouseMoveEvent : unknown selectionMode(1), mode is %d", view->getSelectionMode());
		case SELECT_NONE:
			break;
		case SELECT_RECT:
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
			break;

		case SELECT_FILL:
			selectX =  e->x() + view->X()*BO_TILE_SIZE;
			selectY =  e->y() + view->Y()*BO_TILE_SIZE;
			if (oldX==selectX && oldY==selectY)
				return;
			oldX = selectX; oldY = selectY;
			actionClicked( oldX, oldY);
			break;

		case SELECT_PUT:
			break;
	}
}

void visualBigDisplay::mouseReleaseEvent(QMouseEvent *e)
{
	QPainter p;
	QPen pen(green, 2);

	switch( view->getSelectionMode()) {
		default:
			logf(LOG_WARNING, "visualBigDisplay::mouseReleaseEvent : unknown selectionMode(2), mode is %d", view->getSelectionMode());
		case SELECT_NONE:
		case SELECT_FILL:
			break;
		case SELECT_RECT:
			p.begin(this);
			p.setPen(pen);
			p.setRasterOp(XorROP);
			/* erase rect */
			if (oldX != selectX && oldY != selectY)	
				drawRectSelect(selectX, selectY, oldX, oldY, p);
			p.end();
		
			/* generate multiple selection */
			selectX	+= BO_TILE_SIZE * view->X();
			selectY	+= BO_TILE_SIZE * view->Y();
			oldX	+= BO_TILE_SIZE * view->X();
			oldY	+= BO_TILE_SIZE * view->Y();
			
			view->selectArea(selectX, selectY, oldX, oldY);
			view->field->update();

			break;
		case SELECT_PUT:
			break;
			return;
	}
	view->setSelectionMode( SELECT_NONE);
}

void visualBigDisplay::resizeEvent(QResizeEvent *e)
{
	emit reSizeView (	(width()+BO_TILE_SIZE-1)/BO_TILE_SIZE,
				(height()+BO_TILE_SIZE-1)/BO_TILE_SIZE  );
}


/*
void visualBigDisplay::putSomething(void)
{
	view->setSelectionMode( SELECT_PUT);
	oldX = selectX = 0;
	oldY = selectY = 0;
	view->unSelectFix();
	return;
}
*/


void visualBigDisplay::mousePressEvent(QMouseEvent *e)
{
	int x, y;
	
	x = e->x();
	y = e->y();
	
	if (e->button() & MidButton) {
		emit relativeReCenterView( x/BO_TILE_SIZE , y/BO_TILE_SIZE);
		return;
		}

	/* Now we transpose coo into the map referential */
	x += view->X()*BO_TILE_SIZE; y += view->Y()*BO_TILE_SIZE;
	
	if (e->button() & LeftButton) {	

		if (view->getSelectionMode() == SELECT_PUT) {
			view->object_put(e->x(), e->y());
			return;
		}

		/* Control -> multiselection, else... */
		if (! (e->state()&ControlButton)) {
			view->unSelectAll();
			}
	
		QwSpriteFieldGraphic *sfg = view->field->findUnitAt( x, y);

		if (!sfg) {
			// nothing has been found : it's a ground-click
			// Here, we have to draw a "selection box"...
			view->setSelectionMode( SELECT_RECT);
			oldX = selectX = e->x();
			oldY = selectY = e->y();
			view->unSelectFix();
			return;
		}
	
	
		if ( IS_MOBILE(sfg->rtti())) {
			visualMobUnit *m = (visualMobUnit *) sfg;
	
			view->unSelectFix();
			if ((e->state()&ControlButton) && view->mobSelected.find(m->key))
				view->unSelectMob(m->key);
			else
				view->selectMob(m->key, m);

			view->field->update();
			return;
		}

		if ( IS_FACILITY(sfg->rtti())) {
			visualFacility *f = (visualFacility *) sfg;
			view->unSelectAll();		// anyway 
			view->selectFix(f);

			view->field->update();
			return;
		}

		// should never be reached !
		logf(LOG_ERROR, "visual/fieldEvent.c, unexpeted field->findUnitAt() result");
	
	} // LeftButton 

	if (e->button() & RightButton) {
		view->setSelectionMode( SELECT_FILL) ;
		actionClicked( x, y);
		oldX = x; oldY = y;
		return;
		}
	
}


