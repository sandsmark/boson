/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/basic/ubasiccheckboxui.cpp
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

#include "ufo/ui/basic/ubasiccheckboxui.hpp"

#include "ufo/widgets/ucheckbox.hpp"

#include "ufo/uicon.hpp"

#include "ufo/ugraphics.hpp"
#include "ufo/ui/uuimanager.hpp"
#include "ufo/ui/ustyle.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UBasicCheckBoxUI, UButtonUI)

UBasicCheckBoxUI * UBasicCheckBoxUI::m_checkBoxUI = new UBasicCheckBoxUI();
std::string UBasicCheckBoxUI::m_lafId("UCheckBox");

UBasicCheckBoxUI *
UBasicCheckBoxUI::createUI(UWidget * w) {
	return m_checkBoxUI;
}

void
UBasicCheckBoxUI::installUI(UWidget * w) {
	UBasicButtonUI::installUI(w);

	UButton * button = dynamic_cast<UButton*>(w);
	button->setRolloverEnabled(false);

	UUIManager * manager = w->getUIManager();
	button->setIcon(static_cast<UIcon*>(manager->get(getLafId() + ".icon")));
	button->setPressedIcon(static_cast<UIcon*>(manager->get(getLafId() + ".selectedIcon")));
	button->setDisabledIcon(static_cast<UIcon*>(manager->get(getLafId() + ".inactiveIcon")));
}

void
UBasicCheckBoxUI::uninstallUI(UWidget * w) {
	UBasicButtonUI::uninstallUI(w);

	UButton * button = dynamic_cast<UButton*>(w);
	button->setRolloverEnabled(true);

	button->setIcon(NULL);
	button->setPressedIcon(NULL);
	button->setDisabledIcon(NULL);
}


void
UBasicCheckBoxUI::paint(UGraphics * g, UWidget * w) {
	UButtonUI::paint(g, w);

	UButton * button;
	button = dynamic_cast<UButton*>(w);

	const UFont * f = button->getFont();
	const UInsets & insets = button->getInsets();
	UStyle * style = w->getUIManager()->getStyle();

	URectangle viewRect(insets.left, insets.top,
		button->getWidth() - insets.getHorizontal(),
		button->getHeight() - insets.getVertical());
	// due to focus we add some spacing to the view rect
	if (button->isFocusable()) {
		viewRect.x += 1;
		viewRect.y += 1;
		viewRect.w -= 2;
		viewRect.h -= 2;
	}
	/*if (button->isPressed()) {
		g->translate(1, 1);//glTranslatef(1, 1, 0);
	}*/

	style->paintCompoundTextAndIcon(g, button, viewRect,
		button->getText(), button->getIcon());

	/*if (button->isPressed()) {
		g->translate(-1, -1);//glTranslatef( -1, -1, 0);
	}*/
}

const std::string &
UBasicCheckBoxUI::getLafId() {
	return m_lafId;
}
