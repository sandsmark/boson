/***************************************************************************
                          visualCanvas.cpp  -  description                              
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
#include "common/boconfig.h"
#include "common/bomap.h"

#include "visual.h"
#include "sprites.h"
  

visualCanvas::visualCanvas( QPixmap p, uint w, uint h)
	: QCanvas ( p, w, h, BO_TILE_SIZE, BO_TILE_SIZE)
{
	/* map geometry */
	maxX = w; maxY = h;
	_pm = p;
}



visualCanvas::visualCanvas(void)
	: QCanvas ()
{
	/* map geometry */
	maxX = 0; maxY = 0;
}
	
	

void visualCanvas::resize (int w, int h)
{

	/* map geometry */
	maxX = w; maxY = h;
	QCanvas::resize(w * BO_TILE_SIZE ,h * BO_TILE_SIZE);
	QCanvas::setTiles( _pm, w, h, BO_TILE_SIZE, BO_TILE_SIZE);
	logf(LOG_INFO, "visualCanvas::resize to %d, %d", w, h);
}


void visualCanvas::setCell(int i, int j, cell_t c)
{
	boAssert(i>=0);
	boAssert(j>=0);
	boAssert(i< tilesHorizontally() );
	boAssert(j< tilesVertically() );

//	printf("setCell :  i,j,c = %d,%d,%d\n", i, j, c); fflush(stdout);

	setTile( i, j, c);

	///orzel : XXX this is the bottleneck in editor::New
	emit newCell(i,j, ground(c));
}



QCanvasItem * visualCanvas::findUnitAt(QPoint p)
{
	QCanvasItemList list = collisions( p );
	QCanvasItemList::Iterator it;

	for( it = list.begin(); it != list.end(); ++it )
		if ( IS_UNIT( (*it)->rtti() ) )
			return (*it);
	return NULL;
}

