/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ui/ubuttonui.hpp
    begin             : Sun May 27 2001
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

#ifndef UBUTTONUI_HPP
#define UBUTTONUI_HPP

#include "uwidgetui.hpp"

#include "../signals/usignal.hpp"

namespace ufo {

class UEvent;
class UMouseEvent;
class UButton;

/**
  *@author Johannes Schmidt
  */

class UFO_EXPORT UButtonUI : public UWidgetUI {
	UFO_DECLARE_DYNAMIC_CLASS(UButtonUI)
};

} // namespace ufo

#endif // UBUTTONUI_HPP
