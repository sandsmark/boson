/*
    This file is part of the Boson game
    Copyright (C) 2005-2006 Andreas Beckermann <b_mann@gmx.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef EXT_BLEND_COLOR_DECL_P_H
#define EXT_BLEND_COLOR_DECL_P_H

// note: compliant OpenGL implementations don't have to implement an extension!

extern "C" {
	// GL typedefs
	typedef void (*_glBlendColorEXT)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

	// GL function pointers
	extern _glBlendColorEXT bo_glBlendColorEXT;
}; // extern "C"


#define GL_CONSTANT_COLOR_EXT                 0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR_EXT       0x8002
#define GL_CONSTANT_ALPHA_EXT                 0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA_EXT       0x8004
#define GL_BLEND_COLOR_EXT                    0x8005


#if BOGL_DO_DLOPEN
#define glBlendColorEXT bo_glBlendColorEXT
#endif // BOGL_DO_DLOPEN


#endif

