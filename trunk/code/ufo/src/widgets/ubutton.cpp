/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/widgets/ubutton.cpp
    begin             : Mon May 28 2001
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

#include "ufo/widgets/ubutton.hpp"

#include "ufo/utoolkit.hpp"
#include "ufo/udisplay.hpp"
#include "ufo/events/uactionevent.hpp"
#include "ufo/events/utimerevent.hpp"

#include "ufo/uicon.hpp"
#include "ufo/ubuttongroup.hpp"

#include "ufo/ukeystroke.hpp"
#include "ufo/uinputmap.hpp"
#include "ufo/widgets/urootpane.hpp"
//#include "ufo/ui/uuimanager.hpp"

using namespace ufo;

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UButton, UCompound)

UButton::UButton()
	: UCompound()
	, m_pressedIcon(NULL)
	, m_rolloverIcon(NULL)
	, m_actionCommand("")
	, m_flags(RolloverEnabled | BorderPainted)
	, m_buttonGroup(NULL)
	, m_accelerator()
	, m_acceleratorIndex(-1)
{
	trackPointer(m_pressedIcon);
	trackPointer(m_rolloverIcon);
}

UButton::UButton(UIcon * icon)
	: UCompound(icon)
	, m_pressedIcon(NULL)
	, m_rolloverIcon(NULL)
	, m_actionCommand("")
	, m_flags(RolloverEnabled | BorderPainted)
	, m_buttonGroup(NULL)
	, m_accelerator()
	, m_acceleratorIndex(-1)
{
	trackPointer(m_pressedIcon);
	trackPointer(m_rolloverIcon);
}

UButton::UButton(const std::string & text, UIcon * icon)
	: UCompound(text, icon)
	, m_pressedIcon(NULL)
	, m_rolloverIcon(NULL)
	, m_actionCommand("")
	, m_flags(RolloverEnabled | BorderPainted)
	, m_buttonGroup(NULL)
	, m_accelerator()
	, m_acceleratorIndex(-1)
{
	trackPointer(m_pressedIcon);
	trackPointer(m_rolloverIcon);

	// we need this to remove '&' for accelerators
	setText(text);
}


//*
//* hides | overrides UWidget
//*
/*
void
UButton::setUI(UButtonUI * ui) {
	UWidget::setUI(ui);
}

UWidgetUI *
UButton::getUI() const {
	return static_cast<UButtonUI*>(UWidget::getUI());
}

void
UButton::updateUI() {
	setUI(static_cast<UButtonUI*>(getUIManager()->getUI(this)));
}
*/

//
// Overrides UCompound
//

UIcon *
UButton::getIcon() const {
	UIcon * icon = NULL;

	if (!isEnabled()) {
		icon = getDisabledIcon();
	} else if (isPressed() || isSelected()) {
		icon = getPressedIcon();
	} else if (isRolloverEnabled() && isRollover()) {
		icon = getRolloverIcon();
	}

	// if there is no pressed/.. icon or if this button is not pressed/..
	if (!icon) {
		icon = getDefaultIcon();
	}
	return icon;
}


void
UButton::setText(const std::string & text) {
	unsigned int index = text.find('&');
	if (index < text.length() - 1 && text[index + 1] != '&') {
		std::string accel("Alt");
		accel += '+';
		accel += text[index + 1];
		setAccelerator(accel);
		std::string newtext(text);
		newtext.erase(index, 1);
		UCompound::setText(newtext);
		m_acceleratorIndex = index;
	} else {
		UCompound::setText(text);
	}
}

//
// public API
//

void
UButton::doClick() {
	doClick(100);
}

void
UButton::doClick(int millis) {
	setPressed(true);

	if (millis) {
		UDisplay::getDefault()->pushEvent(new UTimerEvent(100, slot(*this, &UButton::buttonUp)));
	} else {
		buttonUp();
	}
}

bool
UButton::isSelected() const {
	return (m_flags & Selected);
}
void
UButton::setSelected(bool b) {
	if (b) {
		m_flags |= Selected;
	} else {
		m_flags &= ~Selected;
	}
	repaint();
}

bool
UButton::isPressed( ) const {
	return (m_flags & Pressed);
}
void
UButton::setPressed(bool b) {//, UMod_t modifiers) {
	if (b) {
		m_flags |= Pressed;
	} else {
		m_flags &= ~Pressed;
/*
		// this button was clicked
		if (isArmed()) {
			if (isToggable()) {
				if (m_buttonGroup) {
					m_buttonGroup->setSelectedButton(this, true);
				} else {
					setSelected(!isSelected());
				}
			}
			fireActionPerformed(modifiers);
		}
		*/
	}
	repaint();
}

bool
UButton::isRolloverEnabled() const {
	return (m_flags & RolloverEnabled);
}
void
UButton::setRolloverEnabled(bool b) {
	if (b) {
		m_flags |= RolloverEnabled;
	} else {
		m_flags &= ~RolloverEnabled;
	}
}

bool
UButton::isRollover() const {
	return (m_flags & Rollover);
}
void
UButton::setRollover(bool b) {
	if (b) {
		m_flags |= Rollover;
	} else {
		m_flags &= ~Rollover;
	}
	repaint();
}


void
UButton::setButtonGroup(UButtonGroup * buttonGroup) {
	m_buttonGroup = buttonGroup;
}
UButtonGroup *
UButton::getButtonGroup() const {
	return m_buttonGroup;
}

void
UButton::activate() {
	if (isToggable()) {
		if (m_buttonGroup) {
			m_buttonGroup->setSelectedButton(this, true);
		} else {
			setSelected(!isSelected());
		}
	}
	fireActionEvent();
}


bool
UButton::isArmed() const {
	return (m_flags & Armed);
}
void
UButton::setArmed(bool b) {
	if (b) {
		m_flags |= Armed;
	} else {
		m_flags &= ~Armed;
	}
	repaint();
}

void
UButton::buttonUp() {
	setPressed(false);
	activate();
}


bool
UButton::isToggable() const {
	return (m_flags & Toggable);
}
void
UButton::setToggable(bool b) {
	if (b) {
		m_flags |= Toggable;
	} else {
		m_flags &= ~Toggable;
	}
}


std::string
UButton::getActionCommand() const {
	return (m_actionCommand != "") ? m_actionCommand : getText();
}
void
UButton::setActionCommand(const std::string & actionCommand) {
	m_actionCommand = actionCommand;
}

void
UButton::setPressedIcon(UIcon * icon) {
	swapPointers(m_pressedIcon, icon);
	m_pressedIcon = icon;
	repaint();
}


void
UButton::setRolloverIcon(UIcon * icon) {
	swapPointers(m_rolloverIcon, icon);
	m_rolloverIcon = icon;
	repaint();
}


bool
UButton::isBorderPainted() const {
	return (m_flags & BorderPainted);
}
void
UButton::setBorderPainted(bool b) {
	if (b) {
		m_flags |= BorderPainted;
	} else {
		m_flags &= ~BorderPainted;
	}
	repaint();
}

UInsets
UButton::getInsets() const {
	if (isBorderPainted()) {
		return UWidget::getInsets();
	} else {
		return UInsets();
	}
}



//
// Protected methods
//

void
UButton::paintBorder(UGraphics * g) {
	if (isBorderPainted()) {
		UWidget::paintBorder(g);
	}
}


void
UButton::addedToHierarchy() {
	if (m_accelerator.getKeyCode() != UKey::UK_UNDEFINED) {
		getRootPane(true)->getInputMap(WhenAncestorFocused)
			->put(m_accelerator, slot(*this, &UButton::keybindingSlot));
	}
	UWidget::addedToHierarchy();
}

void
UButton::removedFromHierarchy() {
	// about to be removed
	if (m_accelerator.getKeyCode() != UKey::UK_UNDEFINED) {
		if (getRootPane(true)) {
			getRootPane(true)->getInputMap(WhenAncestorFocused)
				->put(m_accelerator, NULL);
		}
	}
	UWidget::removedFromHierarchy();
}

void
UButton::fireActionEvent() {
	UMod_t modifiers = UMod::NoModifier;
	// FIXME: Use modifiers from current event?
	UActionEvent * ae = new UActionEvent(this, UEvent::Action,
		modifiers, getActionCommand());
	ae->reference();
	m_sigActivated(ae);
	ae->unreference();
}

void
UButton::keybindingSlot(UActionEvent * e) {
	doClick();
}

/*
void
UButton::setMnemonic(UKeyCode_t keyA) {
	m_mnemonic = keyA;
	UKeyStroke k(m_mnemonic, UMod::Alt);
	if (getParent()) {
		if (URootPane * root = getRootPane(true)) {
			root->getInputMap(WhenAncestorFocused)->put(k, slot(*this, &UButton::fireAction));
		}
	}
}

UKeyCode_t
UButton::getMnemonic() const {
	return m_mnemonic;
}*/

void
UButton::setAccelerator(const UKeyStroke & stroke) {
	m_accelerator = stroke;

	// search for accel index
	std::string text(getText());
	unsigned int index;

	// try lower case first
	index = text.find(char(/*std::*/tolower(stroke.getKeyCode() + 32)));
	if (index < text.length()) {
		m_acceleratorIndex = index;
	} else {
		index = text.find(char(stroke.getKeyCode()));
		if (index < text.length()) {
			m_acceleratorIndex = index;
		}
	}

	if (isInValidHierarchy()) {
		if (URootPane * root = getRootPane(true)) {
			root->getInputMap(WhenAncestorFocused)->
				put(stroke, slot(*this, &UButton::keybindingSlot));
		}
	}
}

UKeyStroke
UButton::getAccelerator() const {
	return m_accelerator;
}

int
UButton::getAcceleratorIndex() const {
	return m_acceleratorIndex;
}
