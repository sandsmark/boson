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
#include "speciesTheme.h"
#include "../map/map.h"

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
//printf("mobilePropNb = %d\n", mobilePropNb);
//completeMob();
for(i=0; i<facilityPropNb; i++) {
	if (!loadFix(i, path + facilityProp[i].name )) isLoaded = false;
//	loadFix(i, path + facilityProp[i].name);
	progress.setProgress(i+mobilePropNb);
	}
//printf("facilityPropNb = %d\n", facilityPropNb);

progress.setProgress(i++);

///orzel: beurk...
//if (mobSprite[0][0]->isNull() || mobSprite[0][1]->isNull())
//	isLoaded = FALSE;

if (isLoaded) printf("SpeciesTheme : %d mobiles and %d facilities loaded\n", mobilePropNb, facilityPropNb);
	else printf("SpeciesTheme : problem while loading pixmaps\n");
}

bool speciesTheme::loadMob(int index, QString const &path)
{
QList<QPixmap> pix;
QList<QPoint> pts;

QPixmap	onePix;
QBitmap	mask;
QPoint	pt;

/* 0 deg view */
onePix.load(path + "/Field.0.xpm");
  mask.load(path + "/Field.0.mask.xpm");
if (mask.isNull())
	mask = onePix.createHeuristicMask();
onePix.setMask(mask);

pt.setX(onePix.width()/2);
pt.setY(onePix.height()/2);
pix.append( &onePix);
pts.append( &pt );

/* 30 deg view */
mask.resize(0,0);

onePix.load(path + "/Field.30.xpm");
  mask.load(path + "/Field.30.mask.xpm");
if (mask.isNull())
	mask = onePix.createHeuristicMask();
onePix.setMask(mask);

pt.setX(onePix.width()/2);
pt.setY(onePix.height()/2);
pix.append( &onePix);
pts.append( &pt );

for(int i=2; i<12; i++) { ///orzel : q&d fix [tm] to createmobile
	pix.append( &onePix);
	pts.append( &pt );
	}
/*
mobSprite[index][1] = new QPixmap(path + "/Field.30.xpm");
mask.load(path + "/Field.30.mask.xpm");
if (mask.isNull())
	mask = mobSprite[index][1]->createHeuristicMask();
mobSprite[index][1]->setMask(mask);
*/

mobSprite[index] = new QwSpritePixmapSequence(pix,pts);

/* overview */
mobOverview[index] = new QPixmap(path + "/Overview.big.xpm");

return true;
}

bool speciesTheme::loadFix(int i, QString const &path)
{
//printf("Loading %s ...", (path+"/Field.005.bmp").data());
//fixSprite[i] = new QPixmap(path + ".xpm");

/*
QList<QPixmap> pix;
QList<QPoint> pts;

for (i=0; i<5; i++) {
	pix.append( *(new QPixmapkkk) );
	pts.append(QPoint(BO_TILE_SIZE/2, BO_TILE_SIZE/2));
	temp[i] = new QPixmap(path + "/Field.005.bmp");
fixSprite[i]-> setMask( fixSprite[i]-> createHeuristicMask() );
	}
*/

int j;
QwSpritePixmap *p;

fixSprite[i] = new QwSpritePixmapSequence(path + "/Field.%03d.bmp", 0l, 6);

for(j=0; j<6; j++) {
	p = fixSprite[i]->image(j);
	p->setMask( p-> createHeuristicMask() );
	}

/*fixSprite[i] = new QPixmap(path + "/Field.005.bmp");
fixSprite[i]-> setMask( fixSprite[i]-> createHeuristicMask() );
if (fixSprite[i]->isNull()) {
	printf("SpeciesTheme : Can't load %s ...\n", (path+"/Field.005.bmp").data());
	return false;
	}

boAssert(BO_TILE_SIZE * facilityProp[i].width == fixSprite[i]->width());
boAssert(BO_TILE_SIZE * facilityProp[i].height == fixSprite[i]->height());
*/

fixOverview[i] = new QPixmap(path + "/Overview.big.xpm");
if (fixOverview[i]->isNull()) {
	printf("SpeciesTheme : Can't load %s ...\n", (const char *)(path+"/Overview.big.bmp"));
	return false;
	}
//puts("ok");
return true;
}

/*

bool speciesTheme::completeMob()
{
sprite = new (QPixmap *)[12];
QWMatrix *matrix = new QWMatrix();
int i;

matrix->setMatrix(0,-1,-1,0,0,0);
for (i=0; i<mobilePropNb; i++)
	mobSprite[i][2] = new QPixmap(mobSprite[i][1]->xForm(*matrix));

matrix->setMatrix(0,1,-1,0,0,0);
for (i=0; i<mobilePropNb; i++) {
	mobSprite[i][3] = new QPixmap(mobSprite[i][0]->xForm(*matrix));
	mobSprite[i][4] = new QPixmap(mobSprite[i][1]->xForm(*matrix));
	mobSprite[i][5] = new QPixmap(mobSprite[i][2]->xForm(*matrix));
	}

matrix->setMatrix(-1,0,0,-1,0,0);
for (i=0; i<mobilePropNb; i++) {
	mobSprite[i][6] = new QPixmap(mobSprite[i][0]->xForm(*matrix));
	mobSprite[i][7] = new QPixmap(mobSprite[i][1]->xForm(*matrix));
	mobSprite[i][8] = new QPixmap(mobSprite[i][2]->xForm(*matrix));
	}

matrix->setMatrix(0,-1,1,0,0,0);
for (i=0; i<mobilePropNb; i++) {
	mobSprite[i][9] = new QPixmap(mobSprite[i][0]->xForm(*matrix));
	mobSprite[i][10] = new QPixmap(mobSprite[i][1]->xForm(*matrix));
	mobSprite[i][11] = new QPixmap(mobSprite[i][2]->xForm(*matrix));
	}

return true;
}
*/
