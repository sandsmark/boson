/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/basic/ubasicmenubarui.cpp
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

#include "ufo/ui/basic/ubasicmenubarui.hpp"

#include "ufo/layouts/uflowlayout.hpp"

#include "ufo/widgets/uwidget.hpp"


using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UBasicMenuBarUI, UMenuBarUI)

UBasicMenuBarUI * UBasicMenuBarUI::m_menuBarUI = new UBasicMenuBarUI();

std::string UBasicMenuBarUI::m_lafId("UMenuBar");


UBasicMenuBarUI *
UBasicMenuBarUI::createUI(UWidget * w) {
	return m_menuBarUI;
}

const std::string &
UBasicMenuBarUI::getLafId() {
	return m_lafId;
}


void
UBasicMenuBarUI::installUI(UWidget * w) {
	UWidgetUI::installUI(w);
	w->setLayout( new UFlowLayout( 0, 0 ) );
}

void UBasicMenuBarUI::uninstallUI(UWidget * w) {
	UWidgetUI::uninstallUI(w);

	w->setLayout( NULL );
}
