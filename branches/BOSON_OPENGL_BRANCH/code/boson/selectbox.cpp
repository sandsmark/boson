/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#ifndef NO_OPENGL

#include "bosontexturearray.h"

#include <kdebug.h>
#include <kimageeffect.h>

#include <qimage.h>
#include <qgl.h>

#define POWER_LEVELS 15

SelectBoxData* SelectBox::mBoxData = 0;

SelectBoxData::SelectBoxData()
{
 mTextures = 0;
}

SelectBoxData::~SelectBoxData()
{
 // TODO: free the display lists?
}

GLuint SelectBoxData::list(double factor)
{
 int list = (int)((POWER_LEVELS - 1) * factor);
 if (!mDisplayLists.contains(list)) {
	loadBoxes();
	if (!mDisplayLists.contains(list)) {
		kdError() << k_funcinfo << "Unable to generate a SelectBox for " << factor << endl;
		return 0;
	}
	return mDisplayLists[list];
 }
 return mDisplayLists[list];
}

void SelectBoxData::loadBoxes()
{
 // TODO: we might want to use mipmaps here - interesting for big units, as well as for zooming
 GLuint list = glGenLists(POWER_LEVELS);
 if (mTextures) {
	kdWarning() << k_funcinfo << "textures loaded before" << endl;
	delete mTextures;
	mTextures = 0;
 }
 QValueList<QImage> textureImages;
 glEnable(GL_TEXTURE_2D); // should already be enabled, cause we need it for units. just to be sure...

// AB: the size is hardcoded. mipmaps might be VERY useful here!
// FIXME: Qt::red simply doesn't work - we need to use Qt::blue .. why???
// QImage image = KImageEffect::gradient(QSize(BO_TILE_SIZE, 6), Qt::red, Qt::green, KImageEffect::HorizontalGradient);
// QImage image = KImageEffect::gradient(QSize(BO_TILE_SIZE, 6), QColor(255,0,0), Qt::green, KImageEffect::HorizontalGradient);
 QImage image = KImageEffect::gradient(QSize(BO_TILE_SIZE, 6), Qt::blue, Qt::green, KImageEffect::HorizontalGradient);
 image = QGLWidget::convertToGLFormat(image);
 textureImages.append(image);

 mTextures = new BosonTextureArray(textureImages);
 if (!mTextures->texture(0)) {
	kdWarning() << k_funcinfo << "textures got loaded improperly" << endl;
 }

 for (unsigned int i = 0; i < POWER_LEVELS; i++) {
	glNewList(list + i, GL_COMPILE);
		drawCube();
		glBindTexture(GL_TEXTURE_2D, mTextures->texture(0));
		drawHealthBar(i);
	glEndList();
	mDisplayLists.insert(i, list+i);
 }
}

void SelectBoxData::drawCube()
{
 glDisable(GL_TEXTURE_2D);
 glDisable(GL_BLEND);
 glLineWidth(2.0);
		
 const float s = 0.3; // size (width or height depending on direction) of a line
 glBegin(GL_LINES);
	// bottom quad
	glVertex3f(0.0, 0.0, 0.0); glVertex3f(s, 0.0, 0.0);
	glVertex3f(1.0 - s, 0.0, 0.0); glVertex3f(1.0, 0.0, 0.0);
	glVertex3f(1.0, 0.0, 0.0); glVertex3f(1.0, s, 0.0);
	glVertex3f(1.0, 1.0 - s, 0.0); glVertex3f(1.0, 1.0, 0.0);
	glVertex3f(1.0 - s, 1.0, 0.0); glVertex3f(1.0, 1.0, 0.0);
	glVertex3f(0.0, 1.0, 0.0); glVertex3f(s, 1.0, 0.0);
	glVertex3f(0.0, 1.0, 0.0); glVertex3f(0.0, 1.0 - s, 0.0);
	glVertex3f(0.0, 0.0, 0.0); glVertex3f(0.0, s, 0.0);

	// top quad
	glVertex3f(0.0, 0.0, 1.0); glVertex3f(s, 0.0, 1.0);
	glVertex3f(1.0 - s, 0.0, 1.0); glVertex3f(1.0, 0.0, 1.0);
	glVertex3f(1.0, 0.0, 1.0); glVertex3f(1.0, s, 1.0);
	glVertex3f(1.0, 1.0 - s, 1.0); glVertex3f(1.0, 1.0, 1.0);
	glVertex3f(1.0 - s, 1.0, 1.0); glVertex3f(1.0, 1.0, 1.0);
	glVertex3f(0.0, 1.0, 1.0); glVertex3f(s, 1.0, 1.0);
	glVertex3f(0.0, 1.0, 1.0); glVertex3f(0.0, 1.0 - s, 1.0);
	glVertex3f(0.0, 0.0, 1.0); glVertex3f(0.0, s, 1.0);

	// front quad
	glVertex3f(0.0, 0.0, 0.0); glVertex3f(0.0, 0.0, s);
	glVertex3f(0.0, 0.0, 1.0); glVertex3f(0.0, 0.0, 1.0 - s);
	glVertex3f(1.0, 0.0, 0.0); glVertex3f(1.0, 0.0, s);
	glVertex3f(1.0, 0.0, 1.0); glVertex3f(1.0, 0.0, 1.0 - s);

	// back quad
	glVertex3f(0.0, 1.0, 0.0); glVertex3f(0.0, 1.0, s);
	glVertex3f(0.0, 1.0, 1.0); glVertex3f(0.0, 1.0, 1.0 - s);
	glVertex3f(1.0, 1.0, 0.0); glVertex3f(1.0, 1.0, s);
	glVertex3f(1.0, 1.0, 1.0); glVertex3f(1.0, 1.0, 1.0 - s);

	// left quad
	// right quad
	// --> no need to since, that'll be the same lines as in the other quads
 glEnd();

 glLineWidth(1.0);
 glEnable(GL_TEXTURE_2D);
 glEnable(GL_BLEND);
}

void SelectBoxData::drawHealthBar(int frame)
{
 GLfloat texLength = ((float)frame) / (float)(POWER_LEVELS - 1);
// double factor = (double)frame / (frames() - 1);
// return (int)(boxWidth() * factor - 2); // -2: the white frame around the box
 GLfloat length = 1.0 * texLength; // y-direction
 GLfloat hy = 0.15; // height in y-direction
 GLfloat hz = 0.15; // height in z-direction
 GLfloat depth = 0.15;
 glDisable(GL_BLEND);
// FIXME: a lot of redundant vertices here!
 glTranslatef(0.0, 1.0 - hy, 1.0 - hz);

 glBegin(GL_QUADS);
	// bottom
	glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, 0.0);
	glTexCoord2f(texLength, 0.0); glVertex3f(length, 0.0, 0.0);
	glTexCoord2f(texLength, 0.0); glVertex3f(length, hy, 0.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.0, hy, 0.0);
 
	// top
	glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, hz);
	glTexCoord2f(texLength, 0.0); glVertex3f(length, 0.0, hz);
	glTexCoord2f(texLength, 0.0); glVertex3f(length, hy, hz);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.0, hy, hz);

	// front
	glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, 0.0);
	glTexCoord2f(texLength, 0.0); glVertex3f(length, 0.0, 0.0);
	glTexCoord2f(texLength, 0.0); glVertex3f(length, 0.0, hz);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, hz);

	// back
	glTexCoord2f(0.0, 0.0); glVertex3f(0.0, hy, 0.0);
	glTexCoord2f(texLength, 0.0); glVertex3f(length, hy, 0.0);
	glTexCoord2f(texLength, 0.0); glVertex3f(length, hy, hz);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.0, hy, hz);

	// left
	glTexCoord2f(0.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, hz);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(0.0, hy, hz);
	glVertex3f(0.0, hy, 0.0);

	//right
	glTexCoord2f(texLength, 0.0);
	glVertex3f(length, 0.0, 0.0);
	glVertex3f(length, 0.0, hz);
	glTexCoord2f(texLength, 1.0);
	glVertex3f(length, hy, hz);
	glVertex3f(length, hy, 0.0);
 glEnd();

 glDisable(GL_TEXTURE_2D);
 // bottom
 glBegin(GL_LINE_STRIP);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(1.0, 0.0, 0.0);
	glVertex3f(1.0, hy, 0.0);
	glVertex3f(0.0, hy, 0.0);
 glEnd();

 // top 
 glBegin(GL_LINE_STRIP);
	glVertex3f(0.0, 0.0, hz);
	glVertex3f(1.0, 0.0, hz);
	glVertex3f(1.0, hy, hz);
	glVertex3f(0.0, hy, hz);
 glEnd();

 // front
 glBegin(GL_LINE_STRIP);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(1.0, 0.0, 0.0);
	glVertex3f(1.0, 0.0, hz);
	glVertex3f(0.0, 0.0, hz);
 glEnd();

 // back
 glBegin(GL_LINE_STRIP);
	glVertex3f(0.0, hy, 0.0);
	glVertex3f(1.0, hy, 0.0);
	glVertex3f(1.0, hy, hz);
	glVertex3f(0.0, hy, hz);
 glEnd();

 // left
 glBegin(GL_LINE_STRIP);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, hz);
	glVertex3f(0.0, hy, hz);
	glVertex3f(0.0, hy, 0.0);
 glEnd();

 // right
 glBegin(GL_LINE_STRIP);
	glVertex3f(1.0, 0.0, 0.0);
	glVertex3f(1.0, 0.0, hz);
	glVertex3f(1.0, hy, hz);
	glVertex3f(1.0, hy, 0.0);
 glEnd();

 glTranslatef(0.0, hy - 1.0, hz - 1.0);
 glColor3f(1.0, 1.0, 1.0);
 glEnable(GL_BLEND);
 glEnable(GL_TEXTURE_2D);
}

SelectBox::SelectBox(BosonSprite*, BosonCanvas*, bool groupLeader)
{
 mDisplayList = 0;
 if (!mBoxData) {
	mBoxData = new SelectBoxData;
 }
}

SelectBox::~SelectBox()
{
}

void SelectBox::update(double div)
{
 mDisplayList = mBoxData->list(div);
}

#else

 // TODO

#endif // !NO_OPENGL


#if 0
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

//SelectBox::SelectBox(float x, float y, int width, int height, float z, QCanvas* canvas, bool leader)
SelectBox::SelectBox(BosonSprite* item, BosonCanvas* canvas, bool groupLeader)
//	: QCanvasSprite(0, canvas)
{
 mSprite = new QCanvasSprite(0, canvas);
 mWidth = width + 2 * SP_H_DISTANCE;
 mHeight = height + 2 * SP_V_DISTANCE;
 setZ(z + 1);
 mLeader = leader;

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
 QColor rectColor = (mLeader == false) ? Qt::white : Qt::red;
 painter.fillRect(boxWidth() - cornerLength(), barHeight() + SP_CORNER_BAR_DISTANCE, 
		cornerLength(), SP_THICK, 
		mask ? Qt::color1 : rectColor);
 painter.fillRect(boxWidth() - SP_THICK, barHeight() + SP_CORNER_BAR_DISTANCE, 
		SP_THICK, cornerLength(), 
		mask ? Qt::color1 : rectColor);

// upper left
 painter.fillRect(0, barHeight() + SP_CORNER_BAR_DISTANCE,
		cornerLength(), SP_THICK, 
		mask ? Qt::color1 : rectColor);
 painter.fillRect(0, barHeight() + SP_CORNER_BAR_DISTANCE,
		SP_THICK, cornerLength(), 
		mask ? Qt::color1 : rectColor);

// lower left
 painter.fillRect(0, boxHeight() - cornerLength(),
		SP_THICK, cornerLength(), 
		mask ? Qt::color1 : rectColor);
 painter.fillRect(0, boxHeight() - SP_THICK,
		cornerLength(), SP_THICK,
		mask ? Qt::color1 : rectColor);

// lower right
 painter.fillRect(boxWidth() - cornerLength(), boxHeight() - SP_THICK,
		cornerLength(), SP_THICK,
		mask ? Qt::color1 : rectColor);
 painter.fillRect(boxWidth() - SP_THICK, boxHeight() - cornerLength(),
		SP_THICK, cornerLength(),
		mask ? Qt::color1 : rectColor);
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
//	pix.fill(Qt::white);
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
 setFrame((int)((frames() - 1) * factor));
}

int SelectBox::barHeight()
{
 return 6;
}

int SelectBox::barWidth(int frame)
{
 double factor = (double)frame / (frames() - 1);
 return (int)(boxWidth() * factor - 2); // -2: the white frame around the box
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

void SelectBox::setVisible(bool v)
{
 if (mSprite) {
	mSprite->setVisible(v);
 }
}

void SelectBox::setZ(float z)
{
 if (mSprite) {
	mSprite->setZ((double)z);
 }
}

void SelectBox::setFrame(int f)
{
 if (mSprite) {
	mSprite->setFrame(f);
 }
}


#endif // 0
