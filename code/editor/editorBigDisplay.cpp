/***************************************************************************
                          editorBigDisplay.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Tue Sep 21 01:18:00 CET 1999
                                           
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

#include <stdlib.h>  // random

#include "common/log.h"
#include "common/bomap.h"

#include "editorBigDisplay.h"
#include "editorCanvas.h"

editorBigDisplay::editorBigDisplay(editorTopLevel *v, QWidget *p, const char *n, WFlags f)
	:visualBigDisplay(v,p,n,f)
{

	c = makeCell(GROUND_UNKNOWN, 0);
	otype = OT_NONE;

	setWho(0);
}

void editorBigDisplay::actionClicked(int mx, int my, int state)
{
	editorCanvas *_canvas	= (((editorCanvas*)vcanvas));
	int	x		= mx / BO_TILE_SIZE,
		y		= my / BO_TILE_SIZE;


	if ( x<0 || y<0 || x>= _canvas->maxX || y>=_canvas->maxY ) {
		logf(LOG_ERROR, "actionClicked with x,y = %d,%d, aborting", x, y);
		return;
	}
	
	switch (otype){
		case OT_NONE:
			return; // nothing is selected
			break;

		case OT_GROUND:
			if ( IS_BIG_TRANS(ground(c)) )
			       if ( x+1>=_canvas->maxX || y+1>=_canvas->maxY) return;

			if ( IS_PLAIN(ground(c)) && (state&ShiftButton) ) {
				int i,j;
				for (i=-2; i< 3; i++)
					if (x+i>=0 && x+i<_canvas->maxX)
						for (j=-2; j< 3; j++)
							if (y+j>=0 && y+j<_canvas->maxY) {
								setVersion(c, random()%4 );
								_canvas->changeCell( x+i, y+j, c); // some kind of randomness in 'c' here
							}
			} else {
				setVersion(c, random()%4 );
				_canvas->changeCell( x, y, c); // some kind of randomness in 'c' here
			}

			vtl->setSelectionMode( editorTopLevel::SELECT_FILL);
			break;

		case OT_FACILITY:
			facilityMsg_t	fix;
			fix.who 	= who;
			fix.x		= x;
			fix.y		= y;
			fix.state	= CONSTRUCTION_STEPS-1;
			fix.type	= f; 
			_canvas->createFixUnit(fix);
			vtl->setSelectionMode( editorTopLevel::SELECT_PUT);
			break;

		case OT_UNIT:
			mobileMsg_t	mob;
			mob.who		= who;
			mob.x		= x;
			mob.y		= y;
			mob.type	= m;
			_canvas->createMobUnit(mob);
			vtl->setSelectionMode( editorTopLevel::SELECT_PUT);
			break;
	}

	_canvas->update(); // could be smarter.
}

void editorBigDisplay::setSelectedObject(object_type t, int n)
{
	otype = t;
	switch (t){
		default:
		case OT_NONE:
			break;
		case OT_GROUND:
			c = makeCell ( (groundType) n, 0);
			break;
		case OT_FACILITY:
			f = (facilityType)n;
			break;
		case OT_UNIT:
			m = (mobType)n;
			break;
	}
}


