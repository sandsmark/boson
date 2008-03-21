/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/events/utimerevent.cpp
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

#include "ufo/events/utimerevent.hpp"

#include "ufo/utoolkit.hpp"

using namespace ufo;


UFO_IMPLEMENT_DYNAMIC_CLASS(UTimerEvent, UEvent)

UTimerEvent::UTimerEvent(uint32_t timeOut, const USlot0 & slot)
	: UEvent(this, UEvent::Timer)
	, m_timeOut(timeOut)
	, m_slot(slot)
	, m_startTime(0)
{}

uint32_t
UTimerEvent::getTimeOut() {
	return m_timeOut;
}

void
UTimerEvent::startTimer() {
	m_startTime = UToolkit::getToolkit()->getTicks();
}

bool
UTimerEvent::isReadyToRun() {
	if (m_startTime) {
		if ((UToolkit::getToolkit()->getTicks() - m_startTime) > m_timeOut) {
			return true;
		}
	}
	return false;
}

void
UTimerEvent::run() {
	m_slot();
}
