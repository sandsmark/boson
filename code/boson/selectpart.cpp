/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "selectpart.h"

#include <qpainter.h>
#include <qbitmap.h>

#include <kpixmap.h>
#include <kpixmapeffect.h>

#define SP_THICK	2
#define SP_CORNER_LEN	25
#define SP_CORNER_POS	8
#define SP_W		(PART_NB*2)
#define SP_H		(SP_CORNER_LEN + SP_CORNER_POS)

#define POWER_LEVELS 15// AB: is in common/unit.h - the number of frames for the SelectPart, each with less power
#define PART_NB (POWER_LEVELS)

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
void SelectPart::drawSelectBox(QPainter &painter, bool mask, int frame)
{
// int len = 2 * ( PART_NB - 1 - power );

 if (mask) {
	// "scrollbar" - or rather "health bar"
	painter.fillRect(0, 0, barWidth(frame), barHeight(), Qt::color1);

	// selection corner
	painter.fillRect(SP_W - SP_CORNER_LEN, SP_CORNER_POS,
			SP_CORNER_LEN, SP_THICK, Qt::color1);
	painter.fillRect(SP_W - SP_THICK, SP_CORNER_POS,
			SP_THICK, SP_CORNER_LEN, Qt::color1);
 } else {
	// read rendering 
	// "scrollbar" - or rather "health bar"
//	int a = barWidth(0); // why is this hack necessary????
	KPixmap pix(QPixmap(barWidth(0), barHeight())); 
	pix.fill(Qt::green);
	KPixmapEffect::gradient(pix, Qt::red, Qt::green, 
			KPixmapEffect::HorizontalGradient);
	painter.drawPixmap(0, 0, pix);

	// selection corner 
	painter.fillRect(SP_W - SP_CORNER_LEN, SP_CORNER_POS,
			SP_CORNER_LEN, SP_THICK, Qt::white);
	painter.fillRect(SP_W - SP_THICK, SP_CORNER_POS,
			SP_THICK, SP_CORNER_LEN, Qt::white);
 }
}


QCanvasPixmapArray* SelectPart::initStatic(SelectPartType type)
{
 QValueList<QPixmap> pixmaps;
 QPointArray points(PART_NB);
 QPainter painter;

 QPixmap pix(SP_W, SP_H);
 for(int i = 0; i < PART_NB; i++) {
	// generate the mask first
	QBitmap mask (SP_W, SP_H);
	painter.begin(&mask);
	if (SelectPart::PartDown == type) {
		painter.rotate(180);
		painter.translate(-SP_W + 1, -SP_H + 1);
	}
	mask.fill(Qt::color0);
	drawSelectBox(painter, true, i);
	painter.end();

	// now draw the pixmap
	painter.begin(&pix);
	if (SelectPart::PartDown == type) {
		painter.rotate(180);
		painter.translate(-SP_W + 1, -SP_H + 1);
	}
	pix.fill(Qt::white);
	drawSelectBox(painter, false, i);
	painter.end();

	// merge results 
	pix.setMask(mask);

	// create entries in QValueList 
	pixmaps.append(pix);

	if (SelectPart::PartDown == type) {
		points.setPoint(i, 1, SP_H - 2 - SP_CORNER_POS);
	} else {
		points.setPoint(i, SP_W - 2, SP_CORNER_POS);
	}
 }

 return new QCanvasPixmapArray(pixmaps, points);
}

void SelectPart::update(double factor)
{
 setFrame((frames() - 1) * factor);
}

int SelectPart::barHeight()
{
 return 2 * SP_THICK;
}

int SelectPart::barWidth(int frame)
{
 return SP_W - 2 * (PART_NB - 1 - frame);
}
