/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/basic/ubasicmenuitemui.cpp
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

#include "ufo/ui/basic/ubasicmenuitemui.hpp"

#include "ufo/ui/ustyle.hpp"
#include "ufo/ui/uuimanager.hpp"

#include "ufo/widgets/umenuitem.hpp"
#include "ufo/widgets/umenu.hpp"
#include "ufo/widgets/upopupmenu.hpp"
#include "ufo/widgets/urootpane.hpp"

#include "ufo/uicon.hpp"

#include "ufo/font/ufont.hpp"
#include "ufo/ugraphics.hpp"
#include "ufo/umenumanager.hpp"

#include "ufo/util/ucolor.hpp"
#include "ufo/events/umouseevent.hpp"


using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UBasicMenuItemUI, UMenuItemUI)

UBasicMenuItemUI * UBasicMenuItemUI::m_menuItemUI = new UBasicMenuItemUI();

std::string UBasicMenuItemUI::m_lafId("UMenuItem");


UBasicMenuItemUI *
UBasicMenuItemUI::createUI(UWidget * w) {
	return m_menuItemUI;
}


void
UBasicMenuItemUI::paint(UGraphics * g, UWidget * w) {
	UMenuItem * menuItem = dynamic_cast<UMenuItem *>(w);

	UUIManager * manager = w->getUIManager();
	URectangle viewRect(menuItem->getInnerBounds());

	// FIXME
	// should this be done by the WidgetUI class?
	// paint background
	if (menuItem->isRollover()) {
		g->setColor(w->getColorGroup().highlight());
		g->fillRect(0, 0, w->getWidth(), w->getHeight());
		//g->fillRect(insets.left, insets.top, innerSize.w, innerSize.h);
	}/* else {
		UColor color(w->getColorGroup().background());
		color.getFloat()[3] = w->getOpacity();
		g->setColor(color);
		g->fillRect(viewRect.x, viewRect.y, viewRect.w, viewRect.h);
	}*/

	UStyle * style = w->getUIManager()->getStyle();
	if (menuItem->isPressed()) {
		g->translate(1, 1);//glTranslatef(1, 1, 0);
	}


	style->paintCompoundTextAndIcon(g, menuItem, viewRect,
		menuItem->getText(), menuItem->getIcon());

	if (menuItem->getPopupMenu() && dynamic_cast<UPopupMenu*>(menuItem->getParent())) {
		URectangle rect(menuItem->getWidth() - 10,
			(menuItem->getHeight() / 2) - 3, 6, 8);

		style->paintArrow(g, menuItem,
			rect,
			false, menuItem->getPopupMenu()->isVisible(),
			Right);
	}
	if (menuItem->isPressed()) {
		g->translate(-1, -1);//glTranslatef( -1, -1, 0);
	}
}


const std::string &
UBasicMenuItemUI::getLafId() {
	return m_lafId;
}


void
UBasicMenuItemUI::installUI(UWidget * w) {
	UMenuItemUI::installUI(w);
	w->setMargin(2, 4, 2, 4);
	installSignals(dynamic_cast<UMenuItem*>(w));
}

void
UBasicMenuItemUI::uninstallUI(UWidget * w) {
	UMenuItemUI::installUI(w);
	w->setMargin(0, 0, 0, 0);
	uninstallSignals(dynamic_cast<UMenuItem*>(w));
}

UDimension
UBasicMenuItemUI::getPreferredSize(const UWidget * w) {
	const UMenuItem * item = dynamic_cast<const UMenuItem*>(w);
	//return UUIUtilities::getCompoundPreferredSize(button);
	UDimension ret = w->getUIManager()->getStyle()->getCompoundPreferredSize(
		item,
		item->getFont(),
		item->getText(),
		item->getIcon()
	);
	// add extra space for a sub menu icon
	if (item->getPopupMenu() && dynamic_cast<UPopupMenu*>(item->getParent())) {
		ret.w += 10;
	}
	return ret;
}


void
UBasicMenuItemUI::installSignals(UMenuItem * item) {
	item->sigMousePressed().connect(slot(*this, &UBasicMenuItemUI::mousePressed));
	item->sigMouseReleased().connect(slot(*this, &UBasicMenuItemUI::mouseReleased));

	item->sigMouseEntered().connect(slot(*this, &UBasicMenuItemUI::mouseEntered));
	item->sigMouseExited().connect(slot(*this, &UBasicMenuItemUI::mouseExited));
}



void
UBasicMenuItemUI::uninstallSignals(UMenuItem * item) {
	item->sigMousePressed().disconnect(slot(*this, &UBasicMenuItemUI::mousePressed));
	item->sigMouseReleased().disconnect(slot(*this, &UBasicMenuItemUI::mouseReleased));

	item->sigMouseEntered().disconnect(slot(*this, &UBasicMenuItemUI::mouseEntered));
	item->sigMouseExited().disconnect(slot(*this, &UBasicMenuItemUI::mouseExited));
}


void
UBasicMenuItemUI::mousePressed(UMouseEvent *e) {
	if (UMenu * menu = dynamic_cast<UMenu*>(e->getSource())) {
		if (menu->contains(e->getLocation())) {
			e->consume();
			menu->activate();
		}
		//UMenuManager::getMenuManager()->activateItem(item);
	}
}

void
UBasicMenuItemUI::mouseReleased(UMouseEvent * e) {
	UWidget * w_under_mouse = e->getWidget();
	if (!w_under_mouse->contains(e->getLocation())) {
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

void
UBasicMenuItemUI::mouseEntered(UMouseEvent * e) {
	if (UMenuItem * item = dynamic_cast<UMenuItem*>(e->getSource())) {
		e->consume();
		UMenuManager::getMenuManager()->highlightItem(item);
		if (item->isRolloverEnabled()) {
			item->setRollover(true);
		}
	}
}

void
UBasicMenuItemUI::mouseExited(UMouseEvent * e) {
	if (UMenuItem * item = dynamic_cast<UMenuItem*>(e->getSource())) {
		e->consume();
		item->setRollover(false);
	}
}
