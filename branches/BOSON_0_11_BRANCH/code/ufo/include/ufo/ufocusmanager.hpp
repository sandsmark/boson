/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ufocusmanager.hpp
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

#ifndef UFOCUSMANAGER_HPP
#define UFOCUSMANAGER_HPP

#include "uobject.hpp"

namespace ufo {

class UEvent;
class UMouseEvent;
class UKeyEvent;

/** @short This class controls the mouse and keyboard focus.
  * @ingroup misc
  * @ingroup events
  *
  * The focus manager is not bound to a specific UFO context.
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UFocusManager : public UObject {
	UFO_DECLARE_DYNAMIC_CLASS(UFocusManager)
public: // Public types
	// FIXME: are there any other focus policies?
	enum FocusPolicy {
		ClickToFocus = 0,
		FocusUnderMouse = 1
	};

public:
	UFocusManager();

	static UFocusManager * getFocusManager();
	static void setFocusManager(UFocusManager * focusManager);

public: // Public methods
	/** Processes an event.
	  * If it is an appropriate event, focus may be changed.
	  */
	virtual void processEvent(UEvent * e);

	/** Sets the focus policy. The default policy is ClickToFocus. */
	void setFocusPolicy(FocusPolicy policy);
	FocusPolicy getFocusPolicy() const;

protected: // Protected methods
	void processMouseEvent(UMouseEvent * e);
	void processKeyEvent(UKeyEvent * e);

private: // Private attributes
	FocusPolicy m_policy;
	static UFocusManager * m_focusManager;
};

} // namespace ufo

#endif // UFOCUSMANAGER_HPP
