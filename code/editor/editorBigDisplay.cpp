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

#include "editorBigDisplay.h"
#include "visualView.h"
#include "visualCell.h"
#include "editorField.h"

editorBigDisplay::editorBigDisplay(visualView *v, QWidget *p, const char *n, WFlags f)
	:visualBigDisplay(v,p,n,f)
{

	g = GROUND_UNKNOWN;
}

void editorBigDisplay::actionClicked(int mx, int my)
{
	editorField *field	= (((editorField*)(view->field)));
	int	x		= mx / BO_TILE_SIZE,
		y		= my / BO_TILE_SIZE;


	boAssert(x>=0);
	boAssert(x<field->width());
	boAssert(y>=0);
	boAssert(y<field->height());

	if (GROUND_UNKNOWN == g) return; // no ground is selected

	if (IS_BIG_TRANS(g) )
	       if ( x+1>=field->maxX || y+1>=field->maxY) return;

	
	field->deleteCell(x,y);
	field->setCell(x,y,g);

	view->field->update();
}


