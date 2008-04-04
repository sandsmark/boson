/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/umenuitem.cpp
    begin             : Sun Jun 17 2001
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

#include "ufo/widgets/umenuitem.hpp"

#include "ufo/umodel.hpp"
#include "ufo/widgets/umenu.hpp"
#include "ufo/widgets/umenubar.hpp"
#include "ufo/widgets/urootpane.hpp"
#include "ufo/widgets/upopupmenu.hpp"

#include "ufo/umenumanager.hpp"
#include "ufo/events/umouseevent.hpp"

#include "ufo/ui/ustyle.hpp"

//#include "ufo/ui/uuimanager.hpp"

using namespace ufo;


UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UMenuItem, UButton)

UMenuItemModel *
_createMenuItemModel(UCompoundModel * model, int checkType,
		int maxIconWidth, const UKeyStroke & shortcut) {
	UMenuItemModel * c = new UMenuItemModel();
	c->widgetState = model->widgetState;
	c->icon = model->icon;
	c->text = model->text;
	c->acceleratorIndex = model->acceleratorIndex;
	c->buttonFeatures = UMenuItemModel::None;;//model->buttonFeatures;
	c->checkType = checkType;
	c->maxIconWidth = maxIconWidth;
	c->shortcut = shortcut;
	delete (model);
	return c;
}

UMenuItem::UMenuItem()
	: UButton()
{
	setCssType("menuitem");
	m_model = _createMenuItemModel(getCompoundModel(), UMenuItemModel::NotCheckable, 16, UKeyStroke());
}

UMenuItem::UMenuItem(const std::string & text, UIcon * icon)
	: UButton(text, icon)
{
	setCssType("menuitem");
	m_model = _createMenuItemModel(getCompoundModel(), UMenuItemModel::NotCheckable, 16, getAccelerator());
}

UMenuItem::UMenuItem(UIcon * icon)
	: UButton(icon)
{
	setCssType("menuitem");
	m_model = _createMenuItemModel(getCompoundModel(), UMenuItemModel::NotCheckable, 16, UKeyStroke());
}


UMenu *
UMenuItem::getParentMenu() const {
	if (getParent()) {
		UPopupMenu * popup = dynamic_cast<UPopupMenu*>(getParent());
		if (popup) {
			return dynamic_cast<UMenu*>(popup->getInvoker());
		}
	}
	return NULL;
}

//
// Overrides UWidget
//

void
UMenuItem::activate() {
	UMenuManager::getMenuManager()->activateItem(this);
	UButton::activate();
}

UStyle::ComponentElement
UMenuItem::getStyleType() const {
	if (dynamic_cast<UMenuBar*>(getParent())) {
		return UStyle::CE_MenuBarItem;
	} else {
		return UStyle::CE_MenuItem;
	}
}

void
UMenuItem::processMouseEvent(UMouseEvent * e) {
	UMenuManager::getMenuManager()->processMouseEvent(e);
	return;
	// FYI: this is generic event code which does not need a this pointer
	switch (e->getType()) {
		case UEvent::MousePressed:
			if (UMenu * menu = dynamic_cast<UMenu*>(e->getSource())) {
				if (menu->contains(e->getLocation())) {
					e->consume();
					menu->requestFocus();
					menu->activate();
				}
			}
		break;
		case UEvent::MouseReleased: {
			// FIXME: should always be this?
			UWidget * w_under_mouse = e->getWidget();
			if (!w_under_mouse->contains(e->getLocation()) &&
					e->getWidget()->getRootPane(true)) {
				w_under_mouse = e->getWidget()->
					getRootPane(true)->getVisibleWidgetAt(e->getRootLocation());
			}

			if (dynamic_cast<UMenu*>(w_under_mouse) == NULL) {
				if (UMenuItem * item = dynamic_cast<UMenuItem*>(w_under_mouse)) {
					e->consume();
					item->activate();
				}
			}
		}
		break;
		case UEvent::MouseEntered:
			if (dynamic_cast<UMenu*>(this)) {
				UMenuManager::getMenuManager()->activateItem(this);
			} else {
				UMenuManager::getMenuManager()->highlightItem(this);
			}
		break;
		default:
		break;
	}
	UWidget::processMouseEvent(e);
}

void
UMenuItem::processKeyEvent(UKeyEvent * e) {
	UMenuManager::getMenuManager()->processKeyEvent(e);
	UWidget::processKeyEvent(e);
}

UMenuItemModel *
UMenuItem::getMenuItemModel() const {
	return static_cast<UMenuItemModel*>(m_model);
}
