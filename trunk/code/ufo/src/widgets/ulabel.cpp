/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/ulabel.cpp
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

#include "ufo/widgets/ulabel.hpp"

#include "ufo/uicon.hpp"

//#include "ufo/ui/uuimanager.hpp"

namespace ufo {


UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(ULabel, UCompound)

ULabel::ULabel()
	: UCompound()
{
}

ULabel::ULabel(UIcon * icon)
	: UCompound(icon)
{
}

ULabel::ULabel(const std::string & text, UIcon * icon)
	: UCompound(text, icon)
{
}

//*
//* hides | overrides UWidget
//*
/*
void
ULabel::setUI(ULabelUI * ui) {
	UWidget::setUI(ui);
}

UWidgetUI *
ULabel::getUI() const {
	return static_cast<ULabelUI*>(UWidget::getUI());
}

void
ULabel::updateUI() {
	setUI(static_cast<ULabelUI*>(getUIManager()->getUI(this)));
}
*/
//*
//* public methods
//*

} // namespace ufo
