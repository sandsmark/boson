/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/image/uxbmicon.cpp
    begin             : Thu Feb 13 2003
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

#include "ufo/image/uxbmicon.hpp"

#include "ufo/gl/ugl_driver.hpp"
#include "ufo/util/ucolor.hpp"

using namespace ufo;

UFO_IMPLEMENT_ABSTRACT_CLASS(UXBMIcon, UIcon)

static uint8_t hiNibble[16] = {
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
	0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0
};

static uint8_t loNibble[16] = {
	0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e,
	0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0f
};

UXBMIcon::UXBMIcon(const uint8_t * src, int width, int height)
	: m_data(NULL)
	, m_width(width)
	, m_height(height)
{
	int bytes_per_row = (width + 7) / 8;
	int pad = bytes_per_row & 1;
	//int shr = ((width - 1) & 7)  + 1;

	m_data = new uint8_t[(bytes_per_row + pad) * height * 3];
	uint8_t * dst = m_data;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < bytes_per_row; j++) {
			uint8_t b = *src++;
			*dst++ = (hiNibble[b&15]) | (loNibble[(b>>4)&15]);
		}
		if (pad) {
			*dst++ = 0;
		}
	}
}

UXBMIcon::~UXBMIcon() {
	if (m_data) {
		delete[] (m_data);
	}
}

void
UXBMIcon::paintIcon(UGraphics * g, UColor * color, int x, int y) {
	ugl_driver->glColor3fv(color->getFloat());

	ugl_driver->glRasterPos2i(x, y + m_height - 1);

	ugl_driver->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	ugl_driver->glBitmap(
		m_width, // width
		m_height, // height
		0, // xorig
		0, // yorig
		0,
		0,
		m_data
	);
}

void
UXBMIcon::paintIcon(UGraphics * g, UWidget * widget, int x, int y) {
	ugl_driver->glColor3fv(widget->getColorGroup().foreground().getFloat());

	ugl_driver->glRasterPos2i(x, y + m_height - 1);

	ugl_driver->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	ugl_driver->glBitmap(
		m_width, // width
		m_height, // height
		0, // xorig
		0, // yorig
		0,
		0,
		m_data
	);
}

int
UXBMIcon::getIconWidth() const {
	return m_width;
}

int
UXBMIcon::getIconHeight() const {
	return m_height;
}
