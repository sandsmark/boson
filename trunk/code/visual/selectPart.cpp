/***************************************************************************
                         selectPart.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jun 26 16:23:00 CET 1999
                                           
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

#include "../common/log.h"

#include "selectPart.h"

#define SP_THICK	4
#define SP_CORNER_LEN	15
#define SP_CORNER_POS	8
#define SP_W		35
#define SP_H		(SP_CORNER_LEN+SP_CORNER_POS)

static void drawSelectBox(QPainter &painter, QColor c1, QColor c2, int power);
static QwSpritePixmapSequence *initStatic(bool isDown);

QwSpritePixmapSequence * selectPart::qsps_up = 0l;
QwSpritePixmapSequence * selectPart::qsps_down = 0l;

/*
 *  selectPart
 */
selectPart::selectPart(int _f, int _z, bool isDown)
{
	if (isDown) {
		if (!qsps_down) qsps_down = initStatic(true);
		setSequence(qsps_down);
	} else {
		if (!qsps_up) qsps_up = initStatic(false);
		setSequence(qsps_up);
	}
	boAssert(_f>=0);
	boAssert(_f<PART_NB);
	if (_f<0 ) _f = 0;
	if (_f>= PART_NB ) _f = PART_NB-1;
	frame(_f);
	z( _z + 1);
}


/*
 *  Drawing functions
 */
void drawSelectBox(QPainter &painter, QColor c1, QColor c2, int power)
{
	QPen	pen(red);
	int	len =  3 * ( PART_NB-power);

	painter.setPen(pen);
	painter.fillRect(len,0, SP_W - len ,SP_THICK,c1);
	painter.fillRect(
		SP_W - SP_CORNER_LEN	, SP_CORNER_POS,
		SP_CORNER_LEN		, SP_THICK, c2);
	painter.fillRect(
		SP_W - SP_THICK		, SP_CORNER_POS,
		SP_THICK		, SP_CORNER_LEN, c2);
}


QwSpritePixmapSequence *initStatic(bool isDown)
{
	int i;
	QList<QPixmap>	pixmaps;
	QPixmap		*pix, *_pix;
	QList<QPoint>	points;
	QPoint		*point;
	QBitmap		*_mask;
	QPainter	painter;
	
	_pix = new QPixmap(SP_W, SP_H);
	_mask = new QBitmap(SP_W, SP_H);

	pixmaps.setAutoDelete( TRUE ); 
	points.setAutoDelete( TRUE ); 

	for(i=0; i<PART_NB; i++) {
		
		/* draw it */
		_pix->fill();
	
		painter.begin(_pix);
		if (isDown) {
			painter.rotate(180);
			painter.translate(-SP_W+1, -SP_H+1);
		}
		drawSelectBox(painter, red, green, i);
		painter.end();
	
		/* draw the mask */
		_mask->fill(black);
	
		painter.begin(_mask);
		if (isDown) {
			painter.rotate(180);
			painter.translate(-SP_W+1, -SP_H+1);
		}
		drawSelectBox(painter, white, white, i);
		painter.end();
	
		/* merge results */
		_pix->setMask(*_mask);

		/* create entries in QList */
		pix = new QPixmap(*_pix);
		pixmaps.append (pix);

		if (isDown)
			point = new QPoint(1, SP_H-2 - SP_CORNER_POS);
		else
			point = new QPoint(SP_W-2, SP_CORNER_POS);
		points.append (point);
		}

	delete _mask;
	delete _pix;

	return new QwSpritePixmapSequence(pixmaps,points);

}

