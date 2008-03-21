/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/config/ufo_config.hpp
    begin             : Sat Jan 26 2002
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

#ifndef UFO_CONFIG_HPP
#define UFO_CONFIG_HPP

#if defined (_WIN32) && !defined (WIN32)
#define WIN32
#endif

#if defined HAVE_CONFIG_H
// this is our auto configured config header
#include "ufo_config_gnu.hpp"
#elif defined(WIN32) && defined(_MSC_VER)
// use msvc header for win 32 targets without configure
#include "ufo_config_msvc.hpp"
#else
// last resort
#include "ufo_config_gnu.hpp"
#endif

#undef PACKAGE
#undef VERSION

// Some stuff needed for Win32
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#endif // WIN32

#ifndef _UFO_EXPORT_RULE
# ifdef WIN32
#    define _UFO_EXPORT_RULE __declspec(dllexport)
#    define _UFO_IMPORT_RULE __declspec(dllimport)
# endif
#endif

#ifndef _UFO_EXPORT_RULE
#define _UFO_EXPORT_RULE
#define _UFO_IMPORT_RULE
#endif

#if defined(UFO_BUILDING_DLL) || (UFO_EXPORTS)
# define UFO_EXPORT _UFO_EXPORT_RULE
#elif defined(UFO_USING_DLL)
# define UFO_EXPORT _UFO_IMPORT_RULE
#else
// use import rule as default
# define UFO_EXPORT _UFO_IMPORT_RULE
#endif

// debug macros
#if !defined(UFO_DEBUG) && defined DEBUG
#define UFO_DEBUG
#endif

// ufo internal run time type information
#define UFO_RTTI

//
// OS specific defines

#if defined(WIN32)
#define UFO_OS_WIN32
#elif defined __BEOS__
#define UFO_OS_BEOS
#elif defined macintosh
#define UFO_OS_MAC
#else
/** */
#define UFO_OS_UNIX
#endif


// taken from libSDL
// defines the graphic target
#if (defined(unix) || defined(__unix__) || defined(_AIX) || defined(__OpenBSD__)) && \
    (!defined(UFO_DISABLE_X11) && !defined(__CYGWIN32__) && !defined(ENABLE_NANOX)) && \
    !defined(UFO_TARGET_X11)
#define UFO_GFX_X11
#elif defined(WIN32) && !defined(UFO_GFX_WIN32)
#define UFO_GFX_WIN32
#endif


#endif // UFO_CONFIG_HPP
