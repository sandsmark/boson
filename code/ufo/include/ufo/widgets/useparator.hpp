/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/useparator.hpp
    begin             : Fri Aug 10 2001
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

#ifndef USEPARATOR_HPP
#define USEPARATOR_HPP

#include "uwidget.hpp"

// we need this for proper getUI() overriding
//#include "../ui/useparatorui.hpp"

namespace ufo {

/** A separator for menus, ..
  * @author Johannes Schmidt
  */

class UFO_EXPORT USeparator : public UWidget {
	UFO_DECLARE_DYNAMIC_CLASS(USeparator)
	UFO_UI_CLASS(USeparatorUI)
public:
	USeparator(Orientation orientation = Horizontal);
/*
public: // hides | overrides UWidget
	virtual void setUI(USeparatorUI * ui);
	virtual UWidgetUI * getUI() const;
	virtual void updateUI();
*/
public: // Public methods

	Orientation getOrientation() const;
	void setOrientation(Orientation orientation);

private: // Private attributes
	/**  */
	Orientation m_orientation;
};

} // namespace ufo

#endif // USEPARATOR_HPP
