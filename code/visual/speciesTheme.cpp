/***************************************************************************
                      speciesTheme.cpp  -  description                              
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

#include <qstring.h>
#include <qbitmap.h>
#include <qwmatrix.h>
#include <qbitarray.h>

#include <QwSpriteField.h>

#include <kapp.h>

#include "../common/log.h"
#include "../common/unit.h"
#include "../common/map.h"

#include "speciesTheme.h"


speciesTheme::speciesTheme(char *themeName)
{

	mobBigOverview		= new (QPixmap*)[mobilePropNb];
	fixBigOverview		= new (QPixmap*)[facilityPropNb];
	mobSmallOverview	= new (QPixmap*)[mobilePropNb];
	fixSmallOverview	= new (QPixmap*)[facilityPropNb];


	mobSprite		= new (QwSpritePixmapSequence*)[mobilePropNb];
	fixSprite		= new (QwSpritePixmapSequence*)[facilityPropNb];


	mobiles			= new QBitArray(mobilePropNb);
	facilities		= new QBitArray(facilityPropNb);

	boAssert ( mobiles->fill(false) );
	boAssert ( facilities->fill(false) );

	themePath = new QString(kapp->kde_datadir() + "/boson/themes/species/" + themeName);

	/* preload some units */ 
	loadFix(FACILITY_CMDBUNKER);
	loadMob(MOB_QUAD);

}



bool speciesTheme::loadMob(int index)
{
int j;
QList<QPixmap>	pix_l;
QList<QPoint>	point_l;
QPixmap		*p;
QPoint		*pp;
char		buffer[100];
QString		path(*themePath + "/units/" + mobileProp[index].name);


for(j=0; j<12; j++) {
	sprintf(buffer, "/field.%02d.bmp", j);
	p = new QPixmap(path + buffer);
	if (p->isNull()) {
		logf(LOG_ERROR, "SpeciesTheme : Can't load(mob) %s/Field.%02d.bmp ...\n", (const char *)path, j);
		return false;
		}
	p->setMask( p->createHeuristicMask() );
	pix_l.append(p);
	pp = new QPoint(p->width()/2, p->height()/2 );
	point_l.append(pp);
	}

mobSprite[index] = new QwSpritePixmapSequence(pix_l, point_l);

/* big overview */
mobBigOverview[index] = new QPixmap(path + "/overview.big.bmp");
if (mobBigOverview[index]->isNull()) {
	logf(LOG_ERROR, "SpeciesTheme : Can't load %s ...\n", (const char *)(path+"/overview.big.bmp"));
	return false;
	}

/* small overview */
mobSmallOverview[index] = new QPixmap(path + "/overview.small.bmp");
if (mobSmallOverview[index]->isNull()) {
	logf(LOG_ERROR, "SpeciesTheme : Can't load %s ...\n", (const char *)(path+"/overview.small.bmp"));
	return false;
	}

mobiles->setBit(index);
return true;
}



bool speciesTheme::loadFix(int i)
{
int j;
QList<QPixmap>	pix_l;
QList<QPoint>	point_l;
QPixmap		*p;
QPoint		*pp;
char		buffer[100];

QString		path(*themePath + "/facilities/" + facilityProp[i].name);

for(j=0; j< CONSTRUCTION_STEP ; j++) {
	sprintf(buffer, "/field.%03d.bmp", j);
	p = new QPixmap(path + buffer);
	if (p->isNull()) {
		logf(LOG_ERROR, "SpeciesTheme : Can't load(fix) %s/Field.%03d.bmp ...\n", (const char *)path, j);
		return false;
		}
	p->setMask( p->createHeuristicMask() );
	pix_l.append(p);
	pp = new QPoint( 0, 0);
	point_l.append(pp);
	}

fixSprite[i] = new QwSpritePixmapSequence(pix_l, point_l);

/* big overview */
fixBigOverview[i] = new QPixmap(path + "/overview.big.bmp");
if (fixBigOverview[i]->isNull()) {
	logf(LOG_ERROR, "SpeciesTheme : Can't load %s ...\n", (const char *)(path+"/overview.big.bmp"));
	return false;
	}
/* small overview */
fixSmallOverview[i] = new QPixmap(path + "/overview.small.bmp");
if (fixSmallOverview[i]->isNull()) {
	logf(LOG_ERROR, "SpeciesTheme : Can't load %s ...\n", (const char *)(path+"/overview.small.bmp"));
	return false;
	}

facilities->setBit(i);
return true;
}


QPixmap	* speciesTheme::getBigOverview(mobType unit)
{
	if (!mobiles->testBit(unit))
		loadMob(unit);

	boAssert(mobiles->testBit(unit));
	return mobBigOverview[unit];
}


QPixmap	* speciesTheme::getBigOverview(facilityType unit)
{
	if (!facilities->testBit(unit))
		loadFix(unit);

	boAssert(facilities->testBit(unit));
	return fixBigOverview[unit];
}


QPixmap	* speciesTheme::getSmallOverview(mobType unit)
{
	if (!mobiles->testBit(unit))
		loadMob(unit);

	boAssert(mobiles->testBit(unit));
	return mobSmallOverview[unit];
}


QPixmap	* speciesTheme::getSmallOverview(facilityType unit)
{
	if (!facilities->testBit(unit))
		loadFix(unit);

	boAssert(facilities->testBit(unit));
	return fixSmallOverview[unit];
}


QwSpritePixmapSequence *speciesTheme::getPixmap(mobType unit)
{
	if (!mobiles->testBit(unit))
		loadMob(unit);

	boAssert(mobiles->testBit(unit));
	return mobSprite[unit];
}


QwSpritePixmapSequence *speciesTheme::getPixmap(facilityType unit)
{
	if (!facilities->testBit(unit))
		loadFix(unit);

	boAssert(facilities->testBit(unit));
	return fixSprite[unit];
}


