/***************************************************************************
                      speciesTheme.cpp  -  description                              
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

allLoaded = true;
progress.setProgress(0);
progress.show();

for(i=0; i<mobilePropNb; i++) {
	if (!loadMob(i, path + mobileProp[i].name)) allLoaded = false;
	progress.setProgress(i);
	}

for(i=0; i<facilityPropNb; i++) {
	if (!loadFix(i, path + facilityProp[i].name )) allLoaded = false;
	progress.setProgress(i+mobilePropNb);
	}

progress.setProgress(i++);

if (!allLoaded)	logf(LOG_ERROR, "SpeciesTheme : problem while loading pixmaps");
	else	logf(LOG_INFO, "SpeciesTheme loaded : %d mobiles, %d facilities",
			mobilePropNb, facilityPropNb);
}



bool speciesTheme::loadMob(int index, QString const &path)
{
int j;
QList<QPixmap>	pix_l;
QList<QPoint>	point_l;
QPixmap		*p;
QPoint		*pp;
char		buffer[100];
bool		ret = true;


for(j=0; j<12; j++) {
	sprintf(buffer, "/Field.%02d.bmp", j);
	p = new QPixmap(path + buffer);
	if (p->isNull()) {
		printf("SpeciesTheme : Can't load(mob) %s/Field.%02d.bmp ...\n", (const char *)path, j);
		ret = false;
		continue;
		}
	p->setMask( p->createHeuristicMask() );
	pix_l.append(p);
	pp = new QPoint(p->width()/2, p->height()/2 );
	point_l.append(pp);
	}

mobSprite[index] = new QwSpritePixmapSequence(pix_l, point_l);

/* big overview */
mobBigOverview[index] = new QPixmap(path + "/Overview.big.bmp");
if (mobBigOverview[index]->isNull()) {
	printf("SpeciesTheme : Can't load %s ...\n", (const char *)(path+"/Overview.big.bmp"));
	ret = false;
	}

/* small overview */
mobSmallOverview[index] = new QPixmap(path + "/Overview.small.bmp");
if (mobSmallOverview[index]->isNull()) {
	printf("SpeciesTheme : Can't load %s ...\n", (const char *)(path+"/Overview.small.bmp"));
	ret = false;
	}

return ret;
}



bool speciesTheme::loadFix(int i, QString const &path)
{
int j;
QList<QPixmap>	pix_l;
QList<QPoint>	point_l;
QPixmap		*p;
QPoint		*pp;
char		buffer[100];
bool		ret = true;

for(j=0; j<6; j++) {
	sprintf(buffer, "/Field.%03d.bmp", j);
	p = new QPixmap(path + buffer);
	if (p->isNull()) {
		printf("SpeciesTheme : Can't load(fix) %s/Field.%03d.bmp ...\n", (const char *)path, j);
		ret = false;
		continue;
		}
	p->setMask( p->createHeuristicMask() );
	pix_l.append(p);
	pp = new QPoint( 0, 0);
	point_l.append(pp);
	}

fixSprite[i] = new QwSpritePixmapSequence(pix_l, point_l);

/* big overview */
fixBigOverview[i] = new QPixmap(path + "/Overview.big.xpm");
if (fixBigOverview[i]->isNull()) {
	printf("SpeciesTheme : Can't load %s ...\n", (const char *)(path+"/Overview.big.xpm"));
	ret = false;
	}
/* small overview */
fixSmallOverview[i] = new QPixmap(path + "/Overview.small.bmp");
if (fixSmallOverview[i]->isNull()) {
	printf("SpeciesTheme : Can't load %s ...\n", (const char *)(path+"/Overview.small.bmp"));
	ret = false;
	}

return ret;
}
