/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ui/basic/ubasicbuttonui.cpp
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

#include "ufo/ui/basic/ubasicbuttonui.hpp"

#include "ufo/ugraphics.hpp"
#include "ufo/ui/ustyle.hpp"

#include "ufo/ui/uuimanager.hpp"

#include "ufo/widgets/ubutton.hpp"

#include "ufo/uicon.hpp"
#include "ufo/ucontext.hpp"

#include "ufo/util/ucolor.hpp"

#include "ufo/events/umouseevent.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UBasicButtonUI, UButtonUI)

UBasicButtonUI * UBasicButtonUI::m_buttonUI = new UBasicButtonUI();
std::string UBasicButtonUI::m_lafId("UButton");


UBasicButtonUI::UBasicButtonUI()
{}

UBasicButtonUI *
UBasicButtonUI::createUI(UWidget * w) {
	return m_buttonUI;
}

void
UBasicButtonUI::installUI(UWidget * w) {
	UButtonUI::installUI(w);

	//w->setMargin(5, 12, 5, 12);
	//w->setMargin(2, 4, 2, 4);
	installSignals(dynamic_cast<UButton*>(w));
}

void
UBasicButtonUI::uninstallUI(UWidget * w) {
	UButtonUI::uninstallUI(w);

	//w->setMargin(0, 0, 0, 0);
	uninstallSignals(dynamic_cast<UButton*>(w));
}


void
UBasicButtonUI::paint(UGraphics * g, UWidget * w) {
	UButtonUI::paint(g, w);
	//g->paintControlBackground(g, w, isActive(), false);

	UButton * button;
	button = dynamic_cast<UButton*>(w);

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

	if (button->isPressed()) {
		g->translate(1, 1);//glTranslatef(1, 1, 0);
	}

	style->paintCompoundTextAndIcon(g, button, viewRect,
		button->getText(), button->getIcon());

	if (button->isPressed()) {
		g->translate(-1, -1);//glTranslatef( -1, -1, 0);
	}

	if (button->isFocused()) {
		style->paintFocus(g, button, viewRect);
	}
}

const std::string &
UBasicButtonUI::getLafId() {
	return m_lafId;
}



UDimension
UBasicButtonUI::getPreferredSize(const UWidget * w) {
	const UButton * button = dynamic_cast<const UButton*>(w);
	//UDimension ret = UUIUtilities::getCompoundPreferredSize(button);
	UDimension ret = w->getUIManager()->getStyle()->getCompoundPreferredSize(
		button,
		button->getFont(),
		button->getText(),
		button->getIcon()
	);
	if (button->isFocusable()) {
		// Add some margin to allow focus painting
		ret.w += 2;
		ret.h += 2;
	}
	return ret;
}



void
UBasicButtonUI::installSignals(UButton * b) {
	b->sigMousePressed().connect(slot(*this, &UBasicButtonUI::mousePressed));
	b->sigMouseReleased().connect(slot(*this, &UBasicButtonUI::mouseReleased));

	b->sigMouseEntered().connect(slot(*this, &UBasicButtonUI::mouseEntered));
	b->sigMouseExited().connect(slot(*this, &UBasicButtonUI::mouseExited));
}



void
UBasicButtonUI::uninstallSignals(UButton * b) {
	b->sigMousePressed().disconnect(slot(*this, &UBasicButtonUI::mousePressed));
	b->sigMouseReleased().disconnect(slot(*this, &UBasicButtonUI::mouseReleased));

	b->sigMouseEntered().disconnect(slot(*this, &UBasicButtonUI::mouseEntered));
	b->sigMouseExited().disconnect(slot(*this, &UBasicButtonUI::mouseExited));
}

static UButton * s_mouse_press_button = NULL;
void
UBasicButtonUI::mousePressed(UMouseEvent *e) {
	UButton * b = dynamic_cast<UButton*>(e->getSource());
	if (b && (e->getButton() & UMod::LeftButton)) {
		e->consume();
		s_mouse_press_button = b;
		//b->setArmed(true);
		b->setPressed(true);

		b->requestFocus();
		b->repaint();
	}
}
void
UBasicButtonUI::mouseReleased(UMouseEvent *e) {
	UButton * b = dynamic_cast<UButton*>(e->getSource());
	if (b && (e->getButton() & UMod::LeftButton)) {
		e->consume();
		if (b->contains(e->getLocation())) {
			b->activate();
			b->setPressed(false);
			b->repaint();
		}
		s_mouse_press_button = NULL;
	}
}

void
UBasicButtonUI::mouseEntered(UMouseEvent * e) {
	if (UButton * b = dynamic_cast<UButton*>(e->getSource())) {
		e->consume();
		if (b->isRolloverEnabled()) {
			b->setRollover(true);
		}
		/*if (e->hasMouseModifiers() && b->isArmed()) {
			b->setPressed(true);
		//} else {
			b->setArmed(false);
		}*/
		if (s_mouse_press_button == b) {
			b->setPressed(true);
		}
		b->repaint();
	}
}
void
UBasicButtonUI::mouseExited(UMouseEvent * e) {
	if (UButton * b = dynamic_cast<UButton*>(e->getSource())) {
		e->consume();
		if (b->isRolloverEnabled()) {
			b->setRollover(false);
		}
		b->setPressed(false);
		b->repaint();
	}
}
