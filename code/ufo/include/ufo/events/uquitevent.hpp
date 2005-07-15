/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/events/uquitevent.hpp
    begin             : Sun Dec 22 2002
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

#ifndef UQUITEVENT_HPP
#define UQUITEVENT_HPP

#include "uevent.hpp"

namespace ufo {

/** @short This event is used to request the application to terminate.
  * @ingroup events
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UQuitEvent : public UEvent {
	UFO_DECLARE_DYNAMIC_CLASS(UQuitEvent)
public:
	/** Normal program exit requested. Error code 0.*/
	UQuitEvent();
	/** If there is an abnormal application abort requested, use error code
	  * as return an print reason.
	  */
	UQuitEvent(const std::string & reason, int errorCode);

	std::string getReason();
	int getErrorCode();

private: // Private attributes
	std::string m_reason;
	int m_errorCode;
};

//
// inline implementation
//

inline
UQuitEvent::UQuitEvent()
	: UEvent(this, UEvent::QuitEvent)
	, m_reason()
	, m_errorCode(0)
{}

inline
UQuitEvent::UQuitEvent(const std::string & reason, int errorCode)
	: UEvent(this, UEvent::QuitEvent)
	, m_reason(reason)
	, m_errorCode(errorCode)
{}

inline std::string
UQuitEvent::getReason() {
	return m_reason;
}

inline int
UQuitEvent::getErrorCode() {
	return m_errorCode;
}

}

#endif // UQUITEVENT_HPP
