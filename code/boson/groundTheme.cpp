/***************************************************************************
                          groundTheme.cpp  -  description                              
                             -------------------                                         

    version              :                                   
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

#include <qpixmap.h>
#include <qprogressdialog.h>

#include <QwSpriteField.h>

#include <kapp.h>

#include "../common/log.h"
#include "../map/map.h"
#include "groundTheme.h"


#define PROGRESS_N (groundPropNb + TILES_PER_TRANSITION * groundTransPropNb)

groundTheme::groundTheme(char *themeName)
{
int i ; 
QProgressDialog progress("Loading ground theme...", 0, PROGRESS_N, NULL, "progress.groundTheme", true);
QString path(kapp->kde_datadir() + "/boson/pics/groundThemes/" + themeName + "/" );

allLoaded = true;
progress.setProgress(0);
progress.show();
for (i=0; i< groundPropNb; i++)
	if (!loadGround(i, path + groundProp[i].name, progress))
		allLoaded = false;

for (i=0; i< groundTransPropNb; i++) {

	assert(groundTransProp[i].from>=0);
	assert(groundTransProp[i].from<GROUND_LAST);
	assert(groundTransProp[i].to>=0);
	assert(groundTransProp[i].to<GROUND_LAST);

	if (!loadTransition(GROUND_LAST + i * TILES_PER_TRANSITION ,
		path + groundProp[groundTransProp[i].from].name +
		"_"  + groundProp[groundTransProp[i].to].name,
		progress ))
	     allLoaded = false;
	}

no_pixmap = new QPixmap(BO_TILE_SIZE,BO_TILE_SIZE);
no_pixmap->fill(black);

progress.setProgress(PROGRESS_N);

if (!allLoaded) puts("groundTheme : not all loaded !");
	else printf(	"\ngroundTheme : %d ground tiles loaded\n"
			"              %d transition tiles loaded\n",
	groundPropNb, groundTransPropNb);

}

bool groundTheme::loadGround(int i, const QString &path, QProgressDialog &progress)
{
  	groundPix[i] = new QwSpritePixmapSequence(path+".bmp",0);
/*	boAssert(BO_TILE_SIZE == pixmap[i]->width());
	boAssert(BO_TILE_SIZE == pixmap[i]->height()); */
///orzel : do some boAssert with QwSpritePixmapSequence sizes... 
	progress.setProgress(i);
	if (groundPix[i]->image(0)->isNull()) return false;
	return true;
}

bool groundTheme::loadTransition(int i, const QString &path, QProgressDialog &progress)
{
	int j;
	static const char *trans_ext[TILES_PER_TRANSITION] = {
		"_ul", "_ur", "_dl", "_dr", 
		"_up", "_down", "_left", "_right", 
		"_uli", "_uri", "_dli", "_dri", 
		};

	for (j=0; j<12; j++)
		if (!loadGround(i+j, path + trans_ext[j], progress ))
			return false;
	return true;
}

groundTheme::~groundTheme()
{
}
