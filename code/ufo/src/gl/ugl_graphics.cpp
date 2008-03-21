/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
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
#include "ufo/font/ufontmetrics.hpp"

#include "ufo/widgets/uwidget.hpp"
#include "ufo/widgets/urootpane.hpp"

#include "ufo/uvertexarray.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UGL_Graphics, UGraphics)

UGL_Graphics::UGL_Graphics(UContext * context)
	: m_context(context)
	, m_color()//UColor::black)
	, m_clearColor()//UColor::black)
	, m_font()
	, m_clipRect(URectangle::invalid)
	, m_translationX(0)
	, m_translationY(0)
{
}

UGL_Graphics::~UGL_Graphics() {
}

UContext *
UGL_Graphics::getContext() const {
	return m_context;
}

void
UGL_Graphics::resetDeviceAttributes() {
	ugl_driver->glDisable(GL_TEXTURE_2D);
	ugl_driver->glDisable(GL_DEPTH_TEST);
	ugl_driver->glDisable(GL_LIGHTING);
	ugl_driver->glCullFace(GL_FRONT);

	ugl_driver->glShadeModel(GL_FLAT);

	ugl_driver->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	//ugl_driver->glEnable(GL_BLEND);
	//ugl_driver->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void
UGL_Graphics::resetDeviceViewMatrix() {
	UContext * context = getContext();

	URectangle deviceBounds;
	URectangle contextBounds;
	if (context) {
		deviceBounds = context->getDeviceBounds();
		contextBounds = context->getContextBounds();
	} else {
		// assume full gl viewport
		int vport[4];
		ugl_driver->glGetIntegerv(GL_VIEWPORT, vport);
		deviceBounds = URectangle(vport[0], vport[1] + vport[3], vport[2], vport[3]);
		contextBounds = deviceBounds;
	}

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
	// see the red book, appendix ?
	ugl_driver->glOrtho(
		0,
		contextBounds.w,
		contextBounds.h,
		0,
		-100,
		100
	);
	//ugl_driver->glTranslatef(0.375, 0.375, 0);

	ugl_driver->glMatrixMode(GL_MODELVIEW);
	ugl_driver->glLoadIdentity();

	ugl_driver->glTranslatef(getTranslationX(), getTranslationY(), 0);
}


URectangle
UGL_Graphics::mapToDevice(const URectangle & rect) {
	int vport[4];
	ugl_driver->glGetIntegerv(GL_VIEWPORT, vport);

	// y-flip
	return URectangle(vport[0] + rect.x, vport[1] + vport[3] - rect.y - rect.h, rect.w, rect.h);
}

URectangle
UGL_Graphics::mapFromDevice(const URectangle & rect) {
	int vport[4];
	ugl_driver->glGetIntegerv(GL_VIEWPORT, vport);

	// y-flip
	return URectangle(rect.x - vport[0], - vport[1] - vport[3] + rect.y + rect.h, rect.w, rect.h);
}


void
UGL_Graphics::begin() {
	// push all changing attributes
	ugl_driver->glPushAttrib(GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT |
		GL_LIGHTING_BIT | GL_LINE_BIT | GL_LINE_BIT | GL_SCISSOR_BIT |
		GL_TEXTURE_BIT | GL_VIEWPORT_BIT);

	// push matrices
	ugl_driver->glMatrixMode(GL_PROJECTION);
	ugl_driver->glPushMatrix();
	ugl_driver->glMatrixMode(GL_MODELVIEW);
	ugl_driver->glPushMatrix();

	// reset states
	resetDeviceAttributes();
	resetDeviceViewMatrix();
	ugl_driver->glFlush();
}

void
UGL_Graphics::end() {
	// pop matrices
	ugl_driver->glMatrixMode(GL_PROJECTION);
	ugl_driver->glPopMatrix();
	ugl_driver->glMatrixMode(GL_MODELVIEW);
	ugl_driver->glPopMatrix();

	// pop attributes
	ugl_driver->glPopAttrib();
}

void
UGL_Graphics::clear() {
	UContext * context = getContext();

	if (context) {
		URectangle deviceBounds = context->getDeviceBounds();
		URectangle contextBounds = context->getContextBounds();
		if (deviceBounds != contextBounds) {
			UColor colTemp = m_color;
			setColor(m_clearColor);
			drawRect(mapToDevice(contextBounds));
			setColor(colTemp);
		} else {
			ugl_driver->glClear(GL_COLOR_BUFFER_BIT);
		}
	} else {
		ugl_driver->glClear(GL_COLOR_BUFFER_BIT);
	}
}

void
UGL_Graphics::setEnabled(GCState state, bool b) {/*
	static bool has_blend_enabled = false;
	if (state == LineAntialiasing) {
		if (b) {
			ugl_driver->glEnable(GL_LINE_SMOOTH);
			setEnabled(Blending, true);
		} else {
			ugl_driver->glDisable(GL_LINE_SMOOTH);
			if (has_blend_enabled) {
				setEnabled(Blending, false);
			}
		}
	} else if (state == Blending) {
		if (b) {
			if (!has_blend_enabled && !isEnabled(Blending)) {
				has_blend_enabled = true;
			}
			ugl_driver->glEnable(GL_BLEND);
			ugl_driver->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		} else {
			ugl_driver->glEnable(GL_LINE_SMOOTH);
			has_blend_enabled = false;
		}
	}*/
}

bool
UGL_Graphics::isEnabled(GCState state) const {
	GLboolean ret[1];
	ret[0] = 0;
	if (state == LineAntialiasing) {
		ugl_driver->glGetBooleanv(GL_LINE_SMOOTH, ret);
	} else if (state == Blending) {
		ugl_driver->glGetBooleanv(GL_BLEND, ret);
	}
	return ret[0];
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
	bool toggleAlpha = false;
	if ((m_color.getAlpha() == 1.0f && color.getAlpha() < 1.0f) ||
		(color.getAlpha() == 1.0f && m_color.getAlpha() < 1.0f)) {
		toggleAlpha = true;
	}
	m_color = color;

	ugl_driver->glColor4fv(m_color.getFloat());
	// FIXME: calling those functions between an glBegin()/glEnd()
	// section is an invalid operation
	if (toggleAlpha) {
		if (color.getAlpha() < 1.0f) {
			ugl_driver->glEnable(GL_BLEND);
			ugl_driver->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		} else {
			ugl_driver->glDisable(GL_BLEND);
		}
	}
}

UColor
UGL_Graphics::getColor() const {
	return m_color;
}

void
UGL_Graphics::setClearColor(const UColor & clearColor) {
	m_clearColor = clearColor;
	ugl_driver->glClearColor(m_clearColor.getRed(), m_clearColor.getGreen(), m_clearColor.getBlue(), m_clearColor.getAlpha());
}

UColor
UGL_Graphics::getClearColor() const {
	return m_clearColor;
}

void
UGL_Graphics::setFont(const UFont & font) {
	m_font = font;
}

UFont
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

	if (m_clipRect.isInvalid()) {
		ugl_driver->glDisable(GL_SCISSOR_TEST);
	} else {
		ugl_driver->glEnable(GL_SCISSOR_TEST);
		// ensure at least zero size
		m_clipRect.expand(UDimension(0, 0));
		URectangle clipRect = mapToDevice(m_clipRect);

		ugl_driver->glScissor(
			clipRect.x, clipRect.y,
			clipRect.w, clipRect.h
		);
	}
}

URectangle
UGL_Graphics::getClipRect() const {
	return m_clipRect;
}

void
UGL_Graphics::setLineWidth(float width) {
	ugl_driver->glLineWidth(width);
}

float
UGL_Graphics::getLineWidth() const {
	float ret[1];
	ugl_driver->glGetFloatv(GL_LINE_WIDTH, ret);
	return ret[0];
}

void
UGL_Graphics::drawString(const std::string & text, int x, int y) {
	m_font.getRenderer()->drawString(this, text.data(), text.length(), x, y);
}

UDimension
UGL_Graphics::getStringSize(const std::string & text) {
	const UFontMetrics * metrics = m_font.getFontMetrics();
	return UDimension(metrics->getStringWidth(text), metrics->getHeight());
}

//
// transformations
//

void
UGL_Graphics::translate(float x, float y) {
	m_translationX += x;
	m_translationY += y;
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
static float ufo_line_add =  0.375f;
void
UGL_Graphics::drawRect(const URectangle & rect) {
	// -1 is correct, as lines are drawn like rectangles with
	// width 1 measured in y direction
	ugl_driver->glTranslatef(ufo_line_add, ufo_line_add, 0);
	ugl_driver->glBegin(GL_LINE_LOOP);
	ugl_driver->glVertex2i(rect.x, rect.y);
	ugl_driver->glVertex2i(rect.x, rect.y + rect.h - 1);
	ugl_driver->glVertex2i(rect.x + rect.w - 1, rect.y + rect.h - 1);
	ugl_driver->glVertex2i(rect.x + rect.w - 1, rect.y);
	ugl_driver->glEnd();
	ugl_driver->glTranslatef(-ufo_line_add, -ufo_line_add, 0);
}

void
UGL_Graphics::fillRect(const URectangle & rect) {
	ugl_driver->glRecti(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);
}

void
UGL_Graphics::drawLine(const UPoint & p1, const UPoint & p2) {
	ugl_driver->glTranslatef(ufo_line_add, ufo_line_add, 0);
	ugl_driver->glBegin(GL_LINES);
	ugl_driver->glVertex2i(p1.x, p1.y);
	ugl_driver->glVertex2i(p2.x, p2.y);
	ugl_driver->glEnd();
	ugl_driver->glTranslatef(-ufo_line_add, -ufo_line_add, 0);
}
void
UGL_Graphics::drawVertexArray(VertexType type, UVertexArray * buffer) {
	int glType;
			ugl_driver->glTranslatef(ufo_line_add, ufo_line_add, 0);
	switch (type) {
		case Lines:
			glType = GL_LINES;
		break;
		case LineStrip:
			glType = GL_LINE_STRIP;
			//ugl_driver->glTranslatef(ufo_line_add, ufo_line_add, 0);
		break;
		case Triangles:
			glType = GL_TRIANGLES;
		break;
		case TriangleStrip:
			glType = GL_TRIANGLE_STRIP;
		break;
		case TriangleFan:
			glType = GL_TRIANGLE_FAN;
		break;
		default:
			glType = GL_LINE_STRIP;
		break;
	}
	// workaround for buggy glPopClientAttrib in MESA 6.4.*
	GLboolean vertexArrayWasEnabled = false;
	GLboolean colorArrayWasEnabled = false;
	ugl_driver->glGetBooleanv(GL_VERTEX_ARRAY, &vertexArrayWasEnabled);
	ugl_driver->glGetBooleanv(GL_COLOR_ARRAY, &colorArrayWasEnabled);
	if (buffer->getType() == UVertexArray::V3F) {
		ugl_driver->glEnableClientState(GL_VERTEX_ARRAY);
		ugl_driver->glInterleavedArrays(GL_V3F, 0, buffer->getArray());
		ugl_driver->glDrawArrays(glType, 0, buffer->getCount());
		if (!vertexArrayWasEnabled) {
			ugl_driver->glDisableClientState(GL_VERTEX_ARRAY);
		}
	} else if (buffer->getType() == UVertexArray::C3F_V3F) {
		// we use color arrays ...
		//ugl_driver->glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
		ugl_driver->glEnableClientState(GL_VERTEX_ARRAY);
		ugl_driver->glEnableClientState(GL_COLOR_ARRAY);
		ugl_driver->glShadeModel(GL_SMOOTH);
		ugl_driver->glInterleavedArrays(GL_C3F_V3F, 0, buffer->getArray());
		ugl_driver->glDrawArrays(glType, 0, buffer->getCount());
		ugl_driver->glShadeModel(GL_FLAT);
		//ugl_driver->glPopClientAttrib();
		if (!vertexArrayWasEnabled) {
			ugl_driver->glDisableClientState(GL_VERTEX_ARRAY);
		}
		if (!colorArrayWasEnabled) {
			ugl_driver->glDisableClientState(GL_COLOR_ARRAY);
		}
	}
			ugl_driver->glTranslatef(-ufo_line_add, -ufo_line_add, 0);
	switch (type) {
		case Lines:
			glType = GL_LINES;
		break;
		case LineStrip:
			glType = GL_LINE_STRIP;
			//ugl_driver->glTranslatef(-ufo_line_add, -ufo_line_add, 0);
		break;
	}
}

void
UGL_Graphics::flush() {
}
/*
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
*/
void
UGL_Graphics::drawImage(UImage * image, const URectangle & rect) {
	UGL_Image * tex = static_cast<UGL_Image*>(image);
	tex->paint(this, rect);
}
/*
void
UGL_Graphics::drawSubImage(UImage * image,
		const URectangle & srcRect, const UPoint & destLocation) {
	UGL_Image * tex = static_cast<UGL_Image*>(image);
	tex->paintSubImage(this, srcRect, destLocation);
}
*/
void
UGL_Graphics::drawSubImage(UImage * image,
		const URectangle & srcRect, const URectangle & destRect) {
	UGL_Image * tex = static_cast<UGL_Image*>(image);
	tex->paintSubImage(this, srcRect, destRect);
}
