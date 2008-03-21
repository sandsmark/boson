/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/events/utimerevent.hpp
    begin             : Mon Sep 13 2004
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

#ifndef UTIMEREVENT_HPP
#define UTIMEREVENT_HPP

#include "uevent.hpp"

#include "../signals/uslot.hpp"

namespace ufo {

/** @short A very simplistic time out event.
  * @ingroup events
  *
  * If sent to the event queue, it is guaranteed that the call back
  * is not executed before the given time has elapsed.
  * It is executed in the event dispatch "thread".
  * But be careful: There is absolutely no guarantee when the call back
  * is executed.
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UTimerEvent : public UEvent {
	UFO_DECLARE_DYNAMIC_CLASS(UTimerEvent)
public:
	/** Constructor.
	  * @param slot The slot to be executed
	  * @param timeOut The amount of milli seconds which should at
	  *  least elapse before the slot is executed.
	  */
	UTimerEvent(uint32_t timeOut, const USlot0 & slot);

	uint32_t getTimeOut();
	/** Sets the start time used as reference time for the time out.
	  * This is automatically executed by the display object.
	  */
	void startTimer();
	/** Returns true if the time out has elapsed. */
	bool isReadyToRun();
	/** Executes the slot. */
	void run();
private: // Private attributes
	uint32_t m_timeOut;
	USlot0 m_slot;
	uint32_t m_startTime;
};

} // namespace ufo

#endif // UTIMEREVENT_HPP
