/***************************************************************************
                          physMap.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jan  9 19:35:36 CET 1999
                                           
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

#include <assert.h>

#include <kapp.h>
#include <kmsgbox.h>

#include "../common/log.h"
#include "../common/boconfig.h"
#include "../common/map.h"

#include "physMap.h"
#include "speciesTheme.h"
#include "groundTheme.h"
#include "visual.h"
  
physMap::physMap(uint w, uint h, QObject *parent, const char *name=0L)
	: QObject(parent, name)
	, QwSpriteField (w * BO_TILE_SIZE ,h * BO_TILE_SIZE)
{

/* map geometry */
maxX = w; maxY = h;

/* Themes selection (should be moved thereafter) */
vpp.ground	= new groundTheme("earth");
	if (!vpp.ground->isOk()) KMsgBox::message(0l,
		i18n("Pixmap loading error"),
		i18n("Error while loading groundTheme,\nsome images will show up awfully"),
		KMsgBox::EXCLAMATION);

vpp.species[1]	= new speciesTheme("blue_human");
	if (!vpp.species[1]->isOk()) KMsgBox::message(0l,
		i18n("Pixmap loading error"),
		i18n("Error while loading \"blue\" specie theme,\nsome images will show up awfully"),
		KMsgBox::EXCLAMATION);

vpp.species[0]	= new speciesTheme("red_human");
	if (!vpp.species[0]->isOk()) KMsgBox::message(0l,
		i18n("Pixmap loading error"),
		i18n("Error while loading \"red\" specie theme,\nsome images will show up awfully"),
		KMsgBox::EXCLAMATION);


}


void physMap::setCell(int i, int j, groundType g)
{
	boAssert(i>=0); boAssert(j>=0);
	boAssert(i<width()); boAssert(j<height());

	(void) new visualCell(g, i, j);

	emit newCell(i,j,g);
}

