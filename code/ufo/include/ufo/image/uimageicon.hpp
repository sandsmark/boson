/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/image/uimageicon.hpp
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

#ifndef UIMAGEICON_HPP
#define UIMAGEICON_HPP

#include "../uicon.hpp"

namespace ufo {

class UImage;

/** An icon which uses a image as source.
  * @author Johannes Schmidt
  */

class UFO_EXPORT UImageIcon : public UIcon  {
	UFO_DECLARE_DYNAMIC_CLASS(UImageIcon)
public:
	UImageIcon(const std::string & fileNameA);
	UImageIcon(UImage * image);

	void paintIcon(UGraphics * g, UWidget * widget, int x, int y);

	int getIconWidth() const;
	int getIconHeight() const;

private:
	UImage * m_image;
	int m_width;
	int m_height;
};

} // namespace ufo

#endif // UIMAGEICON_HPP
