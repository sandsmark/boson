/***************************************************************************
                      speciesTheme.cpp  -  description                              
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

#include <qstring.h>
#include <qbitmap.h>
#include <qwmatrix.h>
#include <qprogressdialog.h>

#include <QwSpriteField.h>

#include <kapp.h>

#include "../common/log.h"
#include "../common/unit.h"
#include "../map/map.h"
#include "speciesTheme.h"


#define PROGRESS_N	(mobilePropNb+facilityPropNb)


speciesTheme::speciesTheme(char *themeName)
{
int	i;
QProgressDialog progress("Loading species theme...", 0, PROGRESS_N, NULL, "progress.speciesTheme", TRUE);
QString path(kapp->kde_datadir() + "/boson/pics/speciesThemes/" + themeName + "/" );

isLoaded = true;
progress.setProgress(0);
progress.show();

for(i=0; i<mobilePropNb; i++) {
	if (!loadMob(i, path + mobileProp[i].name)) isLoaded = false;
	progress.setProgress(i);
	}
printf("mobilePropNb = %d\n", mobilePropNb);

for(i=0; i<facilityPropNb; i++) {
	if (!loadFix(i, path + facilityProp[i].name )) isLoaded = false;
	progress.setProgress(i+mobilePropNb);
	}
printf("facilityPropNb = %d\n", facilityPropNb);

progress.setProgress(i++);

if (isLoaded) printf("\nSpeciesTheme : %d mobiles and %d facilities loaded\n", mobilePropNb, facilityPropNb);
	else printf("\nSpeciesTheme : problem while loading pixmaps\n");
}



bool speciesTheme::loadMob(int index, QString const &path)
{
int j;
QwSpritePixmap *p;

/* positionned sprites */
//printf("loading %s/Field.*.bmp ...\n", (const char *)path);
mobSprite[index] = new QwSpritePixmapSequence(path + "/Field.%02d.bmp", 0l, 12);
for(j=0; j<12; j++) {
	p = mobSprite[index]->image(j);
//	printf("mob %d...", j);
	if (p->isNull()) {
		printf("SpeciesTheme : Can't load(mob) %s/Field.%02d.bmp ...\n", (const char *)path, j);
		return false;
		}
//	printf("ok\n");
	p->setMask( p->createHeuristicMask() );
	p->setHotSpot( p->width()/2, p->height()/2 );
	}

/* big overview */
mobBigOverview[index] = new QPixmap(path + "/Overview.big.bmp");
if (mobBigOverview[index]->isNull()) {
	printf("SpeciesTheme : Can't load %s ...\n", (const char *)(path+"/Overview.big.bmp"));
	return false;
	}

/* small overview */
mobSmallOverview[index] = new QPixmap(path + "/Overview.small.bmp");
if (mobSmallOverview[index]->isNull()) {
	printf("SpeciesTheme : Can't load %s ...\n", (const char *)(path+"/Overview.small.bmp"));
	return false;
	}

return true;
}



bool speciesTheme::loadFix(int i, QString const &path)
{
int j;
QwSpritePixmap *p;

/* construction set */
fixSprite[i] = new QwSpritePixmapSequence(path + "/Field.%03d.bmp", 0l, 6);
for(j=0; j<6; j++) {
	p = fixSprite[i]->image(j);
	if (p->isNull()) {
		printf("SpeciesTheme : Can't load(fix) %s/Field.%03d.bmp ...\n", (const char *)path, j);
		return false;
		}
	p->setMask( p-> createHeuristicMask() );
	}

/* big overview */
fixBigOverview[i] = new QPixmap(path + "/Overview.big.xpm");
if (fixBigOverview[i]->isNull()) {
	printf("SpeciesTheme : Can't load %s ...\n", (const char *)(path+"/Overview.big.xpm"));
	return false;
	}
/* small overview */
fixSmallOverview[i] = new QPixmap(path + "/Overview.small.bmp");
if (fixSmallOverview[i]->isNull()) {
	printf("SpeciesTheme : Can't load %s ...\n", (const char *)(path+"/Overview.small.bmp"));
	return false;
	}

return true;
}
