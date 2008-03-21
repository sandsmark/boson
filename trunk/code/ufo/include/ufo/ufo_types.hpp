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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
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


// some environments ( mingw32 ) don´t have the STL string stream
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
	NoOrientation = 0,
	Horizontal,
	Vertical
};

enum Direction {
	NoDirection = 0,
	Up,
	Down,
	Left,
	Right,
	LeftToRight = Right,
	RightToLeft = Left
};

/** The relief resp. contour of an object. Used by UBevelBorder */
enum Relief {
	Raised = 0,
	Lowered
};

// Alignment
enum Alignment {
	AlignNone = 0,
	AlignStart,
	AlignEnd,
	AlignCenter,
	AlignStretch,
	AlignLeft = AlignStart,
	AlignRight = AlignEnd,
	AlignTop = AlignStart,
	AlignBottom = AlignEnd
};

enum BorderType {
	NoBorder = 0,
	LineBorder,
	BottomLineBorder,
	RaisedBevelBorder,
	LoweredBevelBorder,
	StyleBorder = 100,
	CssBorder,
	BorderLast = 199
};

enum BorderStyle {
	NoBorderStyle = 0,
	BorderSolid,
	BorderDotted,
	BorderDashed,
	BorderDouble,
	BorderGroove,
	BorderRidge,
	BorderInset,
	BorderOutset,
	BorderStyleLast = BorderOutset
};

/** Some generic widget flags which covers most widget states. */
enum WidgetState {
	WidgetNoState 			= 0x000000,
	WidgetVisible			= 0x000001,
	WidgetForceInvisible	= 0x000002,
	WidgetDisabled			= 0x000004,
	WidgetForceDisabled		= 0x000008,
	WidgetEditable			= 0x000010,
	WidgetFocusable			= 0x000020,
	WidgetHasFocus			= 0x000040,
	WidgetHasMouseFocus		= 0x000080,
	WidgetSelected			= 0x000100,
	WidgetPressed			= 0x000200,
	WidgetRaised			= 0x000400,
	WidgetToggable			= 0x000800,
	WidgetHighlighted		= 0x001000,
	WidgetEditing			= 0x002000
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
	FrameActive = FrameStyleLast << 10,
	FrameStateLast = FrameActive,
	FrameStateFlags = FrameModal | FrameSticky |
		FrameStaysOnTop | FrameSkipTaskBar | FrameMaximized | FrameMinimized |
		FrameFullScreen | FrameCreated | FrameVisible | FrameActive
};

enum DockWidgetArea {
	LeftDockWidgetArea = 0x01,
	RightDockWidgetArea = 0x02,
	TopDockWidgetArea = 0x04,
	BottomDockWidgetArea = 0x08,
	AllDockWidgetAreas = 0xff
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
