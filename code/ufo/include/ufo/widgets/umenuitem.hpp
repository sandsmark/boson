/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/widgets/umenuitem.hpp
    begin             : Sun Jun 17 2001
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

#ifndef UMENUITEM_HPP
#define UMENUITEM_HPP

#include "ubutton.hpp"

// we need this for proper getUI() overriding
//#include "../ui/umenuitemui.hpp"

namespace ufo {

class UPopupMenu;

/**
  *@author Johannes Schmidt
  */

class UFO_EXPORT UMenuItem : public UButton {
	UFO_DECLARE_DYNAMIC_CLASS(UMenuItem)
	UFO_UI_CLASS(UMenuItemUI)
public:
	UMenuItem();
	UMenuItem(UIcon * icon);
	UMenuItem(const std::string & text, UIcon * icon = NULL);
/*
public: // hides | overrides UWidget
	virtual void setUI(UMenuItemUI * ui);
	virtual UWidgetUI * getUI() const;
	virtual void updateUI();
*/
public: // Overrides UWidget
	virtual void activate();
};

} // namespace ufo

#endif // UMENUITEM_HPP
