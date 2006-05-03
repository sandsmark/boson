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

#ifndef BOGL_1_3_DECL_P_H
#define BOGL_1_3_DECL_P_H

#include "bogl_1_2_1_decl_p.h"

/*
 additions in OpenGL 1.3:
 previously extensions and now directly included:
 * ARB_texture_compression
 * ARB_texture_cube_map
 * ARB_multisample
 * ARB_multitexture
 * ARB_texture_env_add
 * ARB_texture_env_combine
 * ARB_texture_env_dot3
 * ARB_texture_border_clamp
 * ARB_transpose_matrix
*/

/*
 new functions:
 * ARB_texture_compression:
   glCompressedTexImage3D()
   glCompressedTexImage2D()
   glCompressedTexImage1D()
   glCompressedTexSubImage3D()
   glCompressedTexSubImage2D()
   glCompressedTexSubImage1D()
   glGetCompressedTexImage()
 * ARB_texture_cube_map
   none
 * ARB_multisample
   glSampleCoverage()
 * ARB_multitexture (see also OpenGL 1.2.1 - just remove the ARB suffix)
   glMultiTexCoord{1234}{sifd}()
   glMultiTexCoord{1234}{sifd}v()
   glClientActiveTexture()
   glActiveTexture()
 * ARB_texture_env_add
   none
 * ARB_texture_env_combine
   none
 * ARB_texture_env_dot3
   none
 * ARB_texture_border_clamp
   none
 * ARB_transpose_matrix
   glLoadTransposeMatrixf()
   glLoadTransposeMatrixd()
   glMultTransposeMatrixf()
   glMultTransposeMatrixd()
*/


extern "C" {
	// GL typedefs
	typedef void (*_glCompressedTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
	typedef void (*_glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
	typedef void (*_glCompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
	typedef void (*_glCompressedTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
	typedef void (*_glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
	typedef void (*_glCompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
	typedef void (*_glGetCompressedTexImage)(GLenum target, GLint lod, GLvoid *img);
	typedef void (*_glSampleCoverage)(GLclampf value, GLboolean invert);
	typedef void (*_glMultiTexCoord1d)(GLenum target, GLdouble s);
	typedef void (*_glMultiTexCoord1dv)(GLenum target, const GLdouble *v);
	typedef void (*_glMultiTexCoord1f)(GLenum target, GLfloat s);
	typedef void (*_glMultiTexCoord1fv)(GLenum target, const GLfloat *v);
	typedef void (*_glMultiTexCoord1i)(GLenum target, GLint s);
	typedef void (*_glMultiTexCoord1iv)(GLenum target, const GLint *v);
	typedef void (*_glMultiTexCoord1s)(GLenum target, GLshort s);
	typedef void (*_glMultiTexCoord1sv)(GLenum target, const GLshort *v);
	typedef void (*_glMultiTexCoord2d)(GLenum target, GLdouble s, GLdouble t);
	typedef void (*_glMultiTexCoord2dv)(GLenum target, const GLdouble *v);
	typedef void (*_glMultiTexCoord2f)(GLenum target, GLfloat s, GLfloat t);
	typedef void (*_glMultiTexCoord2fv)(GLenum target, const GLfloat *v);
	typedef void (*_glMultiTexCoord2i)(GLenum target, GLint s, GLint t);
	typedef void (*_glMultiTexCoord2iv)(GLenum target, const GLint *v);
	typedef void (*_glMultiTexCoord2s)(GLenum target, GLshort s, GLshort t);
	typedef void (*_glMultiTexCoord2sv)(GLenum target, const GLshort *v);
	typedef void (*_glMultiTexCoord3d)(GLenum target, GLdouble s, GLdouble t, GLdouble r);
	typedef void (*_glMultiTexCoord3dv)(GLenum target, const GLdouble *v);
	typedef void (*_glMultiTexCoord3f)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
	typedef void (*_glMultiTexCoord3fv)(GLenum target, const GLfloat *v);
	typedef void (*_glMultiTexCoord3i)(GLenum target, GLint s, GLint t, GLint r);
	typedef void (*_glMultiTexCoord3iv)(GLenum target, const GLint *v);
	typedef void (*_glMultiTexCoord3s)(GLenum target, GLshort s, GLshort t, GLshort r);
	typedef void (*_glMultiTexCoord3sv)(GLenum target, const GLshort *v);
	typedef void (*_glMultiTexCoord4d)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
	typedef void (*_glMultiTexCoord4dv)(GLenum target, const GLdouble *v);
	typedef void (*_glMultiTexCoord4f)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
	typedef void (*_glMultiTexCoord4fv)(GLenum target, const GLfloat *v);
	typedef void (*_glMultiTexCoord4i)(GLenum target, GLint s, GLint t, GLint r, GLint q);
	typedef void (*_glMultiTexCoord4iv)(GLenum target, const GLint *v);
	typedef void (*_glMultiTexCoord4s)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
	typedef void (*_glMultiTexCoord4sv)(GLenum target, const GLshort *v);
	typedef void (*_glClientActiveTexture)(GLenum texture);
	typedef void (*_glActiveTexture)(GLenum texture);
	typedef void (*_glLoadTransposeMatrixd)(const GLdouble m[16]);
	typedef void (*_glLoadTransposeMatrixf)(const GLfloat m[16]);
	typedef void (*_glMultTransposeMatrixd)(const GLdouble m[16]);
	typedef void (*_glMultTransposeMatrixf)(const GLfloat m[16]);




	// GL function pointers
	extern _glCompressedTexImage1D bo_glCompressedTexImage1D;
	extern _glCompressedTexImage2D bo_glCompressedTexImage2D;
	extern _glCompressedTexImage3D bo_glCompressedTexImage3D;
	extern _glCompressedTexSubImage1D bo_glCompressedTexSubImage1D;
	extern _glCompressedTexSubImage2D bo_glCompressedTexSubImage2D;
	extern _glCompressedTexSubImage3D bo_glCompressedTexSubImage3D;
	extern _glGetCompressedTexImage bo_glGetCompressedTexImage;
	extern _glSampleCoverage bo_glSampleCoverage;
	extern _glMultiTexCoord1d bo_glMultiTexCoord1d;
	extern _glMultiTexCoord1dv bo_glMultiTexCoord1dv;
	extern _glMultiTexCoord1f bo_glMultiTexCoord1f;
	extern _glMultiTexCoord1fv bo_glMultiTexCoord1fv;
	extern _glMultiTexCoord1i bo_glMultiTexCoord1i;
	extern _glMultiTexCoord1iv bo_glMultiTexCoord1iv;
	extern _glMultiTexCoord1s bo_glMultiTexCoord1s;
	extern _glMultiTexCoord1sv bo_glMultiTexCoord1sv;
	extern _glMultiTexCoord2d bo_glMultiTexCoord2d;
	extern _glMultiTexCoord2dv bo_glMultiTexCoord2dv;
	extern _glMultiTexCoord2f bo_glMultiTexCoord2f;
	extern _glMultiTexCoord2fv bo_glMultiTexCoord2fv;
	extern _glMultiTexCoord2i bo_glMultiTexCoord2i;
	extern _glMultiTexCoord2iv bo_glMultiTexCoord2iv;
	extern _glMultiTexCoord2s bo_glMultiTexCoord2s;
	extern _glMultiTexCoord2sv bo_glMultiTexCoord2sv;
	extern _glMultiTexCoord3d bo_glMultiTexCoord3d;
	extern _glMultiTexCoord3dv bo_glMultiTexCoord3dv;
	extern _glMultiTexCoord3f bo_glMultiTexCoord3f;
	extern _glMultiTexCoord3fv bo_glMultiTexCoord3fv;
	extern _glMultiTexCoord3i bo_glMultiTexCoord3i;
	extern _glMultiTexCoord3iv bo_glMultiTexCoord3iv;
	extern _glMultiTexCoord3s bo_glMultiTexCoord3s;
	extern _glMultiTexCoord3sv bo_glMultiTexCoord3sv;
	extern _glMultiTexCoord4d bo_glMultiTexCoord4d;
	extern _glMultiTexCoord4dv bo_glMultiTexCoord4dv;
	extern _glMultiTexCoord4f bo_glMultiTexCoord4f;
	extern _glMultiTexCoord4fv bo_glMultiTexCoord4fv;
	extern _glMultiTexCoord4i bo_glMultiTexCoord4i;
	extern _glMultiTexCoord4iv bo_glMultiTexCoord4iv;
	extern _glMultiTexCoord4s bo_glMultiTexCoord4s;
	extern _glMultiTexCoord4sv bo_glMultiTexCoord4sv;
	extern _glClientActiveTexture bo_glClientActiveTexture;
	extern _glActiveTexture bo_glActiveTexture;
	extern _glLoadTransposeMatrixd bo_glLoadTransposeMatrixd;
	extern _glLoadTransposeMatrixf bo_glLoadTransposeMatrixf;
	extern _glMultTransposeMatrixd bo_glMultTransposeMatrixd;
	extern _glMultTransposeMatrixf bo_glMultTransposeMatrixf;
}; // extern "C"


// GL defines
#if BOGL_DO_DLOPEN

#define glCompressedTexImage1D bo_glCompressedTexImage1D
#define glCompressedTexImage2D bo_glCompressedTexImage2D
#define glCompressedTexImage3D bo_glCompressedTexImage3D
#define glCompressedTexSubImage1D bo_glCompressedTexSubImage1D
#define glCompressedTexSubImage2D bo_glCompressedTexSubImage2D
#define glCompressedTexSubImage3D bo_glCompressedTexSubImage3D
#define glGetCompressedTexImage bo_glGetCompressedTexImage
#define glSampleCoverage bo_glSampleCoverage
#define glMultiTexCoord1d bo_glMultiTexCoord1d
#define glMultiTexCoord1dv bo_glMultiTexCoord1dv
#define glMultiTexCoord1f bo_glMultiTexCoord1f
#define glMultiTexCoord1fv bo_glMultiTexCoord1fv
#define glMultiTexCoord1i bo_glMultiTexCoord1i
#define glMultiTexCoord1iv bo_glMultiTexCoord1iv
#define glMultiTexCoord1s bo_glMultiTexCoord1s
#define glMultiTexCoord1sv bo_glMultiTexCoord1sv
#define glMultiTexCoord2d bo_glMultiTexCoord2d
#define glMultiTexCoord2dv bo_glMultiTexCoord2dv
#define glMultiTexCoord2f bo_glMultiTexCoord2f
#define glMultiTexCoord2fv bo_glMultiTexCoord2fv
#define glMultiTexCoord2i bo_glMultiTexCoord2i
#define glMultiTexCoord2iv bo_glMultiTexCoord2iv
#define glMultiTexCoord2s bo_glMultiTexCoord2s
#define glMultiTexCoord2sv bo_glMultiTexCoord2sv
#define glMultiTexCoord3d bo_glMultiTexCoord3d
#define glMultiTexCoord3dv bo_glMultiTexCoord3dv
#define glMultiTexCoord3f bo_glMultiTexCoord3f
#define glMultiTexCoord3fv bo_glMultiTexCoord3fv
#define glMultiTexCoord3i bo_glMultiTexCoord3i
#define glMultiTexCoord3iv bo_glMultiTexCoord3iv
#define glMultiTexCoord3s bo_glMultiTexCoord3s
#define glMultiTexCoord3sv bo_glMultiTexCoord3sv
#define glMultiTexCoord4d bo_glMultiTexCoord4d
#define glMultiTexCoord4dv bo_glMultiTexCoord4dv
#define glMultiTexCoord4f bo_glMultiTexCoord4f
#define glMultiTexCoord4fv bo_glMultiTexCoord4fv
#define glMultiTexCoord4i bo_glMultiTexCoord4i
#define glMultiTexCoord4iv bo_glMultiTexCoord4iv
#define glMultiTexCoord4s bo_glMultiTexCoord4s
#define glMultiTexCoord4sv bo_glMultiTexCoord4sv
#define glClientActiveTexture bo_glClientActiveTexture
#define glActiveTexture bo_glActiveTexture
#define glLoadTransposeMatrixd bo_glLoadTransposeMatrixd
#define glLoadTransposeMatrixf bo_glLoadTransposeMatrixf
#define glMultTransposeMatrixd bo_glMultTransposeMatrixd
#define glMultTransposeMatrixf bo_glMultTransposeMatrixf

#endif // BOGL_DO_DLOPEN

#endif

