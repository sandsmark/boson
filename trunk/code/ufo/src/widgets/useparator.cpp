/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/useparator.cpp
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

#include "ufo/widgets/useparator.hpp"

//#include "ufo/ui/uuimanager.hpp"

using namespace ufo;

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(USeparator, UWidget)

USeparator::USeparator(Orientation orientation) : m_orientation(orientation) {}

//*
//* hides | overrides UWidget
//*
/*
void
USeparator::setUI(USeparatorUI * ui) {
	UWidget::setUI(ui);
}

UWidgetUI *
USeparator::getUI() const {
	return static_cast<USeparatorUI*>(UWidget::getUI());
}

void
USeparator::updateUI() {
	setUI(static_cast<USeparatorUI*>(getUIManager()->getUI(this)));
}
*/
//*
//* public methods
//*


Orientation
USeparator::getOrientation() const {
	return m_orientation;
}

void
USeparator::setOrientation(Orientation orientation) {
	m_orientation = orientation;
}
