/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef UICON_HPP
#define UICON_HPP

#include "uobject.hpp"

#include "widgets/uwidget.hpp"

namespace ufo {

/**abstract class for label and button icons
  *@author Johannes Schmidt
  */

class UFO_EXPORT UIcon : public UObject {
	UFO_DECLARE_ABSTRACT_CLASS(UIcon)
public:
	virtual void paintIcon(UGraphics * g, UWidget * widget, int x, int y) = 0;

	virtual int getIconWidth() const = 0;
	virtual int getIconHeight() const = 0;
};

} // namespace ufo

#endif // UICON_HPP
