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

#ifndef EXT_TEXTURE_3D_DECL_P_H
#define EXT_TEXTURE_3D_DECL_P_H

// note: compliant OpenGL implementations don't have to implement an extension!

/*
 new functions:
   _glTexImage3DEXT()
   _glTexSubImage3DEXT()   (not mentioned in the extension, but I think it's part of it anyway)
   _glCopyTexSubImage3DEXT()   (not mentioned in the extension, but I think it's part of it anyway)
*/

extern "C" {
	// GL typedefs
	typedef void (*_glTexImage3DEXT)(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
	typedef void (*_glTexSubImage3DEXT)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
	typedef void (*_glCopyTexSubImage3DEXT)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

	// GL function pointers
	extern _glTexImage3DEXT bo_glTexImage3DEXT;
	extern _glTexSubImage3DEXT bo_glTexSubImage3DEXT;
	extern _glCopyTexSubImage3DEXT bo_glCopyTexSubImage3DEXT;
}; // extern "C"


#if BOGL_DO_DLOPEN
#define glTexImage3DEXT bo_glTexImage3DEXT
#define glTexSubImage3DEXT bo_glTexSubImage3DEXT
#define glCopyTexSubImage3DEXT bo_glCopyTexSubImage3DEXT
#endif // BOGL_DO_DLOPEN


#endif

