/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/basic/ubasicpopupmenuui.cpp
    begin             : Mon Jul 22 2002
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

#include "ufo/ui/basic/ubasicpopupmenuui.hpp"

#include "ufo/layouts/uboxlayout.hpp"

#include "ufo/widgets/upopupmenu.hpp"


using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UBasicPopupMenuUI, UPopupMenuUI)

UBasicPopupMenuUI * UBasicPopupMenuUI::m_popupMenuUI = new UBasicPopupMenuUI();

std::string UBasicPopupMenuUI::m_lafId("UPopupMenu");


UBasicPopupMenuUI *
UBasicPopupMenuUI::createUI(UWidget * w) {
	return m_popupMenuUI;
}


void
UBasicPopupMenuUI::installUI(UWidget * w) {
	UPopupMenuUI::installUI(w);

	w->setLayout(new UBoxLayout(UBoxLayout::YAxis, 0, 0));
}

void
UBasicPopupMenuUI::uninstallUI(UWidget * w) {
	UPopupMenuUI::uninstallUI(w);

	w->setLayout(NULL);
}

const std::string &
UBasicPopupMenuUI::getLafId() {
	return m_lafId;
}
