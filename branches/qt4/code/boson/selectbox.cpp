/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2002 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "selectbox.h"

#include "../bomemory/bodummymemory.h"
#include "defines.h"
#include "botexture.h"
#include "bodebug.h"

#include <QImage>
#include <QPainter>
#include <QGLWidget>

#define POWER_LEVELS 15

SelectBoxData::SelectBoxData()
{
 mTexture = 0;
 mDisplayListCount = 0;
 mDisplayListBase = 0;
}

SelectBoxData::~SelectBoxData()
{
 delete mTexture;
 if (mDisplayListCount > 0 && mDisplayListBase) {
	glDeleteLists(mDisplayListBase, mDisplayListCount);
	mDisplayListBase = 0;
	mDisplayListCount = 0;
 }
}

GLuint SelectBoxData::list(float factor)
{
 if (factor > 1.0) {
	factor = 1.0;
 } else if (factor < 0.0) {
	factor = 0.0;
 }
 if (mDisplayListBase == 0) {
	loadBoxes();
 }
 if (mDisplayListBase == 0) {
	boError() << k_funcinfo << "Unable to generate a SelectBox for " << factor << endl;
	return 0;
 }
 if (mDisplayListCount == 0) {
	boError() << k_funcinfo << "no display lists loaded" << endl;
	return 0;
 }
 int list = (int)((mDisplayListCount - 1) * factor);
 if (list < 0) {
	boError() << k_funcinfo << "list < 0: " << list << endl;
	list = 0;
 } else if (list >= mDisplayListCount) {
	boError() << k_funcinfo << "list >= display list count: " << mDisplayListCount << " >= " << mDisplayListCount << endl;
	list = mDisplayListCount - 1;
 }
 return mDisplayListBase + list;
}

void SelectBoxData::loadBoxes()
{
 // TODO: we might want to use mipmaps here - interesting for big units, as well as for zooming
 mDisplayListCount = POWER_LEVELS;
 mDisplayListBase = glGenLists(mDisplayListCount);
 if (mTexture) {
	boWarning() << k_funcinfo << "textures loaded before" << endl;
	delete mTexture;
	mTexture = 0;
 }
// AB: the size is hardcoded. mipmaps might be VERY useful here!
// FIXME: Qt::red simply doesn't work - we need to use Qt::blue .. why???
// QImage image = KImageEffect::gradient(QSize(48, 6), Qt::red, Qt::green, KImageEffect::HorizontalGradient);
// QImage image = KImageEffect::gradient(QSize(48, 6), QColor(255,0,0), Qt::green, KImageEffect::HorizontalGradient);
 QLinearGradient gradient(QPointF(0.0, 0.0), QPointF(64.0, 0.0));
 gradient.setColorAt(0.0, Qt::red);
 gradient.setColorAt(1.0, Qt::green);
 QImage image(64, 1, QImage::Format_RGB32);
 QPainter painter(&image);
 painter.fillRect(0, 0, 64, 1, gradient);
 painter.end();
 image = QGLWidget::convertToGLFormat(image);

 mTexture = new BoTexture(image.bits(), image.width(), image.height(), BoTexture::UI);

 for (int i = 0; i < mDisplayListCount; i++) {
	glNewList(mDisplayListBase + i, GL_COMPILE);
		// ok, this is the quick and dirty way. correct would be to
		// adjust all vertices - but it doesn't really matter.
		// this ensures, that we render around the *center*
		// note: we render in the ceter of x/y but from bottom to top!
		glTranslatef(-0.5, -0.5, 0.0);
		drawCube();
		// We can't use BoTexture::bind(), because it's display list
		glBindTexture(GL_TEXTURE_2D, mTexture->id());
		drawHealthBar(i, mDisplayListCount);
	glEndList();
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
}

void SelectBoxData::drawHealthBar(int frame, int displayListCount)
{
 GLfloat texLength = ((float)frame) / (float)(displayListCount - 1);
 GLfloat length = 1.0 * texLength; // y-direction
 GLfloat hy = 0.15; // height in y-direction
 GLfloat hz = 0.15; // height in z-direction

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
 glColor3ub(255, 255, 255);
 glEnable(GL_TEXTURE_2D);
}


