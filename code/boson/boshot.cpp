/***************************************************************************
                         boshot  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Dec 16 14:35:00 CET 1999
                                           
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

#include <kapp.h>

#include <qimage.h>

#include "common/log.h"
#include "boshot.h"


QwSpritePixmapSequence * boShot::shotSequ = 0l;
QwSpritePixmapSequence * boShot::bigShotSequ = 0l;



static QwSpritePixmapSequence *loadBig(void);
static bool loadPixmap(const QString &path, QPixmap **pix);

/*
 *  boshot
 */
boShot::boShot(int _x, int _y, int _z, bool isBig)
{

	if (isBig) {
		/* BIG explosion (unit destroyed) */
		if (!bigShotSequ)
			bigShotSequ = loadBig();
		setSequence(bigShotSequ);	// set image set
		maxCounter = BIG_SHOT_FRAMES;
	} else {
		/* small shot (unit hitten) */
		if (!shotSequ) { // static imagepool initialization
			QString path(kapp->kde_datadir() + "/boson/themes/species/human/explosions/shot/explosion%02d");
			shotSequ = new QwSpritePixmapSequence(path+".ppm", path+".pbm", SHOT_FRAMES);
		}

		setSequence(shotSequ);		// set image set
		maxCounter = SHOT_FRAMES;
	}


	counter = 0; frame( 0);		// position the first image of the animation
	moveTo(_x, _y); z( _z + 1);	// position in the field
	startTimer(60);			// begin animation, 60 ms/frame
}

void  boShot::timerEvent( QTimerEvent * )
{
	counter++;
	if (counter<maxCounter) {
		frame(counter);
		return;
	}
	killTimers();
	delete this;
}


QwSpritePixmapSequence *loadBig(void) // XXX should be factorized with speciesTheme.cpp stuff...
{

	int		j;
	char		buffer[200];
	QList<QPixmap>	pix_l;
	QList<QPoint>	point_l;
	QPixmap		*p;
	QPoint		*pp;
	QString		path(kapp->kde_datadir() + "/boson/themes/species/human/explosions/big");

	// XXX will use  speciesTheme::loadPixmap() (made non-speciesTheme dep and public)
	// and QCanvasPixmap(const QPixmap&, QPoint hotspot);


	for(j=0; j< BIG_SHOT_FRAMES ; j++) {
		sprintf(buffer, "/big.%02d.bmp", j);
		if (!loadPixmap(path + buffer, &p)) {
			logf(LOG_ERROR, "boshot::loadBig Can't load %s/big.%02d.bmp...\n", (const char *)path, j);
		}
		pix_l.append(p);
		pp = new QPoint( 0, 0);
		point_l.append(pp);
	}

	return new QwSpritePixmapSequence(pix_l, point_l);
}




bool loadPixmap(const QString &path, QPixmap **pix)
{
	QImage	image(path), *mask;
	QBitmap	*m;
	int	x, y, w, h;
    	uchar	*yp;
	QRgb	*p;

	static const QRgb background  = qRgb(255,  0, 255) & RGB_MASK ;

	
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
		for ( x = 0; x < w; x++, p++ )
			if ( (*p & 0x00fff0ff) == background ) {// set transparent 
				*(yp + (x >> 3)) &= ~(1 << (x & 7));
				continue;
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


