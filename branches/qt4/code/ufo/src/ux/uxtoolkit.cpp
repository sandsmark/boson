/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/ux/uxtoolkit
    begin             : Mon Jul 26 2004
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

#include "ufo/ux/uxtoolkit.hpp"

#include "ufo/ux/uxcontext.hpp"
#include "ufo/ux/uxframe.hpp"

using namespace ufo;

UFO_IMPLEMENT_DYNAMIC_CLASS(UXToolkit, UAbstractToolkit)

UXToolkit::UXToolkit()
	: UAbstractToolkit(NULL)
	, m_context(NULL)
{
	startTicks();
	setToolkit(this);
}

UXToolkit::UXToolkit(UProperties * prop)
	: UAbstractToolkit(prop)
	, m_context(NULL)
{
	startTicks();
	setToolkit(this);
}

UXToolkit::~UXToolkit()
{}

void
UXToolkit::makeContextCurrent(UContext * contextA) {
	m_context = contextA;
	if (UXContext * uxcontext = dynamic_cast<UXContext*>(contextA)) {
		if (uxcontext->getFrame()) {
			uxcontext->getFrame()->makeContextCurrent();
		}
	}
}

UContext *
UXToolkit::getCurrentContext() const {
	return m_context;
}


UDimension
UXToolkit::getScreenSize() const {
	UDimension ret;

#ifdef UFO_TARGET_X11
	Display * display = XOpenDisplay(NULL);
	ret.w = DisplayWidth(display, DefaultScreen(display));
	ret.h = DisplayHeight(display, DefaultScreen(display));
	XCloseDisplay(display);
#elif defined(UFO_TARGET_WIN32)
		RECT desktopRect;

		HWND desktop = GetDesktopWindow();
		GetWindowRect(desktop, &desktopRect);

		ret.w = desktopRect.right - desktopRect.left;
		ret.h = desktopRect.bottom - desktopRect.top;
#else
//#warning UToolkit::getScreenSize() not implemented for your system
#endif
	return ret;
}

UInsets
UXToolkit::getScreenInsets() const {
	UInsets ret;

#ifdef UFO_TARGET_X11
#elif defined(UFO_TARGET_WIN32)
#else
//#warning UToolkit::getScreenSize() not implemented for your system
#endif
	return ret;
}

int
UXToolkit::getScreenDepth() const {
	int ret = 16;
#ifdef UFO_TARGET_X11
	Display * dpy = XOpenDisplay(NULL);
	ret = DefaultDepth(dpy, DefaultScreen(dpy));
	XCloseDisplay(display);
#elif defined(UFO_TARGET_WIN32)
#else
//#warning UToolkit::getScreenSize() not implemented for your system
#endif
	return ret;
}


#if defined(UFO_OS_WIN32)
#include <windows.h>
#define TIME_WRAP_VALUE	(~(DWORD)0)
static DWORD start;
#elif defined(UFO_OS_UNIX)
#include <unistd.h>
#include <sys/time.h>
static struct timeval start;
#endif

void
UXToolkit::sleep(uint32_t millis) {
#if defined(UFO_OS_WIN32)
	Sleep(millis);
#elif defined(UFO_OS_UNIX)
	usleep(millis * 1000);
#endif
}


void
UXToolkit::startTicks() {
#if defined(UFO_OS_WIN32)
	start = GetTickCount();
#elif defined(UFO_OS_UNIX)
	gettimeofday(&start, NULL);
#endif
}

uint32_t
UXToolkit::getTicks() const {
#if defined(UFO_OS_WIN32)
	DWORD now, ticks;
	now = GetTickCount();
	if (now < start) {
		ticks = (TIME_WRAP_VALUE - start) + now;
	} else {
		ticks = (now - start);
	}
	return ticks;
#elif defined(UFO_OS_UNIX)
	struct timeval now;
	uint32_t ticks;

	gettimeofday(&now, NULL);
	ticks=(now.tv_sec-start.tv_sec)*1000+(now.tv_usec-start.tv_usec)/1000;
	return ticks;
#else
	return 0;
#endif
}
