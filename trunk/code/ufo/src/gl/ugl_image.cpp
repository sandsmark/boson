/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/gl/ugl_image.cpp
    begin             : Sat Oct 18 2003
    $Id$
 ***************************************************************************/

/***************************************************************************
 *  This library is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include "ufo/gl/ugl_image.hpp"

#include "ufo/ugraphics.hpp"
//#include "ufo/ucontext.hpp"
#include "ufo/ucontextgroup.hpp"
//#include "ufo/utoolkit.hpp"

#include "ufo/gl/ugl_driver.hpp"
#include "ufo/image/uimagefilter.hpp"

#include "ufo/signals/ufo_signals.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UGL_Image, UImage)

UGL_Image::UGL_Image(const std::string & fileName, bool autoRefresh,
		GLenum imageFormat, GLenum internalFormat)
	: m_index(0)
	, m_imageFormat(imageFormat)
	, m_internalFormat(internalFormat)
	, m_size()
	, m_imageComponents(0)
	, m_isValid(false)
	, m_imageData(NULL)
	//, m_contextGroup(NULL)
{
	/*UContext * context = UToolkit::getToolkit()->getCurrentContext();
	if (!context) {
		uError() << "UGL_Image: There is no current UFO context\n";
		return;
	}*/
	//m_contextGroup = context->getContextGroup();

	UImageIO * imageIO = new UImageIO(fileName);
	imageIO->reference();

	m_size = imageIO->getSize();
	m_imageComponents = imageIO->getImageComponents();

	createGLTexture(imageIO->getPixels(), imageIO->getImageComponents(),
		imageFormat, internalFormat);

	if (autoRefresh) {
		m_imageData = imageIO;
		//context->sigRefresh().connect(slot(*this, &UGL_Image::refresh));
		getContextGroup()->addVolatileData(this);
	} else {
		imageIO->unreference();
	}
}

UGL_Image::UGL_Image(UImageIO * imageIO, bool autoRefresh,
		GLenum imageFormat, GLenum internalFormat)
	: m_index(0)
	, m_imageFormat(imageFormat)
	, m_internalFormat(internalFormat)
	, m_size()
	, m_imageComponents(0)
	, m_isValid(false)
	, m_imageData(NULL)
{
	imageIO->reference();

	m_size = imageIO->getSize();
	m_imageComponents = imageIO->getImageComponents();

	createGLTexture(imageIO->getPixels(), imageIO->getImageComponents(),
		imageFormat, internalFormat);

	if (autoRefresh) {
		m_imageData = imageIO;
		//UContext * context = UToolkit::getToolkit()->getCurrentContext();
		//context->sigRefresh().connect(slot(*this, &UGL_Image::refresh));
		getContextGroup()->addVolatileData(this);
	} else {
		imageIO->unreference();
	}
}

UGL_Image::~UGL_Image() {
	disposeGL();
	dispose();
	// FIXME:
	// should this be handled by a dispose routine?
	//if (m_isValid) {
	//	glDeleteTextures(1, &m_index);
	//}
}

//
// implements UImage
//

int
UGL_Image::getImageWidth() const {
	return m_size.w;
}

int
UGL_Image::getImageHeight() const {
	return m_size.h;
}

int
UGL_Image::getImageComponents() const {
	return m_imageComponents;
}
/*
UContextGroup *
UGL_Image::getContextGroup() const {
	return m_contextGroup;
}
*/
unsigned long
UGL_Image::handle() const {
	return m_index;
}

//
// public methods
//

const UDimension &
UGL_Image::getSize() const {
	return m_size;
}

/*
int
UGL_Image::getWidth() const {
	return m_size.w;
}

int
UGL_Image::getHeight() const {
	return m_size.h;
}
*/


//
// painting
//

void
UGL_Image::paint(UGraphics * g) const {
	paint(g, URectangle(0, 0, m_size.w, m_size.h));
}

void
UGL_Image::paint(UGraphics * g, const UPoint & location) const {
	paint(g, URectangle(location.x, location.y, m_size.w, m_size.h));
}

void
UGL_Image::paint(UGraphics * g, const URectangle & rect) const {
	if (m_isValid) {
		ugl_driver->glEnable(GL_TEXTURE_2D);
		ugl_driver->glBindTexture(GL_TEXTURE_2D, m_index);

		ugl_driver->glColor4f(1, 1, 1, 1);

		if (m_imageComponents == 2 || m_imageComponents == 4) {
			//ugl_driver->glEnable(GL_BLEND);
			//ugl_driver->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		ugl_driver->glBegin(GL_QUADS);
		{
			ugl_driver->glTexCoord2f(0.0f, 0.0f);
			ugl_driver->glVertex2i(rect.x , rect.y);
			ugl_driver->glTexCoord2f(0.0f, 1.0f);
			ugl_driver->glVertex2i(rect.x , rect.y + rect.h);
			ugl_driver->glTexCoord2f(1.0f, 1.0f);
			ugl_driver->glVertex2i(rect.x + rect.w, rect.y + rect.h);
			ugl_driver->glTexCoord2f(1.0f, 0.0f);
			ugl_driver->glVertex2i(rect.x + rect.w, rect.y);
		}
		ugl_driver->glEnd();

		if (m_imageComponents == 2 || m_imageComponents == 4) {
			//ugl_driver->glDisable(GL_BLEND);
		}

		ugl_driver->glDisable(GL_TEXTURE_2D);
	}
}


void
UGL_Image::paintSubImage(UGraphics * g, const URectangle & rect,
		const UPoint & dest) const {
	paintSubImage(g, rect, URectangle(dest.x, dest.y, m_size.w, m_size.h));
}


void
UGL_Image::paintSubImage(UGraphics * g, const URectangle & rect,
		const URectangle & dest) const {
	if (m_isValid) {
		ugl_driver->glEnable(GL_TEXTURE_2D);
		ugl_driver->glBindTexture(GL_TEXTURE_2D, m_index);

		if (m_imageComponents == 2 || m_imageComponents == 4) {
			//ugl_driver->glEnable(GL_BLEND);
			//ugl_driver->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		//ugl_driver->glColor4f(1, 1, 1, 1);
		float texx = ((float)rect.x) / m_size.w;
		float texy = ((float)rect.y) / m_size.h;
		float texw = ((float)rect.x + rect.w) / m_size.w;
		float texh = ((float)rect.y + rect.h) / m_size.h;

		ugl_driver->glBegin(GL_QUADS);
		{
			ugl_driver->glTexCoord2f(texx, texy);
			ugl_driver->glVertex2i(dest.x , dest.y);
			ugl_driver->glTexCoord2f(texx, texh);
			ugl_driver->glVertex2i(dest.x , dest.y + dest.h);
			ugl_driver->glTexCoord2f(texw, texh);
			ugl_driver->glVertex2i(dest.x + dest.w, dest.y + dest.h);
			ugl_driver->glTexCoord2f(texw, texy);
			ugl_driver->glVertex2i(dest.x + dest.w, dest.y);
		}
		ugl_driver->glEnd();

		if (m_imageComponents == 2 || m_imageComponents == 4) {
			//ugl_driver->glDisable(GL_BLEND);
		}

		ugl_driver->glDisable(GL_TEXTURE_2D);
	}
}


void
UGL_Image::dispose() {
	if (m_imageData) {
		m_imageData->unreference();
		m_imageData = NULL;
	}
}

void
UGL_Image::disposeGL() {
	if (m_isValid) {
		ugl_driver->glDeleteTextures(1, &m_index);
		m_isValid = false;
	}
}

void
UGL_Image::refresh() {
	disposeGL();
	if (m_imageData) {
		createGLTexture(m_imageData->getPixels(), m_imageData->getImageComponents(),
			m_imageFormat, m_internalFormat);
	}
}


//
// private methods
//

int
UGL_Image::round2(int n) {
	int m;

	for (m = 1; m < n; m *= 2)
		;

	/* m>=n */
	if (m - n <= n - m / 2) {
		return m;
	} else {
		return m / 2;
	}
}

#ifndef GL_BGR
# ifdef GL_BGR_EXT
#  define GL_BGR GL_BGR_EXT
# else
    // dirty workaround
#  define GL_BGR 0x80E0
# endif
#endif

#ifndef GL_BGRA
# ifdef GL_BGRA_EXT
#  define GL_BGRA GL_BGRA_EXT
# else
    // dirty workaround
#  define GL_BGRA 0x80E1
# endif
#endif
bool
UGL_Image::createGLTexture(GLubyte * dataA, int componentsA,
		GLenum imageFormatA, GLenum internalFormatA) {

	GLubyte * imageData;
	int w, h;
	GLint maxTexSize;

	// set standard format
	m_imageFormat = imageFormatA;

	// check if desired format is valid
	if (componentsA == 4) {
		// are there other opengl formats with 4 components?
		if ((m_imageFormat != GL_RGBA) && (m_imageFormat != GL_BGRA)) {
			m_imageFormat = GL_RGBA;
		}
	} else if (componentsA == 3) {
		// are there other opengl formats with 3 components?
		if ((m_imageFormat != GL_RGB) && (m_imageFormat != GL_BGR)) {
			m_imageFormat = GL_RGB;
		}
	} else if (componentsA == 2) {
		// are there other opengl formats with 2 components?
		m_imageFormat = GL_LUMINANCE_ALPHA;
	} else { // if (componentsA == 1) {
		// are there other opengl formats with 1 components?
		if ((m_imageFormat != GL_COLOR_INDEX) &&
				(m_imageFormat != GL_STENCIL_INDEX) &&
				(m_imageFormat != GL_DEPTH_COMPONENT) &&
				(m_imageFormat != GL_RED) &&
				(m_imageFormat != GL_GREEN) &&
				(m_imageFormat != GL_BLUE) &&
				(m_imageFormat != GL_ALPHA) &&
				(m_imageFormat != GL_LUMINANCE)) {
			m_imageFormat = GL_LUMINANCE;
		}
	}

	if (internalFormatA == 0) {
		//ok, this is deprecated since opengl1.1 but
		// is there another quick (and not dirty) solution?
		internalFormatA = componentsA;
	}

	ugl_driver->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);

	w = round2(m_size.w);
	if (w > maxTexSize) {
		w = maxTexSize;
	}

	h = round2(m_size.h);
	if (h > maxTexSize) {
		h = maxTexSize;
	}

	imageData = dataA;

	if (w != m_size.w || h != m_size.h) {
		imageData = new uint8_t[w * h * componentsA];
		UImageFilter::scale(componentsA, m_size.w, m_size.h, dataA, w, h, imageData);

		/*
		GLint error = ugl_driver->gluScaleImage(m_imageFormat,
			m_size.w, m_size.h,
			GL_UNSIGNED_BYTE,
			dataA,
			w, h,
			GL_UNSIGNED_BYTE,
			imageData
		);
		if (error) {
			uError() << "UGL_Image: GLU at scaling image says: Error code:  "
			<< error << " (" << ugl_driver->gluErrorString(error) << ")\n";
			return false;
		}
		*/
		// resize the size object
		//m_size.w = w;
		//m_size.h = h;
	}

	ugl_driver->glGenTextures(1, &m_index);
	ugl_driver->glBindTexture(GL_TEXTURE_2D, m_index);   // 2d texture (x and y size)

	// linear filtered texture


	// ??
	ugl_driver->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// repeat texture if tex is bigger ?
	ugl_driver->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	ugl_driver->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// scale linearly when image bigger than texture
	ugl_driver->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// scale linearly when image smalled than texture
	ugl_driver->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// ??
	ugl_driver->glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

/*
	if (mipmaps) {
		// a mipmap texture
		ugl_driver->glTexParameteri( GL_TEXTURE_2D,
		                 GL_TEXTURE_MIN_FILTER,
		                 GL_LINEAR_MIPMAP_LINEAR );
		ugl_driver->glTexParameteri( GL_TEXTURE_2D,
		                 GL_TEXTURE_MAG_FILTER,
		                 GL_LINEAR_MIPMAP_LINEAR );

		ugl_driver->gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormatA, m_size.w, m_size.h,
		                  m_imageFormat, GL_UNSIGNED_BYTE, imageData);

		m_isValid = true;
	} else {*/
		ugl_driver->glTexImage2D(GL_TEXTURE_2D, 0, internalFormatA,
		//m_size.w, m_size.h,
		w, h,
		             0, m_imageFormat, GL_UNSIGNED_BYTE, imageData);

		m_isValid = true;
	//}
	if (dataA != imageData) {
		delete[] (imageData);
	}
	return true;
}
