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

#include "selectbox.h"

#include <qpainter.h>
#include <qbitmap.h>

#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <kdebug.h>

#define SP_THICK 2

#define SP_CORNER_LEN 15
#define SP_CORNER_BAR_DISTANCE 4 // distance of the upper selection corners from the health bar

#define SP_H_DISTANCE 5 // distance of the rect from the unit (horizontal)
#define SP_V_DISTANCE 5 // distance of the rect from the unit (vertical)

#define POWER_LEVELS 15// AB: is from common/unit.h - the number of frames for the SelectBox, each with less power
#define PART_NB (POWER_LEVELS)

SelectBox::SelectBox(int x, int y, int width, int height, int z, QCanvas* canvas) // perhaps replace unit by width,height -> no need for unit.h
	: QCanvasSprite(0, canvas)
{
 mWidth = width + 2 * SP_H_DISTANCE;
 mHeight = height + 2 * SP_V_DISTANCE;
 setZ(z + 1);

 init(); // problem: is this efficient for many selections at once? can we use static somehow?

 setFrame(PART_NB - 1);
 move(x - SP_H_DISTANCE, y - SP_V_DISTANCE - barHeight());
 // do not yet show
}

void SelectBox::init()
{
 mPixmapArray = initPixmapArray();
 setSequence(mPixmapArray);
}

int SelectBox::frames()
{
 return POWER_LEVELS;
}


/*
 *  Drawing functions
 */
void SelectBox::drawSelectBox(QPainter &painter, bool mask)
{ // selection corner
// upper right
 painter.fillRect(boxWidth() - cornerLength(), barHeight() + SP_CORNER_BAR_DISTANCE, 
		cornerLength(), SP_THICK, 
		mask ? Qt::color1 : Qt::white);
 painter.fillRect(boxWidth() - SP_THICK, barHeight() + SP_CORNER_BAR_DISTANCE, 
		SP_THICK, cornerLength(), 
		mask ? Qt::color1 : Qt::white);

// upper left
 painter.fillRect(0, barHeight() + SP_CORNER_BAR_DISTANCE,
		cornerLength(), SP_THICK, 
		mask ? Qt::color1 : Qt::white);
 painter.fillRect(0, barHeight() + SP_CORNER_BAR_DISTANCE,
		SP_THICK, cornerLength(), 
		mask ? Qt::color1 : Qt::white);

// lower left
 painter.fillRect(0, boxHeight() - cornerLength(),
		SP_THICK, cornerLength(), 
		mask ? Qt::color1 : Qt::white);
 painter.fillRect(0, boxHeight() - SP_THICK,
		cornerLength(), SP_THICK,
		mask ? Qt::color1 : Qt::white);

// lower right
 painter.fillRect(boxWidth() - cornerLength(), boxHeight() - SP_THICK,
		cornerLength(), SP_THICK,
		mask ? Qt::color1 : Qt::white);
 painter.fillRect(boxWidth() - SP_THICK, boxHeight() - cornerLength(),
		SP_THICK, cornerLength(),
		mask ? Qt::color1 : Qt::white);
}

void SelectBox::drawHealthBar(QPainter &painter, bool mask, int frame)
{
 if (mask) {
	// "scrollbar" - or rather "health bar"
	painter.fillRect(1, 1, barWidth(frame), barHeight(), Qt::color1);
 } else {
	// "scrollbar" - or rather "health bar"
//	KPixmap pix(QPixmap(barWidth(3), barHeight())); // not working by any reason
	KPixmap pix(QPixmap((int)barWidth(frames() - 1), barHeight()));
	pix.fill(Qt::green);
	KPixmapEffect::gradient(pix, Qt::red, Qt::green, 
			KPixmapEffect::HorizontalGradient);
	painter.drawPixmap(1, 1, pix);
 }

// the white frame around it
 painter.setPen(mask ? Qt::color1 : Qt::white);
 painter.drawRect(0, 0, boxWidth(), barHeight() + 1);
}



QCanvasPixmapArray* SelectBox::initPixmapArray()
{
 QValueList<QPixmap> pixmaps;
 QPointArray points(PART_NB);
 QPainter painter;

 QPixmap pix(boxWidth(), boxHeight());
 for(int i = 0; i < PART_NB; i++) {
	// generate the mask first
	QBitmap mask (boxWidth(), boxHeight());
	painter.begin(&mask);
	mask.fill(Qt::color0);
	drawSelectBox(painter, true);
	drawHealthBar(painter, true, i);
	painter.end();

	// now draw the pixmap
	painter.begin(&pix);
	pix.fill(Qt::white);
	drawSelectBox(painter, false);
	drawHealthBar(painter, false, i);
	painter.end();

	// merge results 
	pix.setMask(mask);

	// create entries in QValueList 
	pixmaps.append(pix);

	points.setPoint(i, 0, 0);
 }

 return new QCanvasPixmapArray(pixmaps, points);
}

void SelectBox::update(double factor)
{
 setFrame((frames() - 1) * factor);
}

int SelectBox::barHeight()
{
 return 6;
}

int SelectBox::barWidth(int frame)
{
 double factor = (double)frame / (frames() - 1);
 return boxWidth() * factor - 2; // -2: the white frame around the box
}

int SelectBox::boxWidth() const
{
 return mWidth;
}

int SelectBox::boxHeight() const
{
 return mHeight;
}

int SelectBox::cornerLength() const
{
 return mWidth < mHeight ? mWidth / 5 : mHeight / 5;
 // alternative version:
 /*
 if (mWidth > 95 && mHeight > 95) {
	return SP_CORNER_LEN * 2;
 } else {
	return SP_CORNER_LEN;
 }
 */
}
