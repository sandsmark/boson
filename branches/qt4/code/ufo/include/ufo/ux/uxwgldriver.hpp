/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ux/uxwgldriver.hpp
    begin             : Thu Sep 23 2004
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

#ifndef UXWGLDRIVER_HPP
#define UXWGLDRIVER_HPP

#include "../uvideodriver.hpp"
#include "../uvideodevice.hpp"

#include "../events/ukeysym.hpp"

#include <map>

// FIXME should be done by ufo_config ?
#include <windows.h>

namespace ufo {

class UXContext;
class UXDisplay;
class UXWGLDevice;
class UVideoPlugin;
class UPluginBase;
class USharedLib;

/** @short A video driver which uses Win32/WGL for windowing.
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UXWGLDriver : public UVideoDriver {
	UFO_DECLARE_DYNAMIC_CLASS(UXWGLDriver)
public:
	UXWGLDriver();
	virtual ~UXWGLDriver();

public: // Implements UVideoDriver
	virtual bool init();
	virtual bool isInitialized();
	virtual void quit();
	virtual std::string getName();

	virtual void pumpEvents();

	virtual UVideoDevice * createVideoDevice();

	bool isValid() const;

#define UFO_WGL_PROC(ret,func,params) ret (WINAPI *func) params;
UFO_WGL_PROC(BOOL,wglMakeCurrent,(HDC, HGLRC))
UFO_WGL_PROC(HGLRC,wglCreateContext,(HDC))
UFO_WGL_PROC(BOOL,wglDeleteContext,(HGLRC))
UFO_WGL_PROC(BOOL,wglShareLists,(HGLRC, HGLRC))
#undef UFO_WGL_PROC

public: // Public methods
	HINSTANCE getInstance() const;
	UXContext * getContextFromWindow(HWND window);
	UXWGLDevice * getDeviceFromWindow(HWND window);

public: // Public methods
	void initKeymap();

	LRESULT windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	UMod_t mapWin32Button(int msg);

public: // plugin methods
	static UPluginBase * createPlugin();
	static void destroyPlugin(UPluginBase * plugin);

private: // Private attributes
	bool m_isValid;
	bool m_isInit;
	bool m_createdGLDriver;
	UXDisplay * m_display;
	HINSTANCE m_instance;
	std::vector<UXWGLDevice*> m_windowMap;

	// FIXME: do not use static array size
	UKeyCode_t VK_keymap[256];
};


/** @short A video device which uses Win32/WGL for windowing.
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UXWGLDevice : public UVideoDevice {
	UFO_DECLARE_DYNAMIC_CLASS(UXWGLDevice)
public:
	UXWGLDevice(UXWGLDriver * driver);
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
	virtual UXFrame * getFrame() const;
	bool setupPixelFormat(unsigned char layer_type);
	HWND getWindow() { return m_window; }
	HDC getDC();
	HGLRC getGLContext() { return m_glContext; }

	void setDecorations();
	void setSizeHints();
	void setWMHints();
protected:
	int getAttribute(int key);
	void setAttribute(int key, int value);

private: // Private attributes
	UXWGLDriver * m_wglDriver;
	HWND m_window;
	HDC m_dc;
	HGLRC m_glContext;
	UXFrame * m_frame;
	UDimension m_size;
	UPoint m_pos;
	bool m_isVisible;
	uint32_t m_frameStyle;
	uint32_t m_frameState;
	int m_depth;
	std::string m_title;
	std::map<int, int> m_attributes;
};

} // namespace ufo

#endif // UXWGLDRIVER_HPP
