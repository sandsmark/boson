/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/image/uxbmicon.hpp
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

#ifndef UXBMICON_HPP
#define UXBMICON_HPP

#include "../uicon.hpp"

namespace ufo {

/** This icon uses an x bitmap and paints it via glBitmap.
  * WARNING: This class is deprecated and should not be used.
  * @author Johannes Schmidt
  */

class UFO_EXPORT UXBMIcon : public UIcon  {
	UFO_DECLARE_DYNAMIC_CLASS(UXBMIcon)
public:
	UXBMIcon(const uint8_t * src, int width, int height);
	~UXBMIcon();

	/** Use color (instead of widget attributes) to paint the bitmap. */
	void paintIcon(UGraphics * g, UColor * color, int x, int y);

public: // Overrides UIcon
	void paintIcon(UGraphics * g, UWidget * widget, int x, int y);

	int getIconWidth() const;
	int getIconHeight() const;

private:
	uint8_t * m_data;
	int m_width;
	int m_height;
};

} // namespace ufo

#endif // UXBMICON_HPP
