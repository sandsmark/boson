/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosontexturearray.h"

#include <qimage.h>
#include <qgl.h>

#include <kdebug.h>

BosonTextureArray::BosonTextureArray()
{
 init();
}

BosonTextureArray::BosonTextureArray(QValueList<QImage> images, GLenum mode)
{
 init();
 createTextures(images, mode);
}

void BosonTextureArray::init()
{
 mTextures = 0;
 mCount = 0;
 mWidths = 0;
 mHeights = 0;
}

BosonTextureArray::~BosonTextureArray()
{
// kdDebug() << k_funcinfo << endl;
 if (mTextures && mCount) {
	glDeleteTextures(mCount, &mTextures[0]);
	delete[] mTextures;
	delete[] mWidths;
	delete[] mHeights;
 } else {
	kdDebug() << k_funcinfo << "no textures allocated" << endl;
 }
// kdDebug() << k_funcinfo << "done" << endl;
}

bool BosonTextureArray::createTextures(QValueList<QImage> images, GLenum mode)
{
 // TODO: performance: use smaller textures!! so we can store more textures in
 // texture memory and don't need to swap from/to system memory
 if (mTextures || mCount) {
	return false;
 }
 QImage buffer;
 mMode = mode;
 mCount = images.count();
 mTextures = new GLuint[mCount];
 mWidths = new int[mCount];
 mHeights = new int[mCount];
 glGenTextures(mCount, &mTextures[0]);

// kdDebug() << k_funcinfo << "count=" << mCount << endl;

 for (unsigned int i = 0; i < mCount; i++) {
	//FIXME: minimum size should be 64x64!!

	int w = nextPower2(images[i].width());
	int h = nextPower2(images[i].height());

	if (w != images[i].width() || h != images[i].height()) {
		buffer = images[i].scale(w, h, QImage::ScaleFree);
	} else {
		buffer = images[i];
	}
	if (buffer.isNull()) {
		kdWarning() << k_funcinfo << "using fallback image" << endl;
		buffer = QImage(w, h, 32);
		buffer.fill(Qt::red.rgb()); // fallback
		// should not appear at all... - do we need an alpha buffer?
	}

	// the original size (NOT the texture size)
	mWidths[i] = images[i].width();
	mHeights[i] = images[i].height();

	buffer = QGLWidget::convertToGLFormat(buffer);
	glBindTexture(mMode, mTextures[i]);

        // note: width and height MUST be a power of 2!! they must be >= 64
	// and should be <= 256
	// (we already scaled above - this is rather meant as a reminder)
	// FIXME: all params (e.g. GL_RGBA) are hardcoded
	// AB: performance: GL_UNSIGNED_BYTE is said to be the fastest format
	// (usually!!) - so don't change it :)
	glTexImage2D(mMode, 0, GL_RGBA, buffer.width(), 
			buffer.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 
			buffer.bits());

	// FIXME: this is hardcoded, too...
	// FIXME: performance: GL_NEAREST is said to be fastest, GL_LINEAR
	// second fastest
	// FIXME: performance: do we gain anything by using mipmaps here?
	// probably...
	// TODO: performance: combine several textures into a single one and
	// adjust the coordinates in glTexCoord
	// TODO: performance: glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST)
	// is fast - do we loose anything from it? is this the correct file to
	// place it in? can it go to initializeGL() ?
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 }
}

int BosonTextureArray::nextPower2(int n) const
{
 // FIXME: texture must be >= 64x64
 // maybe we should always return a value >= 64
 if (n <= 0) {
	return 1;
 }
 int i = 1;
 while (n > i) {
	i *= 2;
 }
 return i;
}

void BosonTextureArray::setHotspots(QPointArray points)
{
 if (!mCount) {
	kdWarning() << k_funcinfo << "load the textures first!" << endl;
	return;
 }
 if (points.count() != mCount) {
	kdError() << k_funcinfo << "invalid list count" << endl;
	return;
 }
 mHotspots = points.copy();
}

