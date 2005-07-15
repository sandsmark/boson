/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/uabstractdisplay.hpp
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

#ifndef UABSTRACTDISPLAY_HPP
#define UABSTRACTDISPLAY_HPP

#include "udisplay.hpp"

#include <list>
#include "ufo/util/upoint.hpp"

namespace ufo {

class UTimerEvent;
class UKeyEvent;
class UMouseEvent;

/** @short Implements some platform independent methods of UDisplay.
  *  Provided for convenience.
  * @ingroup native
  * @ingroup internal
  *
  * This class is not part of the official UFO API and
  * may be changed without warning.
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UAbstractDisplay : public UDisplay {
	UFO_DECLARE_ABSTRACT_CLASS(UAbstractDisplay)
public:
	UAbstractDisplay();
	virtual ~UAbstractDisplay();

public: // Implements UDisplay
	virtual UImage * createImage(const std::string fileName);
	virtual UImage * createImage(UImageIO * io);
	virtual void addVolatileData(UVolatileData * vdata);
	virtual void removeVolatileData(UVolatileData * vdata);
	virtual void pushEvent(UEvent * e);
	virtual bool dispatchEvent(UEvent * e);
	virtual bool dispatchEvents(unsigned int nevents);
	virtual bool dispatchEvents();
	virtual UEvent * getCurrentEvent() const;

	virtual int getEventCount() const;


	virtual UEvent * peekEvent() const;
	virtual UEvent * peekEvent(UEvent::Type type) const;

	virtual UEvent * pollEvent();

public:
	virtual UMod_t getModState() const;
	virtual UMod_t getMouseState(int * x, int * y) const;

protected: //
	/** Manually sets the modifier state. */
	void setModState(UMod_t modifiers);
	void checkTimerEvents();

private: // Private methods
	bool privateDispatchEvent(UEvent * e);
	void preprocessEvent(UEvent * e);
	void processKeyModChange(UKeyEvent * keyEvent);
	void processMouseModChange(UMouseEvent * mouseEvent);

private: // Private attributes
	enum {
		NUM_PRIORITIES = 3
	};
	std::list<UEvent*> m_queues[NUM_PRIORITIES];
	std::list<UTimerEvent*> m_timerQueue;
	UMod_t m_keyModState;
	UMod_t m_mouseModState;
	UPoint m_mouseLocation;
	UEvent * m_currentEvent;
	std::list<UVolatileData*> m_volatileData;
};

} // namespace ufo

#endif // UABSTRACTDISPLAY_HPP
