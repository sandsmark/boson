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

#include "../common/log.h"
#include "../common/map.h"

#include "editorBigDisplay.h"
#include "visualCell.h"
#include "editorField.h"

editorBigDisplay::editorBigDisplay(visualView *v, QWidget *p, const char *n, WFlags f)
	:visualBigDisplay(v,p,n,f)
{

	g = GROUND_UNKNOWN;
	otype = OT_NONE;

	setWho(0);
}

void editorBigDisplay::actionClicked(int mx, int my, int state)
{
	editorField *field	= (((editorField*)(view->field)));
	int	x		= mx / BO_TILE_SIZE,
		y		= my / BO_TILE_SIZE;


	boAssert(x>=0);
	boAssert(x<field->width());
	boAssert(y>=0);
	boAssert(y<field->height());

	
	switch (otype){
		case OT_NONE:
			return; // nothing is selected
			break;

		case OT_GROUND:
			if ( IS_BIG_TRANS(g) )
			       if ( x+1>=field->maxX || y+1>=field->maxY) return;

			if ( IS_PLAIN(g) && (state&ShiftButton) ) {
				int i,j;
				for (i=-2; i< 3; i++)
					if (x+i>=0 && x+i<field->maxX)
						for (j=-2; j< 3; j++)
							if (y+j>=0 && y+j<field->maxY)
								field->changeCell( x+i, y+j, g);
			} else
				field->changeCell( x, y, g);

			view->setSelectionMode( SELECT_FILL);
			break;

		case OT_FACILITY:
			facilityMsg_t	fix;
			fix.who 	= who;
			fix.x		= x;
			fix.y		= y;
			fix.state	= CONSTRUCTION_STEP-1;
			fix.type	= f; 
			field->createFixUnit(fix);
			view->setSelectionMode( SELECT_PUT);
			break;

		case OT_UNIT:
			mobileMsg_t	mob;
			mob.who		= who;
			mob.x		= mx;
			mob.y		= my;
			mob.type	= m;
			field->createMobUnit(mob);
			view->setSelectionMode( SELECT_PUT);
			break;
	}

	view->field->update();
}

void editorBigDisplay::setSelectedObject(object_type t, int n)
{
	otype = t;
	switch (t){
		default:
		case OT_NONE:
			break;
		case OT_GROUND:
			g = (groundType) n;
			break;
		case OT_FACILITY:
			f = (facilityType)n;
			break;
		case OT_UNIT:
			m = (mobType)n;
			break;
	}
}


