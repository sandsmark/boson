/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/events/ukeyevent.hpp
    begin             : Wed May 9 2001
    $Id$
 ***************************************************************************/

/***************************************************************************
 * This library is free software; you can redistribute it and/or     *
 * modify it under the terms of the GNU Lesser General        *
 * License as published by the Free Software Foundation; either      *
 * version 2.1 of the License; or (at your option) any later version.   *
 *                                     *
 * This library is distributed in the hope that it will be useful;     *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU    *
 * Lesser General License for more details.             *
 *                                     *
 * You should have received a copy of the GNU Lesser General     *
 * License along with this library; if not; write to the Free Software   *
 * Foundation; Inc.; 59 Temple Place; Suite 330; Boston; MA 02111-1307 USA *
 ***************************************************************************/

#ifndef UKEYEVENT_HPP
#define UKEYEVENT_HPP

#include "uinputevent.hpp"
#include "ukeysym.hpp"

namespace ufo {

/**
  * A key event is used for indicating two types of events:
  *<p>
  * 1. KEY_PRESSED / KEY_RELEASED Events: Generated whenever a
  * key is pressed or release. Only with these events you can get
  * information about keys which do not generate character output.
  * </p><p>
  * 2. KEY_TYPED Events: Generated whenever a Unicode character
  * is entered. It can be represented by a single key press (e.g. 'u'), but
  * most of the it represents multiple key presses (like shift + 'u' which
  * results in a Unicode character 'U'). Some Keys can�t create Unicode
  * character (like modifiers, action keys, ..) and don�t create KEY_TYPED
  * events.
  * </p>
  * The key code (returned by getKeyCode()) of KEY_TYPED events return
  * always VK_UNDEFINED. The getKeyChar() returns a valid Unicode character
  * or CHAR_UNDEFINED.
  * <p>
  * <strong>NOTE:</strong><br>
  * Do not rely on the UKeyCode_t values of the virtual key constants (VK_*).
  * These values may change in the future to support a wider range of
  * keyboards.
  * </p>
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UKeyEvent : public UInputEvent {
	UFO_DECLARE_DYNAMIC_CLASS(UKeyEvent)
public:
	UKeyEvent(UWidget * sourceA, Type typeA, UMod_t modifiersA,
		UKeyCode_t keyCodeA, uint16_t keyCharA = 0);

	/** Returns the virtual key code for this key event.
	  * If this is a KEY_TYPED key event, this value will be always
	  * VK_UNDEFINED, else it should contain a valid key code.
	  */
	UKeyCode_t getKeyCode();
	/** Returns the Unicode character for this key event.
	  * If this is a KEY_PRESSED or KEY_RELEASED key event,
	  * the value will be likely a CHAR_UNDEFINED ( at least for
	  * action keys, modifiers, ..)
	  */
	uint16_t getKeyChar();

protected: // Protected methods
	virtual std::ostream & paramString(std::ostream & os) const;

protected:  // Protected attributes
	UKeyCode_t m_keyCode;
	uint16_t m_keyChar;

public:  // Public attributes
	/** KEY_PRESSED and KEY_RELEASED events
	   * which do not map to a valid Unicode character
	   * use this for the keyChar value.
	  */
	static const uint16_t CHAR_UNDEFINED;
};

} // namespace ufo

#endif // UKEYEVENT_HPP
