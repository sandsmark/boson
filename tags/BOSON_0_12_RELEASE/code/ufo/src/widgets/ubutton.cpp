/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
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
#include "ufo/events/umouseevent.hpp"
#include "ufo/events/ukeyevent.hpp"
#include "ufo/events/utimerevent.hpp"
#include "ufo/events/ushortcutevent.hpp"

#include "ufo/uicon.hpp"
#include "ufo/ubuttongroup.hpp"

#include "ufo/ukeystroke.hpp"
#include "ufo/uinputmap.hpp"
#include "ufo/widgets/urootpane.hpp"
#include "ufo/ui/ustylehints.hpp"
#include "ufo/ui/ustyle.hpp"
#include "ufo/umodel.hpp"
//#include "ufo/ui/uuimanager.hpp"

using namespace ufo;

UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(UButton, UCompound)

UButton::UButton()
	: UCompound()
	, m_pressedIcon(NULL)
	, m_rolloverIcon(NULL)
	, m_actionCommand("")
	, m_buttonGroup(NULL)
	, m_accelerator()
	, m_acceleratorIndex(-1)
{
	trackPointer(m_pressedIcon);
	trackPointer(m_rolloverIcon);
	setCssType("button");
}

UButton::UButton(UIcon * icon)
	: UCompound(icon)
	, m_pressedIcon(NULL)
	, m_rolloverIcon(NULL)
	, m_actionCommand("")
	, m_buttonGroup(NULL)
	, m_accelerator()
	, m_acceleratorIndex(-1)
{
	trackPointer(m_pressedIcon);
	trackPointer(m_rolloverIcon);
	setCssType("button");
}

UButton::UButton(const std::string & text, UIcon * icon)
	: UCompound(text, icon)
	, m_pressedIcon(NULL)
	, m_rolloverIcon(NULL)
	, m_actionCommand("")
	, m_buttonGroup(NULL)
	, m_accelerator()
	, m_acceleratorIndex(-1)
{
	trackPointer(m_pressedIcon);
	trackPointer(m_rolloverIcon);
	setCssType("button");

	// we need this to remove '&' for accelerators
	setText(text);
}

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
	/*unsigned int index = text.find('&');
	if (index < text.length() - 1 && text[index + 1] != '&') {
		std::string accel("Alt");
		accel += '+';
		accel += text[index + 1];
		setAccelerator(accel);
		std::string newtext(text);
		newtext.erase(index, 1);
		UCompound::setText(newtext);
		m_acceleratorIndex = index;
	} else*/ {
		UCompound::setText(text);
	}/*
	if (getAcceleratorIndex() != -1) {
		std::string accel("Alt+");
		accel += getText()[getAcceleratorIndex()];
		setAccelerator(accel);
	}*/
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
	return testState(WidgetSelected);
}
void
UButton::setSelected(bool b) {
	// test if something changed
	if (b == isSelected()) {
		return;
	}
	setState(WidgetSelected, b);

	if (b && m_buttonGroup) {
		// we do not get an infinite recursion as UButtonGroup checks
		// whether this button is selected before explicitly selecting it
		m_buttonGroup->setSelectedButton(this, b);
	}

	// fire toggle event
	fireActionEvent();

	repaint();
}

bool
UButton::isPressed( ) const {
	return testState(WidgetPressed);
}
void
UButton::setPressed(bool b) {
	setState(WidgetPressed, b);
}

bool
UButton::isRolloverEnabled() const {
	//return (m_flags & RolloverEnabled);
	return true;
}
void
UButton::setRolloverEnabled(bool b) {
	if (b) {
		//m_flags |= RolloverEnabled;
	} else {
		//m_flags &= ~RolloverEnabled;
	}
}

bool
UButton::isRollover() const {
	return (hasMouseFocus());// & isRolloverEnabled());
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
			m_buttonGroup->setSelectedButton(this, !isSelected());
		} else {
			setSelected(!isSelected());
		}
	} else {
		fireActionEvent();
	}
}

void
UButton::buttonUp() {
	setPressed(false);
	activate();
}


bool
UButton::isToggable() const {
	return testState(WidgetToggable);
}
void
UButton::setToggable(bool b) {
	setState(WidgetToggable);
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

UDimension
UButton::getContentsSize(const UDimension & maxSize) const {
	UDimension ret(getStyle()->getCompoundPreferredSize(
		getStyleHints(),
		getText(),
		getIcon())
	);

	if (ret.isValid()) {
		ret.clamp(maxSize);
		return ret;
	}
	return UDimension::invalid;
}


bool
UButton::isActive() const {
	return hasMouseFocus();
}


//
// Protected methods
//

void
UButton::processKeyEvent(UKeyEvent * e) {
	if (e->isConsumed()) {
		return;
	}

	UKeyCode_t key = e->getKeyCode();

	if (key == UKey::UK_SPACE) {
		if (e->getType() == UEvent::KeyPressed) {
			setPressed(true);
			e->consume();
		} else if (e->getType() == UEvent::KeyReleased && isPressed()) {
			// FIXME: check for key repeat events and ignore them
			setPressed(false);
			activate();
			e->consume();
		}
	}
	UWidget::processKeyEvent(e);
}

static UButton * s_mouse_press_button = NULL;
void
UButton::processMouseEvent(UMouseEvent * e) {
	switch (e->getType()) {
		case UEvent::MousePressed:
			e->consume();
			s_mouse_press_button = this;
			setPressed(true);
			requestFocus();
		break;
		case UEvent::MouseReleased:
			e->consume();
			if (s_mouse_press_button == this && contains(e->getLocation())) {
				activate();
			}
			setPressed(false);
			s_mouse_press_button = NULL;
		break;
		case UEvent::MouseEntered:
			if (s_mouse_press_button == this) {
				e->consume();
				setPressed(true);
			}
		break;
		case UEvent::MouseExited:
			if (isPressed()) {
				e->consume();
				setPressed(false);
			}
		break;
		default:
		break;
	}
	UWidget::processMouseEvent(e);
}

void
UButton::processShortcutEvent(UShortcutEvent * e) {
	if (isVisible() && isEnabled()) {
		requestFocus();
		if (!e->isAmbiguous()) {
			doClick();
		}
		e->consume();
	}
	UWidget::processShortcutEvent(e);
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
	// remove old accelerator
	if (m_accelerator.getKeyCode() != UKey::UK_UNDEFINED) {
		releaseShortcut(m_accelerator);
	}
	m_accelerator = stroke;

	// check whether this is a mnemonic accelerator
	if (stroke.getModifiers() == UMod::Alt) {
		// release old mnemonic
		if (getCompoundModel()->acceleratorIndex != -1) {
			std::string accel("Alt+");
			accel += '+';
			accel += getText()[getCompoundModel()->acceleratorIndex];

			releaseShortcut(accel);
		}
		// grab new mnemonic
		std::string text(getText());
		std::string::size_type index;
		// try lower case first
		index = text.find(char(/*std::*/tolower(stroke.getKeyCode() + 32)));

		if (index >= text.length()) {
			// now normal
			index = text.find(char(stroke.getKeyCode()));
			if (index >= text.length()) {
				//FIXME: oops, nothing
				getCompoundModel()->acceleratorIndex = -1;
				updateMnemonic();
				return;
			}
		}
		getCompoundModel()->acceleratorIndex = int(index);
		updateMnemonic();
	} else {
		// no mnemonic, grab it ourself
		grabShortcut(stroke);
	}
}

UKeyStroke
UButton::getAccelerator() const {
	return m_accelerator;
}

int
UButton::getAcceleratorIndex() const {
	return getCompoundModel()->acceleratorIndex;
}
