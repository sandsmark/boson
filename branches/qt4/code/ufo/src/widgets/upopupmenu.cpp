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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
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

#include "ufo/layouts/uboxlayout.hpp"

using namespace ufo;


UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UPopupMenu, UWidget)


UPopupMenu::UPopupMenu()
	: m_invoker(NULL)
	, m_popupLocation()
	, m_closeSlot(slot(*this, &UPopupMenu::popupCloseSlot))
{
	setLayout(new UBoxLayout(Vertical, 0, 0));
	setCssType("popupmenu");
}

UPopupMenu::UPopupMenu(UWidget * invoker)
	: m_invoker(invoker)
	, m_popupLocation()
	, m_closeSlot(slot(*this, &UPopupMenu::popupCloseSlot))
{
	setLayout(new UBoxLayout(Vertical, 0, 0));
	setCssType("popupmenu");
}


void
UPopupMenu::setVisible(bool v) {
	if (UWidget::isVisible() == v) {
		return;
	}
	UWidget::setVisible(v);

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
}


//
// public methods
//

void
UPopupMenu::addSeparator() {
	add(new USeparator());
}

void
UPopupMenu::setInvoker(UWidget * invoker) {
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
