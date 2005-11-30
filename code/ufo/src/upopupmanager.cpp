/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/upopupmanager.cpp
    begin             : Tue Jun 10 2003
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

#include "ufo/upopupmanager.hpp"

#include "ufo/upopup.hpp"
#include "ufo/ucontext.hpp"

#include "ufo/widgets/ulayeredpane.hpp"
#include "ufo/widgets/urootpane.hpp"
#include "ufo/widgets/uwidget.hpp"
#include "ufo/layouts/uborderlayout.hpp"

#include "ufo/events/uevent.hpp"
#include "ufo/events/umouseevent.hpp"

using namespace ufo;
/*
static void popup_close_detection(UEvent * e);
static void popup_about_to_close(UPopup * popup);
static void popup_dispose(UPopup * popup);
*/
class ULightWeightPopup : public UPopup {
	UWidget * m_owner;
	URectangle m_desiredBounds;
	UWidget * m_popupWidget;
public:
	ULightWeightPopup(UWidget * owner, UWidget * content,
			const URectangle & desiredBounds)
			: m_owner(owner)
			, m_desiredBounds(desiredBounds)
			, m_popupWidget(new UWidget()) {
		trackPointer(m_popupWidget);

		m_popupWidget->setLayout(new UBorderLayout());
		m_popupWidget->setCssClass("transparent");
		m_popupWidget->add(content);
		m_popupWidget->setLocation(m_owner->pointToRootPoint(desiredBounds.getLocation()));

		UWidget * layeredPane = m_owner->getRootPane(true)->getLayeredPane();

		if (layeredPane) {
			// always put the new popup to front
			layeredPane->add(m_popupWidget, ULayeredPane::PopupLayer, 0);

			m_popupWidget->invalidateTree();
			//m_isVisible = true;
			//m_contentPane->validate(UWidget::ValidationUI);
			//content->validate(UWidget::ValidationUI);

			// the layered pane has no layout manager
			if (m_desiredBounds.w == 0 || m_desiredBounds.h == 0) {
				m_popupWidget->validate();
				UDimension prefSize = m_popupWidget->getPreferredSize();
				if (m_desiredBounds.w != 0) {
					prefSize.w = m_desiredBounds.w;
				}
				if (m_desiredBounds.h != 0) {
					prefSize.h = m_desiredBounds.h;
				}
				m_popupWidget->setSize(prefSize);
			} else {
				m_popupWidget->setSize(m_desiredBounds.getSize());
			}

			m_popupWidget->setVisible(true);
			layeredPane->repaint();
		}
	}
	virtual void pack() {
		m_popupWidget->setSize(m_popupWidget->getPreferredSize());
	}
	virtual void hide() {
		sigPopupAboutToClose().emit(this);
		m_popupWidget->removeAll();
		UWidget * layeredPane = m_owner->getRootPane(true)->getLayeredPane();
		layeredPane->remove(m_popupWidget);
		//popup_dispose(this);
	}
	virtual UWidget * getContentPane() const {
		return m_popupWidget;
	}

};
/*
// FIXME !
// very firty
static UPopup * s_popup;
static UWidget * s_popupOwner;
void
popup_close_detection(UEvent * e) {
	if (e->getType() == UEvent::MousePressed) {
		UMouseEvent * mouseEvent = dynamic_cast<UMouseEvent*>(e);
		if (!s_popup->getContentPane()->containsRootPoint(mouseEvent->getRootLocation()) &&
			!s_popupOwner->containsRootPoint(mouseEvent->getRootLocation())) {

			// notify listeners
			s_popup->hide();
		}
	}
}

void
popup_about_to_close(UPopup * popup) {
	s_popupOwner->getContext()->disconnectListener(slot(&popup_close_detection));
	s_popup->sigPopupAboutToClose().disconnect(slot(&popup_about_to_close));
}

void
popup_dispose(UPopup * popup) {
	s_popup->unreference();

	s_popup = NULL;
	s_popupOwner = NULL;
}
*/

UFO_IMPLEMENT_DYNAMIC_CLASS(UPopupManager, UObject)
UFO_IMPLEMENT_ABSTRACT_CLASS(UPopup, "")

UPopupManager * UPopupManager::sm_popupManager = NULL;

UPopupManager *
UPopupManager::getPopupManager() {
	// FIXME ! memory leak
	if (!sm_popupManager) {
		sm_popupManager = new UPopupManager();
	}
	return sm_popupManager;
}

void
UPopupManager::setPopupManager(UPopupManager * popupManager) {
	if (popupManager) {
		sm_popupManager = popupManager;
	}
}


UPopup *
UPopupManager::createPopup(
		UWidget * owner,
		UWidget * content,
		int x, int y, int w, int h) {
	ULightWeightPopup * popup =
		new ULightWeightPopup(owner, content, URectangle(x, y, w, h));
	//s_popupContainer.trackPointer(popup);

	//s_popup = popup;
	//s_popupOwner = owner;
	//s_popupOwner->getContext()->connectListener(slot(&popup_close_detection));
	//s_popup->sigPopupAboutToClose().connect(slot(&popup_about_to_close));
	//s_popup->reference();

	return popup;
}
