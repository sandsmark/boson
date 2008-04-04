/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/events/uinputevent.hpp
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

#ifndef UINPUTEVENT_HPP
#define UINPUTEVENT_HPP

#include "uwidgetevent.hpp"
#include "ukeysym.hpp"

namespace ufo {

/** @short This event is used as base class for user inputs
  *  (keyboard, mouse etc.).
  * @ingroup events
  *
  * @see UKeyEvent
  * @see UMouseEvent
  * @author Johannes Schmidt
  */

class UFO_EXPORT UInputEvent : public UWidgetEvent {
	UFO_DECLARE_DYNAMIC_CLASS(UInputEvent)
public:
	UInputEvent(UWidget * sourceA, Type typeA, UMod_t modifiersA);

	/** Returns the modifiers associated with this input event. */
	UMod_t getModifiers();

	bool isControlDown();
	bool isAltDown();
	bool isAltGraphDown();
	bool isShiftDown();
	bool isCapsDown();
	bool isMetaDown();
	bool isNumDown();

	/** Returns true if a mouse button was pressed when fireing this event.
	  */
	bool hasMouseModifiers();
	/** Returns true if a keyboard modifier was pressed
	  * when fireing this event.
	  */
	bool hasKeyboardModifiers();

protected: // Protected methods
	virtual std::ostream & paramString(std::ostream & os) const;

protected:  // Protected attributes
	/**  */
	UMod_t m_modifiers;
};

} // namespace ufo

#endif // UINPUTEVENT_HPP
