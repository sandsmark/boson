/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/umenubar.cpp
    begin             : Tue May 29 2001
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

#include "ufo/widgets/umenubar.hpp"

#include "ufo/widgets/urootpane.hpp"
#include "ufo/widgets/umenu.hpp"

//#include "ufo/ui/uuimanager.hpp"

#include "ufo/events/umouseevent.hpp"

namespace ufo {

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UMenuBar, UWidget)

UMenuBar::UMenuBar()
	: m_visMenu(NULL)
{
}

//*
//* hides | overrides UWidget
//*
/*
void
UMenuBar::setUI(UMenuBarUI * ui) {
	UWidget::setUI(ui);
}

UWidgetUI *
UMenuBar::getUI() const {
	return static_cast<UMenuBarUI*>(UWidget::getUI());
}

void
UMenuBar::updateUI() {
	setUI(static_cast<UMenuBarUI*>(getUIManager()->getUI(this)));
}
*/
void
UMenuBar::addImpl(UWidget * w, UObject * constraints, int index) {
	if (UMenu * menu = dynamic_cast<UMenu*>(w)) {
		UWidget::addImpl(menu, constraints, index);
		menu->sigMouseEntered().connect(slot(*this, &UMenuBar::menuPopup));
	}
}

//*
//* public methods
//*

void
UMenuBar::closePopups() {
	const std::vector<UWidget*> & children = getWidgets();
	for (std::vector<UWidget*>::const_iterator iter = children.begin();
			iter != children.end(); ++iter ) {
		if (UMenu * menu = dynamic_cast<UMenu *>(*iter)) {
			menu->setPopupMenuVisible(false);
		}
	}
}


void
UMenuBar::setVisibleMenu(UMenu * menu) {
	m_visMenu = menu;
}

UMenu *
UMenuBar::getVisibleMenu() {
	return m_visMenu;
}


//
// protected slots
//

void
UMenuBar::menuPopup(UMouseEvent * e) {
	UMenu * menu = dynamic_cast<UMenu *>(e->getSource());
	UMenu * visMenu = getVisibleMenu();
	
	if (visMenu && (menu != visMenu)) {
		// eliminate the pressed status of the menu buttons
		visMenu->doClick(0);
		menu->doClick(0);
	}
}

} // namespace ufo
