/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/uvideodevice.hpp
    begin             : Wed Jul 28 2004
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

#ifndef UVIDEODEVICE_HPP
#define UVIDEODEVICE_HPP

#include "uobject.hpp"
#include "util/upoint.hpp"
#include "util/udimension.hpp"
#include "signals/usignal.hpp"

namespace ufo {

class UXFrame;

/** @short An abstraction for a native video device (e.g. X11 windows).
  * @ingroup windowing
  * @ingroup native
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UVideoDevice : public UObject {
	UFO_DECLARE_ABSTRACT_CLASS(UVideoDevice)
public: // public methods
	virtual void setSize(int w, int h) = 0;
	virtual UDimension getSize() const = 0;

	virtual void setLocation(int x, int y) = 0;
	virtual UPoint getLocation() const = 0;

	virtual void setTitle(const std::string & title) = 0;
	virtual std::string getTitle() const = 0;

	virtual void setDepth(int depth) = 0;
	virtual int getDepth() const = 0;

	virtual void swapBuffers() = 0;
	/** Makes the OpenGL context associated with this device the current
	  * OpenGL context for the current thread.
	  */
	virtual void makeContextCurrent() = 0;

	virtual bool show() = 0;
	virtual void hide() = 0;

	/** Changes the frame style for the native device.
	  * @see FrameStyle
	  */
	virtual void setFrameStyle(uint32_t frameStyle) = 0;
	virtual uint32_t getFrameStyle() const = 0;

	/** Changes the initial frame state for the native device.
	  * This method may not have any effect if the window is already visible.
	  * @see FrameState
	  */
	virtual void setInitialFrameState(uint32_t frameState) = 0;
	/** @return The current frame state or the initial frame state if this
	  *  window is not visible.
	  */
	virtual uint32_t getFrameState() const = 0;

public: // Public methods, used to notify the device of system changes
	virtual void setFrame(UXFrame * frame) = 0;
	/** Notifies this device abstraction of changes to the native device.
	  * Warning: The interface for this method is experimental.
	  * @param type The type of the change (we are using UEvent::Type for
	  *  distinction)
	  * @param arg1 The first argument (may be 0)
	  * @param arg2 The second argument (may be 0)
	  * @param arg3 The third argument (may be 0)
	  * @param arg4 The fourth argument (may be 0)
	  */
	virtual void notify(uint32_t type, int arg1, int arg2, int arg3, int arg4) = 0;

public: // Public signals
	USignal1<UVideoDevice*> & sigMoved();
	USignal1<UVideoDevice*> & sigResized();
private: // Private signals
	USignal1<UVideoDevice*> m_sigMoved;
	USignal1<UVideoDevice*> m_sigResized;
};

//
// inline implementation
//

inline USignal1<UVideoDevice*> &
UVideoDevice::sigMoved() {
	return m_sigMoved;
}

inline USignal1<UVideoDevice*> &
UVideoDevice::sigResized() {
	return m_sigResized;
}

} // namespace ufo

#endif // UVIDEODEVICE_HPP
