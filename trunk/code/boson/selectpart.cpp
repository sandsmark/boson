/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
/***************************************************************************
                         selectPart.cpp  -  description                              
                             -------------------                                         

    version              : $Id$
    begin                : Sat Jun 26 16:23:00 CET 1999
                                           
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

#include "selectpart.h"

#include <qpainter.h>
#include <qbitmap.h>

#define SP_THICK	2
#define SP_CORNER_LEN	25
#define SP_CORNER_POS	8
#define SP_W		(PART_NB*2)
#define SP_H		(SP_CORNER_LEN+SP_CORNER_POS)

#define POWER_LEVELS 15// AB: is in common/unit.h - the number of frames for the SelectPart, each with less power
#define PART_NB (POWER_LEVELS)

static void drawSelectBox(QPainter &painter, bool bw, int power = 0);

QCanvasPixmapArray * SelectPart::mPartUp = 0l;
QCanvasPixmapArray * SelectPart::mPartDown = 0l;

SelectPart::SelectPart(int z, SelectPartType type, QCanvas* canvas)
	: QCanvasSprite(0, canvas)
{
 init(type);
 setZ (z + 1);
 setFrame(PART_NB - 1);
 // do not yet show
}

SelectPart::SelectPart(int frame, int z, SelectPartType type, QCanvas* canvas)
	: QCanvasSprite(0, canvas)
{
 init(type);

 // no segfault with buggy values for frame
 if (frame < 0) {
	frame = 0;
 }
 if (frame >= PART_NB) {
	frame = PART_NB - 1;
 }
 // actually do it
 setFrame(frame);
 setZ (z + 1);
 show();
}

void SelectPart::init(SelectPartType type)
{
 if (PartDown == type) {
	if (!mPartDown) {
		mPartDown = initStatic(PartDown);
	}
	setSequence(mPartDown);
 } else {
	if (!mPartUp) {
		mPartUp = initStatic(PartUp);
	}
	setSequence(mPartUp);
 }
}

int SelectPart::frames()
{
 return POWER_LEVELS;
}


/*
 *  Drawing functions
 */
void drawSelectBox(QPainter &painter, bool bw, int power)
{
 int len = 2 * ( PART_NB - 1 - power );

 if (bw) {
	// mask
	// "scrollbar"
	painter.fillRect(0  ,0, SP_W       ,2*SP_THICK, Qt::white);
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
	painter.fillRect(0  ,0, SP_W       ,2*SP_THICK, Qt::red);
	painter.fillRect(len,0, SP_W - len ,2*SP_THICK, Qt::green);
	/* selection corner */
	painter.fillRect(
			SP_W - SP_CORNER_LEN	, SP_CORNER_POS,
			SP_CORNER_LEN		, SP_THICK, Qt::white);
	painter.fillRect(
			SP_W - SP_THICK		, SP_CORNER_POS,
			SP_THICK		, SP_CORNER_LEN, Qt::white);
 }

}


QCanvasPixmapArray* SelectPart::initStatic(SelectPartType type)
{
 QList<QPixmap>	pixmaps;
 QPixmap *pix, *_pix;
 QList<QPoint> points;
 QPoint *point;
 QPainter painter;
	
 _pix = new QPixmap(SP_W, SP_H);
	
 /* draw the mask */
 QBitmap _mask (SP_W, SP_H);
 _mask.fill(Qt::black);

 painter.begin(&_mask);
 if (SelectPart::PartDown == type) {
	painter.rotate(180);
	painter.translate(-SP_W+1, -SP_H+1);
 }
 drawSelectBox(painter, true); 
 painter.end();
	


 pixmaps.setAutoDelete( TRUE ); 
 points.setAutoDelete( TRUE ); 

 for(int i = 0; i < PART_NB; i++) {
	/* draw it */
	_pix->fill();

	painter.begin(_pix);
	if (SelectPart::PartDown == type) {
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

	if (SelectPart::PartDown == type) {
		point = new QPoint(1, SP_H-2 - SP_CORNER_POS);
	} else {
		point = new QPoint(SP_W-2, SP_CORNER_POS);
	}
	points.append (point);
 }

 delete _pix;

 return new QCanvasPixmapArray(pixmaps,points);
}

