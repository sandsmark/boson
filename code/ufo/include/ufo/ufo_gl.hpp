/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/ufo_gl.hpp
    begin             : Sat Nov 3 2001
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

/* This is a simple encapsulation of opengl headers for libUFO.
 * The idea is taken gratefully from the SDL project
 * (http://www.libsdl.org), file SDL_opengl.h
 * Changes: Removed the NO_SDL_GLEXT thing
 */
 

#if defined(UFO_OS_WIN32) || defined(WIN32) || defined (_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // WIN32

// AB: ufo code replaced by our own
#include <bogl.h>
