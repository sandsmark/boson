/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/events/umouseevent.hpp
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

#ifndef UMOUSEEVENT_HPP
#define UMOUSEEVENT_HPP

#include "uinputevent.hpp"
#include "ukeysym.hpp"

#include "../util/upoint.hpp"

namespace ufo {

/** @short This event is used for mouse button and mouse move events.
  * @ingroup events
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UMouseEvent : public UInputEvent {
	UFO_DECLARE_DYNAMIC_CLASS(UMouseEvent)
public:
	/** Creates a new mouse event.
	  * @param sourceA The widge below the mouse cursor
	  * @param typeA The event type. Should be UEvent::MousePressed,
	  *          UEvent::MouseReleased, UEvent::MouseClicked,
	  *          UEvent::MouseMoved, UEvent::MouseDragged,
	  *          UEvent::MouseEntered, UEvent::MouseExited
	  * @param modifiersA All modifiers which were being pressed
	  *          immediately before the event was generated.
	  */
	UMouseEvent(UWidget * sourceA, Type typeA, UMod_t modifiersA,
		const UPoint & pos, UMod_t button, int clickCountA);

	/** Creates a new mouse event.
	  * @param sourceA The widge below the mouse cursor
	  * @param typeA The event type. Should be UEvent::MousePressed,
	  *          UEvent::MouseReleased, UEvent::MouseClicked,
	  *          UEvent::MouseMoved, UEvent::MouseDragged,
	  *          UEvent::MouseEntered, UEvent::MouseExited
	  * @param modifiersA All modifiers which were being pressed
	  *          immediately before the event was generated.
	  * @param relMove
	  * 	The relative movement to the former mouse position. Strictly
	  * 	spoken is this a vector.
	  */
	UMouseEvent(UWidget * sourceA, Type typeA, UMod_t modifiersA,
		const UPoint & pos, const UPoint & relMove, UMod_t button, int clickCountA);

	/** Creates a new mouse event.
	  * @param sourceA The widge below the mouse cursor
	  * @param typeA The event type. Should be UEvent::MousePressed,
	  *          UEvent::MouseReleased, UEvent::MouseClicked,
	  *          UEvent::MouseMoved, UEvent::MouseDragged,
	  *          UEvent::MouseEntered, UEvent::MouseExited
	  * @param modifiersA All modifiers which were being pressed
	  *          immediately before the event was generated.
	  * @param relMove
	  * 	The relative movement to the former mouse position. Strictly
	  * 	spoken is this a vector.
	  */
	UMouseEvent(UWidget * sourceA, Type typeA, UMod_t modifiersA,
		const UPoint & pos, const UPoint & relMove,
		const UPoint & root,
		UMod_t button, int clickCountA);

	/** returns how many times this mouse button was clicked within ?? ms */
	int getClickCount() const;

	/** Returns the mouse button which was changed.
	  * Returned value is a modifier type and one of the values of:
	  * @arg NoButton no button changed
	  * @arg LeftButton left mouse button changed
	  * @arg MiddleButton middle mouse button changed
	  * @arg RightButton right mouse button changed
	  * @arg MouseButton{1-5} mouse button 1..5 changed
	  */
	UMod_t getButton() const;

	/** @return The mouse location relative to the source widget. */
	const UPoint & getLocation() const;
	/** @return The mouse location relative to the UFO context. */
	const UPoint & getRootLocation() const;
	/** @return The relative mouse movement. */
	const UPoint & getRelMovement() const;

	/** X value relative to the source widget. */
	int getX() const;
	/** Y value relative to the source widget. */
	int getY() const;

	/** The mouse movement in x direction. */
	int getXRel() const;
	/** The mouse movement in y direction. */
	int getYRel() const;

	/** X value relative to the top most root pane. */
	int getRootX() const;
	/** Y value relative to the top most root pane. */
	int getRootY() const;

	/** translates the origin of the event */
	void translate(const UPoint & pos);

protected: // Protected methods
	virtual std::ostream & paramString(std::ostream & os) const;

private:  // Private attributes
	UPoint m_pos;

	UPoint m_rel;

	UPoint m_root;

	UMod_t m_button;
	/** the number of clicks */
	int m_clickCount;
};

//
// inline implementation
//


inline int
UMouseEvent::getClickCount() const {
	return m_clickCount;
}

inline UMod_t
UMouseEvent::getButton() const {
	return m_button;
}


inline const UPoint &
UMouseEvent::getLocation() const {
	return m_pos;
}

inline const UPoint &
UMouseEvent::getRootLocation() const {
	return m_root;
}

inline const UPoint &
UMouseEvent::getRelMovement() const {
	return m_rel;
}

inline int
UMouseEvent::getX() const {
	return m_pos.x;
}

inline int
UMouseEvent::getY() const {
	return m_pos.y;
}

inline int
UMouseEvent::getXRel() const {
	return m_rel.x;
}

inline int
UMouseEvent::getYRel() const {
	return m_rel.y;
}

inline int
UMouseEvent::getRootX() const {
	return m_root.x;
}

inline int
UMouseEvent::getRootY() const {
	return m_root.y;
}

inline void
UMouseEvent::translate(const UPoint & pos) {
	m_pos.translate(pos);
	m_root.translate(pos);
}

} // namespace ufo

#endif // UMOUSEEVENT_HPP
