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
#include <qimage.h>

#include <QwSpriteField.h>

#include <kapp.h>

#include "../common/log.h"
#include "../common/unit.h"
#include "../common/map.h"

#include "speciesTheme.h"


speciesTheme::speciesTheme(char *themeName, QRgb color)
{
	team_color		= color;

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
	if ( ! loadPixmap(path + buffer, &p)) {
		logf(LOG_ERROR, "SpeciesTheme : Can't load(mob) %s/Field.%02d.bmp ...\n", (const char *)path, j);
		return false;
		}
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



bool speciesTheme::loadPixmap(const QString &path, QPixmap **pix)
{
	QImage	image(path), *mask;
	QBitmap	*m;
	int	x, y, w, h;
    	uchar	*yp;
	QRgb	*p;
	QRgb	background  = qRgb(255,  0, 255) & RGB_MASK ;
	QRgb	background2 = qRgb(248, 40, 240) & RGB_MASK ;
	QRgb	team_mask   = qRgb(255, 16,  16) & RGB_MASK;
	QRgb	team_mask2  = qRgb(248,  0,   0) & RGB_MASK;


	
	w = image.width(); h = image.height();

	boAssert(image.depth()==32);
	boAssert( w>31 );
	boAssert( h>31 );
	

	if (image.isNull() || w < 32 || h < 32) 
		return false;
	
	mask = new QImage ( w, h, 1, 2, QImage::LittleEndian);
	boAssert ( ! mask->isNull() );
	mask->setColor( 0, 0xffffff );
	mask->setColor( 1, 0 );
	mask->fill(0xff); 
	
	

	for ( y = 0; y < h; y++ ) {
		yp = mask->scanLine(y);	// mask
		p  = (QRgb *)image.scanLine(y);	// image
		for ( x = 0; x < w; x++, p++ ) {
			if ( (*p & 0x00fff0ff) == background ) {// set transparent 
				*(yp + (x >> 3)) &= ~(1 << (x & 7));
				continue;
			}
			if ( (*p & 0x00f8f8f8) == background2) {// set transparent 
				*(yp + (x >> 3)) &= ~(1 << (x & 7));
				continue;
			}
			if ( (*p & 0x00fff8f8) == team_mask ) {// set team color
				*p = team_color;
				puts("bof");
				continue;
			}
			if ( (*p & 0x00f8f8f8) == team_mask2) {// set team color
				*p = team_color;
				continue;
			}
			if ( (qRed(*p) > 0x80) && (qGreen(*p) < 0x70) && (qBlue(*p) < 0x70))
				*p = team_color;
		}
	}

	*pix = new QPixmap;
	m = new QBitmap;
	(*pix)->convertFromImage(image);
	m->convertFromImage(*mask);
	(*pix)->setMask( *m );

	delete mask;

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
	if (!loadPixmap(path + buffer, &p)) {
		logf(LOG_ERROR, "SpeciesTheme : Can't load(fix) %s/Field.%03d.bmp ...\n", (const char *)path, j);
		return false;
	}
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


