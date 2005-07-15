/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/events/ufocusevent.hpp
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

#ifndef UFOCUSEVENT_HPP
#define UFOCUSEVENT_HPP

#include "uwidgetevent.hpp"

namespace ufo {

/** @short This event is used for input focus changes.
  * @ingroup events
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UFocusEvent : public UWidgetEvent {
	UFO_DECLARE_DYNAMIC_CLASS(UFocusEvent)
public:
	/** Creates a new focus event. Type has to be either UEvent::FocusGained
	  * or UEvent::FocusLost.
	  */
	UFocusEvent(UWidget * sourceA, Type typeA) : UWidgetEvent(sourceA, typeA) {}

	bool focusGained() const { return m_type == UEvent::FocusGained; }
	bool focusLost() const { return m_type == UEvent::FocusLost; }

protected: // Protected methods
	virtual std::ostream & paramString(std::ostream & os) const;
};

} // namespace ufo

#endif // UFOCUSEVENT_HPP
