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

#include "defines.h"
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
		// ok, this is the quick and dirty way. correct would be to
		// adjust all vertices - but it doesn't really matter.
		// this ensures, that we render around the *center*
		// note: we render in the ceter of x/y but from bottom to top!
		glTranslatef(-0.5, -0.5, 0.0);
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
 glColor3f(1.0, 1.0, 1.0);
 glEnable(GL_BLEND);
 glEnable(GL_TEXTURE_2D);
}

SelectBox::SelectBox(BosonItem*, BosonCanvas*, bool groupLeader)
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

