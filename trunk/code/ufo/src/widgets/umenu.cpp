/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/umenu.cpp
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

#include "ufo/widgets/umenu.hpp"

#include "ufo/widgets/umenubar.hpp"
#include "ufo/widgets/upopupmenu.hpp"

//#include "ufo/ui/uuimanager.hpp"

namespace ufo {

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UMenu, UMenuItem)

UMenu::UMenu(const std::string & text, UIcon * icon)
		: UMenuItem(text, icon)
{
	//m_popupMenu = new UPopupMenu(this);
	setPopupMenu(new UPopupMenu(this));
	//trackPointer(m_popupMenu);
}
UMenu::UMenu(UIcon * icon)
		: UMenuItem(icon)
{
	//m_popupMenu = new UPopupMenu(this);
	setPopupMenu(new UPopupMenu(this));
	//trackPointer(m_popupMenu);
}


//*
//* hides | overrides UWidget
//*
/*
void
UMenu::setUI(UMenuUI * ui) {
	UWidget::setUI(ui);
}

UWidgetUI *
UMenu::getUI() const {
	return static_cast<UMenuUI*>(UWidget::getUI());
}

void
UMenu::updateUI() {
	setUI(static_cast<UMenuUI*>(getUIManager()->getUI(this)));
}
*/

void
UMenu::addImpl(UWidget * w, UObject * constraints, int index) {
	if (getPopupMenu()) {
		getPopupMenu()->add(w, constraints, index);
	}
}

//*
//* public methods
//*

bool
UMenu::isTopLevelMenu() const {
	if (dynamic_cast<UPopupMenu*>(getParent())) {
		return false;
	}
	return true;
}


void
UMenu::addSeparator() {
	if (getPopupMenu()) {
		getPopupMenu()->addSeparator();
	}
}


bool
UMenu::isPopupMenuVisible() const {
	if (getPopupMenu()) {
		return getPopupMenu()->isVisible();
	}
	return false;
}

void
UMenu::setPopupMenuVisible(bool b) {
	if (b == isPopupMenuVisible()) {
		return ;
	}

	if (b) {
		if (dynamic_cast<UPopupMenu*>(getParent())) {
			// this is a sub menu
			getPopupMenu()->setPopupLocation(UPoint(getWidth(), 0));
		} else {
			getPopupMenu()->setPopupLocation(UPoint(0, getHeight()));
		}
		getPopupMenu()->setVisible(true);

		if (UMenuBar * mb = dynamic_cast<UMenuBar*>(getParent())) {
			mb->setVisibleMenu(this);
		}
	} else {
		if (UMenuBar * mb = dynamic_cast<UMenuBar*>(getParent()) ) {
			mb->setVisibleMenu(NULL);
		}
		getPopupMenu()->setVisible(false);
	}
}

void
UMenu::invalidateSelf() {
	UWidget::invalidateSelf();
	if (getPopupMenu()) {
		getPopupMenu()->invalidateTree();
	}
}


//
// protected methods
//

std::ostream &
UMenu::paramString(std::ostream & os) const {
	os << "\"" << getText() << "\"  ";

	// add common widget params
	return UWidget::paramString(os);
}

} // namespace ufo
