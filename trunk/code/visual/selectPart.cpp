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

#include "common/log.h"

#include "selectPart.h"

#define SP_THICK	4
#define SP_CORNER_LEN	15
#define SP_CORNER_POS	8
#define SP_W		(PART_NB*2)
#define SP_H		(SP_CORNER_LEN+SP_CORNER_POS)

extern QCanvas	*vcanvas;

static void drawSelectBox(QPainter &painter, bool bw, int power);
static QCanvasPixmapArray *initStatic(selectPart::sp_type type);

QCanvasPixmapArray * selectPart::qsps_up = 0l;
QCanvasPixmapArray * selectPart::qsps_down = 0l;

/*
 *  selectPart
 */
selectPart::selectPart(int _f, int _z, sp_type type)
	: QCanvasSprite(0, vcanvas)
{
	if (PART_DOWN == type) {
		if (!qsps_down) qsps_down = initStatic(PART_DOWN);
		setSequence(qsps_down);
	} else {
		if (!qsps_up) qsps_up = initStatic(PART_UP);
		setSequence(qsps_up);
	}
	// no segfault with buggy values for _f
	boAssert(_f>=0); if (_f<0 ) _f = 0;
	boAssert(_f<=PART_NB); if (_f>= PART_NB ) _f = PART_NB-1;
	// actually do it
	setFrame(_f);
	setZ ( _z + 1);

	show();
}


/*
 *  Drawing functions
 */
void drawSelectBox(QPainter &painter, bool bw, int power)
{
	int	len =  2 * ( PART_NB-1-power);

	if (bw) {
	// mask
		// "scrollbar"
		painter.fillRect(0  ,0, SP_W       ,SP_THICK, Qt::white);
		// selection corner
		painter.fillRect(
			SP_W - SP_CORNER_LEN	, SP_CORNER_POS,
			SP_CORNER_LEN		, SP_THICK, Qt::white);
		painter.fillRect(
			SP_W - SP_THICK		, SP_CORNER_POS,
			SP_THICK		, SP_CORNER_LEN, Qt::white);
	} else {
	// read rendering 
		/* "scrollbar" */
		painter.fillRect(0  ,0, SP_W       ,SP_THICK, Qt::red);
		painter.fillRect(len,0, SP_W - len ,SP_THICK, Qt::green);
		/* selection corner */
		painter.fillRect(
			SP_W - SP_CORNER_LEN	, SP_CORNER_POS,
			SP_CORNER_LEN		, SP_THICK, Qt::magenta);
		painter.fillRect(
			SP_W - SP_THICK		, SP_CORNER_POS,
			SP_THICK		, SP_CORNER_LEN, Qt::magenta);
	}

}


QCanvasPixmapArray *initStatic(selectPart::sp_type type)
{
	int i;
	QList<QPixmap>	pixmaps;
	QPixmap		*pix, *_pix;
	QList<QPoint>	points;
	QPoint		*point;
	QPainter	painter;
	
	_pix = new QPixmap(SP_W, SP_H);
	
	/* draw the mask */
	QBitmap _mask (SP_W, SP_H);
	_mask.fill(Qt::black);

	painter.begin(&_mask);
	if (selectPart::PART_DOWN == type) {
		painter.rotate(180);
		painter.translate(-SP_W+1, -SP_H+1);
	}
	drawSelectBox(painter, true, i);
	painter.end();
	


	pixmaps.setAutoDelete( TRUE ); 
	points.setAutoDelete( TRUE ); 

	for(i=0; i<PART_NB; i++) {
		
		/* draw it */
		_pix->fill();
	
		painter.begin(_pix);
		if (selectPart::PART_DOWN == type) {
			painter.rotate(180);
			painter.translate(-SP_W+1, -SP_H+1);
		}
		drawSelectBox(painter, false, i);
		painter.end();

		/* merge results */
		_pix->setMask(_mask);

		/* create entries in QList */
		pix = new QPixmap(*_pix);
		pixmaps.append (pix);

		if (selectPart::PART_DOWN == type)
			point = new QPoint(1, SP_H-2 - SP_CORNER_POS);
		else
			point = new QPoint(SP_W-2, SP_CORNER_POS);
		points.append (point);
		}

	delete _pix;

	return new QCanvasPixmapArray(pixmaps,points);

}

