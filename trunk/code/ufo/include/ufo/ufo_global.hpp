/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ufo_global.hpp
    begin             : Wed Jul 10 2002
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

#ifndef UFO_GLOBAL_HPP
#define UFO_GLOBAL_HPP

// global macros, types and functions
// which should be included by every header file


// this is necessary as the boehm garbage collector overwrites some
// functions (e.g. pthread*)
//#include "ufo_gc.hpp"

//
// system dependent macros
//

#include "config/ufo_config.hpp"

#include "ufo_types.hpp"

#include "ufo_debug.hpp"

#endif // UFO_GLOBAL_HPP
