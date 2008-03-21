/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ux/uxwgldriver.cpp
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

#include "ufo/config/ufo_config.hpp"

#ifdef  UFO_USE_WGL

#include "ufo/ux/uxwgldriver.hpp"

#include "ufo/events/ukeysym.hpp"

#include "ufo/ux/uxcontext.hpp"
#include "ufo/ux/uxdisplay.hpp"
#include "ufo/ux/uxframe.hpp"

#include "ufo/events/uquitevent.hpp"
#include "ufo/widgets/urootpane.hpp"
#include "ufo/uplugin.hpp"
#include "ufo/usharedlib.hpp"

#include "ufo/gl/ugl_driver.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UXWGLDevice, UVideoDevice)
UFO_IMPLEMENT_DYNAMIC_CLASS(UXWGLDriver, UVideoDriver)

// FIXME:
//

#include <assert.h>

#define REPEATED_KEYMASK	(1<<30)
#define EXTENDED_KEYMASK	(1<<24)

#ifndef VK_0
#define VK_0	'0'
#define VK_1	'1'
#define VK_2	'2'
#define VK_3	'3'
#define VK_4	'4'
#define VK_5	'5'
#define VK_6	'6'
#define VK_7	'7'
#define VK_8	'8'
#define VK_9	'9'
#define VK_A	'A'
#define VK_B	'B'
#define VK_C	'C'
#define VK_D	'D'
#define VK_E	'E'
#define VK_F	'F'
#define VK_G	'G'
#define VK_H	'H'
#define VK_I	'I'
#define VK_J	'J'
#define VK_K	'K'
#define VK_L	'L'
#define VK_M	'M'
#define VK_N	'N'
#define VK_O	'O'
#define VK_P	'P'
#define VK_Q	'Q'
#define VK_R	'R'
#define VK_S	'S'
#define VK_T	'T'
#define VK_U	'U'
#define VK_V	'V'
#define VK_W	'W'
#define VK_X	'X'
#define VK_Y	'Y'
#define VK_Z	'Z'
#endif // VK_0

// These keys haven't been defined, but were experimentally determined
#define VK_SEMICOLON	0xBA
#define VK_EQUALS	0xBB
#define VK_COMMA	0xBC
#define VK_MINUS	0xBD
#define VK_PERIOD	0xBE
#define VK_SLASH	0xBF
#define VK_GRAVE	0xC0
#define VK_LBRACKET	0xDB
#define VK_BACKSLASH	0xDC
#define VK_RBRACKET	0xDD
#define VK_APOSTROPHE	0xDE
#define VK_BACKTICK	0xDF


//std::vector<UXWGLDevice*> m_windowMap;
//UKeyCode_t UXWGLDriver::VK_keymap[256];

static UXWGLDriver * s_instance = NULL;

UXWGLDriver::UXWGLDriver()
	: m_isValid(false)
	, m_isInit(false)
	, m_createdGLDriver(false)
	, m_display(NULL)
	, m_instance(NULL)
{
	m_display = dynamic_cast<UXDisplay*>(UDisplay::getDefault());
	s_instance = this;
}

UXWGLDriver::~UXWGLDriver() {
	s_instance = NULL;
}

LRESULT CALLBACK
localWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (s_instance) {
		return s_instance->windowProc(hWnd, uMsg, wParam, lParam);
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool
UXWGLDriver::init() {
	// Load OpenGL driver
	if (!ugl_driver) {
		ugl_driver = new UGL_Driver("");
		m_createdGLDriver = true;
	}

#define UFO_WGL_PROC(ret,func,params) \
{ \
	func = (ret (WINAPI *)params)(ugl_driver->getProcAddress(#func)); \
	if (!func) { \
		std::cerr << "Couldn't load WGL function: " << #func << "\n"; \
		m_isValid = false; \
	} \
}
UFO_WGL_PROC(BOOL,wglMakeCurrent,(HDC, HGLRC))
UFO_WGL_PROC(HGLRC,wglCreateContext,(HDC))
UFO_WGL_PROC(BOOL,wglDeleteContext,(HGLRC))
UFO_WGL_PROC(BOOL,wglShareLists,(HGLRC, HGLRC))
#undef UFO_WGL_PROC

	WNDCLASS wc;
	ATOM atom;

	// What we need to do is to initialize the fgDisplay global structure here.
	m_instance = GetModuleHandle(NULL);

	atom = GetClassInfo(m_instance, "LIBUFO", &wc);
	if(atom == 0) {
		ZeroMemory(&wc, sizeof(WNDCLASS));

		/*
		 * Each of the windows should have its own device context, and we
		 * want redraw events during Vertical and Horizontal Resizes by
		 * the user.
		 *
		 * XXX Old code had "| CS_DBCLCKS" commented out.  Plans for the
		 * XXX future?  Dead-end idea?
		 */
		wc.style          = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc    = localWindowProc;//UXWGLDriver::windowProc;
		wc.cbClsExtra     = 0;
		wc.cbWndExtra     = 0;
		wc.hInstance      = m_instance;
		//wc.hIcon          = LoadIcon(m_instance, "GLUT_ICON");
		//wc.hIcon          = LoadImage(m_instance, "LIBUFO", IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
		if (!wc.hIcon) {
			wc.hIcon      = LoadIcon(NULL, IDI_WINLOGO);
		}

		wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground  = NULL;
		wc.lpszMenuName   = NULL;
		wc.lpszClassName  = "LIBUFO";

		/*
		* Register the window class
		*/
		atom = RegisterClass(&wc);
		assert(atom);
	}

	initKeymap();
	m_isInit = true;
	return true;
}

bool
UXWGLDriver::isInitialized() {
	return m_isInit;
}

void
UXWGLDriver::quit() {
	//XSync(m_x11Display, false);
	//XCloseDisplay(m_x11Display);
	m_isInit = false;
}

std::string
UXWGLDriver::getName() {
	return "WGL";
}

void
UXWGLDriver::pumpEvents() {
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
		// if getmessage == 0, window was closed?
		if (GetMessage(&msg, NULL, 0, 0) > 0) {
			//TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}



UVideoDevice *
UXWGLDriver::createVideoDevice() {
	UXWGLDevice * device = new UXWGLDevice(this);
	m_windowMap.push_back(device);
	return device;
}

bool
UXWGLDriver::isValid() const {
	return m_isValid;
}

HINSTANCE
UXWGLDriver::getInstance() const {
	return m_instance;
}

UMod_t
UXWGLDriver::mapWin32Button(int msg) {
	UMod_t ret = UMod::NoButton;
	switch(msg) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
			ret = UMod::LeftButton;
		break;
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
			ret = UMod::MiddleButton;
		break;
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			ret = UMod::RightButton;
		break;
		default:
		break;
	}
	return ret;
}

UXContext *
UXWGLDriver::getContextFromWindow(HWND window) {
	for (std::vector<UXWGLDevice*>::const_iterator iter = m_windowMap.begin();
			iter != m_windowMap.end(); ++iter) {
		if ((*iter)->getWindow() == window) {
			return (*iter)->getFrame()->getContext();
		}
	}
	return NULL;
}

UXWGLDevice *
UXWGLDriver::getDeviceFromWindow(HWND window) {
	for (std::vector<UXWGLDevice*>::const_iterator iter = m_windowMap.begin();
			iter != m_windowMap.end(); ++iter) {
		if ((*iter)->getWindow() == window) {
			return (*iter);
		}
	}
	return NULL;
}

void
UXWGLDriver::initKeymap() {
	/* Map the VK keysyms */
	for (int i = 0; i< 256; ++i) {
		VK_keymap[i] = UKey::UK_UNDEFINED;
	}

	VK_keymap[VK_BACK] = UKey::UK_BACKSPACE;
	VK_keymap[VK_TAB] = UKey::UK_TAB;
	VK_keymap[VK_CLEAR] = UKey::UK_CLEAR;
	VK_keymap[VK_RETURN] = UKey::UK_RETURN;
	VK_keymap[VK_PAUSE] = UKey::UK_PAUSE;
	VK_keymap[VK_ESCAPE] = UKey::UK_ESCAPE;
	VK_keymap[VK_SPACE] = UKey::UK_SPACE;
	VK_keymap[VK_APOSTROPHE] = UKey::UK_QUOTE;
	VK_keymap[VK_COMMA] = UKey::UK_COMMA;
	VK_keymap[VK_MINUS] = UKey::UK_MINUS;
	VK_keymap[VK_PERIOD] = UKey::UK_PERIOD;
	VK_keymap[VK_SLASH] = UKey::UK_SLASH;
	VK_keymap[VK_0] = UKey::UK_0;
	VK_keymap[VK_1] = UKey::UK_1;
	VK_keymap[VK_2] = UKey::UK_2;
	VK_keymap[VK_3] = UKey::UK_3;
	VK_keymap[VK_4] = UKey::UK_4;
	VK_keymap[VK_5] = UKey::UK_5;
	VK_keymap[VK_6] = UKey::UK_6;
	VK_keymap[VK_7] = UKey::UK_7;
	VK_keymap[VK_8] = UKey::UK_8;
	VK_keymap[VK_9] = UKey::UK_9;
	VK_keymap[VK_SEMICOLON] = UKey::UK_SEMICOLON;
	VK_keymap[VK_EQUALS] = UKey::UK_EQUALS;
	VK_keymap[VK_LBRACKET] = UKey::UK_LEFT_BRACKET;
	VK_keymap[VK_BACKSLASH] = UKey::UK_BACKSLASH;
	VK_keymap[VK_RBRACKET] = UKey::UK_RIGHT_BRACKET;
	VK_keymap[VK_GRAVE] = UKey::UK_BACKQUOTE;
	VK_keymap[VK_BACKTICK] = UKey::UK_BACKQUOTE;
	VK_keymap[VK_A] = UKey::UK_A;
	VK_keymap[VK_B] = UKey::UK_B;
	VK_keymap[VK_C] = UKey::UK_C;
	VK_keymap[VK_D] = UKey::UK_D;
	VK_keymap[VK_E] = UKey::UK_E;
	VK_keymap[VK_F] = UKey::UK_F;
	VK_keymap[VK_G] = UKey::UK_G;
	VK_keymap[VK_H] = UKey::UK_H;
	VK_keymap[VK_I] = UKey::UK_I;
	VK_keymap[VK_J] = UKey::UK_J;
	VK_keymap[VK_K] = UKey::UK_K;
	VK_keymap[VK_L] = UKey::UK_L;
	VK_keymap[VK_M] = UKey::UK_M;
	VK_keymap[VK_N] = UKey::UK_N;
	VK_keymap[VK_O] = UKey::UK_O;
	VK_keymap[VK_P] = UKey::UK_P;
	VK_keymap[VK_Q] = UKey::UK_Q;
	VK_keymap[VK_R] = UKey::UK_R;
	VK_keymap[VK_S] = UKey::UK_S;
	VK_keymap[VK_T] = UKey::UK_T;
	VK_keymap[VK_U] = UKey::UK_U;
	VK_keymap[VK_V] = UKey::UK_V;
	VK_keymap[VK_W] = UKey::UK_W;
	VK_keymap[VK_X] = UKey::UK_X;
	VK_keymap[VK_Y] = UKey::UK_Y;
	VK_keymap[VK_Z] = UKey::UK_Z;
	VK_keymap[VK_DELETE] = UKey::UK_DELETE;

	VK_keymap[VK_NUMPAD0] = UKey::UK_KP0;
	VK_keymap[VK_NUMPAD1] = UKey::UK_KP1;
	VK_keymap[VK_NUMPAD2] = UKey::UK_KP2;
	VK_keymap[VK_NUMPAD3] = UKey::UK_KP3;
	VK_keymap[VK_NUMPAD4] = UKey::UK_KP4;
	VK_keymap[VK_NUMPAD5] = UKey::UK_KP5;
	VK_keymap[VK_NUMPAD6] = UKey::UK_KP6;
	VK_keymap[VK_NUMPAD7] = UKey::UK_KP7;
	VK_keymap[VK_NUMPAD8] = UKey::UK_KP8;
	VK_keymap[VK_NUMPAD9] = UKey::UK_KP9;
	VK_keymap[VK_DECIMAL] = UKey::UK_KP_PERIOD;
	VK_keymap[VK_DIVIDE] = UKey::UK_KP_DIVIDE;
	VK_keymap[VK_MULTIPLY] = UKey::UK_KP_MULTIPLY;
	VK_keymap[VK_SUBTRACT] = UKey::UK_KP_MINUS;
	VK_keymap[VK_ADD] = UKey::UK_KP_PLUS;

	VK_keymap[VK_UP] = UKey::UK_UP;
	VK_keymap[VK_DOWN] = UKey::UK_DOWN;
	VK_keymap[VK_RIGHT] = UKey::UK_RIGHT;
	VK_keymap[VK_LEFT] = UKey::UK_LEFT;
	VK_keymap[VK_INSERT] = UKey::UK_INSERT;
	VK_keymap[VK_HOME] = UKey::UK_HOME;
	VK_keymap[VK_END] = UKey::UK_END;
	VK_keymap[VK_PRIOR] = UKey::UK_PAGEUP;
	VK_keymap[VK_NEXT] = UKey::UK_PAGEDOWN;

	VK_keymap[VK_F1] = UKey::UK_F1;
	VK_keymap[VK_F2] = UKey::UK_F2;
	VK_keymap[VK_F3] = UKey::UK_F3;
	VK_keymap[VK_F4] = UKey::UK_F4;
	VK_keymap[VK_F5] = UKey::UK_F5;
	VK_keymap[VK_F6] = UKey::UK_F6;
	VK_keymap[VK_F7] = UKey::UK_F7;
	VK_keymap[VK_F8] = UKey::UK_F8;
	VK_keymap[VK_F9] = UKey::UK_F9;
	VK_keymap[VK_F10] = UKey::UK_F10;
	VK_keymap[VK_F11] = UKey::UK_F11;
	VK_keymap[VK_F12] = UKey::UK_F12;
	VK_keymap[VK_F13] = UKey::UK_F13;
	VK_keymap[VK_F14] = UKey::UK_F14;
	VK_keymap[VK_F15] = UKey::UK_F15;

	VK_keymap[VK_NUMLOCK] = UKey::UK_NUMLOCK;
	VK_keymap[VK_CAPITAL] = UKey::UK_CAPSLOCK;
	VK_keymap[VK_SCROLL] = UKey::UK_SCROLLOCK;
	VK_keymap[VK_RSHIFT] = UKey::UK_RSHIFT;
	VK_keymap[VK_LSHIFT] = UKey::UK_LSHIFT;
	VK_keymap[VK_RCONTROL] = UKey::UK_RCTRL;
	VK_keymap[VK_LCONTROL] = UKey::UK_LCTRL;
	VK_keymap[VK_RMENU] = UKey::UK_RALT;
	VK_keymap[VK_LMENU] = UKey::UK_LALT;
	VK_keymap[VK_RWIN] = UKey::UK_RSUPER;
	VK_keymap[VK_LWIN] = UKey::UK_LSUPER;

	VK_keymap[VK_HELP] = UKey::UK_HELP;
#ifdef VK_PRINT
	VK_keymap[VK_PRINT] = UKey::UK_PRINT;
#endif
	VK_keymap[VK_SNAPSHOT] = UKey::UK_PRINT;
	VK_keymap[VK_CANCEL] = UKey::UK_BREAK;
	VK_keymap[VK_APPS] = UKey::UK_MENU;

}

//
// X11 specific
//
/*
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
UXWGLDriver::mapX11Keycode(const XKeyEvent & xkey) {
	KeySym xsym;

	// Get the raw keyboard scancode
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
UXWGLDriver::mapX11Unicode(const XKeyEvent & xkey) {
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
UXWGLDriver::mapX11Modifiers(int modifiers) {
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
*/

// The window procedure for handling Win32 events
LRESULT
UXWGLDriver::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	UXWGLDevice * device = getDeviceFromWindow(hWnd);
	UXContext * context = getContextFromWindow(hWnd);
	PAINTSTRUCT ps;
	LONG lRet = 1;

	if ((device == NULL) && (uMsg != WM_CREATE)) {
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	// printf ("Window %3d message <%04x> %12d %12d\n", window?window->ID:0, uMsg, wParam, lParam);
	switch(uMsg) {
	//case WM_CREATE:
	//	break;

	//case WM_ACTIVATE:
	//	break;

	//case WM_SIZE:
	//	break;

	//case WM_SETCURSOR:
	//	break;
	case WM_WINDOWPOSCHANGED: {
		RECT bounds;
		GetClientRect(hWnd, &bounds);
		ClientToScreen(hWnd, (LPPOINT)&bounds);
		ClientToScreen(hWnd, (LPPOINT)&bounds+1);
		if (bounds.left || bounds.top ) {
			device->notify(UEvent::WidgetMoved, bounds.left, bounds.top, 0, 0);
		}
		device->notify(UEvent::WidgetResized,
			bounds.right - bounds.left, bounds.bottom - bounds.top,
			0, 0);
		if (context) {
			context->setDeviceBounds(URectangle(
				bounds.left, bounds.top,
				bounds.right - bounds.left, bounds.bottom - bounds.top
			));
			context->getRootPane()->repaint();
		}
		//if ( this->input_grab != SDL_GRAB_OFF ) {
		//	ClipCursor(&SDL_bounds);
		//	}
		}
		break;

	case WM_SHOWWINDOW:
		//window->State.Visible = GL_TRUE;
		//window->State.Redisplay = GL_TRUE;
		break;

	case WM_PAINT:
		BeginPaint(hWnd, &ps);
		if (context) {
			context->getRootPane()->repaint();
		}
		EndPaint(hWnd, &ps);
		break;

	case WM_CLOSE: {
		int visFrames = 0;
		std::vector<UXFrame*> frames = m_display->getFrames();
		for (int i = 0; i < frames.size(); ++i) {
			if (frames[i]->isVisible()) {
				visFrames++;
			}
		}
		if (visFrames == 1) {
			// FIXME: What is the quit message?
			m_display->pushEvent(new UQuitEvent());
			PostQuitMessage(0);
		}
		if (context) {
			context->getFrame()->setVisible(false);
		}
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;;

	case WM_MOUSEMOVE: {
			m_display->pushMouseMove(
				context,
				LOWORD(lParam),
				HIWORD(lParam)
			);
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		m_display->pushMouseButtonDown(
				context,
				LOWORD(lParam),
				HIWORD(lParam),
				mapWin32Button(uMsg)
		);
		break;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		m_display->pushMouseButtonUp(
				context,
				LOWORD(lParam),
				HIWORD(lParam),
				mapWin32Button(uMsg)
		);
		break;

	case 0x020a: // WM_MOUSEWHEEL: // ms vc6 does not know mouse wheels
		{
			int mouse_x = 0;
			int mouse_y = 0;
			m_display->getMouseState(&mouse_x, &mouse_y);
			/*
			 * XXX THIS IS SPECULATIVE -- John Fay, 10/2/03
			 * XXX Should use WHEEL_DELTA instead of 120
			 */
			int wheel_number = LOWORD(wParam);
			short ticks = (short)HIWORD(wParam) / 120;

			if (ticks > 0) {
				while (ticks--) {
					m_display->pushMouseWheelUp(context,
						mouse_x, mouse_y);
				}
			} else {
				while (ticks++) {
					m_display->pushMouseWheelDown(context,
						mouse_x, mouse_y);
				}
			}
		}
		break ;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN: {
		// Ignore repeated keys */
		//if (lParam&REPEATED_KEYMASK) {
		//	return(0);
		//}
		switch (wParam) {
			case VK_CONTROL:
				if ( lParam&EXTENDED_KEYMASK )
					wParam = VK_RCONTROL;
				else
					wParam = VK_LCONTROL;
				break;
			case VK_SHIFT:
				/* EXTENDED trick doesn't work here */
				/*
				if (!prev_shiftstates[0] && (GetKeyState(VK_LSHIFT) & 0x8000)) {
					wParam = VK_LSHIFT;
					prev_shiftstates[0] = TRUE;
				} else if (!prev_shiftstates[1] && (GetKeyState(VK_RSHIFT) & 0x8000)) {
					wParam = VK_RSHIFT;
					prev_shiftstates[1] = TRUE;
				} else {
					// Huh?
				}
				*/
				break;
			case VK_MENU:
				if (lParam&EXTENDED_KEYMASK)
					wParam = VK_RMENU;
				else
					wParam = VK_LMENU;
				break;
		}
		//
		UKeyCode_t keycode = VK_keymap[wParam];
		wchar_t unicode = 0;
		BYTE keystate[256];
		BYTE chars[2];

		GetKeyboardState(keystate);
		if (ToAscii(wParam, HIWORD(lParam), keystate, (WORD *)chars, 0) == 1) {
			unicode = chars[0];
		}
		m_display->pushKeyDown(context, keycode, unicode);
		}
		break;

	case WM_SYSKEYUP:
	case WM_KEYUP: {
		// Ignore repeated keys */
		//if (lParam&REPEATED_KEYMASK) {
		//	return(0);
		//}
		switch (wParam) {
			case VK_CONTROL:
				if ( lParam&EXTENDED_KEYMASK )
					wParam = VK_RCONTROL;
				else
					wParam = VK_LCONTROL;
				break;
			case VK_SHIFT:
				/* EXTENDED trick doesn't work here */
				/*
				if (!prev_shiftstates[0] && (GetKeyState(VK_LSHIFT) & 0x8000)) {
					wParam = VK_LSHIFT;
					prev_shiftstates[0] = TRUE;
				} else if (!prev_shiftstates[1] && (GetKeyState(VK_RSHIFT) & 0x8000)) {
					wParam = VK_RSHIFT;
					prev_shiftstates[1] = TRUE;
				} else {
					// Huh?
				}
				*/
				break;
			case VK_MENU:
				if (lParam&EXTENDED_KEYMASK)
					wParam = VK_RMENU;
				else
					wParam = VK_LMENU;
				break;
		}
		//
		UKeyCode_t keycode = VK_keymap[wParam];
		wchar_t unicode = 0;
		//
		// no unicode translation at key up
		m_display->pushKeyUp(context, keycode, unicode);
		}
		break;

	case WM_SYSCHAR:
	case WM_CHAR: {
	/*
			if(fgState.IgnoreKeyRepeat && (lParam & KF_REPEAT))
				break;

			fgState.Modifiers = fgGetWin32Modifiers();
			INVOKE_WCB(*window, Keyboard,
			            ((char)wParam,
			              window->State.MouseX, window->State.MouseY)
			         );
			fgState.Modifiers = 0xffffffff;
			*/
		}
		break;

	case WM_CAPTURECHANGED:
		// User has finished resizing the window, force a redraw
		context->getRootPane()->repaint();
		break;

		// Other messages that I have seen and which are not handled already
	case WM_SETTEXT:  /* 0x000c */
		lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
		/* Pass it on to "DefWindowProc" to set the window text */
		break;

	case WM_GETTEXT:  /* 0x000d */
		// Ideally we would copy the title of the window into "lParam"
		// strncpy ((char *)lParam, "Window Title", wParam);
		//   lRet = (wParam > 12) ? 12 : wParam;
		// the number of characters copied */
		lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_GETTEXTLENGTH:  /* 0x000e */
		// Ideally we would get the length of the title of the window
		lRet = 12;
		// the number of characters in "Window Title\0" (see above)
		break;

	case WM_ERASEBKGND:  /* 0x0014 */
		lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_SYNCPAINT:  /* 0x0088 */
		/* Another window has moved, need to update this one */
		context->getRootPane()->repaint();
		lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
		/* Help screen says this message must be passed to "DefWindowProc" */
		break;

	case WM_NCPAINT:  /* 0x0085 */
		/* Need to update the border of this window */
		lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
		/* Pass it on to "DefWindowProc" to repaint a standard border */
		break;

	case WM_SYSCOMMAND :  /* 0x0112 */
		/* We need to pass the message on to the operating system as well */
		lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;

	default:
		/*
		 * Handle unhandled messages
		 */
		lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return lRet;
}



class UWGLVideoPlugin : public UVideoPlugin {
	UFO_DECLARE_DYNAMIC_CLASS(UWGLVideoPlugin)
public:
	UWGLVideoPlugin() : m_wgldriver(NULL) {}
	virtual ~UWGLVideoPlugin() {
		if (m_wgldriver) {
			delete (m_wgldriver);
			m_wgldriver = NULL;
		}
	}
	virtual bool isAvailable() {
		return true;
	}
	virtual UVideoDriver * createVideoDriver() {
		if (!m_wgldriver) {
			m_wgldriver = new UXWGLDriver();
		}
		return m_wgldriver;
	}
private:
	UXWGLDriver * m_wgldriver;
};
UFO_IMPLEMENT_DYNAMIC_CLASS(UWGLVideoPlugin, UVideoPlugin)

UPluginBase *
UXWGLDriver::createPlugin() {
	return new UWGLVideoPlugin();
}

void
UXWGLDriver::destroyPlugin(UPluginBase * plugin) {
	delete (plugin);
}



//
// class UXWGLDevice
//
//

UXWGLDevice::UXWGLDevice(UXWGLDriver * driver)
		: m_wglDriver(driver)
		, m_window(0)
		, m_glContext(NULL)
		, m_frame(NULL)
		, m_size()
		, m_pos()
		, m_isVisible(false)
		, m_frameStyle(0)
		, m_frameState(0)
		, m_depth(0)
{
	/*
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
		*/
}

UXFrame *
UXWGLDevice::getFrame() const {
	return m_frame;
}

void
UXWGLDevice::setFrame(UXFrame * frame) {
	m_frame = frame;
}


void
UXWGLDevice::setSize(int w, int h) {
	UDimension size(w, h);
	if (size == m_size) {
		return;
	}
	m_size = size;
	if (m_isVisible) {
		RECT rect;
		GetWindowRect(m_window, &rect);
		rect.right  = rect.left + w;
		rect.bottom = rect.top  + h;

		// For windowed mode, get the current position of the
		// window and resize taking the size of the frame
		// decorations into account.
		rect.right += GetSystemMetrics(SM_CXSIZEFRAME) * 2;
		rect.bottom += GetSystemMetrics(SM_CYSIZEFRAME) * 2 +
				GetSystemMetrics(SM_CYCAPTION);

		// SWP_NOACTIVATE	Do not activate the window
		// SWP_NOOWNERZORDER	Do not change position in z-order
		// SWP_NOSENDCHANGING	Supress WM_WINDOWPOSCHANGING message
		// SWP_NOZORDER		Retains the current Z order (ignore 2nd param)

		SetWindowPos(m_window,
		             HWND_TOP,
		             rect.left,
		             rect.top,
		             rect.right  - rect.left,
		             rect.bottom - rect.top,
		             SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING |
		             SWP_NOZORDER
		           );

		// FIXME?
		//glViewport(0, 0, width, height);
	}
}

UDimension
UXWGLDevice::getSize() const {
	return m_size;
}

void
UXWGLDevice::setLocation(int x, int y) {
	UPoint pos(x, y);
	if (m_pos == pos) {
		return;
	}
	m_pos = pos;
	if (m_isVisible) {
		RECT winRect;

		GetWindowRect(m_window, &winRect);
		MoveWindow(
			m_window,
			x,
			y,
			winRect.right - winRect.left,
			winRect.bottom - winRect.top,
			TRUE
		);
	}
}


UPoint
UXWGLDevice::getLocation() const {
	return m_pos;
}


void
UXWGLDevice::setTitle(const std::string & title) {
	m_title = title;
}

std::string
UXWGLDevice::getTitle() const {
	return m_title;
}


void
UXWGLDevice::setDepth(int depth) {
	m_depth = depth;
}

int
UXWGLDevice::getDepth() const {
	return m_depth;
}


void
UXWGLDevice::swapBuffers() {
	//m_sdldriver->SDL_GL_SwapBuffers();
	//m_wglDriver->wglSwapBuffers(m_glxDriver->getX11Display(), m_window);
	SwapBuffers(m_dc);
}

void
UXWGLDevice::makeContextCurrent() {
	m_wglDriver->wglMakeCurrent(m_dc, m_glContext);
}

static HGLRC s_wgl_shared_context = NULL;
bool
UXWGLDevice::show() {
	WNDCLASS wc;
	DWORD dwStyle;
	DWORD dwExStyle;
	ATOM atom;

	// Grab the window class we have registered on glutInit():
	atom = GetClassInfo(m_wglDriver->getInstance(), "LIBUFO", &wc);
	assert(atom != 0);

	// this is from SDL?
	//(WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX),
	dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	dwStyle |= WS_OVERLAPPEDWINDOW;

	dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

	RECT bounds;

	bounds.left = m_pos.x;
	bounds.top = m_pos.y;
	bounds.right = m_pos.x + m_size.w;
	bounds.bottom = m_pos.y + m_size.h;
	AdjustWindowRectEx(&bounds, dwStyle, FALSE, dwExStyle);
	/*
	if(!fgState.Position.Use) {
		x = CW_USEDEFAULT;
		y = CW_USEDEFAULT;
	}
	if(!fgState.Size.Use) {
		w = CW_USEDEFAULT;
		h = CW_USEDEFAULT;
	}*/


	m_window = CreateWindowEx(
		dwExStyle,
		"LIBUFO",
		m_title.c_str(),
		dwStyle,
		m_pos.x, m_pos.y, // always use the desired window pos
		bounds.right - bounds.left, bounds.bottom - bounds.top,
		NULL, // parent
		(HMENU) NULL,
		m_wglDriver->getInstance(),
		NULL // lparam args
	);

	m_dc = GetDC(m_window);
	setupPixelFormat(PFD_MAIN_PLANE);
	m_glContext = m_wglDriver->wglCreateContext(m_dc);
	makeContextCurrent();
	if (s_wgl_shared_context == NULL) {
		s_wgl_shared_context = m_glContext;
	} else {
		m_wglDriver->wglShareLists(m_glContext, s_wgl_shared_context);
	}

	//setSizeHints();
	setWMHints();
	setDecorations();
	ShowWindow(m_window, SW_SHOW);

	SetForegroundWindow(m_window); // slightly higher priority?
	SetFocus(m_window);
	// FIXME: needed?
	UpdateWindow(m_window);
	ShowCursor(TRUE);



	m_isVisible = true;
	return true;
}

void
UXWGLDevice::hide() {
	if (s_wgl_shared_context == m_glContext) {
		s_wgl_shared_context = NULL;
	}
	m_wglDriver->wglDeleteContext(m_glContext);
	m_glContext = NULL;

	DestroyWindow(m_window);
	m_window = 0;

	m_isVisible = false;
}


void
UXWGLDevice::notify(uint32_t type, int arg1, int arg2, int arg3, int arg4) {
	switch (type) {
		case UEvent::WidgetResized: {
			UDimension newSize(arg1, arg2);
			if (m_size != newSize) {
				m_size = newSize;
				// FIXME: throw event
				if (m_frame) {
					m_frame->getContext()->setContextBounds(m_size);
				}
			}

		}
		break;
		case UEvent::WidgetMoved: {
			UPoint newPos(arg1, arg2);
			if (m_pos != newPos) {
				m_pos = newPos;
				// FIXME: throw event
			}
		}
		break;
		case UEvent::FocusGained:
		break;
		case UEvent::FocusLost:
		break;
		default:
		break;
	}
}

void
UXWGLDevice::setDecorations() {
	LONG style;
	LONG new_style_bits = 0;
	const LONG all_style_bits = WS_BORDER | WS_CAPTION | WS_SYSMENU |
		WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	// WS_THICKFRAME|

	style = GetWindowLong(m_window, GWL_STYLE);

	new_style_bits = 0;

	if (m_frameStyle & FrameNormalBorder) {
		new_style_bits |= WS_BORDER;
	}
	if (m_frameStyle & FrameTitleBar) {
		new_style_bits |= WS_CAPTION;
	}
	if (m_frameStyle & FrameSysMenu) {
		new_style_bits |= WS_SYSMENU;
	}
	if (m_frameStyle & FrameMinimizeBox) {
		new_style_bits |= WS_MINIMIZEBOX;
	}
	if (m_frameStyle & FrameMaximizeBox) {
		new_style_bits |= WS_MAXIMIZEBOX;
	}
	if (m_frameStyle == FrameDefault) {
		new_style_bits = all_style_bits;
	}

	SetWindowLong(m_window, GWL_STYLE, style);
	SetWindowPos(m_window, NULL, 0, 0, 0, 0,
		SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOMOVE |
		SWP_NOREPOSITION | SWP_NOSIZE | SWP_NOZORDER);
}

void
UXWGLDevice::setSizeHints() {
}

void
UXWGLDevice::setWMHints() {
	if (m_frameState & FrameFullScreen) {
		// FIXME
	} else if (m_frameState & FrameMinimized) {
		ShowWindow (m_window, SW_RESTORE);
    } else if (m_frameState & FrameMaximized) {
		ShowWindow (m_window, SW_MAXIMIZE);
    } else {
		ShowWindow (m_window, SW_SHOWNORMAL);
	}
}


void
UXWGLDevice::setFrameStyle(uint32_t frameStyle) {
	m_frameStyle = frameStyle;
}
uint32_t
UXWGLDevice::getFrameStyle() const {
	return m_frameStyle;
}

void
UXWGLDevice::setInitialFrameState(uint32_t frameState) {
	m_frameState = frameState;
}
uint32_t
UXWGLDevice::getFrameState() const {
	return m_frameState;
}

int
UXWGLDevice::getAttribute(int key) {
	return m_attributes[key];
}
void
UXWGLDevice::setAttribute(int key, int value) {
	m_attributes[key] = value;
}


bool
UXWGLDevice::setupPixelFormat(unsigned char layer_type) {
	PIXELFORMATDESCRIPTOR* ppfd, pfd;
	int flags, pixelformat;

	//freeglut_return_val_if_fail( window != NULL, 0 );
	flags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	// FIXME: check for double buffer support
	flags |= PFD_DOUBLEBUFFER;

//#pragma message( "fgSetupPixelFormat(): there is still some work to do here!" )

	// Specify which pixel format do we opt for...
	pfd.nSize           = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion        = 1;
	pfd.dwFlags         = flags;
	pfd.iPixelType      = PFD_TYPE_RGBA;
	pfd.cColorBits      = 24;
	pfd.cRedBits        = 0;
	pfd.cRedShift       = 0;
	pfd.cGreenBits      = 0;
	pfd.cGreenShift     = 0;
	pfd.cBlueBits       = 0;
	pfd.cBlueShift      = 0;
	pfd.cAlphaBits      = 0;
	pfd.cAlphaShift     = 0;
	pfd.cAccumBits      = 0;
	pfd.cAccumRedBits   = 0;
	pfd.cAccumGreenBits = 0;
	pfd.cAccumBlueBits  = 0;
	pfd.cAccumAlphaBits = 0;
#if 0
	pfd.cDepthBits      = 32;
	pfd.cStencilBits    = 0;
#else
	pfd.cDepthBits      = 24;
	pfd.cStencilBits    = 8;
#endif
	pfd.cAuxBuffers     = 0;
	pfd.iLayerType      = layer_type;
	pfd.bReserved       = 0;
	pfd.dwLayerMask     = 0;
	pfd.dwVisibleMask   = 0;
	pfd.dwDamageMask    = 0;

	pfd.cColorBits = (BYTE) GetDeviceCaps(m_dc, BITSPIXEL);
	ppfd = &pfd;

	pixelformat = ChoosePixelFormat(m_dc, ppfd);
	if(pixelformat == 0) {
		return false;
	}

	return SetPixelFormat(m_dc, pixelformat, ppfd);
}

#endif // UFO_USE_WGL
