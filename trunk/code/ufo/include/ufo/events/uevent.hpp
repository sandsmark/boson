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

/** This is the base class for all UFO events
  * @author Johannes Schmidt
  */

class UFO_EXPORT UEvent : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UEvent)
public:
	enum Type {
		NoEvent = 0,
		Timer = 1, // timer event
		Action = 2, // action event
		RunnableEvent = 3,
		QuitEvent = 4, // Quit event
		Repaint = 5,

		MousePressed = 10, // mouse event
		MouseReleased = 11,
		MouseClicked = 12,
		MouseMoved = 13,
		MouseDragged = 14,
		MouseEntered = 15,
		MouseExited = 16,

		MouseWheel = 19,

		KeyPressed = 20, // key event
		KeyReleased = 21,
		KeyTyped = 22,

		FocusGained = 25, // focus event
		FocusLost = 26,

		WidgetMoved = 30, // move event
		WidgetResized = 31,

		WidgetShown = 32,
		WidgetHidden = 33,

		PropertyChanged = 40 // property change event
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
	  * @return
	  * 	true if this event is consumed
	  */
	virtual bool isConsumed() const;

protected: // Protected methods
	virtual std::ostream & paramString(std::ostream & os) const;

protected:  // Protected attributes
	/** the object that emitted the event */
	UObject * m_source;
	/** an int describing the event */
	Type m_type;
	/** the consume property
	  * @see #consume
	  */
	bool m_isConsumed;
};

} // namespace ufo

#endif // UEVENT_HPP
