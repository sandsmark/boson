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

#include "../common/log.h"
#include "../common/map.h"

#include "bosonView.h"
#include "visualBigDisplay.h"
//#include "orderWin.h"
#include "bosonField.h"


#define mobileList (((bosonField*)(view->field))->mobile)
#define facilityList (((bosonField*)(view->field))->facility)

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
		//orzel : ugly fix..
		((bosonView*)view)->leftClicked( x, y);
		return;
		}
	
}


