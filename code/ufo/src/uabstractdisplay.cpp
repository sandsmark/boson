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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
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
				// we got a quit event, return immediately
				e->unreference();
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
	int keymod = modifiers;
	int mousemod = 0;

	// remove mouse modifiers
	if (keymod & UMod::Button1) {
		mousemod |= UMod::Button1;
		keymod &= ~UMod::Button1;
	}
	if (keymod & UMod::Button2) {
		mousemod |= UMod::Button2;
		keymod &= ~UMod::Button2;
	}
	if (keymod & UMod::Button3) {
		mousemod |= UMod::Button3;
		keymod &= ~UMod::Button3;
	}
	if (keymod & UMod::Button4) {
		mousemod |= UMod::Button4;
		keymod &= ~UMod::Button4;
	}
	if (keymod & UMod::Button5) {
		mousemod |= UMod::Button5;
		keymod &= ~UMod::Button5;
	}
	m_keyModState = UMod_t(keymod);
	m_mouseModState = UMod_t(mousemod);
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
	int modstate = m_keyModState;
	if (keyEvent->getType() == UEvent::KeyPressed) {
		switch (keyEvent->getKeyCode()) {
			case UKey::UK_NUMLOCK: modstate |= UMod::Num; break;
			case UKey::UK_CAPSLOCK: modstate |= UMod::Caps; break;
			case UKey::UK_SCROLLOCK: break;
			case UKey::UK_RSHIFT: modstate |= UMod::RShift; break;
			case UKey::UK_LSHIFT: modstate |= UMod::LShift; break;
			case UKey::UK_RCTRL: modstate |= UMod::RCtrl; break;
			case UKey::UK_LCTRL: modstate |= UMod::LCtrl; break;
			case UKey::UK_RALT: modstate |= UMod::RAlt; break;
			case UKey::UK_LALT: modstate |= UMod::LAlt; break;
			case UKey::UK_RMETA: modstate |= UMod::RMeta; break;
			case UKey::UK_LMETA: modstate |= UMod::LMeta; break;
			/** Left "Windows" key */
			case UKey::UK_LSUPER: break;
			/** Right "Windows" key */
			case UKey::UK_RSUPER: break;
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
			case UKey::UK_RSHIFT: modstate &= ~UMod::RShift; break;
			case UKey::UK_LSHIFT: modstate &= ~UMod::LShift; break;
			case UKey::UK_RCTRL: modstate &= ~UMod::RCtrl; break;
			case UKey::UK_LCTRL: modstate &= ~UMod::LCtrl; break;
			case UKey::UK_RALT: modstate &= ~UMod::RAlt; break;
			case UKey::UK_LALT: modstate &= ~UMod::LAlt; break;
			case UKey::UK_RMETA: modstate &= ~UMod::RMeta; break;
			case UKey::UK_LMETA: modstate &= ~UMod::LMeta; break;
			/** Left "Windows" key */
			case UKey::UK_LSUPER: break;
			/** Right "Windows" key */
			case UKey::UK_RSUPER: break;
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
