/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef UACTIONEVENT_HPP
#define UACTIONEVENT_HPP

#include "uevent.hpp"
// for modifiers
#include "uinputevent.hpp"

namespace ufo {

/**
  *@author Johannes Schmidt
  */

class UFO_EXPORT UActionEvent : public UEvent {
	UFO_DECLARE_DYNAMIC_CLASS(UActionEvent)
public:
	UActionEvent(UObject * sourceA, Type typeA, UMod_t modifiersA,
			const std::string & actionCommandA)
		: UEvent(sourceA, typeA)
		, m_modifiers(modifiersA)
		, m_actionCommand(actionCommandA) {}

	UMod_t getModifiers() {
		return m_modifiers;
	}

	const std::string & getActionCommand() {
		return m_actionCommand;
	}

protected: // Protected methods
	virtual std::ostream & paramString(std::ostream & os) const;

private:  // Protected attributes
	UMod_t m_modifiers;
	/**  */
	std::string m_actionCommand;
};

} // namespace ufo

#endif // UACTIONEVENT_HPP
