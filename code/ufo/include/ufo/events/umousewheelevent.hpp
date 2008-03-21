/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/events/umousewheelevent.hpp
    begin             : Sun Jul 21 2002
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

#ifndef UMOUSEWHEELEVENT_HPP
#define UMOUSEWHEELEVENT_HPP

#include "uinputevent.hpp"

#include "../util/upoint.hpp"

namespace ufo {

/** @short This event is used for mouse wheel events.
  * @ingroup events
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UMouseWheelEvent : public UInputEvent  {
	UFO_DECLARE_DYNAMIC_CLASS(UMouseWheelEvent)
public: // Public types
	enum {
		WHEEL_DATA = 120
	};
public:
	/** @param wheel
	  * 	The number of the mouse wheel (some mices have two mouse wheels).
	  * 	'0' is the first wheel.
	  * @param delta
	  * 	The amount that the mouse wheel was scrolled.
	  * 	If the mouse wheel was scrolled forwards/away from the user, the
	  * 	value is positive otherwise negative.
	  * 	Please Note:
	  * 	It is not the number of clicks but a multiple or a fraction
	  * 	of WHEEL_DELTA which was set to 120 by mouse vendors to allow the
	  * 	building of mouse wheels with a finer resolution.
	  */
	UMouseWheelEvent(
		UWidget * source,
		Type type,
		UMod_t modifiersA,
		const UPoint & pos,
		const UPoint & root,
		int delta,
		int wheel);

	UMouseWheelEvent(
		UWidget * source,
		Type type,
		UMod_t modifiersA,
		const UPoint & pos,
		int delta,
		int wheel);

	//
	// similar to mouse events
	//

	const UPoint & getLocation() const;
	const UPoint & getRootLocation() const;

	/** X value relative to the source widget. */
	int getX() const;
	/** Y value relative to the source widget. */
	int getY() const;

	/** X value relative to the top most root pane. */
	int getRootX() const;
	/** Y value relative to the top most root pane. */
	int getRootY() const;

	/** translates the origin of the event */
	void translate(const UPoint & pos);

	//
	// new methods
	//

	int getWheel() const;

	/** The amount that the mouse wheel was scrolled as multiple or fraction
	  * of WHEEL_DATA
	  */
	int getDelta() const;

	/** This method is only for convenience and returns the numbers of
	  * "clicks" of the rotation with current "low-resolution" mouse wheels.
	  * A negative amount of clicks means a rotation towards the user, positive
	  * values are rotations away from the user.
	  * It is equal to getScrollAmount() / UMouseWheelEvent::WHEEL_DATA.
	  */
	int getWheelRotation() const;

protected: // Protected methods
	virtual std::ostream & paramString(std::ostream & os) const;

private: // Private attributes
	UPoint m_pos;
	UPoint m_root;

	int m_delta;
	int m_wheel;
};


//
// inline implementation
//


inline
UMouseWheelEvent::UMouseWheelEvent(
		UWidget * source,
		Type type,
		UMod_t modifiersA,
		const UPoint & pos,
		const UPoint & root,
		int delta,
		int wheel)
	: UInputEvent(source, type, modifiersA)
	, m_pos(pos)
	, m_root(root)
	, m_delta(delta)
	, m_wheel(wheel) {}

inline
UMouseWheelEvent::UMouseWheelEvent(
		UWidget * source,
		Type type,
		UMod_t modifiersA,
		const UPoint & pos,
		int delta,
		int wheel)
	: UInputEvent(source, type, modifiersA)
	, m_pos(pos)
	, m_root(pos)
	, m_delta(delta)
	, m_wheel(wheel) {}


inline const UPoint &
UMouseWheelEvent::getLocation() const {
	return m_pos;
}

inline const UPoint &
UMouseWheelEvent::getRootLocation() const {
	return m_root;
}

inline int
UMouseWheelEvent::getX() const {
	return m_pos.x;
}

inline int
UMouseWheelEvent::getY() const {
	return m_pos.y;
}

inline int
UMouseWheelEvent::getRootX() const {
	return m_root.x;
}

inline int
UMouseWheelEvent::getRootY() const {
	return m_root.y;
}

inline void
UMouseWheelEvent::translate(const UPoint & pos) {
	m_pos.translate(pos);
	m_root.translate(pos);
}

inline int
UMouseWheelEvent::getWheel() const {
	return m_wheel;
}
inline int
UMouseWheelEvent::getDelta() const {
	return m_delta;
}

inline int
UMouseWheelEvent::getWheelRotation() const {
	return m_delta / WHEEL_DATA;
}

} // namespace ufo

#endif // UMOUSEWHEELEVENT_HPP
