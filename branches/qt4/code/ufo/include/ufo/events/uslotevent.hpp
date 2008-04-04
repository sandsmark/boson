/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/events/uslotevent.hpp
    begin             : Sat Nov 23 2002
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

#ifndef USLOTEVENT_HPP
#define USLOTEVENT_HPP

#include "urunnableevent.hpp"
#include "../signals/uslot.hpp"

namespace ufo {

/** @short This event can execute a slot (function or method) on dispachting.
  * @ingroup events
  * This class is provided for convenience. If pushed to the event queue,
  * the callback slot will be executed as soon as all prior events are
  * dispatched.
  * @see URunnableEvent
  * @author Johannes Schmidt
  */
class UFO_EXPORT USlotEvent : public URunnableEvent {
	UFO_DECLARE_DYNAMIC_CLASS(USlotEvent)
public:
	USlotEvent(const USlot0 & slot)
		: URunnableEvent(this, UEvent::RunnableEvent)
		, m_slot(slot) {}

	virtual void run() {
		m_slot();
	}
private: // Private attributes
	USlot0 m_slot;
};

} // namespace ufo

#endif // USLOTEVENT_HPP
