/***************************************************************************
                          groundTheme.cpp  -  description                              
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
QString path(kapp->kde_datadir() + "/boson/themes/grounds/" + themeName + "/" );
QString transS;

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

	transS	= groundProp[groundTransProp[i].from].name;
	transS += "_";
	transS += groundProp[groundTransProp[i].to].name;

	if (!loadTransition(GROUND_LAST + i * TILES_PER_TRANSITION ,
		path + transS + "/" + transS ,
		progress ))
	     allLoaded = false;
	}

progress.setProgress(PROGRESS_N);

if (!allLoaded) logf(LOG_ERROR, "groundTheme : not all loaded !");
	else logf(LOG_INFO, "groundTheme loaded : %d ground tiles, %d transition tiles",
		groundPropNb, groundTransPropNb);

}

bool groundTheme::loadGround(int i, const QString &path, QProgressDialog &progress)
{
  	groundPix[i] = new QwSpritePixmapSequence(path+".%.2d.bmp",0l, 4);
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
	bool ret = true;
	static const char *trans_ext[TILES_PER_TRANSITION] = {
		".01", ".03", ".07", ".05",
		".02", ".06", ".08", ".04",
		".09", ".10", ".12", ".11"
		};

	for (j=0; j<TILES_PER_TRANSITION; j++)
		if (!loadGround(i+j, path + trans_ext[j], progress ))
			ret = false;
	return ret;
}
