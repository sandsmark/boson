/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ux/uxsdldriver.hpp
    begin             : Sun Aug 8 2004
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

#ifndef UXSDLDRIVER_HPP
#define UXSDLDRIVER_HPP

#include "../uvideodriver.hpp"
#include "../uvideodevice.hpp"

#include "../events/ukeysym.hpp"
#include "SDL.h"
#include "SDL_syswm.h"

namespace ufo {

class UXContext;
class UVideoPlugin;
class UPluginBase;
class USharedLib;
class UXSDLDevice;

/** @short A video driver which uses SDL for windowing.
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UXSDLDriver : public UVideoDriver {
	UFO_DECLARE_DYNAMIC_CLASS(UXSDLDriver)
public:
	UXSDLDriver(const char * sdlPath = "");
	virtual ~UXSDLDriver();

public: // Implements UVideoDriver
	virtual bool init();
	virtual bool isInitialized();
	virtual void quit();
	virtual std::string getName();

	virtual void pumpEvents();

	virtual UVideoDevice * createVideoDevice();

public: // sdl methods
#define UFO_SDL_PROC(ret,func,params) ret (SDLCALL *func) params;
#include "ux_sdl_prototypes.hpp"
#undef UFO_SDL_PROC

	bool isValid() const;

public: // init
	void * getProcAddress(const char* proc);

public: // Public static methods
	/** Pushes the given SDL events to all UFO contexts registered
	  * at the default display.
	  */
	static void pushSDLEvents(SDL_Event * events, int numEvents);
	/** Pushes the given SDL events to the given UX context. */
	static void pushSDLEvents(UXContext * uxcontext, SDL_Event * events, int numEvents);

public: // plugin methods
	static UPluginBase * createPlugin();
	static void destroyPlugin(UPluginBase * plugin);

private: // Private attributes
	USharedLib * m_sdlLib;
	UXSDLDevice * m_device;
	bool m_isValid;
	bool m_isInit;
	bool m_createdGLDriver;
};


/** @short A video device which uses SDL for windowing.
  * @ingroup internal
  *
  * @author Johannes Schmidt
  */
class UFO_EXPORT UXSDLDevice : public UVideoDevice {
	UFO_DECLARE_ABSTRACT_CLASS(UXSDLDevice)
public:
	UXSDLDevice(UXSDLDriver * driver);
public: // Implements UVideoDevice
	virtual void setFrame(UXFrame * frame);

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

	virtual void notify(uint32_t type, int arg1, int arg2, int arg3, int arg4);
private: // Private attributes
	UXSDLDriver * m_sdldriver;
	UXFrame * m_frame;
	UDimension m_size;
	UPoint m_pos;
	bool m_isVisible;
	uint32_t m_frameStyle;
	uint32_t m_frameState;
	int m_depth;
};

} // namespace ufo

#endif // UXSDLDRIVER_HPP
