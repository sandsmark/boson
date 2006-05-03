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

#ifndef EXT_POINT_PARAMETERS_DECL_P_H
#define EXT_POINT_PARAMETERS_DECL_P_H

// note: compliant OpenGL implementations don't have to implement an extension!

/*
 new functions:
   glPointParameterfEXT()
   glPointParameterfvEXT()
*/

extern "C" {
	// GL typedefs
	typedef void (*_glPointParameterfEXT)(GLenum pname, GLfloat param);
	typedef void (*_glPointParameterfvEXT)(GLenum pname, const GLfloat *params);

	// GL function pointers
	extern _glPointParameterfEXT bo_glPointParameterfEXT;
	extern _glPointParameterfvEXT bo_glPointParameterfvEXT;
}; // extern "C"


#if BOGL_DO_DLOPEN
#define glPointParameterfvEXT bo_glPointParameterfvEXT
#define glPointParameterfEXT bo_glPointParameterfEXT
#endif // BOGL_DO_DLOPEN


#endif

