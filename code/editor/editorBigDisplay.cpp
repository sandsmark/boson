/***************************************************************************
                          editorBigDisplay.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Tue Sep 21 01:18:00 CET 1999
                                           
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

#include <stdlib.h>  // random

#include "common/log.h"
#include "common/bomap.h"

#include "editorBigDisplay.h"
#include "editorCanvas.h"
#include "game.h"

editorBigDisplay::editorBigDisplay(editorTopLevel *v, QWidget *p, const char *n, WFlags f)
	:visualBigDisplay(v,p,n,f)
{

	c = makeCell(GROUND_UNKNOWN, 0);
	otype = OT_NONE;

	setWho(0u);
}

void editorBigDisplay::actionClicked(QPoint mp, int state)
{
	int	x		= mp.x() / BO_TILE_SIZE,
		y		= mp.y() / BO_TILE_SIZE;


		//XX is valid in visual, before calling actionClicked
	if ( x<0 || y<0 || x>= ecanvas->maxX || y>=ecanvas->maxY ) {
//		logf(LOG_ERROR, "actionClicked with x,y = %d,%d, aborting", x, y);
		return;
	}
	
	switch (otype){
		default:
			return; // nothing is selected
			break;

		case OT_GROUND:
			if ( IS_BIG_TRANS(ground(c)) )
			       if ( x+1>=ecanvas->maxX || y+1>=ecanvas->maxY) return;

			if ( IS_PLAIN(ground(c)) && (state&ShiftButton) ) {
				int i,j;
				for (i=-2; i< 3; i++)
					if (x+i>=0 && x+i<ecanvas->maxX)
						for (j=-2; j< 3; j++)
							if (y+j>=0 && y+j<ecanvas->maxY) { // XXXX   !vcanvas->isValid()
								setVersion(c, random()%4 );
								ecanvas->changeCell( x+i, y+j, c); // some kind of randomness in 'c' here
							}
			} else {
				setVersion(c, random()%4 );
				ecanvas->changeCell( x, y, c); // some kind of randomness in 'c' here
			}
			break;
	}

	ecanvas->update(); // could be smarter.
}


void editorBigDisplay::object_put(QPoint p)
{

	p/= BO_TILE_SIZE;
	p+= vtl->_pos();

	switch(otype) {
		default:
			logf(LOG_ERROR, "object_put : unexpected \"otype\" value");
			return;

		case OT_FACILITY:
			facilityMsg_t	fix;
			fix.who 	= who;
			fix.x		= p.x();
			fix.y		= p.y();
			fix.state	= CONSTRUCTION_STEPS-1;
			fix.type	= f; 
			ecanvas->createFixUnit(fix);
			break;

		case OT_UNIT:
			mobileMsg_t	mob;
			mob.who		= who;
			mob.x		= p.x();
			mob.y		= p.y();
			mob.type	= m;
			ecanvas->createMobUnit(mob);
			break;
	}

	editorTopLevel *etl = (editorTopLevel*)vtl;
	etl->setSelectionMode( visualTopLevel::SELECT_NONE);
	ecanvas->update(); // could be smarter.
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


