/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/events/uactionevent.hpp
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

#ifndef UACTIONEVENT_HPP
#define UACTIONEVENT_HPP

#include "uevent.hpp"
// for modifiers
#include "uinputevent.hpp"

namespace ufo {

/** @short This event is used for many synchronous messages
  *  which indicate an action like in button, check boxes etc.
  * @ingroup events
  *
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UActionEvent : public UEvent {
	UFO_DECLARE_DYNAMIC_CLASS(UActionEvent)
public:
	/** Creates a new action event.
	  * @param source The emitting object
	  * @param type The event type (should be @p UEvent::Action)
	  * @param modifiers Mouse and keyboard modifiers
	  * @param actionCommand A string describing this action
	  */
	UActionEvent(UObject * source, Type type, UMod_t modifiers,
			const std::string & actionCommand)
		: UEvent(source, type)
		, m_modifiers(modifiers)
		, m_actionCommand(actionCommand)
		, m_isRevoked(false)
	{}

	/** @return Mouse and keyboard modifiers which where pressed at the time
	  * this event was fired. */
	UMod_t getModifiers() {
		return m_modifiers;
	}

	/** @return A string describing this action
	  */
	const std::string & getActionCommand() {
		return m_actionCommand;
	}

	/** Tries to revoke this action. The library or application might
	  * revoke its action.
	  */
	void revoke() {
		m_isRevoked = true;
	}
	/** @return True if this event has been revoked.
	  */
	bool isRevoked() const {
		return m_isRevoked;
	}

protected: // Protected methods
	virtual std::ostream & paramString(std::ostream & os) const;

private:  // Protected attributes
	/** Mouse and keyboard modifiers. */
	UMod_t m_modifiers;
	/** A string describing this action */
	std::string m_actionCommand;
	/** True if this event was revoked by a listener. */
	bool m_isRevoked;
};

} // namespace ufo

#endif // UACTIONEVENT_HPP
