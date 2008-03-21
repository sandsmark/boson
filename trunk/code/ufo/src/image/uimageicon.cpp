/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/image/uimageicon.cpp
    begin             : Mon Apr 29 2002
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

#include "ufo/image/uimageicon.hpp"

#include "ufo/utoolkit.hpp"
#include "ufo/udisplay.hpp"
#include "ufo/ugraphics.hpp"
#include "ufo/image/uimage.hpp"
//#include "ufo/ufo_gl.hpp"

#include "ufo/image/uimageio.hpp"
#include "ufo/util/ufilearchive.hpp"

using namespace ufo;

UFO_IMPLEMENT_ABSTRACT_CLASS(UImageIcon, UIcon)

UImageIcon::UImageIcon(const std::string & fileName)
	: m_image(NULL)
{
	m_image = UDisplay::getDefault()->createImage(fileName);
	trackPointer(m_image);
}


UImageIcon::UImageIcon(UImage * image)
	: m_image(image)
{
	if (m_image) {
		trackPointer(m_image);
	}
	// FIXME
	// trow an error if texture is NULL?
}

void
UImageIcon::paintIcon(UGraphics * g, const URectangle & rect,
		const UStyleHints * hints, uint32_t widgetState) {
	if (m_image) {
		UDimension size = rect.getSize();
		if (size.isEmpty() || size == UDimension::invalid) {
			g->drawImage(m_image, rect.x, rect.y);
		} else {
			g->drawImage(m_image, rect);
		}
	}
}

UDimension
UImageIcon::getIconSize() const {
	if (m_image) {
		return m_image->getImageSize();
	}
	return UDimension();
}
