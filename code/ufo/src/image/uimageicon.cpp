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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include "ufo/image/uimageicon.hpp"

#include "ufo/utoolkit.hpp"
#include "ufo/ugraphics.hpp"
#include "ufo/image/uimage.hpp"
//#include "ufo/ufo_gl.hpp"

#include "ufo/image/uimageio.hpp"
#include "ufo/util/ufilearchive.hpp"

using namespace ufo;

UFO_IMPLEMENT_ABSTRACT_CLASS(UImageIcon, UIcon)

UImageIcon::UImageIcon(const std::string & fileNameA) {
	m_image = UToolkit::getToolkit()->getCurrentContext()->createImage(fileNameA);
	//new UTexture();

	//m_tex->loadFromArchive(fileNameA);

	trackPointer(m_image);

	// cache size
	m_width = m_image->getImageWidth();
	m_height = m_image->getImageHeight();
}


UImageIcon::UImageIcon(UImage * image)
	: m_image(image)
	, m_width(image->getImageWidth())
	, m_height(image->getImageHeight())
{
	if (m_image) {
		trackPointer(m_image);
	}
	// FIXME
	// trow an error if texture is NULL?
}

void
UImageIcon::paintIcon(UGraphics * g, UWidget * widget, int x, int y) {
	if (m_image) {
		g->drawImage(m_image, x, y);
		//glTranslatef(x, y, 0);

		//m_tex->paint(g);

		//glTranslatef(-x, -y, 0);
	}
}

int
UImageIcon::getIconWidth() const {
	return m_width;
}

int
UImageIcon::getIconHeight() const {
	return m_height;
}
