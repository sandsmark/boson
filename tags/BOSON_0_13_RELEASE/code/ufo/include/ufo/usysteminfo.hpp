/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/usysteminfo.hpp
    begin             : Mon Mar 31 2003
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

#ifndef UFO_SYSTEMINFO_HPP
#define UFO_SYSTEMINFO_HPP

// This file describes system specific information which can be retained
// by a UFO context via UContext::getSystemInfo() const;

// Backends may overwrite this system info by using their own system info
// header
// They should however do this in a "binary compatible way", i.e. keep
// the order of the struct

// the version info attribute should be filled by the user so that the ufo
// implementation can decide which attributes should be filled

// e.g.:
// USystemInfo info
// UFO_VERSION(&info.version)
// context->getSystemInfo(&info)


#if defined(UFO_GFX_X11)

#include <bogl.h>
#include <boglx.h>
#include <X11/Xlib.h>
#include <GL/glu.h>

namespace ufo {
struct USystemInfo {
	Window window;
	GLXContext glContext;
	Display * display;
};
} // namespace ufo

#elif defined(UFO_GFX_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace ufo {
struct USystemInfo {
	HWND window;
	HGLRC glContext;
};
} // namespace ufo

#else // unknown system

namespace ufo {
struct USystemInfo {
	void * data;
};
} // namespace ufo

#endif


#endif // UFO_SYSTEMINFO_HPP
