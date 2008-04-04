/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/uabstractdisplay.cpp
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ***************************************************************************/

#include "ufo/uabstractdisplay.hpp"

#include "ufo/ucontext.hpp"
#include "ufo/utoolkit.hpp"
#include "ufo/urepaintmanager.hpp"
#include "ufo/ugraphics.hpp"
#include "ufo/events/ukeyevent.hpp"
#include "ufo/events/umouseevent.hpp"
#include "ufo/events/urunnableevent.hpp"
#include "ufo/events/uwidgetevent.hpp"
#include "ufo/events/utimerevent.hpp"

#include "ufo/widgets/uwidget.hpp"
#include "ufo/widgets/urootpane.hpp"

#include "ufo/gl/ugl_image.hpp"
#include "ufo/image/uimageio.hpp"
#include "ufo/uvolatiledata.hpp"

#include <bitset>

namespace ufo {

// FIXME: this should be in a file udisplay.cpp
UDisplay * UDisplay::m_default = NULL;


UFO_IMPLEMENT_ABSTRACT_CLASS(UAbstractDisplay, UObject)

UAbstractDisplay::UAbstractDisplay()
	: m_queues()
	, m_timerQueue()
	, m_keyModState(UMod::NoModifier)
	, m_mouseModState(UMod::NoButton)
	, m_mouseLocation()
	, m_currentEvent(NULL)
{}

UAbstractDisplay::~UAbstractDisplay() {
	if (this == getDefault()) {
		setDefault(NULL);
	}
}


UImage *
UAbstractDisplay::createImage(const std::string fileName) {
	return new UGL_Image(fileName);
}

UImage *
UAbstractDisplay::createImage(UImageIO * io) {
	return new UGL_Image(io);
}

void
UAbstractDisplay::addVolatileData(UVolatileData * vdata) {
	if (std::find(m_volatileData.begin(), m_volatileData.end(), vdata) ==
			m_volatileData.end()) {
		m_volatileData.push_back(vdata);
	}
}

void
UAbstractDisplay::removeVolatileData(UVolatileData * vdata) {
	std::list<UVolatileData*>::iterator iter =
		std::find(m_volatileData.begin(), m_volatileData.end(), vdata);

	if (iter != m_volatileData.end()) {
		m_volatileData.erase(iter);
	}
}

bool
UAbstractDisplay::dispatchEvents(unsigned int nevents) {
	// check if a timer event is ready to run
	checkTimerEvents();
	for (int i = NUM_PRIORITIES - 1; i >= 0 && nevents; --i) {
		while (m_queues[i].size() && nevents) {
			checkTimerEvents();

			UEvent * e = (*(m_queues[i].begin()));
			m_queues[i].pop_front();

			if (privateDispatchEvent(e)) {
				// we got a quit event
				// push back and return immediately
				m_queues[i].push_front(e);
				//e->unreference();
				return true;
			}

			e->unreference();
			nevents--;
		}
	}
	return false;
}

bool
UAbstractDisplay::dispatchEvents() {
	return dispatchEvents(getEventCount()); // -1);
}

void
UAbstractDisplay::pushEvent(UEvent * e) {
	if (e) {
		e->reference();

		preprocessEvent(e);

		// choose event queue
		switch(e->getType()) {
			case UEvent::QuitEvent:
				m_queues[2].push_back(e);
			break;
			case UEvent::Refresh:
				m_queues[2].push_back(e);
			break;
			/*case UEvent::Repaint:
				m_queues[0].push_back(e);
			break;*/
			case UEvent::Timer:
				if (UTimerEvent * te = dynamic_cast<UTimerEvent*>(e)) {
					m_timerQueue.push_back(te);
				}
			break;
			default:
				m_queues[1].push_back(e);
			break;
		}
	}
}

bool
UAbstractDisplay::dispatchEvent(UEvent * e) {
	e->reference();
	preprocessEvent(e);
	bool ret = privateDispatchEvent(e);
	e->unreference();
	return ret;
}

UEvent *
UAbstractDisplay::getCurrentEvent() const {
	return m_currentEvent;
}

int
UAbstractDisplay::getEventCount() const {
	int ret = 0;
	for (int i = NUM_PRIORITIES - 1; i >= 0 ; --i) {
		ret += m_queues[i].size();
	}
	ret += m_timerQueue.size();
	return ret;
}

UEvent *
UAbstractDisplay::peekEvent() const {
	for (int i = NUM_PRIORITIES - 1; i >= 0; --i) {
		if (m_queues[i].size()) {
			return *(m_queues[i].begin());
		}
	}

	return NULL;
}

UEvent *
UAbstractDisplay::peekEvent(UEvent::Type type) const {
	for (int i = NUM_PRIORITIES - 1; i >= 0; --i) {
		for (std::list<UEvent*>::const_iterator iter = m_queues[i].begin();
				iter != m_queues[i].end();
				++iter) {
			if ((*iter)->getType() == type) {
				return (*iter);
			}
		}
	}

	return NULL;
}

UEvent *
UAbstractDisplay::pollEvent() {
	UEvent * e = (*(m_queues[0].begin()));
	m_queues[0].pop_front();
	return e;
}


void
UAbstractDisplay::setModState(UMod_t modifiers) {
	m_keyModState = UMod_t(modifiers & UMod::MouseModifierMask);
	m_keyModState = UMod_t(modifiers & UMod::KeyboardModifierMask);
}

UMod_t
UAbstractDisplay::getModState() const {
	return UMod_t(m_keyModState | m_mouseModState);
}

UMod_t
UAbstractDisplay::getMouseState(int * x, int * y) const {
	if (x) {
		*x = m_mouseLocation.x;
	}
	if (y) {
		*y = m_mouseLocation.y;
	}

	return m_mouseModState;
}


//
// protected methods
//

void
UAbstractDisplay::checkTimerEvents() {
	// msvc6 woes
	std::list<UTimerEvent*>::iterator iter = m_timerQueue.begin();
	while (iter != m_timerQueue.end()) {
		if ((*iter)->isReadyToRun()) {
			m_currentEvent = (*iter);
			(*iter)->run();
			m_currentEvent = NULL;
			(*iter)->unreference();
			iter = m_timerQueue.erase(iter);
		} else {
			++iter;
		}
	}
}

//
// private methods
//


bool
UAbstractDisplay::privateDispatchEvent(UEvent * e) {
	m_currentEvent = e;
	switch (e->getType()) {
		case UEvent::QuitEvent:
			m_currentEvent = NULL;
			return true;
		break;
		case UEvent::Refresh: {
			// refresh volatile data and remove unused data
			std::list<UVolatileData*>::iterator iter = m_volatileData.begin();
			std::list<UVolatileData*>::iterator next_iter;
			while (iter != m_volatileData.end()) {
				next_iter = ++iter;
				(*iter)->refresh();
				iter = next_iter;
			}
			// refresh contexts
			std::vector<UContext*> contexts = getContexts();
			for(std::vector<UContext*>::iterator citer = contexts.begin();
					citer != contexts.end();
					++citer) {
				(*citer)->refresh();
			}
		}
		break;
		case UEvent::RunnableEvent:
			if (URunnableEvent * re = dynamic_cast<URunnableEvent*>(e)) {
				re->run();
			}
		break;
		default:
		{
			if (UWidgetEvent * we = dynamic_cast<UWidgetEvent*>(e)) {
				UWidget * w = we->getWidget();
				if (w) {
					UContext * context = w->getContext();
					if (context) {
						context->dispatchEvent(we);
					}
				}
			}
		}
		break;
	}
	m_currentEvent = NULL;
	return false;
}

void
UAbstractDisplay::preprocessEvent(UEvent * e) {
	// pre process events
	switch(e->getType()) {
		case UEvent::KeyPressed:
		case UEvent::KeyReleased:
			// change mod state if necessarry
			if (UKeyEvent * ke = dynamic_cast<UKeyEvent*>(e)) {
				processKeyModChange(ke);
			}
		break;
		case UEvent::MousePressed:
		case UEvent::MouseReleased:
			if (UMouseEvent * ke = dynamic_cast<UMouseEvent*>(e)) {
				processMouseModChange(ke);
			}
		break;
		case UEvent::MouseMoved:
		case UEvent::MouseDragged:
			if (UMouseEvent * ke = dynamic_cast<UMouseEvent*>(e)) {
				m_mouseLocation = ke->getLocation();
			}
		break;
		case UEvent::Timer:
			if (UTimerEvent * ke = dynamic_cast<UTimerEvent*>(e)) {
				ke->startTimer();
			}
		break;
		default:
		break;
	}
}

void
UAbstractDisplay::processKeyModChange(UKeyEvent * keyEvent) {
	// Whoo, this is really error prone
	static std::bitset<16> modifier_keys;
	int modstate = m_keyModState;
	if (keyEvent->getType() == UEvent::KeyPressed) {
		switch (keyEvent->getKeyCode()) {
			case UKey::UK_NUMLOCK: modstate |= UMod::Num; break;
			case UKey::UK_CAPSLOCK: modstate |= UMod::Caps; break;
			case UKey::UK_SCROLLOCK: break;
			case UKey::UK_RSHIFT:
				modstate |= UMod::Shift;
				modifier_keys.set(UKey::UK_RSHIFT & 0xff);
			break;
			case UKey::UK_LSHIFT:
				modstate |= UMod::Shift;
				modifier_keys.set(UKey::UK_RSHIFT & 0xff);
			break;
			case UKey::UK_RCTRL:
				modstate |= UMod::Ctrl;
				modifier_keys.set(UKey::UK_RCTRL & 0xff);
			break;
			case UKey::UK_LCTRL:
				modstate |= UMod::Ctrl;
				modifier_keys.set(UKey::UK_LCTRL & 0xff);
			break;
			case UKey::UK_RALT:
				modstate |= UMod::Alt;
				modifier_keys.set(UKey::UK_RALT & 0xff);
			break;
			case UKey::UK_LALT:
				modstate |= UMod::Alt;
				modifier_keys.set(UKey::UK_LALT & 0xff);
			break;
			case UKey::UK_RMETA:
				modstate |= UMod::Meta;
				modifier_keys.set(UKey::UK_RMETA & 0xff);
			break;
			case UKey::UK_LMETA:
				modstate |= UMod::Meta;
				modifier_keys.set(UKey::UK_LMETA & 0xff);
			break;
			case UKey::UK_RSUPER: //right windows key
				modstate |= UMod::Super;
				modifier_keys.set(UKey::UK_RSUPER & 0xff);
			break;
			case UKey::UK_LSUPER:  //left windows key
				modstate |= UMod::Super;
				modifier_keys.set(UKey::UK_LSUPER & 0xff);
			break;
			case UKey::UK_ALT_GRAPH: modstate |= UMod::AltGraph; break;
			/** Multi-key compose key */
			case UKey::UK_COMPOSE: break;
			default: break;
		}
	} else if (keyEvent->getType() == UEvent::KeyReleased) {
		switch (keyEvent->getKeyCode()) {
			case UKey::UK_NUMLOCK: modstate &= ~UMod::Num; break;
			case UKey::UK_CAPSLOCK: modstate &= ~UMod::Caps; break;
			case UKey::UK_SCROLLOCK: break;
			case UKey::UK_RSHIFT:
				if (!modifier_keys[UKey::UK_LSHIFT & 0xff])
					modstate &= ~UMod::Shift;
				modifier_keys.reset(UKey::UK_RSHIFT & 0xff);
			break;
			case UKey::UK_LSHIFT:
				if (!modifier_keys[UKey::UK_RSHIFT & 0xff])
					modstate &= ~UMod::Shift;
				modifier_keys.reset(UKey::UK_RSHIFT & 0xff);
			break;
			case UKey::UK_RCTRL:
				if (!modifier_keys[UKey::UK_LCTRL & 0xff])
					modstate &= ~UMod::Ctrl;
				modifier_keys.reset(UKey::UK_RCTRL & 0xff);
			break;
			case UKey::UK_LCTRL:
				if (!modifier_keys[UKey::UK_RCTRL & 0xff])
					modstate &= ~UMod::Ctrl;
				modifier_keys.reset(UKey::UK_LCTRL & 0xff);
			break;
			case UKey::UK_RALT:
				if (!modifier_keys[UKey::UK_LALT & 0xff])
					modstate &= ~UMod::Alt;
				modifier_keys.reset(UKey::UK_RALT & 0xff);
			break;
			case UKey::UK_LALT:
				if (!modifier_keys[UKey::UK_RALT & 0xff])
					modstate &= ~UMod::Alt;
				modifier_keys.reset(UKey::UK_LALT & 0xff);
			break;
			case UKey::UK_RMETA:
				if (!modifier_keys[UKey::UK_LMETA & 0xff])
					modstate &= ~UMod::Meta;
				modifier_keys.reset(UKey::UK_RMETA & 0xff);
			break;
			case UKey::UK_LMETA:
				if (!modifier_keys[UKey::UK_RMETA & 0xff])
					modstate &= ~UMod::Meta;
				modifier_keys.reset(UKey::UK_LMETA & 0xff);
			break;
			case UKey::UK_RSUPER: //right windows key
				if (!modifier_keys[UKey::UK_LSUPER & 0xff])
					modstate &= ~UMod::Super;
				modifier_keys.reset(UKey::UK_RSUPER & 0xff);
			break;
			case UKey::UK_LSUPER:  //left windows key
				if (!modifier_keys[UKey::UK_RSUPER & 0xff])
					modstate &= ~UMod::Super;
				modifier_keys.reset(UKey::UK_LSUPER & 0xff);
			break;
			case UKey::UK_ALT_GRAPH: modstate &= ~UMod::AltGraph; break;
			/** Multi-key compose key */
			case UKey::UK_COMPOSE: break;
			default: break;
		}
	}
	m_keyModState = UMod_t(modstate);
}

void
UAbstractDisplay::processMouseModChange(UMouseEvent * mouseEvent) {
	if (mouseEvent->getType() == UEvent::MousePressed) {
		m_mouseModState = UMod_t(m_mouseModState | mouseEvent->getButton());
	} else if (mouseEvent->getType() == UEvent::MouseReleased) {
		m_mouseModState = UMod_t(m_mouseModState & ~mouseEvent->getButton());
	}
}

} // namespace ufo
