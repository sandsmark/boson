/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/config/ufo_config_msvc.h
    begin             : Sun Jan 27 2002
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

#ifndef UFO_CONFIG_MSVC_H
#define UFO_CONFIG_MSVC_H

// disable win32 warnings on exporting STL objects
#pragma warning(disable: 4231) // nonstandard extension used : 'extern' before template explicit instantiation
#pragma warning(disable: 4244) // 'conversion' conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable: 4251) // class 'A' needs to have dll interface for to be used by clients of class 'B'.
#pragma warning(disable: 4275) // non - DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier'

#pragma warning(disable: 4355) // 'this' : used in base member initializer list

#pragma warning(disable: 4786) // truncating debug info after 255 characters
#pragma warning(disable: 4800) // 'type' : forcing value to bool 'true' or 'false' (performance warning)
#pragma warning(disable: 4291) // no matching operator delete found; memory will not be freed if initialization throws an exception

/* Define if you want to enable DEBUG mode. */
/* #undef DEBUG */

/* Define if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Define if you have the <sstream> header file. */
#define HAVE_SSTREAM 1

/* Define if you have the <stdint.h> header file. */
/* #undef HAVE_STDINT_H */

/* Name of package */
/* #undef PACKAGE */

/* insert the path to the theme and data dir of libUFO */
#define UFO_DATADIR "./data"

/* Define if you want to use the STL allocator of libUFO instead of libgc. */
/*#undef USE_UFO_STL_ALLOC */

// STLport specific
//#define USE_STLPORT_STL_ALLOC 1

// msvc specific

#define NOMINMAX

// Fixes STL problems with msvc <= 6
#include <algorithm>
#if defined(_MSC_VER) && (_MSC_VER < 1300) && !defined(_STLPORT_VERSION) && \
 !defined(__MWERKS__) && !defined (__ICL) && !defined (__COMO__)
 
namespace std {

template <typename Type>
inline const Type& min(const Type & a, const Type & b) {
	return b < a ? b : a;
}
template <typename Type>
inline const Type& max(const Type & a, const Type & b) {
	return a < b ? b : a;
}
template <typename Type>
inline Type abs(const Type & a) {
	return a > 0 ? a : -a;
}

	using ::size_t;
} // namespace std

#endif

/* Version number of package */
/* #undef VERSION */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define as `__inline' if that's what the C compiler calls it, or to nothing
    if it is not supported. */
#define inline __inline

// FIXME this will work only on intel 32 bit systems
// lazily copied from gnu config
/* The size of a `char' */
#define SIZEOF_CHAR 1

/* The size of a `short' */
#define SIZEOF_SHORT 2

/* The size of a `int' */
#define SIZEOF_INT 4

/* The size of a `long' */
#define SIZEOF_LONG 4


#endif // UFO_CONFIG_MSVC_H
