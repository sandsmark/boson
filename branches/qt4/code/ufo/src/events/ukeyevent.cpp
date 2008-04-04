/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/events/ukeyevent.cpp
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#include "ufo/events/ukeyevent.hpp"

namespace ufo {

UFO_IMPLEMENT_ABSTRACT_CLASS(UKeyEvent, UInputEvent)

const uint16_t UKeyEvent::CHAR_UNDEFINED = 0;

UKeyEvent::UKeyEvent(UWidget * sourceA, Type idA, UMod_t modifiersA,
		UKeyCode_t keyCodeA, uint16_t keyCharA)
	: UInputEvent(sourceA, idA, modifiersA)
	, m_keyCode(keyCodeA)
	, m_keyChar(keyCharA) {}


UKeyCode_t
UKeyEvent::getKeyCode() {
	return m_keyCode;
}
/** No descriptions */
/*std::string UKeyEvent::getKeyName() {
	return SDL_GetKeyName( m_key );
}
*/


uint16_t
UKeyEvent::getKeyChar() {
	return m_keyChar;
}

//
// protected
//
std::ostream &
UKeyEvent::paramString(std::ostream & os) const {
	UInputEvent::paramString(os);
	os << ";key code " << m_keyCode << ";key char " << m_keyChar;
	return os;
}

} // namespace ufo
