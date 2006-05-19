/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/events/umouseevent.cpp
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

#include "ufo/events/umouseevent.hpp"

namespace ufo {

UFO_IMPLEMENT_ABSTRACT_CLASS(UMouseEvent, UInputEvent)

UMouseEvent::UMouseEvent(UWidget * sourceA, Type typeA, UMod_t modifiersA,
		const UPoint & pos, UMod_t button, int clickCountA)
	: UInputEvent(sourceA, typeA, modifiersA)
	, m_pos(pos)
	, m_rel()
	, m_root(pos)
	, m_button(button)
	, m_clickCount(clickCountA) {}

UMouseEvent::UMouseEvent(UWidget * sourceA, Type typeA, UMod_t modifiersA,
		const UPoint & pos, const UPoint & relMove, UMod_t button, int clickCountA)
	: UInputEvent(sourceA, typeA, modifiersA)
	, m_pos(pos)
	, m_rel(relMove)
	, m_root(pos)
	, m_button(button)
	, m_clickCount(clickCountA) {}

UMouseEvent::UMouseEvent(UWidget * sourceA, Type typeA, UMod_t modifiersA,
		const UPoint & pos, const UPoint & relMove,
		const UPoint & root,
		UMod_t button, int clickCountA)
	: UInputEvent(sourceA, typeA, modifiersA)
	, m_pos(pos)
	, m_rel(relMove)
	, m_root(root)
	, m_button(button)
	, m_clickCount(clickCountA) {}

//*
//* protected
//*
std::ostream &
UMouseEvent::paramString(std::ostream & os) const {
	UInputEvent::paramString(os);
	os << ";pos: " << m_pos
	<< ";root pos: " << m_root
	<< ";relative " << m_rel
	<< ";button " << m_button
	<< ";click count " << m_clickCount;
	return os;
}

} // namespace ufo
