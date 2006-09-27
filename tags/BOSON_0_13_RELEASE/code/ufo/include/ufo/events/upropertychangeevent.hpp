/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/events/upropertychangeevent.hpp
    begin             : Tue May 8 2001
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

#ifndef UPROPERTYCHANGEEVENT_HPP
#define UPROPERTYCHANGEEVENT_HPP

#include "uevent.hpp"

namespace ufo {

/** @short This event is used to notify property changes.
  * @ingroup events
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UPropertyChangeEvent : public UEvent {
	UFO_DECLARE_DYNAMIC_CLASS(UPropertyChangeEvent)
public:
	UPropertyChangeEvent(UObject * source, Type type,
			const std::string & propName,
			UObject * oldValue, UObject * newValue)
		: UEvent(source, type)
		, m_propName(propName)
		, m_oldValue(oldValue)
		, m_newValue(newValue) {}

	/** returns the name of the canged property */
	const std::string & getPropertyName() const {
		return m_propName;
	}

	/** returns the old Value */
	UObject * getOldValue() const {
		return m_oldValue;
	}

	/** returns the new value */
	UObject * getNewValue() const {
		return m_newValue;
	}

protected: // Protected methods
	virtual std::ostream & paramString(std::ostream & os) const;

protected:  // Protected attributes
	std::string m_propName;

	UObject * m_oldValue;
	UObject * m_newValue;
};

} // namespace ufo

#endif // UPROPERTYCHANGEEVENT_HPP
