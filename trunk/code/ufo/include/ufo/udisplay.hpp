/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/udisplay.hpp
    begin             : Sat Nov 23 2002
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

#ifndef UDISPLAY_HPP
#define UDISPLAY_HPP

#include "uobject.hpp"
#include "events/uevent.hpp"
#include "events/ukeysym.hpp"

#include <vector>

namespace ufo {

class UContext;
class UImage;
class UImageIO;
class UVolatileData;

/** @short The display represents the connection to the underlying system and
  *  contains the UFO event queue.
  * @ingroup native
  * @ingroup events
  *
  * The display object is responsible for creating the video driver
  * (depending on the arguments given to the constructor) and to handle
  * the system events send to the created video devices.
  * <p>
  * Usually there exists only one implementation of a Display.
  * <p>
  * All native ressources are loaded by the display object. For example
  * OpenGL textures and video devices.
  *
  * @see UVideoDriver
  * @see UVideoDevice
  * @see UImage
  * @author Johannes Schmidt
  */
class UFO_EXPORT UDisplay : public UObject {
	UFO_DECLARE_ABSTRACT_CLASS(UDisplay)
public: // Public methods
	/** Returns a vector with all contextes which were registered at this
	  * display object.
	  */
	virtual std::vector<UContext*> getContexts() const = 0;

	virtual UImage * createImage(const std::string fileName) = 0;
	virtual UImage * createImage(UImageIO * io) = 0;

	virtual void addVolatileData(UVolatileData * vdata) = 0;
	virtual void removeVolatileData(UVolatileData * vdata) = 0;
public: // event methods
	/** Pumps system events to the event queue.
	  * May be a no-op on some systems. */
	virtual void pumpEvents() = 0;

	/** Adds e to the event queue. Updates the internal mouse and key states.
	  */
	virtual void pushEvent(UEvent * e) = 0;

	/** Dispatches immediately the given event without pushing it to
	  * the event queue. Updates the internal mouse and key states.
	  * @param e The event which should be dispatched.
	  * @return True if a quit event was processed, otherwise false.
	  */
	virtual bool dispatchEvent(UEvent * e) = 0;

	/** Dispatches nevents events to the receiver contexts.
	  * If the current event is a quit event, it returns immediately true
	  * without dispatching more events.
	  * @return True if a quit event was processed, otherwise false.
	  */
	virtual bool dispatchEvents(unsigned int nevents) = 0;

	/** Dispatches all pending events of the event queues.
	  * @return True if a quit event was processed, otherwise false.
	  */
	virtual bool dispatchEvents() = 0;

	/** Returns the event which is currently dispatched by this display
	  * object. Returns NULL if no event is currently dispatched.
	  */
	virtual UEvent * getCurrentEvent() const = 0;

	/** Returns the number of pending events. */
	virtual int getEventCount() const = 0;


	/** Returns the first event on the event queue without removeing it. */
	virtual UEvent * peekEvent() const = 0;
	/** Returns the first event on the event queue of the given type
	  * without removeing it. */
	virtual UEvent * peekEvent(UEvent::Type type) const = 0;

	/** Returns the first event on the event queue and
	  * removes it from the event queue.
	  * The user takes control over this event and is responsible for
	  * unreferencing and deallocating.
	  */
	virtual UEvent * pollEvent() = 0;

public:
	/** Returns the current state of the modifier keys (like CRTL, ALT, ..).
	  * Please note that the current mod state may differ from the current
	  * event you are processing.
	  * The current mod state is determinded by the last pushed event.
	  *
	  * @see UInputEvent
	  */
	virtual UMod_t getModState() const = 0;

	/** Returns the current button state as modifier mask.
	  * x and y are set to the current mouse position. Both may
	  * be NULL.
	  * The current mouse state is determinded by the last pushed event.
	  *
	  * @param x filled with the x position of the mouse
	  * @param y filled with the y position of the mouse
	  * @see UInputEvent
	  */
	virtual UMod_t getMouseState(int * x, int * y) const = 0;

public: // Public static methods
	/** Returns the default display. */
	static UDisplay * getDefault();

protected: // Protected methods
	/** Should be used by backend implementation to set a default display. */
	static void setDefault(UDisplay * defaultDisplay);

private: // Private attributes
	static UDisplay * m_default;
};

//
// inline implementation
//

inline UDisplay *
UDisplay::getDefault() {
	return m_default;
}

inline void
UDisplay::setDefault(UDisplay * defaultDisplay) {
	m_default = defaultDisplay;
}

} // namespace ufo

#endif // UDISPLAY_HPP
