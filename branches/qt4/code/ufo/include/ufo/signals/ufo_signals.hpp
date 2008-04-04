/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/signals/ufo_signals.hpp
    begin             : Tue Jul 2 2002
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

#ifndef UFO_SIGNALS_HPP
#define UFO_SIGNALS_HPP

// The UFO signal system is a minimal signal and slot system to
// connect signal with abstract function slots.

// A slot can be a static function or an object method

// So far this is very similar to the libsigc++ signal system
// (http://libsigcpp.sf.net)

// The main difference is that UFO signals don't know connections.
// Instead you disconnect a slot from a signal by implicitly calling
// signal.disconnect(slot) where slot is equal (via equals(SlotBase*))
// to the previously connected slot.

// thanks to the libsigc++ people at libsigc.sf.net

// WARNING:
// The UFO signal system is not meant to be stable and
// is definitly _not_ usable for common signal and slot communication

#include "uslot.hpp"
#include "uobjectslot.hpp"
#include "ufunctionslot.hpp"

#include "usignal.hpp"

namespace ufo {
// common action slot specialization
class UActionEvent;
typedef USlot1<UActionEvent*> UActionSlot;

} // namespace ufo

#endif // UFO_SIGNALS_HPP
