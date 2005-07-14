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
#include "bosonconfig.h"
#include "bosonglwidget.h"
#include "bodebug.h"

#include <qimage.h>

#include <GL/gl.h>
#include <GL/glu.h>

#ifndef GL_VERSION_1_1
#warning Your system lacks OpenGL 1.1 - this version introduced texture objects. Do you really think you can run an OpenGL game without support for texture objects ?? ;)
#endif

// warning: mAllTextures is *not* setAutoDelete(true) !
QIntDict<BoTextureInfo> BosonTextureArray::mAllTextures = QIntDict<BoTextureInfo>();

class BoTextureInfo
{
public:
	BoTextureInfo()
	{
		mTexture = 0;
		mMipmap = false;
	}
	GLuint mTexture;
	bool mMipmap;
};

BosonTextureArray::BosonTextureArray()
{
 init();
}

BosonTextureArray::BosonTextureArray(QValueList<QImage> images, bool useMipmaps, bool useAlpha)
{
 init();
 if (!createTextures(images, useMipmaps, useAlpha)) {
	boWarning(110) << k_funcinfo << "Could not create textures" << endl;
 }
}

void BosonTextureArray::init()
{
 boDebug(110) << k_funcinfo << endl;
 mTextures = 0;
 mCount = 0;
}

BosonTextureArray::~BosonTextureArray()
{
 boDebug(110) << k_funcinfo << endl;
 if (mTextures && mCount) {
	glBindTexture(GL_TEXTURE_2D, 0); // in case we want to delete currently bound texture
	for (unsigned int i = 0; i < mCount; i++) {
		BoTextureInfo* t = mAllTextures.take(mTextures[i]);
		delete t;
	}
	boDebug(110) << k_funcinfo << "delete " << mCount << " texture objects" << endl;
	glDeleteTextures(mCount, &mTextures[0]);
	delete[] mTextures;
 } else {
	boDebug(110) << k_funcinfo << "no textures allocated" << endl;
 }
 boDebug(110) << k_funcinfo << "done" << endl;
}

bool BosonTextureArray::createTexture(const QImage& image, GLuint texture, bool useMipmaps, bool useAlpha)
{
 if (!BoContext::currentContext()) {
	boError(110) << k_funcinfo << "NULL current context!!" << endl;
	return false; // baaaad - we should delay loading or so
 }
 GLenum error = glGetError();
 if (error != GL_NO_ERROR) {
	boError(110) << k_funcinfo << "OpenGL Error before loading texture" << endl;
 }
 if (image.isNull()) {
	boError(110) << k_funcinfo << "NULL image" << endl;
	return false;
 }

 QImage buffer;

 int w = nextPower2(image.width());
 int h = nextPower2(image.height());

 if (w != image.width() || h != image.height()) {
	buffer = image.scale(w, h, QImage::ScaleFree);
 } else {
	buffer = image;
 }
 if (buffer.isNull()) {
	boWarning(110) << k_funcinfo << "using fallback image" << endl;
	buffer = QImage(w, h, 32);
	buffer.fill(Qt::red.rgb()); // fallback
	// should not appear at all...
 }

 buffer = BosonGLWidget::convertToGLFormat(buffer);
 glBindTexture(GL_TEXTURE_2D, texture);

 // note: width and height MUST be a power of 2!! they should be <= 256
 // (we already scaled above - this is rather meant as a reminder)

 // AB: performance: GL_UNSIGNED_BYTE is said to be the fastest format
 // (usually!!) - so don't change it :)

 // AB: performance: we don't use the alpha component for most textures. so we
 // could replace the first GL_RGBA parameter by GL_RGB (leave the second at
 // GL_RGBA!). note that at least the cursor needs alpha!

 int internalFormat = useAlpha ? GL_RGBA : GL_RGB;
 if (useMipmaps) {
	resetMipmapTexParameter();
	int error = gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat,
			buffer.width(), buffer.height(), GL_RGBA,
			GL_UNSIGNED_BYTE, buffer.bits());
	if (error) {
		boWarning(110) << k_funcinfo << "gluBuild2DMipmaps returned error: " << error << endl;
	}
 } else {
	// AB: we dont build mipmaps for cell and cursor textures (e.g.) those
	// are not so much quality relevant
	// TODO: performance: combine several textures into a single one and
	// adjust the coordinates in glTexCoord
	resetTexParameter();

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
			buffer.width(), buffer.height(), 0, GL_RGBA,
			GL_UNSIGNED_BYTE, buffer.bits());
 }

 error = glGetError();
 if (error != GL_NO_ERROR) {
	boError(110) << k_funcinfo << "OpenGL Error when loading texture " << texture << endl;
	return false;
 }
 return true;
}

void BosonTextureArray::resetAllTexParameter()
{
 boDebug(110) << k_funcinfo << "reset " << mAllTextures.count() << " textures" << endl;
 QIntDictIterator<BoTextureInfo> it(mAllTextures);
 for (; it.current(); ++it) {
	GLuint tex = it.currentKey();
	glBindTexture(GL_TEXTURE_2D, tex);
	if (it.current()->mMipmap) {
		resetMipmapTexParameter();
	} else {
		resetTexParameter();
	}
 }
}

void BosonTextureArray::resetTexParameter()
{
 glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLenum)boConfig->magnificationFilter());
 glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLenum)boConfig->minificationFilter());

}

void BosonTextureArray::resetMipmapTexParameter()
{
 glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLenum)boConfig->magnificationFilter());
 // note: GL_*_MIPMAP_* is slower! GL_NEAREST would be fastest
 glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLenum)boConfig->mipmapMinificationFilter());

}

bool BosonTextureArray::createTextures(QValueList<QImage> images, bool useMipmaps, bool useAlpha)
{
 GLenum error = glGetError();
 if (error != GL_NO_ERROR) {
	boError(110) << k_funcinfo << "OpenGL Error before loading textures: " << gluErrorString(error) << endl;
 }
 // TODO: performance: use smaller textures!! so we can store more textures in
 // texture memory and don't need to swap from/to system memory
 if (mTextures || mCount) {
	boDebug(110) << k_funcinfo << "textures already generated?!" << endl;
	return false;
 }
 if (!BoContext::currentContext()) {
	boError(110) << k_funcinfo << "NULL current context!!" << endl;
	return false; // baaaad - we should delay loading or so
 }
 QImage buffer;
 mCount = images.count();
 mTextures = new GLuint[mCount];
 glGenTextures(mCount, &mTextures[0]);
 for (unsigned int i = 0; i < mCount; i++) {
	BoTextureInfo* t = new BoTextureInfo;
	t->mTexture = mTextures[i];
	t->mMipmap = useMipmaps;
	mAllTextures.insert(mTextures[i], t);
 }

// boDebug(110) << k_funcinfo << "count=" << mCount << endl;

 for (unsigned int i = 0; i < mCount; i++) {
	createTexture(images[i], mTextures[i], useMipmaps, useAlpha);
 }
 return true;
}

int BosonTextureArray::nextPower2(int n)
{
 if (n <= 0) {
	return 1;
 }
 int i = 1;
 while (n > i) {
	i *= 2;
 }
 return i;
}
