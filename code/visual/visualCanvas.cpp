/***************************************************************************
                          visualCanvas.cpp  -  description                              
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
//#include <kmessagebox.h>

#include "common/log.h"
#include "common/boconfig.h"
#include "common/map.h"

#include "speciesTheme.h"
#include "groundTheme.h"
#include "visual.h"
  

visualCanvas::visualCanvas(uint w, uint h)
	: QCanvas (w * BO_TILE_SIZE ,h * BO_TILE_SIZE)
{
	/* map geometry */
	maxX = w; maxY = h;

	init();
}



visualCanvas::visualCanvas(void)
	: QCanvas ()
{
	/* map geometry */
	maxX = 0; maxY = 0;

	init();
}
	
	
	
void visualCanvas::init(void)
{


/* Themes selection (should be moved thereafter) */
ground	= new groundTheme("earth");
/*
	if (!ground->isOk()) KMsgBox::message(0l,
		i18n("Pixmap loading error"),
		i18n("Error while loading groundTheme,\nsome images will show up awfully"),
		KMsgBox::EXCLAMATION);
*/

species[1]	= new speciesTheme("human", qRgb(0, 0, 255) );
/*
	if (!species[1]->isOk()) KMsgBox::message(0l,
		i18n("Pixmap loading error"),
		i18n("Error while loading \"blue\" specie theme,\nsome images will show up awfully"),
		KMsgBox::EXCLAMATION);
*/

species[0]	= new speciesTheme("human", qRgb( 0, 255, 0) );
/*
	if (!species[0]->isOk()) KMsgBox::message(0l,
		i18n("Pixmap loading error"),
		i18n("Error while loading \"red\" specie theme,\nsome images will show up awfully"),
		KMsgBox::EXCLAMATION);
*/


}


void visualCanvas::resize (int w, int h)
{

	/* map geometry */
	maxX = w; maxY = h;
	QCanvas::resize(w * BO_TILE_SIZE ,h * BO_TILE_SIZE);
}


void visualCanvas::setCell(int i, int j, groundType g)
{
	boAssert(i>=0); boAssert(j>=0);
	boAssert(i<width()); boAssert(j<height());

//	(void) new visualCell(g, i, j);
//	printf("setCell bypassed, g,i,j = %d,%d,%d \n", g, i, j);
	//	XXXX

	emit newCell(i,j,g);
}



QCanvasItem * visualCanvas::findUnitAt(int x, int y)
{
	QCanvasItem *u;

	/* XXXX 
	for( p = topAt(x,y); p; next(p))
		if (IS_UNIT(at(p)->rtti()) && exact(p))  {
			u =  at(p);
			end(p);
			return u;
		}

	*/
	return NULL;
}


groundType visualCanvas::findGroundAt(int x, int y)
{
	groundType g;
 
	/* XXXX
        for( p = topAt(x,y); p; next(p))
                if (IS_GROUND(at(p)->rtti()) && exact(p))  {
			g = ((visualCell*)at(p))->getGroundType();
//			printf("rtti: %d, rtti-S_GROUND : %d g : %d\n", at(p)->rtti(), at(p)->rtti()- S_GROUND,  g);
                        end(p);
			return g;
		}
	logf(LOG_ERROR, "can't find ground in visualCanvas::findGroundAt");
	*/
	return GROUND_UNKNOWN;
}


