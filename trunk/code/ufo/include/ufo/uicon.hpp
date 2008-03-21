/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/uicon.hpp
    begin             : Wed May 23 2001
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

#ifndef UICON_HPP
#define UICON_HPP

#include "uobject.hpp"

#include "util/udimension.hpp"
#include "util/urectangle.hpp"

namespace ufo {

class UGraphics;
class UStyleHints;

/** @short Abstract class for label and button icons
  * @ingroup drawing
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UIcon : public UObject {
	UFO_DECLARE_ABSTRACT_CLASS(UIcon)
public:
	/** Paints this icon on the given graphics object.
	  * If the size of the rectangle is empty or invalid, the actual icon
	  * size is used. Furthermore, the icon may ignore given sizes.
	  *
	  * @param g The graphics object
	  * @param rect The rectangle
	  * @param hints Style hints which may be used to paint the icon
	  * @param widgetState state flags
	  */
	virtual void paintIcon(UGraphics * g, const URectangle & rect,
		const UStyleHints * hints, uint32_t widgetState = 0) = 0;

	/** @overload */
	void paintIcon(UGraphics * g, int x, int y,
		const UStyleHints * hints, uint32_t widgetState = 0) {
		paintIcon(g, URectangle(UPoint(x, y), getIconSize()), hints, widgetState);
	}

	/** @return The actual icon size */
	virtual UDimension getIconSize() const = 0;
};

} // namespace ufo

#endif // UICON_HPP
