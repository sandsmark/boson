/***************************************************************************
                         boshot  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Thu Dec 16 14:35:00 CET 1999
                                           
    copyright            : (C) 1999-2000 by Thomas Capricelli                         
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

#include <stdlib.h>		// random.

#include <qimage.h>

#include "common/log.h"
#include "boshot.h"
#include "visual.h"


QCanvasPixmapArray	*boShot::shotSequ;

QCanvasPixmapArray	*boShot::unitSequ[UNITS_SHOTS_NB];
QCanvasPixmapArray	*boShot::fixSequ[FIX_SHOTS_NB];

QBitArray		boShot::qba_units(UNITS_SHOTS_NB);
QBitArray		boShot::qba_fix(FIX_SHOTS_NB);



static bool loadPixmap(const QString &path, QPixmap **pix);	// load one frame of the animation

#define SHOTS "shots/explosion%1"
#define _SHOTS "shots/explosion0000"

/*
 *  boshot
 */
boShot::boShot(int _x, int _y, int _z, shot_style style)
	: QCanvasSprite (0, vcanvas)
{

	int	version;
	bool	ret;
	switch(style) {

		case SHOT_SHOT:
			/* small shot (unit hitten) */
			if (!shotSequ) { // static imagepool initialization
	//			QString path( locate ( "data", "boson/themes/species/human/explosions/shot/explosion00.pbm") + "explosion%02d" );
				QString path = *dataPath + "themes/species/human/explosions/" SHOTS ;
	//			printf("path is %s\n", (const char*)path);
	//			printf("path is %s\n", (const char*)path);
				shotSequ = new QCanvasPixmapArray(path+".ppm", SHOT_FRAMES);
				//shotSequ = new QCanvasPixmapArray(path+".ppm", path+".pbm", SHOT_FRAMES);
	//			printf("image0 is %d %d\n", shotSequ->image(0)->width(), shotSequ->image(0)->height());
				boAssert(!shotSequ->image(0)->isNull());
				if (shotSequ->image(0)->width()!=8 || shotSequ->image(0)->height()!=11) {
					delete shotSequ;
					shotSequ = 0;
					delete this;
					return;
				}
			}
			setSequence(shotSequ);		// set image set
			maxCounter = SHOT_FRAMES;
			break;

		case SHOT_UNIT:
			version = random()% UNITS_SHOTS_NB;
			ret = loadBig(style, version);
			boAssert(ret);
			if (!ret) {
				delete this;
				return;
			}
			maxCounter = UNITS_SHOT_FRAMES;
			setSequence(unitSequ[version]);
			break;
		case SHOT_FACILITY:
			version = random()% FIX_SHOTS_NB;
			ret = loadBig(style, version);
			boAssert(ret);
			if (!ret) {
				delete this;
				return;
			}
			maxCounter = FIX_SHOT_FRAMES;
			setSequence(fixSequ[version]);
			/*
			_x -= BIG_W >> 1;
			_y -= BIG_H >> 1;
			*/
			break;
	};

	counter = 0; setFrame( 0);		// position the first image of the animation
	move(_x, _y); setZ( _z + 1);	// position in the canvas
	show();
	startTimer(60);			// begin animation, 60 ms/frame
}

void  boShot::timerEvent( QTimerEvent * )
{
	counter++;
//	printf("boShot::timerEvent : counter = %d\n", counter);
	if (counter<maxCounter) {
		setFrame(counter);
		return;
	}
	killTimers();
	delete this;
}


bool boShot::loadBig(shot_style style, int version)
{

	boAssert(style!=SHOT_SHOT);
	if (SHOT_SHOT==style) return false;

	boAssert(version>=0);

	int		j;
	char		buffer[200];
	QList<QPixmap>	pix_l;
	QList<QPoint>	point_l;
	QPixmap		*p;
	QPoint		*pp;
	QString		path  = *dataPath + "themes/species/human/explosions/";
	int		frame_nb;


	switch(style) {
		default:
			logf(LOG_ERROR, "unexpected style in boShot,loadBig");
			return false;
		case SHOT_UNIT:
			boAssert(version<UNITS_SHOTS_NB);
			if (qba_units.testBit(version)) return true;
			frame_nb =  UNITS_SHOT_FRAMES;
			path+= "units/";
			break;

		case SHOT_FACILITY:
			boAssert(version<FIX_SHOTS_NB);
			if (qba_fix.testBit(version)) return true;
			frame_nb =  FIX_SHOT_FRAMES;
			path+= "facilities/";
			break;
	}

	// orzel, +1 because shots # are from 1->4, will change soon, XXX
	sprintf(buffer, "expl.%02d", version+1);
	path += buffer;
//	printf("path is %s\n", (const char*)path);

	for(j=0; j< frame_nb; j++) {
		sprintf(buffer, ".%04d.bmp", j);
		if (!loadPixmap(path + buffer, &p)) {
			logf(LOG_ERROR, "boshot::loadBig Can't load %s.%04d.bmp...\n", (const char *)path, j);
			return false;
		}
		pix_l.append(p);
		pp = new QPoint( p->width() >> 1, p->height() >> 1); // hotspot in the center
		point_l.append(pp);
	}

	switch(style) {
		default:
			logf(LOG_ERROR, "unexpected style in boShot,loadBig(2)");
			return false;
		case SHOT_UNIT:
			unitSequ[version] = new QCanvasPixmapArray(pix_l, point_l);
			qba_units.setBit(version);
			break;

		case SHOT_FACILITY:
			fixSequ[version] = new QCanvasPixmapArray(pix_l, point_l);
			qba_fix.setBit(version);
			break;
	}
	return true;
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
	boAssert( w>7 );
	boAssert( h>10 );
	

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


