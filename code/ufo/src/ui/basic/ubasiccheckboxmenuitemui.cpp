/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/basic/ubasiccheckboxmenuitemui.cpp
    begin             : Thu Sep 16 2004
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

#include "ufo/ui/basic/ubasiccheckboxmenuitemui.hpp"

#include "ufo/ui/uuimanager.hpp"

#include "ufo/widgets/ucheckboxmenuitem.hpp"

#include "ufo/uicon.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UBasicCheckBoxMenuItemUI, UBasicMenuItemUI)

UBasicCheckBoxMenuItemUI * UBasicCheckBoxMenuItemUI::m_checkBoxMenuItemUI =
	new UBasicCheckBoxMenuItemUI();

std::string UBasicCheckBoxMenuItemUI::m_lafId("UCheckBoxMenuItem");


UBasicCheckBoxMenuItemUI *
UBasicCheckBoxMenuItemUI::createUI(UWidget * w) {
	return m_checkBoxMenuItemUI;
}




const std::string &
UBasicCheckBoxMenuItemUI::getLafId() {
	return m_lafId;
}


void
UBasicCheckBoxMenuItemUI::installUI(UWidget * w) {
	UBasicMenuItemUI::installUI(w);

	UUIManager * manager = w->getUIManager();
	UCheckBoxMenuItem * item = dynamic_cast<UCheckBoxMenuItem*>(w);
	item->setIcon(static_cast<UIcon*>(manager->get(getLafId() + ".icon")));
	item->setPressedIcon(static_cast<UIcon*>(manager->get(getLafId() + ".selectedIcon")));
	item->setDisabledIcon(static_cast<UIcon*>(manager->get(getLafId() + ".inactiveIcon")));
}

void
UBasicCheckBoxMenuItemUI::uninstallUI(UWidget * w) {
	UBasicMenuItemUI::uninstallUI(w);

	UCheckBoxMenuItem * item = dynamic_cast<UCheckBoxMenuItem*>(w);
	item->setIcon(NULL);
	item->setPressedIcon(NULL);
	item->setDisabledIcon(NULL);
}
