/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
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

/** @short An icon which uses an image as source.
  * @ingroup drawing
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UImageIcon : public UIcon  {
	UFO_DECLARE_DYNAMIC_CLASS(UImageIcon)
public:
	/** Creates an image icon with an image using the given file name.
	  * @param fileName The file name of the desired image
	  */
	UImageIcon(const std::string & fileName);
	/** Creates an icon displaying the given image.
	  * @param image The image which should be displayed.
	  */
	UImageIcon(UImage * image);

public: // Implements UIcon
	virtual void paintIcon(UGraphics * g, const URectangle & rect,
		const UStyleHints * hints, uint32_t widgetState = 0);

	virtual UDimension getIconSize() const;

private:
	UImage * m_image;
};

} // namespace ufo

#endif // UIMAGEICON_HPP
