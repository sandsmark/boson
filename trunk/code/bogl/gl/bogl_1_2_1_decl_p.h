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

#ifndef BOGL_1_2_1_DECL_P_H
#define BOGL_1_2_1_DECL_P_H

#include "bogl_1_2_decl_p.h"

// AB:
// OpenGL 1.2.1 introduced ARB extensions.

/*
 additions in OpenGL 1.2.1:
 ARB extensions (do not HAVE to be implemented by an OpenGL implementation!):
 * ARB_multitexture
*/

/*
 new functions:
 * ARB_multitexture:
   glMultiTexCoord{1234}{sifd}ARB()
   glMultiTexCoord{1234}{sifd}vARB()
   glClientActiveTextureARB()
   glActiveTextureARB()
*/


extern "C" {
	// GL typedefs
	typedef void (*_glMultiTexCoord1dARB)(GLenum target, GLdouble s);
	typedef void (*_glMultiTexCoord1dvARB)(GLenum target, const GLdouble *v);
	typedef void (*_glMultiTexCoord1fARB)(GLenum target, GLfloat s);
	typedef void (*_glMultiTexCoord1fvARB)(GLenum target, const GLfloat *v);
	typedef void (*_glMultiTexCoord1iARB)(GLenum target, GLint s);
	typedef void (*_glMultiTexCoord1ivARB)(GLenum target, const GLint *v);
	typedef void (*_glMultiTexCoord1sARB)(GLenum target, GLshort s);
	typedef void (*_glMultiTexCoord1svARB)(GLenum target, const GLshort *v);
	typedef void (*_glMultiTexCoord2dARB)(GLenum target, GLdouble s, GLdouble t);
	typedef void (*_glMultiTexCoord2dvARB)(GLenum target, const GLdouble *v);
	typedef void (*_glMultiTexCoord2fARB)(GLenum target, GLfloat s, GLfloat t);
	typedef void (*_glMultiTexCoord2fvARB)(GLenum target, const GLfloat *v);
	typedef void (*_glMultiTexCoord2iARB)(GLenum target, GLint s, GLint t);
	typedef void (*_glMultiTexCoord2ivARB)(GLenum target, const GLint *v);
	typedef void (*_glMultiTexCoord2sARB)(GLenum target, GLshort s, GLshort t);
	typedef void (*_glMultiTexCoord2svARB)(GLenum target, const GLshort *v);
	typedef void (*_glMultiTexCoord3dARB)(GLenum target, GLdouble s, GLdouble t, GLdouble r);
	typedef void (*_glMultiTexCoord3dvARB)(GLenum target, const GLdouble *v);
	typedef void (*_glMultiTexCoord3fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
	typedef void (*_glMultiTexCoord3fvARB)(GLenum target, const GLfloat *v);
	typedef void (*_glMultiTexCoord3iARB)(GLenum target, GLint s, GLint t, GLint r);
	typedef void (*_glMultiTexCoord3ivARB)(GLenum target, const GLint *v);
	typedef void (*_glMultiTexCoord3sARB)(GLenum target, GLshort s, GLshort t, GLshort r);
	typedef void (*_glMultiTexCoord3svARB)(GLenum target, const GLshort *v);
	typedef void (*_glMultiTexCoord4dARB)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
	typedef void (*_glMultiTexCoord4dvARB)(GLenum target, const GLdouble *v);
	typedef void (*_glMultiTexCoord4fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
	typedef void (*_glMultiTexCoord4fvARB)(GLenum target, const GLfloat *v);
	typedef void (*_glMultiTexCoord4iARB)(GLenum target, GLint s, GLint t, GLint r, GLint q);
	typedef void (*_glMultiTexCoord4ivARB)(GLenum target, const GLint *v);
	typedef void (*_glMultiTexCoord4sARB)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
	typedef void (*_glMultiTexCoord4svARB)(GLenum target, const GLshort *v);
	typedef void (*_glClientActiveTextureARB)(GLenum texture);
	typedef void (*_glActiveTextureARB)(GLenum texture);




	// GL function pointers
	extern _glMultiTexCoord1dARB bo_glMultiTexCoord1dARB;
	extern _glMultiTexCoord1dvARB bo_glMultiTexCoord1dvARB;
	extern _glMultiTexCoord1fARB bo_glMultiTexCoord1fARB;
	extern _glMultiTexCoord1fvARB bo_glMultiTexCoord1fvARB;
	extern _glMultiTexCoord1iARB bo_glMultiTexCoord1iARB;
	extern _glMultiTexCoord1ivARB bo_glMultiTexCoord1ivARB;
	extern _glMultiTexCoord1sARB bo_glMultiTexCoord1sARB;
	extern _glMultiTexCoord1svARB bo_glMultiTexCoord1svARB;
	extern _glMultiTexCoord2dARB bo_glMultiTexCoord2dARB;
	extern _glMultiTexCoord2dvARB bo_glMultiTexCoord2dvARB;
	extern _glMultiTexCoord2fARB bo_glMultiTexCoord2fARB;
	extern _glMultiTexCoord2fvARB bo_glMultiTexCoord2fvARB;
	extern _glMultiTexCoord2iARB bo_glMultiTexCoord2iARB;
	extern _glMultiTexCoord2ivARB bo_glMultiTexCoord2ivARB;
	extern _glMultiTexCoord2sARB bo_glMultiTexCoord2sARB;
	extern _glMultiTexCoord2svARB bo_glMultiTexCoord2svARB;
	extern _glMultiTexCoord3dARB bo_glMultiTexCoord3dARB;
	extern _glMultiTexCoord3dvARB bo_glMultiTexCoord3dvARB;
	extern _glMultiTexCoord3fARB bo_glMultiTexCoord3fARB;
	extern _glMultiTexCoord3fvARB bo_glMultiTexCoord3fvARB;
	extern _glMultiTexCoord3iARB bo_glMultiTexCoord3iARB;
	extern _glMultiTexCoord3ivARB bo_glMultiTexCoord3ivARB;
	extern _glMultiTexCoord3sARB bo_glMultiTexCoord3sARB;
	extern _glMultiTexCoord3svARB bo_glMultiTexCoord3svARB;
	extern _glMultiTexCoord4dARB bo_glMultiTexCoord4dARB;
	extern _glMultiTexCoord4dvARB bo_glMultiTexCoord4dvARB;
	extern _glMultiTexCoord4fARB bo_glMultiTexCoord4fARB;
	extern _glMultiTexCoord4fvARB bo_glMultiTexCoord4fvARB;
	extern _glMultiTexCoord4iARB bo_glMultiTexCoord4iARB;
	extern _glMultiTexCoord4ivARB bo_glMultiTexCoord4ivARB;
	extern _glMultiTexCoord4sARB bo_glMultiTexCoord4sARB;
	extern _glMultiTexCoord4svARB bo_glMultiTexCoord4svARB;
	extern _glClientActiveTextureARB bo_glClientActiveTextureARB;
	extern _glActiveTextureARB bo_glActiveTextureARB;

}; // extern "C"


// GL defines
#if BOGL_DO_DLOPEN
// (TODO)
#endif // BOGL_DO_DLOPEN

#endif

