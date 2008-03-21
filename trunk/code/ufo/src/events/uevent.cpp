/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/events/uevent.cpp
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#include "ufo/events/uevent.hpp"

using namespace ufo;

UFO_IMPLEMENT_ABSTRACT_CLASS(UEvent, UObject)

UEvent::UEvent(UObject * sourceA, Type typeA)
	: m_source(sourceA)
	, m_type(typeA)
	, m_isConsumed(false) {}


UObject *
UEvent::getSource() const {
	return m_source;
}

void
UEvent::setSource(UObject * newSourceA) {
	m_source = newSourceA;
}


UEvent::Type
UEvent::getType() const {
	return m_type;
}

void
UEvent::consume() {
	m_isConsumed = true;
}

bool
UEvent::isConsumed() const {
	return m_isConsumed;
}

//
// protected
//
std::ostream &
UEvent::paramString(std::ostream & os) const {
	os << "source: " << m_source
	<< ";type " << m_type;

	if (isConsumed()) {
		os << ";is consumed";
	}
	return os;
}

// FIXME !

#include "ufo/events/uquitevent.hpp"
#include "ufo/events/urunnableevent.hpp"
#include "ufo/events/uslotevent.hpp"
#include "ufo/events/ushortcutevent.hpp"

UFO_IMPLEMENT_ABSTRACT_CLASS(UQuitEvent, UEvent)
UFO_IMPLEMENT_ABSTRACT_CLASS(URunnableEvent, UEvent)
UFO_IMPLEMENT_ABSTRACT_CLASS(USlotEvent, URunnableEvent)
UFO_IMPLEMENT_ABSTRACT_CLASS(UShortcutEvent, UWidgetEvent)
