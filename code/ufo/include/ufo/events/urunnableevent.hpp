/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/events/urunnableevent.hpp
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

#ifndef URUNNABLEEVENT_HPP
#define URUNNABLEEVENT_HPP

#include "uevent.hpp"

namespace ufo {

/** @short This event represents an event which run method is called at
  *  processing.
  * @ingroup events
  *
  * If posted to a display, the run method will be executed
  * as soon as all events prior are dispatched.
  *
  * @see USlotEvent
  * @author Johannes Schmidt
  */
class UFO_EXPORT URunnableEvent : public UEvent {
	UFO_DECLARE_DYNAMIC_CLASS(URunnableEvent)
public:
	URunnableEvent(UObject * source, Type type) : UEvent(source, type) {}

	virtual void run() {}
};

} // namespace ufo

#endif // URUNNABLEEVENT_HPP
