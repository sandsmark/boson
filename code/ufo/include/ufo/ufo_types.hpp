/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ufo_types.hpp
    begin             : Sat Jul 7 2001
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

#ifndef UFO_TYPES_HPP
#define UFO_TYPES_HPP

// C99 integer types

#ifdef HAVE_STDINT_H
#include <stdint.h>

#else // HAVE_STDINT_H

// taken gratefully from the SDL 2.0 sources (SDL_stdint.h)
#if SIZEOF_CHAR == 1
typedef signed char     int8_t;
typedef unsigned char   uint8_t;
#endif

#if SIZEOF_SHORT == 2
typedef signed short    int16_t;
typedef unsigned short  uint16_t;
#else
# if SIZEOF_INT == 2
typedef signed int      int16_t;
typedef unsigned int    uint16_t;
# endif
#endif

#if SIZEOF_INT == 4
typedef signed int      int32_t;
typedef unsigned int    uint32_t;
#else
# if SIZEOF_LONG == 4
typedef signed long     int32_t;
typedef unsigned long   uint32_t;
# endif
#endif

#if SIZEOF_LONG == 8
typedef signed long     int64_t;
typedef unsigned long   uint64_t;
#else
# if SIZEOF_LONG_LONG == 8
typedef signed long long int int64_t;
typedef unsigned long long int uint64_t;
# endif
#endif

#endif // HAVE_STDINT_H


// some environments ( mingw32 ) don�t have the STL string stream
#ifdef HAVE_SSTREAM
#include <sstream>
namespace ufo {
typedef std::stringstream UStringStream;
typedef std::ostringstream UOStringStream;
typedef std::istringstream UIStringStream;
} // namespace ufo
#else
#  include <strstream>
namespace ufo {
typedef std::strstream UStringStream;
typedef std::ostrstream UOStringStream;
typedef std::istrstream UIStringStream;
} // namespace ufo
#endif


namespace ufo {

/** Used by scroll bars and separators */
enum Orientation {
	Horizontal = 0,
	Vertical
};

enum Direction {
	Up = 0,
	Down,
	Left,
	Right
};

/** The relief resp. contour of an object. Used by UBevelBorder */
enum Relief {
	Raised = 0,
	Lowered
};

// Alignment
enum Alignment {
	AlignNone = 0,
	AlignLeft,
	AlignRight,
	AlignCenter,
	AlignTop,
	AlignBottom
};

enum BorderType {
	NoBorder = 0,
	LineBorder,
	RaisedBevelBorder,
	LoweredBevelBorder,
	TitledBorder,
	UIBorder = 100,
	BorderLast = 199
};

enum FrameStyle {
	FrameDefault = 0,
	FrameNoBorder = 1 << 0,
	FrameNormalBorder = 1 << 1,
	FrameTitleBar = 1 << 2,
	FrameSysMenu = 1 << 3,
	FrameMinimizeBox = 1 << 4,
	FrameMaximizeBox = 1 << 5,
	FrameMinMaxBox = FrameMinimizeBox | FrameMaximizeBox,
	FrameCloseBox = 1 << 6,
	FrameResizable = 1 << 7,
	FrameStyleLast = FrameResizable,
	FrameStyleFlags = FrameNoBorder | FrameNormalBorder | FrameTitleBar |
		FrameSysMenu | FrameMinMaxBox | FrameCloseBox | FrameResizable
};

enum FrameState {
	FrameModal = FrameStyleLast << 1,
	FrameSticky = FrameStyleLast << 2,
	FrameStaysOnTop = FrameStyleLast << 3,
	FrameSkipTaskBar = FrameStyleLast << 4,
	FrameMaximized = FrameStyleLast << 5,
	FrameMinimized = FrameStyleLast << 6,
	FrameFullScreen = FrameStyleLast << 7,
	FrameCreated = FrameStyleLast << 8,
	FrameVisible = FrameStyleLast << 9,
	FrameStateLast = FrameVisible,
	FrameStateFlags = FrameModal | FrameSticky |
		FrameStaysOnTop | FrameSkipTaskBar | FrameMaximized | FrameMinimized |
		FrameFullScreen | FrameCreated | FrameVisible
};

//
// disable copy constructor and assignment operator
//

#define UFO_DECLARE_NO_COPY_CLASS(classname)        \
	private:                                    \
		classname(const classname&);            \
		classname& operator=(const classname&);

} // namespace ufo

#endif // UFO_TYPES_HPP
