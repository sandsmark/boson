/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/ulabel.hpp
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

#ifndef ULABEL_HPP
#define ULABEL_HPP

#include "ucompound.hpp"

// we need this for proper getUI() overriding
//#include "../ui/ulabelui.hpp"

namespace ufo {

class UIcon;

/** A static text field. Can display icons and static text. Most functionality
  * of this class is provided by @ref UCompound. See the @ref UCompound
  * documentation for further information.
  *
  * You can specify and/or the icon in the constructor and/or set it later,
  * using @ref setText and @ref setIcon.
  *
  * @short Widget that displays one line of text
  * @author Johannes Schmidt
  */

class UFO_EXPORT ULabel : public UCompound {
	UFO_DECLARE_DYNAMIC_CLASS(ULabel)
	UFO_UI_CLASS(ULabelUI)
public:
	ULabel();
	ULabel(UIcon * icon);
	ULabel(const std::string & text, UIcon * icon = NULL);
/*
public: // hides | overrides UWidget
	virtual void setUI(ULabelUI * ui);
	virtual UWidgetUI * getUI() const;
	virtual void updateUI();
*/
};

} // namespace ufo

#endif // ULABEL_HPP
