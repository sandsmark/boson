/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/gl/ugl_graphics.cpp
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

#include "ufo/gl/ugl_graphics.hpp"

#include "ufo/ucontext.hpp"

//#include "ufo/ufo_gl.hpp"

//#include "ufo/gl/ufo_gl.hpp"
#include "ufo/gl/ugl_driver.hpp"
#include "ufo/gl/ugl_image.hpp"

#include "ufo/util/ucolor.hpp"
#include "ufo/util/udimension.hpp"
#include "ufo/util/uinsets.hpp"

#include "ufo/font/ufont.hpp"
#include "ufo/font/ufontrenderer.hpp"

#include "ufo/widgets/uwidget.hpp"
#include "ufo/widgets/urootpane.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UGL_Graphics, UGraphics)

UGL_Graphics::UGL_Graphics(UContext * context)
	: m_context(context)
	, m_color()//UColor::black)
	, m_clearColor()//UColor::black)
	, m_font(NULL) // FIXME ! insert valid value
	, m_clipRect()
{
	//m_color->reference();
	//m_clearColor->reference();
}

UGL_Graphics::~UGL_Graphics() {
	//m_color->unreference();
	//m_clearColor->unreference();
	if (m_font) {
		m_font->unreference();
	}
}

UContext *
UGL_Graphics::getContext() const {
	return m_context;
}

void
UGL_Graphics::resetDeviceAttributes() {
	ugl_driver->glDisable(GL_TEXTURE_2D);
	ugl_driver->glDisable(GL_DEPTH_TEST);

	ugl_driver->glShadeModel(GL_FLAT);

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	ugl_driver->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

void
UGL_Graphics::resetDeviceViewMatrix() {
	UContext * context = getContext();

	const URectangle & deviceBounds = context->getDeviceBounds();
	const URectangle & contextBounds = context->getContextBounds();

	ugl_driver->glViewport(
		contextBounds.x,
		deviceBounds.h - contextBounds.y - contextBounds.h,
		contextBounds.w,
		contextBounds.h
	);

	ugl_driver->glMatrixMode(GL_PROJECTION);
	ugl_driver->glLoadIdentity();

	// set the ortho projection matrix using an offset of 3/8
	// to eliminate graphic bugs with coordinates drawn at pixel boundaries.
	// thanks to Paul Martz
	ugl_driver->glOrtho(
		-0.375,
		contextBounds.w - 0.375,
		contextBounds.h - 0.375,
		-0.375,
		-100,
		100
	);


	ugl_driver->glMatrixMode(GL_MODELVIEW);
	ugl_driver->glLoadIdentity();
}

URectangle
UGL_Graphics::mapToDevice(const URectangle & rect) {
	// FIXME
	// this works only when the graphic viewport was set by the graphics object
	//const UPoint & pos = w->pointToRootPoint(rect.x, rect.y);

	int vport[4];
	ugl_driver->glGetIntegerv(GL_VIEWPORT, vport);

	// y-flip
	return URectangle(vport[0] + rect.x, vport[1] + vport[3] - rect.y - rect.h, rect.w, rect.h);
}

URectangle
UGL_Graphics::mapFromDevice(const URectangle & rect) {
	int vport[4];
	ugl_driver->glGetIntegerv(GL_VIEWPORT, vport);

	// FIXME
	// this is simply wrong
	// y-flip
	return URectangle(rect.x - vport[0], - vport[1] - vport[3] + rect.y + rect.h, rect.w, rect.h);
}


void
UGL_Graphics::flush() {
	ugl_driver->glFlush();
}

void
UGL_Graphics::clear() {
	// FIXME what about UFO contexts which do not cover the whole screen?
	ugl_driver->glClear(GL_COLOR_BUFFER_BIT);
}


// FIXME
// should we use some 4-byte hacking?
static inline void
my_memcpy(unsigned char * dest, unsigned char * src, int num) {
	for (int i = 0; i < num ; ++i) {
		*(dest++) = *(src++);
	}
}

UImageIO *
UGL_Graphics::dump() {
	UContext * context = getContext();

	const URectangle & deviceBounds = context->getDeviceBounds();
	const URectangle & contextBounds = context->getContextBounds();

	GLubyte * pixels = new GLubyte[contextBounds.w * contextBounds.h * 4];

	// read from the back buffer
	// FIXME
	ugl_driver->glReadBuffer(GL_BACK);

	ugl_driver->glReadPixels(
		contextBounds.x,
		deviceBounds.h - contextBounds.y - contextBounds.h,
		contextBounds.w,
		contextBounds.h,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		pixels
	);

	// y-flip screen
	const int pitch = contextBounds.w * 4;

	GLubyte * tmprow = new GLubyte[pitch];

	GLubyte * ptr[2] = { pixels,
		pixels + pitch * contextBounds.h - pitch };

	while (ptr[0] < ptr[1]) {
		my_memcpy(tmprow, ptr[0], pitch);
		my_memcpy(ptr[0], ptr[1], pitch);
		my_memcpy(ptr[1], tmprow, pitch);
		ptr[0] += pitch;
		ptr[1] -= pitch;
	}

	UImageIO * ret = new UImageIO(pixels, contextBounds.w, contextBounds.h, 4);

	// clean up
	delete[] (tmprow);
	delete[] (pixels);

	return ret;
}

void
UGL_Graphics::setColor(const UColor & color) {
	// color might be overwritten by system (or ui)
	//if (color && m_color != color) {
	//if (color) { // && !m_color->equals(color)) {
	//	m_color->unreference();
		m_color = color;
	//	m_color->reference();

		ugl_driver->glColor3fv(m_color.getFloat());
	//}
}

const UColor &
UGL_Graphics::getColor() const {
	return m_color;
}

void
UGL_Graphics::setClearColor(const UColor & clearColor) {
	//if (clearColor) {
	//	m_clearColor->unreference();
		m_clearColor = clearColor;
	//	m_clearColor->reference();

		//ugl_driver->glClearColor(m_clearColor->getFloat());
	//}
}

const UColor &
UGL_Graphics::getClearColor() const {
	return m_clearColor;
}

void
UGL_Graphics::setFont(const UFont * font) {
	if (font && m_font != font) {
		//swapChildren(m_font, font);
		if (m_font) {
			m_font->unreference();
		}
		m_font = font;
		m_font->reference();
	}
}

const UFont *
UGL_Graphics::getFont() const {
	return m_font;
}

//
// clipping
//
/*
void
UGL_Graphics::pushClipRect() {
	glPushAttrib(GL_SCISSOR_BIT);
	glEnable(GL_SCISSOR_TEST);
}

void
UGL_Graphics::popClipRect() {
	glPopAttrib();
}
*/
void
UGL_Graphics::setClipRect(const URectangle & rect) {
	m_clipRect = rect;

	if (!m_clipRect.isEmpty()) {
		ugl_driver->glEnable(GL_SCISSOR_TEST);
		//URectangle clipRect = m_context->mapToOpenGL(rect, m_context->getRootPane());
		URectangle clipRect = mapToDevice(rect);

		ugl_driver->glScissor(clipRect.x, clipRect.y, clipRect.w, clipRect.h);
	} else {
		ugl_driver->glDisable(GL_SCISSOR_TEST);
	}
}

URectangle
UGL_Graphics::getClipRect() const {
	return m_clipRect;
}

void
UGL_Graphics::drawString(const std::string & text, int x, int y) {
	m_font->getRenderer()->drawString(this, text.data(), text.length(), x, y);
}

UDimension
UGL_Graphics::getStringSize(const std::string & text) {
	const UFontMetrics * metrics = m_font->getFontMetrics();
	return UDimension(metrics->getStringWidth(text), metrics->getHeight());
}

//
// transformations
//

void
UGL_Graphics::translate(float x, float y) {
	ugl_driver->glTranslatef(x, y, 0);
}

float
UGL_Graphics::getTranslationX() const {
	return m_translationX;
}

float
UGL_Graphics::getTranslationY() const {
	return m_translationY;
}

//
// basic drawing operations
//

void
UGL_Graphics::drawRect(const URectangle & rect) {
	ugl_driver->glBegin(GL_LINE_LOOP);
	ugl_driver->glVertex2i(rect.x, rect.y);
	ugl_driver->glVertex2i(rect.x + rect.w, rect.y);
	ugl_driver->glVertex2i(rect.x + rect.w, rect.y + rect.h);
	ugl_driver->glVertex2i(rect.x, rect.y + rect.h);
	ugl_driver->glVertex2i(rect.x, rect.y);
	ugl_driver->glEnd();
}

void
UGL_Graphics::fillRect(const URectangle & rect) {
	ugl_driver->glRecti(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);
}

void
UGL_Graphics::drawLine(const UPoint & p1, const UPoint & p2) {
	ugl_driver->glBegin(GL_LINES);
	ugl_driver->glVertex2i(p1.x, p1.y);
	ugl_driver->glVertex2i(p2.x, p2.y);
	ugl_driver->glEnd();
}

void
UGL_Graphics::drawLines(int x[], int y[], int nPoints) {
	ugl_driver->glBegin(GL_LINES);
	for (int i = 0; i < nPoints; ++i) {
		ugl_driver->glVertex2i(x[i], y[i]);
	}
	ugl_driver->glEnd();
}

void
UGL_Graphics::drawPolygon(int x[], int y[], int nPoints) {
	ugl_driver->glBegin(GL_LINE_STRIP);
	for (int i = 0; i < nPoints; ++i) {
		ugl_driver->glVertex2i(x[i], y[i]);
	}
	ugl_driver->glEnd();
}

void
UGL_Graphics::fillPolygon(int x[], int y[], int nPoints) {
	ugl_driver->glBegin(GL_POLYGON);
	for (int i = 0; i < nPoints; ++i) {
		ugl_driver->glVertex2i(x[i], y[i]);
	}
	ugl_driver->glEnd();
}

void
UGL_Graphics::drawImage(UImage * image, const UPoint & p) {
	UGL_Image * tex = static_cast<UGL_Image*>(image);
	tex->paint(this, p);
}

void
UGL_Graphics::drawImage(UImage * image, const URectangle & rect) {
	UGL_Image * tex = static_cast<UGL_Image*>(image);
	tex->paint(this, rect);
}

void
UGL_Graphics::drawSubImage(UImage * image,
		const URectangle & srcRect, const UPoint & destLocation) {
	UGL_Image * tex = static_cast<UGL_Image*>(image);
	tex->paintSubImage(this, srcRect, destLocation);
}

void
UGL_Graphics::drawSubImage(UImage * image,
		const URectangle & srcRect, const URectangle & destRect) {
	UGL_Image * tex = static_cast<UGL_Image*>(image);
	tex->paintSubImage(this, srcRect, destRect);
}
