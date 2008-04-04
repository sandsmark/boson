/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ux/uxglxdriver.hpp
    begin             : Fri Aug 13 2004
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

#ifndef UXGLXDRIVER_HPP
#define UXGLXDRIVER_HPP

#include "../uvideodriver.hpp"
#include "../uvideodevice.hpp"

#include "../events/ukeysym.hpp"
#include "GL/glx.h"
#include <X11/Xlib.h>

namespace ufo {

class UXContext;
class UXDisplay;
class UXGLXDevice;
class UVideoPlugin;
class UPluginBase;
class USharedLib;

/** @short A video driver which uses GLX to create video devices.
  * @ingroup internal
  *
  * @see UXGLXVideoDevice
  * @author Johannes Schmidt
  */
class UFO_EXPORT UXGLXDriver : public UVideoDriver {
	UFO_DECLARE_DYNAMIC_CLASS(UXGLXDriver)
public:
	UXGLXDriver();
	virtual ~UXGLXDriver();

public: // Implements UVideoDriver
	virtual bool init();
	virtual bool isInitialized();
	virtual void quit();
	virtual std::string getName();

	virtual void pumpEvents();

	virtual UVideoDevice * createVideoDevice();

	bool isValid() const;

#define UFO_GLX_PROC(ret,func,params) ret (*func) params;
UFO_GLX_PROC(Bool,glXMakeCurrent,(Display *dpy, GLXDrawable drawable, GLXContext ctx))
UFO_GLX_PROC(Bool,glXQueryExtension,(Display *dpy, int *errorBase, int *eventBase))
UFO_GLX_PROC(void,glXSwapBuffers,(Display *dpy, GLXDrawable drawable))
UFO_GLX_PROC(XVisualInfo*,glXChooseVisual,(Display *dpy, int screen, int *attribList))
UFO_GLX_PROC(void,glXDestroyContext,(Display *dpy, GLXContext ctx))
UFO_GLX_PROC(GLXContext,glXCreateContext,(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct))
#undef UFO_GLX_PROC

public: // Public methods
	Display * getX11Display() const;
	Window getRootWindow() const;
	Atom * getDeleteWindowAtom();
	UXContext * getContextFromWindow(int window) const;

public: // Public methods
	void initKeymap();
	void pushXEvent(UXContext * context, const XEvent & event);
	UKeyCode_t mapX11Keycode(const XKeyEvent & xkey);
	wchar_t mapX11Unicode(const XKeyEvent & xkey);
	UMod::Modifier mapX11Modifiers(int modifiers);

protected:
	friend class UXGLXDevice;
	/** Removes device from device list. */
	void destroyed(UXGLXDevice * device);

public: // plugin methods
	static UPluginBase * createPlugin();
	static void destroyPlugin(UPluginBase * plugin);

private: // Private attributes
	bool m_isValid;
	bool m_isInit;
	bool m_createdGLDriver;
	Display * m_x11Display;
	Window m_rootWindow;
	Atom m_deleteWindowAtom;
	UXDisplay * m_display;
	std::vector<UXGLXDevice*> m_windowMap;

	UKeyCode_t m_MISC_keymap[256];
	UKeyCode_t m_ODD_keymap[255];
};

/** @short A video device which uses GLX for windowing.
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UXGLXDevice : public UVideoDevice {
	UFO_DECLARE_DYNAMIC_CLASS(UXGLXDevice)
public:
	UXGLXDevice(UXGLXDriver * driver);
	virtual ~UXGLXDevice();
public: // Implements UVideoDevice
	virtual void setSize(int w, int h);
	virtual UDimension getSize() const;

	virtual void setLocation(int x, int y);
	virtual UPoint getLocation() const;

	virtual void setTitle(const std::string & title);
	virtual std::string getTitle() const;

	virtual void setDepth(int depth);
	virtual int getDepth() const;

	virtual void swapBuffers();
	virtual void makeContextCurrent();

	virtual bool show();
	virtual void hide();

	virtual void setFrameStyle(uint32_t frameStyle);
	virtual uint32_t getFrameStyle() const;

	virtual void setInitialFrameState(uint32_t frameState);
	virtual uint32_t getFrameState() const;

	virtual void setFrame(UXFrame * frame);
	virtual void notify(uint32_t type, int arg1, int arg2, int arg3, int arg4);
public:
	virtual UXFrame * getFrame();
	Window getWindow() { return m_window; }
	GLXContext getGLContext() { return m_glContext; }

	void setDecorations();
	void setSizeHints();
	void setWMHints();
protected:
	virtual XVisualInfo * chooseVisual();
	int getAttribute(int key);
	void setAttribute(int key, int value);

private: // Private attributes
	UXGLXDriver * m_glxDriver;
	Window m_window;
	GLXContext m_glContext;
	UXFrame * m_frame;
	mutable UDimension m_size;
	mutable UPoint m_pos;
	bool m_isVisible;
	uint32_t m_frameStyle;
	uint32_t m_frameState;
	int m_depth;
	std::string m_title;
	std::map<int, int> m_attributes;
};

} // namespace ufo

#endif // UXGLXDRIVER_HPP
