/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ufo_versioninfo.hpp
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

#ifndef UFO_VERSIONINFO_HPP
#define UFO_VERSIONINFO_HPP

// versioning in libUFO
// the version macros are based on the SDL sources
// http://www.libsdl.org
// thanks Sam!

/*
*/
#define UFO_MAJOR_VERSION	0
#define UFO_MINOR_VERSION	8
#define UFO_MICRO_VERSION	4

namespace ufo {
struct UVersionInfo {
	uint8_t major;
	uint8_t minor;
	uint8_t micro;
};


/** fills a version info struct with the compile time version of libufo */
#define UFO_FILL_VERSIONINFO(info) \
{ \
	info->major = UFO_MAJOR_VERSION \
	info->minor = UFO_MINOR_VERSION \
	info->micro = UFO_MICRO_VERSION \
}

#define UFO_VERSION(info) UFO_FILL_VERSIONINFO(info)

/** This macro turns the version numbers into a numeric value:
  * (1,2,3) -> 0x010203
  * This assumes that there will never be more than 256 patchlevels
  */
#define UFO_MAKE_VERSION(X, Y, Z) \
(X) << 16 + (Y) << 8 + (Z)

/** This is the version number macro for the current UFO version */
#define UFO_COMPILED_VERSION \
UFO_MAKE_VERSION(UFO_MAJOR_VERSION, UFO_MINOR_VERSION, UFO_MICRO_VERSION)

/** This macro will evaluate to true if compiled with UFO at least X.Y.Z */
#define UFO_VERSION_ATLEAST(X, Y, Z) \
(UFO_COMPILED_VERSION >= UFO_MAKE_VERSION(X, Y, Z))

} // namespace ufo

#endif // UFO_VERSIONINFO_HPP
