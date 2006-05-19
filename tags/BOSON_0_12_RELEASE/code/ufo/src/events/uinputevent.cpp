/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/events/uinputevent.cpp
    begin             : Wed May 9 2001
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

#include "ufo/events/uinputevent.hpp"

using namespace ufo;

UFO_IMPLEMENT_ABSTRACT_CLASS(UInputEvent, UWidgetEvent)

UInputEvent::UInputEvent(UWidget * sourceA, Type typeA, UMod_t modifiersA)
	: UWidgetEvent(sourceA, typeA)
	, m_modifiers(modifiersA) {}


UMod_t
UInputEvent::getModifiers() {
	return m_modifiers;
}


bool
UInputEvent::isControlDown() {
	return (m_modifiers & UMod::Ctrl);
}

bool
UInputEvent::isAltDown() {
	return (m_modifiers & UMod::Alt);
}

bool
UInputEvent::isAltGraphDown() {
	return (m_modifiers & UMod::AltGraph);
}

bool
UInputEvent::isShiftDown() {
	return (m_modifiers & UMod::Shift);
}

bool
UInputEvent::isCapsDown() {
	return (m_modifiers & UMod::Caps);
}

bool
UInputEvent::isMetaDown() {
	return (m_modifiers & UMod::Meta);
}

bool
UInputEvent::isNumDown() {
	return (m_modifiers & UMod::Num);
}

bool
UInputEvent::hasMouseModifiers() {
	return (m_modifiers & UMod::MouseModifierMask);
}

bool
UInputEvent::hasKeyboardModifiers() {
	return (m_modifiers & UMod::KeyboardModifierMask);
}

//*
//* protected
//*
std::ostream &
UInputEvent::paramString(std::ostream & os) const {
	UEvent::paramString(os);
	os << ";modifiers " << m_modifiers;
	return os;
}
