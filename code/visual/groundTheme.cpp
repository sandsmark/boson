/***************************************************************************
                          groundTheme.cpp  -  description                              
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

#include <qbitarray.h>

#include <QwSpriteField.h>


#include <kapp.h>		// kde_datadir()

#include "common/log.h"
#include "groundTheme.h"


groundTheme::groundTheme(char *themeName)
{
	int i ; 
	QString transS;

	boAssert (GROUND_LAST == groundPropNb);
	boAssert (TRANS_LAST  == groundTransPropNb);

	groundPix	= new (QwSpritePixmapSequence *)[NB_GROUND_TILES];
	themePath	= new QString(kapp->kde_datadir() + "/boson/themes/grounds/" + themeName + "/" );
	pixLoaded	= new QBitArray(NB_GROUND_TILES);

	for (i=0; i< TRANS_LAST; i++) { // pre-load transitions name
		transS	 = groundProp[groundTransProp[i].from].name;
		transS	+= "_";
		transS	+= groundProp[groundTransProp[i].to].name;
		transName[i] = *themePath +  transS + "/" + transS;
	}

	boAssert ( pixLoaded->fill(false) );

	for (i=0; i< groundPropNb; i++) // load non-transitions
		loadGround(i, *themePath + groundProp[i].name);

}

groundTheme::~groundTheme()
{
	unsigned int i,n;
	
	for (i=0, n=NB_GROUND_TILES; i<n; i++)
		if (pixLoaded[i])
			delete groundPix[i];

	delete groundPix;
	delete themePath;
	delete pixLoaded;

}

void groundTheme::loadGround(int i, const QString &path)
{
  	groundPix[i] = new QwSpritePixmapSequence(path+".%.2d.bmp",0l, 4);
/*	boAssert(BO_TILE_SIZE == pixmap[i]->width());
	boAssert(BO_TILE_SIZE == pixmap[i]->height()); */
/// XXX orzel : do some boAssert with QwSpritePixmapSequence sizes... 
	if (groundPix[i]->image(0)->isNull()) {
		logf(LOG_ERROR, "groundTheme : Can't load %s.XX.bmp ...", (const char *)path);
	}
	pixLoaded->setBit(i);
}

void groundTheme::loadTransition(groundType gt)
{
	int ref = GET_TRANS_REF(gt);
	int tile = GET_TRANS_TILE(gt);

	static const char *trans_ext[TILES_PER_TRANSITION] = {
		".01", ".03", ".07", ".05",	// 48x48 transitions
		".02", ".06", ".08", ".04",
		".09", ".10", ".12", ".11",
		".13", ".14", ".15", ".16",	// 96x96 transitions
		".17", ".18", ".19", ".20",
		".21", ".22", ".23", ".24",
		".25", ".26", ".27", ".28",
		};

	boAssert(IS_TRANS(gt));
	loadGround(gt, transName[ref] + trans_ext[tile]);
}


QwSpritePixmapSequence *groundTheme::getPixmap(groundType gt)
{

	if (!pixLoaded->testBit(gt))
			loadTransition(gt);

	return groundPix[gt];
}


