/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/events/uevent.hpp
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef UEVENT_HPP
#define UEVENT_HPP

#include "../uobject.hpp"

namespace ufo {

/** @short This is the base class for all UFO events
  * @ingroup events
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UEvent : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UEvent)
public:
	enum Type {
		NoEvent = 0,
		/** A timer event (for simple time outs). */
		Timer = 1,
		/** Generic action event. */
		Action = 2,
		/** An event whose run method is executed on processing. */
		RunnableEvent = 3,
		/** Request for application quit. */
		QuitEvent = 4,
		/** Repaint event. */
		Repaint = 5,
		/** Lost hardware surfaces, request for refresh. */
		Refresh = 6,

		MousePressed = 10, // mouse event
		MouseReleased = 11,
		MouseClicked = 12,
		MouseMoved = 13,
		MouseDragged = 14,
		/** Widget got mouse focus. */
		MouseEntered = 15,
		/** Widget lost mouse focus. */
		MouseExited = 16,

		MouseWheel = 19,

		KeyPressed = 20,
		KeyReleased = 21,
		/** A unicode character was pressed. */
		KeyTyped = 22,
		Shortcut = 23,

		/** Got input focus. */
		FocusGained = 25,
		/** Lost input focus. */
		FocusLost = 26,

		WidgetMoved = 30,
		WidgetResized = 31,

		WidgetShown = 32,
		WidgetHidden = 33,

		/** Widget added to a parent visible on screen. */
		WidgetAdded = 34,
		/** Widget about to be removed. */
		WidgetRemoved = 35,
		/** The z order of this widget has been changed. */
		WidgetZOrderChanged = 36,

		/** A Property has been changed. */
		PropertyChanged = 40
	};

public:
	UEvent(UObject * sourceA, Type idA);

	/** returns the emitting object */
	UObject * getSource() const;
	/** retargets this event */
	void setSource(UObject * newSourceA);

	/** returns the id desribing the event */
	Type getType() const;

	/** consumes the event.
	  * If an event is consumed, no other event listener can process it.
	  */
	virtual void consume();

	/** returns whether an event is consumed.
	  * @see #consume
	  * @return True if this event is consumed
	  */
	virtual bool isConsumed() const;

protected: // Protected methods
	virtual std::ostream & paramString(std::ostream & os) const;

protected:  // Protected attributes
	/** the object that emitted the event */
	UObject * m_source;
	/** an int describing the event */
	Type m_type;
	/** The consume property
	  * @see #consume
	  */
	bool m_isConsumed;
};

} // namespace ufo

#endif // UEVENT_HPP
