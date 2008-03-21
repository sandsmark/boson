/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/ucheckbox.hpp
    begin             : Sun Jun 2 2002
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

#ifndef UCHECKBOX_HPP
#define UCHECKBOX_HPP

#include "ubutton.hpp"

namespace ufo {

/** @short A togglable button.
  * @ingroup widgets
  *
  * If a check box is "checked", isSelected() returns true.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UCheckBox : public UButton  {
	UFO_DECLARE_DYNAMIC_CLASS(UCheckBox)
	UFO_UI_CLASS(UCheckBoxUI)
	UFO_STYLE_TYPE(UStyle::CE_CheckBox)
public:
	UCheckBox();
	UCheckBox(const std::string & text);
};

} // namespace ufo

#endif // UCHECKBOX_HPP
