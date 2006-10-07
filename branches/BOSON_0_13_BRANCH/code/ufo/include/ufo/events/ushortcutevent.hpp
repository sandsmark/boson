/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/events/ushortcutevent.hpp
    begin             : So Jun 12 2005
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

#ifndef USHORTCUTEVENT_HPP
#define USHORTCUTEVENT_HPP

#include "uwidgetevent.hpp"

#include "../ukeystroke.hpp"

namespace ufo {

/** @short This event represents a keyboard shortcut
  * @ingroup events
  * A shortcut event is fired when there is at least one visible,
  * focused widget.
  * The widget which has registered the shortcut gets this event via
  * processEvent and processShortcutEvent.
  * Only one widgets gets a shortcut event.
  * If more than one widget registered for the same shortcut, isAmbiguous()
  * returns true. Pressing the stroke again dispatches a newly created event
  * to the next listener.
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UShortcutEvent : public UWidgetEvent {
	UFO_DECLARE_CLASS(UShortcutEvent)
public:
	UShortcutEvent(UWidget * source, Type type, const UKeyStroke & stroke, bool ambiguous = false)
		: UWidgetEvent(source, type), m_stroke(stroke), m_isAmbiguous(ambiguous) {}

	/** @return The key stroke which activated this shortcut event. */
	const UKeyStroke & getKey() const { return m_stroke; }
	/** @return True if more than one widgets listens for this shortcut. */
	bool isAmbiguous() const { return m_isAmbiguous; }
private:
	UKeyStroke m_stroke;
	bool m_isAmbiguous;
};

} // namespace ufo

#endif // USHORTCUTEVENT_HPP
