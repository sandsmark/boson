/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2004 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ux/uxglxdriver.cpp
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include "ufo/config/ufo_config.hpp"

#ifdef  UFO_USE_GLX

#include "ufo/ux/uxglxdriver.hpp"

#include "ufo/events/ukeysym.hpp"

#include "ufo/ux/uxcontext.hpp"
#include "ufo/ux/uxdisplay.hpp"
#include "ufo/ux/uxframe.hpp"

#include "ufo/events/uquitevent.hpp"
#include "ufo/widgets/urootpane.hpp"
#include "ufo/uplugin.hpp"
#include "ufo/usharedlib.hpp"

#include "ufo/gl/ugl_driver.hpp"

#include <X11/Xatom.h>
#include <X11/keysym.h>

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UXGLXDevice, UVideoDevice)
UFO_IMPLEMENT_DYNAMIC_CLASS(UXGLXDriver, UVideoDriver)


//UKeyCode_t UXGLXDriver::m_MISC_keymap[256];
//UKeyCode_t UXGLXDriver::m_ODD_keymap[255];

UXGLXDriver::UXGLXDriver()
	: m_isValid(false)
	, m_isInit(false)
	, m_x11Display(NULL)
	, m_rootWindow(0)
	, m_display(NULL)
{
	m_display = dynamic_cast<UXDisplay*>(UDisplay::getDefault());

#define UFO_GLX_PROC(ret,func,params) \
{ \
	func = (ret (*)params)(ugl_driver->getProcAddress(#func)); \
	if (!func) { \
		std::cerr << "Couldn't load SDL function: " << #func << "\n"; \
		m_isValid = false; \
	} \
}
UFO_GLX_PROC(Bool,glXMakeCurrent,(Display *dpy, GLXDrawable drawable, GLXContext ctx))
UFO_GLX_PROC(Bool,glXQueryExtension,(Display *dpy, int *errorBase, int *eventBase))
UFO_GLX_PROC(void,glXSwapBuffers,(Display *dpy, GLXDrawable drawable))
UFO_GLX_PROC(XVisualInfo*,glXChooseVisual,(Display *dpy, int screen, int *attribList))
UFO_GLX_PROC(void,glXDestroyContext,(Display *dpy, GLXContext ctx))
UFO_GLX_PROC(GLXContext,glXCreateContext,(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct))
#undef UFO_GLX_PROC
}

bool
UXGLXDriver::init() {
	// Get it from DISPLAY environment variable?
	m_x11Display = XOpenDisplay(XDisplayName(NULL));

	if (m_x11Display == NULL ) {
		uError() << "Couldn't open X11 display\n";
		return false;
	}
	if(!this->glXQueryExtension(m_x11Display, NULL, NULL)) {
		// GLX extensions have not been found...
		uError() << "OpenGL GLX extension not supported by display '"
		<< XDisplayName(NULL)
		<< "'\n";
	}

	// The same applying to the root window
	m_rootWindow = RootWindow(m_x11Display, DefaultScreen(m_x11Display));

	// Create the window deletion atom
	m_deleteWindow = XInternAtom(
		m_x11Display,
		"WM_DELETE_WINDOW",
		false
	);

	initKeymap();
	m_isInit = true;
	return true;
}

bool
UXGLXDriver::isInitialized() {
	return m_isInit;
}

void
UXGLXDriver::quit() {
	XSync(m_x11Display, false);
	XCloseDisplay(m_x11Display);
	m_isInit = false;
}

void
UXGLXDriver::pumpEvents() {
	XFlush(m_x11Display);
	XEvent xevent;
	while (XPending(m_x11Display)) {//XEventsQueued(m_x11Display, QueuedAlready)) { //
		XNextEvent(m_x11Display, &xevent);
		pushXEvent(getContextFromWindow(xevent.xany.window), xevent);
	}
}



UVideoDevice *
UXGLXDriver::createVideoDevice() {
	UXGLXDevice * device = new UXGLXDevice(this);
	m_windowMap.push_back(device);
	return device;
}

bool
UXGLXDriver::isValid() const {
	return m_isValid;
}

Display *
UXGLXDriver::getX11Display() const {
	return m_x11Display;
}

Window
UXGLXDriver::getRootWindow() const {
	return m_rootWindow;
}

Atom *
UXGLXDriver::getDeleteWindowAtom() {
	return &m_deleteWindow;
}

UXContext *
UXGLXDriver::getContextFromWindow(int window) const {
	for (std::vector<UXGLXDevice*>::const_iterator iter = m_windowMap.begin();
			iter != m_windowMap.end(); ++iter) {
		if ((*iter)->getWindow() == window) {
			return (*iter)->getFrame()->getContext();
		}
	}
	return NULL;
}
/*
	std::map<int, UXGLXDevice*>::const_iterator iter = m_windowMap.find(window);

	if (iter != m_windowMap.end() && ((*iter).second)) {
		std::cerr << (*iter).second << "\n" << ((*iter).second)->getFrame() << "\n";
		return ((*iter).second)->getFrame()->getContext();
	}
	return NULL;
}*/

void
UXGLXDriver::initKeymap() {
	// Odd keys used in international keyboards
	for (int i=0; i<256; ++i ) {
		m_ODD_keymap[i] = UKey::UK_UNDEFINED;
	}

#ifdef XK_dead_circumflex
	// These X keysyms have 0xFE as the high byte
	m_ODD_keymap[XK_dead_circumflex&0xFF] = UKey::UK_CARET;
#endif

	// Map the miscellaneous keys
	for (int i=0; i<256; ++i) {
		m_MISC_keymap[i] = UKey::UK_UNDEFINED;
	}

	// These X keysyms have 0xFF as the high byte
	m_MISC_keymap[XK_BackSpace&0xFF] = UKey::UK_BACKSPACE;
	m_MISC_keymap[XK_Tab&0xFF] = UKey::UK_TAB;
	m_MISC_keymap[XK_Clear&0xFF] = UKey::UK_CLEAR;
	m_MISC_keymap[XK_Return&0xFF] = UKey::UK_RETURN;
	m_MISC_keymap[XK_Pause&0xFF] = UKey::UK_PAUSE;
	m_MISC_keymap[XK_Escape&0xFF] = UKey::UK_ESCAPE;
	m_MISC_keymap[XK_Delete&0xFF] = UKey::UK_DELETE;

	m_MISC_keymap[XK_KP_0&0xFF] = UKey::UK_KP0;		/* Keypad 0-9 */
	m_MISC_keymap[XK_KP_1&0xFF] = UKey::UK_KP1;
	m_MISC_keymap[XK_KP_2&0xFF] = UKey::UK_KP2;
	m_MISC_keymap[XK_KP_3&0xFF] = UKey::UK_KP3;
	m_MISC_keymap[XK_KP_4&0xFF] = UKey::UK_KP4;
	m_MISC_keymap[XK_KP_5&0xFF] = UKey::UK_KP5;
	m_MISC_keymap[XK_KP_6&0xFF] = UKey::UK_KP6;
	m_MISC_keymap[XK_KP_7&0xFF] = UKey::UK_KP7;
	m_MISC_keymap[XK_KP_8&0xFF] = UKey::UK_KP8;
	m_MISC_keymap[XK_KP_9&0xFF] = UKey::UK_KP9;
	m_MISC_keymap[XK_KP_Insert&0xFF] = UKey::UK_KP0;
	m_MISC_keymap[XK_KP_End&0xFF] = UKey::UK_KP1;
	m_MISC_keymap[XK_KP_Down&0xFF] = UKey::UK_KP2;
	m_MISC_keymap[XK_KP_Page_Down&0xFF] = UKey::UK_KP3;
	m_MISC_keymap[XK_KP_Left&0xFF] = UKey::UK_KP4;
	m_MISC_keymap[XK_KP_Begin&0xFF] = UKey::UK_KP5;
	m_MISC_keymap[XK_KP_Right&0xFF] = UKey::UK_KP6;
	m_MISC_keymap[XK_KP_Home&0xFF] = UKey::UK_KP7;
	m_MISC_keymap[XK_KP_Up&0xFF] = UKey::UK_KP8;
	m_MISC_keymap[XK_KP_Page_Up&0xFF] = UKey::UK_KP9;
	m_MISC_keymap[XK_KP_Delete&0xFF] = UKey::UK_KP_PERIOD;
	m_MISC_keymap[XK_KP_Decimal&0xFF] = UKey::UK_KP_PERIOD;
	m_MISC_keymap[XK_KP_Divide&0xFF] = UKey::UK_KP_DIVIDE;
	m_MISC_keymap[XK_KP_Multiply&0xFF] = UKey::UK_KP_MULTIPLY;
	m_MISC_keymap[XK_KP_Subtract&0xFF] = UKey::UK_KP_MINUS;
	m_MISC_keymap[XK_KP_Add&0xFF] = UKey::UK_KP_PLUS;
	m_MISC_keymap[XK_KP_Enter&0xFF] = UKey::UK_KP_ENTER;
	m_MISC_keymap[XK_KP_Equal&0xFF] = UKey::UK_KP_EQUALS;

	m_MISC_keymap[XK_Up&0xFF] = UKey::UK_UP;
	m_MISC_keymap[XK_Down&0xFF] = UKey::UK_DOWN;
	m_MISC_keymap[XK_Right&0xFF] = UKey::UK_RIGHT;
	m_MISC_keymap[XK_Left&0xFF] = UKey::UK_LEFT;
	m_MISC_keymap[XK_Insert&0xFF] = UKey::UK_INSERT;
	m_MISC_keymap[XK_Home&0xFF] = UKey::UK_HOME;
	m_MISC_keymap[XK_End&0xFF] = UKey::UK_END;
	m_MISC_keymap[XK_Page_Up&0xFF] = UKey::UK_PAGEUP;
	m_MISC_keymap[XK_Page_Down&0xFF] = UKey::UK_PAGEDOWN;

	m_MISC_keymap[XK_F1&0xFF] = UKey::UK_F1;
	m_MISC_keymap[XK_F2&0xFF] = UKey::UK_F2;
	m_MISC_keymap[XK_F3&0xFF] = UKey::UK_F3;
	m_MISC_keymap[XK_F4&0xFF] = UKey::UK_F4;
	m_MISC_keymap[XK_F5&0xFF] = UKey::UK_F5;
	m_MISC_keymap[XK_F6&0xFF] = UKey::UK_F6;
	m_MISC_keymap[XK_F7&0xFF] = UKey::UK_F7;
	m_MISC_keymap[XK_F8&0xFF] = UKey::UK_F8;
	m_MISC_keymap[XK_F9&0xFF] = UKey::UK_F9;
	m_MISC_keymap[XK_F10&0xFF] = UKey::UK_F10;
	m_MISC_keymap[XK_F11&0xFF] = UKey::UK_F11;
	m_MISC_keymap[XK_F12&0xFF] = UKey::UK_F12;
	m_MISC_keymap[XK_F13&0xFF] = UKey::UK_F13;
	m_MISC_keymap[XK_F14&0xFF] = UKey::UK_F14;
	m_MISC_keymap[XK_F15&0xFF] = UKey::UK_F15;
	m_MISC_keymap[XK_F16&0xFF] = UKey::UK_F16;
	m_MISC_keymap[XK_F17&0xFF] = UKey::UK_F17;
	m_MISC_keymap[XK_F18&0xFF] = UKey::UK_F18;
	m_MISC_keymap[XK_F19&0xFF] = UKey::UK_F19;
	m_MISC_keymap[XK_F20&0xFF] = UKey::UK_F20;
	m_MISC_keymap[XK_F21&0xFF] = UKey::UK_F21;
	m_MISC_keymap[XK_F22&0xFF] = UKey::UK_F22;
	m_MISC_keymap[XK_F23&0xFF] = UKey::UK_F23;
	m_MISC_keymap[XK_F24&0xFF] = UKey::UK_F24;

	m_MISC_keymap[XK_Num_Lock&0xFF] = UKey::UK_NUMLOCK;
	m_MISC_keymap[XK_Caps_Lock&0xFF] = UKey::UK_CAPSLOCK;
	m_MISC_keymap[XK_Scroll_Lock&0xFF] = UKey::UK_SCROLLOCK;
	m_MISC_keymap[XK_Shift_R&0xFF] = UKey::UK_RSHIFT;
	m_MISC_keymap[XK_Shift_L&0xFF] = UKey::UK_LSHIFT;
	m_MISC_keymap[XK_Control_R&0xFF] = UKey::UK_RCTRL;
	m_MISC_keymap[XK_Control_L&0xFF] = UKey::UK_LCTRL;
	m_MISC_keymap[XK_Alt_R&0xFF] = UKey::UK_RALT;
	m_MISC_keymap[XK_Alt_L&0xFF] = UKey::UK_LALT;
	m_MISC_keymap[XK_Meta_R&0xFF] = UKey::UK_RMETA;
	m_MISC_keymap[XK_Meta_L&0xFF] = UKey::UK_LMETA;
	m_MISC_keymap[XK_Super_L&0xFF] = UKey::UK_LSUPER; /* Left "Windows" */
	m_MISC_keymap[XK_Super_R&0xFF] = UKey::UK_RSUPER; /* Right "Windows */
	m_MISC_keymap[XK_Mode_switch&0xFF] = UKey::UK_MODE; /* "Alt Gr" key */
	m_MISC_keymap[XK_Multi_key&0xFF] = UKey::UK_COMPOSE; /* Multi-key compose */

	m_MISC_keymap[XK_Help&0xFF] = UKey::UK_HELP;
	m_MISC_keymap[XK_Print&0xFF] = UKey::UK_PRINT;
	m_MISC_keymap[XK_Sys_Req&0xFF] = UKey::UK_SYSREQ;
	m_MISC_keymap[XK_Break&0xFF] = UKey::UK_BREAK;
	m_MISC_keymap[XK_Menu&0xFF] = UKey::UK_MENU;
	m_MISC_keymap[XK_Hyper_R&0xFF] = UKey::UK_MENU;   /* Windows "Menu" key */
}

//
// X11 specific
//

UMod::Modifier
mapX11Button(int button) {
	int ret = 0;//UMod::None;

	if (button == Button1) {
		ret = UMod::LeftButton;
	} else if (button == Button2) {
		ret = UMod::MiddleButton;
	} else if (button == Button3) {
		ret = UMod::RightButton;
	}
	return UMod_t(ret);
}

UKeyCode_t
UXGLXDriver::mapX11Keycode(const XKeyEvent & xkey) {
	KeySym xsym;

	// Get the raw keyboard scancode */
	//keysym->scancode = keyCode;
	xsym = XKeycodeToKeysym(m_x11Display, xkey.keycode, 0);
//#ifdef DEBUG_KEYS
//	std::cerr << "Translating key " << xsym << "," << keyCode << std::endl;
//#endif
	// Get the translated UFO virtual keysym
	int ret = UKey::UK_UNDEFINED;

	if (xsym) {
		switch (xsym>>8) {
			case 0x1005FF:
#ifdef SunXK_F36
				if (xsym == SunXK_F36) {
					ret = UKEY::UK_F11;
				}
#endif
#ifdef SunXK_F37
				if (xsym == SunXK_F37) {
					ret = UKEY::UK_F12;
				}
#endif
				break;
			case 0x00:	// Latin 1
			case 0x01:	// Latin 2
			case 0x02:	// Latin 3
			case 0x03:	// Latin 4
			case 0x04:	// Katakana
			case 0x05:	// Arabic
			case 0x06:	// Cyrillic
			case 0x07:	// Greek
			case 0x08:	// Technical
			case 0x0A:	// Publishing
			case 0x0C:	// Hebrew
			case 0x0D:	// Thai
				ret = UKeyCode_t(xsym&0xFF);
				// Map capital letter syms to lowercase
				//if ((keysym->sym >= 'A')&&(keysym->sym <= 'Z'))
				//	keysym->sym += ('a'-'A');
				//break;
				// Map lowercase letter syms to uppercase
				if ((ret >= 'A')&&(ret <= 'Z'))
					ret += ('a'-'A');
				break;
			case 0xFE:
				ret = m_ODD_keymap[xsym&0xFF];
				break;
			case 0xFF:
				ret = m_MISC_keymap[xsym&0xFF];
				break;
			default:
				std::cerr << "X11: Unknown xsym, sym = " << xsym << std::endl;
				break;
		}
	} else {
		// X11 doesn't know how to translate the key!
		switch (xkey.keycode) {
			//These keycodes are from the Microsoft Keyboard
			case 115:
				ret = UKey::UK_LSUPER;
				break;
			case 116:
				ret = UKey::UK_RSUPER;
				break;
			case 117:
				ret = UKey::UK_MENU;
				break;
			default:
				// no point in an error message; happens for
				// several keys when we get a keymap notify
				break;
		}
	}

	return UKeyCode_t(ret);
}


wchar_t
UXGLXDriver::mapX11Unicode(const XKeyEvent & xkey) {
	XComposeStatus composeStatus;
	char asciiCode[32];
	KeySym keySym;
	int len;

	XKeyEvent key = xkey;
	len = XLookupString(&key, asciiCode, sizeof(asciiCode),
		&keySym, &composeStatus);

	// FIXME: handles only 1 and 2 byte unicode
	switch (len) {
		case 1:
			return wchar_t(asciiCode[0]);
		break;
		case 2:
		default:
			return wchar_t(asciiCode[0] << 8 || asciiCode[1]);
		break;
	}
}

UMod::Modifier
UXGLXDriver::mapX11Modifiers(int modifiers) {
	int ret = 0;

	if (modifiers & ShiftMask) {
		ret |= UMod::Shift;
	}
	if (modifiers & LockMask) {
		ret |= UMod::Caps;
	}
	if (modifiers & ControlMask) {
		ret |= UMod::Ctrl;
	}
	// FIXME: this works for me?
	if (modifiers & Mod1Mask) {
		ret |= UMod::Alt;
	}
	if (modifiers & Button1Mask) {
		ret |= UMod::LeftButton;
	}
	if (modifiers & Button2Mask) {
		ret |= UMod::MiddleButton;
	}
	if (modifiers & Button3Mask) {
		ret |= UMod::RightButton;
	}

	return UMod::Modifier(ret);
}

void
UXGLXDriver::pushXEvent(UXContext * context, const XEvent & event) {
	if (!context) {
		return;
	}
	UXFrame * frame = context->getFrame();
	UXDisplay * display = dynamic_cast<UXDisplay*>(UDisplay::getDefault());
	UVideoDevice * device = NULL;
	if (frame) {
		device = frame->getVideoDevice();
	}

	switch (event.type) {
		// Gaining mouse coverage?
		case EnterNotify: {
			if ((event.xcrossing.mode != NotifyGrab) &&
					(event.xcrossing.mode != NotifyUngrab)) {
			if (frame) {
				//frame->sigMouseFocusGained();
			}
		}
		}
		break;

	    // Losing mouse coverage?
	    case LeaveNotify: {
		if ((event.xcrossing.mode != NotifyGrab) &&
				(event.xcrossing.mode != NotifyUngrab) &&
				(event.xcrossing.detail != NotifyInferior)) {
			if (frame) {
				//frame->sigMouseFocusLost();
			}
		}
		}
		break;

		// Gaining input focus?
		case FocusIn: {
			if (frame) {
				//frame->sigIntputFocusGained();
			}
		}
		break;

		/* Losing input focus? */
		case FocusOut: {
			if (frame) {
				//frame->sigIntputFocusLost();
			}
		}
		// what's this?
		// taken from SDL sources (also in FocusIn).
		// Queue leaving fullscreen mode
		//switch_waiting = 0x01;
		//switch_time = SDL_GetTicks() + 200;
		//}
		break;

		// Generated upon EnterWindow and FocusIn
		case KeymapNotify: {
			//setKeyboardState(m_display, event.xkeymap.key_vector);
		}
		break;

		// Mouse motion?
		case MotionNotify: {
			display->pushMouseMove(
				context,
				//mapX11Modifiers(event.xmotion.state),
				event.xmotion.x,
				event.xmotion.y
			);
		}
		break;

		// Mouse button press?
		case ButtonPress: {
			if (event.xbutton.button == 4) {
				// mouse wheel up
				display->pushMouseWheelUp(context,
					//mapX11Modifiers(event.xmotion.state),
					event.xbutton.x, event.xbutton.y);
			} else if (event.xbutton.button == 5) {
				// mouse wheel up
				display->pushMouseWheelDown(context,
					//mapX11Modifiers(event.xmotion.state),
					event.xbutton.x, event.xbutton.y);
			} else {
				display->pushMouseButtonDown(
					context,
					//mapX11Modifiers(event.xmotion.state),
					event.xbutton.x,
					event.xbutton.y,
					mapX11Button(event.xbutton.button)
				);
			}
		}
		break;

		// Mouse button release?
		case ButtonRelease: {
			if (event.xbutton.button == 4) {
			} else if (event.xbutton.button == 5) {
			} else {
				display->pushMouseButtonUp(
					context,
					//mapX11Modifiers(event.xmotion.state),
					event.xbutton.x,
					event.xbutton.y,
					mapX11Button(event.xbutton.button)
				);
			}
		}
		break;

		// Key press?
		case KeyPress: {
		//std::cerr << "KeyPress (X11 keycode = " << uint32_t(event.xkey.keycode) << "\n";
			display->pushKeyDown(context,
				//mapX11Modifiers(event.xmotion.state),
				mapX11Keycode(event.xkey), mapX11Unicode(event.xkey));
		}
		break;

		// Key release?
		case KeyRelease: {
		//std::cerr << "KeyRelease (X11 keycode = " << uint32_t(event.xkey.keycode) << "\n";
			display->pushKeyUp(context,
				//mapX11Modifiers(event.xmotion.state),
				mapX11Keycode(event.xkey), mapX11Unicode(event.xkey));
		// Check to see if this is a repeated key */
		//if ( ! X11_KeyRepeat(SDL_Display, &event) ) {
		}
		break;

		// Have we been iconified?
		case UnmapNotify: {
		/* If we're active, make ourselves inactive
		//if ( SDL_GetAppState() & SDL_APPACTIVE ) {
			// Swap out the gamma before we go inactive
			X11_SwapVidModeGamma(this);

			// Send an internal deactivate event
			posted = SDL_PrivateAppActive(0,
					SDL_APPACTIVE|SDL_APPINPUTFOCUS);
		}*/
		}
		break;

	    /* Have we been restored? */
	    case MapNotify: {
		/* If we're not active, make ourselves active
		if ( !(SDL_GetAppState() & SDL_APPACTIVE) ) {
			// Send an internal activate event
			posted = SDL_PrivateAppActive(1, SDL_APPACTIVE);

			// Now that we're active, swap the gamma back
			X11_SwapVidModeGamma(this);
		}

		if ( SDL_VideoSurface &&
		     (SDL_VideoSurface->flags & SDL_FULLSCREEN) ) {
			X11_EnterFullScreen(this);
		} else {
			X11_GrabInputNoLock(this, this->input_grab);
		}
		X11_CheckMouseModeNoLock(this);

		if ( SDL_VideoSurface ) {
			X11_RefreshDisplay(this);
		}
		*/
			if (frame) {
				frame->getRootPane()->repaint();
			}
		}
		break;

		/* Have we been resized or moved? */
		case ConfigureNotify: {
		/*if ( SDL_VideoSurface ) {
		    if ((event.xconfigure.width != SDL_VideoSurface->w) ||
		        (event.xconfigure.height != SDL_VideoSurface->h)) {
			// FIXME: Find a better fix for the bug with KDE 1.2
			if ( ! ((event.xconfigure.width == 32) &&
			        (event.xconfigure.height == 32)) ) {
				SDL_PrivateResize(event.xconfigure.width,
				                  event.xconfigure.height);
			}
		    } else {
			// OpenGL windows need to know about the change
			if ( SDL_VideoSurface->flags & SDL_OPENGL ) {
				SDL_PrivateExpose();
			}
		    }
		}*/
			if ((event.xconfigure.width != frame->getSize().w) ||
					(event.xconfigure.height != frame->getSize().h)) {
				device->sigResized().emit(device);
			}
			if ((event.xconfigure.x != frame->getLocation().x) ||
					(event.xconfigure.y != frame->getLocation().y)) {
				device->sigMoved().emit(device);
			}
		}
		break;

		// Have we been requested to quit (or another client message?)
		case ClientMessage: {
		if ((event.xclient.format == 32) &&
				(event.xclient.data.l[0] == m_deleteWindow)) {
			int visFrames = 0;
			std::vector<UXFrame*> frames = m_display->getFrames();
			for (int i = 0; i < frames.size(); ++i) {
				if (frames[i]->isVisible()) {
					visFrames++;
				}
			}
			if (visFrames == 1) {
				m_display->pushEvent(new UQuitEvent());
			}
			if (frame) {
				frame->setVisible(false);
			}
		}
		}
		break;

		// Do we need to refresh ourselves?
		case Expose: {
			if (frame) {
				frame->getRootPane()->repaint();
			}
		/*if ( SDL_VideoSurface && (event.xexpose.count == 0) ) {
			X11_RefreshDisplay(this);
		}*/
		}
		break;

	    default: {
		// Only post the event if we're watching for it
		/*
		if ( SDL_ProcessEvents[SDL_SYSWMEVENT] == SDL_ENABLE ) {
			SDL_SysWMmsg wmmsg;

			SDL_VERSION(&wmmsg.version);
			wmmsg.subsystem = SDL_SYSWM_X11;
			wmmsg.event.event = event;
			posted = SDL_PrivateSysWMEvent(&wmmsg);
		}*/
	    }
	    break;
	}
}



class UGLXVideoPlugin : public UVideoPlugin {
	UFO_DECLARE_DYNAMIC_CLASS(UGLXVideoPlugin)
public:
	UGLXVideoPlugin() : m_glxdriver(NULL) {}
	virtual ~UGLXVideoPlugin() {
		if (m_glxdriver) {
			delete (m_glxdriver);
			m_glxdriver = NULL;
		}
	}
	virtual bool isAvailable() {
		Display *display = display = XOpenDisplay(NULL);
		if (display != NULL) {
			XCloseDisplay(display);
		}
		return(display != NULL);
	}
	virtual UVideoDriver * createVideoDriver() {
		if (!m_glxdriver) {
			m_glxdriver = new UXGLXDriver();
		}
		return m_glxdriver;
	}
private:
	UXGLXDriver * m_glxdriver;
};
UFO_IMPLEMENT_DYNAMIC_CLASS(UGLXVideoPlugin, UVideoPlugin)

UPluginBase *
UXGLXDriver::createPlugin() {
	return new UGLXVideoPlugin();
}

void
UXGLXDriver::destroyPlugin(UPluginBase * plugin) {
	delete (plugin);
}



//
// class UXGLXDevice
//
//

UXGLXDevice::UXGLXDevice(UXGLXDriver * driver)
	: m_glxDriver(driver)
	, m_window(0)
	, m_glContext(NULL)
	, m_frame(NULL)
	, m_size()
	, m_pos()
	, m_isVisible(false)
	, m_frameStyle(0)
	, m_depth(0)
{
	//m_attributes[GLX_INDEX] = 0;

	m_attributes[GLX_RED_SIZE] = 5;
	m_attributes[GLX_GREEN_SIZE] = 5;
	m_attributes[GLX_BLUE_SIZE] = 5;
	m_attributes[GLX_ALPHA_SIZE] = 0;

	m_attributes[GLX_DOUBLEBUFFER] = 1;
	m_attributes[GLX_STEREO] = 0;

	m_attributes[GLX_DEPTH_SIZE] = 16;

	m_attributes[GLX_STENCIL_SIZE] = 0;//1;

	m_attributes[GLX_ACCUM_RED_SIZE] = 0;
	m_attributes[GLX_ACCUM_GREEN_SIZE] = 0;
	m_attributes[GLX_ACCUM_BLUE_SIZE] = 0;
	m_attributes[GLX_ACCUM_ALPHA_SIZE] = 0;
}

UXFrame *
UXGLXDevice::getFrame() const {
	return m_frame;
}

void
UXGLXDevice::setFrame(UXFrame * frame) {
	m_frame = frame;
}


void
UXGLXDevice::setSize(int w, int h) {
	UDimension size(w, h);
	if (size == m_size) {
		return;
	}
	m_size = size;
	if (m_isVisible) {
		XSizeHints sizeHints;
		if (m_frameStyle & FrameResizable) {
			sizeHints.min_width = 32;
			sizeHints.min_height = 32;
			sizeHints.max_height = 4096;
			sizeHints.max_width = 4096;
		} else {
			sizeHints.min_width = sizeHints.max_width = m_size.w;
			sizeHints.min_height = sizeHints.max_height = m_size.h;
		}
		XSetWMNormalHints(m_glxDriver->getX11Display(), m_window, &sizeHints);
		XResizeWindow(m_glxDriver->getX11Display(), m_window, w, h);
		// FIXME: is flush here necessary?
		//XFlush(m_glxDriver->getX11Display());
	}
}

UDimension
UXGLXDevice::getSize() const {
	return m_size;
}

void
UXGLXDevice::setLocation(int x, int y) {
	UPoint pos(x, y);
	if (m_pos == pos) {
		return;
	}
	m_pos = pos;
	if (m_isVisible) {
		XMoveWindow(m_glxDriver->getX11Display(), m_window, x, y);
	}
}


UPoint
UXGLXDevice::getLocation() const {
	return m_pos;
}


void
UXGLXDevice::setTitle(const std::string & title) {
	m_title = title;
	if (m_isVisible) {
		XTextProperty textProperty;
		// Prepare the window and iconified window names...
		const char * title = m_title.c_str();
		XStringListToTextProperty((char **) &title, 1, &textProperty);

		XSetWMName(m_glxDriver->getX11Display(), m_window, &textProperty);
		XSetWMIconName(m_glxDriver->getX11Display(), m_window, &textProperty);
	}
}

std::string
UXGLXDevice::getTitle() const {
	return m_title;
}


void
UXGLXDevice::setDepth(int depth) {
	m_depth = depth;
}

int
UXGLXDevice::getDepth() const {
	return m_depth;
}


void
UXGLXDevice::swapBuffers() {
	//m_sdldriver->SDL_GL_SwapBuffers();
	m_glxDriver->glXSwapBuffers(m_glxDriver->getX11Display(), m_window);
}

void
UXGLXDevice::makeContextCurrent() {
	m_glxDriver->glXMakeCurrent(m_glxDriver->getX11Display(), m_window, m_glContext);
}

bool
UXGLXDevice::show() {
	// Here we are upon the stage. Have the visual selected.
	XVisualInfo * visualinfo = chooseVisual();

	XSetWindowAttributes winAttr;
	XSizeHints sizeHints;
	uint32_t mask;

	// Have the windows attributes set
	//
	// HINT: the masks should be updated when adding/removing callbacks.
	//       This might speed up message processing. Is that true?
	winAttr.event_mask = StructureNotifyMask | SubstructureNotifyMask | ExposureMask |
		ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask |
		VisibilityChangeMask | EnterWindowMask | LeaveWindowMask |
		PointerMotionMask | ButtonMotionMask;

	winAttr.background_pixmap = None;
	winAttr.background_pixel  = 0;
	winAttr.border_pixel      = 0;

	// The color map is required, too
	winAttr.colormap = XCreateColormap(
		m_glxDriver->getX11Display(), m_glxDriver->getRootWindow(),
		visualinfo->visual, AllocNone
	);

	// This tells the XCreateWindow() what attributes are we supplying it with
	mask = CWBackPixmap | CWBorderPixel | CWColormap | CWEventMask;

	// Have the window created now
	m_window = XCreateWindow(
		m_glxDriver->getX11Display(),
		//m_parent == NULL ? m_glxDriver->getRootWindow() : m_parent->m_window,
		m_glxDriver->getRootWindow(),
		m_pos.x, m_pos.y, m_size.w, m_size.h, 0,
		visualinfo->depth, InputOutput,
		visualinfo->visual, mask,
		&winAttr
	);

	// The GLX context creation, possibly trying the direct context rendering
	m_glContext = m_glxDriver->glXCreateContext(
		m_glxDriver->getX11Display(),
		visualinfo,
		//m_parent == NULL ? NULL : m_parent->m_glContext,
		NULL,
		true
	);

	// Set the new context as the current one. That's all about the window creation.
	m_glxDriver->glXMakeCurrent(
		m_glxDriver->getX11Display(),
		m_window,
		m_glContext
	);

	// Assume the new window is visible by default
	m_isVisible = true;
/*
	// We can have forced all new windows start in iconified state:
	wmHints.flags = StateHint;
	wmHints.initial_state = NormalState;
	//(fgState.ForceIconic == FALSE) ? NormalState : IconicState;

	// Prepare the window and iconified window names...
	const char * title = m_title.c_str();
	XStringListToTextProperty( (char **) &title, 1, &textProperty );

	// Set the window's properties now
	XSetWMProperties(
		m_glxDriver->getX11Display(),
		m_window,
		&textProperty,
		&textProperty,
		0,
		0,
		&sizeHints,
		&wmHints,
		NULL
	);
	*/

	/*
	XSetStandardProperties(
		m_display->getX11Display(),
		m_window,
		titleA.c_str(),
		titleA.c_str(),
		None,
		NULL,
		0,
		NULL
	);*/

	setSizeHints();
	setWMHints();
	setTitle(m_title);
	setDecorations();

	// Make sure we are informed about the window deletion commands
	// FIXME window deletion command to be implemented
	XSetWMProtocols(m_glxDriver->getX11Display(), m_window,
		m_glxDriver->getDeleteWindowAtom(), 1);

	// Finally, have the window mapped to our display
	XMapWindow(m_glxDriver->getX11Display(), m_window);


	// And finally, raise the window
	//XMapRaised(m_glxDriver->getX11Display(), m_window);

	m_isVisible = true;
}

void
UXGLXDevice::hide() {
	// As easy as kill bunnies with axes. Destroy the context first:
	// nock: crazy freeglut people ;-)
	m_glxDriver->glXDestroyContext(m_glxDriver->getX11Display(), m_glContext);
	m_glContext = NULL;

	// Then have the window killed:
	XDestroyWindow(m_glxDriver->getX11Display(), m_window);
	m_window = 0;

	// Finally, flush the rests down the stream
	XFlush(m_glxDriver->getX11Display());

	m_isVisible = false;
}


//
// FIXME: whoops, a hack
// taken gratefully from the gdk sources: www.gtk.org

typedef struct {
unsigned long flags;
unsigned long functions;
unsigned long decorations;
long input_mode;
unsigned long status;
} ufo_MotifWmHints;

#define MWM_HINTS_DECORATIONS   (1L << 1)
typedef enum
{
GDK_DECOR_ALL		= 1 << 0,
GDK_DECOR_BORDER	= 1 << 1,
GDK_DECOR_RESIZEH	= 1 << 2,
GDK_DECOR_TITLE	= 1 << 3,
GDK_DECOR_MENU	= 1 << 4,
GDK_DECOR_MINIMIZE	= 1 << 5,
GDK_DECOR_MAXIMIZE	= 1 << 6
} ufo_decoration;

void
UXGLXDevice::setDecorations() {
	ufo_MotifWmHints hints;
	hints.flags = MWM_HINTS_DECORATIONS;
	if (m_frameStyle == 0) {
		hints.decorations = GDK_DECOR_ALL;
	}
	if (m_frameStyle & FrameNormalBorder) {
		hints.decorations |= GDK_DECOR_BORDER;
	}
	if (m_frameStyle & FrameTitleBar) {
		hints.decorations |= GDK_DECOR_TITLE;
	}
	if (m_frameStyle & FrameSysMenu) {
		hints.decorations |= GDK_DECOR_MENU;
	}
	if (m_frameStyle & FrameMinimizeBox) {
		hints.decorations |= GDK_DECOR_MINIMIZE;
	}
	if (m_frameStyle & FrameMaximizeBox) {
		hints.decorations |= GDK_DECOR_MAXIMIZE;
	}
	if (m_frameStyle & FrameNoBorder) {
		hints.decorations = 0;
	}

	Atom hints_atom = XInternAtom
		(m_glxDriver->getX11Display(), "_MOTIF_WM_HINTS", false);

	XChangeProperty (m_glxDriver->getX11Display(), m_window,
		hints_atom, hints_atom, 32, PropModeReplace,
		(uint8_t *)&hints, sizeof (ufo_MotifWmHints)/sizeof (long));
}

void
UXGLXDevice::setSizeHints() {
	XSizeHints *hints;

	hints = XAllocSizeHints();
	if (hints) {
		if (m_frameStyle & FrameResizable) {
			hints->min_width = 32;
			hints->min_height = 32;
			hints->max_height = 4096;
			hints->max_width = 4096;
		} else {
			hints->min_width = hints->max_width = m_size.w;
			hints->min_height = hints->max_height = m_size.h;
		}
		hints->flags = PMaxSize | PMinSize;
		/*
		if (m_frameStyle & FrameFullScreen) {
			hints->x = 0;
			hints->y = 0;
			hints->flags |= USPosition;
		} else {
			hints->x = m_pos.x;
			hints->y = m_pos.y;
		}
		*/
		XSetWMNormalHints(m_glxDriver->getX11Display(), m_window, hints);
		XFree(hints);
	}
/*

	// For the position and size hints -- make sure we are passing valid values
	sizeHints.flags = 0;

	//sizeHints.flags |= (fgState.Position.Use == TRUE) ? USPosition : PPosition;
	//sizeHints.flags |= (fgState.Size.Use     == TRUE) ? USSize     : PSize;
	sizeHints.flags |= USPosition;
	sizeHints.flags |= USSize;

	// Fill in the size hints values now (the x, y, width and height
	// settings are obsolote, are there any more WMs that support them?)
	sizeHints.x      = m_pos.x;
	sizeHints.y      = m_pos.y;
	sizeHints.width  = m_size.w;
	sizeHints.height = m_size.h;

	if (frameStyle & FrameResizable) {
		sizeHints.min_width = 32;
		sizeHints.min_height = 32;
		sizeHints.max_height = 4096;
		sizeHints.max_width = 4096;
	} else {
		sizeHints.min_width = sizeHints.max_width = m_size.w;
		sizeHints.min_height = sizeHints.max_height = m_size.h;
	}
*/
}

void
UXGLXDevice::setWMHints() {
	Display * display = m_glxDriver->getX11Display();
	XWMHints * wmHints = XAllocWMHints();
	wmHints->flags = StateHint | InputHint;
	wmHints->input = True;

	if (m_frameStyle & FrameMinimized) {
		wmHints->initial_state = IconicState;
    } else {
		wmHints->initial_state = NormalState;
	}
	XSetWMHints(display, m_window, wmHints);

	XFree(wmHints);

	// using the new _net spec for other hints ...

	Atom atoms[7];
	int i = 0;


	if (m_frameStyle & FrameMaximized) {
		atoms[i] = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", false);
		++i;
		atoms[i] = XInternAtom (display, "_NET_WM_STATE_MAXIMIZED_HORZ", false);
		++i;
	}

	// ignoring
	// "_NET_WM_STATE_ABOVE"
    // "_NET_WM_STATE_BELOW"

	if (m_frameStyle & FrameSticky) {
		atoms[i] = XInternAtom (display, "_NET_WM_STATE_STICKY", false);
		++i;
	}
	if (m_frameStyle & FrameFullScreen) {
		atoms[i] = XInternAtom (display, "_NET_WM_STATE_FULLSCREEN", false);
		++i;
	}
	if (m_frameStyle & FrameModal) {
		atoms[i] = XInternAtom (display, "_NET_WM_STATE_MODAL", false);
		++i;
	}
	if (m_frameStyle & FrameSkipTaskBar) {
		atoms[i] = XInternAtom (display, "_NET_WM_STATE_SKIP_TASKBAR", false);
		++i;
	}

	// ignore
	// "_NET_WM_STATE_SKIP_PAGER"

	if (i > 0) {
		XChangeProperty (display, m_window,
			XInternAtom(display, "_NET_WM_STATE", false),
			XA_ATOM, 32, PropModeReplace,
			(uint8_t*) atoms, i);
	} else {
		XDeleteProperty (display, m_window,
			XInternAtom(display, "_NET_WM_STATE", false)
		);
    }
	// FIXME what's this
	if (m_frameStyle & FrameSticky) {
		atoms[0] = 0xFFFFFFFF;
		XChangeProperty (display, m_window,
			XInternAtom(display, "_NET_WM_DESKTOP", false),
			XA_CARDINAL, 32, PropModeReplace,
			(uint8_t*) atoms, 1);
	} else {
		XDeleteProperty (display, m_window,
			XInternAtom(display, "_NET_WM_DESKTOP", false)
		);
	}
}


void
UXGLXDevice::setFrameStyle(uint32_t frameStyle) {
	m_frameStyle = frameStyle;
}
uint32_t
UXGLXDevice::getFrameStyle() const {
	return m_frameStyle;
}

void
UXGLXDevice::setInitialFrameState(uint32_t frameState) {
	m_frameState = frameState;
}
uint32_t
UXGLXDevice::getFrameState() const {
	return m_frameState;
}

/*
bool
UXGLXDevice::toggleFrameState(uint32_t frameState) {
	if (!m_isVisible) {
		return false;
	}
	bool ret = false;
	Display * display = m_glxDriver->getX11Display();

	bool add = true;
	if (m_frameState & frameState) {
		add = false;
	}
	Atom atoms[4];
	for (int i = 0; i < 4; ++i) { atoms[i] = 0; }
	int i = 0;
	// check for _net state
#define TOGGLE(attr) \
if (m_frameState & attr) { \
	m_frameState &= ~attr; \
} else { \
	m_frameState |= attr; \
}
	if (frameState & FrameMinimized) {
		TOGGLE(FrameMinimized)
		if (m_frameState & FrameMinimized) {
			ret = XIconifyWindow(display, m_window, DefaultScreen(display));
		} else {
			XMapWindow (display, m_window);
		}
	}
	if (frameState & FrameMaximized) {
		TOGGLE(FrameMaximized)
		atoms[i] = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", false);
		i++;
		atoms[i] = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", false);
		i++;
		ret = true;
	}
	if (frameState & FrameModal) {
		TOGGLE(FrameModal)
		atoms[i] = XInternAtom(display, "_NET_WM_STATE_MODAL", false);
		i++;
		ret = true;
	}
	if (frameState & FrameSkipTaskBar) {
		TOGGLE(FrameSkipTaskBar)
		atoms[i] = XInternAtom(display, "_NET_WM_STATE_SKIP_TASKBAR", false);
		i++;
		ret = true;
	}
	if (frameState & FrameFullScreen && i < 4) {
		TOGGLE(FrameFullScreen)
		atoms[i] = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", false);
		i++;
		ret = true;
	}

// ignoring
//_NET_WM_STATE_SKIP_PAGER
//_NET_WM_STATE_BELOW
//_NET_WM_STATE_ABOVE

	XEvent xev;

#define _NET_WM_STATE_REMOVE        0    // remove/unset property
#define _NET_WM_STATE_ADD           1    // add/set property
#define _NET_WM_STATE_TOGGLE        2    // toggle property

	xev.xclient.type = ClientMessage;
	xev.xclient.serial = 0;
	xev.xclient.send_event = True;
	xev.xclient.window = m_window;
	xev.xclient.message_type = XInternAtom(display, "_NET_WM_STATE", false);
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = _NET_WM_STATE_TOGGLE;
	xev.xclient.data.l[1] = atoms[0];
	xev.xclient.data.l[2] = atoms[1];
	xev.xclient.data.l[3] = atoms[2];
	xev.xclient.data.l[4] = atoms[3];

	XSendEvent (display, m_window, False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		&xev);

	return ret;
}
*/

XVisualInfo *
UXGLXDevice::chooseVisual() {
	int bufferSize[] = {16, 12, 8, 4, 2, 1};
	bool wantIndexedMode = false;
	int attributes[32];
	int where = 0;

	// First we have to process the display mode settings...
#   define ATTRIB(a) attributes[where++]=a;

	// Decide if we want a true or indexed color visual:
	// FIXME: What about indexed modes
	if (true) {//getAttribute(GLX_INDEX) == 0) {

		// We are sure that there will be R, B and B components requested:
		ATTRIB(GLX_RGBA);
		ATTRIB(GLX_RED_SIZE); ATTRIB(getAttribute(GLX_RED_SIZE));
		ATTRIB(GLX_GREEN_SIZE); ATTRIB(getAttribute(GLX_GREEN_SIZE));
		ATTRIB(GLX_BLUE_SIZE); ATTRIB(getAttribute(GLX_BLUE_SIZE));

		// Check if the A component is required, too:
		if (getAttribute(GLX_ALPHA_SIZE)) {
			ATTRIB(GLX_ALPHA_SIZE); ATTRIB(getAttribute(GLX_ALPHA_SIZE));
		}
	} else {
		// We've got an indexed color request
		ATTRIB(GLX_BUFFER_SIZE); ATTRIB(8);

		// Set the 'I want indexed mode' switch
		wantIndexedMode = true;
	}

	// We can have double or single buffered contexts created
	if (getAttribute(GLX_DOUBLEBUFFER)) {
		ATTRIB(GLX_DOUBLEBUFFER);
	}

	// Stereoscopy seems a nice thing to have
	if (getAttribute(GLX_STEREO)) {
		ATTRIB(GLX_STEREO);
	}

	// Depth buffer is almost always required
	if (getAttribute(GLX_DEPTH_SIZE)) {
		ATTRIB(GLX_DEPTH_SIZE); ATTRIB(getAttribute(GLX_DEPTH_SIZE));
	}

	// Stenciling support
	if (getAttribute(GLX_STENCIL_SIZE)) {
		ATTRIB(GLX_STENCIL_SIZE); ATTRIB(getAttribute(GLX_STENCIL_SIZE));
	}

	// And finally the accumulation buffers
	//if (getAttribute(GLX_ACCUM)) {
		ATTRIB(GLX_ACCUM_RED_SIZE);
		ATTRIB(getAttribute(GLX_ACCUM_RED_SIZE));

		ATTRIB(GLX_ACCUM_GREEN_SIZE);
		ATTRIB(getAttribute(GLX_ACCUM_GREEN_SIZE));

		ATTRIB(GLX_ACCUM_BLUE_SIZE);
		ATTRIB(getAttribute(GLX_ACCUM_BLUE_SIZE));

		// Check if the A component is required, too:
		if(getAttribute(GLX_ALPHA_SIZE)) {
			ATTRIB(GLX_ACCUM_ALPHA_SIZE);
			ATTRIB(getAttribute(GLX_ALPHA_SIZE));
		}
	//}

	// Push a null at the end of the list
	ATTRIB(None);

	//
	// OKi now, we've got two cases -- RGB(A) and index mode visuals
	//
	if(wantIndexedMode == false) {
		XVisualInfo* visualInfo = NULL;
		// The easier one. And more common, too.
		visualInfo = m_glxDriver->glXChooseVisual(
			m_glxDriver->getX11Display(),
			DefaultScreen(m_glxDriver->getX11Display()),
			attributes
		);
		if (visualInfo == NULL) {
			std::cerr << "Whee!\n"
			<< "  display() " << m_glxDriver->getX11Display()
			<< "  screen " << DefaultScreen(m_glxDriver->getX11Display())
			<< "\n";
		}
		return visualInfo;

	} else {
		XVisualInfo* visualInfo = NULL;

		// In indexed mode, we need to check how many bits of depth can we achieve
		for(int i=0; i<6; i++) {
			// The GLX_BUFFER_SIZE value comes always first, so:
			attributes[1] = bufferSize[i];

			//* Check if such visual is possible
			visualInfo = m_glxDriver->glXChooseVisual(
				m_glxDriver->getX11Display(),
				DefaultScreen(m_glxDriver->getX11Display()),
				attributes
			);

			// The buffer size are sorted in descendant order, so choose the first:
			if(visualInfo != NULL) {
				return(visualInfo);
			}
		}
	}

	// If we are still here, it means that the visual info was not found
	std::cerr << " :: WARNING: Couldn't find matching GLX visual\n";
	return(NULL);
}

int
UXGLXDevice::getAttribute(int key) {
	return m_attributes[key];
}
void
UXGLXDevice::setAttribute(int key, int value) {
	m_attributes[key] = value;
}

#endif // UFO_USE_GLX
