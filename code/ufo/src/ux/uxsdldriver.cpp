/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ux/uxsdldriver.cpp
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

#include "ufo/config/ufo_config.hpp"

#ifdef  UFO_USE_SDL

#include "ufo/ux/uxsdldriver.hpp"

#include "ufo/events/ukeysym.hpp"

#include "ufo/ux/uxcontext.hpp"
#include "ufo/ux/uxdisplay.hpp"
#include "ufo/ux/uxframe.hpp"

#include "ufo/events/uquitevent.hpp"
#include "ufo/widgets/urootpane.hpp"
#include "ufo/uplugin.hpp"
#include "ufo/usharedlib.hpp"
#include "ufo/ucontextgroup.hpp"

#include "SDL/SDL.h"
#include "SDL/SDL_syswm.h"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UXSDLDevice, UVideoDevice)
UFO_IMPLEMENT_DYNAMIC_CLASS(UXSDLDriver, UVideoDriver)

static UXSDLDriver * s_sdl_driver = NULL;

#ifdef UFO_OS_WIN32
#define UFO_SDL_LIB "SDL.DLL"
#else
#define UFO_SDL_LIB "libSDL.so"
#endif

UXSDLDriver::UXSDLDriver(const char * sdlPath)
	: m_sdlLib(NULL)
	, m_device(NULL)
	, m_isValid(false)
	, m_isInit(false)
{
	m_sdlLib = new USharedLib();

	// Unix style: load linked libs first
	m_isValid = m_sdlLib->load("");
	if(!m_isValid || m_sdlLib->symbol("SDL_Init") == NULL) {
		m_sdlLib->unload();

		m_isValid = m_sdlLib->load(UFO_SDL_LIB);
	}

#define UFO_SDL_PROC(ret,func,params) \
{ \
	func = (ret (SDLCALL *)params)(getProcAddress(#func)); \
	if (!func) { \
		m_isValid = false; \
	} \
}
#include "ufo/ux/ux_sdl_prototypes.hpp"
#undef UFO_SDL_PROC
	// Removed from macro
	//std::cerr << "Couldn't load SDL function: " << #func << "\n"; \
	s_sdl_driver = this;
}

UXSDLDriver::~UXSDLDriver() {
	s_sdl_driver = NULL;
}

bool
UXSDLDriver::init() {
	if (this->SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0) {
		uError() << "couldn´t initialize SDL:"
			<< this->SDL_GetError() << "\n";
		return false;
	}
	m_isInit = true;
	this->SDL_EnableUNICODE(1);

	//enable key repeat
	this->SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
		SDL_DEFAULT_REPEAT_INTERVAL);

	return true;
}

bool
UXSDLDriver::isInitialized() {
	return m_isInit;
}

void
UXSDLDriver::quit() {
	this->SDL_Quit();
	m_isInit = false;
}

#define UX_MAX_EVENTS 16
void
UXSDLDriver::pumpEvents() {
	this->SDL_PumpEvents();

	// FIXME
	// Should we use a static array
	// SDL_Event is a union and shouldn't take too much time to create
	static SDL_Event events[UX_MAX_EVENTS];
	//if (nmax > USDL_MAX_EVENTS) {
	//	nmax = USDL_MAX_EVENTS;
	//}

	int num;

	num = this->SDL_PeepEvents(events, UX_MAX_EVENTS,
		SDL_GETEVENT, 0xffffff);
	pushSDLEvents(events, num);
}

UVideoDevice *
UXSDLDriver::createVideoDevice() {
	if (!m_device) {
		m_device = new UXSDLDevice(this);
		return m_device;
	}
	return NULL;
}

bool
UXSDLDriver::isValid() const {
	return m_isValid;
}

void *
UXSDLDriver::getProcAddress(const char* proc) {
	return m_sdlLib->symbol(proc);
}

//
// SDL specific
//
UMod_t
mapSDLModifier(SDLMod mod) {
	if (mod  == KMOD_NONE) {
		return UMod::NoModifier;
	}

	int ret = UMod::NoModifier;

	if (mod & KMOD_LSHIFT) {
		ret |= UMod::LShift;
	}
	if (mod & KMOD_RSHIFT) {
		ret |= UMod::RShift;
	}
	if (mod & KMOD_LCTRL) {
		ret |= UMod::LCtrl;
	}
	if (mod & KMOD_RCTRL) {
		ret |= UMod::RCtrl;
	}
	if (mod & KMOD_LALT) {
		ret |= UMod::LAlt;
	}
	if (mod & KMOD_RALT) {
		ret |= UMod::RAlt;
	}
	if (mod & KMOD_LMETA) {
		ret |= UMod::LMeta;
	}
	if (mod & KMOD_RMETA) {
		ret |= UMod::RMeta;
	}
	if (mod & KMOD_NUM) {
		ret |= UMod::Num;
	}
	if (mod & KMOD_CAPS) {
		ret |= UMod::Caps;
	}
	if (mod & KMOD_MODE) {
		ret |= UMod::AltGraph;
	}

	return UMod_t(ret);
}

#undef Button1
#undef Button2
#undef Button3
UMod_t
mapSDLButtonState(int button) {
	int ret = UMod::NoButton;

	if (button & SDL_BUTTON(1)) {
		ret |= UMod::Button1;
	}
	if (button & SDL_BUTTON(2)) {
		ret |= UMod::Button2;
	}
	if (button & SDL_BUTTON(3)) {
		ret |= UMod::Button3;
	}

	return UMod_t(ret);
}

UMod_t
mapSDLButtonChange(int button) {
	if (button == SDL_BUTTON_LEFT) {
		return UMod::LeftButton;
	} else if (button == SDL_BUTTON_MIDDLE) {
		return UMod::MiddleButton;
	} else if (button == SDL_BUTTON_RIGHT) {
		return UMod::RightButton;
	}

	return UMod::NoButton;
}

UKeyCode_t
mapSDLKey(SDLKey key) {
	if (key <= 96 || key == 127) {
		return UKeyCode_t(key);
	}
	// lower case to upper case constants
	if (key <= 126) {
		return UKeyCode_t(key - 32);
	}
	if (key <= 255) {
		return UKeyCode_t(key);
	}
	// key pad
	if (key <= 272) {
		return UKeyCode_t(key + 256);
	}
	// Arrows + Home/End pad
	if (key <= 281) {
		return UKeyCode_t(key + 287);
	}
	// Function keys
	if (key <= 299) {
		return UKeyCode_t(key + 486);
	}
	// Key state modifier keys
	if (key <= 314) {
		return UKeyCode_t(key + 61140);//26);
	}
	// Miscellaneous function keys
	if (key <= 321) {
		return UKeyCode_t(key + 61157);
	}
	// oops, no corresponding key found
	return UKey::UK_UNDEFINED;
}

void
UXSDLDriver::pushSDLEvents(SDL_Event * events, int numEvents) {
	UXDisplay * display = dynamic_cast<UXDisplay*>(UDisplay::getDefault());
	std::vector<UContext*> contexts = display->getContexts();

	for (std::vector<UContext*>::iterator iter = contexts.begin();
			iter != contexts.end();
			++iter) {
		if (UXContext * uxcontext = dynamic_cast<UXContext*>(*iter)) {
			pushSDLEvents(uxcontext, events, numEvents);
		}
	}
}

void
UXSDLDriver::pushSDLEvents(UXContext * uxcontext, SDL_Event * events, int numEvents) {
	UXDisplay * display = dynamic_cast<UXDisplay*>(UDisplay::getDefault());

	/*SDLMod mod = usdl_driver->SDL_GetModState();
	UMod_t modifiers = UMod_t(
		mapSDLModifier(mod) |
		mapSDLButtonState(usdl_driver->SDL_GetMouseState(NULL, NULL))
	);*/

	for (int i = 0; i < numEvents; i++) {
	// always use the modifiers of the last event
	UMod_t modifiers = display->getModState();
	switch (events[i].type) {
		case SDL_MOUSEBUTTONDOWN:
			if (events[i].button.button == 4) {
				// mouse wheel up
				display->pushMouseWheelUp(uxcontext, modifiers,
					events[i].button.x, events[i].button.y);
			} else if (events[i].button.button == 5) {
				// mouse wheel up
				display->pushMouseWheelDown(uxcontext, modifiers,
					events[i].button.x, events[i].button.y);
			} else {
				display->pushMouseButtonDown(
					uxcontext,
					modifiers,
					events[i].button.x,
					events[i].button.y,
					mapSDLButtonChange(events[i].button.button)
				);
			}
			break;
		case SDL_MOUSEBUTTONUP:
			if (events[i].button.button == 4) {
			} else if (events[i].button.button == 5) {
			} else {
			display->pushMouseButtonUp(
				uxcontext,
				modifiers,
				events[i].button.x,
				events[i].button.y,
				mapSDLButtonChange(events[i].button.button)
			);
			}
			break;
		case SDL_MOUSEMOTION:
			display->pushMouseMove(uxcontext, modifiers, events[i].motion.x, events[i].motion.y);
			break;
		case SDL_KEYDOWN:
			display->pushKeyDown(uxcontext, mapSDLModifier(events[i].key.keysym.mod),
				mapSDLKey(events[i].key.keysym.sym), events[i].key.keysym.unicode);
			break;
		case SDL_KEYUP:
			display->pushKeyUp(uxcontext, mapSDLModifier(events[i].key.keysym.mod),
				mapSDLKey(events[i].key.keysym.sym), events[i].key.keysym.unicode);
			break;
		case SDL_VIDEORESIZE:
			if (uxcontext) {
				if (s_sdl_driver) {
					// if we have an SDL driver, reset the video mode
					// otherwise this should be called by client code
					SDL_Surface * screen = s_sdl_driver->SDL_GetVideoSurface();
					s_sdl_driver->SDL_SetVideoMode(events[i].resize.w, events[i].resize.h,
						screen->format->BitsPerPixel, screen->flags);
				}
				if (UXFrame * frame = uxcontext->getFrame()) {
					frame->getVideoDevice()->sigResized().emit(
						frame->getVideoDevice()
					);
				} else {
					uxcontext->setDeviceBounds(
						URectangle(0, 0, events[i].resize.w, events[i].resize.h)
					);
				}
#if !defined(UFO_GFX_X11)
				uxcontext->getContextGroup()->refresh();
#endif
			}
			break;
		case SDL_VIDEOEXPOSE:
			uxcontext->getRootPane()->repaint();
			break;
		case SDL_QUIT:
			display->pushEvent(new UQuitEvent());
			return;
			break;
		default:
			break;
	}
	}
}


class USDLVideoPlugin : public UVideoPlugin {
	UFO_DECLARE_DYNAMIC_CLASS(USDLVideoPlugin)
public:
	USDLVideoPlugin() : m_isInit(false), m_sdldriver(NULL) {}
	virtual ~USDLVideoPlugin() {
		if (m_sdldriver) {
			delete (m_sdldriver);
			m_sdldriver = NULL;
		}
	}
	virtual bool isAvailable() {
		m_sdldriver = new UXSDLDriver();
		if (m_sdldriver->isValid()) {
			return true;
		}
		delete (m_sdldriver);
		m_sdldriver = NULL;
		return false;
	}
	virtual UVideoDriver * createVideoDriver() {
		return m_sdldriver;
	}
private:
	bool m_isInit;
	UXSDLDriver * m_sdldriver;
};
UFO_IMPLEMENT_DYNAMIC_CLASS(USDLVideoPlugin, UVideoPlugin)

UPluginBase *
UXSDLDriver::createPlugin() {
	return new USDLVideoPlugin();
}

void
UXSDLDriver::destroyPlugin(UPluginBase * plugin) {
	delete (plugin);
}



//
// class UXSDLDevice
//
//

UXSDLDevice::UXSDLDevice(UXSDLDriver * driver)
	: m_sdldriver(driver)
	, m_frame(NULL)
	, m_size()
	, m_pos()
	, m_isVisible(false)
	, m_frameStyle(0)
	, m_frameState(0)
	, m_depth(0)
{}

void
UXSDLDevice::setFrame(UXFrame * frame) {
	m_frame = frame;
}


void
UXSDLDevice::setSize(int w, int h) {
	UDimension size(w, h);
	if (size == m_size) {
		return;
	}
	m_size = size;
	if (m_isVisible) {
		show();
	}
}

UDimension
UXSDLDevice::getSize() const {
	return m_size;
}

void
UXSDLDevice::setLocation(int x, int y) {
	UPoint pos(x, y);
	if (m_pos == pos) {
		return;
	}
	m_pos = pos;
	if (m_isVisible) {
		// init system info struct
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);

		if (m_sdldriver->SDL_GetWMInfo(&info) > 0 ) {
#if defined(UFO_GFX_X11)
			if (info.subsystem == SDL_SYSWM_X11) {
				XMoveWindow(info.info.x11.display, info.info.x11.window, x, y);
			}
#elif defined(UFO_GFX_WIN32)
			RECT windowRect;
			GetWindowRect(info.window, &windowRect);
			MoveWindow(info.window,
				x, // x
				y, // y
				windowRect.right - windowRect.left, // new width (== old width)
				windowRect.bottom - windowRect.top, // new height (== old height)
				true // repaint
			);
#endif // UFO_GFX_WIN32
		}
	}
}


UPoint
UXSDLDevice::getLocation() const {
	return m_pos;
}


void
UXSDLDevice::setTitle(const std::string & title) {
	m_sdldriver->SDL_WM_SetCaption(title.c_str(), NULL);
}

std::string
UXSDLDevice::getTitle() const {
	char * titleL;
	m_sdldriver->SDL_WM_GetCaption(&titleL, NULL);
	return titleL;
}


void
UXSDLDevice::setDepth(int depth) {
	m_depth = depth;
}

int
UXSDLDevice::getDepth() const {
	return m_depth;
}


void
UXSDLDevice::swapBuffers() {
	m_sdldriver->SDL_GL_SwapBuffers();
}

void
UXSDLDevice::makeContextCurrent() {
}


bool
UXSDLDevice::show() {
	int sdl_flags = SDL_OPENGL;
	if (m_frameState & FrameFullScreen) {
		sdl_flags |= SDL_FULLSCREEN;
	}
	if (m_frameState & FrameResizable) {
		sdl_flags |= SDL_RESIZABLE;
	}

	// ignore video resize events which are created by this method
	// this is a bug in SDL
	Uint8 state = m_sdldriver->SDL_EventState(SDL_VIDEORESIZE, SDL_IGNORE);

	if (m_sdldriver->SDL_SetVideoMode(m_size.w, m_size.h, m_depth,
			sdl_flags) == NULL) {
		std::cerr << "tried to create SDL frame with"
		<< "\nwidth=" << m_size.w
		<< "\nheight=" << m_size.h
		<< "\ndepth=" << m_depth << std::endl;
		// ..
		std::cerr << "SDL Error: couldn´t set video mode : "
		<< m_sdldriver->SDL_GetError() << std::endl;

		//USDL_App::getInstance()->shutdown();
		return false;
	}
	if (m_depth == 0) {
//		m_depth = screen->format->BitsPerPixel;
	}
	// restore event state
	m_sdldriver->SDL_EventState(SDL_VIDEORESIZE, state);
	return true;
}

void
UXSDLDevice::hide()
{}


void
UXSDLDevice::setFrameStyle(uint32_t frameStyle) {
	m_frameStyle = frameStyle;
}

uint32_t
UXSDLDevice::getFrameStyle() const {
	return m_frameStyle;
}


void
UXSDLDevice::setInitialFrameState(uint32_t frameState) {
	m_frameState = frameState;
}

uint32_t
UXSDLDevice::getFrameState() const {
	return m_frameState;
}


#endif // UFO_USE_SDL
