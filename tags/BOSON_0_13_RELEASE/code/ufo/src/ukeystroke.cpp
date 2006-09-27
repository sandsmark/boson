/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ukeystroke.cpp
    begin             : Mon Feb 11 2002
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

#include "ufo/ukeystroke.hpp"

#include "ufo/events/ukeyevent.hpp"
#include "ufo/util/ustring.hpp"


using namespace ufo;

UFO_IMPLEMENT_ABSTRACT_CLASS(UKeyStroke, UObject)


UKeyStroke::UKeyStroke()
	: m_keyChar(UKeyEvent::CHAR_UNDEFINED)
	, m_keyCode(UKey::UK_UNDEFINED)
	, m_modifiers(UMod::NoModifier)
	, m_onKeyRelease(false)
{
}

UKeyStroke::UKeyStroke(const char * key)
	: m_keyChar(UKeyEvent::CHAR_UNDEFINED)
	, m_keyCode(UKey::UK_UNDEFINED)
	, m_modifiers(UMod::NoModifier)
	, m_onKeyRelease(false)
{
	initFromString(key);
}

UKeyStroke::UKeyStroke(const std::string & key)
	: m_keyChar(UKeyEvent::CHAR_UNDEFINED)
	, m_keyCode(UKey::UK_UNDEFINED)
	, m_modifiers(UMod::NoModifier)
	, m_onKeyRelease(false)
{
	initFromString(key.c_str());
}

UKeyStroke::UKeyStroke(wchar_t keyChar, bool onKeyRelease)
	: m_keyChar(keyChar)
	, m_keyCode(UKey::UK_UNDEFINED)
	, m_modifiers(UMod::NoModifier)
	, m_onKeyRelease(onKeyRelease)
{
}

UKeyStroke::UKeyStroke(UKeyCode_t keyCode, UMod_t modifiers,
		bool onKeyRelease)
	: m_keyChar(UKeyEvent::CHAR_UNDEFINED)
	, m_keyCode(keyCode)
	, m_modifiers(modifiers)
	, m_onKeyRelease(onKeyRelease)
{
}

UKeyStroke::UKeyStroke(wchar_t keyChar, UKeyCode_t keyCode,
		UMod_t modifiers, bool onKeyRelease)
	: m_keyChar(keyChar)
	, m_keyCode(keyCode)
	, m_modifiers(modifiers)
	, m_onKeyRelease(onKeyRelease)
{
}

UKeyStroke::UKeyStroke(UKeyEvent * e)
	: m_keyChar(UKeyEvent::CHAR_UNDEFINED)
	, m_keyCode(UKey::UK_UNDEFINED)
	, m_modifiers(UMod::NoModifier)
	, m_onKeyRelease(false)
{
	if (e->getType() == UEvent::KeyTyped) {
		m_keyChar = e->getKeyChar();
	} else {
		// upper case key char
		m_keyCode = UKeyCode_t(/*std::*/toupper(e->getKeyCode()));
		m_modifiers = e->getModifiers();
		m_onKeyRelease = (e->getType() == UKeyEvent::KeyReleased);
	}
}

bool
UKeyStroke::operator==(const UKeyStroke & stroke) const {
	// special case for shift, alt and ctrl
	int mod = m_modifiers;
	int stroke_mod = stroke.m_modifiers;
	if (((mod & UMod::Alt) == UMod::Alt && (stroke_mod & UMod::Alt)) ||
			((stroke_mod & UMod::Alt) == UMod::Alt && (mod & UMod::Alt))) {
		mod &= ~UMod::Alt;
		stroke_mod &= ~UMod::Alt;
	}
	if (((mod & UMod::Shift) == UMod::Shift && (stroke_mod & UMod::Shift)) ||
			((stroke_mod & UMod::Shift) == UMod::Shift && (mod & UMod::Shift))) {
		mod &= ~UMod::Shift;
		stroke_mod &= ~UMod::Shift;
	}
	if (((mod & UMod::Ctrl) == UMod::Ctrl && (stroke_mod & UMod::Ctrl)) ||
			((stroke_mod & UMod::Ctrl) == UMod::Ctrl && (mod & UMod::Ctrl))) {
		mod &= ~UMod::Ctrl;
		stroke_mod &= ~UMod::Ctrl;
	}
	return (stroke.m_keyChar == m_keyChar &&
		stroke.m_keyCode == m_keyCode &&
		mod == stroke_mod &&
		stroke.m_onKeyRelease == m_onKeyRelease);
}


bool
UKeyStroke::operator!= (const UKeyStroke & stroke) const {
	return !(*this == stroke);
}

bool
UKeyStroke::operator<(const UKeyStroke & stroke) const {
	return (hashCode() < stroke.hashCode());
}


// Overrides UObject
std::string
UKeyStroke::toString() const {
	UOStringStream stream;
	if (m_keyChar == UKeyEvent::CHAR_UNDEFINED) {
		if (m_modifiers & UMod::Alt) {
			stream << "ALT+";
		}
		if (m_modifiers & UMod::Ctrl) {
			stream << "CTRL+";
		}
		if (m_modifiers & UMod::Shift) {
			stream << "SHIFT+";
		}
		if (m_modifiers & UMod::Meta) {
			stream << "META+";
		}
		stream << char(m_keyCode);
		// test if key code was printable
		std::string temp(stream.str());
		if (temp.length() && temp[temp.length() - 1] == ' ') {
			// if not printable, print the int value
			stream.str(temp.substr(0, temp.length() - 1));
			stream << uint32_t(m_keyCode);
		}
	} else {
		stream << char(m_keyChar);
	}
	return stream.str();
}


void
UKeyStroke::initFromString(const char * key) {
	UString str(UString(key).upperCase());
	std::vector<std::string> keys = str.tokenize('+');
	int modifiers = 0;

	for (std::vector<std::string>::const_iterator iter = keys.begin();
			iter != keys.end();
			++iter) {
		if ((*iter) == "ALT") {
			modifiers |= UMod::Alt;
		} else if ((*iter).length() == 1) {
			// this case is very often, so move to front
			// probably a character
			//if ((*iter)[0] < 256) {
				m_keyCode = UKeyCode_t((*iter)[0]);
			//}
		} else if ((*iter) == "CTRL") {
			modifiers |= UMod::Ctrl;
		} else if ((*iter) == "SHIFT") {
			modifiers |= UMod::Shift;
		} else if ((*iter) == "META") {
			modifiers |= UMod::Meta;
		} else if ((*iter) == "ESCAPE") {
			std::cerr << "got escape\n";
			m_keyCode = UKey::UK_ESCAPE;
		} else if ((*iter) == "RETURN") {
			m_keyCode = UKey::UK_RETURN;
		}  else if ((*iter) == "KP_ENTER") {
			m_keyCode = UKey::UK_KP_ENTER;
		} else if ((*iter).length() == 2 && (*iter)[0] == 'F' &&
				(*iter)[1] >= '1' && (*iter)[1] <= '9') {
			// a function key?
			std::cerr << "got function key " << int(UKey::UK_F1 + (*iter)[1] - '1') << "\n";
			m_keyCode = UKeyCode_t(UKey::UK_F1 + (*iter)[1] - '1');
		} else if ((*iter).length() == 3 && (*iter)[0] == 'F' &&
				(*iter)[1] >= '1' && (*iter)[1] <= '2' &&
				(*iter)[2] >= '1' && (*iter)[2] <= '9') {
			// a function key?
			m_keyCode = UKeyCode_t(UKey::UK_F1 + ((*iter)[1] - '1') * 10 + (*iter)[2] - '1');
		}
	}
	m_modifiers = UMod_t(modifiers);
}

/*
bool
UKeyStroke::equals(const UObject * objA) const;

bool
UKeyStroke::equals(const UKeyStroke * strokeA) const;
*/
/*
UKeyStroke::CacheType UKeyStroke::m_cache;
UObject UKeyStroke::m_MemoryManager;

// FIXME !
// memory leak
UKeyStroke * UKeyStroke::m_checkStroke = new UKeyStroke(
	UKeyEvent::CHAR_UNDEFINED,
	UKey::UK_UNDEFINED, UMod::NoModifier,
	false);


UKeyStroke *
UKeyStroke::getKeyStroke(uint16_t keyCharA, UKeyCode_t keyCodeA,
		UMod_t modifiersA, bool onKeyReleaseA) {
	return getCachedKeyStroke(keyCharA, keyCodeA, modifiersA, onKeyReleaseA);
}

UKeyStroke *
UKeyStroke::getKeyStroke(uint16_t keyCharA, bool onKeyReleaseA) {
	return getCachedKeyStroke(keyCharA, UKey::UK_UNDEFINED, UMod::NoModifier,
		onKeyReleaseA);
}

UKeyStroke *
UKeyStroke::getKeyStroke(UKeyCode_t keyCodeA, UMod_t modifiersA,
	bool onKeyReleaseA) {
	return getCachedKeyStroke(UKeyEvent::CHAR_UNDEFINED,
		UKey::UK_UNDEFINED, modifiersA, onKeyReleaseA);
}

UKeyStroke *
UKeyStroke::getKeyStroke(UKeyEvent * e) {
	if (e->getType() == UEvent::KeyTyped) {
		return getCachedKeyStroke(e->getKeyChar(),
			UKey::UK_UNDEFINED, UMod::NoModifier,
			false);
	} else {
		return getCachedKeyStroke(UKeyEvent::CHAR_UNDEFINED,
			e->getKeyCode(), e->getModifiers(),
			(e->getType() == UKeyEvent::KeyReleased));
	}
}
*/
bool
UKeyStroke::equals(const UObject * objA) const {
	if (const UKeyStroke * stroke = dynamic_cast<const UKeyStroke*>(objA)) {
		return (*this == *stroke);
	}
	return false;
}
bool
UKeyStroke::equals(const UKeyStroke * strokeA) const {
	return (*this == *strokeA);
}
/*
UKeyStroke::UKeyStroke(uint16_t keyCharA, UKeyCode_t keyCodeA, UMod_t modifiersA,
	 bool onKeyReleaseA) :
m_keyChar(keyCharA),
m_keyCode(keyCodeA),
m_modifiers(modifiersA),
m_onKeyRelease(onKeyReleaseA) {
}

UKeyStroke::~UKeyStroke() {}

UKeyStroke *
UKeyStroke::getCachedKeyStroke(uint16_t keyCharA, UKeyCode_t keyCodeA,
	UMod_t modifiersA, bool onKeyReleaseA) {

	m_checkStroke->m_keyChar = keyCharA;
	m_checkStroke->m_keyCode = keyCodeA;
	m_checkStroke->m_modifiers = modifiersA;
	m_checkStroke->m_onKeyRelease = onKeyReleaseA;

	CacheType::iterator iter = m_cache.find(m_checkStroke);

	if (iter != m_cache.end()) {
		return (*iter).second;
	} else {
		UKeyStroke * stroke = new UKeyStroke(keyCharA, keyCodeA,
			modifiersA, onKeyReleaseA);

		m_cache.insert(std::pair<UKeyStroke*, UKeyStroke*>(stroke, stroke));

		m_MemoryManager.addChild(stroke);

		return stroke;
	}
}
*/
