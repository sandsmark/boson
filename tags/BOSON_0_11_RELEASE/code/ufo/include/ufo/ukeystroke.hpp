/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ukeystroke.hpp
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

#ifndef UKEYSTROKE_HPP
#define UKEYSTROKE_HPP

#include "uobject.hpp"

#include "events/ukeysym.hpp"

namespace ufo {

class UKeyEvent;

/** @short A representation of a typed key on the keyboard. Used for hotkeys.
  * @ingroup events
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UKeyStroke : public UObject  {
	UFO_DECLARE_ABSTRACT_CLASS(UKeyStroke)
public:
	UKeyStroke();
	UKeyStroke(const char * key);
	UKeyStroke(const std::string & key);
	explicit UKeyStroke(wchar_t keyChar, bool onKeyRelease = false);
	explicit UKeyStroke(UKeyCode_t keyCode, UMod_t modifiers = UMod::NoModifier,
		bool onKeyRelease = false);
	explicit UKeyStroke(wchar_t keyChar, UKeyCode_t keyCode,
		UMod_t modifiers = UMod::NoModifier, bool onKeyRelease = false);
	UKeyStroke(UKeyEvent * e);

public:
	bool operator==(const UKeyStroke&) const;
	bool operator!= (const UKeyStroke&) const;
	/** For use in std::map */
	bool operator<(const UKeyStroke&) const;

	wchar_t getKeyChar() const { return m_keyChar; }
	UKeyCode_t getKeyCode() const { return m_keyCode; }
	UMod_t getModifiers() const { return m_modifiers; }
	bool onKeyRelease() const { return m_onKeyRelease; }

public: // Overrides UObject
	virtual std::string toString() const;

	virtual unsigned int hashCode() const {
		unsigned int ret = m_keyCode << 16;
		ret |= m_keyChar;
		ret += m_modifiers << 16;
		ret += (m_onKeyRelease) ? 1 : 0;
		return ret;
	}

	virtual bool equals(const UObject * objA) const;
	virtual bool equals(const UKeyStroke * strokeA) const;

private: // Private methods
	void initFromString(const char * key);

private: // Private attributes
	wchar_t m_keyChar;
	UKeyCode_t m_keyCode;
	UMod_t m_modifiers;
	bool m_onKeyRelease;
};

} // namespace ufo

#endif // UKEYSTROKE_HPP
