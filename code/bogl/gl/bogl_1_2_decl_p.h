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

#ifndef BOGL_1_2_DECL_P_H
#define BOGL_1_2_DECL_P_H

#include "bogl_1_1_decl_p.h"

/*
 additions in OpenGL 1.2:
 previously extensions and now directly included:
 * EXT_texture3D
 * EXT_bgra
 * EXT_packed_pixels
 * EXT_rescale_normal
 * EXT_separate_specular_color
 * SGIS_texture_edge_clamp
 * SGIS_texture_lod
 * EXT_draw_range_elements

 as part of the imaging subset:
 * EXT_color_table
 * EXT_color_subtable
 * EXT_convolution
 * HP_convolution_border_modes
 * SGI_color_matrix
 * EXT_histogram
 * EXT_blend_color
 * EXT_blend_minmax
 * EXT_blend_subtract
*/

/*
 new functions:
 * EXT_texture3D:
   glTexImage3D()
   glTexSubImage3D()     (not mentioned in the extension!)
   glCopyTexSubImage3D() (not mentioned in the extension!)
 * EXT_bgra:
   none
 * EXT_packed_pixels:
   none
 * EXT_rescale_normal:
   none
 * EXT_separate_specular_color:
   none
 * SGIS_texture_edge_clamp:
   none
 * SGIS_texture_lod:
   none
 * EXT_draw_range_elements:
   glDrawRangeElements()

 as part of the imaging subset:
 * EXT_color_table
   glColorTable()
   glCopyColorTable()
   glColorTableParameteriv()
   glColorTableParameterfv()
   glGetColorTable()
   glGetColorTableParameteriv()
   glGetColorTableParameterfv()
 * EXT_color_subtable
   glColorSubTable()
   glCopyColorSubTable()
 * EXT_convolution
   glConvolutionFilter1D()
   glConvolutionFilter2D()
   glCopyConvolutionFilter1D()
   glCopyConvolutionFilter2D()
   glGetConvolutionFilter()
   glSeparableFilter2D()
   glGetSeparableFilter()
   glConvolutionParameteri()
   glConvolutionParameteriv()
   glConvolutionParameterf()
   glConvolutionParameterfv()
   glGetConvolutionParameteriv()
   glGetConvolutionParameterfv()
 * HP_convolution_border_modes
   none
 * SGI_color_matrix
   none
 * EXT_histogram
   glHistogram()
   glResetHistogram()
   glGetHistogram()
   glGetHistogramParameteriv()
   glGetHistogramParameterfv()
   glMinmax()
   glResetMinmax()
   glGetMinmax()
   glGetMinmaxParameteriv()
   glGetMinmaxParameterfv()
 * EXT_blend_color
   glBlendColor()
 * EXT_blend_minmax
   glBlendEquation()
 * EXT_blend_subtract
   none

*/


extern "C" {
	// GL typedefs
	typedef void (*_glTexImage3D)(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
	typedef void (*_glTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
	typedef void (*_glCopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	typedef void (*_glDrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
	typedef void (*_glColorTable)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
	typedef void (*_glCopyColorTable)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
	typedef void (*_glColorTableParameteriv)(GLenum target, GLenum pname, const GLint *params);
	typedef void (*_glColorTableParameterfv)(GLenum target, GLenum pname, const GLfloat *params);
	typedef void (*_glGetColorTable)(GLenum target, GLenum format, GLenum type, GLvoid *table);
	typedef void (*_glGetColorTableParameterfv)(GLenum target, GLenum pname, GLfloat *params);
	typedef void (*_glGetColorTableParameteriv)(GLenum target, GLenum pname, GLint *params);
	typedef void (*_glColorSubTable)(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
	typedef void (*_glCopyColorSubTable)(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
	typedef void (*_glConvolutionFilter1D)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
	typedef void (*_glConvolutionFilter2D)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
	typedef void (*_glCopyConvolutionFilter1D)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
	typedef void (*_glCopyConvolutionFilter2D)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
	typedef void (*_glGetConvolutionFilter)(GLenum target, GLenum format, GLenum type, GLvoid *image);
	typedef void (*_glSeparableFilter2D)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
	typedef void (*_glGetSeparableFilter)(GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
	typedef void (*_glConvolutionParameterf)(GLenum target, GLenum pname, GLfloat params);
	typedef void (*_glConvolutionParameterfv)(GLenum target, GLenum pname, const GLfloat *params);
	typedef void (*_glConvolutionParameteri)(GLenum target, GLenum pname, GLint params);
	typedef void (*_glConvolutionParameteriv)(GLenum target, GLenum pname, const GLint *params);
	typedef void (*_glGetConvolutionParameterfv)(GLenum target, GLenum pname, GLfloat *params);
	typedef void (*_glGetConvolutionParameteriv)(GLenum target, GLenum pname, GLint *params);
	typedef void (*_glHistogram)(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
	typedef void (*_glResetHistogram)(GLenum target);
	typedef void (*_glGetHistogram)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
	typedef void (*_glGetHistogramParameterfv)(GLenum target, GLenum pname, GLfloat *params);
	typedef void (*_glGetHistogramParameteriv)(GLenum target, GLenum pname, GLint *params);
	typedef void (*_glMinmax)(GLenum target, GLenum internalformat, GLboolean sink);
	typedef void (*_glResetMinmax)(GLenum target);
	typedef void (*_glGetMinmax)(GLenum target, GLboolean reset, GLenum format, GLenum types, GLvoid *values);
	typedef void (*_glGetMinmaxParameterfv)(GLenum target, GLenum pname, GLfloat *params);
	typedef void (*_glGetMinmaxParameteriv)(GLenum target, GLenum pname, GLint *params);
	typedef void (*_glBlendColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
	typedef void (*_glBlendEquation)(GLenum mode);



	// GL function pointers
	extern _glTexImage3D bo_glTexImage3D;
	extern _glTexSubImage3D bo_glTexSubImage3D;
	extern _glCopyTexSubImage3D bo_glCopyTexSubImage3D;
	extern _glDrawRangeElements bo_glDrawRangeElements;
	extern _glColorTable bo_glColorTable;
	extern _glCopyColorTable bo_glCopyColorTable;
	extern _glColorTableParameteriv bo_glColorTableParameteriv;
	extern _glColorTableParameterfv bo_glColorTableParameterfv;
	extern _glGetColorTable bo_glGetColorTable;
	extern _glGetColorTableParameterfv bo_glGetColorTableParameterfv;
	extern _glGetColorTableParameteriv bo_glGetColorTableParameteriv;
	extern _glColorSubTable bo_glColorSubTable;
	extern _glCopyColorSubTable bo_glCopyColorSubTable;
	extern _glConvolutionFilter1D bo_glConvolutionFilter1D;
	extern _glConvolutionFilter2D bo_glConvolutionFilter2D;
	extern _glCopyConvolutionFilter1D bo_glCopyConvolutionFilter1D;
	extern _glCopyConvolutionFilter2D bo_glCopyConvolutionFilter2D;
	extern _glGetConvolutionFilter bo_glGetConvolutionFilter;
	extern _glSeparableFilter2D bo_glSeparableFilter2D;
	extern _glGetSeparableFilter bo_glGetSeparableFilter;
	extern _glConvolutionParameterf bo_glConvolutionParameterf;
	extern _glConvolutionParameterfv bo_glConvolutionParameterfv;
	extern _glConvolutionParameteri bo_glConvolutionParameteri;
	extern _glConvolutionParameteriv bo_glConvolutionParameteriv;
	extern _glGetConvolutionParameterfv bo_glGetConvolutionParameterfv;
	extern _glGetConvolutionParameteriv bo_glGetConvolutionParameteriv;
	extern _glHistogram bo_glHistogram;
	extern _glResetHistogram bo_glResetHistogram;
	extern _glGetHistogram bo_glGetHistogram;
	extern _glGetHistogramParameterfv bo_glGetHistogramParameterfv;
	extern _glGetHistogramParameteriv bo_glGetHistogramParameteriv;
	extern _glMinmax bo_glMinmax;
	extern _glResetMinmax bo_glResetMinmax;
	extern _glGetMinmax bo_glGetMinmax;
	extern _glGetMinmaxParameterfv bo_glGetMinmaxParameterfv;
	extern _glGetMinmaxParameteriv bo_glGetMinmaxParameteriv;
	extern _glBlendColor bo_glBlendColor;
	extern _glBlendEquation bo_glBlendEquation;

}; // extern "C"


// GL defines
#if BOGL_DO_DLOPEN

#define glTexImage3D bo_glTexImage3D
#define glTexSubImage3D bo_glTexSubImage3D
#define glCopyTexSubImage3D bo_glCopyTexSubImage3D
#define glDrawRangeElements bo_glDrawRangeElements
#define glColorTable bo_glColorTable
#define glCopyColorTable bo_glCopyColorTable
#define glColorTableParameteriv bo_glColorTableParameteriv
#define glColorTableParameterfv bo_glColorTableParameterfv
#define glGetColorTable bo_glGetColorTable
#define glGetColorTableParameterfv bo_glGetColorTableParameterfv
#define glGetColorTableParameteriv bo_glGetColorTableParameteriv
#define glColorSubTable bo_glColorSubTable
#define glCopyColorSubTable bo_glCopyColorSubTable
#define glConvolutionFilter1D bo_glConvolutionFilter1D
#define glConvolutionFilter2D bo_glConvolutionFilter2D
#define glCopyConvolutionFilter1D bo_glCopyConvolutionFilter1D
#define glCopyConvolutionFilter2D bo_glCopyConvolutionFilter2D
#define glGetConvolutionFilter bo_glGetConvolutionFilter
#define glSeparableFilter2D bo_glSeparableFilter2D
#define glGetSeparableFilter bo_glGetSeparableFilter
#define glConvolutionParameterf bo_glConvolutionParameterf
#define glConvolutionParameterfv bo_glConvolutionParameterfv
#define glConvolutionParameteri bo_glConvolutionParameteri
#define glConvolutionParameteriv bo_glConvolutionParameteriv
#define glGetConvolutionParameterfv bo_glGetConvolutionParameterfv
#define glGetConvolutionParameteriv bo_glGetConvolutionParameteriv
#define glHistogram bo_glHistogram
#define glResetHistogram bo_glResetHistogram
#define glGetHistogram bo_glGetHistogram
#define glGetHistogramParameterfv bo_glGetHistogramParameterfv
#define glGetHistogramParameteriv bo_glGetHistogramParameteriv
#define glMinmax bo_glMinmax
#define glResetMinmax bo_glResetMinmax
#define glGetMinmax bo_glGetMinmax
#define glGetMinmaxParameterfv bo_glGetMinmaxParameterfv
#define glGetMinmaxParameteriv bo_glGetMinmaxParameteriv
#define glBlendColor bo_glBlendColor
#define glBlendEquation bo_glBlendEquation

#endif // BOGL_DO_DLOPEN

#endif
