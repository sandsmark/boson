/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ufocusmanager.cpp
    begin             : Mon Sep 9 2002
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

#include "ufo/ufocusmanager.hpp"

#include "ufo/events/umouseevent.hpp"
#include "ufo/events/ukeyevent.hpp"

#include "ufo/widgets/uwidget.hpp"


namespace ufo {

UFO_IMPLEMENT_DYNAMIC_CLASS(UFocusManager, UObject)

UFocusManager * UFocusManager::m_focusManager = NULL;

UFocusManager::UFocusManager() : m_policy(ClickToFocus) {}

UFocusManager *
UFocusManager::getFocusManager() {
	// FIXME ! memory leak
	if (!m_focusManager) {
		m_focusManager = new UFocusManager();
	}
	return m_focusManager;
}

void
UFocusManager::setFocusManager(UFocusManager * focusManager) {
	if (focusManager) {
		m_focusManager = focusManager;
	}
}

void
UFocusManager::processEvent(UEvent * e) {
	switch (e->getType()) {
		case UEvent::MousePressed:
		case UEvent::MouseEntered:
			// FIXME: is static cast really save?
			processMouseEvent(static_cast<UMouseEvent*>(e));
			break;
		case UEvent::KeyPressed:
			processKeyEvent(static_cast<UKeyEvent*>(e));
			break;
		default:
			break;
	}
}

void
UFocusManager::setFocusPolicy(FocusPolicy policy) {
	m_policy = policy;
}

UFocusManager::FocusPolicy
UFocusManager::getFocusPolicy() const {
	return m_policy;
}


//
// Protected methods
//
void
UFocusManager::processMouseEvent(UMouseEvent * e) {
	if ((m_policy == ClickToFocus) && (e->getType() == UEvent::MousePressed)) {
		UWidget * w = e->getWidget();
		w->requestFocus();
		e->consume();
	}
	else if ((m_policy == FocusUnderMouse) &&
			(e->getType() == UEvent::MouseEntered)) {
		UWidget * w = e->getWidget();
		w->requestFocus();
		e->consume();
	}
}

void
UFocusManager::processKeyEvent(UKeyEvent * e) {
}

} // namespace ufo
