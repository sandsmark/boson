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

#include <assert.h>

#include <qpixmap.h>
#include <qbitarray.h>

#include <QwSpriteField.h>

#include <kapp.h>

#include "../common/log.h"
#include "../common/map.h"

#include "groundTheme.h"


groundTheme::groundTheme(char *themeName)
{
	int i ; 

	assert (GROUND_LAST == groundPropNb);
	assert (TRANS_LAST  == groundTransPropNb);

	groundPix	= new (QwSpritePixmapSequence *)[NB_GROUND_TILES];
	themePath	= new QString(kapp->kde_datadir() + "/boson/themes/grounds/" + themeName + "/" );
	transitions	= new QBitArray(TRANS_LAST);

	boAssert ( transitions->fill(false) );

	for (i=0; i< groundPropNb; i++)
		loadGround(i, *themePath + groundProp[i].name);

}

bool groundTheme::loadGround(int i, const QString &path)
{
  	groundPix[i] = new QwSpritePixmapSequence(path+".%.2d.bmp",0l, 4);
/*	boAssert(BO_TILE_SIZE == pixmap[i]->width());
	boAssert(BO_TILE_SIZE == pixmap[i]->height()); */
///orzel : do some boAssert with QwSpritePixmapSequence sizes... 
	if (groundPix[i]->image(0)->isNull()) {
		fprintf(stderr, "groundTheme : Can't load %s.XX.bmp ...\n", (const char *)path);
		return false;
	}
	return true;
}

bool groundTheme::loadTransition(int ref)
{
	int i, j;
	bool ret = true;
	static const char *trans_ext[TILES_PER_TRANSITION] = {
		".01", ".03", ".07", ".05",	// 48x48 transitions
		".02", ".06", ".08", ".04",
		".09", ".10", ".12", ".11",
		".13", ".14", ".15", ".16",	// 96x96 transitions
		".17", ".18", ".19", ".20",
		".21", ".22", ".23", ".24",
		".25", ".26", ".27", ".28",
		};
	QString transS;
	QString	path(*themePath);

	i = GROUND_LAST + ref * TILES_PER_TRANSITION;

	transS	 = groundProp[groundTransProp[ref].from].name;
	transS	+= "_";
	transS	+= groundProp[groundTransProp[ref].to].name;

	path = *themePath + transS + "/" + transS;

	for (j=0; j<TILES_PER_TRANSITION; j++)
		if (!loadGround(i+j, path + trans_ext[j] ))
			ret = false;

	if (ret) transitions->setBit(ref);
	return ret;
}


QwSpritePixmapSequence *groundTheme::getPixmap(groundType gt)
{

	if (IS_TRANS(gt)) {
		int ref = GET_TRANS_REF(gt);
		if (!transitions->testBit(ref))
			loadTransition(ref);
	
		boAssert(transitions->testBit(ref));
	}

	return groundPix[gt];
}


