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
#include "sprites.h"


#define SP_THICK	4
#define SP_CORNER_LEN	15
#define SP_CORNER_POS	8
#define SP_W		35
#define SP_H		(SP_CORNER_LEN+SP_CORNER_POS)

static void drawSelectBox(QPainter &painter, QColor c1, QColor c2, int power);

QwSpritePixmapSequence * selectPart_up::qsps = 0l;
QwSpritePixmapSequence * selectPart_down::qsps = 0l;


void drawSelectBox(QPainter &painter, QColor c1, QColor c2, int power)
{
	QPen	pen(red);
	int	len =  2 * ( PART_NB-power);

	painter.setPen(pen);
	painter.fillRect(len,0, SP_W - len ,SP_THICK,c1);
	painter.fillRect(
		SP_W - SP_CORNER_LEN	, SP_CORNER_POS,
		SP_CORNER_LEN		, SP_THICK, c2);
	painter.fillRect(
		SP_W - SP_THICK		, SP_CORNER_POS,
		SP_THICK		, SP_CORNER_LEN, c2);
}


/*
 *  selectPart_up
 */
selectPart_up::selectPart_up(int _f, int _z)
{
	if (!qsps) {
		logf(LOG_WARNING, "selectPart_up : qsps not loaded");
		initStatic();
		}
	setSequence(qsps);
	boAssert( _f>=0); boAssert( _f<PART_NB);
	frame( _f);
	z( _z + 1);
}

void selectPart_up::initStatic()
{
	int i;
	QList<QPixmap>	pixmaps;
	QPixmap		*pix;
	QList<QPoint>	points;
	QPoint		*point;
	QBitmap		*mask;
	QPainter	painter;

	pix = new QPixmap(SP_W, SP_H);
	mask = new QBitmap(SP_W, SP_H);

	for(i=0; i<PART_NB; i++) {
		
		/* draw it */
		pix->fill();
	
		painter.begin(pix);
		drawSelectBox(painter, red, green, i);
		painter.end();
	
		/* draw the mask */
		mask->fill(black);
	
		painter.begin(mask);
		drawSelectBox(painter, white, white, i);
		painter.end();
	
		/* create entries in QList */
		pix->setMask(*mask);
		pix = new QPixmap(*pix);
		pixmaps.append (pix);
		point = new QPoint(SP_W-2, SP_CORNER_POS);
		points.append (point);
		}

//	delete mask; ///orzel : shoud I ?
	qsps = new QwSpritePixmapSequence(pixmaps,points);

	delete mask;
	delete pix;
}


/*
 *  selectPart_down
 */
selectPart_down::selectPart_down(int _f, int _z)
{
	if (!qsps) {
		logf(LOG_WARNING, "selectPart_down : qsps not loaded");
		initStatic();
		}
	setSequence(qsps);
	boAssert(_f>=0); boAssert(_f<PART_NB);
	frame(_f);
	z( _z + 1);
}


void selectPart_down::initStatic()
{
	int i;
	QList<QPixmap>	pixmaps;
	QPixmap		*pix;
	QList<QPoint>	points;
	QPoint		*point;
	QBitmap		*mask;
	QPainter	painter;
	
	pix = new QPixmap(SP_W, SP_H);
	mask = new QBitmap(SP_W, SP_H);

	for(i=0; i<PART_NB; i++) {
		
		/* draw it */
		pix->fill();
	
		painter.begin(pix);
		painter.rotate(180);
		painter.translate(-SP_W+1, -SP_H+1);
		drawSelectBox(painter, red, green, i);
		painter.end();
	
		/* draw the mask */
		mask->fill(black);
	
		painter.begin(mask);
		painter.rotate(180);
		painter.translate(-SP_W+1, -SP_H+1);
		drawSelectBox(painter, white, white, i);
		painter.end();
	
		/* create entries in QList */
		pix->setMask(*mask);
		pix = new QPixmap(*pix);
		pixmaps.append (pix);
		point = new QPoint(1, SP_H-2 - SP_CORNER_POS);
		points.append (point);
		}



//	delete mask; ///orzel : shoud I ?
	qsps = new QwSpritePixmapSequence(pixmaps,points);

	delete mask;
	delete pix;
}

