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
#include "../common/map.h"

#include "visualView.h"
#include "editorBigDisplay.h"
//#include "orderWin.h"
#include "editorField.h"

// orzel : quite ugly ....
#define mobileList	(((editorField*)(view->field))->mobile)
#define facilityList	(((editorField*)(view->field))->facility)
#define eMap		(((editorField*)(view->field)))

/* ugly orzel hack......*/
void visualBigDisplay::mousePressEvent(QMouseEvent *e)
{
}


void editorBigDisplay::mousePressEvent(QMouseEvent *e)
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
		/* Control -> multiselection, else... */
		if (! (e->state()&ControlButton)) {
			unSelectAll();
			}
	
		QwSpriteFieldGraphic *sfg = view->field->findUnitAt( x, y);

		if (!sfg) {
			// nothing has been found : it's a ground-click
			// Here, we have to draw a "selection box"...
			view->setSelectionMode( SELECT_RECT);
			oldX = selectX = e->x();
			oldY = selectY = e->y();
			unSelectFix();
			return;
		}
	
	
		if ( IS_MOBILE(sfg->rtti())) {
			visualMobUnit *m = (visualMobUnit *) sfg;
	
			unSelectFix();
			if ((e->state()&ControlButton) && view->mobSelected.find(m->key))
				unSelectMob(m->key);
			else
				view->selectMob(m->key, m);

			return;
		}

		if ( IS_FACILITY(sfg->rtti())) {
			visualFacility *f = (visualFacility *) sfg;
			unSelectAll();		// anyway 
			view->selectFix(f);

			return;
		}

		// should never be reached !
		logf(LOG_ERROR, "boson/fieldEvent.c, unexpeted field->findUnitAt() result");
	
	} // LeftButton 


	if (e->button() & RightButton) {
		x = e->x() / BO_TILE_SIZE; y = e->y() / BO_TILE_SIZE;
		x += view->X(); y += view->Y();
		boAssert(x>0);
//		boAssert(x<eMap->width);
		boAssert(y>0);
//		boAssert(y<eMap->height);
		selectedCell = &(eMap->cells[x][y]);
		popup->exec(QCursor::pos());
		return;
	}
	
}


