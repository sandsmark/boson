/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/upopupmenu.cpp
    begin             : Wed May 30 2001
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

#include "ufo/widgets/upopupmenu.hpp"

#include "ufo/widgets/urootpane.hpp"
#include "ufo/widgets/ulayeredpane.hpp"

#include "ufo/widgets/useparator.hpp"

#include "ufo/widgets/umenu.hpp"
#include "ufo/umenumanager.hpp"

#include "ufo/events/umouseevent.hpp"

//#include "ufo/ui/uuimanager.hpp"

#include "ufo/ucontext.hpp"

#include "ufo/upopupmanager.hpp"
#include "ufo/upopup.hpp"

namespace ufo {


UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UPopupMenu, UWidget)


UPopupMenu::UPopupMenu()
	: m_invoker(NULL)
	, m_popupLocation()
	, m_closeSlot(slot(*this, &UPopupMenu::popupCloseSlot))
{}

UPopupMenu::UPopupMenu(UWidget * invoker)
	: m_invoker(invoker)
	, m_popupLocation()
	, m_closeSlot(slot(*this, &UPopupMenu::popupCloseSlot))
{}


//*
//* hides | overrides UWidget
//*
/*
void
UPopupMenu::setUI(UPopupMenuUI * ui) {
	UWidget::setUI(ui);
}

UWidgetUI *
UPopupMenu::getUI() const {
	return static_cast<UPopupMenuUI*>(UWidget::getUI());
}

void
UPopupMenu::updateUI() {
	setUI(static_cast<UPopupMenuUI*>(getUIManager()->getUI(this)));
}
*/
void
UPopupMenu::setVisible(bool v) {
	if (UWidget::isVisible() == v) {
		return;
	}

	if (v) {
	m_popup = UPopupManager::getPopupManager()->
		createPopup(m_invoker, this, m_popupLocation.x, m_popupLocation.y, 0, 0);
	m_popup->sigPopupAboutToClose().connect(m_closeSlot);
	} else if (!v && m_popup) {
		/*m_popup->sigPopupAboutToClose().disconnect(m_closeSlot);
		m_sigMenuAboutToClose(this);
		UWidget::setVisible(false);*/
		m_popup->hide();
	}
/*
	if (!v) {
		if (UWidget * parent = getParent()) {
			parent->remove(this);
			parent->repaint();
		}
		UWidget::setVisible(false);
		getContext()->disconnectListener(m_closeSlot);
	} else if (m_invoker) {
		// reset the wanted location
		setLocation(m_invoker->pointToRootPoint(m_popupLocation));

		// notify listeners
		m_sigMenuAboutToOpen(this);

		// get the layered pane to display this popup menu in the poup layer
		UWidget * layeredPane = m_invoker->getRootPane(true)->getLayeredPane();

		if (layeredPane) {
			layeredPane->add(this, ULayeredPane::PopupLayer);

			//m_isVisible = true;
			validate(ValidationUI);

			// the layered pane has no layout manager
			setSize(getPreferredSize());

			UWidget::setVisible(true);
			layeredPane->repaint();
		}
		getContext()->connectListener(m_closeSlot);
	}
*/
}


//*
//* public methods
//*

void
UPopupMenu::addSeparator() {
	add(new USeparator());
}

void
UPopupMenu::setInvoker(UWidget * invoker) {
	// FIXME !
	// ref count? what about cyclic pointers?
	if (invoker != m_invoker) {
		// FIXME !
		// should we reinstall UI immediately?
		//if (m_ui) {
		//	m_ui->uninstallUI(this);
		//	m_ui->installUI(this);
		//}

		// it may be another ufo context
		//invalidate(ValidationAll);
		m_invoker = invoker;
	}
}
UWidget *
UPopupMenu::getInvoker() const {
	return m_invoker;
}
/*
void
UPopupMenu::setPopupLocation(int x, int y) {
	if (m_invoker) {
		const UPoint & pos = m_invoker->pointToRootPoint(x, y);
		setLocation(pos);
	} else {
		setLocation(x, y);
	}
}
*/

//
// Protected methods
//
void
UPopupMenu::popupCloseSlot(UPopup * popup) {
	m_popup->sigPopupAboutToClose().disconnect(m_closeSlot);
	m_sigMenuAboutToClose(this);
	// clear menu path
	/*if (UMenuItem * item = dynamic_cast<UMenuItem*>(m_invoker)) {
		UMenuManager::getMenuManager()->clearPath();
	}*/
	UWidget::setVisible(false);
	m_popup = NULL;
}
/*
void
UPopupMenu::popupCloseSlot(UEvent * e) {
	if (e->getType() == UEvent::MousePressed) {
		UMouseEvent * mouseEvent = dynamic_cast<UMouseEvent*>(e);
		if (!containsRootPoint(mouseEvent->getRootLocation())) {
			// special case for menu popups
			*//*if (NULL == dynamic_cast<UMenu*>(e->getSource())) {
				if (UMenu * menu = dynamic_cast<UMenu*>(getInvoker())) {
					menu->setPopupMenuVisible(false);
				} else {
					setVisible(false);
				}
			}*//*
			// notify listeners
			m_sigMenuAboutToClose(this);
			setVisible(false);
		}
	}
}
*/
} // namespace ufo
