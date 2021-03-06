/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/ucheckboxmenuitem.hpp
    begin             : Thu Sep 16 2004
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

#ifndef UCHECKBOXMENUITEM_HPP
#define UCHECKBOXMENUITEM_HPP

#include "umenuitem.hpp"

namespace ufo {

/** @short A checkbox menu item is a toggable UFO menu item.
  * @ingroup widgets
  *
  * If a check box is "checked", isSelected() returns true.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UCheckBoxMenuItem : public UMenuItem  {
	UFO_DECLARE_DYNAMIC_CLASS(UCheckBoxMenuItem)
	UFO_UI_CLASS(UCheckBoxMenuItemUI)
public:
	UCheckBoxMenuItem();
	UCheckBoxMenuItem(const std::string & text);
};

} // namespace ufo

#endif // UCHECKBOXMENUITEM_HPP
