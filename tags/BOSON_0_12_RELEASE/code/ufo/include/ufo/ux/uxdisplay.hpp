/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ux/uxdisplay
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef UXDISPLAY_HPP
#define UXDISPLAY_HPP

#include "../uabstractdisplay.hpp"

namespace ufo {

class UVideoDriver;
class UXContext;
class UXFrame;

/** @short The Display is the connection to the underlying windowing system.
  * @ingroup native
  *
  * On construction, a video driver is loaded and initialized.
  * The display can create frames.
  * On destruction, all frames and system ressources allocated by this object
  * are cleaned up.
  * You may use the dummy video driver (by specifying "dummy" at construction)
  * if you want to use your custom OpenGL context. In this case, you have to
  * pump your own events to the event queue
  * (either by calling pushEvent(UEvent*) or the convenience push-methods).
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UXDisplay : public UAbstractDisplay {
	UFO_DECLARE_DYNAMIC_CLASS(UXDisplay)
public:
	friend class UXContext;
public:
	/** Creates a new UX display object and registers it as default display.
	  * @param videoDriver The display driver to use.
	  *   If empty, all available drivers are tested.
	  */
	UXDisplay(const std::string & videoDriver = "");
	virtual ~UXDisplay();

public:
	/** Returns true if this UXDisplay is a valid display connection
	  * with a valid video driver.
	  */
	virtual bool isValid() const;
	/** Returns the video driver used by this object for all
	  * system operations.
	  * If this display is not valid, returns NULL.
	  */
	virtual UVideoDriver * getVideoDriver() const;

	/** Uses the video driver to create a valid video device.
	  * If this was succesful, a frame is returned, otherwise NULL.
	  * The returned frame should have a title bar, is resizable, closable,
	  * maximizable and iconifyable.
	  * @see UXFrame::setFrameStyle
	  */
	virtual UXFrame * createFrame();

	/** Returns all frames created by this display object. */
	virtual std::vector<UXFrame*> getFrames() const;

public: // Implements UDisplay
	virtual void pumpEvents();
	virtual std::vector<UContext*> getContexts() const;

public: // general event push methods
	/** Resets the modifier states. LibUFO usually keeps track of the pressed
	  * modifiers. But if a key is pressed while the UFO frame isn't focused,
	  * i.e. UFO does not receive the key event, the modifier state has to
	  * be updated.
	  */
	virtual void setModState(UMod_t modifiers);
	/** Pushes a new mouse down event on the event queue.
	  * @param context The context this event belongs to
	  * @param x The x coordinate of the mouse event
	  * @param y The y coordinate of the mouse event
	  * @param button The mouse button which has been pressed
	  */
	virtual void pushMouseButtonDown(UContext * context,
		int x, int y, UMod_t button);
	/** Pushes a new mouse up event on the event queue.
	  * @param context The context this event belongs to
	  * @param x The x coordinate of the mouse event
	  * @param y The y coordinate of the mouse event
	  * @param button The mouse button which has been released
	  */
	virtual void pushMouseButtonUp(UContext * context,
		int x, int y, UMod_t button);
	/** Pushes a new mouse wheel down event on the event queue.
	  * @param context The context this event belongs to
	  * @param x The x coordinate of the mouse event
	  * @param y The y coordinate of the mouse event
	  * @param delta The number of units to be scrolled (mod 120)
	  * @param mouseWheelNum The number of the mouse wheel
	  */
	virtual void pushMouseWheelDown(UContext * context,
		int x, int y, int delta = -120, int mouseWheelNum = 0);
	/** Pushes a new mouse wheel up event on the event queue.
	  * @param context The context this event belongs to
	  * @param x The x coordinate of the mouse event
	  * @param y The y coordinate of the mouse event
	  * @param delta The number of units to be scrolled (mod 120)
	  * @param mouseWheelNum The number of the mouse wheel
	  */
	virtual void pushMouseWheelUp(UContext * context,
		int x, int y, int delta = 120, int mouseWheelNum = 0);
	/** Pushes a new mouse move event on the event queue.
	  * @param context The context this event belongs to
	  * @param x The x coordinate of the mouse event
	  * @param y The y coordinate of the mouse event
	  */
	virtual void pushMouseMove(UContext * context,
		int x, int y);
	/** Pushes a new key down event on the event queue.
	  * @param context The context this event belongs to
	  * @param keyCode The UFO key symbol
	  * @param keyChar The unicode representation of the key
	  */
	virtual void pushKeyDown(UContext * context,
		UKeyCode_t keyCode, wchar_t keyChar);
	/** Pushes a new key up event on the event queue.
	  * @param context The context this event belongs to
	  * @param keyCode The UFO key symbol
	  * @param keyChar The unicode representation of the key
	  */
	virtual void pushKeyUp(UContext * context,
		UKeyCode_t keyCode, wchar_t keyChar);

	/** @obsolete */
	void pushMouseButtonDown(UContext * context, UMod_t modifiers,
		int x, int y, UMod_t button) {
		pushMouseButtonDown(context, x, y, button);
	}
	void pushMouseButtonUp(UContext * context, UMod_t modifiers,
		int x, int y, UMod_t button) {
		pushMouseButtonUp(context, x, y, button);
	}
	void pushMouseWheelDown(UContext * context, UMod_t modifiers,
		int x, int y, int delta = -120, int mouseWheelNum = 0) {
		pushMouseWheelDown(context, x, y, delta, mouseWheelNum);
	}
	void pushMouseWheelUp(UContext * context, UMod_t modifiers,
		int x, int y, int delta = 120, int mouseWheelNum = 0) {
		pushMouseWheelUp(context, x, y, delta, mouseWheelNum);
	}
	void pushMouseMove(UContext * context, UMod_t modifiers,
		int x, int y) {
		pushMouseMove(context, x, y);
	}
	void pushKeyDown(UContext * context, UMod_t modifiers,
		UKeyCode_t keyCode, wchar_t keyChar) {
		pushKeyDown(context, keyCode, keyChar);
	}
	void pushKeyUp(UContext * context, UMod_t modifiers,
		UKeyCode_t keyCode, wchar_t keyChar) {
		pushKeyUp(context, keyCode, keyChar);
	}
public: // general event dispatch methods
	/** Like @ref pushMouseButtonDown, but the event is dispatched
	  * immediately
	  * @return TRUE, if the event got consumed, otherwise FALSE
	  */
	virtual bool dispatchMouseButtonDown(UContext * context,
		int x, int y, UMod_t button);
	/** Like @ref pushMouseButtonUp, but the event is dispatched
	  * immediately
	  * @return TRUE, if the event got consumed, otherwise FALSE
	  */
	virtual bool dispatchMouseButtonUp(UContext * context,
		int x, int y, UMod_t button);
	/** Like @ref pushMouseWheelDown, but the event is dispatched
	  * immediately
	  * @return TRUE, if the event got consumed, otherwise FALSE
	  */
	virtual bool dispatchMouseWheelDown(UContext * context,
		int x, int y, int delta = -120, int mouseWheelNum = 0);
	/** Like @ref pushMouseWheelUp, but the event is dispatched
	  * immediately
	  */
	virtual bool dispatchMouseWheelUp(UContext * context,
		int x, int y, int delta = 120, int mouseWheelNum = 0);
	/** Like @ref pushMouseMove , but the event is dispatched
	  * immediately
	  * @return TRUE, if the event got consumed, otherwise FALSE
	  */
	virtual bool dispatchMouseMove(UContext * context,
		int x, int y);
	/** Like @ref pushKeyDown, but the event is dispatched
	  * immediately
	  * @return TRUE, if the event got consumed, otherwise FALSE
	  */
	virtual bool dispatchKeyDown(UContext * context,
		UKeyCode_t keyCode, wchar_t keyChar);
	/** Like @ref pushKeyUp, but the event is dispatched
	  * immediately
	  * @return TRUE, if the event got consumed, otherwise FALSE
	  */
	virtual bool dispatchKeyUp(UContext * context,
		UKeyCode_t keyCode, wchar_t keyChar);

protected: // Protected methods
	/** registers a new UXContext. */
	void registerContext(UXContext * context);
	void unregisterContext(UXContext * context);

	virtual bool mouseButtonDown(UContext * context,
		int x, int y, UMod_t button, bool push = true);
	virtual bool mouseButtonUp(UContext * context,
		int x, int y, UMod_t button, bool push = true);
	virtual bool mouseWheelDown(UContext * context,
		int x, int y, int delta = -120, int mouseWheelNum = 0, bool push = true);
	virtual bool mouseWheelUp(UContext * context,
		int x, int y, int delta = 120, int mouseWheelNum = 0, bool push = true);
	virtual bool mouseMove(UContext * context,
		int x, int y, bool push = true);
	virtual bool keyDown(UContext * context,
		UKeyCode_t keyCode, wchar_t keyChar, bool push = true);
	virtual bool keyUp(UContext * context,
		UKeyCode_t keyCode, wchar_t keyChar, bool push = true);

private: // Private attributes
	UVideoDriver * m_videoDriver;
	std::vector<UXFrame*> m_frames;
	std::vector<UContext*> m_contexts;
	UPoint m_lastMousePressPos;
	uint32_t m_lastMousePressTime;
	uint32_t m_clickDelay;
	uint32_t m_clickCount;
	UPoint m_mousePos;
};

} // namespace ufo

#endif // UXDISPLAY_HPP
