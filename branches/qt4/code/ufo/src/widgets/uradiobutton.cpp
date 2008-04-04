/***************************************************************************
                          uradiobutton.cpp  -  description
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#include "ufo/widgets/uradiobutton.hpp"

using namespace ufo;


UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(URadioButton, UButton)

URadioButton::URadioButton() : UButton() {
	setToggable(true);
	setCssType("radio");
}

URadioButton::URadioButton(const std::string & text) : UButton(text) {
	setToggable(true);
	setCssType("radio");
}
