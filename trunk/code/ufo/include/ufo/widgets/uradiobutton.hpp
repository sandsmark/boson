/***************************************************************************
                          uradiobutton.hpp  -  description
                             -------------------
    begin                : Sat Jan 10 2004
    copyright            : (C) 2004 by Johannes Schmidt
    email                : schmidtjf at users.sourceforge.net
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

#ifndef URADIOBUTTON_HPP
#define URADIOBUTTON_HPP

#include "ubutton.hpp"

namespace ufo {

/** @short A radio button is a toggle UFO button.
  * @ingroup widgets
  *
  * If a radio button is "checked", isSelected() returns true.
  * Radio Buttons are usually combined in button groups.
  *
  * @see UButtonGroup
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT URadioButton : public UButton  {
	UFO_DECLARE_DYNAMIC_CLASS(URadioButton)
	UFO_UI_CLASS(URadioButtonUI)
	UFO_STYLE_TYPE(UStyle::CE_RadioButton)
public:
	URadioButton();
	URadioButton(const std::string & text);
};

} // namespace ufo

#endif // URADIOBUTTON_HPP
